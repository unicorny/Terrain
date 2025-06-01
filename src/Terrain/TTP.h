#ifndef __TERRAIN_TRIBASE_TERRAIN_TTP_FILE_H
#define __TERRAIN_TRIBASE_TERRAIN_TTP_FILE_H

namespace Terrain {
	// Size are one Dimension, quadratic texture, to get full size multiply 
	struct TTPHeader {
		TTPHeader() :
			heightMapSize(0),
			colorMapSize(0),
			detailMapSize(0),
			normalMapSize(0),
			lightMapSize(0),
			terrainSize(0.0f),
			terrainHeight(0.0f),
			terrainTilling(0.0f)
		{}

		int heightMapSize;
		int colorMapSize;
		int detailMapSize;
		int normalMapSize;
		int lightMapSize;
		float terrainSize;
		float terrainHeight;
		float terrainTilling;

		static TTPHeader loadFromFile(const char* fileName);
	};
}

#endif // __TERRAIN_TRIBASE_TERRAIN_TTP_FILE_H