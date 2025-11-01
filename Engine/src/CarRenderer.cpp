#include "graphics/models/CarRenderer.h"

CarRenderer::CarRenderer() {
	transform = Quad(DYNAMIC);
	transform.init();
	assetImage = NULL;
}

CarRenderer::CarRenderer(const char* imageName) {
	transform = Quad({ imageName }, DYNAMIC);
	transform.init();
	assetImage = imageName;
}

CarRenderer::CarRenderer(glm::vec3 pos, const char* imageName) {
	transform = Quad({ imageName }, DYNAMIC);
	transform.init();
	transform.pos = pos;
	assetImage = imageName;
}

//glm::vec3 Car::getForwardDirection() {
//	float angleRad = glm::radians(currentAngle - 90.0f);
//	return glm::normalize(glm::vec3(glm::sin(angleRad), glm::cos(angleRad), 0.0f));
//}

void CarRenderer::render(Shader shader) {
	transform.render(shader);
}