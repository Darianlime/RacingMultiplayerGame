#ifndef UNIFORM_GRID_PARTITION_H
#define UNIFORM_GRID_PARTITION_H

#include <vector>
#include <map>
#include <limits>
#include "Quad.h"

namespace Core {
	class UniformPartition {
	private:
		float tileWidth;
		float tileHeight;
		uint32_t numberOfTiles;
		uint32_t capacity;
	public:
		std::vector<uint8_t> pool;
		std::vector<uint32_t> tileStart;
		std::vector<uint16_t> tileCount;

		UniformPartition() = default;
		UniformPartition(float tileWidth, float tileHeight, uint32_t numberOfTiles);
		bool Insert(float x, float y, uint8_t carID, uint16_t& queryIndex);
		uint32_t GetPoolStart(uint16_t tileIndex) const { return tileStart[tileIndex] - 1; }
		void ClearMap();
	};
}
#endif // !UNIFORM_GRID_PARTITION_H