#ifndef CAR_RENDE
#define CAR_RENDER_H

#include <vector>
#include <deque>
#include <memory>
#include <glm/glm.hpp>

#include "Quad.hpp"
#include "io/InputManager.h"
#include "Physics/Collision2D.h"
#include "Network/Packets.h"

using namespace NetworkClient;

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

		glm::vec3 GetForwardDirection() {
			float angleRad = glm::radians(currentAngle - 90.0f);
			return glm::normalize(glm::vec3(glm::sin(angleRad), glm::cos(angleRad), 0.0f));
		}

		glm::vec3 GetForwardDirectionSim(float simCurrentAngle) {
			float angleRad = glm::radians(simCurrentAngle);
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
		CarState SimulatePhysicsUpdate(CarState state, const InputState& inputState, double fixedDeltaTime);
		//void pushCarHistory(int tick);
		Quad& GetTransform() { return transform; }
		CarProperties& GetProperties() { return carProperties; }
		float GetCurrentAngle() { return currentAngle; }
		void GetCurrentAngle(float angle) { currentAngle = angle; }
		const char* GetImage() { return assetImage; }

		void Render(Shader shader);
		void Render(Shader shader, glm::vec3 pos, float rot);
	};
};
#endif // !CAR_H