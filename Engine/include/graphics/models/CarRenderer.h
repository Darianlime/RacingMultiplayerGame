#ifndef CAR_RENDER_H
#define CAR_RENDER_H

#include <vector>

#include <glm/glm.hpp>

#include "Quad.hpp"

namespace Engine {
	class CarRenderer {
	private:
		// car model
		Quad transform;
		float currentAngle;
		const char* assetImage;
	public:
		CarRenderer();
		CarRenderer(const char* imageName);
		CarRenderer(glm::vec3 pos, const char* imageName);

		Quad& getTransform() { return transform; }
		float getCurrentAngle() { return currentAngle; }
		void setCurrentAngle(float angle) { currentAngle = angle; }
		const char* getImage() { return assetImage; }

		void render(Shader shader);
	};
}
#endif // !CAR_H