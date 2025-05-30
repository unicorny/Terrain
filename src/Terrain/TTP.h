#ifndef __TERRAIN_TRIBASE_TERRAIN_TTP_FILE_H
#define __TERRAIN_TRIBASE_TERRAIN_TTP_FILE_H

namespace Terrain {
	// Size are one Dimension, quadratic texture, to get full size multiply 
	struct TTPHeader {
		int heightMapSize;
		int colorMapSize;
		int detailMapSize;
		int normalMapSize;
		int lightMapSize;
		float terrainSize;
		float terrainHeight;
		float terrainTilling;
	};

	TTPHeader loadTTPHeaderFromFile(const char* fileName);
}

#endif // __TERRAIN_TRIBASE_TERRAIN_TTP_FILE_H