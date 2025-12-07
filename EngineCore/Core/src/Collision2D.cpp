#include "pch.h"
#include "Physics/Collision2D.h"

namespace Core {
	bool Collision2D::CheckOBBCollision(Quad& q1, Quad& q2) {
		Quad* quad1 = &q1;
		Quad* quad2 = &q2;

		for (int s = 0; s < 2; s++) {
			if (s == 1) {
				std::swap(quad1, quad2);
			}
			for (int i = 0; i < 4; i++) {
				glm::vec3 p1 = quad1->worldVerts[i];
				glm::vec3 p2 = quad1->worldVerts[(i + 1) % 4];
				glm::vec3 edge = p2 - p1;
				glm::vec3 axis = glm::normalize(glm::vec3(-edge.y, edge.x, 0.0f));
				float min1 = FLT_MAX;
				float max1 = -FLT_MAX;
				for (const auto& v : quad1->worldVerts) {
					float projection = glm::dot(v, axis);
					min1 = std::min(min1, projection);
					max1 = std::max(max1, projection);
				}
				float min2 = FLT_MAX;
				float max2 = -FLT_MAX;
				for (const auto& v : quad2->worldVerts) {
					float projection = glm::dot(v, axis);
					min2 = std::min(min2, projection);
					max2 = std::max(max2, projection);
				}
				if (max1 < min2 || max2 < min1) {
					return false; // No collision
				}
			}
		}
		return true;
	}

	bool Collision2D::CheckOBBCollisionResolve(Quad& q1, Quad& q2) {
		Quad* quad1 = &q1;
		Quad* quad2 = &q2;

		const float EPS = 1e-6f;
		float smallestOverlap = FLT_MAX;
		glm::vec2 smallestAxis(0.0f);

		auto testAxis = [&](const glm::vec2& axisUnnorm) -> bool {
			float axisLen = glm::length(axisUnnorm);
			if (axisLen < EPS) return true; // degenerate edge, skip axis

			glm::vec2 axis = axisUnnorm / axisLen; // normalized axis

			// project quad1
			float min1 = FLT_MAX, max1 = -FLT_MAX;
			for (auto& v : q1.worldVerts) {
				float proj = glm::dot(glm::vec2(v.x, v.y), axis);
				min1 = std::min(min1, proj);
				max1 = std::max(max1, proj);
			}

			// project quad2
			float min2 = FLT_MAX, max2 = -FLT_MAX;
			for (auto& v : q2.worldVerts) {
				float proj = glm::dot(glm::vec2(v.x, v.y), axis);
				min2 = std::min(min2, proj);
				max2 = std::max(max2, proj);
			}

			float overlap = std::min(max1, max2) - std::max(min1, min2);
			if (overlap <= 0.0f) return false;

			if (overlap < smallestOverlap) {
				smallestOverlap = overlap;
				smallestAxis = axis;
			}
			return true;
			};

		for (int i = 0; i < 4; ++i) {
			glm::vec2 edge = glm::vec2(q1.worldVerts[(i + 1) % 4]) - glm::vec2(q1.worldVerts[i]);
			glm::vec2 axisUnnorm = glm::vec2(-edge.y, edge.x);
			if (!testAxis(axisUnnorm)) return false;
		}
		for (int i = 0; i < 4; ++i) {
			glm::vec2 edge = glm::vec2(q2.worldVerts[(i + 1) % 4]) - glm::vec2(q2.worldVerts[i]);
			glm::vec2 axisUnnorm = glm::vec2(-edge.y, edge.x);
			if (!testAxis(axisUnnorm)) return false;
		}

		if (smallestOverlap == FLT_MAX) return false;

		glm::vec2 dir = glm::vec2(q2.pos.x - q1.pos.x, q2.pos.y - q1.pos.y);
		if (glm::dot(dir, smallestAxis) < 0.0f) smallestAxis = -smallestAxis;

		glm::vec2 correction = smallestAxis * smallestOverlap;

		if (q1.physicsType == STATIC && q2.physicsType == DYNAMIC)
			q2.pos += glm::vec3(correction, 0.0f);
		else if (q1.physicsType == DYNAMIC && q2.physicsType == STATIC)
			q1.pos -= glm::vec3(correction, 0.0f);
		else if (q1.physicsType == DYNAMIC && q2.physicsType == DYNAMIC) {
			q1.pos -= glm::vec3(correction * 0.5f, 0.0f);
			q2.pos += glm::vec3(correction * 0.5f, 0.0f);
		}

		return true;
	}

	CollisionResult Collision2D::CheckOBBCollisionResolve(Quad& q1, CarState& carState1, Quad& q2, CarState& carState2) {
		q1.CalculateWorldVerts(carState1.pos, glm::vec3(1.0f), carState1.rot);
		q2.CalculateWorldVerts(carState2.pos, glm::vec3(1.0f), carState2.rot);

		const float EPS = 1e-6f;
		float smallestOverlap = FLT_MAX;
		glm::vec2 smallestAxis(0.0f);

		auto testAxis = [&](const glm::vec2& axisUnnorm) -> bool {
			float axisLen = glm::length(axisUnnorm);
			if (axisLen < EPS) return true; // degenerate edge, skip axis

			glm::vec2 axis = axisUnnorm / axisLen; // normalized axis

			// project quad1
			float min1 = FLT_MAX, max1 = -FLT_MAX;
			for (auto& v : q1.worldVerts) {
				float proj = glm::dot(glm::vec2(v.x, v.y), axis);
				min1 = std::min(min1, proj);
				max1 = std::max(max1, proj);
			}

			// project quad2
			float min2 = FLT_MAX, max2 = -FLT_MAX;
			for (auto& v : q2.worldVerts) {
				float proj = glm::dot(glm::vec2(v.x, v.y), axis);
				min2 = std::min(min2, proj);
				max2 = std::max(max2, proj);
			}

			float overlap = std::min(max1, max2) - std::max(min1, min2);
			if (overlap <= 0.0f) return false;

			if (overlap < smallestOverlap) {
				smallestOverlap = overlap;
				smallestAxis = axis;
			}
			return true;
			};

		for (int i = 0; i < 4; ++i) {
			glm::vec2 edge = glm::vec2(q1.worldVerts[(i + 1) % 4]) - glm::vec2(q1.worldVerts[i]);
			glm::vec2 axisUnnorm = glm::vec2(-edge.y, edge.x);
			if (!testAxis(axisUnnorm)) return { false, glm::vec2(0.0f), 0.0f };
		}
		for (int i = 0; i < 4; ++i) {
			glm::vec2 edge = glm::vec2(q2.worldVerts[(i + 1) % 4]) - glm::vec2(q2.worldVerts[i]);
			glm::vec2 axisUnnorm = glm::vec2(-edge.y, edge.x);
			if (!testAxis(axisUnnorm)) return { false, glm::vec2(0.0f), 0.0f };
		}

		if (smallestOverlap == FLT_MAX) return { false, glm::vec2(0.0f), 0.0f };

		glm::vec2 dir = glm::vec2(carState2.pos.x - carState1.pos.x, carState2.pos.y - carState1.pos.y);
		if (glm::dot(dir, smallestAxis) < 0.0f) smallestAxis = -smallestAxis;

		return { true, smallestAxis, smallestOverlap };
	}

	bool Collision2D::CheckAABBCollsion(Quad& q1, Quad& q2) {
		bool x = q1.size.x + q1.pos.x >= q2.pos.x && q2.size.x + q2.pos.x >= q1.pos.x;
		bool y = q1.size.y + q1.pos.y >= q2.pos.y && q2.size.y + q2.pos.y >= q1.pos.y;
		return x && y;
	}
}