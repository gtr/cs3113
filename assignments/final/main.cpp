
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LEVEL1_RIGHT_EDGE 25.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include "Entity.h"
#include "LevelA.h"
#include "Menu.h"
#include "Map.h"
#include "Scene.h"
#include "ShaderProgram.h"
#include "Utility.h"
#include "cmath"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_opengl.h>
#include <ctime>
#include <vector>

int lives = 3;

enum LevelStatus {
    MENU,
    LEVEL_A,
    LEVEL_B,
    LEVEL_C,
    CREDITS,
};

/**
 CONSTANTS
 */
bool game_over = false;
const int WINDOW_WIDTH  = 1280,
          WINDOW_HEIGHT = 960;

const float BG_RED     = 0.06f,
            BG_BLUE    = 0.04f,
            BG_GREEN   = 0.01f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

GLuint font_text;

const float MILLISECONDS_IN_SECOND = 1000.0;

/**
 VARIABLES
 */
Scene *current_scene;
LevelA *level_a;
Menu *menu;
LevelStatus level_status;

SDL_Window *display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

void initialize_level() {
    level_a = new LevelA();

    level_status = LEVEL_A;
    current_scene = level_a;
    current_scene->initialise();
}

void switch_to_level_b() {
    // level_b = new Level
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Scout v.s. Aliens!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    view_matrix = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    glUseProgram(program.programID);
    font_text = Utility::load_texture("assets/font1.png");

    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);

    initialize_level();

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void print_location() {
    std::cout << current_scene->state.player->get_position().x << ", " 
    << current_scene->state.player->get_position().y << "\n";
}

void process_input() {
    current_scene->state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        game_is_running = false;
                        break;
                    case SDLK_t:
                        print_location();

                       
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT]) {
        current_scene->state.player->move_left();
    } else if (key_state[SDL_SCANCODE_RIGHT]) {
        current_scene->state.player->move_right();
    } else if (key_state[SDL_SCANCODE_UP]) {
        current_scene->state.player->move_up();
    } else if (key_state[SDL_SCANCODE_DOWN]) {
        current_scene->state.player->move_down();
    }

    if (glm::length(current_scene->state.player->movement) > 1.0f) {
        current_scene->state.player->movement = glm::normalize(current_scene->state.player->movement);
    }
}

void update_levelA_camera() {
    float x = current_scene->state.player->get_position().x;
    float y = current_scene->state.player->get_position().y;

    if (x <= 5) {
        view_matrix = glm::translate(view_matrix, glm::vec3(-5, 3.25, 0));
    } else if (x >= 30) {
        view_matrix = glm::translate(view_matrix, glm::vec3(-30, 3.25, 0));
    } else {
        view_matrix = glm::translate(view_matrix, glm::vec3(-x, 3.25, 0));
    }
}

void update() {
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;
    
    delta_time += accumulator;
    
    if (delta_time < FIXED_TIMESTEP) {
        accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        current_scene->update(FIXED_TIMESTEP);
        delta_time -= FIXED_TIMESTEP;
    }
    
    accumulator = delta_time;
    view_matrix = glm::mat4(1.0f);

    switch (level_status) {
        case LEVEL_A:
            update_levelA_camera();
            break;
    }

    // if (current_scene->state.player->get_position().x > LEVEL1_LEFT_EDGE) {
    //     view_matrix = glm::translate(view_matrix, glm::vec3(-current_scene->state.player->get_position().x, 2.75, 0));
    // } else if (current_scene->state.player->get_position().x > LEVEL1_RIGHT_EDGE) {
    //     view_matrix = glm::translate(view_matrix, glm::vec3(-10, 2.75, 0));
    // } else {
    //     view_matrix = glm::translate(view_matrix, glm::vec3(-5, 3.75, 0));
    // }
}

void show_enemy_text() {

}

void show_player_lives() {
    std::string lives = "Lives: 3/3";
    glm::vec3 title_pos = glm::vec3(-1.0f, -1.0f, 0.0f);

    // Utility::draw_text(&program, font_text, view_matrix, lives, .4f, 0.0005f,
    //                    title_pos);
}


void render() {
    program.SetViewMatrix(view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);

    current_scene->render(&program);
    
    show_player_lives();

    float x = current_scene->state.player->get_position().x;
    float y = current_scene->state.player->get_position().y;



    SDL_GL_SwapWindow(display_window);
}

void shutdown() {
    SDL_Quit();
    
    delete menu;
    delete level_a;
}

int main(int argc, char* argv[]) {
    initialise();
    
    while (game_is_running) {
        process_input();
        update();
        render();
        if (game_over) {
             while (game_is_running) {
                process_input();
            }
        }
    }
    
    shutdown();
    return 0;
}
