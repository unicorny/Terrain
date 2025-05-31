#ifndef __TERRAIN_GRID_DIMENSIONS_H
#define __TERRAIN_GRID_DIMENSIONS_H

#include "DRCore2/Foundation/DRVector2.h"

namespace Terrain {
	struct GridDimensions
	{
		GridDimensions(DRVector2 _size, DRVector2 _stepSize)
		: size(_size), stepSize(_stepSize), stepsCount(_size / _stepSize) {}
		~GridDimensions() {}

		DRVector2 size;
		DRVector2 stepSize;
		DRVector2i stepsCount;
	};
}

#endif //__TERRAIN_GRID_DIMENSIONS_H