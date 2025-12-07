#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include <glm/glm.hpp>

#include "Shader.h"
#include "Texture.h"

namespace Client {
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;

		static std::vector<struct Vertex> GenList(float* vertices, int noVertices);
	};
	typedef struct Vertex Vertex;

	class Mesh {
	private:
		unsigned int VBO, EBO;

		void Setup();
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		unsigned int VAO;

		std::vector<Texture> textures;

		Mesh();
		Mesh(std::vector<Vertex> vertiecs, std::vector<unsigned int> indices, std::vector<Texture> textures);

		void Render(Shader shader);

		void Cleanup();
	};
}
#endif // !MESH_H