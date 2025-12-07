#ifndef CAR_RENDER_H
#define CAR_RENDER_H

#include <vector>
#include <deque>
#include <memory>
#include <glm/glm.hpp>

#include "QuadRender.h"
#include "Core/io/InputManager.h"
#include "Core/Physics/CarPhysics.h"

namespace Client {
	using namespace Core;

	struct CarProperties {
		float minSpeed;
		float maxSpeed;
		//float turnRate;
		float accel;
		float forwardRot;
		float velocity;
		float acceleration;
		float currentAngle;

		CarProperties() :
			minSpeed(-2700.0f),
			maxSpeed(2700.0f),
			accel(0.0f),
			forwardRot(0.0f),
			velocity(0.0f),
			acceleration(0.0f),
			currentAngle(0.0f) {}
	};

	class CarRenderer {
	private:
		// car model
		QuadRender transform;
		CarProperties carProperties;
		float currentAngle;
		const char* assetImage;
	public:

		CarRenderer();
		CarRenderer(const char* imageName);
		CarRenderer(glm::vec3 pos, const char* imageName);

		//void pushCarHistory(int tick);
		QuadRender& GetTransform() { return transform; }
		CarProperties& GetProperties() { return carProperties; }
		float GetCurrentAngle() { return currentAngle; }
		void GetCurrentAngle(float angle) { currentAngle = angle; }
		const char* GetImage() { return assetImage; }

		void Render(Shader shader);
		void Render(Shader shader, glm::vec3 pos, float rot);
	};
};
#endif // !CAR_RENDER_H