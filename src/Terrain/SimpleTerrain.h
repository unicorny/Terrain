#ifndef __TERRAIN_SIMPLE_TERRAIN_H
#define __TERRAIN_SIMPLE_TERRAIN_H

#include "TTP.h"
#include "HeightMap.h"
#include "../GPU/DRVertexBuffer.h"

#include "DRCore2/DRTypes.h"
#include "DRCore2/Threading/DRCPUTask.h"
#include "DRCore2/Utils/DRProfiler.h"

#include "DREngine/DRLogging.h"
#include <string>
#include <atomic>

namespace Terrain {
	class SimpleTerrain
	{
	public:
		SimpleTerrain();
		~SimpleTerrain();
		DRReturn loadFromTTP(const char* ttpFileName);
		DRReturn Render();

		inline bool isLoaded() const { return mIsLoaded; }
		inline void setLoaded() { 
			mIsLoaded = true;
			DRLog.writeToLog("[SimpleTerrain] %s loading SimpleTerrain", mLoadingTimeSum.string().data()); 
		}
		void generating3DVertices(
			std::shared_ptr<CollectInterpolatedHeights> heightGenerator,
			std::shared_ptr<GenerateGeometrieTask> geometryGenerator
		);

	protected:
		TTPHeader mTTPInfos;
		std::atomic<bool> mIsLoaded;
		DRVertexBuffer mVertexBuffer;
		DRProfiler mLoadingTimeSum;

		std::vector<DRVector3> mVertices;
		std::vector<unsigned int> mIndices;
	};


	class GeneratingSimpleTerrainTask : public DRCPUTask
	{
	public:
		GeneratingSimpleTerrainTask(
			DRCPUScheduler* scheduler, 
			std::shared_ptr<CollectInterpolatedHeights> heightsGenerator,
			std::shared_ptr<GenerateGeometrieTask> geometryGenerator,
			SimpleTerrain* simpleTerrain
		)
			: DRCPUTask(scheduler, 2), 
			mHeightsGenerator(heightsGenerator), 
			mGeometrieGenerator(geometryGenerator),
			mSimpleTerrain(simpleTerrain) {
			setParentTaskPtrInArray(mGeometrieGenerator, 0);
			setParentTaskPtrInArray(mHeightsGenerator, 1);
		};
		virtual ~GeneratingSimpleTerrainTask() {}
		DRReturn run();
		virtual const char* getResourceType() const { return "GeneratingSimpleTerrainTask"; }
	protected:
		std::shared_ptr<CollectInterpolatedHeights> mHeightsGenerator;
		std::shared_ptr<GenerateGeometrieTask> mGeometrieGenerator;
		SimpleTerrain* mSimpleTerrain;
	};
}


#endif //__TERRAIN_SIMPLE_TERRAIN_H