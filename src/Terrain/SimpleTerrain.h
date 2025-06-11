#ifndef __TERRAIN_SIMPLE_TERRAIN_H
#define __TERRAIN_SIMPLE_TERRAIN_H

#include "AbstractTerrain.h"
#include "../GPU/DRGLBufferStatic.h"

#include "TTP.h"
#include "HeightMap.h"
#include "Normals.h"

#include "DRCore2/Threading/DRCPUTask.h"

#include "DREngine/DRLogging.h"
#include <string>
#include <atomic>

namespace Terrain {
	class SimpleTerrain : public AbstractTerrain
	{
	public:
		using AbstractTerrain::AbstractTerrain;
		virtual DRReturn load(const char* heightMapFileName);
		virtual void exit();
		virtual DRReturn Render();
		
		void generating3DVertices(
			std::shared_ptr<CollectInterpolatedHeights> heightGenerator,
			std::shared_ptr<GenerateGeometrieTask> geometryGenerator
		);

	protected:
		inline void setLoaded() {
			mIsLoaded = true;
			DRLog.writeToLog("[SimpleTerrain] %s loading SimpleTerrain", mLoadingTimeSum.string().data());
		}		
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