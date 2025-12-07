#ifndef WORLD_MAP_H
#define WORLD_MAP_H

#include <vector>
#include <string>

namespace Server {
	class WorldMap {
	public:
		enum Type : uint8_t {
			NONE,
			STRAIGHT_VERTICAL, // │
			STRAIGHT_HORIZONTAL, // ─
			CORNER_TR, // ┐
			CORNER_BL,//└
			CORNER_BR, // ┘
			CORNER_TL // ┌
		};

		WorldMap();
		void createMap();

		const std::vector<uint8_t>& GetMap() const {
			return tileMap;
		}

		uint8_t GetRow() const { return row; }
		uint8_t GetColumn() const { return column; }
	private:
		std::vector<uint8_t> tileMap;
		uint8_t row;
		uint8_t column;
	};
}

#endif // !WORLD_MAP_H