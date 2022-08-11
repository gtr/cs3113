#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include "ShaderProgram.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/mat4x4.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <vector>

class Utility {
public:
    static GLuint load_texture(const char* filepath);
    static void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position);
};
