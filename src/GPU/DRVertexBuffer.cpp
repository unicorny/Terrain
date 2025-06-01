#include "DRVertexBuffer.h"
#include "DRCore2/Utils/DRProfiler.h"
#include "DREngine/DRLogging.h"
#include <type_traits>
#include <cassert>

static_assert(std::is_same<DRReal, float>::value, "DRReal ist NICHT float! Rendering erwartet float-basiertes Layout!");

DRVertexBuffer::DRVertexBuffer()
	: mVAO(0), mVBOVertices(0), mVBONormals(0), mVBOIndices(0),
    mVertexCount(0), mNormalsCount(0), mIndexCount(0),
    mInitialized(false)
{

}

DRVertexBuffer::~DRVertexBuffer()
{
	Exit();
}

DRReturn DRVertexBuffer::Init(DRVector3* pVertices, uint32_t iNumVertices, DRColor* pColors, DRVector2* pTexCoords, DRVector3* pNormals/* = nullptr*/, bool bValue/* = true*/)
{
    // Simple VAO + VBO Init
    GLuint vbo[4];
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    int attribIndex = 0;
    mVertexCount = iNumVertices;

    // Vertex positions
    if (pVertices) {
        glGenBuffers(1, &vbo[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DRVector3) * iNumVertices, pVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(attribIndex++);
    }

    // Colors
    if (pColors) {
        glGenBuffers(1, &vbo[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DRColor) * iNumVertices, pColors, GL_STATIC_DRAW);
        glVertexAttribPointer(attribIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(attribIndex++);
    }

    // Texture Coordinates
    if (pTexCoords) {
        glGenBuffers(1, &vbo[2]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DRVector2) * iNumVertices, pTexCoords, GL_STATIC_DRAW);
        glVertexAttribPointer(attribIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(attribIndex++);
    }

    // Normals
    if (pNormals) {
        glGenBuffers(1, &vbo[3]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DRVector3) * iNumVertices, pNormals, GL_STATIC_DRAW);
        glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(attribIndex++);
    }

    glBindVertexArray(0);
    return DR_OK;
}

DRReturn DRVertexBuffer::Init(DRVector3* pVertices, uint32_t iNumVertices, DRVector3* pNormals/* = nullptr*/, u32* indices, uint32_t indexCount)
{
    // Simple VAO + VBO Init
    GLuint vbo[4];
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    int attribIndex = 0;
    mVertexCount = iNumVertices;

    // Vertex positions
    if (pVertices) {
        glGenBuffers(1, &vbo[0]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DRVector3) * iNumVertices, pVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(attribIndex++);
    }

    // Normals
    if (pNormals) {
        glGenBuffers(1, &mVBONormals);
        glBindBuffer(GL_ARRAY_BUFFER, mVBONormals);
        glBufferData(GL_ARRAY_BUFFER, sizeof(DRVector3) * iNumVertices, pNormals, GL_STATIC_DRAW);
        glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(attribIndex++);
    }

    if (indices) {
        GLuint indexBufferID;
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexCount, indices, GL_STATIC_DRAW);
        mIndexCount = static_cast<uint32_t>(indexCount);
    }

    glBindVertexArray(0);
    mInitialized = true;
    return DR_OK;
}

DRReturn DRVertexBuffer::Init(const std::vector<DetailedVertex>& vertexData)
{
    if (vertexData.empty()) return DR_ERROR;

    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    setVertexData(vertexData);
    mVertexCount = static_cast<uint32_t>(vertexData.size());    

    glBindVertexArray(0);
    return DR_OK;
}

/*DRReturn DRVertexBuffer::Init(const std::vector<DRVector3>& vertices, const std::vector<unsigned int>& indices)
{
    if (vertices.empty()) return DR_ERROR;

    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    GLuint vbo;
    int attribIndex = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DRVector3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(attribIndex++);

    if (indices.size()) {
        GLuint indexBufferID;
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);
        mIndexCount = static_cast<uint32_t>(indices.size());
    }

    mVertexCount = static_cast<uint32_t>(vertices.size());
    

    glBindVertexArray(0);
    return DR_OK;
}*/

DRReturn DRVertexBuffer::Init()
{
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    int attribIndex = 0;    
    glBindBuffer(GL_ARRAY_BUFFER, mVBOVertices);
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(attribIndex++);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    if (mVBONormals) {
        glBindBuffer(GL_ARRAY_BUFFER, mVBONormals);
        glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_TRUE, 0, 0);
        glEnableVertexAttribArray(attribIndex++);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    if (mVBOIndices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBOIndices);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    glBindVertexArray(0);
    mInitialized = true;
    return DR_OK;
}

DRVector3* DRVertexBuffer::getVerticesBufferPtr(size_t vertexCount)
{
    assert(mVBOVertices == 0);    
    mVertexCount = vertexCount;
    auto verticesByteSize = vertexCount * sizeof(DRVector3);
    GLenum flags = GL_MAP_WRITE_BIT;

    glGenBuffers(1, &mVBOVertices);
    glBindBuffer(GL_ARRAY_BUFFER, mVBOVertices);    
    glBufferStorage(GL_ARRAY_BUFFER, verticesByteSize, 0, GL_MAP_WRITE_BIT);
    return static_cast<DRVector3*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
}

DRVector3* DRVertexBuffer::getNormalsBufferPtr(size_t normalCount)
{
    assert(mVBONormals == 0);
    mNormalsCount = normalCount;
    auto normalsByteSize = normalCount * sizeof(DRVector3);
    GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    glGenBuffers(1, &mVBONormals);
    glBindBuffer(GL_ARRAY_BUFFER, mVBONormals);
    glBufferStorage(GL_ARRAY_BUFFER, normalsByteSize, 0, GL_MAP_WRITE_BIT);
    return static_cast<DRVector3*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
}

u32* DRVertexBuffer::getIndicesBufferPtr(size_t indicesCount)
{
    assert(mVBOIndices == 0);
    mIndexCount = indicesCount;
    auto indicesByteSize = indicesCount * sizeof(u32);
    GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    glGenBuffers(1, &mVBOIndices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVBOIndices);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, indicesByteSize, 0, GL_MAP_WRITE_BIT);
    return static_cast<u32*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
}

DRReturn DRVertexBuffer::Init(const std::vector<DetailedVertex>& vertexData, const std::vector<int>& indices)
{
    if (vertexData.empty()) return DR_ERROR;

    GLuint indexBufferID;
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    setVertexData(vertexData);

    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    mVertexCount = static_cast<uint32_t>(vertexData.size());
    mIndexCount = static_cast<uint32_t>(indices.size());    
    
    glBindVertexArray(0);
    return DR_OK;
}

DRReturn DRVertexBuffer::Render(GLenum renderMode)
{
    glBindVertexArray(mVAO);
    glDrawArrays(renderMode, 0, mVertexCount);
    glBindVertexArray(0);
	return DR_OK;
}

DRReturn DRVertexBuffer::RenderIndex(GLenum renderMode, uint32_t indexCount, u32* pIndices)
{
    glBindVertexArray(mVAO);
    glDrawElements(renderMode, indexCount, GL_UNSIGNED_INT, pIndices);
    glBindVertexArray(0);
	return DR_OK;
}

DRReturn DRVertexBuffer::RenderIndex(GLenum renderMode, uint32_t indexCount, u16* pIndices)
{
    glBindVertexArray(mVAO);
    glDrawElements(renderMode, indexCount, GL_UNSIGNED_SHORT, pIndices);
    glBindVertexArray(0);
    return DR_OK;
}

DRReturn DRVertexBuffer::RenderIndex(GLenum renderMode)
{
    glBindVertexArray(mVAO);
    if (mVBONormals) {
        glBindBuffer(GL_ARRAY_BUFFER, mVBONormals);
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, 0, (void*)0);
    }
    glDrawElements(renderMode, mIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    return DR_OK;
}

void DRVertexBuffer::Exit()
{
    if (mVAO) {
        glDeleteVertexArrays(1, &mVAO);
    }
}

void DRVertexBuffer::setVertexData(const std::vector<DetailedVertex>& vertexData)
{
    size_t offset = 0;
    int attribIndex = 0;

    glBufferData(GL_ARRAY_BUFFER, sizeof(DetailedVertex) * vertexData.size(), vertexData.data(), GL_STATIC_DRAW);

    // Position (vec3)
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(DetailedVertex), (void*)offset);
    glEnableVertexAttribArray(attribIndex++);
    offset += sizeof(DRVector3);

    // Normal (vec3)
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(DetailedVertex), (void*)offset);
    glEnableVertexAttribArray(attribIndex++);
    offset += sizeof(DRVector3);

    // VolumenTexCoord (vec3)
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, sizeof(DetailedVertex), (void*)offset);
    glEnableVertexAttribArray(attribIndex++);
    offset += sizeof(DRVector3);

    // vTex (vec2)
    glVertexAttribPointer(attribIndex, 2, GL_FLOAT, GL_FALSE, sizeof(DetailedVertex), (void*)offset);
    glEnableVertexAttribArray(attribIndex++);
    offset += sizeof(DRVector2);

    // vTexDetail (vec2)
    glVertexAttribPointer(attribIndex, 2, GL_FLOAT, GL_FALSE, sizeof(DetailedVertex), (void*)offset);
    glEnableVertexAttribArray(attribIndex++);
    offset += sizeof(DRVector2);

    // Color (vec4)
    glVertexAttribPointer(attribIndex, 4, GL_FLOAT, GL_FALSE, sizeof(DetailedVertex), (void*)offset);
    glEnableVertexAttribArray(attribIndex++);
    offset += sizeof(DRColor);
}