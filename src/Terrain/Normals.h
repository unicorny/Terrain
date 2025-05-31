#ifndef __TERRAIN_NORMALS_H
#define __TERRAIN_NORMALS_H

#include "HeightMap.h"
#include "GridDimensions.h"

#include "DRCore2/Threading/DRCPUTask.h"
#include "DRCore2/Threading/DRCPUScheduler.h"
#include "DRCore2/Foundation/DRVector3.h"
#include "DRCore2/Foundation/DRVector2.h"

#include <vector>
#include <memory>
#include <cassert>

namespace Terrain {

	class CalculateNormalsTask : public DRCPUTask
	{
	public:
		CalculateNormalsTask(
			DRCPUScheduler* scheduler, 
			GridDimensions grid,
			DRVector3* normalsBuffer,
			std::shared_ptr<CollectInterpolatedHeights> interpolatedHeights
		) : DRCPUTask(scheduler, 1), mGrid(grid), mNormalsBuffer(normalsBuffer), mInterpolatedHeights(interpolatedHeights)
		{
			setParentTaskPtrInArray(mInterpolatedHeights, 0);
		}
		virtual ~CalculateNormalsTask() {}

		DRReturn run();
		virtual const char* getResourceType() const { return "CalculateNormalsTask"; }
	protected:
		GridDimensions mGrid;
		std::shared_ptr<CollectInterpolatedHeights> mInterpolatedHeights;
		DRVector3* mNormalsBuffer;
	};

}

#endif // __TERRAIN_NORMALS_H