#include "glm/ext/vector_float3.hpp"
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include "Entity.h"
#include "ShaderProgram.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

/*
 * [x] Constructor for an instance of Entity. All values will be set to 0.0f
 * except the model matrix.
 */
Entity::Entity() {
    position     = glm::vec3(0.0f);
    velocity     = glm::vec3(0.0f);
    acceleration = glm::vec3(0.0f);
    
    movement = glm::vec3(0.0f);
    
    speed = 0;
    model_matrix = glm::mat4(1.0f);
}

/* 
 * [ ] Destructor for an instance of Entity.
 */
Entity::~Entity() {
    delete [] animation_up;
    delete [] animation_down;
    delete [] animation_left;
    delete [] animation_right;
    delete [] walking;
}

void Entity::draw_sprite_from_texture_atlas(
    ShaderProgram *program, GLuint texture_id, int index
) {
   
    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    // glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    // glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::activate_ai(Entity *player) {
    switch (ai_type) {
        case WALKER:
            ai_walker();
            break;
            
        case GUARD:
            ai_guard(player);
            break;
            
        case FLY:
            ai_fly();
            break;

        case SHOOT:
            ai_shoot(player);

        default:
            break;
    }
}

void Entity::ai_walker() {

    switch (ai_state)  {
        case WALK_LEFT:
            set_acceleration(glm::vec3(-1.0f, -9.81f, 0.0f)); 
            if (position.x < 1.0f)  {
                set_acceleration(glm::vec3(1.0f,-9.81f, 0.0f));
                ai_state = WALK_RIGHT;
                break;
            }
        case WALK_RIGHT:
            set_acceleration(glm::vec3(1.0f, -9.81f, 0.0f));
            if (position.x < 1.0f)  {
                set_acceleration(glm::vec3(-1.0f, -9.81f, 0.0f));
                ai_state = WALK_LEFT;
                break;
            }
        default:  break;
    }
}

void Entity::ai_fly() {
    switch (ai_state) {
        case FLY_UP:
            set_acceleration(glm::vec3(0.0f, 3.81f, 0.0f));
            if (position.y > 4.0f) {
                set_acceleration(glm::vec3(0.0f, -4.81f, 0.0f));
                ai_state = FLY_DOWN;
                break;
            }
        case FLY_DOWN:
            set_acceleration(glm::vec3(0.0f, -3.81f, 0.0f));
            if (position.y < -1.0f) {
                set_acceleration(glm::vec3(0.0f, 4.81f, 0.0f));
                ai_state = FLY_UP;
                break;
            }

        default: break;
    }
}

void Entity::ai_guard(Entity *player) {
    switch (ai_state) {
        case IDLE:
            if (glm::distance(position, player->position) < 4.0f) ai_state = WALKING;
            break;
            
        case WALKING:
            if (position.x > player->get_position().x) {
                movement = glm::vec3(-3.0f, 0.0f, 0.0f);
            } else {
                movement = glm::vec3(3.0f, 0.0f, 0.0f);
            }
            break;
            
        case ATTACKING:
            break;
            
        default:
            break;
    }
}

void Entity::ai_shoot(Entity *player) {
    switch (fire_state) {
        case ON_LEFT:
            set_movement(glm::vec3(-5.0f, 0.0f, 0.0f));
            break;
        case ON_RIGHT:
            set_movement(glm::vec3(5.0f, 0.0f, 0.0f));
            break;
        case OFF:
            deactivate();
        default: break;
    }
}

/*
 * [ ] Updates the internal state of an Entity instance.
 */
void Entity::update(
    float delta_time, Entity *player,  Entity* fire,
    Entity *collidable_entities, int collidable_entity_count
) {
    if (!is_active) return;

 
    collided_top    = false;
    collided_bottom = false;
    collided_left   = false;
    collided_right  = false;
    
    if (entity_type == ENEMY) {
        activate_ai(player);
        if (check_collision(player)) {
            player->status = LOST;
            LOG("LOST!");
        }
        if (check_collision(fire)) {
            deactivate();
            fire->enemy_count -= 1;
        }
    }

    if (entity_type == FIRE) activate_ai(player);


    // Our character moves from left to right, so they need an initial
    // velocity
    velocity.x = movement.x * speed;
    
    // Now we add the rest of the gravity physics
    velocity += acceleration * delta_time;
    
    position.y += velocity.y * delta_time;
    check_collision_y(collidable_entities, collidable_entity_count);

    position.x += velocity.x * delta_time;
    check_collision_x(collidable_entities, collidable_entity_count);

    // Jump
    if (is_jumping) {
        // STEP 1: Immediately return the flag to its original false state
        is_jumping = false;
        
        // STEP 2: The player now acquires an upward velocity
        velocity.y += jumping_power;
    }

    if (entity_type == ENEMY && collided_left) {
      movement = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    if (entity_type == ENEMY && position.x > 5.0f) {
      movement = glm::vec3(-1.0f, 0.0f, 0.0f);
    }

    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
}

void const Entity::check_collision_fire(Entity* enemies, int enemy_count) {
    for (int i = 0; i < enemy_count; i++) {
        Entity* enemy = &enemies[i];
        if (check_collision(enemy)) {
            enemy->deactivate();
        }
    }
}

void const Entity::check_collision_player(Entity* entity) {
    if (check_collision(entity)) {
        float y_distance = fabs(position.y - entity->position.y);
        float y_overlap = fabs(y_distance - (height / 2.0f) - (entity->height / 2.0f));

        if (velocity.y > 0) {
            position.y -= y_overlap;
            velocity.y = 0;
            collided_top = true;
        } else if (velocity.y < 0) {
            position.y += y_overlap;
            velocity.y = 0;
            collided_bottom = true;
        }
        deactivate();
    }
}

/*
 * [x] Checks for collisions with all possible entities in the y-direction.
 */
void const Entity::check_collision_y(Entity *entities, int entity_count) {
    for (int i = 0; i < entity_count; i++) {
        Entity *entity = &entities[i];

        if (check_collision(entity)) {
            float y_distance = fabs(position.y - entity->position.y);
            float y_overlap = fabs(y_distance - (height / 2.0f) - (entity->height / 2.0f));

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

/*
 * [x] Checks for collisions with all possible entities in the x-direction.
 */
void const Entity::check_collision_x(Entity *collidable_entities, int entity_count) {
    for (int i = 0; i < entity_count; i++) {
        Entity *entity = &collidable_entities[i];

        if (check_collision(entity)) {
            float x_distance = fabs(position.x - entity->position.x);
            float x_overlap = fabs(x_distance - (width / 2.0f) - (entity->width / 2.0f));
            
            if (velocity.x > 0) {
                position.x      -= x_overlap;
                velocity.x      = 0;
                collided_right  = true;
            } else if (velocity.x < 0) {
                position.x      += x_overlap;
                velocity.x      = 0;
                collided_left   = true;
            }
        }
    }
}

/*
 * [x] Renders an entity.
 */
void Entity::render(ShaderProgram *program) {
    if (!is_active) return;

    program->SetModelMatrix(model_matrix);

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

/*
 * [x] Returns a boolean indicating wether this entity has collided with another
 * given entity, `other`.
 */
bool const Entity::check_collision(Entity *other) const {
    if (!is_active || !other->is_active || other == this) return false;

    float x_distance = fabs(position.x - other->position.x) - ((width  + other->width)  / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
    
    return (x_distance < 0.0f) && (y_distance < 0.0f);
}
