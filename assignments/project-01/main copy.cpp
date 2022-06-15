#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define STB_IMAGE_IMPLEMENTATION

#define GL_GLEXT_PROTOTYPES 1
#include "ShaderProgram.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include <SDL2/SDL.h>
#include "stb_image.h"
#include <SDL2/SDL_opengl.h>

#define LOG(argument) std::cout << argument << '\n'

// Path for our player sprite.
const char PLAYER_SPRITE[] = "man.png";

GLuint player_texture_id;

const int WINDOW_WIDTH = 800, WINDOW_HEIGHT = 500;

const float BG_RED = 0.1922f, BG_BLUE = 0.549f, BG_GREEN = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0, VIEWPORT_Y = 0, VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

// Shader files for textures.
const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const int TRIANGLE_RED = 1.0, TRIANGLE_BLUE = 0.4, TRIANGLE_GREEN = 0.4,
          TRIANGLE_OPACITY = 1.0;

const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;

SDL_Window *display_window;
bool game_is_running = true;
bool is_growing = true;

ShaderProgram program;
glm::mat4 view_matrix, model_matrix, projection_matrix, trans_matrix;

float triangle_x = 0.0f;
float triangle_rotate = 0.0f;
float previous_ticks = 0.0f;

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

GLuint load_texture(const char *filepath) {
  // STEP 1: Loading the image file
  int width, height, number_of_components;
  unsigned char *image = stbi_load(filepath, &width, &height,
                                   &number_of_components, STBI_rgb_alpha);

  if (image == NULL) {
    LOG("image == NULL");
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
  display_window = SDL_CreateWindow("Triangle!", SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                                    WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

  SDL_GLContext context = SDL_GL_CreateContext(display_window);
  SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
  glewInit();
#endif

  glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

  program.Load(V_SHADER_PATH, F_SHADER_PATH);

  // Defines the position (location and orientation) of the camera
  view_matrix = glm::mat4(1.0f);

  // Defines the characteristics of your camera, such as clip planes, field of
  // view, projection method etc.
  projection_matrix =glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
  trans_matrix = model_matrix;

  program.SetProjectionMatrix(projection_matrix);
  program.SetViewMatrix(view_matrix);

  program.SetColor(TRIANGLE_RED, TRIANGLE_BLUE, TRIANGLE_GREEN,
                   TRIANGLE_OPACITY);

  glUseProgram(program.programID);

  glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

  // Load the shaders for handling textures
  program.Load(V_SHADER_PATH, F_SHADER_PATH);

  // Load our player image
  player_texture_id = load_texture(PLAYER_SPRITE);

  // Enable blending.
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

void update() {
  // Get the current number of ticks.
  float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;

  // The delta time is the difference from the last frame.
  float delta_time = ticks -previous_ticks;
  previous_ticks = ticks;

  triangle_x += 1.0f * delta_time;
  // 90-degrees per second
  triangle_rotate += DEGREES_PER_SECOND * delta_time;
  model_matrix = glm::mat4(1.0f);

  /* Translate -> Rotate */
  model_matrix =
      glm::translate(model_matrix, glm::vec3(triangle_x, 0.0f, 0.0f));

}

void render() {
  glClear(GL_COLOR_BUFFER_BIT);

  program.SetModelMatrix(model_matrix);

  // Vertices
  float vertices[] = {
      -0.5f, -0.5f, 0.5f, -0.5f, 0.5f,  0.5f, // triangle 1
      -0.5f, -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f  // triangle 2
  };

  glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0,
                        vertices);
  glEnableVertexAttribArray(program.positionAttribute);

  // Textures
  float texture_coordinates[] = {
      0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, // triangle 1
      0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, // triangle 2
  };

  glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0,
                        texture_coordinates);
  glEnableVertexAttribArray(program.texCoordAttribute);

  // Bind texture
  glBindTexture(GL_TEXTURE_2D, player_texture_id);
  glDrawArrays(GL_TRIANGLES, 0,
               6); // we are now drawing 2 triangles, so we use 6 instead of 3

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
