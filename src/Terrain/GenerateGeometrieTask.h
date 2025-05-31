#ifndef __TERRAIN_GENERATE_GEOMETRIE_TASK_H
#define __TERRAIN_GENERATE_GEOMETRIE_TASK_H

#include "DRCore2/Threading/DRCPUTask.h"
#include "DRCore2/Foundation/DRVector2.h"
#include "DRCore2/Foundation/DRVector3.h"
#include "GridDimensions.h"
#include <memory>
#include <cassert>

namespace Terrain {
	class GenerateGeometrieTask : public DRCPUTask
	{
	public:
		/*!
		* \param cpu scheduler
		* \param size terrain size
		* \param stepSize difference between vertices, until size is fullfiled, so stepSize < terrain size
		*/
		GenerateGeometrieTask(
			DRCPUScheduler* scheduler,
			GridDimensions grid,
			u32* indicesBuffer
		) : DRCPUTask(scheduler), mGrid(grid), mIndicesBuffer(indicesBuffer) {}

		virtual ~GenerateGeometrieTask() {};

		DRReturn run();

		std::shared_ptr<std::vector<DRVector2>> getPositions() { assert(isTaskFinished()); return mPositions; }
		DRVector2 getSize() const { return mGrid.size; }

		virtual const char* getResourceType() const { return "GenerateGeometrieTask"; }

	protected:
		GridDimensions mGrid;
		std::shared_ptr<std::vector<DRVector2>> mPositions;
		u32* mIndicesBuffer;
	};
}

#endif //__TERRAIN_GENERATE_GEOMETRIE_TASK_H