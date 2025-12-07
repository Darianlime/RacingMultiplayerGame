#include "pch.h"
#include "Physics/QuadTree.h"

namespace Core {
	QuadTree::QuadTree(Quad bounds, uint8_t capcity)
		: bounds(bounds), capcity(capcity) {
		this->topLTree = nullptr;
		this->topRTree = nullptr;
		this->botRTree = nullptr;
		this->botLTree = nullptr;
	}

	void QuadTree::Subdivide() {
		glm::vec3 quadPos((bounds.worldVerts[0].x + bounds.pos.x) / 2.0f, (bounds.worldVerts[0].y + bounds.pos.y) / 2.0f, 0.0f);
		glm::vec3 quadSize(bounds.size.x / 2.0f, bounds.size.y / 2.0f, 1.0f);
		Quad topLQuad(quadPos, quadSize, 0.0f, PhysicsType::STATIC);
		topLQuad.CalculateWorldVerts(topLQuad.pos, glm::vec3(1.0f), 0.0f);
		Quad topRQuad(glm::vec3(quadPos.x + bounds.pos.x, quadPos.y, 0.0f), quadSize, 0.0f, PhysicsType::STATIC);
		topRQuad.CalculateWorldVerts(topRQuad.pos, glm::vec3(1.0f), 0.0f);
		Quad botRQuad(glm::vec3(quadPos.x + bounds.pos.x, quadPos.y + bounds.pos.y, 0.0f), quadSize, 0.0f, PhysicsType::STATIC);
		botRQuad.CalculateWorldVerts(botRQuad.pos, glm::vec3(1.0f), 0.0f);
		Quad botLQuad(glm::vec3(quadPos.x, quadPos.y + bounds.pos.y, 0.0f), quadSize, 0.0f, PhysicsType::STATIC);
		botLQuad.CalculateWorldVerts(botLQuad.pos, glm::vec3(1.0f), 0.0f);

		this->topLTree = std::make_unique<QuadTree>(topLQuad, capcity);
		this->topRTree = std::make_unique<QuadTree>(topRQuad, capcity);
		this->botRTree = std::make_unique<QuadTree>(botRQuad, capcity);
		this->botLTree = std::make_unique<QuadTree>(botLQuad, capcity);
		isDivided = true;
	}

	bool QuadTree::Insert(glm::vec3 point) {
		if (!bounds.ContainsPoint(point)) {
			return false;
		}

		if (this->points.size() < this->capcity) {
			this->points.push_back(point);
			return true;
		}
		else {
			if (!isDivided) {
				Subdivide();
			}
			if (topLTree.get()->Insert(point) || topRTree.get()->Insert(point) || botRTree.get()->Insert(point) || botLTree.get()->Insert(point)) {
				return true;
			}
		}
		return false;
		//if (topLTree == nullptr || topRTree == nullptr || botRTree == nullptr || botLTree == nullptr) return;

		//return points;
	}
}