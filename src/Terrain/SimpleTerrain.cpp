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
		: mTerrainHeight(0), mIsLoaded(false)
	{

	}

	SimpleTerrain::~SimpleTerrain()
	{

	}

	DRReturn SimpleTerrain::load(QuadraticGridLogic grid, const char* heightMapFileName)
	{
		mLoadingTimeSum.reset();
		DRProfiler timeUsed;

		mGrid = grid;
		mVerticesBuffer = std::make_shared<GPU::DRGLBufferStatic>(grid.totalVertices());
		mVerticesBuffer->createOpenGLBuffer(GPU::DRGLBufferType::VERTEX_BUFFER);
		mNormalsBuffer = std::make_shared<GPU::DRGLBufferStatic>(grid.totalVertices());
		mNormalsBuffer->createOpenGLBuffer(GPU::DRGLBufferType::NORMALS_BUFFER);
		mIndicesBuffer = std::make_shared<GPU::DRGLBufferStatic>(grid.totalIndices(GL_TRIANGLE_STRIP));
		mIndicesBuffer->createOpenGLBuffer(GPU::DRGLBufferType::INDICES_BUFFER);

		DRLog.writeToLog("%s for creating buffer with openGL", timeUsed.string().data());
		timeUsed.reset();

		DRReal vertexDataSize = (grid.totalVertices() * 2 * sizeof(DRVector3)) / 1024.0 / 1024.0;
		DRReal indexDataSize = (grid.totalIndices(GL_TRIANGLE_STRIP) * sizeof(u32)) / 1024.0f / 1024.0f;
		DRLog.writeToLog("[SimpleTerrain] %.2f MByte Vertex Data, %.2f MByte Index Data", vertexDataSize, indexDataSize);

		// generate DRVector2 positions
		auto geometrieTask = std::make_shared<GenerateGeometrieTask>(g_MainScheduler, grid, mIndicesBuffer);
		// geometrieTask->scheduleTask(geometrieTask);
		// load height map from file
		auto heightMapLoadingTask = std::make_shared<HeightMapLoader>(g_DiskScheduler, heightMapFileName);
		// heightMapLoadingTask->scheduleTask(heightMapLoadingTask);
		// interpolate heights from height map
		auto heightMapTask = std::make_shared<CollectInterpolatedHeights>(g_MainScheduler, geometrieTask, heightMapLoadingTask);
		// calculate normals with height map
		auto normalsTask = std::make_shared<CalculateNormalsTask>(g_MainScheduler, grid, mNormalsBuffer, heightMapTask);
		// heightMapTask->scheduleTask(heightMapTask);
		// bring all together
		auto generatingSimpleTerrainTask = std::make_shared<GeneratingSimpleTerrainTask>(g_MainScheduler, heightMapTask, geometrieTask, normalsTask, this);
		generatingSimpleTerrainTask->scheduleTask(generatingSimpleTerrainTask);
		DRLog.writeToLog("[SimpleTerrain] %s for get glBuffer Pointer and starting tasks", timeUsed.string().data());
		return DR_OK;
	}

	DRReturn SimpleTerrain::Render()
	{
		if (!mIsLoaded) {
			g_MainScheduler->checkPendingTasks();
			return DR_NOT_ERROR;
		}
		// hack because we can umap GLBuffer only in same Thread in which opengl context was created
		if (mIsLoaded && !mVerticesBuffer->isReadyForRender() ) {
			DRProfiler timeUsed;
			mVerticesBuffer->unmap();
			mNormalsBuffer->unmap();
			mIndicesBuffer->unmap();
			DRLog.writeToLog("%s used time for unmap vertex and index buffer", timeUsed.string().data());
		}

		// Enable it
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_NORMALIZE);
		GLfloat lightPos[] = { 1.0f, 1.0f, 1.0f, 0.0f };  // x, y, z, w=1 (Position) oder w=0 (Richtung)
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 0.4f };
		GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 0.4f };
		GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 0.4f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

		/*
		// glEnable(GL_LIGHT1);
		GLfloat lightPos2[] = {1024.0f, 700.0f, 1024.0f, 1.0f};  // x, y, z, w=1 (Position) oder w=0 (Richtung)
		glLightfv(GL_LIGHT1, GL_POSITION, lightPos2);

		GLfloat ambient2[] = { 0.4f, 0.4f, 0.4f, 1.0f };
		GLfloat diffuse2[] = { 1.0f, 0.2f, 0.2f, 1.0f };
		GLfloat specular2[] = { 1.0f, 1.0f, 1.0f, 0.4f };

		glLightfv(GL_LIGHT1, GL_AMBIENT, ambient2);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse2);
		glLightfv(GL_LIGHT1, GL_SPECULAR, specular2);

		// glEnable(GL_LIGHT2);
		GLfloat lightPos3[] = { 2048.0f, 600.0f, 2048.0f, 1.0f };  // x, y, z, w=1 (Position) oder w=0 (Richtung)
		glLightfv(GL_LIGHT1, GL_POSITION, lightPos3);

		GLfloat ambient3[] = { 0.4f, 0.4f, 0.4f, 1.0f };
		GLfloat diffuse3[] = { 0.2f, 1.0f, 0.2f, 1.0f };
		GLfloat specular3[] = { 1.0f, 1.0f, 1.0f, 0.4f };

		glLightfv(GL_LIGHT2, GL_AMBIENT, ambient3);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse3);
		glLightfv(GL_LIGHT2, GL_SPECULAR, specular3);

		*/

		GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
		GLfloat mat_specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
		GLfloat mat_shininess[] = { 16.0f };

		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
		

		// Send This Vertex To OpenGL To Be Rendered
		//glEnable(GL_TEXTURE_2D);
		//glBindTexture(GL_TEXTURE_2D, m_uiColorMap);
		// glEnable(GL_MULTISAMPLE_ARB);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);

		mVerticesBuffer->bind();
		glVertexPointer(3, GL_FLOAT, 0, nullptr);

		mNormalsBuffer->bind();
		glNormalPointer(GL_FLOAT, 0, nullptr);

		mIndicesBuffer->bind();
		
		glDrawElements(GL_TRIANGLE_STRIP, mIndicesBuffer->getElementCount(), GL_UNSIGNED_INT, nullptr);
		// glDrawArrays(GL_POINTS, 0, mVerticesBuffer->getElementCount());
		//mVertexBuffer.RenderIndex(GL_TRIANGLE_STRIP);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);

		// glDisable(GL_MULTISAMPLE_ARB);
		// glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
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
		auto& heightMap = heightGenerator->getHeightMap();
		auto& positions = geometryGenerator->getPositions();
		assert(heights && positions && heights->size() == positions->size());
		DRReal xzScale = static_cast<DRReal>(mGrid.size()) / static_cast<DRReal>(heightMap->width);
		DRReal yScale = mGrid.height();

		std::vector<DRVector3> temp;

		auto vertices = mVerticesBuffer->getVector3Ptr(GPU::DRGLBufferType::VERTEX_BUFFER);
		for (int i = 0; i < positions->size(); i++) {
			const auto& pos = (*positions)[i];
			const auto& height = (*heights)[i];
			vertices[i] = DRVector3(pos.x * xzScale, height * yScale, pos.y * xzScale);
			temp.push_back(DRVector3(pos.x * xzScale, height * yScale, pos.y * xzScale));
		}
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