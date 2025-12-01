#ifndef COLLISION2D_H
#define COLLISION2D_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../graphics/models/Quad.hpp"

class Collision2D {
private:

public:
	Collision2D();

	static bool checkOBBCollision(Quad& q1, Quad& q2);
	static bool checkOBBCollisionResolve(Quad& q1, Quad& q2);
};

#endif // !COLLISION2D_H