#ifndef RACE_TRACK_RENDER_H
#define RACE_TRACK_RENDER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <map>
#include "Quad.hpp"

namespace Engine {
	class RaceTrackRender {
	public:
		enum Type : uint8_t {
			NONE,
			STRAIGHT_VERTICAL, // │
			STRAIGHT_HORIZONTAL, // ─
			CORNER_TR, // ┐
			CORNER_BL,// └
			CORNER_BR, // ┘
			CORNER_TL // ┌
		};

		struct TileProperties {
			const char* tileName;
			float rotation;
			bool isStartLine = false;
		};

		std::map<uint8_t, TileProperties> tileData = {
			{STRAIGHT_VERTICAL, {"assets/Road_Straight.png", 0.0f}},
			{STRAIGHT_HORIZONTAL, {"assets/Road_Straight.png", 90.0f}},
			{CORNER_TL, {"assets/Road_Turn.png", 0.0f}},
			{CORNER_TR, {"assets/Road_Turn.png", -90.0f}},
			{CORNER_BR, {"assets/Road_Turn.png", -180.0f}},
			{CORNER_BL, {"assets/Road_Turn.png", -270.0f}},
			{NONE, {"assets/Grass.png", 0.0f}}
		};

		RaceTrackRender();
		void Init(std::vector<uint8_t> map, uint8_t row, uint8_t column);
		void Render(Shader shader);
	private:
		std::vector<uint8_t> map;
		uint8_t row, column;
		std::vector<Quad> tiles;
	};
}
#endif // !RACE_TRACK_RENDER_H
