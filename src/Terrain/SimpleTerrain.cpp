#include "SimpleTerrain.h"

#include "../main.h"
#include "Normals.h"
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
		// calculate normals with height map
		auto normalsTask = std::make_shared<CalculateNormalsTask>(g_MainScheduler, mTTPInfos.heightMapSize, fStepSize, heightMapLoadingTask);
		// heightMapTask->scheduleTask(heightMapTask);
		// bring all together
		auto generatingSimpleTerrainTask = std::make_shared<GeneratingSimpleTerrainTask>(g_MainScheduler, heightMapTask, geometrieTask, normalsTask, this);
		generatingSimpleTerrainTask->scheduleTask(generatingSimpleTerrainTask);
		DRLog.writeToLog("[SimpleTerrain] %s for chaining together Simple Terrain Tasks", timeUsed.string().data());
		return DR_OK;
	}

	DRReturn SimpleTerrain::loadFromHMP(const char* hmpFileName, const char* ttpFileName)
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
		auto heightMapLoadingTask = std::make_shared<LoadHeightMapFromHme>(g_DiskScheduler, hmpFileName);
		// heightMapLoadingTask->scheduleTask(heightMapLoadingTask);
		// interpolate heights from height map
		auto heightMapTask = std::make_shared<CollectInterpolatedHeights>(g_MainScheduler, geometrieTask, heightMapLoadingTask);
		// heightMapTask->scheduleTask(heightMapTask);
		// calculate normals with height map
		auto normalsTask = std::make_shared<CalculateNormalsTask>(g_MainScheduler, mTTPInfos.heightMapSize, fStepSize, heightMapLoadingTask);
		// bring all together
		auto generatingSimpleTerrainTask = std::make_shared<GeneratingSimpleTerrainTask>(g_MainScheduler, heightMapTask, geometrieTask, normalsTask, this);
		generatingSimpleTerrainTask->scheduleTask(generatingSimpleTerrainTask);
		DRLog.writeToLog("[SimpleTerrain] %s for chaining together Simple Terrain Tasks", timeUsed.string().data());
		return DR_OK;
	}

	DRReturn SimpleTerrain::Render()
	{
		if (!mIsLoaded) {
			g_MainScheduler->checkPendingTasks();
			return DR_NOT_ERROR;
		}
		// hack because we can fill Vertex Buffer only in Same Thread in which opengl context was created
		if (mIsLoaded && !mVertexBuffer.isFilled()) {
			DRProfiler timeUsed;
			mVertexBuffer.Init(mVertices, mNormals, mIndices);
			DRLog.writeToLog("[SimpleTerrain] %s used for fill Vertex Buffer", timeUsed.string().data());
		}

		// Enable it
		glEnable(GL_LIGHTING); 
		glEnable(GL_LIGHT0);
		GLfloat lightPos[] = { 0.0f, 100.0f, 100.0f, 1.0f };  // x, y, z, w=1 (Position) oder w=0 (Richtung)
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
		GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

		glEnable(GL_LIGHT1);
		GLfloat lightPos2[] = {0.0f, 100.0f, -100.0f, 1.0f};  // x, y, z, w=1 (Position) oder w=0 (Richtung)
		glLightfv(GL_LIGHT1, GL_POSITION, lightPos2);

		GLfloat ambient2[] = { 0.4f, 0.4f, 0.4f, 1.0f };
		GLfloat diffuse2[] = { 1.0f, 0.9f, 1.0f, 1.0f };
		GLfloat specular2[] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glLightfv(GL_LIGHT1, GL_AMBIENT, ambient2);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse2);
		glLightfv(GL_LIGHT1, GL_SPECULAR, specular2);

		GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
		GLfloat mat_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
		GLfloat mat_shininess[] = { 32.0f };

		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);


		// Send This Vertex To OpenGL To Be Rendered
		//glCallList(m_uiList);
		glEnable(GL_TEXTURE_2D);
		//glBindTexture(GL_TEXTURE_2D, m_uiColorMap);
		glEnable(GL_MULTISAMPLE_ARB);
		
		if (mVertexBuffer.RenderIndex(GL_QUADS)) LOG_ERROR("Fehler beim Rendern des Terrain VertexBuffers!", DR_ERROR);
		glDisable(GL_MULTISAMPLE_ARB);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		if (DRGrafikError("Fehler beim Terrain Render!")) return DR_ERROR;
		return DR_OK;
	}

	void SimpleTerrain::generating3DVertices(
		std::shared_ptr<CollectInterpolatedHeights> heightGenerator,
		std::shared_ptr<GenerateGeometrieTask> geometryGenerator,
		std::shared_ptr<CalculateNormalsTask> normalsGenerator
	)
	{
		if (mIsLoaded) {
			LOG_WARNING("already loaded, called again");
			return;
		}
		DRProfiler timeUsed;
		auto& heights = heightGenerator->getResult();
		auto& positions = geometryGenerator->getPositions();
		auto& normals = normalsGenerator->getResult();
		assert(heights && positions && normals && heights->size() == positions->size() && heights->size() == normals->size());
		DRReal xzScale = mTTPInfos.terrainSize / mTTPInfos.heightMapSize;
		DRReal yScale = mTTPInfos.terrainHeight;

		mVertices.reserve(positions->size());
		for (int i = 0; i < positions->size(); i++) {
			const auto& pos = (*positions)[i];
			const auto& height = (*heights)[i];
			mVertices.push_back(DRVector3(pos.x * xzScale, height * yScale, pos.y * xzScale));
		}
		mIndices.swap(*geometryGenerator->getIndices());
		mNormals.swap(*normals);
		DRLog.writeToLog("[SimpleTerrain] %s used for create 3D Vertices", timeUsed.string().data());
		timeUsed.reset();

		setLoaded();
	}

	DRReturn GeneratingSimpleTerrainTask::run()
	{
		mSimpleTerrain->generating3DVertices(mHeightsGenerator, mGeometrieGenerator, mNormalsGenerator);
		return DR_OK;
	}
}