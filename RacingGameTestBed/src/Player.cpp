#include "Player.h"
#include <cassert>
#include <cmath>
#include <iostream>

static int steerDir = 0;
static bool inCounterSteer = false;
static bool chaingDriftLeft = false;
static bool chaingDriftRight = false;

Player::Player(Car& car, int id) : car(car), id(id) {}

float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

void Player::PhysicsUpdate(float dt) {
	bool throttle = Keyboard::key(GLFW_KEY_W);
	bool reversing = Keyboard::key(GLFW_KEY_S);
	bool brake = Keyboard::key(GLFW_KEY_SPACE);
	bool driftLeft = Keyboard::key(GLFW_KEY_LEFT_SHIFT) && Keyboard::key(GLFW_KEY_A);
	bool driftRight = Keyboard::key(GLFW_KEY_LEFT_SHIFT) && Keyboard::key(GLFW_KEY_D);
	if (Keyboard::key(GLFW_KEY_A)) {
		steerDir = -1;
	}
	else if (Keyboard::key(GLFW_KEY_D)) {
		steerDir = 1;
	}
	else {
		steerDir = 0;
	}
	
	if (car.pressedclutchkick && throttle) {
		car.acceleration = glm::mix(car.acceleration, 4000.0f, 3.0f * dt);
	}
	else if (car.pressedclutchkick) {
		car.acceleration = glm::mix(car.acceleration, 0.0f, 3.0f * dt);
	}
	else if ((throttle && !brake && !car.spinout)) {
		car.acceleration = 4000.0f;
	}
	else if (reversing) {
		car.acceleration = -4000.0f;
	}
	else {
		car.acceleration = 0.0f;
	}
	//printf("%.2f\n", car.acceleration);
	float rateOfAccel = glm::clamp(car.acceleration/4000.0f, 0.1f, 1.0f);
	//printf("time: %f\n", car.acceleration);
	car.velocity += car.acceleration * dt;
	float drag = 1000.0f + 0.5f * glm::abs(car.velocity);
	if (car.velocity > 0.0f)
		car.velocity -= drag * dt;
	else if (car.velocity < 0.0f)
		car.velocity += drag * dt;
	car.velocity = glm::clamp(car.velocity, -800.0f, 2700.0f);
	//printf("Velocity: %.2f\n", car.velocity);

	// Handbrake or cluctch kick initiate counter steer
	float speedFactor = glm::clamp(car.velocity / 7000.0f, 0.4f, 1.5f); 
	float slipDelta = fabs(car.getTransform().rot - car.forwardRot - car.slipAngle); // how fast rear steps out
	float slipFactor = glm::clamp(slipDelta, 0.5f, 1.5f);

	if (!car.pressedclutchkick && car.velocity > 2000.0f && driftLeft) {
		car.tractionDecay = 60.0f * speedFactor * slipFactor;
		car.initalTractionDecay = car.tractionDecay;
		Keyboard::setLastPressedKey(GLFW_KEY_A);
		car.pressedclutchkick = true;
		car.acceleration = 4000.0f;
		car.slipAngle = 20.0f;
		car.trackOversteer = 200.0f * dt;
		car.velocity += 300.0f;
	}
	if (!car.pressedclutchkick && car.velocity > 2000.0f && driftRight) {
		car.tractionDecay = 60.0f * speedFactor * slipFactor;
		car.initalTractionDecay = car.tractionDecay;
		Keyboard::setLastPressedKey(GLFW_KEY_D);
		car.pressedclutchkick = true;
		car.acceleration = 4000.0f;
		car.slipAngle = -20.0f;
		car.trackOversteer = 200.0f * dt;
		car.velocity += 300.0f;
	}

	
	float turnRate = 100.0f;
	float calcRot = glm::sign(car.velocity) * turnRate * dt;
	if (car.pressedclutchkick) {
		car.slipAngle = car.getTransform().rot - car.forwardRot;
		float slip = car.slipAngle;
		float slipMag = fabs(slip);

		float lateralVel = glm::sin(glm::radians(car.slipAngle)) * car.velocity;
		car.lateralVelocity = glm::mix(car.lateralVelocity, lateralVel, 5.0f * dt);
		bool hasMomentum = fabs(lateralVel) > 300.0f;
		bool isChainingDrift = (car.prevSlipAngle * car.slipAngle < 0.0f);

		car.driftDir = (car.slipAngle > 0.0f) ? 1.0f : -1.0f;
		car.chainCooldown = glm::max(0.0f, car.chainCooldown - dt);
		// Switch drift direction mid-slide
		if (car.chainCooldown <= 0 && hasMomentum && isChainingDrift) {
			if (car.slipAngle <= -10.0f) {
				Keyboard::setLastPressedKey(GLFW_KEY_D);
				//car.slipAngle = 15.0f;
				car.tractionDecay = glm::max(car.tractionDecay, 25.0f);
				car.chainCooldown = 1.0f;
				printf("Chained LEFT drift!\n");
			}
			else if (car.slipAngle >= 10.0f) {
				Keyboard::setLastPressedKey(GLFW_KEY_A);
				//car.slipAngle = -15.0f;
				car.tractionDecay = glm::max(car.tractionDecay, 25.0f);
				car.chainCooldown = 1.0f;
				printf("Chained RIGHT drift!\n");
			}
		}

		printf("%.2f, regain: %d\n", car.lateralVelocity, isChainingDrift);
		float tractionDecayRate = glm::clamp((car.tractionDecay / car.initalTractionDecay), 0.0f, 1.0f);
		float driftDrag = 400.0f + 1500.0f * slipMag / 45.0f;
		driftDrag *= tractionDecayRate;

		float throttleInput = Keyboard::key(GLFW_KEY_W) ? 1.0f : 0.0f;
		car.velocity -= driftDrag * (1.2f - throttleInput * 0.7f) * dt;

		if (car.velocity < 200.0f) car.velocity = 200.0f;

		float regainRate = glm::clamp((1.0f - slipMag / 60.0f), 0.0f, 1.0f);
		regainRate *= (1.0f - throttleInput * 0.5f);

		if (throttle) { // ====CHANGED TO BASE OFF OF THROTTLE AND TRACTION======= 
			car.trackOversteer = 80.0f * rateOfAccel * dt;
		}
		else {
			// ====CHANGED TO BASE OFF OF LETTING GO OF THROTTLE AND TRACTION=======
			car.trackOversteer = -150.0f * rateOfAccel * dt;
		}

		if (Keyboard::getLastPressedKey() == GLFW_KEY_A) {
			car.getTransform().rot += calcRot + car.trackOversteer;
			car.forwardRot += calcRot;
			car.prevSlipAngle = 1;
		} 
		else if (Keyboard::getLastPressedKey() == GLFW_KEY_D) {
			car.getTransform().rot -= calcRot + car.trackOversteer;
			car.forwardRot -= calcRot;
			car.prevSlipAngle = -1;
		}

		//=====MAKE SO IF REGAIN TRACTION IT WILL SLOWLY AUTO ALIGN======

		// Counter Steering 
		float stoppedCounterSteering = 1.0f;
		if (Keyboard::getLastPressedKey() == GLFW_KEY_D && Keyboard::key(GLFW_KEY_A)) {
			//printf("in\n");
			car.forwardRot -= calcRot;
			car.trackOversteer = -10.0f * dt;
		}
		else if (Keyboard::getLastPressedKey() == GLFW_KEY_A && Keyboard::key(GLFW_KEY_D)) {
			//printf("in\n");
			car.forwardRot += calcRot;
			car.trackOversteer = -10.0f * dt;
		}
		else { //not counter steering regain traction slowly
			stoppedCounterSteering = 1.5f;
		}

		car.tractionDecay = glm::mix(car.tractionDecay, 0.0f, regainRate * stoppedCounterSteering * dt * 2.0f);

		// Exit drift
		if (car.tractionDecay < 2.0f && fabs(car.slipAngle) < 7.0f) {
			car.pressedclutchkick = false;
			car.trackOversteer = 0.0f;
			printf("Traction regained — exiting drift.\n");
		}

		if (glm::abs(car.getTransform().rot - car.forwardRot) >= 70.0f) {
			car.velocity -= 500.0f * dt;
			car.spinout = true;
			if (car.velocity < 100.0f) {
				car.acceleration = 0.0f;
				car.pressedclutchkick = false;
				car.spinout = false;
			}
		}
	}
	else { // retarget camera and make new foward rot
		//car.forwardRot += car.getTransform().rot - car.forwardRot;
		car.spinout = false;
		float rot = car.getTransform().rot;
		float target = car.forwardRot;
		float diff = fmodf(target - rot + 180.0f, 360.0f) - 180.0f;
		float smooth = 4.0f; // higher = snappier
		rot += diff * (1.0f - expf(-smooth * dt)); // exponential easing

		car.getTransform().rot = rot;
		car.trackOversteer = 0;
		//printf("in0");

		Keyboard::setLastPressedKey(GLFW_KEY_LAST);

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
