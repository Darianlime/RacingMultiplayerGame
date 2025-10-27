#include "io/Screen.h"
#include "io/Keyboard.h"
#include "io/Mouse.h"

unsigned int Screen::SCR_WIDTH = 800;
unsigned int Screen::SCR_HEIGHT = 600;

void Screen::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}

Screen::Screen() : window(nullptr), monitor(nullptr), mode(nullptr) {
	monitor = glfwGetPrimaryMonitor();
	mode = glfwGetVideoMode(monitor);
	GetMonitorSizeGLFW(monitor, mode, SCR_WIDTH, SCR_HEIGHT);
}

Screen::Screen(unsigned int scr_width, unsigned int scr_height) : window(nullptr), monitor(nullptr), mode(nullptr) {
	this->SCR_HEIGHT = scr_height;
	this->SCR_WIDTH = scr_width;
}

bool Screen::init() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Racing Game", NULL, NULL);
	if (!window)
	{
		return false;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwTerminate();
		return false;
	}
	setParameters();
	return true;
}

void Screen::GetMonitorSizeGLFW(GLFWmonitor* monitor, const GLFWvidmode* mode, unsigned int& width, unsigned int& height) {
	if (monitor) {
		width = mode->width;
		height = mode->height;
	}
	else {
		// Handle error: No primary monitor found
		width = 800;
		height = 600;
	}
}

void Screen::setParameters() {
	glfwSetFramebufferSizeCallback(window, Screen::framebufferSizeCallback);

	glfwSetKeyCallback(window, Keyboard::keyCallback);

	glfwSetCursorPosCallback(window, Mouse::cursorPosCallback);
	glfwSetMouseButtonCallback(window, Mouse::mouseButtonCallback);
	glfwSetScrollCallback(window, Mouse::mouseWheelCallback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Screen::update() {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void Screen::newFrame() {
	glfwSwapBuffers(window);
	glfwPollEvents();
}

bool Screen::shouldClose() {
	return glfwWindowShouldClose(window);
}

void Screen::setShouldClose(bool shouldClose) {
	glfwSetWindowShouldClose(window, shouldClose);
}