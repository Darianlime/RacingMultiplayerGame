#include "pch.h"
#include "io/InputManager.h"

namespace Core {
	InputManager::InputManager() : input(0) {}

	InputState& InputManager::ProcessInputs(uint8_t input, uint64_t tick) {
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