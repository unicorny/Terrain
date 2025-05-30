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
		//! \param pos assume pos inside borders
		//! \return [0,1]
		DRReal getInterpolatedHeight(const DRVector2 pos) const;
	};

	class HeightMapLoader : public DRCPUTask
	{
	public:
		HeightMapLoader(DRCPUScheduler* scheduler, std::string_view fileName)
			: DRCPUTask(scheduler), mFileName(fileName) {}
		virtual ~HeightMapLoader() {}

		virtual DRReturn run() = 0;
		inline std::shared_ptr<HeightMap> getResult() { assert(isTaskFinished()); return mHeightMap; }

		virtual const char* getResourceType() const { return "HeightMapLoader"; }
	protected:
		std::string_view mFileName;
		std::shared_ptr<HeightMap> mHeightMap;
	};


	class CollectInterpolatedHeights : public DRCPUTask
	{
	public:
		CollectInterpolatedHeights(
			DRCPUScheduler* scheduler,
			const std::shared_ptr<const HeightMap> heightMap,
			const std::shared_ptr<const std::vector<DRVector2>> positions
		)
			: DRCPUTask(scheduler), mHeightMap(heightMap), mPositions(positions) {}

		CollectInterpolatedHeights(
			DRCPUScheduler* scheduler,
			const std::shared_ptr<const HeightMap> heightMap,
			const std::shared_ptr<const std::vector<DRVector2>> positions,
			std::vector<DRTaskPtr> parentTasks
		)
			: DRCPUTask(scheduler, parentTasks.size()), mHeightMap(heightMap), mPositions(positions) {
			for (int i = 0; i < parentTasks.size(); i++) {
				setParentTaskPtrInArray(parentTasks[i], i);
			}
		}

		CollectInterpolatedHeights(
			DRCPUScheduler* scheduler,
			const std::shared_ptr<const std::vector<DRVector2>> positions,
			std::shared_ptr<HeightMapLoader> heightMapLoaderTask
		)
			: DRCPUTask(scheduler, 1), mPositions(positions), mHeightMapLoaderTask(heightMapLoaderTask) {
			setParentTaskPtrInArray(mHeightMapLoaderTask, 0);
		}

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
		std::shared_ptr<const std::vector<DRVector2>> getPositions();

		virtual const char* getResourceType() const { return "CollectInterpolatedHeights"; }
	protected:
		std::shared_ptr<const HeightMap> mHeightMap;
		std::shared_ptr<const std::vector<DRVector2>> mPositions;
		std::shared_ptr<std::vector<DRReal>> mHeights;
		std::shared_ptr<HeightMapLoader> mHeightMapLoaderTask;
		std::shared_ptr<GenerateGeometrieTask> mPositionsGeneratingTask;
	};

	// best to use with disk scheduler, load hme file
	// written by hme editor: https://hme.sourceforge.net/
	class LoadHeightMapFromHme : public HeightMapLoader
	{
	public:
		using HeightMapLoader::HeightMapLoader;

		DRReturn run();
		virtual const char* getResourceType() const { return "LoadHeightMapFromHme"; }
	};

	// load height map from image, read red value, assume height map in black and white
	// allowed image types depends on DRImage, usually bmp, tga (also compressed), gif, jpeg, usw.
	class LoadHeightMapFromImage : public HeightMapLoader
	{
	public:
		using HeightMapLoader::HeightMapLoader;

		DRReturn run();

		virtual const char* getResourceType() const { return "LoadHeightMapFromImage"; }
	};

	// load height map from David Scherfgens TriBase Engine Terrain Editor TTP File Format
	class LoadHeightMapFromTTP : public HeightMapLoader
	{
	public:
		using HeightMapLoader::HeightMapLoader;

		DRReturn run();

		virtual const char* getResourceType() const { return "LoadHeightMapFromTTP"; }
	};
}

#endif //__TERRAIN_HEIGHT_MAP_H