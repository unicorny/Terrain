#ifndef __VERTEX_BUFFER__
#define __VERTEX_BUFFER__

#include "DRCore2/DRTypes.h"
#include "VertexGroups.h"
#include "DREngine/OpenGL.h"

#include <vector>

/*!
* @author unicorny
* 
* reeimplemantation of vertex buffer, because code was lost
*/


class DRVertexBuffer
{
public:
	DRVertexBuffer();
	~DRVertexBuffer();
	DRReturn Init(DRVector3* pVertices, uint32_t iNumVertices, DRColor* pColors, DRVector2* pTexCoords, DRVector3* pNormals = nullptr, bool bValue = true);
	DRReturn Init(const std::vector<DRVector3>& vertices, const std::vector<unsigned int>& indices);
	DRReturn Init(const std::vector<DRVector3>& vertices, const std::vector<DRVector3>& normals, const std::vector<unsigned int>& indices);
	DRReturn Init(const std::vector<DetailedVertex>& vertexData);
	DRReturn Init(const std::vector<DetailedVertex>& vertexData, const std::vector<int>& indices);
	DRReturn RenderIndex(GLenum renderMode, uint32_t indexCount, u32* pIndices);
	DRReturn RenderIndex(GLenum renderMode, uint32_t indexCount, u16* pIndices);
	DRReturn RenderIndex(GLenum renderMode);
	DRReturn Render(GLenum renderMode);
	void Exit();

	bool isFilled() const { return mVertexCount > 0; }
protected:
	void setVertexData(const std::vector<DetailedVertex>& vertexData);

	GLuint mVAO;
	GLuint mVBONormals;
	uint32_t mVertexCount;
	uint32_t mIndexCount;
};

#endif //__VERTEX_BUFFER__