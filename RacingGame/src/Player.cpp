#include "Player.h"

Player::Player(Car& car, int id) : car(car), id(id) {}

void Player::Update(float dt) {
	if (Keyboard::key(GLFW_KEY_W) && !Keyboard::key(GLFW_KEY_SPACE)) {
		car.acceleration = 2000.0f;
	}
	else if (Keyboard::key(GLFW_KEY_S)) {
		car.acceleration = -400.0f;
	}
	else {
		car.acceleration = 0.0f;
	}
	car.velocity += car.acceleration * dt;
	//car.velocity -= car.velocity * dt;
	float drag = 500.0f + 0.3f * glm::abs(car.velocity);
	if (car.velocity > 0.0f)
		car.velocity -= drag * dt;
	else if (car.velocity < 0.0f)
		car.velocity += drag * dt;
	if (glm::abs(car.velocity) < 0.1f)
		car.velocity = 0.0f;
	car.velocity = glm::clamp(car.velocity, -800.0f, 2700.0f);
	printf("Velocity: %.2f\n", car.velocity);
	float turnRate = 100.0f;
	float calcRot = glm::sign(car.velocity) * turnRate * dt;
	float oversteerAngle = 0.0f;
	if (car.velocity > 500.0f && (((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_D))) || ((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_A))))) {
		oversteerAngle = glm::mix(0.0f, car.driftAngle, 1.1 * dt);
		printf("%f\n", oversteerAngle);
		car.velocity -= 40.0f * dt;
	}
	else {
		car.forwardRot = glm::mix(car.forwardRot, car.getTransform().rot, 1.5 * dt);
	}
	if (Keyboard::key(GLFW_KEY_D) && glm::abs(car.velocity) > 100.0f) {
		car.getTransform().rot -= calcRot + oversteerAngle;
		car.forwardRot -= calcRot;
	}
	if (Keyboard::key(GLFW_KEY_A) && glm::abs(car.velocity) > 100.0f) {
		car.getTransform().rot += calcRot + oversteerAngle;
		car.forwardRot += calcRot;
	}
	if (Keyboard::key(GLFW_KEY_SPACE) && !((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_D))) || !((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_A)))) {
		car.velocity -= 300.0f * dt;
		if (car.velocity < 0.0f) car.velocity = 0.0f;
	}
	glm::vec3 forward = car.getForwardDirection();
	forward = glm::normalize(forward);
	car.getTransform().pos += forward * glm::abs(car.velocity) * dt;
	car.currentAngle = -car.forwardRot;
	printf("Angle: %.2f, Forward: (%.2f, %.2f), %.2f\n",
       car.currentAngle, forward.x, forward.y, car.velocity);
}
