#ifndef __TERRAIN_SIMPLE_TERRAIN_H
#define __TERRAIN_SIMPLE_TERRAIN_H

#include "TTP.h"
#include "HeightMap.h"
#include "Normals.h"
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
		DRReturn loadFromHMP(const char* hmpFileName, const char* ttpFileName);
		DRReturn Render();

		inline bool isLoaded() const { return mIsLoaded; }
		inline void setLoaded() { 
			mIsLoaded = true;
			DRLog.writeToLog("[SimpleTerrain] %s loading SimpleTerrain", mLoadingTimeSum.string().data()); 
		}
		void generating3DVertices(
			std::shared_ptr<CollectInterpolatedHeights> heightGenerator,
			std::shared_ptr<GenerateGeometrieTask> geometryGenerator,
			std::shared_ptr<CalculateNormalsTask> normalsGenerator
		);

	protected:
		TTPHeader mTTPInfos;
		std::atomic<bool> mIsLoaded;
		DRVertexBuffer mVertexBuffer;
		DRProfiler mLoadingTimeSum;
		// directly mapped to gpu buffer, for faster data transfer
		DRVector3* mVerticesBuffer;
		DRVector3* mNormalsBuffer;
		u32* mIndicesBuffer;

		size_t mVerticesCount;
		size_t mNormalCount;
		size_t mIndicesCount;
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
			mNormalsGenerator(normalsGenerator),
			mSimpleTerrain(simpleTerrain) 
		{
			setParentTaskPtrInArray(mGeometrieGenerator, 0);
			setParentTaskPtrInArray(mHeightsGenerator, 1);
			setParentTaskPtrInArray(mNormalsGenerator, 2);
		};
		virtual ~GeneratingSimpleTerrainTask() {}
		DRReturn run();
		virtual const char* getResourceType() const { return "GeneratingSimpleTerrainTask"; }
	protected:
		std::shared_ptr<CollectInterpolatedHeights> mHeightsGenerator;
		std::shared_ptr<GenerateGeometrieTask> mGeometrieGenerator;
		std::shared_ptr<CalculateNormalsTask> mNormalsGenerator;
		SimpleTerrain* mSimpleTerrain;
	};
}


#endif //__TERRAIN_SIMPLE_TERRAIN_H