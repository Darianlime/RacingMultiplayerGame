#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"

namespace Engine {
	class Model {
	public:
		std::vector<Mesh> meshes;

		Model();

		void init();

		void render(Shader shader);

		void cleanup();
	};
}
#endif // !MODEL_H