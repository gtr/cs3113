#include "glm/detail/qualifier.hpp"
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

const int FONTBANK_SIZE = 16;

/*
 * [x] Constructor for an instance of Entity. All values will be set to 0.0f
 * except the model matrix.
 */
Entity::Entity() {
    speed = 0;
    position = glm::vec3(0.0f);
    velocity = glm::vec3(0.0f);
    acceleration = glm::vec3(0.0f);
    movement = glm::vec3(0.0f);
    model_matrix = glm::mat4(1.0f);
}

/*
 * [x] Updates the x position.
 */
void Entity::update_x_position(float delta_time, Entity *entities, int entity_count) {
    position.x += velocity.x * delta_time;
    check_collision_x(entities, entity_count);
}

/*
 * [x] Updates the y position.
 */
void Entity::update_y_position(float delta_time, Entity *entities, int entity_count) {
  position.y += velocity.y * delta_time;
  check_collision_y(entities, entity_count);
}

/*
 * [ ] Updates the internal state of an Entity instance.
 */
void Entity::update(float delta_time, Entity *entities, int entity_count) {
    collided_bottom = false;
    collided_left   = false;
    collided_right  = false;
    
    velocity.x = movement.x * speed;
    velocity += acceleration * delta_time;

    // Update x and y positions.
    update_y_position(delta_time, entities, entity_count);
    update_x_position(delta_time, entities, entity_count);
    
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
}

/*
 * [ ] Checks for collisions with all possible entities in the y-direction.
 */
void const Entity::check_collision_y(Entity *entities, int entity_count) {
    for (int i = 0; i < entity_count; i++) {
        Entity *entity = &entities[i];
        
        if (check_collision(entity)) {
            float y_distance = fabs(position.y - entity->position.y);
            float y_overlap = fabs(y_distance - (height / 2.0f) - (entity->height / 2.0f));
            if (velocity.y < 0) {
                position.y      += y_overlap;
                velocity.y       = 0;
                collided_bottom  = true;
            }
            
            // Check if rocket has touched the target.
            const int PLATFORM_COUNT = 23;
            if (i == PLATFORM_COUNT) {
                status = WON;
            } else {
                // Display "lost!"
                status = LOST;
            }
            
        }
    }
}

/*
 * [ ] Checks for collisions with all possible entities in the x-direction.
 */
void const Entity::check_collision_x(Entity *entities, int entity_count) {
    for (int i = 0; i < entity_count; i++) {
        Entity *entity = &entities[i];
        
        if (check_collision(entity)) {
            float x_distance = fabs(position.x - entity->position.x);
            float x_overlap = fabs(x_distance - (width / 2.0f) - (entity->width / 2.0f));
            
            if (velocity.x > 0) {
                position.x     -= x_overlap;
                velocity.x      = 0;
                collided_right  = true;
            } else if (velocity.x < 0) {
                position.x    += x_overlap;
                velocity.x     = 0;
                collided_left  = true;
            }

            // Check if rocket has touched the target.
            const int PLATFORM_COUNT = 23;
            if (i == PLATFORM_COUNT) {
              status = WON;
            } else {
              // Display "lost!"
              status = LOST;
            }
        }
    }
}

/* 
 * [x] Renders an entity.
 */
void Entity::render(ShaderProgram *program) {
    program->SetModelMatrix(model_matrix);
    
    float vertices[]   = { 
        -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, 
        -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 
    };

    float tex_coords[] = {  
        0.0,  1.0, 1.0,  1.0, 1.0, 0.0,
         0.0,  1.0, 1.0, 0.0,  0.0, 0.0 
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
bool const Entity::check_collision(Entity *other) {
    float x_distance = fabs(position.x - other->position.x) - ((width  + other->width)  / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
    
    return x_distance < 0.0f && y_distance < 0.0f;
}
