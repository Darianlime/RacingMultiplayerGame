#ifndef CAR_RENDER_H
#define CAR_RENDER_H

#include <vector>

#include <glm/glm.hpp>

#include "Quad.hpp"

class CarRenderer {
private:
	// car model
	Quad transform;
public:
	CarRenderer();
	CarRenderer(const char* imageName);
	CarRenderer(glm::vec3 pos, const char* imageName);

	Quad& getTransform() { return transform; }
	void render(Shader shader);
};

#endif // !CAR_H