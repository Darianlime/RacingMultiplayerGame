#ifndef SCREEN_H
#define SCREEN_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Screen {
private:
	GLFWmonitor* monitor;
	const GLFWvidmode* mode;
	GLFWwindow* window;
public:
	static unsigned int SCR_WIDTH;
	static unsigned int SCR_HEIGHT;

	static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

	Screen();

	bool init();

	void GetMonitorSizeGLFW(GLFWmonitor* monitor, const GLFWvidmode* mode, unsigned int& width, unsigned int& height);

	void setParameters();

	//main loop
	void update();
	void newFrame();

	bool shouldClose();
	void setShouldClose(bool shouldClose);
};

#endif // !SCREEN_H#pragma once
