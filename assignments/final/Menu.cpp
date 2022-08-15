#include "Menu.h"
#include "Entity.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

Menu::~Menu() {
    
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
    GLuint map_texture_id = Utility::load_texture("assets/moon-brick.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA0, map_texture_id, 1.0f, 2, 1);

    this->txt = Utility::load_texture("assets/font1.png");
    std::string title = "SCOUT V.S. ALIENS !";
    glm::vec3 title_pos = glm::vec3(1.5f, -3.0f, 0.0f);
}

void Menu::update(float delta_time) {

}

void Menu::render(ShaderProgram *program) {
    std::string title = "SCOUT V.S. ALIENS !";
    glm::vec3 title_pos = glm::vec3(1.5f, -3.0f, 0.0f);

    // Utility::draw_text(program, this->txt, title, .4f, 0.0005f, title_pos);

}