#include "glm/ext/vector_float3.hpp"
#include <cstdio>
#include <string>

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 24
#define GRAVITY -0.4F

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
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_opengl.h>
#include <ctime>
#include <math.h>
#include <time.h>
#include <vector>

struct Game {
    Entity* player;
    Entity* platforms;
};

// Window constants.
const int WINDOW_WIDTH  = 640, WINDOW_HEIGHT = 480;

// Color constants.
const float BG_RED = 0.01f, BG_GREEN = 0.01f, BG_BLUE = 0.05f,
            BG_OPACITY = 1.0f;

// Viewport constants.
const int VIEWPORT_X = 0, VIEWPORT_Y = 0, VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Texture shader paths.
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// Filepaths for sprites.
const char ROCKET_FILEPATH[]    = "img/ship.png";
const char PLATFORM_FILEPATH[]  = "img/moon-brick.png";
const char ROCK_FILEPATH[]      = "img/rock.png";
const char TARGET_FILEPATH[]    = "img/target.png";
const char TEXT_FILEPATH[]      = "img/font1.png";

// Some useful floats.
const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = -120.0f;

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL  = 0;
const GLint TEXTURE_BORDER   = 0;

Game state;

SDL_Window* display_window;
bool game_is_running = true;
bool game_over = false;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

const int FONTBANK_SIZE = 16;

float get_random(float left, float right) {
    float randomNumber = sin(rand() * rand());
    return left + (right - left) * fabs(randomNumber);
}

void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position) {
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
void initialise_platforms() {
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);
    GLuint rock_tecture_id = load_texture(ROCK_FILEPATH);
    GLuint target_texture_id = load_texture(TARGET_FILEPATH);

    int i = 0;
    const int VERTICAL_PLATFORM_COUNT = 7;
    const int HORIZONTAL_PLATFORM_COUNT_STOP = 19;

    state.platforms = new Entity[PLATFORM_COUNT];

    // Vertical platforms.
    for (; i < VERTICAL_PLATFORM_COUNT; i++) {
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3(4.8f, i - 3.1f, 0.0f));
        state.platforms[i].set_width(0.4f);
        state.platforms[i].update(0.0f, NULL, 0);
    }

    // Horizonal platforms.
    for (int j = 0; i < HORIZONTAL_PLATFORM_COUNT_STOP; i++, j++)  {
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3(j - 4.8f, -3.5f, 0.0f));
        state.platforms[i].set_width(0.4f);
        state.platforms[i].update(0.0f, NULL, 0);
    }



    // Random floating rocks >:) just to mess with the rocket.
    state.platforms[i].texture_id = rock_tecture_id;
    state.platforms[i].set_position(glm::vec3(get_random(-3.8f, 3.8f), 2.5f, 0.0f));
    state.platforms[i].set_width(0.4f);
    state.platforms[i].update(0.0f, NULL, 0);
    i++;
    state.platforms[i].texture_id = rock_tecture_id;
    state.platforms[i].set_position(glm::vec3(get_random(-3.8f, 3.8f), 1.5f, 0.0f));
    state.platforms[i].set_width(0.4f);
    state.platforms[i].update(0.0f, NULL, 0);
    i++;
    state.platforms[i].texture_id = rock_tecture_id;
    state.platforms[i].set_position(glm::vec3(get_random(-3.8f, 3.8f), -1.0f, 0.0f));
    state.platforms[i].set_width(0.4f);
    state.platforms[i].update(0.0f, NULL, 0);
    i++;
    state.platforms[i].texture_id = rock_tecture_id;
    state.platforms[i].set_position(glm::vec3(get_random(-3.8f, 3.8f), -2.0f, 0.0f));
    state.platforms[i].set_width(0.4f);
    state.platforms[i].update(0.0f, NULL, 0);
    i++;

    // Load the target platform.
    state.platforms[i].texture_id = target_texture_id;
    state.platforms[i].set_position(glm::vec3(get_random(-3.8f, 3.8f), -2.6f, 0.0f));
    state.platforms[i].set_height(0.1f);
    state.platforms[i].update(0.0f, NULL, 0);
}

/*
 * [x] Initializes the rocket.
 */
void initialise_rocket() {
    state.player = new Entity();
    state.player->set_position(glm::vec3(0.0f, 4.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 1.0f;
    state.player->set_acceleration(glm::vec3(0.0f, GRAVITY, 0.0f));
    state.player->texture_id = load_texture(ROCKET_FILEPATH);

    state.player->set_height(0.8f);
    state.player->set_width(0.8f);
}

/*
 * [x] Initializes the window, sprites, entities, etc.
 */
void initialise() {
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Lunar Lander :))",
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

    initialise_platforms();
    initialise_rocket();

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/*
 * [ ] Processes user input.
 */
void process_input() {
    state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        game_is_running = false;
                        break;
                    case SDLK_SPACE:
                    default:
                        break;
                }
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT]) {
        state.player->add_acceleration(glm::vec3(-3.0f, 0, 0));
    }
    else if (key_state[SDL_SCANCODE_RIGHT]) {
        state.player->add_acceleration(glm::vec3(3.0f, 0, 0));
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
        state.player->update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }
    
    accumulator = delta_time;
}

/*
 * [ ] Renders the player and the platform(s).
 */
void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    GLuint text_texture_id = load_texture(TEXT_FILEPATH);

    state.player->render(&program);
    
    for (int i = 0; i < PLATFORM_COUNT; i++) {
        state.platforms[i].render(&program);
    }

    std::string won_text = "MISSION ACCOMPLISHED";
    std::string lost_text = "MISSION FAILED";
    glm::vec3 text_position_won = glm::vec3(-3.8f, 1.5f, 0.0f);
    glm::vec3 text_position_lost = glm::vec3(-2.8f, 1.5f, 0.0f);
    if (state.player->status == WON) {
        draw_text(&program, text_texture_id, won_text, .4f, 0.0005f, text_position_won);
        game_over = true;
        LOG("MISSION ACCOMPLISHED");
    } else if (state.player->status == LOST) {
        draw_text(&program, text_texture_id, lost_text, .4f, 0.0005f, text_position_lost);
        LOG("MISSION FAILED");
        game_over = true;

    }
    SDL_GL_SwapWindow(display_window);
}

void shutdown() {
    SDL_Quit();
    
    delete [] state.platforms;
    delete state.player;
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
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
