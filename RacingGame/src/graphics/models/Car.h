#ifndef CAR_H
#define CAR_H

#include <vector>

#include <glm/glm.hpp>

#include "Quad.hpp"

class Car {
private:
	// car model
	Quad transform;

	// car properties
	// GEARING SPEEDS (UNUSED) MAKE LATER
	float minSpeed;
	float maxSpeed;
	float turnRate;
	float traction;
public:
	float forwardRot;
	float velocity;
	float acceleration;
	float currentAngle;
	float driftAngle;

	Car(glm::vec3 position, float minSpeed, float maxSpeed, const char* imageName);
	~Car();

	Quad& getTransform() { return transform; }
	glm::vec3 getForwardDirection();
	void setPosition(glm::vec3 pos) { transform.pos = pos; }
	void setScale(glm::vec3 scale) { transform.size = scale;  }
	void setRotation(float rot) { transform.rot = rot; }
	void render(Shader shader);
};

#endif // !CAR_H