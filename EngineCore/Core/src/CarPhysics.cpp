#include "pch.h"
#include "Physics/CarPhysics.h"

namespace Core {
	float CarPhysics::minSpeed = -2700.0f;
	float CarPhysics::maxSpeed = 2700.0f;

	CarPhysics::CarPhysics() :
		transform(DYNAMIC),
		transformQuery(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(704.0f, 704.0f, 1.0f), 0.0f, STATIC),
		queryIndex(transformQuery.noVertices, 0),
		velocity(0.0f),
		currentAngle(90.0f),
		forwardRot(0.0f),
		assetImage(""),
		collided(false) {
	}

	glm::vec3 CarPhysics::GetForwardDirection() const {
		float angleRad = glm::radians(currentAngle);
		return glm::normalize(glm::vec3(glm::sin(angleRad), glm::cos(angleRad), 0.0f));
	}

	glm::vec3 CarPhysics::GetForwardDirectionSim(float simCurrentAngle) {
		float angleRad = glm::radians(simCurrentAngle);
		return glm::normalize(glm::vec3(glm::sin(angleRad), glm::cos(angleRad), 0.0f));
	}

	void CarPhysics::Update(InputState inputState, float fixedDeltaTime) {
		float accel = 0.0f;
		if (inputState.W) {
			accel = maxSpeed;
		}
		else if (inputState.S) {
			accel = minSpeed;
		}

		velocity += accel * fixedDeltaTime;
		float drag = 500.0f + 0.3f * glm::abs(velocity);
		if (velocity > 0.0f)
			velocity -= drag * fixedDeltaTime;
		else if (velocity < 0.0f)
			velocity += drag * fixedDeltaTime;
		velocity = glm::clamp(velocity, minSpeed, maxSpeed);

		float turnRate = 100.0f;
		float calcRot = glm::sign(velocity) * turnRate * (float)fixedDeltaTime;

		if (inputState.D && glm::abs(velocity) > 100.0f) {
			GetTransform().rot -= calcRot;
			forwardRot -= calcRot;
		}
		if (inputState.A && glm::abs(velocity) > 100.0f) {
			GetTransform().rot += calcRot;
			forwardRot += calcRot;
		}
		if (inputState.Space && (!((inputState.Space && inputState.D)) || !((inputState.Space && inputState.A)))) {
			velocity -= 100.0f * float(fixedDeltaTime);
			if (velocity < 0.0f) velocity = 0.0f;
		}

		glm::vec3 forward = GetForwardDirection();
		forward = glm::normalize(forward);
		GetTransform().pos += forward * velocity * (float)fixedDeltaTime;
		currentAngle = -forwardRot;
		GetTransformQuery().pos = GetTransform().pos;
		GetTransformQuery().CalculateWorldVerts(glm::vec3(1.0f), 0.0f);
		GetTransform().CalculateWorldVerts(glm::vec3(444.0f, 208.0f, 1.0f), -90.0f);
	}

	CarState CarPhysics::SimulatePhysicsUpdate(CarState state, const InputState& inputState, double fixedDeltaTime) {
		//std::cout << "inital " << inputState.W << inputState.S << inputState.A << inputState.D << std::endl;
		float accel = 0.0f;
		if (inputState.W) {
			accel = maxSpeed;
		}
		else if (inputState.S) {
			accel = minSpeed;
		}

		state.velocity += accel * fixedDeltaTime;
		float drag = 500.0f + 0.3f * glm::abs(state.velocity);
		if (state.velocity > 0.0f)
			state.velocity -= drag * fixedDeltaTime;
		else if (state.velocity < 0.0f)
			state.velocity += drag * fixedDeltaTime;
		state.velocity = glm::clamp(state.velocity, minSpeed, maxSpeed);

		float turnRate = 100.0f;
		float calcRot = glm::sign(state.velocity) * turnRate * (float)fixedDeltaTime;

		if (inputState.D && glm::abs(state.velocity) > 100.0f) {
			state.rot -= calcRot;
			state.forwardRot -= calcRot;
		}
		if (inputState.A && glm::abs(state.velocity) > 100.0f) {
			state.rot += calcRot;
			state.forwardRot += calcRot;
		}
		if (inputState.Space && (!((inputState.Space && inputState.D)) || !((inputState.Space && inputState.A)))) {
			state.velocity -= 100.0f * float(fixedDeltaTime);
			if (state.velocity < 0.0f) state.velocity = 0.0f;
		}

		glm::vec3 forward = GetForwardDirectionSim(state.currentAngle);
		forward = glm::normalize(forward);
		state.pos += forward * state.velocity * (float)fixedDeltaTime;
		state.currentAngle = -state.forwardRot;

		return state;
	}

}