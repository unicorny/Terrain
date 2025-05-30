#ifndef __TERRAIN_PIXEL_MAP_H
#define __TERRAIN_PIXEL_MAP_H

#include "DRCore2/DRTypes.h"
#include "DRCore2/Foundation/DRColor.h"
#include <vector>


struct ColorMap {
	u16 width;
	u16 height;
	std::vector<DRColor> map;
};

#endif