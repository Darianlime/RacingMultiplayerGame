#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Keyboard {
private:
	static bool keys[];
	static bool keysChanged[];
	static int lastKey;
public:
	// key state call back
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	// accessors
	static bool key(int key);
	static bool keyChanged(int key);
	static bool keyWentUp(int key);
	static bool keyWentDown(int key);
	static int getLastPressedKey();
	static void setLastPressedKey(int key);
};

#endif // !KEYBOARD_H

