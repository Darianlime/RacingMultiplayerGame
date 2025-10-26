#include "Car.h"

Car::Car(glm::vec3 position, float minSpeed, float maxSpeed, const char* imageName) : minSpeed(minSpeed), maxSpeed(maxSpeed) {
	transform = Quad({ imageName });
	transform.init();
	transform.rot = 0.0f;
	transform.pos = position;
	acceleration = 0.0f;
	turnRate = 100.0f;
	currentAngle = 0.0f;
	velocity = 0.0f;
	traction = 0.0f;
	driftAngle = 60.0f;
	forwardRot = 0.0f;
}

Car::~Car() {
}

glm::vec3 Car::getForwardDirection() {
	return glm::vec3(glm::sin(glm::radians(currentAngle - 90.0f)), glm::cos(glm::radians(currentAngle - 90.0f)), 0.0f);
}

void Car::render(Shader shader) {
	transform.render(shader);
}