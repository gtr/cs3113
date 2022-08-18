#include "LevelA.h"
#include "Utility.h"
#include "Entity.h"
#include "glm/ext/vector_float3.hpp"
#include <SDL2/SDL_opengl.h>

#define LEVEL_WIDTH 86
#define LEVEL_HEIGHT 8
#define FIRE_COUNT 20
#define SHOOTER_COUNT 4
#define ENEMY_FIRE_COUNT 20
#define GLIDER_COUNT 6
#define ROCK_COUNT 20
#define SPINNER_COUNT 80

// Keeps track of which fire entity to point at.
int fire_ptr = 0;

const char FIRE_FILEPATH[] = "assets/fire.png";
unsigned int LEVEL_DATA[] = {
    1, 4, 3, 9, 4, 1, 9, 8, 1, 9, 1, 3, 7, 5, 4, 5, 4, 6, 9, 7, 4, 3, 9, 4, 1, 2, 3, 5, 2, 3, 1, 4, 2, 3, 5, 4, 3, 5, 2, 3, 1, 4, 2, 3, 5, 4,  3, 5, 2, 3, 1, 4, 2, 3, 5, 4,  3, 5, 2, 3, 1, 4, 2, 3, 5, 4,  3, 5, 2, 3, 1, 4, 2, 3, 5, 4,3, 5, 2, 3, 1, 4, 2, 3, 5, 4,
    6, 5, 9, 9, 9, 2, 5, 6, 9, 9, 6, 1, 6, 2, 9, 1, 2, 7, 9, 6, 3, 1, 9, 2, 6, 7, 5, 2, 3, 6, 8, 9, 7, 1, 6, 3, 5, 2, 3, 6, 8, 9, 7, 1, 6, 3,  5, 2, 3, 6, 8, 9, 7, 1, 6, 3,  5, 2, 3, 6, 8, 9, 7, 1, 6, 3,  5, 2, 3, 6, 8, 9, 7, 1, 6, 3,5, 2, 3, 6, 8, 9, 7, 1, 6, 3,
    1, 2, 8, 4, 7, 9, 8, 9, 5, 2, 6, 4, 9, 9, 4, 7, 1, 3, 5, 6, 1, 5, 7, 1, 3, 4, 5, 7, 1, 9, 2, 3, 9, 5, 3, 1, 5, 7, 1, 9, 2, 3, 9, 5, 3, 1,  5, 7, 1, 9, 2, 3, 9, 5, 3, 1,  5, 7, 1, 9, 2, 3, 9, 5, 3, 1,  5, 7, 1, 9, 2, 3, 9, 5, 3, 1,5, 7, 1, 9, 2, 3, 9, 5, 3, 1,
    5, 2, 9, 7, 8, 1, 3, 5, 8, 7, 9, 7, 6, 8, 1, 6, 2, 6, 8, 4, 5, 6, 2, 8, 9, 1, 6, 4, 7, 3, 7, 5, 4, 3, 7, 2, 6, 4, 7, 3, 7, 5, 4, 3, 7, 2,  6, 4, 7, 3, 7, 5, 4, 3, 7, 2,  6, 4, 7, 3, 7, 5, 4, 3, 7, 2,  6, 4, 7, 3, 7, 5, 4, 3, 7, 2,6, 4, 7, 3, 7, 5, 4, 3, 7, 2,
    1, 5, 3, 4, 6, 7, 1, 4, 5, 6, 1, 2, 3, 5, 9, 1, 8, 3, 6, 7, 3, 9, 8, 6, 4, 7, 8, 9, 3, 6, 2, 1, 5, 4, 8, 1, 8, 9, 3, 6, 2, 1, 5, 4, 8, 1,  8, 9, 3, 6, 2, 1, 5, 4, 8, 1,  8, 9, 3, 6, 2, 1, 5, 4, 8, 1,  8, 9, 3, 6, 2, 1, 5, 4, 8, 1,8, 9, 3, 6, 2, 1, 5, 4, 8, 1,
    2, 7, 4, 5, 2, 9, 8, 7, 6, 7, 4, 3, 4, 6, 7, 5, 7, 4, 1, 3, 9, 6, 1, 2, 9, 4, 3, 6, 9, 7, 8, 6, 2, 5, 6, 4, 3, 6, 9, 7, 8, 6, 2, 5, 6, 4,  3, 6, 9, 7, 8, 6, 2, 5, 6, 4,  3, 6, 9, 7, 8, 6, 2, 5, 6, 4,  3, 6, 9, 7, 8, 6, 2, 5, 6, 4,3, 6, 9, 7, 8, 6, 2, 5, 6, 4,
    2, 3, 1, 9, 7, 3, 9, 3, 8, 5, 6, 1, 5, 2, 9, 6, 3, 9, 5, 4, 2, 1, 6, 5, 4, 6, 4, 3, 2, 6, 5, 2, 3, 2, 1, 4, 4, 3, 2, 6, 5, 2, 3, 2, 1, 4,  4, 3, 2, 6, 5, 2, 3, 2, 1, 4,  4, 3, 2, 6, 5, 2, 3, 2, 1, 4,  4, 3, 2, 6, 5, 2, 3, 2, 1, 4,4, 3, 2, 6, 5, 2, 3, 2, 1, 4,
    6, 1, 2, 9, 2, 6, 5, 4, 6, 2, 5, 4, 7, 3, 2, 8, 6, 4, 7, 5, 6, 8, 7, 1, 3, 2, 8, 5, 8, 7, 3, 6, 2, 9, 4, 6, 8, 5, 8, 7, 3, 6, 2, 9, 4, 6,  8, 5, 8, 7, 3, 6, 2, 9, 4, 6,  8, 5, 8, 7, 3, 6, 2, 9, 4, 6,  8, 5, 8, 7, 3, 6, 2, 9, 4, 6,8, 5, 8, 7, 3, 6, 2, 9, 4, 6,
};


LevelA::~LevelA() {
    delete      this->state.player;
    delete      this->state.map;
    // delete []   this->state.fire;
    // delete this->state.boss;
    // delete [] this->state.gliders;
    // delete[] this->state.gliders_w2;
    // delete[] this->state.gliders_w3;
    // delete [] this->state.rocks;
}

void LevelA::shoot_fire() {
    if (this->state.fire[fire_ptr].get_fire_state() == OFF) {
        this->state.fire[fire_ptr].activate();
    } else {
        this->state.fire[fire_ptr].fire_off();
        this->state.fire[fire_ptr].set_position(this->state.player->get_position());
        this->state.fire[fire_ptr].activate();
    }
    fire_ptr += 1;
    if (fire_ptr > FIRE_COUNT) {
        fire_ptr = 0;
    }
}

void LevelA::initialize_fire() {
    GLuint fire_texture_id = Utility::load_texture("assets/fire.png");
    this->state.fire = new Entity[FIRE_COUNT];

    for (int i = 0; i < FIRE_COUNT; i++) {
        this->state.fire[i].set_entity_type(FIRE);
        this->state.fire[i].set_ai_type(SHOOT);
        this->state.fire[i].set_fire_state(OFF);
        this->state.fire[i].set_width(0.04f);
        this->state.fire[i].set_height(0.04f);
        this->state.fire[i].set_movement(glm::vec3(0.0f));
        this->state.fire[i].speed = 1.0f;
        this->state.fire[i].texture_id = fire_texture_id;
        this->state.fire[i].deactivate();
    }
}

float increase(float &num) { return 0.5f + num; }

void LevelA::initialize_shooters() {
    this->state.shooters = new Entity[SHOOTER_COUNT];

    GLuint shooter_up_texture = Utility::load_texture("assets/gun-up.png");
    GLuint shooter_down_texture = Utility::load_texture("assets/gun-down.png");
    GLuint shooter_left_texture = Utility::load_texture("assets/gun-left.png");
    GLuint shooter_right_texture = Utility::load_texture("assets/gun-right.png");
    GLuint health_bar_texture = Utility::load_texture("assets/health-bar.png");
    GLuint explosion_texture = Utility::load_texture("assets/explosion.png");
    GLuint enemy_fire_texture  = Utility::load_texture("assets/enemy-fire.png");

    glm::vec3 positions[SHOOTER_COUNT] = {
        glm::vec3(19.0f, 0.0f, 0.0f),   glm::vec3(31.0f, -6.2f, 0.0f),
        glm::vec3(50.0f, -5.2f, 0.0f), glm::vec3(44.0f, -2.2f, 0.0f)
    };

    GLuint textures[SHOOTER_COUNT] = {
        shooter_down_texture,  shooter_up_texture,
        shooter_left_texture, shooter_left_texture
    };

    FireDirection directions[SHOOTER_COUNT] = {
        DOWN, UP,LEFT, LEFT
    };

    for (int i = 0; i < SHOOTER_COUNT; i++) {
        // Shooter info
        this->state.shooters[i].init_health(4);
        this->state.shooters[i].set_entity_type(SHOOTER);
        this->state.shooters[i].set_width(0.4f);
        this->state.shooters[i].set_height(0.4f);
        this->state.shooters[i].set_position(positions[i]);
        this->state.shooters[i].texture_id = textures[i];

        // Health bar
        this->state.shooters[i].has_health_bar = true;
        this->state.shooters[i].health_bar = new Entity;
        this->state.shooters[i].health_bar->set_entity_type(HEALTH_BAR);
        this->state.shooters[i].health_bar->texture_id = health_bar_texture;
        this->state.shooters[i].health_bar->set_position(positions[i]);
        this->state.shooters[i].health_bar->activate();

        // Explosion
        this->state.shooters[i].has_explosion = true;
        this->state.shooters[i].explosion_entity = new Entity;
        this->state.shooters[i].explosion_entity->set_entity_type(EXPLOSION);
        this->state.shooters[i].explosion_entity->texture_id = explosion_texture;
        this->state.shooters[i].explosion_entity->set_position(positions[i]);
        this->state.shooters[i].explosion_entity->deactivate();

        // Enemy fire
        this->state.shooters[i].has_enemy_fire = true;
        this->state.shooters[i].enemy_fire = new Entity[ENEMY_FIRE_COUNT];
        
        for (int j = 0; j < ENEMY_FIRE_COUNT; j++) {
            this->state.shooters[i].enemy_fire[j].set_entity_type(ENEMY_FIRE);
            this->state.shooters[i].enemy_fire[j].set_fire_state(OFF);
            this->state.shooters[i].enemy_fire[j].speed = 1.0f;
            this->state.shooters[i].enemy_fire[j].texture_id = enemy_fire_texture;
            this->state.shooters[i].enemy_fire[j].set_position(positions[i]);
            this->state.shooters[i].enemy_fire[j].deactivate();
            this->state.shooters[i].enemy_fire[j].set_fire_direction(directions[i]);
        }

    }
}

void LevelA::initialize_gliders() {
    GLuint glider_texture = Utility::load_texture("assets/glider.png");
    GLuint explosion_texture = Utility::load_texture("assets/explosion.png");
    GLuint health_bar_texture = Utility::load_texture("assets/health-bar.png");

    this->state.gliders = new Entity[GLIDER_COUNT];
    this->state.gliders_w2 = new Entity[GLIDER_COUNT];
    this->state.gliders_w3 = new Entity[GLIDER_COUNT];

    glm::vec3 positions[GLIDER_COUNT] = {
        glm::vec3(10.0, -3.2f, 0.0f), glm::vec3(20.5, -3.2f, 0.0f),
        glm::vec3(11.0f, -3.2f, 0.0f), glm::vec3(21.5f, -3.2f, 0.0f),
        glm::vec3(12.0f, -3.2f, 0.0f), glm::vec3(22.5f, -3.2f, 0.0f)
    };

        for (int i = 0; i < GLIDER_COUNT; i++) {
      this->state.gliders[i].deactivate();
      this->state.gliders[i].init_health(3);
      this->state.gliders[i].set_entity_type(GLIDER);
      this->state.gliders[i].set_width(0.4f);
      this->state.gliders[i].set_height(0.4f);
      this->state.gliders[i].set_ai_state(GLIDE_DOWN);
      this->state.gliders[i].texture_id = glider_texture;
      this->state.gliders[i].speed = 0.75f;
      this->state.gliders[i].set_movement(glm::vec3(-1, 0, 0));

      // Health bar
      this->state.gliders[i].has_health_bar = true;
      this->state.gliders[i].health_bar = new Entity;
      this->state.gliders[i].health_bar->set_entity_type(HEALTH_BAR);
      this->state.gliders[i].health_bar->texture_id = health_bar_texture;
      this->state.gliders[i].health_bar->set_position(positions[i]);
      this->state.gliders[i].health_bar->activate();

      // Explosion
      this->state.gliders[i].has_explosion = true;
      this->state.gliders[i].explosion_entity = new Entity;
      this->state.gliders[i].explosion_entity->set_entity_type(EXPLOSION);
      this->state.gliders[i].explosion_entity->texture_id = explosion_texture;
      this->state.gliders[i].explosion_entity->set_position(positions[i]);
      this->state.gliders[i].explosion_entity->deactivate();
    }

    for (int i = 0; i < GLIDER_COUNT; i++) {
        this->state.gliders_w2[i].deactivate();
        this->state.gliders_w2[i].init_health(3);
        this->state.gliders_w2[i].set_entity_type(GLIDER);
        this->state.gliders_w2[i].set_width(0.4f);
        this->state.gliders_w2[i].set_height(0.4f);
        this->state.gliders_w2[i].set_position(positions[i]);
        this->state.gliders_w2[i].set_ai_state(GLIDE_UP);
        this->state.gliders_w2[i].texture_id = glider_texture;
        this->state.gliders_w2[i].speed = 0.75f;
        this->state.gliders_w2[i].set_movement(glm::vec3(-1, 0, 0));

        // Health bar
        this->state.gliders_w2[i].has_health_bar = true; 
        this->state.gliders_w2[i].health_bar = new Entity;
        this->state.gliders_w2[i].health_bar->set_entity_type(HEALTH_BAR);
        this->state.gliders_w2[i].health_bar->texture_id = health_bar_texture;
        this->state.gliders_w2[i].health_bar->set_position(positions[i]);
        this->state.gliders_w2[i].health_bar->activate();

        // Explosion
        this->state.gliders_w2[i].has_explosion = true;
        this->state.gliders_w2[i].explosion_entity = new Entity;
        this->state.gliders_w2[i].explosion_entity->set_entity_type(EXPLOSION);
        this->state.gliders_w2[i].explosion_entity->texture_id = explosion_texture;
        this->state.gliders_w2[i].explosion_entity->set_position(positions[i]);
        this->state.gliders_w2[i].explosion_entity->deactivate();
    }

    for (int i = 0; i < GLIDER_COUNT; i++) {
        this->state.gliders_w3[i].deactivate();
        this->state.gliders_w3[i].init_health(3);
        this->state.gliders_w3[i].set_entity_type(GLIDER);
        this->state.gliders_w3[i].set_width(0.4f);
        this->state.gliders_w3[i].set_height(0.4f);
        this->state.gliders_w3[i].set_position(positions[i]);
        this->state.gliders_w3[i].set_ai_state(GLIDE_DOWN);
        this->state.gliders_w3[i].texture_id = glider_texture;
        this->state.gliders_w3[i].speed = 0.75f;
        this->state.gliders_w3[i].set_movement(glm::vec3(-1, 0, 0));

        // Health bar
        this->state.gliders_w3[i].has_health_bar = true; 
        this->state.gliders_w3[i].health_bar = new Entity;
        this->state.gliders_w3[i].health_bar->set_entity_type(HEALTH_BAR);
        this->state.gliders_w3[i].health_bar->texture_id = health_bar_texture;
        this->state.gliders_w3[i].health_bar->set_position(positions[i]);
        this->state.gliders_w3[i].health_bar->activate();

        // Explosion
        this->state.gliders_w3[i].has_explosion = true;
        this->state.gliders_w3[i].explosion_entity = new Entity;
        this->state.gliders_w3[i].explosion_entity->set_entity_type(EXPLOSION);
        this->state.gliders_w3[i].explosion_entity->texture_id = explosion_texture;
        this->state.gliders_w3[i].explosion_entity->set_position(positions[i]);
        this->state.gliders_w3[i].explosion_entity->deactivate();
    }    
}

void LevelA::initialize_boss() {
    GLuint boss_texture = Utility::load_texture("assets/boss.png");
    GLuint explosion_texture = Utility::load_texture("assets/explosion.png");
    GLuint health_bar_texture = Utility::load_texture("assets/health-bar.png");
    GLuint spinner_texture = Utility::load_texture("assets/spinner.png");

    glm::vec3 position = glm::vec3(84, -3, 0);

    this->state.boss = new Entity;

    this->state.boss->deactivate();
    this->state.boss->init_health(30);
    this->state.boss->set_ai_state(GLIDE_UP);
    this->state.boss->set_entity_type(BOSS);
    this->state.boss->set_width(0.7f);
    this->state.boss->set_height(0.6f);
    this->state.boss->set_position(position);
    this->state.boss->texture_id = boss_texture;
    this->state.boss->speed = 0.75f;
    this->state.boss->set_movement(glm::vec3(0, 1, 0));

    // Health bar
    this->state.boss->has_health_bar = true;
    this->state.boss->health_bar = new Entity;
    this->state.boss->health_bar->set_entity_type(HEALTH_BAR);
    this->state.boss->health_bar->texture_id = health_bar_texture;
    this->state.boss->health_bar->set_position(position);
    this->state.boss->health_bar->activate();

    // Explosion
    this->state.boss->has_explosion = true;
    this->state.boss->explosion_entity = new Entity;
    this->state.boss->explosion_entity->set_entity_type(EXPLOSION);
    this->state.boss->explosion_entity->texture_id = explosion_texture;
    this->state.boss->explosion_entity->set_position(position);
    this->state.boss->explosion_entity->deactivate();

    this->state.boss->has_spinners = true;
    this->state.boss->spinners = new Entity[SPINNER_COUNT];
    for (int i = 0; i < SPINNER_COUNT; i++) {

      this->state.boss->spinners[i].deactivate();
      this->state.boss->spinners[i].set_entity_type(SPINNER);
      this->state.boss->spinners[i].set_width(0.7f);
      this->state.boss->spinners[i].set_height(0.6f);
      this->state.boss->spinners[i].set_ai_state(GLIDE_UP);
      this->state.boss->spinners[i].texture_id = spinner_texture;
      this->state.boss->spinners[i].speed = 0.75f;
      this->state.boss->spinners[i].set_movement(glm::vec3(-1, 0, 0));

      this->state.boss->spinners[i].init_health(5);
      this->state.boss->spinners[i].has_health_bar = true;
      this->state.boss->spinners[i].health_bar = new Entity;
      this->state.boss->spinners[i].health_bar->set_entity_type(HEALTH_BAR);
      this->state.boss->spinners[i].health_bar->texture_id = health_bar_texture;
      this->state.boss->spinners[i].health_bar->set_position(position);
      this->state.boss->spinners[i].health_bar->activate();

      this->state.boss->spinners[i].has_explosion = true;
      this->state.boss->spinners[i].explosion_entity = new Entity;
      this->state.boss->spinners[i].explosion_entity->set_entity_type(EXPLOSION);
      this->state.boss->spinners[i].explosion_entity->texture_id = explosion_texture;
    //   this->state.boss->spinners[i].explosion_entity->set_position(position);
      this->state.boss->spinners[i].explosion_entity->deactivate();
    }
}

void LevelA::initialize_rocks() {
    GLuint rock_texture = Utility::load_texture("assets/rocks.png");
    GLuint explosion_texture = Utility::load_texture("assets/explosion.png");

    this->state.rocks = new Entity[ROCK_COUNT];

    glm::vec3 rock_positions[ROCK_COUNT] = {
        glm::vec3(10.0f, -3.2f, 0.0f), glm::vec3(48.0f, -2.2f, 0.0f),
        glm::vec3(12.0f, -4.2f, 0.0f), glm::vec3(48.0f, -1.9f, 0.0f),
        glm::vec3(26.0f, -1.7f, 0.0f), glm::vec3(39.0f, -3.2f, 0.0f),
        glm::vec3(24.0f, -5.2f, 0.0f), glm::vec3(55.0f, -2.9f, 0.0f),
        glm::vec3(32.0f, -3.8f, 0.0f), glm::vec3(69.0f, -0.2f, 0.0f),
        glm::vec3(30.0f, -3.8f, 0.0f), glm::vec3(71.0f, -4.2f, 0.0f),
        glm::vec3(27.0f, -3.2f, 0.0f), glm::vec3(56.0f, -0.8f, 0.0f),
        glm::vec3(21.0f, -0.2f, 0.0f), glm::vec3(68.0f, -3.2f, 0.0f),
        glm::vec3(57.0f, -3.2f, 0.0f), glm::vec3(46.0f, -0.8f, 0.0f),
        glm::vec3(13.0f, -0.2f, 0.0f), glm::vec3(48.0f, -3.2f, 0.0f),
    };

    int idx[ROCK_COUNT] = {
        0, 1, 0, 2, 1, 1, 0, 2, 0, 1, 
        0, 2, 1, 2, 2, 1, 1, 0, 1, 2,
    };

    for (int i = 0; i < ROCK_COUNT; i++) {
        this->state.rocks[i].rock_sprite_idx = idx[i];
        this->state.rocks[i].init_health(1);
        this->state.rocks[i].set_entity_type(ROCK);
        this->state.rocks[i].set_width(0.4f);
        this->state.rocks[i].set_position(rock_positions[i]);
        this->state.rocks[i].set_height(0.4f);
        this->state.rocks[i].texture_id = rock_texture;
        this->state.rocks[i].speed = 0.45f;
        this->state.rocks[i].set_movement(glm::vec3(-1, 0, 0));
        this->state.rocks[i].activate();

        this->state.rocks[i].has_explosion = true;
        this->state.rocks[i].explosion_entity = new Entity;
        this->state.rocks[i].explosion_entity->set_entity_type(EXPLOSION);
        this->state.rocks[i].explosion_entity->texture_id = explosion_texture;
        this->state.rocks[i].explosion_entity->set_position(rock_positions[i]);
        this->state.rocks[i].explosion_entity->deactivate();
    }
}

void LevelA::initialize_enemies() {
    initialize_shooters();
    initialize_gliders();
    initialize_boss();
    initialize_rocks();
}

void LevelA::initialize_map() {
    GLuint map_texture_id = Utility::load_texture("assets/levelA.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 10, 1);
}

void LevelA::initialize_coins() {
    state.coins = new Entity;
    state.coins->set_entity_type(COIN);
    state.coins->set_position(glm::vec3(6.0f, -2.5f, 0.0f));
    state.coins->texture_id = Utility::load_texture("assets/coin.png");
    state.coins->set_height(0.2f);
    state.coins->set_width(0.2f);
}

void LevelA::initialize_player() {
    GLuint health_bar_texture = Utility::load_texture("assets/health-bar.png");
    GLuint player_texture = Utility::load_texture("assets/ship.png");
    GLuint explosion_texture = Utility::load_texture("assets/explosion.png");

    glm::vec3 position = glm::vec3(4.5f, -3.5f, 0.0f);

    state.player = new Entity();
    state.player->activate();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(position);
    state.player->set_movement(glm::vec3(0.0f));
    state.player->set_height(0.6f);
    state.player->set_width(0.4f);
    state.player->speed = 3.5f;
    state.player->texture_id = player_texture;
    state.player->has_health_bar = true;
    state.player->init_health(8);

    state.player->health_bar = new Entity;
    state.player->health_bar->set_entity_type(HEALTH_BAR);
    state.player->health_bar->texture_id = health_bar_texture;
    state.player->health_bar->set_position(position);
    state.player->health_bar->activate();

    state.player->has_explosion = true;
    state.player->explosion_entity = new Entity;
    state.player->explosion_entity->set_entity_type(EXPLOSION);
    state.player->explosion_entity->texture_id = explosion_texture;
    state.player->explosion_entity->set_position(position);
    state.player->explosion_entity->deactivate();
}

void LevelA::initialise() {
    initialize_map();
    initialize_player();
    initialize_fire();
    initialize_coins();
    initialize_enemies();
}

void LevelA::update(float delta_time) { 


    for (int i = 0; i < SHOOTER_COUNT; i++) {
        this->state.shooters[i].update(delta_time, state.player, state.fire, FIRE_COUNT, state.map);
    }

    for (int i = 0; i < GLIDER_COUNT; i++) {
      this->state.gliders[i].update(delta_time, state.player, state.fire, FIRE_COUNT, state.map);
    }

    for (int i = 0; i < GLIDER_COUNT; i++) {
      this->state.gliders_w2[i].update(delta_time, state.player, state.fire, FIRE_COUNT, state.map);
    }

    for (int i = 0; i < GLIDER_COUNT; i++) {
      this->state.gliders_w3[i].update(delta_time, state.player, state.fire, FIRE_COUNT, state.map);
    }

    for (int i = 0; i < ROCK_COUNT; i ++) {
        this->state.rocks[i].update(delta_time, state.player, state.fire, FIRE_COUNT, state.map);
    }

    this->state.boss->update(delta_time, state.player, state.fire, FIRE_COUNT, state.map);

    this->state.player->update(delta_time, state.player, state.fire, FIRE_COUNT,
                               state.map);

    for (int i = 0; i < FIRE_COUNT; i++) {
      this->state.fire[i].update(delta_time, state.player, state.fire, FIRE_COUNT, state.map);
    }

    // if (this->state.boss->curr_lives <= 0.9) {
    //     this->state.boss_dead = true;
    // }

    // if (this->state.player->curr_lives <= 0.9) {
    //     this->state.player_dead = true;
    // }
}

void LevelA::render(ShaderProgram *program) {
    this->state.map->render(program);

    for (int i = 0; i < SHOOTER_COUNT; i++) {
        this->state.shooters[i].render(program);
    }

    for (int i = 0; i < GLIDER_COUNT; i++) {
        this->state.gliders[i].render(program);
    }

    for (int i = 0; i < GLIDER_COUNT; i++) {
        this->state.gliders_w2[i].render(program);
    }

    for (int i = 0; i < GLIDER_COUNT; i++) {
        this->state.gliders_w3[i].render(program);
    }

    for (int i = 0; i < ROCK_COUNT; i ++) {
        this->state.rocks[i].render(program);
    }

    for (int i = 0; i < FIRE_COUNT; i++) {
      state.fire[i].render(program);
    }

    this->state.boss->render(program);
    this->state.player->render(program);
}
