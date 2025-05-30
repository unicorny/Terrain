#ifndef __TERRAIN_GENERATE_GEOMETRIE_TASK_H
#define __TERRAIN_GENERATE_GEOMETRIE_TASK_H

#include "DRCore2/Threading/DRCPUTask.h"
#include "DRCore2/Foundation/DRVector2.h"
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
			DRVector2 size,
			DRVector2 stepSize
		) : DRCPUTask(scheduler), mSize(size), mStepSize(stepSize) {}

		virtual ~GenerateGeometrieTask() {};

		DRReturn run();

		std::shared_ptr<std::vector<DRVector2>> getPositions() { assert(isTaskFinished()); return mPositions; }
		std::shared_ptr<std::vector<unsigned int>> getIndices() { assert(isTaskFinished()); return mIndices; }
		DRVector2 getSize() const { return mSize; }

		virtual const char* getResourceType() const { return "GenerateGeometrieTask"; }

	protected:
		DRVector2 mSize;
		DRVector2 mStepSize;
		std::shared_ptr<std::vector<DRVector2>> mPositions;
		std::shared_ptr<std::vector<unsigned int>> mIndices;
	};
}

#endif //__TERRAIN_GENERATE_GEOMETRIE_TASK_H