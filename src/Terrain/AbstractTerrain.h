#ifndef __TERRAIN_ABSTRACT_TERRAIN_H
#define __TERRAIN_ABSTRACT_TERRAIN_H

#include "DRCore2/DRTypes.h"
#include "QuadraticGridLogic.h"
#include <atomic>

namespace Terrain {
	class AbstractTerrain
	{
	public:
		AbstractTerrain(const QuadraticGridLogic& grid) : mGrid(grid), mIsLoaded(false) {}
		virtual ~AbstractTerrain() {}
		virtual DRReturn load(const char* heightMapFileName) = 0;
		virtual void exit() = 0;
		virtual DRReturn Render() = 0;

		inline bool isLoaded() const { return mIsLoaded; }

	protected:
		QuadraticGridLogic mGrid;
		std::atomic<bool> mIsLoaded;		
	};
}

#endif //__TERRAIN_ABSTRACT_TERRAIN_H