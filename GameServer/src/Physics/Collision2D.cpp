#include "Collision2D.h"

Collision2D::Collision2D() {

}

bool Collision2D::checkOBBCollision(Quad& q1, Quad& q2) {
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

bool Collision2D::checkOBBCollisionResolve(Quad& q1, Quad& q2) {
	Quad* quad1 = &q1;
	Quad* quad2 = &q2;

	float overlap = FLT_MAX;

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

		overlap = std::min(overlap, std::min(max1, max2) - std::max(min1, min2));

		if (max1 < min2 || max2 < min1) {
			return false; // No collision
		}
	}

	for (int i = 0; i < 4; i++) {
		glm::vec3 p1 = quad2->worldVerts[i];
		glm::vec3 p2 = quad2->worldVerts[(i + 1) % 4];
		glm::vec3 edge = p2 - p1;
		glm::vec3 axis = glm::normalize(glm::vec3(-edge.y, edge.x, 0.0f));
		float min1 = FLT_MAX;
		float max1 = -FLT_MAX;
		for (const auto& v : quad2->worldVerts) {
			float projection = glm::dot(v, axis);
			min1 = std::min(min1, projection);
			max1 = std::max(max1, projection);
		}
		float min2 = FLT_MAX;
		float max2 = -FLT_MAX;
		for (const auto& v : quad1->worldVerts) {
			float projection = glm::dot(v, axis);
			min2 = std::min(min2, projection);
			max2 = std::max(max2, projection);
		}
	}

	// =====SEND PACKETS BACK TO AFFECTED QUAD=====
	if (quad1->physicsType == STATIC && quad2->physicsType == DYNAMIC) {
		glm::vec3 direction = quad1->pos - quad2->pos;
		float s = glm::length(direction);
		quad2->pos -= (overlap + 0.01f) * (direction / s);
	} else if (quad1->physicsType == DYNAMIC && quad2->physicsType == STATIC) {
		glm::vec3 direction = quad2->pos - quad1->pos;
		float s = glm::length(direction);
		quad1->pos -= (overlap + 0.01f) * (direction / s);
	} else if (quad1->physicsType == DYNAMIC && quad2->physicsType == DYNAMIC) {
		glm::vec3 direction = quad2->pos - quad1->pos;
		float s = glm::length(direction);
		quad1->pos -= (overlap + 0.01f) * (direction / s);
		glm::vec3 direction1 = quad1->pos - quad2->pos;
		float s1 = glm::length(direction1);
		quad2->pos -= (overlap + 0.01f) * (direction1 / s1);
	}
	return true;
}
