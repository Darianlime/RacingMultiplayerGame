#include "Car.h"

Car::Car(glm::vec3 position, float minSpeed, float maxSpeed, const char* imageName) : minSpeed(minSpeed), maxSpeed(maxSpeed) {
	transform = Quad({ imageName }, DYNAMIC);
	transform.init();
	transform.rot = 0.0f;
	transform.pos = position;
	acceleration = 0.0f;
	turnRate = 100.0f;
	currentAngle = 0.0f;
	velocity = 0.0f;
	traction = 0.0f;
	driftAngle = 80.0f;
	forwardRot = 0.0f;
	baseTraction = 0.8f;
	tractionDecay = 0.0f;
	slipAngle = 0.0f;
	trackOversteer = 0.0f;
	spinout = false;
	pressedclutchkick = false;
}

Car::~Car() {
}

glm::vec3 Car::getForwardDirection() {
	float angleRad = glm::radians(currentAngle - 90.0f);
	return glm::normalize(glm::vec3(glm::sin(angleRad), glm::cos(angleRad), 0.0f));
}

void Car::render(Shader shader) {
	transform.render(shader);
}