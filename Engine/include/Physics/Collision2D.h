#ifndef COLLISION2D_H
#define COLLISION2D_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../Graphics/Models/Quad.hpp"
#include "Shared/CarState.h"

using namespace NetWork;

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