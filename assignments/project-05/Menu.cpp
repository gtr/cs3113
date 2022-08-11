#include "Menu.h"
#include "Entity.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

Menu::~Menu() {
    delete[] this->state.enemies;
    delete this->state.player;
    delete this->state.map;
    delete this->state.fire;
}

unsigned int LEVEL_DATA0[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void Menu::initialise() {
    GLuint map_texture_id = Utility::load_texture("img/moon-brick.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA0, map_texture_id, 1.0f, 2, 1);

    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.player->texture_id = Utility::load_texture("img/scout.png");
    state.enemies = new Entity[this->ENEMY_COUNT];

    GLuint enemy_texture_id = Utility::load_texture("img/soph.png");
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(GUARD);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    this->txt = Utility::load_texture("img/font1.png");
    std::string title = "SCOUT V.S. ALIENS !";
    glm::vec3 title_pos = glm::vec3(1.5f, -3.0f, 0.0f);
}

void Menu::update(float delta_time) {

}

void Menu::render(ShaderProgram *program) {
    std::string title = "SCOUT V.S. ALIENS !";
    glm::vec3 title_pos = glm::vec3(1.5f, -3.0f, 0.0f);

    Utility::draw_text(program, this->txt, title, .4f, 0.0005f, title_pos);

}