#ifndef QUAD_HPP
#define QUAD_HPP

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
	std::vector<glm::vec3> worldVerts; // for collision detection
	PhysicsType physicsType;

    // Default constructor
    Quad() : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(STATIC) {}

    Quad(PhysicsType type) : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(type) {}

    Quad(glm::vec3 pos, glm::vec3 size, float rot, PhysicsType type)
        : pos(pos), size(size), rot(rot), worldVerts(noVertices, glm::vec3(0.0f)), physicsType(type) {
    }

    void calculateWorldVerts(glm::vec3 pos, float rot) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(444, 208, 1.0f) * size);
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
