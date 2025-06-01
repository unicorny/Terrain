#include "GenerateGeometrieTask.h"
#include "DREngine/DRLogging.h"
#include "DRCore2/Utils/DRProfiler.h"

namespace Terrain {

	DRReturn GenerateGeometrieTask::run()
	{
        DRProfiler timeUsed;
		    
		mPositions = std::make_shared<std::vector<DRVector2>>(mGrid.totalVertices(), DRVector2(0.0));
		mGrid.allVertices(mPositions->data());
		mGrid.triangleStripIndices(mIndicesBuffer->getIndicesPtr());
		/*
		u32 indicesCursor = 0;
		for (int y = 0; y < mGrid.quads(); y++)
		{
			for (int x = 0; x < mGrid.quads(); x++)
			{
				mPositions->push_back(mGrid.vertex(x, y));
				if (y + 1 >= mGrid.quads() || x + 1 >= mGrid.quads()) continue;
				int topLeft = y * mGrid.quads() + x;
				int topRight = topLeft + 1;
				int bottomLeft = topLeft + mGrid.quads();
				int bottomRight = bottomLeft + 1;

				mIndicesBuffer[indicesCursor++] = topLeft;
				mIndicesBuffer[indicesCursor++] = topRight;
				mIndicesBuffer[indicesCursor++] = bottomRight;
				mIndicesBuffer[indicesCursor++] = bottomLeft;
			}
		}
		*/
        DRLog.writeToLog("[GenerateGeometrieTask] %s for generating %d 2d Vertices + %d Indices", timeUsed.string().data(), mPositions->size(), mGrid.totalIndices(GL_TRIANGLE_STRIP));
		return DR_OK;
	}

}