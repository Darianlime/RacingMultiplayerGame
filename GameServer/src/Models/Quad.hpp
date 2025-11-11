#ifndef QUAD_HPP
#define QUAD_HPP

enum PhysicsType {
    STATIC,
    DYNAMIC
};

struct Quad {
    static const int noVertices = 4;
    glm::vec3 pos;
    glm::vec3 size;
    float rot;
	std::vector<glm::vec3> worldVerts; // for collision detection
	PhysicsType physicsType;

    // Default constructor
    Quad() : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(STATIC) {}

    Quad(PhysicsType type) : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(type) {}

    Quad(glm::vec3 pos, glm::vec3 size, float rot, PhysicsType type)
        : pos(pos), size(size), rot(rot), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(type) {
    }
};

#endif // QUAD_HPP
