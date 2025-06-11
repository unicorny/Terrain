#ifndef __TERRAIN_ADAPTIVE_TERRAIN_H
#define __TERRAIN_ADAPTIVE_TERRAIN_H

#include "AbstractTerrain.h"

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
}

#endif // __TERRAIN_ADAPTIVE_TERRAIN_H