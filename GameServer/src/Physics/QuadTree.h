#ifndef QUADTREE_H
#define QUADTREE_H

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include "../Models/Quad.hpp"
#include <memory>

class QuadTree {
private:
	std::vector<glm::vec3> points;
	Quad bounds;
	uint8_t capcity;
	bool isDivided = false;
	std::unique_ptr<QuadTree> topLTree;
	std::unique_ptr<QuadTree> topRTree;
	std::unique_ptr<QuadTree> botRTree;
	std::unique_ptr<QuadTree> botLTree;
public:
	QuadTree() = default;
	QuadTree(Quad bounds, uint8_t capcity);
	void Subdivide();
	bool Insert(glm::vec3 point);
};

#endif // !QUADTREE_H