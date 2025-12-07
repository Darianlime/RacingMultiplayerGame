#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <map>

namespace Core {
	typedef enum {
		W, S, A, D, SPACE
	} KeyInput;

	struct InputState {
		bool W = false;
		bool S = false;
		bool A = false;
		bool D = false;
		bool Space = false;
		uint64_t tick = 0;
	};

	class InputManager {
	private:
		const uint8_t HISTORY_MAX = 150;
		InputState inputState;
	public:
		uint8_t input;
		std::map<uint64_t, InputState> inputStateHistory;

		InputManager();

		InputState& ProcessInputs(uint8_t inputs, uint64_t tick);
		InputState& GetInputState() { return inputState; }
		InputState& GetInputStateHistory(uint64_t tick) { return inputStateHistory[tick % HISTORY_MAX]; }
	};
}
#endif // !INPUT_MANAGER_H