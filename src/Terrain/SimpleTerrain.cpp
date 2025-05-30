#include "SimpleTerrain.h"

#include "../main.h"
#include "GenerateGeometrieTask.h"
#include "DRCore2/Utils/DRProfiler.h"
#include "DREngine/DRLogging.h"
#include "DRCore2/Threading/DRCPUScheduler.h"
#include "DRCore2/Threading/DRCPUTask.h"

namespace Terrain {

	SimpleTerrain::SimpleTerrain()
	{

	}

	SimpleTerrain::~SimpleTerrain()
	{

	}

	DRReturn SimpleTerrain::loadFromTTP(const char* ttpFileName)
	{
		mLoadingTimeSum.reset();
		DRProfiler timeUsed;
		mTTPInfos = loadTTPHeaderFromFile(ttpFileName);
		DRLog.writeToLog("[SimpleTerrain] %s time for loading TTP File Header", timeUsed.string().data());
		timeUsed.reset();

		float fStepSize = 1.0f;
		// generate DRVector2 positions
		auto geometrieTask = std::make_shared<GenerateGeometrieTask>(g_MainScheduler, mTTPInfos.heightMapSize, fStepSize);
		// geometrieTask->scheduleTask(geometrieTask);
		// load height map from file
		auto heightMapLoadingTask = std::make_shared<LoadHeightMapFromTTP>(g_DiskScheduler, ttpFileName);
		// heightMapLoadingTask->scheduleTask(heightMapLoadingTask);
		// interpolate heights from height map
		auto heightMapTask = std::make_shared<CollectInterpolatedHeights>(g_MainScheduler, geometrieTask, heightMapLoadingTask);
		// heightMapTask->scheduleTask(heightMapTask);
		// bring all together
		auto generatingSimpleTerrainTask = std::make_shared<GeneratingSimpleTerrainTask>(g_MainScheduler, heightMapTask, geometrieTask, this);
		generatingSimpleTerrainTask->scheduleTask(generatingSimpleTerrainTask);
		DRLog.writeToLog("[SimpleTerrain] %s for chaining together Simple Terrain Tasks", timeUsed.string().data());
		return DR_OK;
	}

	DRReturn SimpleTerrain::Render()
	{
		// Send This Vertex To OpenGL To Be Rendered
		//glCallList(m_uiList);
		glEnable(GL_TEXTURE_2D);
		//glBindTexture(GL_TEXTURE_2D, m_uiColorMap);
		glEnable(GL_MULTISAMPLE_ARB);
		if (!mIsLoaded) {
			g_MainScheduler->checkPendingTasks();
			return DR_NOT_ERROR;
		}
		// hack because I can fill Vertex Buffer only in Same Thread in which opengl context was created
		if (mIsLoaded && !mVertexBuffer.isFilled()) {
			DRProfiler timeUsed;
			mVertexBuffer.Init(mVertices, mIndices);
			DRLog.writeToLog("[SimpleTerrain] %s used for fill Vertex Buffer", timeUsed.string().data());
		}
		if (mVertexBuffer.RenderIndex(GL_QUADS)) LOG_ERROR("Fehler beim Rendern des Terrain VertexBuffers!", DR_ERROR);
		glDisable(GL_MULTISAMPLE_ARB);
		glDisable(GL_TEXTURE_2D);
		if (DRGrafikError("Fehler beim Terrain Render!")) return DR_ERROR;
		return DR_OK;
	}

	void SimpleTerrain::generating3DVertices(
		std::shared_ptr<CollectInterpolatedHeights> heightGenerator,
		std::shared_ptr<GenerateGeometrieTask> geometryGenerator
	)
	{
		if (mIsLoaded) {
			LOG_WARNING("already loaded, called again");
			return;
		}
		DRProfiler timeUsed;
		auto& heights = heightGenerator->getResult();
		auto& positions = geometryGenerator->getPositions();
		assert(heights && positions && heights->size() == positions->size());
		DRReal xzScale = mTTPInfos.terrainSize / mTTPInfos.heightMapSize;
		DRReal yScale = mTTPInfos.terrainHeight;

		mVertices.reserve(positions->size());
		for (int i = 0; i < positions->size(); i++) {
			const auto& pos = (*positions)[i];
			const auto& height = (*heights)[i];
			mVertices.push_back(DRVector3(pos.x * xzScale, height * yScale, pos.y * xzScale));
		}
		mIndices.swap(*geometryGenerator->getIndices());
		DRLog.writeToLog("[SimpleTerrain] %s used for create 3D Vertices", timeUsed.string().data());
		timeUsed.reset();

		setLoaded();
	}

	DRReturn GeneratingSimpleTerrainTask::run()
	{
		mSimpleTerrain->generating3DVertices(mHeightsGenerator, mGeometrieGenerator);
		return DR_OK;
	}
}