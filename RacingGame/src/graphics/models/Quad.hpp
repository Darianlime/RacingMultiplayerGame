#ifndef CUBE_HPP
#define CUBE_HPP

#include "../Model.h"

class Quad : public Model {
public:
    glm::vec3 pos;
    glm::vec3 size;
	float rot;
    std::vector<const char*> textureImageNames;
    std::vector<Texture> textures;

    Quad(glm::vec3 pos, glm::vec3 size, float rot, std::vector<const char*> textureImageNames)
        : pos(pos), size(size), rot(rot), textureImageNames(textureImageNames) {
    }

    void init() {
        int noVertices = 4; // was 6, but only 4 vertex entries are provided
        float vertices[] = {
            // position             normals                 texcoord
            -0.5f,  0.5f, 0.0f,     0.0f,  0.0f, -1.0f,     0.0f, 1.0f,		// Top-left
            0.5f,  0.5f, 0.0f,		0.0f,  0.0f, -1.0f,     1.0f, 1.0f, 	// Top-right
            0.5f, -0.5f, 0.0f,		0.0f,  0.0f, -1.0f,     1.0f, 0.0f,	    // Bottom-right
            -0.5f, -0.5f, 0.0f,		0.0f,  0.0f, -1.0f,     0.0f, 0.0f		// Bottom-left
        };

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
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(textures[0].getWidth(), textures[0].getHeight(), 1.0f) * size);
        //model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
        shader.setMat4("model", model);

        Model::render(shader);
    }
};

#endif // CUBE_HPP
