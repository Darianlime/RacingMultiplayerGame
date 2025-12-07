#include "pch.h"
#include "WorldMap.h"

namespace Server {
	WorldMap::WorldMap() :
		row(4),
		column(4),
		tileMap{
			CORNER_TL, STRAIGHT_HORIZONTAL , STRAIGHT_HORIZONTAL, CORNER_TR,
			STRAIGHT_VERTICAL, NONE ,        NONE,				  STRAIGHT_VERTICAL,
			STRAIGHT_VERTICAL, NONE ,		 NONE,			      STRAIGHT_VERTICAL,
			CORNER_BL, STRAIGHT_HORIZONTAL , STRAIGHT_HORIZONTAL, CORNER_BR
		}
	{
	}

	void WorldMap::createMap() {
	}
}