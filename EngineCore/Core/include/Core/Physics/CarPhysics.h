#ifndef CAR_PHYSICS_H
#define CAR_PHYSICS_H

#include <vector>
#include <iostream>
#include "glm/glm.hpp"
#include "Quad.h"
#include "../io/InputManager.h"

namespace Core {
	struct CarState {
		glm::vec3 pos;
		float rot;
		float currentAngle;
		float velocity;
		float forwardRot;
		bool collided;
	};

	class CarPhysics {
	private:
		Quad transform;
		Quad transformQuery;
		std::vector<uint16_t> queryIndex;

		// car properties
		// GEARING SPEEDS (UNUSED) MAKE LATER
		static float minSpeed;
		static float maxSpeed;
		float turnRate;
		float traction;
	public:
		float forwardRot;
		float velocity;
		float currentAngle;
		float driftAngle;
		bool collided;
		std::string assetImage;

		CarPhysics();

		Quad& GetTransform() { return transform; }
		Quad& GetTransformQuery() { return transformQuery; }

		glm::vec3 GetForwardDirection() const;
		static glm::vec3 GetForwardDirectionSim(float simCurrentAngle);

		std::vector<uint16_t>& GetQueryIndex() { return queryIndex; }
		uint16_t& GetQueryIndexValue(uint8_t index) { return queryIndex[index]; }

		void Update(InputState inputState, float fixedDeltaTime);
		static CarState SimulatePhysicsUpdate(CarState state, const InputState& inputState, double fixedDeltaTime);
	};
}
#endif // !CAR_PHYSICS_H