#include "LevelA.h"
#include "Utility.h"
#include "Entity.h"
#include "glm/ext/vector_float3.hpp"

#define LEVEL_WIDTH 36
#define LEVEL_HEIGHT 8

const char FIRE_FILEPATH[] = "assets/fire.png";
unsigned int LEVEL_DATA[] = {
    1, 4, 3, 9, 4, 1, 9, 8, 1, 9, 1, 3, 7, 5, 4, 5, 4, 6, 9, 7, 4, 3, 9, 4, 1, 2, 3, 5, 2, 3, 1, 4, 2, 3, 5, 4,
    6, 5, 9, 9, 9, 2, 5, 6, 9, 9, 6, 1, 6, 2, 9, 1, 2, 7, 9, 6, 3, 1, 9, 2, 6, 7, 5, 2, 3, 6, 8, 9, 7, 1, 6, 3,
    1, 2, 8, 4, 7, 9, 8, 9, 5, 2, 6, 4, 9, 9, 4, 7, 1, 3, 5, 6, 1, 5, 7, 1, 3, 4, 5, 7, 1, 9, 2, 3, 9, 5, 3, 1,
    5, 2, 9, 7, 8, 1, 3, 5, 8, 7, 9, 7, 6, 8, 1, 6, 2, 6, 8, 4, 5, 6, 2, 8, 9, 1, 6, 4, 7, 3, 7, 5, 4, 3, 7, 2,
    1, 5, 3, 4, 6, 7, 1, 4, 5, 6, 1, 2, 3, 5, 9, 1, 8, 3, 6, 7, 3, 9, 8, 6, 4, 7, 8, 9, 3, 6, 2, 1, 5, 4, 8, 1,
    2, 7, 4, 5, 2, 9, 8, 7, 6, 7, 4, 3, 4, 6, 7, 5, 7, 4, 1, 3, 9, 6, 1, 2, 9, 4, 3, 6, 9, 7, 8, 6, 2, 5, 6, 4,
    2, 3, 1, 9, 7, 3, 9, 3, 8, 5, 6, 1, 5, 2, 9, 6, 3, 9, 5, 4, 2, 1, 6, 5, 4, 6, 4, 3, 2, 6, 5, 2, 3, 2, 1, 4,
    6, 1, 2, 9, 2, 6, 5, 4, 6, 2, 5, 4, 7, 3, 2, 8, 6, 4, 7, 5, 6, 8, 7, 1, 3, 2, 8, 5, 8, 7, 3, 6, 2, 9, 4, 6,
};



LevelA::~LevelA() {
    delete []   this->state.enemies;
    delete      this->state.player;
    delete      this->state.map;
    delete      this->state.fire;
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
  state.player = new Entity();
  state.player->set_entity_type(PLAYER);
  state.player->set_position(glm::vec3(4.5f, -3.5f, 0.0f));
  state.player->set_movement(glm::vec3(0.0f));
  state.player->speed = 2.5f;

  state.player->texture_id = Utility::load_texture("assets/ship.png");

  state.player->set_height(0.6f);
  state.player->set_width(0.4f);
}

void LevelA::initialise() {
    initialize_map();
    initialize_coins();
    initialize_player();
}

void LevelA::update(float delta_time) { 
    this->state.player->update(delta_time, state.player, state.map);
    this->state.coins->update(delta_time, state.player, state.map);
    // this->state.map->update();
}

void LevelA::render(ShaderProgram *program) {
    this->state.map->render(program);
    this->state.player->render(program);
    this->state.coins->render(program);

}
