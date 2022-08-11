#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include "Entity.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"
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

int levelll = 0;


/**
 CONSTANTS
 */
bool game_over = false;
const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480;

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

const char TEXT_FILEPATH[] = "img/font1.png";

const float MILLISECONDS_IN_SECOND = 1000.0;

/**
 VARIABLES
 */
Scene *current_scene;
LevelA *level_a;
LevelB *level_b;
LevelC *level_c;
Menu *menu;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

void switch_to_scene(Scene *scene) {
    current_scene = scene;
    current_scene->initialise();
}

void initialise() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Scout v.s. Aliens!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    view_matrix = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);

    level_a = new LevelA();
    menu = new Menu();
    switch_to_scene(menu);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

                    case SDLK_a:
                        current_scene->state.fire->set_position(
                            current_scene->state.player->get_position()
                        );
                        current_scene->state.fire->activate();
                        current_scene->state.fire->set_fire_state(ON_LEFT);
                        break;
                    case SDLK_d:
                        current_scene->state.fire->set_position(
                            current_scene->state.player->get_position()
                        );
                        current_scene->state.fire->activate();
                        current_scene->state.fire->set_fire_state(ON_RIGHT);
                        break;
                    case SDLK_RETURN:
                        switch_to_scene(level_a);

                    case SDLK_SPACE:
                      // Jump
                      if (current_scene->state.player->collided_bottom) {
                        current_scene->state.player->is_jumping = true;
                        // Mix_PlayChannel(-1, current_scene->state.jump_sfx,
                        // 0);
                      }
                      break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT]) {
        current_scene->state.player->movement.x = -1.0f;
        current_scene->state.player->animation_indices = current_scene->state.player->walking[current_scene->state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT]) {
        current_scene->state.player->movement.x = 1.0f;
        current_scene->state.player->animation_indices = current_scene->state.player->walking[current_scene->state.player->RIGHT];
    }
    
    if (glm::length(current_scene->state.player->movement) > 1.0f) {
        current_scene->state.player->movement = glm::normalize(current_scene->state.player->movement);
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
    
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    view_matrix = glm::mat4(1.0f);
    
    if (current_scene->state.player->get_position().x > LEVEL1_LEFT_EDGE) {
        view_matrix = glm::translate(view_matrix, glm::vec3(-current_scene->state.player->get_position().x, 3.75, 0));
    } else {
        view_matrix = glm::translate(view_matrix, glm::vec3(-5, 3.75, 0));
    }
}

void render() {
    program.SetViewMatrix(view_matrix);

    GLuint text_texture_id = Utility::load_texture(TEXT_FILEPATH);
    glClear(GL_COLOR_BUFFER_BIT);
    
    current_scene->render(&program);

    std::cout << current_scene->state.player->get_position().x << ", "
              << current_scene->state.player->get_position().y << "\n";
    float x = current_scene->state.player->get_position().x;
    float y = current_scene->state.player->get_position().y;



    std::string won_text = "YOU WON!";
    std::string lost_text = "YOU LOST!";
    glm::vec3 text_position_won = glm::vec3(x,-4.0f, 0.0f);
    glm::vec3 text_position_lost = glm::vec3(x, -4.0f, 0.0f);

    if (current_scene->state.finished == true) {
        if (current_scene->final) {
            Utility::draw_text(&program, text_texture_id, won_text, 0.4f, 0.0005f, text_position_won);
            game_over = false;
        } else {
            if (levelll == 0) {
                level_b = new LevelB();
                switch_to_scene(level_b);
                levelll++;
            } else if (levelll == 1) {
                level_c = new LevelC();
                switch_to_scene(level_c);
                levelll++;
            } else if (levelll > 1) {
                Utility::draw_text(&program, text_texture_id, won_text, 0.4f, 0.0005f, text_position_won);
                game_over = false;
            }
        }
    }

    if (current_scene->state.player->status == LOST) {
      Utility::draw_text(&program, text_texture_id, lost_text, .4f, 0.0005f,
                text_position_lost);
        game_over = true;
        std::cout << "LOST!\n" ;
    }

    SDL_GL_SwapWindow(display_window);
}

void shutdown() {
    SDL_Quit();
    
    delete menu;
    delete level_a;
    delete level_b;
    delete level_c;
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
