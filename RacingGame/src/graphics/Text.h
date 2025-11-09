#ifndef TEXT_H
#define TEXT_H
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <map>
#include <glm/glm.hpp>

#include "Texture.h"
#include "Shader.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
	unsigned int TextureID; // ID handle of the glyph texture
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
} Character;


class Text {
private:
	unsigned int VAO, VBO;
	FT_Library ft;
	FT_Face face;
	const char* fontFile;
	int fontSize;
	std::map<GLchar, Character> Characters;
public:
	Text(const char* fontFile, int fontSize);

	int init();
	void renderText(Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);

};

#endif // !TEXT_H