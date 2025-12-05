#ifndef QUAD_HPP
#define QUAD_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum PhysicsType {
    STATIC,
    DYNAMIC
};

struct Quad {
private:
    float vertices[32] = {
        // position             normals                 texcoord
        -0.5f,  0.5f, 0.0f,     0.0f,  0.0f, -1.0f,     0.0f, 1.0f,		// Top-left
        0.5f,  0.5f, 0.0f,		0.0f,  0.0f, -1.0f,     1.0f, 1.0f, 	// Top-right
        0.5f, -0.5f, 0.0f,		0.0f,  0.0f, -1.0f,     1.0f, 0.0f,	    // Bottom-right
        -0.5f, -0.5f, 0.0f,		0.0f,  0.0f, -1.0f,     0.0f, 0.0f		// Bottom-left
    };
public:
    static const int noVertices = 4;
    glm::vec3 pos;
    glm::vec3 size;
    float rot;
	std::vector<glm::vec3> worldVerts; // order: topL, topR, botR, botL
	PhysicsType physicsType;

    // Default constructor
    Quad() : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(STATIC) {}

    Quad(PhysicsType type) : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(type) {}

    Quad(glm::vec3 pos, glm::vec3 size, float rot, PhysicsType type)
        : pos(pos), size(size), rot(rot), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(type) {
    }

    // Manual Cords
    /*Quad(glm::vec3 topL, glm::vec3 topR, glm::vec3 botR, glm::vec3 botL)
        : size(1.0f), rot(0.0f), worldVerts({topL, topR, botR, botL}), physicsType(STATIC)
    {
        float x = (glm::abs(topR.x) + glm::abs(topL.x)) / 2.0f;
        float y = (glm::abs(botR.y) + glm::abs(topR.y)) / 2.0f;
        if (topR.x < 0 || topL.x < 0) {
            x = -x;
        }
        if (botR.y < 0 || topR.y < 0) {
            y = -y;
        }
        pos = { x, y, 0.0f };
    }*/

    bool ContainsPoint(glm::vec3 point) {
        bool isXInQuad = worldVerts[0].x <= point.x && worldVerts[2].x >= point.x;
        bool isYInQuad = worldVerts[0].y >= point.y && worldVerts[2].y <= point.y;
        return isXInQuad && isYInQuad;
    }

    void calculateWorldVerts(glm::vec3 size, float rotationOffset) {
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
    }

    void calculateWorldVerts(glm::vec3 pos, glm::vec3 size, float rot) {
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
    }
};

#endif // QUAD_HPP
