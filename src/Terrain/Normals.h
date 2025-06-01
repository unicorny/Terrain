#ifndef __TERRAIN_NORMALS_H
#define __TERRAIN_NORMALS_H

#include "../GPU/DRGLBufferStatic.h"

#include "HeightMap.h"
#include "QuadraticGridLogic.h"

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
			QuadraticGridLogic grid,
			std::shared_ptr<GPU::DRGLBufferStatic> normalsBuffer,
			std::shared_ptr<CollectInterpolatedHeights> interpolatedHeights
		) : DRCPUTask(scheduler, 1), mGrid(grid), mNormalsBuffer(normalsBuffer), mInterpolatedHeights(interpolatedHeights)
		{
			assert(normalsBuffer);
			setParentTaskPtrInArray(mInterpolatedHeights, 0);
		}
		virtual ~CalculateNormalsTask() {}

		DRReturn run();
		virtual const char* getResourceType() const { return "CalculateNormalsTask"; }
	protected:
		QuadraticGridLogic mGrid;
		std::shared_ptr<CollectInterpolatedHeights> mInterpolatedHeights;
		std::shared_ptr<GPU::DRGLBufferStatic> mNormalsBuffer;
	};

}

#endif // __TERRAIN_NORMALS_H