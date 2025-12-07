#include "pch.h"
#include "graphics/models/RaceTrackRender.h"

namespace Client {
	RaceTrackRender::RaceTrackRender() : map{}, row(0), column(0) {
		
	}

	void RaceTrackRender::Init(std::vector<uint8_t> map, uint8_t row, uint8_t column) {
		this->map = map;
		this->row = row;
		this->column = column;
		int offsetX = 1;
		int offsetY = 1;
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < column; j++) {
				int index = i * column + j;
				TileProperties& prop = tileData.at(map[index]);
				std::vector<const char*> textureNames = { prop.tileName };
				glm::vec3 size(11.0f, 11.0f, 0.0f);
				float x = (j + offsetX) * (64.0f * size.x / 2);
				float y = (i + offsetY) * -(64.0f * size.y / 2);
				tiles.push_back(QuadRender(glm::vec3(x, y, 0.0f), size, prop.rotation, textureNames, STATIC));
				offsetX += 1;
				tiles[index].InitQuads();
				tiles[index].GetQuad().size = tiles[index].GetTextureSize() * size;
				tiles[index].GetQuad().CalculateWorldVerts(glm::vec3(x, y, 0.0f), size, prop.rotation);
			}
			offsetX = 1;
			offsetY += 1;
		}
	}

	void RaceTrackRender::Render(Shader shader) {
		for (int i = 0; i < tiles.size(); i++) {
			tiles[i].Render(shader);
		}
	}
}