#ifndef CUBE_HPP
#define CUBE_HPP

#include "../Model.h"

enum PhysicsType {
    STATIC,
    DYNAMIC
};

class Quad : public Model {
private:
    float vertices[32] = {
        // position             normals                 texcoord
        -0.5f,  0.5f, 0.0f,     0.0f,  0.0f, -1.0f,     0.0f, 1.0f,		// Top-left
        0.5f,  0.5f, 0.0f,		0.0f,  0.0f, -1.0f,     1.0f, 1.0f, 	// Top-right
        0.5f, -0.5f, 0.0f,		0.0f,  0.0f, -1.0f,     1.0f, 0.0f,	    // Bottom-right
        -0.5f, -0.5f, 0.0f,		0.0f,  0.0f, -1.0f,     0.0f, 0.0f		// Bottom-left
    };
public:
    glm::vec3 pos;
    glm::vec3 size;
    float rot;
    std::vector<const char*> textureImageNames;
    std::vector<Texture> textures;
	std::vector<glm::vec3> worldVerts; // for collision detection
	PhysicsType physicsType;
    glm::mat4 model;

    // Default constructor
    Quad() : pos(0.0f), size(1.0f), rot(0.0f), textureImageNames(textureImageNames), physicsType(STATIC) {}

    Quad(PhysicsType type) : pos(0.0f), size(1.0f), rot(0.0f), textureImageNames(textureImageNames), physicsType(type) {}

    Quad(std::vector<const char*> textureImageNames, PhysicsType type) : pos(0.0f), size(1.0f), rot(0.0f), textureImageNames(textureImageNames) , physicsType(type) {}

    Quad(glm::vec3 pos, glm::vec3 size, float rot, std::vector<const char*> textureImageNames, PhysicsType type)
        : pos(pos), size(size), rot(rot), textureImageNames(textureImageNames) , physicsType(type) {
    }

    void init() {
        int noVertices = 4; // was 6, but only 4 vertex entries are provided
        
        for (int i = 0; i < noVertices; i++) {
            worldVerts.push_back(glm::vec3(0.0f,0.0f,0.0f));
        }
        std::vector<unsigned int> indices = {0, 1, 3, 1, 2, 3};
        loadTextures();

        meshes.push_back(Mesh(Vertex::genList(vertices, noVertices), indices, textures));
    }

    void loadTextures() {
        for (int i = 0; i < textureImageNames.size(); i++) {
            char name[9];
			sprintf_s(name, sizeof(name), "texture%d", i);
            printf("%s", name);
			textures.push_back(Texture(textureImageNames[i], name));
            textures[i].load();
        }
	}

    void render(Shader shader) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(textures[0].getWidth(), textures[0].getHeight(), 1.0f) * size);
        shader.setMat4("model", model);
        for (int i = 0; i < 4; i++) {
			glm::vec3 localPos = glm::vec3(vertices[i * 8 + 0], vertices[i * 8 + 1], vertices[i * 8 + 2]);
            glm::vec4 worldPos = model * glm::vec4(localPos, 1.0f);
			worldVerts[i] = glm::vec3(worldPos);
			//std::cout << "Vertex " << i << " world position: (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")" << std::endl;
        }
        Model::render(shader);
    }
};

#endif // CUBE_HPP
