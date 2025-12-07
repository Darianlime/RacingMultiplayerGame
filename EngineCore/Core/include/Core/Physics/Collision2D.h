#ifndef COLLISION2D_H
#define COLLISION2D_H
#define NOMINMAX

#include "Quad.h"
#include "CarPhysics.h"

namespace Core {
	struct CollisionResult {
		bool isCollided;
		glm::vec2 normal;
		float overlap;
	};

	class Collision2D {
	private:

	public:
		Collision2D() = default;

		static bool CheckOBBCollision(Quad& q1, Quad& q2);
		static bool CheckOBBCollisionResolve(Quad& q1, Quad& q2);
		static CollisionResult CheckOBBCollisionResolve(Quad& q1, CarState& carState1, Quad& q2, CarState& carState2);
		static bool CheckAABBCollsion(Quad& q1, Quad& q2);
	};
}
#endif // !COLLISION2D_H