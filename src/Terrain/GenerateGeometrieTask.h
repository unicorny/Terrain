#ifndef __TERRAIN_GENERATE_GEOMETRIE_TASK_H
#define __TERRAIN_GENERATE_GEOMETRIE_TASK_H

#include "../GPU/DRGLBufferStatic.h"
#include "DRCore2/Threading/DRCPUTask.h"
#include "DRCore2/Foundation/DRVector2.h"
#include "DRCore2/Foundation/DRVector3.h"
#include "QuadraticGridLogic.h"

#include "magic_enum/magic_enum.hpp"

#include <memory>
#include <cassert>

using namespace magic_enum;

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
			QuadraticGridLogic grid,
			std::shared_ptr<GPU::DRGLBufferStatic> indicesBuffer
		) : DRCPUTask(scheduler), mGrid(grid), mIndicesBuffer(indicesBuffer) 
		{
			assert(indicesBuffer);
		}

		virtual ~GenerateGeometrieTask() {};

		DRReturn run();

		std::shared_ptr<std::vector<DRVector2>> getPositions() { assert(isTaskFinished()); return mPositions; }
		u16 getSize() const { return mGrid.size(); }

		virtual const char* getResourceType() const { return "GenerateGeometrieTask"; }

	protected:
		QuadraticGridLogic mGrid;
		std::shared_ptr<std::vector<DRVector2>> mPositions;
		std::shared_ptr<GPU::DRGLBufferStatic> mIndicesBuffer;
	};
}

#endif //__TERRAIN_GENERATE_GEOMETRIE_TASK_H