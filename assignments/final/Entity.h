#pragma once
#include "Map.h"

enum GameStatus {RUNNING, WON, LOST};
enum EntityType { PLATFORM, PLAYER, ENEMY, FIRE, COIN };
enum AIType { WALKER, GUARD, SHOOT };
enum AIState    { WALKING, IDLE, ATTACKING };
enum FireState  { OFF, ON_LEFT, ON_RIGHT };

const int PLAYER_LEFT = 2;
const int PLAYER_RIGHT = 0;
const int PLAYER_DOWN = 1;
const int PLAYER_UP = 3;

class Entity
{
private:
    bool is_active = true;
    EntityType entity_type;
    AIType ai_type;
    AIState ai_state;
    FireState fire_state;

    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    
    float width  = 0.8f;
    float height = 0.8f;

    int player_spite_idx = 0;
    int coin_sprite_idx = 0;
    int coin_buffer_idx = 0;
    
public:
    GameStatus status;
    int enemies_dead = 0;
    // Static attributes
    static const int SECONDS_PER_FRAME = 4;

    // Existing
    GLuint texture_id;
    glm::mat4 model_matrix;

    // Translating
    float speed;
    glm::vec3 movement;

    // Jumping
    bool is_jumping = false;
    float jumping_power = 0;

    // Colliding
    bool collided_top = false;
    bool collided_bottom = false;
    bool collided_left = false;
    bool collided_right = false;

    // Methods
    Entity();
    ~Entity();

    void draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index);
    void update(float delta_time, Entity *player, Map *map);
    void render(ShaderProgram *program);
    void activate_ai(Entity *player);
    void ai_walker();
    void ai_guard(Entity *player);
    void ai_shoot(Entity *player);

    void const check_collision_y(Entity *collidable_entities,
                                int collidable_entity_count);
    void const check_collision_x(Entity *collidable_entities,
                                int collidable_entity_count);
    void const check_collision_y(Map *map);
    void const check_collision_x(Map *map);

    void const check_collision_fire(Entity *enemies, int enemy_count);

    bool const check_collision(Entity *other) const;

    void activate() { is_active = true; };
    void deactivate() { is_active = false; };

    EntityType const get_entity_type() const { return entity_type; };
    AIType const get_ai_type() const { return ai_type; };
    AIState const get_ai_state() const { return ai_state; };
    glm::vec3 const get_position() const { return position; };
    glm::vec3 const get_movement() const { return movement; };
    glm::vec3 const get_velocity() const { return velocity; };
    glm::vec3 const get_acceleration() const { return acceleration; };
    int const get_width() const { return width; };
    int const get_height() const { return height; };

    // Player movement
    void move_left() { movement.x = -1.0f; player_spite_idx = PLAYER_LEFT; }
    void move_right() { movement.x = 1.0f; player_spite_idx = PLAYER_RIGHT; }
    void move_up() { movement.y = 1.0f; player_spite_idx = PLAYER_UP; }
    void move_down() { movement.y = -1.0f; player_spite_idx = PLAYER_DOWN; }

    int get_direction() { return this->player_spite_idx; }

    void const set_entity_type(EntityType new_entity_type) { entity_type = new_entity_type;      };
    void const set_fire_state(FireState new_fire_state) { fire_state = new_fire_state; };
    void const set_ai_type(AIType new_ai_type)              { ai_type      = new_ai_type;          };
    void const set_ai_state(AIState new_state)              { ai_state     = new_state;            };
    void const set_position(glm::vec3 new_position)         { position     = new_position;         };
    void const set_movement(glm::vec3 new_movement)         { movement     = new_movement;         };
    void const set_velocity(glm::vec3 new_velocity)         { velocity     = new_velocity;         };
    void const set_acceleration(glm::vec3 new_acceleration) { acceleration = new_acceleration;     };
    void const set_width(float new_width)                   { width        = new_width;            };
    void const set_height(float new_height)                 { height       = new_height;           };
};
