#include "glm/ext/matrix_float4x4.hpp"
#include <SDL2/SDL_timer.h>
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include "ShaderProgram.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include "stb_image.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#define LOG(argument) std::cout << argument << '\n'

// Window constants.
const int WINDOW_WIDTH = 960, WINDOW_HEIGHT = 720;

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
const GLint LEVEL_OF_DETAIL =0;
const GLint TEXTURE_BORDER = 0;

// Filepaths for our sprites.
const char BALL_SPRITE_FILEPATH[] = "ball.png";
const char PLAYER_SPRITE_FILEPATH[] = "player.png";

GLuint ball_texture_id;   // Texture ID for the ball.
GLuint player_texture_id; // Texture ID for the player.

SDL_Window *display_window;
bool game_is_running = true;
bool is_growing = true;

// Program and matricies.
ShaderProgram program;
glm::mat4 view_matrix, ball_model_matrix, player_model_matrix, projection_matrix,
    trans_matrix;

// Ball movement floats.
float ball_x = -3.0f;
float ball_rotate = 0.0f;
float ball_previous_ticks = 0.0f;

// Player movement floats.
float player_x = -3.8f;
float player_previous_ticks = 0.0f;

// load_texture loads a texture given a filepath. 
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
  glGenTextures(NUMBER_OF_TEXTURES, &textureID);
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
  display_window = SDL_CreateWindow("When I say 'Football' I mean 'Soccer'", SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                    WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

  SDL_GLContext context = SDL_GL_CreateContext(display_window);
  SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
  glewInit();
#endif

  glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

  program.Load(V_SHADER_PATH, F_SHADER_PATH);

  ball_model_matrix = glm::mat4(1.0f);
  player_model_matrix = glm::mat4(1.0f);
  view_matrix = glm::mat4(1.0f);
  projection_matrix =
      glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f,
                 1.0f); // Defines the characteristics of your camera, such as
                        // clip planes, field of view, projection method etc.

  program.SetProjectionMatrix(projection_matrix);
  program.SetViewMatrix(view_matrix);

  glUseProgram(program.programID);

  glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);

  ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
  player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);

  // enable blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}











void process_input() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
      game_is_running = false;
    }
  }
}

// get_delta_time calculates delta time.
float get_delta_time() {
  float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
  float new_ticks = ticks - ball_previous_ticks;
  ball_previous_ticks = ticks;
  return new_ticks;
}

void update_ball_position(float delta_time) {
  ball_x += 1.0f * delta_time;
  ball_rotate += DEGREES_PER_SECOND * delta_time;
  ball_model_matrix = glm::mat4(1.0f);

  /* Translate -> Rotate */
  ball_model_matrix = glm::translate(ball_model_matrix, glm::vec3(ball_x, 0.0f, 0.0f));
  ball_model_matrix = glm::rotate(ball_model_matrix, glm::radians(ball_rotate),
                             glm::vec3(0.0f, 0.0f, 1.0f));
}

void update_player_position(float delta_time) {
  player_x += 1.0f * delta_time;
  player_model_matrix = glm::mat4(1.0f);

  player_model_matrix = glm::translate(player_model_matrix, glm::vec3(player_x, 0.0f, 0.0f));
}

void update() {
  float delta_time = get_delta_time();
  update_player_position(delta_time);
  update_ball_position(delta_time);
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id) {
  program.SetModelMatrix(object_model_matrix);
  glBindTexture(GL_TEXTURE_2D, object_texture_id);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
  glClear(GL_COLOR_BUFFER_BIT);

  // Vertices
  float vertices[] = {
      -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,  0.5f, // triangle 1
      -0.5f, -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f  // triangle 2
  };

  // Textures
  float texture_coordinates[] = {
      0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // triangle 1
      0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, // triangle 2
  };

  glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0,
                        vertices);
  glEnableVertexAttribArray(program.positionAttribute);

  glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0,
                        texture_coordinates);
  glEnableVertexAttribArray(program.texCoordAttribute);

  // Bind ball texture.
  draw_object(ball_model_matrix, ball_texture_id);

  // Bind player texture.
  draw_object(player_model_matrix, player_texture_id);

  // We disable two attribute arrays now
  glDisableVertexAttribArray(program.positionAttribute);
  glDisableVertexAttribArray(program.texCoordAttribute);

  SDL_GL_SwapWindow(display_window);
}

void shutdown() { SDL_Quit(); }

/**
 Start here—we can see the general structure of a game loop without worrying too
 much about the details yet.
 */
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
