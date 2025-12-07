#include "pch.h"
#include "graphics/models/CarRenderer.h"

namespace Client {
	CarRenderer::CarRenderer() {
		transform = QuadRender(DYNAMIC);
		transform.InitQuads();
		transform.GetQuad().size = transform.GetTextureSize();
		currentAngle = 0.0f;
		assetImage = NULL;
	}

	CarRenderer::CarRenderer(const char* imageName) {
		transform = QuadRender({ imageName }, DYNAMIC);
		transform.InitQuads();
		transform.GetQuad().size = transform.GetTextureSize();
		currentAngle = 0.0f;
		assetImage = imageName;
	}

	CarRenderer::CarRenderer(glm::vec3 pos, const char* imageName) {
		transform = QuadRender({ imageName }, DYNAMIC);
		transform.InitQuads();
		transform.GetQuad().pos = pos;
		transform.GetQuad().size = transform.GetTextureSize();
		currentAngle = 0.0f;
		assetImage = imageName;
	}

	/*void CarRenderer::pushCarHistory(int tick) {
		CarState state = { transform.pos, transform.rot, carProperties.currentAngle, tick };
		carStateHistory.push_back(state);
	}*/

	void CarRenderer::Render(Shader shader) {
		transform.Render(shader);
	}

	void CarRenderer::Render(Shader shader, glm::vec3 pos, float rot) {
		transform.Render(shader, pos, rot);
	}
}