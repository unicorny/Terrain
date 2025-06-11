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
		enum class SeamOverlapFlag : u8 {
			NONE = 0,
			LEFT = 1,
			TOP = 2,
			RIGHT = 4,
			BOTTOM = 8
		};
		HeightMap(): width(0), height(0), seamOverlapFlags(SeamOverlapFlag::NONE) {}
		~HeightMap() {}

		u16 width;
		u16 height;
		SeamOverlapFlag  seamOverlapFlags; 
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

	// split bigger height map in multiple small height maps
	class HeightMapSplit : public DRCPUTask
	{
	public:
		//! \param width  Width of each resulting height map tile.
		//!               Must evenly divide the original height map's width.
		//! \param height Height of each resulting height map tile.
		//!               Must evenly divide the original height map's height.
		HeightMapSplit(
			DRCPUScheduler* scheduler,
			std::shared_ptr<HeightMapLoader> heightMapLoader,
			u8 width,
			u8 height
		)
			: DRCPUTask(scheduler, 1), mHeightMapLoader(heightMapLoader), mWidth(width), mHeight(height)
		{
			setParentTaskPtrInArray(mHeightMapLoader, 0);
		}
		virtual ~HeightMapSplit() {}

		virtual DRReturn run();
		//! Returns a specific tile by combined linear index.
		//! Only valid after the task has finished.
		inline std::shared_ptr<HeightMap> getResult(u8 combinedIndex);
		inline std::shared_ptr<HeightMap> getResult(u8 x, u8 y) { return getResult(y * mWidth + x); }

		virtual const char* getResourceType() const { return "HeightMapSplit"; }
	protected:
		std::vector<std::shared_ptr<HeightMap>> mHeightMaps;
		std::shared_ptr<HeightMapLoader> mHeightMapLoader;
		u8 mWidth;
		u8 mHeight;
	};

	std::shared_ptr<HeightMap> HeightMapSplit::getResult(u8 combinedIndex) {
		assert(isTaskFinished() && combinedIndex < mHeightMaps.size());
		return mHeightMaps[combinedIndex];
	}


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

		virtual ~CollectInterpolatedHeights() {}

		DRReturn run();
		inline std::shared_ptr<std::vector<DRReal>> getResult() { assert(isTaskFinished()); return mHeights; }
		inline std::shared_ptr<HeightMap> getHeightMap() { return mHeightMapLoaderTask->getResult(); }
		std::shared_ptr<const std::vector<DRVector2>> getPositions() const { assert(mPositionsGeneratingTask->isTaskFinished()); return mPositionsGeneratingTask->getPositions(); }

		virtual const char* getResourceType() const { return "CollectInterpolatedHeights"; }
	protected:
		std::shared_ptr<std::vector<DRReal>> mHeights;
		std::shared_ptr<HeightMapLoader> mHeightMapLoaderTask;
		std::shared_ptr<GenerateGeometrieTask> mPositionsGeneratingTask;
	};
}

#endif //__TERRAIN_HEIGHT_MAP_H