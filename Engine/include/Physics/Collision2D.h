#ifndef COLLISION2D_H
#define COLLISION2D_H
#define NOMINMAX

#include "Network/Packets.h"
#include <glm/glm.hpp>
#include "../Graphics/Models/Quad.hpp"

using namespace NetworkClient;

namespace Engine {
	class Collision2D {
	private:

	public:
		Collision2D();

		static bool checkOBBCollision(Quad& q1, Quad& q2);
		static bool checkOBBCollisionResolve(Quad& q1, Quad& q2);
		static bool checkOBBCollisionResolve(Quad& q1, CarState& carState1, Quad& q2, CarState& carState2, float fixedDeltaTime);
	};
}
#endif // !COLLISION2D_H