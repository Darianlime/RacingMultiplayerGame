#ifndef QUAD_HPP
#define QUAD_HPP

#include "../Model.h"
#include <Network/Packets.h>

using namespace NetworkClient;
namespace Engine {
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
        static const int noVertices = 4;
        glm::vec3 pos;
        glm::vec3 size;
        float rot;
        std::vector<const char*> textureImageNames;
        std::vector<Texture> textures;
        std::vector<glm::vec3> worldVerts; // for collision detection
        PhysicsType physicsType;
        std::vector<Vertex> vertslist;


        Quad() 
            : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), textureImageNames(textureImageNames), physicsType(STATIC) {
            vertslist = Vertex::genList(vertices, noVertices);
        }

        Quad(PhysicsType type) 
            : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), textureImageNames(textureImageNames), physicsType(type) {
            vertslist = Vertex::genList(vertices, noVertices);
        }

        Quad(std::vector<const char*> textureImageNames, PhysicsType type) 
            : pos(0.0f), size(1.0f), rot(0.0f), worldVerts(noVertices, glm::vec3(0.0f)), textureImageNames(textureImageNames), physicsType(type) {
            vertslist = Vertex::genList(vertices, noVertices);
        }

        Quad(glm::vec3 pos, float rot, std::vector<const char*> textureImageNames, PhysicsType type)
            : pos(pos), size(1.0f), rot(rot), worldVerts(noVertices, glm::vec3(0.0f)), textureImageNames(textureImageNames), physicsType(type) {
            vertslist = Vertex::genList(vertices, noVertices);
        }
        Quad(glm::vec3 pos, glm::vec3 size, float rot, std::vector<const char*> textureImageNames, PhysicsType type)
            : pos(pos), size(size), rot(rot), worldVerts(noVertices, glm::vec3(0.0f)), textureImageNames(textureImageNames), physicsType(type) {
            vertslist = Vertex::genList(vertices, noVertices);
        }

      /*  Quad(const Quad& other)
            : pos(other.pos), size(other.size), rot(other.rot), worldVerts(other.worldVerts), textureImageNames(other.textureImageNames),
            physicsType(other.physicsType), vertslist(other.vertslist) {
        }*/

        void initQuads() {
            std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };
            loadTextures();
            meshes.push_back(Mesh(vertslist, indices, textures));
        }

        void initQuads(std::vector<Vertex> verts) {
            std::vector<unsigned int> indices = { 0, 1, 3, 1, 2, 3 };
            loadTextures();
            meshes.push_back(Mesh(verts, indices, textures));
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
            for (int i = 0; i < 4; i++) {
                glm::vec3 localPos = glm::vec3(vertices[i * 8 + 0], vertices[i * 8 + 1], vertices[i * 8 + 2]);
                glm::vec4 worldPos = model * glm::vec4(localPos, 1.0f);
                worldVerts[i] = glm::vec3(worldPos);
                //std::cout << "Vertex " << i << " world position: (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")" << std::endl;
            }
            Model::render(shader);
        }

        void render(Shader shader, glm::vec3 pos, float rot) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos);
            model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(textures[0].getWidth(), textures[0].getHeight(), 1.0f) * size);
            //std::cout << "width " << textures[0].getWidth() << " Hiehgt " << textures[0].getHeight() << std::endl;
            //model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
            shader.setMat4("model", model);
            Model::render(shader);
        }

        void UpdateWorldVerts(const CarState& state) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, state.pos);
            model = glm::rotate(model, glm::radians(state.rot - 90.0f), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(textures[0].getWidth(), textures[0].getHeight(), 1.0f) * size);

            for (int i = 0; i < 4; i++) {
                glm::vec3 localPos(vertices[i * 8 + 0], vertices[i * 8 + 1], vertices[i * 8 + 2]);
                worldVerts[i] = glm::vec3(model * glm::vec4(localPos, 1.0f));
            }
        }
    };
}
#endif // QUAD_HPP
