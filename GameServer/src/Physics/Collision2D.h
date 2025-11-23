#ifndef COLLISION2D_H
#define COLLISION2D_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Models/Quad.hpp"

class Collision2D {
private:

public:
	Collision2D();

	static bool checkOBBCollision(Quad& q1, Quad& q2);
	static bool checkOBBCollisionResolve(Quad& q1, Quad& q2, float fixedDeltaTime);
};

#endif // !COLLISION2D_H