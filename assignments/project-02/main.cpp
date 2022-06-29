
#include "glm/ext/matrix_float4x4.hpp"
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include "ShaderProgram.h"
#include "cmath"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include "stb_image.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_opengl.h>
#include <ctime>

// Window constants.
const int WINDOW_WIDTH = 640, WINDOW_HEIGHT = 480;

// Color constants.
const float BG_RED = 0.10f, BG_GREEN = 0.6059f, BG_BLUE = 0.10f,
            BG_OPACITY = 1.0f;

// Viewport constants.
const int VIEWPORT_X = 0, VIEWPORT_Y = 0, VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Texture shader paths.
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

// Some useful floats.
const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = -120.0f;

// Constants for textures.
const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

const float MINIMUM_COLLISION_DISTANCE = 0.8f;

SDL_Window *display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, player_one_view_matrix, player_two_view_matrix,
    ball_view_matrix, projection_matrix, trans_matrix;


float previous_ticks = 0.0f;

GLuint player_one_texture_id;
GLuint player_two_texture_id;
GLuint ball_texture_id;

// Filepaths for sprites.
const char PLAYER_ONE_FILEPATH[] = "maradona.png";
const char PLAYER_TWO_FILEPATH[] = "pele.png";
const char BALL_FILEPATH[] = "ball.png";

// Player one position and movement, It starts on the left.
glm::vec3 player_one_position = glm::vec3(-4.0f, 0.0f, 0.0f);
glm::vec3 player_one_movement = glm::vec3(0.0f, 0.0f, 0.0f);

// Player two position and movement, It starts on the right.
glm::vec3 player_two_position = glm::vec3(4.f, 0.0f, 0.0f);
glm::vec3 player_two_movement = glm::vec3(0.0f, 0.0f, 0.0f);

// Ball position and movement, it starts at the center.
glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 player_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

// Speeds
float ball_speed = 5.0f;
float player_speed = 4.0f;

// Walls
float bottom_wall   = -4.0f;
float left_wall     = -4.4f;
float top_wall      = 4.0f;
float right_wall    = 4.4f;

#define LOG(argument) std::cout << argument << '\n'

GLuint load_texture(const char *filepath) {
  // STEP 1: Loading the image file
  int width, height, number_of_components;
  unsigned char *image = stbi_load(filepath, &width, &height,
                                   &number_of_components, STBI_rgb_alpha);

  if (image == NULL) {
    LOG("Unable to load image. Make sure the path is correct.");
    assert(false);
  }

  // STEP 2: Generating and binding a texture ID to our image
  GLuint textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height,
               TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

  // STEP 3: Setting our texture filter parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // STEP 4: Releasing our file from memory and returning our texture id
  stbi_image_free(image);

  return textureID;
}

void initialise() {
  SDL_Init(SDL_INIT_VIDEO);
  display_window = SDL_CreateWindow("Football Pong!", SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                    WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

  SDL_GLContext context = SDL_GL_CreateContext(display_window);
  SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
  glewInit();
#endif

  glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

  program.Load(V_SHADER_PATH, F_SHADER_PATH);

  view_matrix = glm::mat4(1.0f);

  player_one_view_matrix = glm::mat4(1.0f);
  player_two_view_matrix = glm::mat4(1.0f);
  ball_view_matrix = glm::mat4(1.0f);
  projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);


  player_one_texture_id = load_texture(PLAYER_ONE_FILEPATH);
  player_two_texture_id = load_texture(PLAYER_TWO_FILEPATH);
  ball_texture_id = load_texture(BALL_FILEPATH);

  program.SetProjectionMatrix(projection_matrix);
  program.SetViewMatrix(view_matrix);
  

  glUseProgram(program.programID);
  
  glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
  
  SDL_Event event;

  
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            // Hit the spacebar to re-start the game.
            case SDLK_SPACE:
                ball_position.x = 0.0f;
                ball_position.y = 0.0f;
                ball_movement.x = 0.85f;
                ball_movement.y = 0.55f;
                break;
            case SDLK_q:
                game_is_running = false;
                break;

            default:
                break;
            }
        default:
            break;
        }
    }

    const Uint8 *key_states = SDL_GetKeyboardState(NULL);

    // Letting the "W" / "S" key go, controls player one's movement.
    if (key_states[SDL_SCANCODE_W]) {
        player_one_movement.y = 1.0f;
    } else if (key_states[SDL_SCANCODE_S]) {
        player_one_movement.y = -1.0f;
    }

    // Letting the "^" / "v" key go, controls player two's movement.
    if (key_states[SDL_SCANCODE_UP]) {
        player_two_movement.y = 1.0f;
    } else if (key_states[SDL_SCANCODE_DOWN]) {
        player_two_movement.y = -1.0f;
    }

    // Normalize.
    if (glm::length(player_one_movement) > 1.0f) {
        player_one_movement = glm::normalize(player_one_movement);
    }
    if (glm::length(player_two_movement) > 1.0f) {
        player_two_movement = glm::normalize(player_two_movement);
    }
}


void check_for_win() {
    // Stop the ball if someone wins.
    if (ball_position.x > right_wall || ball_position.x < left_wall) {
        ball_movement.x = 0;
        ball_movement.y = 0;
    }
}


bool check_collision(glm::vec3 &position_a, glm::vec3 &position_b) {
  return sqrt(pow(position_b[0] - position_a[0], 2) +
              pow(position_b[1] - position_a[1], 2)) <
         MINIMUM_COLLISION_DISTANCE;
}

// get_delta_time calculates delta time.
float get_delta_time() {
  float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
  float new_ticks = ticks - previous_ticks;
  previous_ticks = ticks;
  return new_ticks;
}


void check_for_player_out_of_bounds() {
    if (player_one_position.y > top_wall || player_one_position.y < bottom_wall) {
        player_one_movement.y = -1 * player_one_movement.y;
    }

    if (player_two_position.y > top_wall || player_two_position.y < bottom_wall) {
        player_two_movement.y = -1 * player_two_movement.y;
    }
}

void check_for_ball_out_of_bounds() {
    if (ball_position.y >  top_wall || ball_position.y < bottom_wall) {
        ball_movement.y = -1 * ball_movement.y;
    }
}

void update_player_one(float delta_time) {
    player_one_position += player_one_movement * player_speed * delta_time;

    player_one_view_matrix = glm::mat4(1.0f);
    player_one_view_matrix =
        glm::translate(player_one_view_matrix, player_one_position);
    player_one_movement.y = 0.0f;
}

void update_player_two(float delta_time) {
    player_two_position += player_two_movement * player_speed * delta_time;

    player_two_view_matrix = glm::mat4(1.0f);
    player_two_view_matrix =
        glm::translate(player_two_view_matrix, player_two_position);
    player_two_movement.y = 0.0f;
}

void update_ball(float delta_time) {
    ball_position += ball_movement * ball_speed * delta_time;

    ball_view_matrix = glm::mat4(1.0f);
    ball_view_matrix = glm::translate(ball_view_matrix, ball_position);
}

void update() {
    float delta_time = get_delta_time();

    check_for_win();

    // Check collision between the players and the ball.
    if (check_collision(player_one_position, ball_position) ||
        check_collision(player_two_position, ball_position)) {
      ball_movement.x = -1 * ball_movement.x;
    }

    check_for_ball_out_of_bounds();
    check_for_player_out_of_bounds();

    update_player_one(delta_time);
    update_player_two(delta_time);
    update_ball(delta_time);
}









void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id) {
  program.SetModelMatrix(object_model_matrix);
  glBindTexture(GL_TEXTURE_2D, object_texture_id);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_player_one() {
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    program.SetModelMatrix(player_one_view_matrix);

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,  0.5f, // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f  // triangle 2
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);


    glBindTexture(GL_TEXTURE_2D, player_one_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_player_two() {
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    program.SetModelMatrix(player_two_view_matrix);

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,  0.5f, // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f  // triangle 2
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0,texture_coordinates);

    glBindTexture(GL_TEXTURE_2D, player_two_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render_ball() {
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    program.SetModelMatrix(ball_view_matrix);

    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,  0.5f, // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f  // triangle 2
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0,vertices);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);

    glBindTexture(GL_TEXTURE_2D, ball_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    render_player_one();
    render_player_two();
    render_ball();

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }

int main(int argc, char *argv[]) {
  initialise();

  while (game_is_running) {
    process_input();
    update();
    render();
  }

  shutdown();
  return 0;
}
