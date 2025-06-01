#ifndef __TERRAIN_SIMPLE_TERRAIN_H
#define __TERRAIN_SIMPLE_TERRAIN_H

#include "../GPU/DRGLBufferStatic.h"

#include "TTP.h"
#include "HeightMap.h"
#include "Normals.h"
#include "QuadraticGridLogic.h"

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
		DRReturn load(QuadraticGridLogic grid, const char* heightMapFileName);
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
		QuadraticGridLogic mGrid;
		DRReal			   mTerrainHeight;
		std::atomic<bool> mIsLoaded;
		DRProfiler mLoadingTimeSum;
		// directly mapped to gpu buffer, for faster data transfer
		std::shared_ptr<GPU::DRGLBufferStatic> mVerticesBuffer;
		std::shared_ptr<GPU::DRGLBufferStatic> mNormalsBuffer;
		std::shared_ptr<GPU::DRGLBufferStatic> mIndicesBuffer;
	};


	class GeneratingSimpleTerrainTask : public DRCPUTask
	{
	public:
		GeneratingSimpleTerrainTask(
			DRCPUScheduler* scheduler, 
			std::shared_ptr<CollectInterpolatedHeights> heightsGenerator,
			std::shared_ptr<GenerateGeometrieTask> geometryGenerator,
			std::shared_ptr<CalculateNormalsTask> normalsGenerator,
			SimpleTerrain* simpleTerrain
		)
			: DRCPUTask(scheduler, 3), 
			mHeightsGenerator(heightsGenerator), 
			mGeometrieGenerator(geometryGenerator),
			mSimpleTerrain(simpleTerrain) 
		{
			setParentTaskPtrInArray(mGeometrieGenerator, 0);
			setParentTaskPtrInArray(mHeightsGenerator, 1);
			setParentTaskPtrInArray(normalsGenerator, 2);
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