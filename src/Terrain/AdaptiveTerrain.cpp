#include "AdaptiveTerrain.h"

namespace Terrain {
	DRReturn AdaptiveTerrain::load(const char* heightMapFileName)
	{
		mLoadingTimeSum.reset();
		return DR_OK;
	}
	void AdaptiveTerrain::exit()
	{

	}
	DRReturn AdaptiveTerrain::Render()
	{
		return DR_OK;
	}
}