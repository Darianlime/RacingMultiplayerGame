#include "pch.h"
#include "Physics/Quad.h"

namespace Core {
    Quad::Quad() : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(STATIC) {}

    Quad::Quad(PhysicsType type) : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(type) {}

    Quad::Quad(glm::vec3 pos, glm::vec3 size, float rot, PhysicsType type)
        : pos(pos), size(size), rot(rot), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(type) {
    }

    bool Quad::ContainsPoint(glm::vec3 point) {
        bool isXInQuad = worldVerts[0].x <= point.x && worldVerts[2].x >= point.x;
        bool isYInQuad = worldVerts[0].y >= point.y && worldVerts[2].y <= point.y;
        return isXInQuad && isYInQuad;
    }

    std::vector<glm::vec3> Quad::CalculateWorldVerts(glm::vec3 size, float rotationOffset) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(rot + rotationOffset), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f) * this->size);
        //model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
        for (int i = 0; i < 4; i++) {
            glm::vec3 localPos = glm::vec3(vertices[i * 8 + 0], vertices[i * 8 + 1], vertices[i * 8 + 2]);
            glm::vec4 worldPos = model * glm::vec4(localPos, 1.0f);
            worldVerts[i] = glm::vec3(worldPos);
            //std::cout << "Vertex " << i << " world position: (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")" << std::endl;
        }
        return worldVerts;
    }

    std::vector<glm::vec3> Quad::CalculateWorldVerts(glm::vec3 pos, glm::vec3 size, float rot) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f) * this->size);
        //model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
        for (int i = 0; i < 4; i++) {
            glm::vec3 localPos = glm::vec3(vertices[i * 8 + 0], vertices[i * 8 + 1], vertices[i * 8 + 2]);
            glm::vec4 worldPos = model * glm::vec4(localPos, 1.0f);
            worldVerts[i] = glm::vec3(worldPos);
            //std::cout << "Vertex " << i << " world position: (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")" << std::endl;
        }
        return worldVerts;
    }
}