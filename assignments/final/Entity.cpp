#include "Utility.h"
#include "glm/ext/vector_float3.hpp"
#define GL_SILENCE_DEPRECATION

#define ENEMY_FIRE_COUNT 20

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define SPINNER_COUNT 80

#define GL_GLEXT_PROTOTYPES 1
#include "Entity.h"
#include "ShaderProgram.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Constructor for our entity.
Entity::Entity() {
    position = glm::vec3(0.0f);
    velocity = glm::vec3(0.0f);
    acceleration = glm::vec3(0.0f);

    movement = glm::vec3(0.0f);

    speed = 0;
    model_matrix = glm::mat4(1.0f);
}

Entity::~Entity() {
    delete [] enemy_fire;
    delete [] spinners;
    // delete explosion_entity;
    // delete health_bar;
}

void Entity::fire_shoot(Entity *player) {
    if (!is_active) {
        std::cout << "fire_shoot inactive\n";
        return;
    }
    
    switch (player->get_direction()) {
    case PLAYER_UP:
        set_movement(glm::vec3(0.0f, 5.0f, 0.0f));
        break;
    case PLAYER_DOWN:
        set_movement(glm::vec3(0.0f, -5.0f, 0.0f));
        break;
    case PLAYER_LEFT:
        set_movement(glm::vec3(-5.0f, 0.0f, 0.0f));
        break;
    case PLAYER_RIGHT:
        set_movement(glm::vec3(5.0f, 0.0f, 0.0f));
        break;
    default:
        break;
    }
}

// Activates behavior for an AI entity.
void Entity::activate_ai(Entity *player) {

}

void Entity::explode() {
    if (has_explosion) {
        if (entity_type == BOSS) {
            this->status = WON;
        }
        if (entity_type == PLAYER) {
            this->status = LOST;
        }
        std::cout << "exploding\n";
        explosion_entity->activate();
    }
}

void Entity::update(float delta_time, Entity* player, Entity* fire, int fire_count, Map *map) {
    if (entity_type == SHOOTER || entity_type == GLIDER || entity_type == ROCK || entity_type == BOSS || entity_type == SPINNER) {
        if (this->has_explosion) {
            this->explosion_entity->update(delta_time, player, fire, fire_count, map);
        }
    }
    if (!is_active) { return; }

    ticks += 1;

    collided_top = false;
    collided_bottom = false;
    collided_left = false;
    collided_right = false;

    if (entity_type == SPINNER) {
        if (player->get_x_pos() > this->get_x_pos()) {
            this->movement.x = 2;
        }
        if (player->get_y_pos() > this->get_y_pos()) {
            this->movement.y = 2;
        }
        if (player->get_x_pos() < this->get_x_pos()) {
            this->movement.x = -2;
        }
        if (player->get_y_pos() < this->get_y_pos()) {
            this->movement.y = -2;
        }
    }

    if (entity_type == FIRE && fire_state == OFF) {
        this->set_position(player->get_position());
        this->fire_shoot(player);
        fire_state = ON;
    } 

    if (has_spinners) {
        for (int i = 0; i < SPINNER_COUNT; i++ ) {
            this->spinners[i].update(delta_time, player, fire, fire_count, map);
        }
    }

    if (entity_type == ENEMY_FIRE && fire_state == OFF) {
        fire_state = ON;
        switch (fire_direction) {
            case UP:
                set_movement(glm::vec3(0, 3, 0));
                break;
            case DOWN:
                set_movement(glm::vec3(0, -3, 0));
                break;
            case LEFT:
                set_movement(glm::vec3(-3, 0, 0));
                break;
            case RIGHT:
                set_movement(glm::vec3(3, 0, 0));
                break;
        }
    }

    if (entity_type == GLIDER) {
        float y = player->get_y_pos();
        if (ai_state == GLIDE_DOWN) {
            if (ticks == 200) {
                set_movement(glm::vec3(1.2, -1.5, 0));
            }

            if (ticks == 350) {
                set_movement(glm::vec3(-2.5, 0, 0));
            }
        } else {
            if (ticks == 200) {
                set_movement(glm::vec3(1.2, 1.5, 0));
            }

            if (ticks == 350) {
                set_movement(glm::vec3(-2.5, 0, 0));
            }
        }

    }

    if (entity_type == BOSS) {

        if (this->get_ai_state() == GLIDE_DOWN && this->get_y_pos() <= -5.7) {
            this->movement.y = 1.;
            this->set_ai_state(GLIDE_UP);
        }

        if (this->get_ai_state() == GLIDE_UP && this->get_y_pos() >= -1) {
            this->movement.y = -1;
            this->set_ai_state(GLIDE_DOWN);
        }

        if (ticks % 375 == 0 || ticks % 440 == 0) {
            if (spinner_idx <= SPINNER_COUNT)  {
                this->spinners[spinner_idx].set_position(this->get_position());
                this->spinners[spinner_idx].activate();

                spinner_idx += 1;
            }
        }

    }


    if (entity_type == HEALTH_BAR) {
        // this->set_position(player->get_position());
    }

    if (has_health_bar) {
        this->health_bar->update(delta_time, player, fire, fire_count, map);
        glm::vec3 position = glm::vec3(this->get_position().x, this->get_position().y + 0.15f, 0.0f);
        this->health_bar->set_position(position);
        this->set_position(glm::vec3(position.x, position.y - 0.15f, 0.0f));
    }

    // Update velocity vector
    velocity.x = movement.x * speed;
    velocity.y = movement.y * speed;

    // Now we add the rest of the gravity physics
    velocity += acceleration * delta_time;

    // Update position
    position.y += velocity.y * delta_time;
    position.x += velocity.x * delta_time;

    if (has_explosion) {
        this->explosion_entity->update(delta_time, player, fire, fire_count, map);
    }

    if (entity_type == SHOOTER && has_enemy_fire) {
        enemy_fire_interval += 1;
        if (enemy_fire_interval % 9 == 0 && enemy_fire_interval % 11 == 0 ) {
            if (this->enemy_fire[enemy_fire_ptr].get_fire_state() == OFF) {
                this->enemy_fire[enemy_fire_ptr].activate();
            } else {
                this->enemy_fire[enemy_fire_ptr].fire_off();
                this->enemy_fire[enemy_fire_ptr].set_position(this->get_position());
                this->enemy_fire[enemy_fire_ptr].activate();
            }

            enemy_fire_ptr += 1;
            if (enemy_fire_ptr > ENEMY_FIRE_COUNT) {
                enemy_fire_ptr = 0;
            }
        }
        
    }

    if (has_enemy_fire) {
        for (int i = 0; i < ENEMY_FIRE_COUNT; i++) {
            this->enemy_fire[i].update(delta_time, player,fire, fire_count, map);
        }
    }

    // Check enemy collision with player's fire.
    if (entity_type == SHOOTER || entity_type == GLIDER || entity_type == ROCK || entity_type == BOSS || entity_type == SPINNER) {
        this->explosion_entity->update(delta_time, player, fire, fire_count, map);
        this->explosion_entity->set_position(this->get_position());

        for (int i = 0; i < fire_count; i++) {
            if (check_collision(&fire[i])) {
                fire[i].deactivate();

                this->decrease_lives();
            }
        }
    }

    if (entity_type == SHOOTER || entity_type == GLIDER || entity_type == ROCK || entity_type == SPINNER || entity_type == ENEMY_FIRE) {
        if (check_collision(player)) {
            this->explode();
            this->deactivate();
            player->decrease_lives();
        }
    }

    if (entity_type == BOSS) {

    }

      //     // Check collision with player
      //     // player->decrease_lives();
      //     // this->decrease_lives();
      // }

      // if (entity_type == ENEMY_FIRE) {
      //     if (check_collision(player)) {
      //         this->deactivate();
      //         player->decrease_lives();
      //     }
      // }

      // Check collision with map.
      // check_collision_y(map);
      // check_collision_x(map);

      // Update model matrix.
      model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
}

/*
 *
 */
void const Entity::check_collision_y(Entity *collidable_entities, int collidable_entity_count) {
    for (int i = 0; i < collidable_entity_count; i++) {
        Entity *collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity)) {
            float y_distance = fabs(position.y - collidable_entity->position.y);
            float y_overlap = fabs(y_distance - (height / 2.0f) - (collidable_entity->height / 2.0f));
            if (velocity.y > 0) {
                position.y -= y_overlap;
                velocity.y = 0;
                collided_top = true;
            } else if (velocity.y < 0) {
                position.y += y_overlap;
                velocity.y = 0;
                collided_bottom = true;
            }
        }
    }
}

void const Entity::check_collision_x(Entity *collidable_entities, int collidable_entity_count) {
    for (int i = 0; i < collidable_entity_count; i++) {
        Entity *collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity)) {
            float x_distance = fabs(position.x - collidable_entity->position.x);
            float x_overlap = fabs(x_distance - (width / 2.0f) - (collidable_entity->width / 2.0f));
            if (velocity.x > 0) {
                position.x -= x_overlap;
                velocity.x = 0;
                collided_right = true;
            } else if (velocity.x < 0) {
                position.x += x_overlap;
                velocity.x = 0;
                collided_left = true;
            }
        }
    }
}

void const Entity::check_collision_y(Map *map) {
    // Probes for tiles
    glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
    glm::vec3 top_left = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
    glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);

    glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
    glm::vec3 bottom_left = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
    glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(top, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collided_top = true;
    } else if (map->is_solid(top_left, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collided_top = true;
    } else if (map->is_solid(top_right, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collided_top = true;
    }

    if (map->is_solid(bottom, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collided_bottom = true;
    } else if (map->is_solid(bottom_left, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collided_bottom = true;
    } else if (map->is_solid(bottom_right, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collided_bottom = true;
    }
}

void const Entity::check_collision_x(Map *map) {
    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);

    float penetration_x = 0;
    float penetration_y = 0;

    if (map->is_solid(left, &penetration_x, &penetration_y) && velocity.x < 0) {
        position.x += penetration_x;
        velocity.x = 0;
        collided_left = true;
    }
    if (map->is_solid(right, &penetration_x, &penetration_y) && velocity.x > 0) {
        position.x -= penetration_x;
        velocity.x = 0;
        collided_right = true;
    }
}

void draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index, int rows, int cols) {
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float)(index % cols) / (float)cols;
    float v_coord = (float)(index / cols) / (float)rows;

    // Step 2: Calculate its UV size
    float width = 1.0f / (float)cols;
    float height = 1.0f / (float)rows;

    // Step 3: Just as we have done before, match the texture coordinates to the
    // vertices
    float tex_coords[] = {u_coord,          v_coord + height, u_coord + width,
                            v_coord + height, u_coord + width,  v_coord,
                            u_coord,          v_coord + height, u_coord + width,
                            v_coord,          u_coord,          v_coord};

    float vertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5,  0.5,
                        -0.5, -0.5, 0.5, 0.5,  -0.5, 0.5};

    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0,
                            vertices);
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0,
                            tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::render_health_bar(ShaderProgram *program) {
    ::draw_sprite_from_texture_atlas(program, texture_id, health_bar_idx, 1, 10);
}

void Entity::render(ShaderProgram *program) {
    if (entity_type == SHOOTER || entity_type == GLIDER || entity_type == ROCK || entity_type == BOSS || entity_type == SPINNER) {
        if (this->has_explosion) {
            this->explosion_entity->render(program);
        }
    } 
    
    if (!is_active) return;

    if (has_spinners) {
        for (int i = 0; i < SPINNER_COUNT; i++ ) {
            this->spinners[i].render(program);
        }
    }

    if (has_health_bar) {
        health_bar->render(program);
    }

    if (has_enemy_fire) {
        for (int i = 0; i < ENEMY_FIRE_COUNT; i++) {
            enemy_fire[i].render(program);
        }
    }

    program->SetModelMatrix(model_matrix);

    if (entity_type == PLAYER) {
        ::draw_sprite_from_texture_atlas(program, texture_id, player_spite_idx, 1, 4);
        return;
    } else if (this->entity_type == COIN) {
        coin_buffer_idx += 1;
        if (coin_buffer_idx > 6) {
            coin_sprite_idx += 1;
            coin_buffer_idx = 0;
        }
        ::draw_sprite_from_texture_atlas(program, texture_id, coin_sprite_idx, 1, 6);
        return;
    } else if (this->entity_type == HEALTH_BAR) {
        ::draw_sprite_from_texture_atlas(program, texture_id, health_bar_idx, 1, 10);
        return;
    } else if (this->entity_type == ROCK) {
        ::draw_sprite_from_texture_atlas(program, texture_id, rock_sprite_idx, 1, 3);
        return;
    } else if (this->entity_type == ENEMY_FIRE) {
        enemy_fire_buffer_idx += 1;
        if (enemy_fire_buffer_idx > 4) {
            enemy_fire_sprite_idx += 1;
            enemy_fire_buffer_idx = 0;
        }
        ::draw_sprite_from_texture_atlas(program, texture_id, enemy_fire_sprite_idx, 1, 2);
        return;
    } else if (this->entity_type == BOSS) {
        ::draw_sprite_from_texture_atlas(program, texture_id, 0, 1, 2);
        return;
    } else if (this->entity_type == SPINNER) {
        spinner_buffer_idx += 1;
        if (spinner_buffer_idx > 2) {
            spinner_sprite_idx += 1;
            spinner_buffer_idx = 0;
        }
      ::draw_sprite_from_texture_atlas(program, texture_id, spinner_sprite_idx, 1, 8);
      return;
    } else if (this->entity_type == GLIDER) {
        glider_buffer_idx += 1;
        if (glider_buffer_idx > 6) {
            glider_buffer_idx = 0;
            glider_sprite_idx += 1;
        }
        ::draw_sprite_from_texture_atlas(program, texture_id, glider_sprite_idx, 1, 5);
        return;
    } else if (this->entity_type == EXPLOSION) {
        if (this->explosion_sprite_idx == 13) {
            deactivate();
            return;
        }
        explosion_buffer_idx += 1;
        if (explosion_buffer_idx > 1) {
            explosion_sprite_idx += 1;
            explosion_buffer_idx = 0;
        }
        ::draw_sprite_from_texture_atlas(program, texture_id, explosion_sprite_idx, 1, 13);
        return;
    }

    float vertices[] = {
        -0.5, -0.5, 0.5, -0.5, 0.5,  0.5,
        -0.5, -0.5, 0.5, 0.5,  -0.5, 0.5
    };
    float tex_coords[] = {
        0.0, 1.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 1.0, 1.0, 0.0, 0.0, 0.0
    };

    glBindTexture(GL_TEXTURE_2D, texture_id);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool const Entity::check_collision(Entity *other) const {
    if (other == this)
        return false;

    // If either entity is inactive, there shouldn't be any collision
    if (!is_active || !other->is_active) return false;

    float x_distance = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

    return x_distance < 0.0f && y_distance < 0.0f;
}
