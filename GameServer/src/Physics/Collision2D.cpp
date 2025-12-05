#include "Collision2D.h"
#include <iostream>

Collision2D::Collision2D() {

}

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

	float smallestOverlap = FLT_MAX;
	glm::vec2 smallestAxis;

	auto testAxis = [&](const glm::vec2& axis) {
		float min1 = FLT_MAX;
		float max1 = -FLT_MAX;
		for (const auto& v : quad1->worldVerts) {
			float projection = glm::dot(glm::vec2(v), axis);
			min1 = std::min(min1, projection);
			max1 = std::max(max1, projection);
		}
		float min2 = FLT_MAX;
		float max2 = -FLT_MAX;
		for (const auto& v : quad2->worldVerts) {
			float projection = glm::dot(glm::vec2(v), axis);
			min2 = std::min(min2, projection);
			max2 = std::max(max2, projection);
		}

		float overlap = std::min(max1, max2) - std::max(min1, min2);
		if (overlap <= 0.0f) return false;

		if (overlap < smallestOverlap) {
			smallestOverlap = overlap;
			smallestAxis = axis;
		}
		return true;
	};

	// Test all axes from both quads
	for (int i = 0; i < 4; ++i) {
		glm::vec2 edge = glm::vec2(quad1->worldVerts[(i + 1) % 4] - quad1->worldVerts[i]);
		if (!testAxis(glm::normalize(glm::vec2(-edge.y, edge.x)))) return false;
	}
	for (int i = 0; i < 4; ++i) {
		glm::vec2 edge = glm::vec2(quad2->worldVerts[(i + 1) % 4] - quad2->worldVerts[i]);
		if (!testAxis(glm::normalize(glm::vec2(-edge.y, edge.x)))) return false;
	}

	glm::vec2 direction = glm::vec2(quad2->pos - quad1->pos);
	if (glm::dot(direction, smallestAxis) < 0) smallestAxis = -smallestAxis;

	glm::vec2 correction = smallestAxis * smallestOverlap;

	// =====SEND PACKETS BACK TO AFFECTED QUAD=====
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

bool Collision2D::CheckAABBCollsion(Quad& q1, Quad& q2) {
	bool x = q1.size.x + q1.pos.x >= q2.pos.x && q2.size.x + q2.pos.x >= q1.pos.x;
	bool y = q1.size.y + q1.pos.y >= q2.pos.y && q2.size.y + q2.pos.y >= q1.pos.y;
	return x && y;
}

