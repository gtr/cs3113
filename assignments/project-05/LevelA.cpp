#include "LevelA.h"
#include "Utility.h"
#include "Entity.h"

#define LEVEL_WIDTH 16
#define LEVEL_HEIGHT 8

const char FIRE_FILEPATH[] = "img/fire.png";

unsigned int LEVEL_DATA[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1,
};

LevelA::~LevelA() {
    delete []   this->state.enemies;
    delete      this->state.player;
    delete      this->state.map;
    delete      this->state.fire;
    // Mix_FreeChunk(this->state.jump_sfx);
    // Mix_FreeMusic(this->state.bgm);
}

void LevelA::initialize_fire() {
    GLuint fire_tecture_id = Utility::load_texture(FIRE_FILEPATH);
    this->state.fire = new Entity();

    this->state.fire->set_entity_type(FIRE);
    this->state.fire->set_ai_type(SHOOT);
    this->state.fire->set_fire_state(OFF);
    this->state.fire->set_width(0.04f);
    this->state.fire->set_height(0.04f);
    this->state.fire->set_movement(glm::vec3(0.0f));
    this->state.fire->speed = 1.0f;
    this->state.fire->texture_id = fire_tecture_id;
    this->state.fire->deactivate();
}

void LevelA::initialize_enemies() {
    GLuint enemy_texture_id = Utility::load_texture("img/alienA.png");

    state.enemies = new Entity[ENEMY_COUNT];
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(GUARD);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(8.0f, -6.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    state.enemies[0].set_height(0.8f);
    state.enemies[0].set_width(0.6f);
}

void LevelA::initialise() {
    GLuint map_texture_id = Utility::load_texture("img/moon-brick.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 2, 1);
    // Code from main.cpp's initialise()
    /**
        George's Stuff
        */
    // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(2.0f, -2.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.player->texture_id = Utility::load_texture("img/scout.png");
    
    // This is the final level.
    this->final = false;

    state.player->animation_indices = state.player->walking[state.player->RIGHT];  // start George looking left
    // state.player->animation_frames = 4;
    // state.player->animation_index  = 0;
    // state.player->animation_time   = 0.0f;
    state.player->set_height(0.8f);
    state.player->set_width(0.6f);
    
    // Jumping
    state.player->jumping_power = 6.1f;
    
    /**
     Enemies' stuff */
    this->initialize_enemies();

    this->initialize_fire();

    /**
     BGM and SFX
     */
    // Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    // state.bgm = Mix_LoadMUS("img/dooblydoo.mp3");
    // Mix_PlayMusic(state.bgm, -1);
    // Mix_VolumeMusic(0.0f);
    
    // state.jump_sfx = Mix_LoadWAV("img/bounce.wav");
}

void LevelA::update(float delta_time) { 
    this->state.player->update(delta_time, state.player, state.fire, state.enemies, this->ENEMY_COUNT, this->state.map);
    this->state.fire->update(delta_time, state.player, state.fire, state.enemies, this->ENEMY_COUNT, this->state.map);
    for (int i = 0; i < ENEMY_COUNT; i++) {
        this->state.enemies[i].update(delta_time, state.player, state.fire, state.enemies, this->ENEMY_COUNT, this->state.map);
    }

    if (this->state.fire->enemies_dead == this->ENEMY_COUNT) {
        std::cout << "finished\n";
        this->state.finished = true;
    }
    
}

void LevelA::render(ShaderProgram *program) {
    this->state.map->render(program);
    this->state.player->render(program);
    this->state.fire->render(program);

    for (int i = 0; i < ENEMY_COUNT; i++) {
        this->state.enemies[i].render(program);
    }
}
