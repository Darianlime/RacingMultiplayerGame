#include "pch.h"
#include "graphics/models/QuadRender.h"

namespace Client {
    QuadRender::QuadRender()
        : quad(glm::vec3(0.0f), glm::vec3(1.0f), 0.0f, STATIC), textureImageNames({}) {
        vertslist = Vertex::GenList(quad.GetVerts(), quad.noVertices);
    }

    QuadRender::QuadRender(PhysicsType type)
        : quad(glm::vec3(0.0f), glm::vec3(1.0f), 0.0f, STATIC), textureImageNames({}) {
        vertslist = Vertex::GenList(quad.GetVerts(), quad.noVertices);
    }

    QuadRender::QuadRender(std::vector<const char*> textureImageNames, PhysicsType type)
        : quad(glm::vec3(0.0f), glm::vec3(1.0f), 0.0f, type), textureImageNames(textureImageNames) {
        vertslist = Vertex::GenList(quad.GetVerts(), quad.noVertices);
    }

    QuadRender::QuadRender(glm::vec3 pos, float rot, std::vector<const char*> textureImageNames, PhysicsType type)
        : quad(pos, glm::vec3(1.0f), rot, type), textureImageNames(textureImageNames) {
        vertslist = Vertex::GenList(quad.GetVerts(), quad.noVertices);
    }
    QuadRender::QuadRender(glm::vec3 pos, glm::vec3 size, float rot, std::vector<const char*> textureImageNames, PhysicsType type)
        : quad(pos, size, rot, type), textureImageNames(textureImageNames) {
        vertslist = Vertex::GenList(quad.GetVerts(), quad.noVertices);
    }

    void QuadRender::InitQuads() {
        std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };
        LoadTextures();
        meshes.push_back(Mesh(vertslist, indices, textures));
    }

    void QuadRender::InitQuads(std::vector<Vertex> verts) {
        std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };
        LoadTextures();
        meshes.push_back(Mesh(verts, indices, textures));
    }

    void QuadRender::LoadTextures() {
        for (int i = 0; i < textureImageNames.size(); i++) {
            char name[9];
            sprintf_s(name, sizeof(name), "texture%d", i);
            printf("%s", name);
            textures.push_back(Texture(textureImageNames[i], name));
            textures[i].load();
        }
    }

    void QuadRender::Render(Shader shader) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, quad.pos);
        model = glm::rotate(model, glm::radians(quad.rot), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, quad.size);

        shader.setMat4("model", model);
        Model::Render(shader);
    }

    void QuadRender::Render(Shader shader, glm::vec3 pos, float rot) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, quad.size);

        shader.setMat4("model", model);
        Model::Render(shader);
    }

    void QuadRender::Render(Shader shader, glm::vec3 pos, glm::vec3 size, float rot) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, quad.size * size);

        shader.setMat4("model", model);
        Model::Render(shader);
    }
}