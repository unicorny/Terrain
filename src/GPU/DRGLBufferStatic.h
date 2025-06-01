#ifndef __DR_GPU_GL_BUFFER_H
#define __DR_GPU_GL_BUFFER_H

#include "Exceptions.h"

#include "DRCore2/Foundation/DRVector3.h"
#include "DRCore2/Foundation/DRVector2.h"
#include "DRCore2/Foundation/DRColor.h"
#include "DREngine/OpenGL.h"
#include "DRCore2/DRTypes.h"

#include <atomic>

namespace GPU {
	enum class DRGLBufferType : u8
	{
		NONE = 0,
		VERTEX_BUFFER = 1,
		NORMALS_BUFFER = 2,
		COLOR_BUFFER = 4, 
		TEXTURE_2D_BUFFER = 8,
		TEXTURE_3D_BUFFER = 16,
		INDICES_BUFFER = 32,
		INDICES_SHORT_BUFFER = 64
	};

	/*
	* \brief For Static OpenGL Buffers, directly fill on load/generate, unmap and than bind before draw
	* Example: 
	* \code
	* GLsizei vertexCount = 1024;
	* DRGLBufferStatic buffer(vertexCount);
	* DRVector3* vertices = buffer.getVector3Ptr(DRGLBufferType::VERTEX_BUFFER);
	* // fill vertices 
	* vertices[0] = DRVector3(1.0, 0.0, 1.0);
	* ...
	* vertices[vertexCount] = DRVector3(-1.0, 0.0, 0.0);
	* buffer.unmap();
	* // use before draw call
	* buffer.bind();
	* glVertexPointer(3, GL_FLOAT, 0, nullptr);
	* glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	* \endcode
	*/
	
	class DRGLBufferStatic 
	{
	public:
		DRGLBufferStatic(GLsizei elementCount);
		~DRGLBufferStatic();

		// create buffer in opengl, call only in same thread in which opengl context was created
		void createOpenGLBuffer(DRGLBufferType type);

		// use pointer calls only in other threads after calling createOpenGLBuffer
		// only for type COLOR_BUFFER
		DRColor* getColorPtr();
		// for type VERTEX_BUFFER, NORMALS_BUFFER, TEXTURE_3D_BUFFER
		DRVector3* getVector3Ptr(DRGLBufferType type);
		// for type TEXTURE_2D_BUFFER
		DRVector2* getVector2Ptr();
		// for type INDICES_BUFFER
		u32* getIndicesPtr();
		// for type INDICES_SHORT_BUFFER
		u16* getShortIndicesPtr();
		// call after data was transfered
		void unmap();
		// call to use it in draw call, don't check if Buffer is ready, please call only after uploading data and call to unmap!
		void bind();

		//! \brief return true if a buffer was created
		inline bool isPrepared() const { return mIsPrepared; }
		//! \brief return true if unmap was called in buffer should be therfore filled and ready for used in rendering
		inline bool isReadyForRender() const { return mIsReadyForRender; }

		inline GLsizei getElementCount() const { return mElementCount; }

	protected:
		// will be called from get Ptr functions, will call glGenBuffer if mVBO = 0
		// will set type if none and else check type
		void init(DRGLBufferType type);
		//! create and map buffer if not already exist, else simply return pointer
		//! after unmap call, mRawBufferPointer will be resetted and a additional call to this will
		//! create a new buffer and replace the old one
		//! \param target expect GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER
		void* prepareBuffer(GLsizeiptr byteSize);
		GLenum getTargetByType();

		DRGLBufferType mType;
		GLuint mVBO;
		GLsizei mElementCount;
		void* mRawBufferPointer;
		std::atomic<bool> mIsPrepared;
		std::atomic<bool> mIsReadyForRender;
	};

	// will be thrown if DRGLBuffer is used with different types
	class DRGLBufferWrongType : LogicalException
	{
	public:
		using LogicalException::LogicalException;
	};

	// will be thrown if DRGLBuffer wasn't properly initalized before
	class DRGLBufferNotInitalized : LogicalException
	{
	public:
		using LogicalException::LogicalException;
	};
}

#endif //__DR_GPU_GL_BUFFER_H
