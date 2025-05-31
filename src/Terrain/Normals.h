#ifndef __TERRAIN_NORMALS_H
#define __TERRAIN_NORMALS_H

#include "HeightMap.h"

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
			DRVector2 size,
			DRVector2 stepSize,
			std::shared_ptr<CollectInterpolatedHeights> interpolatedHeights
		) : DRCPUTask(scheduler, 1), mSize(size), mStepSize(stepSize), mInterpolatedHeights(interpolatedHeights)
		{
			setParentTaskPtrInArray(mInterpolatedHeights, 0);
		}
		virtual ~CalculateNormalsTask() {}

		DRReturn run();
		inline std::shared_ptr<std::vector<DRVector3>> getResult() { assert(isTaskFinished()); return mNormals; }
		virtual const char* getResourceType() const { return "CalculateNormalsTask"; }
	protected:
		DRVector2 mSize;
		DRVector2 mStepSize;
		std::shared_ptr<CollectInterpolatedHeights> mInterpolatedHeights;
		std::shared_ptr<std::vector<DRVector3>> mNormals;
	};

}

#endif // __TERRAIN_NORMALS_H