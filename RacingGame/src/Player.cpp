#include "Player.h"
#include <cassert>
#include <cmath>
#include <iostream>

Player::Player(Car& car, int id) : car(car), id(id) {}

float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

void Player::PhysicsUpdate(float dt) {
	if (Keyboard::key(GLFW_KEY_W) && !Keyboard::key(GLFW_KEY_SPACE) && !car.spinout) {
		car.acceleration = 2000.0f;
	}
	else if (Keyboard::key(GLFW_KEY_S)) {
		car.acceleration = -2000.0f;
	}
	else {
		car.acceleration = 0.0f;
	}
	//printf("time: %f\n", car.acceleration);
	car.velocity += car.acceleration * dt;
	float drag = 150.0f + 0.2f * glm::abs(car.velocity);
	if (car.velocity > 0.0f)
		car.velocity -= drag * dt;
	else if (car.velocity < 0.0f)
		car.velocity += drag * dt;
	car.velocity = glm::clamp(car.velocity, -800.0f, 2700.0f);
	//printf("Velocity: %.2f\n", car.velocity);
	float turnRate = 100.0f;
	float calcRot = glm::sign(car.velocity) * turnRate * dt;

	// Handbrake or cluctch kick initiate counter steer
	if (!car.pressedclutchkick && car.velocity > 2000.0f && (Keyboard::key(GLFW_KEY_LEFT_SHIFT) && Keyboard::key(GLFW_KEY_A))) {
		car.tractionDecay = 80.0f;
		Keyboard::setLastPressedKey(GLFW_KEY_A);
		car.pressedclutchkick = true;
	}
	if (!car.pressedclutchkick && car.velocity > 2000.0f && (Keyboard::key(GLFW_KEY_LEFT_SHIFT) && Keyboard::key(GLFW_KEY_D))) {
		car.tractionDecay = 80.0f;
		Keyboard::setLastPressedKey(GLFW_KEY_D);
		car.pressedclutchkick = true;
	}

	if (car.pressedclutchkick) {
		// appling throttle while drifting 
		car.tractionDecay -= 10 * dt;
		//printf("%.2f", car.tractionDecay);
		if (Keyboard::key(GLFW_KEY_W)) {
			car.trackOversteer = 80.0f * dt;
		}
		else {
			car.trackOversteer = -50.0f * dt;
		}
		//oversteerAngle = glm::mix(oversteerAngle, car.driftAngle, 1.2f * dt);
		//if (!(glm::abs(car.getTransform().rot - car.forwardRot) >= 70)) {
		if (Keyboard::getLastPressedKey() == GLFW_KEY_A) {
			car.getTransform().rot += calcRot + car.trackOversteer;
			car.forwardRot += calcRot;
			// relase throttle snap wheel back to other direction 
			if (car.getTransform().rot - car.forwardRot <= -20.0f) {
				printf("in?");
				Keyboard::setLastPressedKey(GLFW_KEY_D);
			}
		}
		if (Keyboard::getLastPressedKey() == GLFW_KEY_D) {
			car.getTransform().rot -= calcRot + car.trackOversteer;
			car.forwardRot -= calcRot;
			// relase throttle snap wheel back to other direction 
			if (car.getTransform().rot - car.forwardRot >= 20.0f) {
				printf("in?");
				Keyboard::setLastPressedKey(GLFW_KEY_A);
			}
		}
		if (car.getTransform().rot - car.forwardRot <= 12.0f && car.getTransform().rot - car.forwardRot >= -12.0f && Keyboard::key(GLFW_KEY_W) && car.tractionDecay < 1.0f) {
			Keyboard::setLastPressedKey(GLFW_KEY_LAST);
			car.pressedclutchkick = false;
			printf("ion\n");
		}
		if (glm::abs(car.getTransform().rot - car.forwardRot) >= 65.0f) {
			car.velocity -= 500.0f * dt;
			car.spinout = true;
			if (car.velocity < 100.0f) {
				car.acceleration = 0.0f;
				car.pressedclutchkick = false;
				car.spinout = false;
			}
		}
		car.velocity -= 500.0f * dt;
	}
	else { // retarget camera and make new foward rot
		//car.forwardRot += car.getTransform().rot - car.forwardRot;
		car.spinout = false;
		if (car.getTransform().rot - car.forwardRot > 1.0f) {
			printf("in>");
			car.trackOversteer -= (dt * 200.0f);
		}
		else if (car.getTransform().rot - car.forwardRot < -1.0f) {
			printf("in<");
			car.trackOversteer += (dt * 200.0f);
		} else {
			printf("in0");
			car.trackOversteer = glm::mix(car.trackOversteer, 0.0f, dt * 5.0f);
		}
		Keyboard::setLastPressedKey(GLFW_KEY_LAST);
	}
	printf("%.2f\n", car.getTransform().rot - car.forwardRot);
	car.slipAngle = car.getTransform().rot - car.forwardRot;
	if (Keyboard::getLastPressedKey() == GLFW_KEY_D && Keyboard::key(GLFW_KEY_A) && car.pressedclutchkick) {
		//printf("in\n");
		car.forwardRot -= calcRot;
		car.trackOversteer = -10.0f * dt;
	}
	if (Keyboard::getLastPressedKey() == GLFW_KEY_A && Keyboard::key(GLFW_KEY_D) && car.pressedclutchkick) {
		//printf("in\n");
		car.forwardRot += calcRot;
		car.trackOversteer = -10.0f * dt;
	}
	if (!(Keyboard::getLastPressedKey() == GLFW_KEY_D) && Keyboard::key(GLFW_KEY_D) && glm::abs(car.velocity) > 100.0f) {
		car.getTransform().rot -= calcRot + car.trackOversteer;
		car.forwardRot -= calcRot;
	}
	if (!(Keyboard::getLastPressedKey() == GLFW_KEY_A) && (Keyboard::key(GLFW_KEY_A) && glm::abs(car.velocity) > 100.0f)) {
		car.getTransform().rot += calcRot + car.trackOversteer;
		car.forwardRot += calcRot;
	}
	if (Keyboard::key(GLFW_KEY_SPACE) && (!((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_D))) || !((Keyboard::key(GLFW_KEY_SPACE) && Keyboard::key(GLFW_KEY_A))))) {
		car.velocity -= 100.0f * dt;
		if (car.velocity < 0.0f) car.velocity = 0.0f;
	}
	glm::vec3 forward = car.getForwardDirection();
	forward = glm::normalize(forward);
	car.getTransform().pos += forward * (car.velocity * dt);
	car.currentAngle = -car.forwardRot;
	//printf("Angle: %.2f, Forward: (%.2f, %.2f), %.2f\n",
       //car.currentAngle, forward.x, forward.y, car.velocity);
}
