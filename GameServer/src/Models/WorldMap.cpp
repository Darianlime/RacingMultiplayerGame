#include "WorldMap.h"

WorldMap::WorldMap() : 
	row(3), 
	column(3),
	tileMap{
		CORNER_TL, STRAIGHT_HORIZONTAL , CORNER_TR,
		STRAIGHT_VERTICAL, NONE , STRAIGHT_VERTICAL,
		CORNER_BL, STRAIGHT_HORIZONTAL , CORNER_BR
	}
{}

void WorldMap::createMap() {
}