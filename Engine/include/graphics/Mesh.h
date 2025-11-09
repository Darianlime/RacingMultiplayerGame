#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include <glm/glm.hpp>

#include "Shader.h"
#include "Texture.h"

namespace Engine {
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;

		static std::vector<struct Vertex> genList(float* vertices, int noVertices);
	};
	typedef struct Vertex Vertex;

	class Mesh {
	private:
		unsigned int VBO, EBO;

		void setup();
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		unsigned int VAO;

		std::vector<Texture> textures;

		Mesh();
		Mesh(std::vector<Vertex> vertiecs, std::vector<unsigned int> indices, std::vector<Texture> textures);

		void render(Shader shader);

		void cleanup();
	};
}
#endif // !MESH_H