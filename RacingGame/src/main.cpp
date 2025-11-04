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
#include "io/Camera2D.h"

#include "Physics/Collision2D.h"

void processInput(float dt);

float mixVal = 0.5f;

Screen screen(1270, 720);
Joystick mainJ(0);
Camera2D cameras[2] = {
	Camera2D(glm::vec3(0.0f, 0.0f, 0.0f)),
	Camera2D(glm::vec3(10.0f, 10.0f, 0.0f))
};
int activeCam = 0;
int activePlayer = 0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
	if (!screen.init()) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//screen.setParameters();

	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);

	// SHADERS============================================================
	Shader shader("assets/object.vert", "assets/object.frag");

	Car car1(glm::vec3(1000.0f, 0.0f, 0.0f), -500.0f, 2700.0f,  "assets/car1_2.png");
	Player player1(car1, 0);
	Car car2(glm::vec3(0.0f, 1000.0f, 0.0f), -500.0f, 2700.0f, "assets/car1_2.png");
	Player player2(car2, 1);

	Quad wall(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, { "assets/handpaintedwall2.png" }, STATIC);
	wall.init();
	Quad wall2(glm::vec3(0.0f, 500.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, { "assets/handpaintedwall2.png" }, STATIC);
	wall2.init();
	Quad wall3(glm::vec3(0.0f, 1000.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, { "assets/handpaintedwall2.png" }, STATIC);
	wall3.init();
	Quad wall4(glm::vec3(0.0f, 1500.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, { "assets/handpaintedwall2.png" }, STATIC);
	wall4.init();

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
		if (deltaTime > 0.033f)
			deltaTime = 0.033f;
		
		processInput(deltaTime);
		//cameras[activePlayer].updateCameraVectors();
		if (activePlayer == 0) {
			player1.PhysicsUpdate(deltaTime);
		}
		else {
			player2.PhysicsUpdate(deltaTime);
		}
		cameras[activePlayer].followTarget(player1.getCar().getTransform().pos);
		cameras[activePlayer].updateCameraDirection(player1.getCar().currentAngle - 90);

		bool collision1 = Collision2D::checkOBBCollisionResolve(player1.getCar().getTransform(), player2.getCar().getTransform());
		bool collision2 = Collision2D::checkOBBCollisionResolve(player1.getCar().getTransform(), wall);
		bool collision3 = Collision2D::checkOBBCollisionResolve(player2.getCar().getTransform(), wall);

		if (collision2) {
			//std::cout << "Player 1 collided with wall!" << std::endl;
			player1.getCar().velocity -= 3000.0f * deltaTime;
		}

		//std::cout << "Collision: " << (collision2 ? "Yes" : "No") << std::endl;
		screen.update();

		shader.activate();

		// create trasformation for screen
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		//glm::mat4 zoom = glm::scale(glm::mat4(1.0f), glm::vec3(cameras[activeCam].getZoom(), cameras[activeCam].getZoom(), 1.0f));

		projection = glm::ortho(-float(Screen::SCR_WIDTH), float(Screen::SCR_WIDTH), -float(Screen::SCR_HEIGHT), float(Screen::SCR_HEIGHT), -1.0f, 100.0f);
		view = cameras[activeCam].getViewMatrix();
		//view = glm::mat4(1.0f);

		//shader.activate();

		//shader.setFloat("mixVal", mixVal);
		/*shader.setMat4("view", view);
		shader.setMat4("projection", projection);*/
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		wall.render(shader);
		wall2.render(shader);
		wall3.render(shader);
		wall4.render(shader);
		player1.render(shader);
		player2.render(shader);
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

	/*if (Keyboard::key(GLFW_KEY_W)) {
		cameras[activePlayer].updateCameraPos(Camera2dDirection::UP, dt);
	}
	if (Keyboard::key(GLFW_KEY_S)) {
		cameras[activePlayer].updateCameraPos(Camera2dDirection::DOWN, dt);
	}
	if (Keyboard::key(GLFW_KEY_A)) {
		cameras[activePlayer].updateCameraPos(Camera2dDirection::LEFT, dt);
	}
	if (Keyboard::key(GLFW_KEY_D)) {
		cameras[activePlayer].updateCameraPos(Camera2dDirection::RIGHT, dt);
	}*/

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

	//double dx = Mouse::getDX(), dy = Mouse::getDY();
	/*if (dx != 0 || dy != 0) {
		cameras[activeCam].updateCameraDirection(dx, dy);
	}*/

	double scrollDy = Mouse::getScrollDY();
	if (scrollDy != 0) {
		cameras[activeCam].updateCameraZoom(scrollDy);
	}
}
