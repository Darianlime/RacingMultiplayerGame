#ifndef COLLISION2D_H
#define COLLISION2D_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Models/Quad.hpp"

class Collision2D {
private:

public:
	Collision2D();

	static bool CheckOBBCollision(Quad& q1, Quad& q2);
	static bool CheckOBBCollisionResolve(Quad& q1, Quad& q2);
	static bool CheckAABBCollsion(Quad& q1, Quad& q2);
};

#endif // !COLLISION2D_H