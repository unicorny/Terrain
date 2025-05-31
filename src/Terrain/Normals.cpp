#include "Normals.h"
#include "DREngine/DRLogging.h"

namespace Terrain {
	DRReturn CalculateNormalsTask::run()
	{
		DRProfiler timeUsed;
		DRVector2i stepsCount = mSize / mStepSize;
		if (stepsCount * mStepSize != mSize) {
			LOG_ERROR("please check params, stepsCount * mStepSize != mSize", DR_ERROR);
		}
		const auto& height = *mHeightMap->getResult();

		mNormals = std::make_shared<std::vector<DRVector3>>();
		mNormals->reserve(stepsCount.x * stepsCount.y);
		for (int y = 0; y < stepsCount.y; y++)
		{
			for (int x = 0; x < stepsCount.x; x++)
			{
				DRReal hLeft = height.getInterpolatedHeight({ y, x > 0 ? x - 1 : x });
				DRReal hRight = height.getInterpolatedHeight({ y, x + 1 < stepsCount.x ? x + 1 : x });
				DRReal hDown = height.getInterpolatedHeight({ y > 0 ? y - 1 : y, x });				
				DRReal hUp = height.getInterpolatedHeight({ y + 1 < stepsCount.y ?  y + 1 : y, x });

				DRVector3 dX = { 2.0f * mStepSize.x, hRight - hLeft, 0.0f };
				DRVector3 dY = { 0.0f, hUp - hDown, 2.0f * mStepSize.y };
				DRVector3 normal = dY.cross(dX);
				mNormals->push_back(normal.normalize());
			}
		}
		DRLog.writeToLog("[CalculateNormalsTask] %s for calculate %d normals", timeUsed.string().data(), mNormals->size());
		return DR_OK;
	}
}