#include "graphics/models/RaceTrackRender.h"

namespace Engine {
	RaceTrackRender::RaceTrackRender() : map{}, row(0), column(0) {
		
	}

	void RaceTrackRender::Init(std::vector<uint8_t> map, uint8_t row, uint8_t column) {
		this->map = map;
		this->row = row;
		this->column = column;
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < column; j++) {
				int index = i * column + j;
				TileProperties& prop = tileData.at(map[index]);
				glm::vec3 size = glm::vec3(11.0f, 11.0f, 0.0f);
				std::vector<const char*> textureNames = { prop.tileName };
				float x = j * 64.0f * size.x;
				float y = i * 64.0f * size.y;
				tiles.push_back(Quad(glm::vec3(x, -y, 0.0f), size, prop.rotation, textureNames, STATIC));
				tiles[index].initQuads();
			}
		}
	}

	void RaceTrackRender::Render(Shader shader) {
		for (int i = 0; i < tiles.size(); i++) {
			tiles[i].render(shader);
		}
	}
}