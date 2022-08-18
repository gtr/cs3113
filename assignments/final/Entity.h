#pragma once
#include "Map.h"
#include "ShaderProgram.h"
#include "glm/ext/vector_float3.hpp"

enum GameStatus {RUNNING, WON, LOST};
enum EntityType {
  PLATFORM,
  PLAYER,
  ENEMY,
  SPINNER,
  FIRE,
  BOSS,
  COIN,
  SHOOTER,
  ENEMY_FIRE,
  HEALTH_BAR,
  EXPLOSION,
  GLIDER,
  ROCK,
};
enum AIType { WALKER, GUARD, SHOOT, GUN_DOWN, GUN_UP, GUN_LEFT, GUN_RIGHT };
enum AIState    { WALKING, IDLE, ATTACKING, GLIDE_UP, GLIDE_DOWN };
enum FireState  { OFF, ON };
enum FireDirection {LEFT, RIGHT, UP, DOWN};

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

    // Ticks
    int ticks = 0;

    // Player animation
    int player_spite_idx = 0;
    
    // Coin animation
    int coin_sprite_idx = 0;
    int coin_buffer_idx = 0;

    // Explosion animation
    int explosion_sprite_idx = 0;
    int explosion_buffer_idx = 0;

    // Health bar animation
    int health_bar_idx = 0;

    // Enemy fire animation;
    int enemy_fire_sprite_idx = 0;
    int enemy_fire_buffer_idx = 0;

    // Enemy fire AI
    int enemy_fire_ptr = 0;
    int enemy_fire_interval = 0;
    FireDirection fire_direction;

    // Glider animation
    int glider_sprite_idx = 0;
    int glider_buffer_idx = 0;

    int spinner_sprite_idx = 0;
    int spinner_buffer_idx = 0;

    int spinner_idx = 0;

  public:
    // Rock indx
    int rock_sprite_idx = 0;
    Entity* spinners;
    bool has_spinners = false;

    // Health
    bool has_health_bar = false;
    float curr_lives;
    float total_lives;
    Entity * health_bar;

    // Explosion
    bool has_explosion = false;
    Entity* explosion_entity;

    // Enemy fire
    bool has_enemy_fire = false;
    Entity* enemy_fire;


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

    void set_ticks(int new_ticks) { ticks = new_ticks; };

    void render_health_bar(ShaderProgram *program);

    void draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index);
    void update(float delta_time, Entity *player, Entity* fire, int fire_count, Map *map);
    void render(ShaderProgram *program);
    void activate_ai(Entity *player);
    void ai_walker();
    void ai_guard(Entity *player);

    // Shooter
    void fire_shoot(Entity *player);
    void set_lives(int total) { curr_lives = total; total_lives = total; }
    void init_shooter() {
        enemy_fire = new Entity[10];
    }
    void explode();

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

    void set_fire_direction(FireDirection d) { fire_direction = d; };
    FireDirection get_fire_direction() { return fire_direction; };

    void init_health(float health) { curr_lives = health; total_lives = health; }

    EntityType const get_entity_type() const { return entity_type; };
    AIType const get_ai_type() const { return ai_type; };
    AIState const get_ai_state() const { return ai_state; };
    glm::vec3 const get_position() { 
        glm::vec3 position_copy = glm::vec3(0.0f, 0.0f, 0.0f);
        position_copy.x = position.x;
        position_copy.y = position.y;
        return position_copy;
    };

    float const get_x_pos() const {return position.x; };
    float const get_y_pos() const { return position.y; };

    glm::vec3 const get_movement() const { return movement; };
    glm::vec3 const get_velocity() const { return velocity; };
    glm::vec3 const get_acceleration() const { return acceleration; };
    int const get_width() const { return width; };
    int const get_height() const { return height; };

    // Player movement
    void move_left() { acceleration.x = -10.0f; movement.x = -0.75f; player_spite_idx = PLAYER_LEFT; }
    void move_right() { acceleration.x = 10.0f; movement.x = .75f; player_spite_idx = PLAYER_RIGHT;}
    void move_up() { acceleration.y = 5.0f; movement.y = 0.75f; player_spite_idx = PLAYER_UP; }
    void move_down() { acceleration.y = -5.0f; movement.y = -0.75f; player_spite_idx = PLAYER_DOWN; }
    void set_x(float new_x) { position.x = new_x; };
    void set_y(float new_y) { position.y = new_y; };

    int get_direction() { return this->player_spite_idx; }

    void player_shoot();
    void fire_off() { deactivate(); fire_state = OFF; }

    void decrease_lives() {
        if (has_explosion) {
            explosion_entity->set_position(this->get_position());
        }

        curr_lives -= 1;
        double health = curr_lives / total_lives;

        if (has_health_bar) {
            if (0.9 <= health && health <= 1.0) {
                this->health_bar->health_bar_idx = 0;
            } else if (0.8 <= health && health <= 0.9) {
                this->health_bar->health_bar_idx = 1;
            } else if (0.7 <= health && health < 0.8) {
                this->health_bar->health_bar_idx = 2;
            } else if (0.6 <= health && health < 0.7) {
                this->health_bar->health_bar_idx = 3;
            } else if (0.5 <= health && health < 0.6) {
                this->health_bar->health_bar_idx = 4;
            } else if (0.4 <= health && health < 0.5) {
                this->health_bar->health_bar_idx = 5;
            } else if (0.3 <= health && health < 0.4) {
                this->health_bar->health_bar_idx = 6;
            } else if (0.2 <= health && health < 0.3) {
                this->health_bar->health_bar_idx = 7;
            } else if (0.1 <= health && health < 0.2) {
                this->health_bar->health_bar_idx = 8;
            } else if (0.0 <= health && health < 0.1) {
                this->health_bar->health_bar_idx = 9;
            }
        }

        if (curr_lives <= 0.09) {
            explode();
            deactivate();
        }
    }


    void const set_entity_type(EntityType new_entity_type) { entity_type = new_entity_type;      };
    void const set_fire_state(FireState new_fire_state) { fire_state = new_fire_state; };
    FireState get_fire_state() { return fire_state; };
    void const set_ai_type(AIType new_ai_type)              { ai_type      = new_ai_type;          };
    void const set_ai_state(AIState new_state)              { ai_state     = new_state;            };
    void const set_position(glm::vec3 new_position)         { position     = new_position;         };
    void const set_movement(glm::vec3 new_movement)         { movement     = new_movement;         };
    void const set_velocity(glm::vec3 new_velocity)         { velocity     = new_velocity;         };
    void const set_acceleration(glm::vec3 new_acceleration) { acceleration = new_acceleration;     };
    void const set_width(float new_width)                   { width        = new_width;            };
    void const set_height(float new_height)                 { height       = new_height;           };
};
