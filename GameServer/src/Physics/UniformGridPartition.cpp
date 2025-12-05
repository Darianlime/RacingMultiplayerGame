#include "UniformGridPartition.h"

UniformPartition::UniformPartition(float tileWidth, float tileHeight, uint32_t numberOfTiles)
	: tileWidth(tileWidth), tileHeight(tileHeight), numberOfTiles(numberOfTiles), capacity(32) {
	pool.resize(numberOfTiles * capacity);
	tileStart.resize(numberOfTiles);
	tileStart[0] = 1;
	for (uint32_t i = 1; i < numberOfTiles; i++) {
		tileStart[i] = capacity * i;
	}
	tileCount.resize(numberOfTiles);
}

bool UniformPartition::Insert(float x, float y, uint8_t carID, uint16_t& queryIndex) {
	if (x < 0 || x > 64 * 11 * sqrt(numberOfTiles) || y > 0 || y < -64 * 11 * sqrt(numberOfTiles)) {
		queryIndex = std::numeric_limits<uint16_t>::max();
		return false;
	}
	uint16_t xIndex = floor(x / (64.0f * 11.0f));
	uint16_t yIndex = floor(-y / (64.0f * 11.0f));
	uint16_t tileIndex = xIndex + sqrt(numberOfTiles) * yIndex;
	pool[tileStart[tileIndex] + tileCount[tileIndex] - 1] = carID;
	tileCount[tileIndex]++;
	queryIndex = tileIndex;
	return true;
}

void UniformPartition::ClearMap() {
	for (uint16_t& count : tileCount) {
		count = 0;
	}
}