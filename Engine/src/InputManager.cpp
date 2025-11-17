#include "io/InputManager.h"

namespace Engine {
	InputManager::InputManager() : input(0) {}

	InputState& InputManager::processInputs(uint64_t tick) {
		input = 0;
		if (Keyboard::key(GLFW_KEY_W)) input |= 1 << KeyInput::W;
		if (Keyboard::key(GLFW_KEY_S)) input |= 1 << KeyInput::S;
		if (Keyboard::key(GLFW_KEY_A)) input |= 1 << KeyInput::A;
		if (Keyboard::key(GLFW_KEY_D)) input |= 1 << KeyInput::D;
		if (Keyboard::key(GLFW_KEY_SPACE)) input |= 1 << KeyInput::SPACE;

		inputState.W = input & (1 << KeyInput::W);
		inputState.S = input & (1 << KeyInput::S);
		inputState.A = input & (1 << KeyInput::A);
		inputState.D = input & (1 << KeyInput::D);
		inputState.Space = input & (1 << KeyInput::SPACE);
		inputState.tick = tick;

		inputStateHistory[tick % HISTORY_MAX] = inputState;

		return inputState;
	}
}