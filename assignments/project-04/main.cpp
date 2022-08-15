#include <SDL2/SDL_assert.h>
#include <SDL2/SDL_keycode.h>
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 16
#define ENEMY_COUNT 3

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include "Entity.h"
#include "ShaderProgram.h"
#include "cmath"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include "stb_image.h"
#include <SDL2/SDL.h>
#include <ctime>
#include <vector>

struct Game {
    Entity *player;
    Entity *platforms;
    Entity *enemies;
    Entity *fire;
};

const int FONTBANK_SIZE = 16;
bool game_over = false;

void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text,
               float screen_size, float spacing, glm::vec3 position) {
  // Scale the size of the fontbank in the UV-plane
  // We will use this for spacing and positioning
  float width = 1.0f / FONTBANK_SIZE;
  float height = 1.0f / FONTBANK_SIZE;

  // Instead of having a single pair of arrays, we'll have a series of pairs—one
  // for each character Don't forget to include <vector>!
  std::vector<float> vertices;
  std::vector<float> texture_coordinates;

  // For every character...
  for (int i = 0; i < text.size(); i++) {
    // 1. Get their index in the spritesheet, as well as their offset (i.e.
    // their position
    //    relative to the whole sentence)
    int spritesheet_index = (int)text[i]; // ascii value of character
    float offset = (screen_size + spacing) * i;

    // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
    float u_coordinate =
        (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
    float v_coordinate =
        (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

    // 3. Inset the current pair in both vectors
    vertices.insert(vertices.end(), {
                                        offset + (-0.5f * screen_size),
                                        0.5f * screen_size,
                                        offset + (-0.5f * screen_size),
                                        -0.5f * screen_size,
                                        offset + (0.5f * screen_size),
                                        0.5f * screen_size,
                                        offset + (0.5f * screen_size),
                                        -0.5f * screen_size,
                                        offset + (0.5f * screen_size),
                                        0.5f * screen_size,
                                        offset + (-0.5f * screen_size),
                                        -0.5f * screen_size,
                                    });

    texture_coordinates.insert(texture_coordinates.end(),
                               {
                                   u_coordinate,
                                   v_coordinate,
                                   u_coordinate,
                                   v_coordinate + height,
                                   u_coordinate + width,
                                   v_coordinate,
                                   u_coordinate + width,
                                   v_coordinate + height,
                                   u_coordinate + width,
                                   v_coordinate,
                                   u_coordinate,
                                   v_coordinate + height,
                               });
  }

  // 4. And render all of them using the pairs
  glm::mat4 model_matrix = glm::mat4(1.0f);
  model_matrix = glm::translate(model_matrix, position);

  program->SetModelMatrix(model_matrix);
  glUseProgram(program->programID);

  glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0,
                        vertices.data());
  glEnableVertexAttribArray(program->positionAttribute);
  glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0,
                        texture_coordinates.data());
  glEnableVertexAttribArray(program->texCoordAttribute);

  glBindTexture(GL_TEXTURE_2D, font_texture_id);
  glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

  glDisableVertexAttribArray(program->positionAttribute);
  glDisableVertexAttribArray(program->texCoordAttribute);
}

// Window constants.
const int WINDOW_WIDTH = 640, WINDOW_HEIGHT = 480;

// Color constants.
const float BG_RED = 0.01f, BG_GREEN = 0.02f, BG_BLUE = 0.05f,
            BG_OPACITY = 1.0f;

// Viewport constants.
const int VIEWPORT_X = 0, VIEWPORT_Y = 0, VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Texture shader paths.
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND      = 1000.0;
const char SPRITESHEET_FILEPATH[]       = "img/scout.png";
const char PLATFORM_FILEPATH[]          = "img/moon-brick.png";
const char FLOAT_PLATFORM_FILEPATH[]    = "img/float-platform.png";
const char FIRE_FILEPATH[]              = "img/fire.png";
const char TEXT_FILEPATH[]              = "img/font1.png";

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL  = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER   = 0;   // this value MUST be zero

const float PLATFORM_OFFSET = 5.0f;

/* ===== VARIABLES ===== */

Game state;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

/* ===== GENERAL FUNCTIONS ===== */

/*
 * [x] loads a texture.
 */
GLuint load_texture(const char* filepath) {
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL) {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Setting our texture wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // the last argument can change depending on what you are looking for
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // STEP 5: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

/*
 * [x] Initializes the platforms.
 */
void initialize_platforms() {
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    GLuint platform_texture_id_2 = load_texture(FLOAT_PLATFORM_FILEPATH);

    state.platforms = new Entity[PLATFORM_COUNT];

    for (int i = 0; i < 11; i++) {
        state.platforms[i].set_entity_type(PLATFORM);
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3(i - PLATFORM_OFFSET, -3.0f, 0.0f));
        state.platforms[i].set_width(0.4f);
        state.platforms[i].update(0.0f, NULL, NULL, NULL, 0);
    }

    for (int j = 11; j < 14; j++) {
        state.platforms[j].set_entity_type(PLATFORM);
        state.platforms[j].texture_id = platform_texture_id;
        state.platforms[j].set_position(glm::vec3(j - 4.0 - PLATFORM_OFFSET, -0.4f, 0.0f));
        state.platforms[j].set_width(0.4f);
        state.platforms[j].update(0.0f, NULL, NULL, NULL, 0);
    }

    state.platforms[PLATFORM_COUNT - 2].set_entity_type(PLATFORM);
    state.platforms[PLATFORM_COUNT - 2].texture_id = platform_texture_id_2;
    state.platforms[PLATFORM_COUNT - 2].set_position(glm::vec3(-1.1f, -1.8f, 0.0f));
    state.platforms[PLATFORM_COUNT - 2].set_width(0.4f);
    state.platforms[PLATFORM_COUNT - 2].set_height(0.4f);
    state.platforms[PLATFORM_COUNT - 2].update(0.0f, NULL, NULL, NULL, 0);

    state.platforms[PLATFORM_COUNT - 1].set_entity_type(PLATFORM);
    state.platforms[PLATFORM_COUNT - 1].texture_id = platform_texture_id_2;
    state.platforms[PLATFORM_COUNT - 1].set_position(glm::vec3(-0.1f, -0.8f, 0.0f));
    state.platforms[PLATFORM_COUNT - 1].set_width(0.4f);
    state.platforms[PLATFORM_COUNT - 1].set_height(0.4f);
    state.platforms[PLATFORM_COUNT - 1].update(0.0f, NULL, NULL, NULL, 0);
}

void initialize_fire() {
    GLuint fire_tecture_id = load_texture(FIRE_FILEPATH);
    state.fire = new Entity();

    state.fire->set_entity_type(FIRE);
    state.fire->set_ai_type(SHOOT);
    state.fire->set_fire_state(OFF);
    state.fire->set_width(0.04f);
    state.fire->set_height(0.04f);
    state.fire->set_movement(glm::vec3(0.0f));
    state.fire->speed = 1.0f;
    state.fire->texture_id = fire_tecture_id;
    state.fire->deactivate();
}

/*
 * [x] Initializes the player.
 */
void initialize_player() {
    // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(-2.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -8.0f, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);

    // Walking
    state.player->walking[state.player->LEFT] = new int[4]{1, 5, 9, 13};
    state.player->walking[state.player->RIGHT] = new int[4]{3, 7, 11, 15};
    state.player->walking[state.player->UP] = new int[4]{2, 6, 10, 14};
    state.player->walking[state.player->DOWN] = new int[4]{0, 4, 8, 12};

    state.player->animation_indices = state.player->walking[state.player->RIGHT];
    // state.player->animation_frames = 4;
    // state.player->animation_index = 0;
    // state.player->animation_time = 0.0f;
    // state.player->animation_cols = 4;
    // state.player->animation_rows = 4;
    state.player->set_height(0.8f);
    state.player->set_width(0.6f);

    // Jumping
    state.player->jumping_power = 5.0f;
}

/*
 * [x] Initializes enemy A.
 */
void initialize_enemy_A() {
    GLuint enemy_texture_id = load_texture("img/alienA.png");

    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(GUARD);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(3.0f, 0.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -7.0f, 0.0f));
}

/*
 * [x] Initializes enemy B.
 */
void initialize_enemy_B() {
    GLuint enemy_texture_id = load_texture("img/alienB.png");

    state.enemies[1].set_entity_type(ENEMY);
    state.enemies[1].set_ai_type(WALKER);
    state.enemies[1].set_ai_state(WALK_LEFT);
    state.enemies[1].texture_id = enemy_texture_id;
    state.enemies[1].set_position(glm::vec3(5.0f, 1.0f, 0.0f));
    state.enemies[1].set_movement(glm::vec3(0.0f));
    state.enemies[1].speed = 1.0f;
    state.enemies[1].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
}

/*
 * [x] Initializes enemy B.
 */
void initialize_enemy_C() {
    GLuint enemy_texture_id = load_texture("img/alienC.png");

    state.enemies[2].set_entity_type(ENEMY);
    state.enemies[2].set_ai_type(FLY);
    state.enemies[2].set_ai_state(FLY_UP);
    state.enemies[2].texture_id = enemy_texture_id;
    state.enemies[2].set_position(glm::vec3(-4.5f, 1.0f, 0.0f));
    state.enemies[2].set_movement(glm::vec3(0.0f, 1.0f, 0.0f));
    state.enemies[2].speed = 1.0f;
}

/*
 * [x] Initializes the enemies.
 */
void initialize_enemies() {
    state.enemies = new Entity[ENEMY_COUNT];
    initialize_enemy_A();
    initialize_enemy_B();
    initialize_enemy_C();
}

/*
 * [ ] Initializes the window, sprites, entities, etc.
 */
void initialise() {
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Rise of the AI :O", 
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                      WINDOW_WIDTH, WINDOW_HEIGHT, 
                                      SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    view_matrix = glm::mat4(1.0f);
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.Load(V_SHADER_PATH, F_SHADER_PATH);
    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);
    
    glUseProgram(program.programID);
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // Initializers
    initialize_platforms();
    initialize_player();
    initialize_fire();
    initialize_enemies();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/*
 * [x] Processes user input.
 */
void process_input() {
    state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false; break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        game_is_running = false; break;

                    case SDLK_a:
                        state.fire->set_position(state.player->get_position());
                        state.fire->activate();
                        state.fire->set_fire_state(ON_LEFT);
                        break;
                    case SDLK_d:
                        state.fire->set_position(state.player->get_position());
                        state.fire->activate();
                        state.fire->set_fire_state(ON_RIGHT);
                        break;
                    case SDLK_SPACE:
                        if (state.player->collided_bottom) state.player->is_jumping = true;
                        break;
                        
                    default: break;
                }
                
            default: break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT]) {
        state.player->movement.x = -1.0f;
        state.player->animation_indices = state.player->walking[state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT]) {
        state.player->movement.x = 1.0f;
        state.player->animation_indices = state.player->walking[state.player->RIGHT];
    }

    // Normalize.
    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

/*
 * [x] Calculates delta time.
 */
float get_delta_time() {
  float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
  float new_ticks = ticks - previous_ticks;
  previous_ticks = ticks;
  return new_ticks;
}

/*
 * [x] Updates the game.
 */
void update() {
    float delta_time = get_delta_time();
    delta_time += accumulator;

    if (delta_time < FIXED_TIMESTEP) {
        accumulator = delta_time;
        return;
    }


    
    while (delta_time >= FIXED_TIMESTEP) {
        state.player->update(FIXED_TIMESTEP, state.player, NULL, state.platforms,
                            PLATFORM_COUNT);

        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].update(FIXED_TIMESTEP, state.player, state.fire, state.platforms, PLATFORM_COUNT);
        }   
            
        delta_time -= FIXED_TIMESTEP;
    }

    state.fire->update(FIXED_TIMESTEP, state.player, NULL, state.platforms, PLATFORM_COUNT);
    
    accumulator = delta_time;
}

/*
 * [ ] Renders the player and the platform(s).
 */
void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    GLuint text_texture_id = load_texture(TEXT_FILEPATH);

    state.player->render(&program);
    state.fire->render(&program);
    
    for (int i = 0; i < PLATFORM_COUNT; i++) state.platforms[i].render(&program);
    for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].render(&program);

    std::string won_text = "YOU WON!";
    std::string lost_text = "YOU LOST!";
    glm::vec3 text_position_won = glm::vec3(-3.8f, 1.5f, 0.0f);
    glm::vec3 text_position_lost = glm::vec3(-2.8f, 1.5f, 0.0f);

    if (state.fire->enemy_count == 0) {
      draw_text(&program, text_texture_id, won_text, .4f, 0.0005f,
                text_position_won);
      game_over = true;
    } 
    if (state.player->status == LOST) {
      draw_text(&program, text_texture_id, lost_text, .4f, 0.0005f,
                text_position_lost);
      game_over = true;
    }

    SDL_GL_SwapWindow(display_window);
}

/*
 * [x] Shutdown function.
 */
void shutdown() {
    SDL_Quit();

    delete [] state.platforms;
    delete [] state.enemies;
    delete    state.player;
}

/*
 * [x] Main function.
 */
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
