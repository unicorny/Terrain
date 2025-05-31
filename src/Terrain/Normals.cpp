#include "Normals.h"
#include "DREngine/DRLogging.h"

namespace Terrain {
	DRReturn CalculateNormalsTask::run()
	{
		DRProfiler timeUsed;
		DRVector2i stepsCount = mSize / mStepSize;
		if (DRVector2(stepsCount) * mStepSize != mSize) {
			LOG_ERROR("please check params, stepsCount * mStepSize != mSize", DR_ERROR);
		}
		const auto& height = *mHeightMap->getResult();
		
		mNormals = std::make_shared<std::vector<DRVector3>>();
		mNormals->reserve(stepsCount.x * stepsCount.y);
		for (DRReal y = 0; y < stepsCount.y; y++)
		{
			for (DRReal x = 0; x < stepsCount.x; x++)
			{
				DRReal hLeft = height.getInterpolatedHeight(DRVector2( y, x > 0 ? x - 1 : x ) * mStepSize);
				DRReal hRight = height.getInterpolatedHeight(DRVector2( y, x + 1 < stepsCount.x ? x + 1 : x ) * mStepSize);
				DRReal hDown = height.getInterpolatedHeight(DRVector2( y > 0 ? y - 1 : y, x ) * mStepSize);
				DRReal hUp = height.getInterpolatedHeight(DRVector2( y + 1 < stepsCount.y ?  y + 1 : y, x ) * mStepSize);

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