#include "Camera2D.h"

Camera2D::Camera2D(glm::vec3 position)
	: cameraPos(position),
	worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	speed(10.5f),
	zoom(45.0f),
	cameraFront(glm::vec3(0.0f, 0.0f, -1.0f)) {
	updateCameraVectors();
}

void Camera2D::updateCameraDirection(float dz) {
	/*yaw += dx;
	pitch += dy;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	else if (pitch < -89.0f) {
		pitch = -89.0f;
	}*/
	roll = dz;
	updateCameraVectors();
}


void Camera2D::followTarget(glm::vec3 targetPos) {
	cameraPos = targetPos;
}

void Camera2D::updateCameraPos(Camera2dDirection direction, double dt) {
	float velocity = (float)dt * speed;

	switch (direction) {
	case Camera2dDirection::RIGHT:
		printf("In Right");
		cameraPos += cameraRight * velocity;
		break;
	case Camera2dDirection::LEFT:
		printf("In LEFT");
		cameraPos -= cameraRight * velocity;
		break;
	case Camera2dDirection::UP:
		printf("In U");
		cameraPos += cameraUp * velocity;
		break;
	case Camera2dDirection::DOWN:
		printf("In DOWN");
		cameraPos -= cameraUp * velocity;
		break;
	}
}

void Camera2D::updateCameraZoom(double dy) {
	if (zoom >= 1.0f && zoom <= 90.0f) {
		zoom -= dy;
	}
	else if (zoom < 1.0f) {
		zoom = 1.0f;
	}
	else {
		zoom = 90.0f;
	}
}

float Camera2D::getZoom() {
	return zoom;
}

glm::mat4 Camera2D::getViewMatrix() {
	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera2D::updateCameraVectors() {
	glm::vec3 direction;
	//direction.x = cos(90.0f);
	//direction.y = sin(0.0f);
	//direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	//cameraFront = glm::normalize(direction);

	cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

	glm::mat4 rollMat = glm::rotate(glm::mat4(1.0f), glm::radians(roll), cameraFront);
	cameraRight = glm::vec3(rollMat * glm::vec4(cameraRight, 0.0f));
	cameraUp = glm::vec3(rollMat * glm::vec4(cameraUp, 0.0f));
}