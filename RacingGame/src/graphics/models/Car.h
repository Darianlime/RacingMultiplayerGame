#ifndef CAR_H
#define CAR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include <glm/glm.hpp>

#include "Quad.hpp"

class Car {
private:
	// car model
	Quad transform;

	// car properties
	glm::vec3 direction;
	float acceleration;
	float velocity;
	float turnRate;
	float currentAngle;

public:
	Car(Quad quad);

	Quad& getTransform() { return transform; }
	void setPosition(glm::vec3 pos) { transform.pos = pos; }
	void setRotation(float rot) { transform.rot = rot; }
	void render(Shader shader);
};

#endif // !CAR_H