#include "graphics/models/CarRenderer.h"
#include <io/InputManager.h>

namespace Engine {
	CarRenderer::CarRenderer() {
		transform = Quad(DYNAMIC);
		transform.initQuads();
		currentAngle = 0.0f;
		assetImage = NULL;
	}

	CarRenderer::CarRenderer(const char* imageName) {
		transform = Quad({ imageName }, DYNAMIC);
		transform.initQuads();
		currentAngle = 0.0f;
		assetImage = imageName;
	}

	CarRenderer::CarRenderer(glm::vec3 pos, const char* imageName) {
		transform = Quad({ imageName }, DYNAMIC);
		transform.initQuads();
		transform.pos = pos;
		currentAngle = 0.0f;
		assetImage = imageName;
	}

	void CarRenderer::PredictionPhysicsUpdate(InputState& inputState, double fixedDeltaTime) {
		//std::cout << "inital " << inputState.W << inputState.S << inputState.A << inputState.D << std::endl;
		if (inputState.W) {
			carProperties.accel = carProperties.maxSpeed;
		}
		else if (inputState.S) {
			carProperties.accel = -2700.0f;
		}
		else {
			carProperties.accel = 0.0;
		}
		carProperties.velocity += carProperties.accel * fixedDeltaTime;
		float drag = 500.0f + 0.3f * glm::abs(carProperties.velocity);
		if (carProperties.velocity > 0.0f)
			carProperties.velocity -= drag * fixedDeltaTime;
		else if (carProperties.velocity < 0.0f)
			carProperties.velocity += drag * fixedDeltaTime;
		carProperties.velocity = glm::clamp(carProperties.velocity, -2700.0f, carProperties.maxSpeed);

		float turnRate = 100.0f;
		float calcRot = glm::sign(carProperties.velocity) * turnRate * (float)fixedDeltaTime;

		if (inputState.D && glm::abs(carProperties.velocity) > 100.0f) {
			transform.rot -= calcRot;
			carProperties.forwardRot -= calcRot;
		}
		if (inputState.A && glm::abs(carProperties.velocity) > 100.0f) {
			transform.rot += calcRot;
			carProperties.forwardRot += calcRot;
		}
		if (inputState.Space && (!((inputState.Space && inputState.D)) || !((inputState.Space && inputState.A)))) {
			carProperties.velocity -= 100.0f * float(fixedDeltaTime);
			if (carProperties.velocity < 0.0f) carProperties.velocity = 0.0f;
		}

		glm::vec3 forward = carProperties.GetForwardDirection();
		forward = glm::normalize(forward);
		transform.pos += forward * carProperties.velocity * (float)fixedDeltaTime;
		carProperties.currentAngle = -carProperties.forwardRot;
	}

	CarState CarRenderer::SimulatePhysicsUpdate(CarState state, const InputState& inputState, double fixedDeltaTime) {
		//std::cout << "inital " << inputState.W << inputState.S << inputState.A << inputState.D << std::endl;
		if (inputState.W) {
			carProperties.accel = carProperties.maxSpeed;
		}
		else if (inputState.S) {
			carProperties.accel = -2700.0f;
		}
		else {
			carProperties.accel = 0.0;
		}
		state.velocity += carProperties.accel * fixedDeltaTime;
		float drag = 500.0f + 0.3f * glm::abs(state.velocity);
		if (state.velocity > 0.0f)
			state.velocity -= drag * fixedDeltaTime;
		else if (state.velocity < 0.0f)
			state.velocity += drag * fixedDeltaTime;
		state.velocity = glm::clamp(state.velocity, -2700.0f, carProperties.maxSpeed);

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

		glm::vec3 forward = carProperties.GetForwardDirectionSim(state.currentAngle);
		forward = glm::normalize(forward);
		state.pos += forward * state.velocity * (float)fixedDeltaTime;
		state.currentAngle = -state.forwardRot;

		return state;
	}

	/*void CarRenderer::pushCarHistory(int tick) {
		CarState state = { transform.pos, transform.rot, carProperties.currentAngle, tick };
		carStateHistory.push_back(state);
	}*/

	void CarRenderer::Render(Shader shader) {
		transform.render(shader);
	}

	void CarRenderer::Render(Shader shader, glm::vec3 pos, float rot) {
		transform.render(shader, pos, rot);
	}
}