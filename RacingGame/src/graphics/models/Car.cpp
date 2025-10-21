#include "Car.h"

Car::Car(Quad quad) : transform(quad) {
	quad.init();
}

void Car::render(Shader shader) {
	transform.render(shader);
}