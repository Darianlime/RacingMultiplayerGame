#ifndef TILE_H
#define TILE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

class Tile {
public:
	Tile(bool isStartTile);

private:
	bool isStartTile;
};

#endif // !WORLD_MAP_H