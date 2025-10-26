#ifndef COLLISION2D_H
#define COLLISION2D_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../graphics/models/Quad.hpp"

class Collision2D {
private:

public:
	Collision2D();

	static bool checkOBBCollision(Quad& quad1, Quad& quad2);
};

#endif // !COLLISION2D_H