#ifndef QUAD_H
#define QUAD_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Core {
    enum PhysicsType {
        STATIC,
        DYNAMIC
    };

    class Quad {
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
        Quad();
        Quad(PhysicsType type);
        Quad(glm::vec3 pos, glm::vec3 size, float rot, PhysicsType type);

        float* GetVerts() {
            return vertices;
        }

        float GetVertexAt(int index) const {
            if (index >= 0 && index < 32) {
                return vertices[index];
            }
            return 0.0f;
        }

        bool ContainsPoint(glm::vec3 point);

        std::vector<glm::vec3> CalculateWorldVerts(glm::vec3 size, float rotationOffset);
        std::vector<glm::vec3> CalculateWorldVerts(glm::vec3 pos, glm::vec3 size, float rot);
    };
}

#endif // QUAD_H
