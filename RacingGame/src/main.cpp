#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/Shader.h"
//#include "graphics/Texture.h"
#include "graphics/models/Quad.hpp"

#include "io/Joystick.h"
#include "io/Keyboard.h"
#include "io/Mouse.h"
#include "io/Camera.h"
#include "io/Screen.h"

void processInput(float dt);

float mixVal = 0.5f;

Screen screen;
Joystick mainJ(0);
Camera cameras[2] = {
	Camera(glm::vec3(0.0f, 0.0f, 0.0f)),
	Camera(glm::vec3(10.0f, 10.0f, 0.0f))
};
int activeCam = 0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 pos(0.0f, 0.0f, 0.0f);
float rot = 0.0f;
float rotLeft = 0.0f;
float rotRight = 0.0f;
float setAngle = 0.0f;
float dirRot = 0.0f;
float currentAngle = 0.0f + 90;
float acceleration = 0.0f;
float velocity = 0.0f;
float driftAngle = 60.0f;
float traction = 0.0f;
glm::vec3 currentVel(0.0f, 0.0f, 0.0f);
glm::vec3 lastVel(0.0f, 0.0f, 0.0f);
glm::vec3 lastPos(0.0f, 0.0f, 0.0f);

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

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// SHADERS============================================================
	Shader shader("assets/object.vert", "assets/object.frag");

	Quad car(pos, glm::vec3(1.0f, 1.0f, 1.0f), -90.0f + rot, {"assets/car1_2.png"});
	car.init();
	//Cube cube(glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.75f));
	//cube.init();

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
		//cube.render(shader);
		car.pos = pos;
		car.rot = -90.0f + rot;
		car.render(shader);

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
	}

	if (Keyboard::key(GLFW_KEY_W) && !Keyboard::key(GLFW_KEY_SPACE)) {
		acceleration = 2700.0f;
	}
	else if (Keyboard::key(GLFW_KEY_S)) {
		acceleration = -400.0f;
	} else {
		acceleration = 0.0f;
	}
	velocity += acceleration * dt;
	velocity -= velocity * dt; // friction / coasting
	//printf("Velocity: %.2f\n", velocity);
	float turnRate = 100.0f;
	float calcRot = glm::sign(velocity) * turnRate * dt;
	float oversteerAngle = 0.0f; 
	if (velocity > 1000.0f && (((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_D))) || ((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_A))))) {
		oversteerAngle = glm::mix(0.0f, driftAngle, 1.1 * dt);
		printf("%f\n", oversteerAngle);
		velocity -= 40.0f * dt;
	}
	else {
		dirRot = glm::mix(dirRot, rot, 1.5 * dt);
	}
	if (Keyboard::key(GLFW_KEY_D) && glm::abs(velocity) > 100.0f) {
		rot -= calcRot + oversteerAngle;
		dirRot -= calcRot;
	}
	if (Keyboard::key(GLFW_KEY_A) && glm::abs(velocity) > 100.0f) {
		rot += calcRot + oversteerAngle;
		dirRot += calcRot;
	}
	if (Keyboard::key(GLFW_KEY_SPACE) && !((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_D))) || !((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_A)))) {
		velocity -= 300.0f * dt;
		if (velocity < 0.0f) velocity = 0.0f;
	}
	glm::vec3 forward = glm::vec3(glm::sin(glm::radians(currentAngle)), glm::cos(glm::radians(currentAngle)), 0.0f);
	forward = glm::normalize(forward);
	pos += forward * velocity * dt;
	currentAngle = -dirRot;

	double dx = Mouse::getDX(), dy = Mouse::getDY();
	if (dx != 0 || dy != 0) {
		cameras[activeCam].updateCameraDirection(dx, dy);
	}

	double scrollDy = Mouse::getScrollDY();
	if (scrollDy != 0) {
		cameras[activeCam].updateCameraZoom(scrollDy);
	}
}
