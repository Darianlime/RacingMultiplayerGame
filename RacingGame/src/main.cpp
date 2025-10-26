#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/Shader.h"
//#include "graphics/Texture.h"
#include "graphics/models/Quad.hpp"
#include "graphics/models/Car.h"
#include "Player.h"

#include "io/Joystick.h"
#include "io/Keyboard.h"
#include "io/Mouse.h"
#include "io/Camera.h"
#include "io/Screen.h"

#include "Physics/Collision2D.h"

void processInput(float dt);

float mixVal = 0.5f;

Screen screen;
Joystick mainJ(0);
Camera cameras[2] = {
	Camera(glm::vec3(0.0f, 0.0f, 0.0f)),
	Camera(glm::vec3(10.0f, 10.0f, 0.0f))
};
int activeCam = 0;
int activePlayer = 0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
	int success;
	char infoLog[512];

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (!screen.init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	screen.setParameters();

	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);

	// SHADERS============================================================
	Shader shader("assets/object.vert", "assets/object.frag");

	//Car car(glm::vec3(1000.0f, 0.0f, 0.0f), -500.0f, 2700.0f, { "assets/car1_2.png" });
	Player player1(Car(glm::vec3(1000.0f, 0.0f, 0.0f), -500.0f, 2700.0f, "assets/car1_2.png"), 0);
	//Player player2(Car(glm::vec3(0.0f, 1000.0f, 0.0f), -500.0f, 2700.0f,  "assets/car1_2.png") , 1);

	Quad wall(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, { "assets/handpaintedwall2.png" });
	wall.init();

	mainJ.update();
	if (mainJ.isPresent()) {
		std::cout << mainJ.getName() << " is present." << std::endl;
	}
	else {
		std::cout << "not present." << std::endl;
	}

	while (!screen.shouldClose())
	{
		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastFrame;
		lastFrame = currentTime;

		
		processInput(deltaTime);
		
		if (activePlayer) {
			player1.Update(deltaTime);
		}
		else {
			//player2.Update(deltaTime);
		}

		bool collision = Collision2D::checkOBBCollision(player1.getCar().getTransform(), wall);
		std::cout << "Collision: " << (collision ? "Yes" : "No") << std::endl;
		screen.update();

		shader.activate();

		// create trasformation for screen
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);

		//view = cameras[activeCam].getViewMatrix();
		projection = glm::ortho(-float(Screen::SCR_WIDTH), float(Screen::SCR_WIDTH), -float(Screen::SCR_HEIGHT), float(Screen::SCR_HEIGHT), -1.0f, 1.0f);
		view = glm::mat4(1.0f);

		//shader.activate();

		//shader.setFloat("mixVal", mixVal);
		/*shader.setMat4("view", view);
		shader.setMat4("projection", projection);*/
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		wall.render(shader);
		player1.render(shader);
		//player2.render(shader);
		//wall.render(shader);
		screen.newFrame();
	}

	glfwTerminate();
	return 0;
}

void processInput(float dt)
{
	if (Keyboard::key(GLFW_KEY_ESCAPE) || mainJ.buttonState(GLFW_JOYSTICK_BTN_RIGHT)) {
		screen.setShouldClose(true);
	}

	//change mix val
	if (Keyboard::key(GLFW_KEY_UP)) {
		mixVal += 0.05f;
		if (mixVal > 1) {
			mixVal = 1.0f;
		}
	}

	if (Keyboard::key(GLFW_KEY_DOWN)) {
		mixVal -= 0.05f;
		if (mixVal < 0) {
			mixVal = 0.0f;
		}
	}

	if (Keyboard::keyWentDown(GLFW_KEY_TAB)) {
		activeCam += (activeCam == 0) ? 1 : -1;
		activePlayer += (activePlayer == 0) ? 1 : -1;
	}

	double dx = Mouse::getDX(), dy = Mouse::getDY();
	if (dx != 0 || dy != 0) {
		cameras[activeCam].updateCameraDirection(dx, dy);
	}

	double scrollDy = Mouse::getScrollDY();
	if (scrollDy != 0) {
		cameras[activeCam].updateCameraZoom(scrollDy);
	}
}
