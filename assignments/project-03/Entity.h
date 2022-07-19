
#include "ShaderProgram.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include <SDL2/SDL_opengl.h>

enum EntityType { PLATFORM, PLAYER, ITEM };
enum GameStatus { RUNNING, WON, LOST };

/// Entity represents an object in our lunar lander game.
class Entity {
    private:
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec3 acceleration;
        
        float width  = 1;
        float height = 1;
    public:
        // Existing
        GLuint texture_id;
        glm::mat4 model_matrix;
        EntityType type;
        GameStatus status;

        // Translating
        float speed;
        glm::vec3 movement;
        
        // Colliding
        bool collided_bottom = false;
        bool collided_left   = false;
        bool collided_right  = false;

        Entity();

        void update_x_position(float delta_time, Entity *entities, int entity_count);
        void update_y_position(float delta_time, Entity *entities, int entity_count);
        void update(float delta_time, Entity *collidable_entities, int collidable_entity_count);
        void render(ShaderProgram *program);
        
        void const check_collision_y(Entity *collidable_entities, int collidable_entity_count);
        void const check_collision_x(Entity *collidable_entities, int collidable_entity_count);
        bool const check_collision(Entity *other);
        
        /// Setters.
        void const set_position(glm::vec3 new_position)         { position = new_position;         };
        void const set_movement(glm::vec3 new_movement)         { movement = new_movement;         };
        void const set_velocity(glm::vec3 new_velocity)         { velocity = new_velocity;         };
        void const set_acceleration(glm::vec3 new_acceleration) { acceleration = new_acceleration; };
        void const set_width(float new_width)                   { width = new_width;               };
        void const set_height(float new_height)                 { height = new_height;             };

        void const add_acceleration(glm::vec3 add_vector)           { acceleration += add_vector; };
};
