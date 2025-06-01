#ifndef __TERRAIN_HEIGHT_MAP_H
#define __TERRAIN_HEIGHT_MAP_H

#include "DRCore2/DRTypes.h"
#include "DRCore2/Foundation/DRVector2.h"
#include "DRCore2/Threading/DRCPUTask.h"

#include "GenerateGeometrieTask.h"

#include <memory>
#include <vector>

namespace Terrain {

	// should be cache efficient up to 256x256 (L2 Cache) or 128x128 (L1 Cache)
	struct HeightMap {
		u16 width;
		u16 height;
		std::vector<u8> map;
		//! \param pos assume pos in height map ccordinates space
		//! \return [0,1]
		DRReal getInterpolatedHeight(const DRVector2& pos) const;
	};

	class HeightMapLoader : public DRCPUTask
	{
	public:
		HeightMapLoader(DRCPUScheduler* scheduler, std::string_view fileName)
			: DRCPUTask(scheduler), mFileName(fileName) {}
		virtual ~HeightMapLoader() {}

		virtual DRReturn run();
		inline std::shared_ptr<HeightMap> getResult() { assert(isTaskFinished()); return mHeightMap; }

		virtual const char* getResourceType() const { return "HeightMapLoader"; }
	protected:
		DRReturn loadFromHme();
		DRReturn loadFromTTP();
		DRReturn loadFromImage();
		std::string mFileName;
		std::shared_ptr<HeightMap> mHeightMap;
	};


	class CollectInterpolatedHeights : public DRCPUTask
	{
	public:
		CollectInterpolatedHeights(
			DRCPUScheduler* scheduler,
			std::shared_ptr<GenerateGeometrieTask> positionsGeneratingTask,
			std::shared_ptr<HeightMapLoader> heightMapLoaderTask
		)
			: DRCPUTask(scheduler, 2), mHeightMapLoaderTask(heightMapLoaderTask), mPositionsGeneratingTask(positionsGeneratingTask) {
			setParentTaskPtrInArray(mHeightMapLoaderTask, 0);
			setParentTaskPtrInArray(mPositionsGeneratingTask, 1);
		}

		virtual ~CollectInterpolatedHeights() {};

		DRReturn run();
		inline std::shared_ptr<std::vector<DRReal>> getResult() { assert(isTaskFinished()); return mHeights; }
		inline std::shared_ptr<HeightMap> getHeightMap() { return mHeightMapLoaderTask->getResult(); }
		std::shared_ptr<const std::vector<DRVector2>> getPositions();

		virtual const char* getResourceType() const { return "CollectInterpolatedHeights"; }
	protected:
		std::shared_ptr<const HeightMap> mHeightMap;
		std::shared_ptr<const std::vector<DRVector2>> mPositions;
		std::shared_ptr<std::vector<DRReal>> mHeights;
		std::shared_ptr<HeightMapLoader> mHeightMapLoaderTask;
		std::shared_ptr<GenerateGeometrieTask> mPositionsGeneratingTask;
	};
}

#endif //__TERRAIN_HEIGHT_MAP_H