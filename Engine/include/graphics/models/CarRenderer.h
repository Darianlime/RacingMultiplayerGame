#ifndef CAR_RENDE
#define CAR_RENDER_H

#include <vector>
#include <deque>
#include <glm/glm.hpp>

#include "Quad.hpp"
#include "io/InputManager.h"
#include "Physics/Collision2D.h"
#include "Shared/CarState.h"

using namespace NetWork;

namespace Engine {
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
			currentAngle(0.0f)
		{
		}

		glm::vec3 getForwardDirection() {
			float angleRad = glm::radians(currentAngle - 90.0f);
			return glm::normalize(glm::vec3(glm::sin(angleRad), glm::cos(angleRad), 0.0f));
		}

		glm::vec3 getForwardDirectionSim(float simCurrentAngle) {
			float angleRad = glm::radians(simCurrentAngle - 90.0f);
			return glm::normalize(glm::vec3(glm::sin(angleRad), glm::cos(angleRad), 0.0f));
		}
	};

	class CarRenderer {
	private:
		// car model
		Quad transform;
		CarProperties carProperties;
		float currentAngle;
		const char* assetImage;
	public:

		CarRenderer();
		CarRenderer(const char* imageName);
		CarRenderer(glm::vec3 pos, const char* imageName);

		void PredictionPhysicsUpdate(InputState& inputState, double fixedDeltaTime);
		CarState SimulatePhysicsUpdate(CarState& state, InputState& inputState, double fixedDeltaTime);
		//void pushCarHistory(int tick);
		Quad& getTransform() { return transform; }
		CarProperties& getProperties() { return carProperties; }
		float getCurrentAngle() { return currentAngle; }
		void setCurrentAngle(float angle) { currentAngle = angle; }
		const char* getImage() { return assetImage; }

		void render(Shader shader);
		void render(Shader shader, glm::vec3 pos, float rot);
	};
};
#endif // !CAR_H