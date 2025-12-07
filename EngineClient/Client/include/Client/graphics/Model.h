#ifndef MODEL_H
#define MODEL_H

#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"

namespace Client {
	class Model {
	public:
		std::vector<Mesh> meshes;

		Model();

		void Init();
		void Render(Shader shader);
		void Cleanup();
	};
}
#endif // !MODEL_H