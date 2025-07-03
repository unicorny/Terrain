#ifndef __TERRAIN_ADAPTIVE_TERRAIN_H
#define __TERRAIN_ADAPTIVE_TERRAIN_H

#include "AbstractTerrain.h"

#include "DRCore2/Foundation/DRVector3.h"

#include <vector>
#include <memory>

namespace Terrain {

	class AdaptiveTerrain: public AbstractTerrain
	{
	public:
		using AbstractTerrain::AbstractTerrain;

		virtual DRReturn load(const char* heightMapFileName);
		virtual void exit();
		virtual DRReturn Render();

	protected:


	};	

	class AdaptiveTerrainQuad
	{
	public:
		AdaptiveTerrainQuad();
		virtual ~AdaptiveTerrainQuad();

	protected:		
		size_t mIndex;
		std::vector<DRVector3> mPositions;
		std::array<std::unique_ptr<AdaptiveTerrainQuad>, 4> mChilds;
	};
}

#endif // __TERRAIN_ADAPTIVE_TERRAIN_H