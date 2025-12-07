#ifndef QUAD_RENDER_H
#define QUAD_RENDER_H

#include "../Model.h"
#include "Core/Physics/Quad.h"
#include "Core/Physics/CarPhysics.h"

namespace Client {
    using namespace Core;

    class QuadRender : public Model {
    private:
        Quad quad;
    public:
        std::vector<const char*> textureImageNames;
        std::vector<Texture> textures;
        std::vector<Vertex> vertslist;

        QuadRender();
        QuadRender(PhysicsType type);
        QuadRender(std::vector<const char*> textureImageNames, PhysicsType type);
        QuadRender(glm::vec3 pos, float rot, std::vector<const char*> textureImageNames, PhysicsType type);
        QuadRender(glm::vec3 pos, glm::vec3 size, float rot, std::vector<const char*> textureImageNames, PhysicsType type);

        void InitQuads();
        void InitQuads(std::vector<Vertex> verts);

        Quad& GetQuad() { return quad; }

        void LoadTextures();

        glm::vec3 GetTextureSize() const {
            return glm::vec3(textures[0].getWidth(), textures[0].getHeight(), 1.0f);
        }

        void Render(Shader shader);
        void Render(Shader shader, glm::vec3 pos, float rot);
        void Render(Shader shader, glm::vec3 pos, glm::vec3 size, float rot);
    };
}
#endif // QUAD_RENDER_HPP
