#include "DRVertexBuffer.h"
#include "DRCore2/Utils/DRProfiler.h"
#include "DREngine/DRLogging.h"
#include <type_traits>

static_assert(std::is_same<DRReal, float>::value, "DRReal ist NICHT float! Rendering erwartet float-basiertes Layout!");

DRVertexBuffer::DRVertexBuffer()
	: mVAO(0), mVBONormals(0), mVertexCount(0), mIndexCount(0)
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

DRReturn DRVertexBuffer::Init(const std::vector<DRVector3>& vertices, const std::vector<unsigned int>& indices)
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
}

DRReturn DRVertexBuffer::Init(const std::vector<DRVector3>& vertices, const std::vector<DRVector3>& normals, const std::vector<unsigned int>& indices)
{
    if (vertices.empty()) return DR_ERROR;

    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    GLuint vbo;
    int attribIndex = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    auto verticesByteSize = sizeof(DRVector3) * vertices.size();
    glBufferStorage(GL_ARRAY_BUFFER, verticesByteSize, 0, flags);
    void* p = glMapBufferRange(GL_ARRAY_BUFFER, 0, verticesByteSize, flags);
    DRProfiler timeUsed;
    memcpy(p, vertices.data(), verticesByteSize);
    DRLog.writeToLog("[DRVertexBuffer] %s time for memcpy vertices", timeUsed.string().data());
    //glBufferData(GL_ARRAY_BUFFER, sizeof(DRVector3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(attribIndex++);

    if (normals.size()) {
        glGenBuffers(1, &mVBONormals);
        glBindBuffer(GL_ARRAY_BUFFER, mVBONormals);
        auto normalsByteSize = sizeof(DRVector3) * normals.size();
        glBufferStorage(GL_ARRAY_BUFFER, normalsByteSize, 0, flags);
        void* p = glMapBufferRange(GL_ARRAY_BUFFER, 0, normalsByteSize, flags);
        timeUsed.reset();
        memcpy(p, normals.data(), normalsByteSize);
        DRLog.writeToLog("[DRVertexBuffer] %s time for memcpy normals", timeUsed.string().data());
        // glBufferData(GL_ARRAY_BUFFER, sizeof(DRVector3) * normals.size(), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_TRUE, 0, 0);
        glEnableVertexAttribArray(attribIndex++);
    }

    if (indices.size()) {
        GLuint indexBufferID;
        glGenBuffers(1, &indexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
        auto indicesByteSize = sizeof(int) * indices.size();
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, indicesByteSize, 0, flags);
        void* p = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, indicesByteSize, flags);
        timeUsed.reset();
        memcpy(p, indices.data(), indicesByteSize);
        DRLog.writeToLog("[DRVertexBuffer] %s time for memcpy indices", timeUsed.string().data());
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);
        mIndexCount = static_cast<uint32_t>(indices.size());
    }

    mVertexCount = static_cast<uint32_t>(vertices.size());
    glBindVertexArray(0);
    return DR_OK;
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