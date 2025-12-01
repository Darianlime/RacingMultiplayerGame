#ifndef PLAYER_H
#define PLAYER_H

#include <vector>

#include <glm/glm.hpp>

#include "graphics/models/Car.h"
#include "io/Keyboard.h"

class Player {
private:
	int id = 0;
	// car model
	Car& car;

public:
	Player(Car& car, int id);
	int getId() { return id; }
	Car& getCar() { return car; }
	void setId(int newId) { id = newId; }
	void render(Shader shader) { car.render(shader); }
	void PhysicsUpdate(float dt);
};

#endif // !CAR_H