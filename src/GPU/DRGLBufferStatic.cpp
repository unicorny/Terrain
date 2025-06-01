#include "DRGLBufferStatic.h"
#include "Exceptions.h"

#include "DREngine/DRLogging.h"

#include "magic_enum/magic_enum.hpp"
#include "magic_enum/magic_enum_flags.hpp"

using namespace magic_enum;
using namespace magic_enum::bitwise_operators;

namespace GPU {
	DRGLBufferStatic::DRGLBufferStatic(GLsizei elementCount)
		: mType(DRGLBufferType::NONE), mVBO(0), 
		mElementCount(elementCount), mRawBufferPointer(nullptr),
		mIsPrepared(false), mIsReadyForRender(false)
	{

	}

	DRGLBufferStatic::~DRGLBufferStatic()
	{
		if (mRawBufferPointer) {
			unmap();
			LOG_WARNING("Buffer was still mapped on deconstruct");
		}
		if (mVBO) {
			glDeleteBuffers(1, &mVBO);
		}
	}

	void DRGLBufferStatic::createOpenGLBuffer(DRGLBufferType type)
	{
		init(type);
		size_t elementSize = 0;
		switch (type) {
		case DRGLBufferType::NONE:
			throw DRGLBufferWrongType("[GPU::DRGLBufferStatic::createOpenGLBuffer] define buffer type other as NONE");
		case DRGLBufferType::VERTEX_BUFFER:
		case DRGLBufferType::NORMALS_BUFFER:
		case DRGLBufferType::TEXTURE_3D_BUFFER:
			elementSize = sizeof(DRVector3);
			break;
		case DRGLBufferType::COLOR_BUFFER:
			elementSize = sizeof(DRColor);
			break;
		case DRGLBufferType::TEXTURE_2D_BUFFER:
			elementSize = sizeof(DRVector2);
			break;
		case DRGLBufferType::INDICES_BUFFER:
			elementSize = sizeof(u32);
			break;
		case DRGLBufferType::INDICES_SHORT_BUFFER:
			elementSize = sizeof(u16);
			break;
		default: 
			throw DRGLBufferWrongType("[GPU::DRGLBufferStatic::createOpenGLBuffer] buffer type not implemented");
		}
		prepareBuffer(elementSize * mElementCount);
	}

	DRColor* DRGLBufferStatic::getColorPtr()
	{
		init(DRGLBufferType::COLOR_BUFFER);
		return static_cast<DRColor*>(prepareBuffer(sizeof(DRColor) * mElementCount));
	}

	DRVector3* DRGLBufferStatic::getVector3Ptr(DRGLBufferType type)
	{
		if (!enum_flags_test(DRGLBufferType::VERTEX_BUFFER | DRGLBufferType::TEXTURE_3D_BUFFER | DRGLBufferType::NORMALS_BUFFER, type)) {
			throw DRGLBufferWrongType("[GPU::DRGLBufferStatic::getVector3Ptr] allow only VERTEX_BUFFER, TEXTURE_3D_BUFFER or NORMALS_BUFFER");
		}
		init(type);
		return static_cast<DRVector3*>(prepareBuffer(sizeof(DRVector3) * mElementCount));
	}

	DRVector2* DRGLBufferStatic::getVector2Ptr()
	{
		init(DRGLBufferType::TEXTURE_2D_BUFFER);
		return static_cast<DRVector2*>(prepareBuffer(sizeof(DRVector2) * mElementCount));
	}

	u32* DRGLBufferStatic::getIndicesPtr()
	{
		init(DRGLBufferType::INDICES_BUFFER);
		return static_cast<u32*>(prepareBuffer(sizeof(u32) * mElementCount));
	}
	u16* DRGLBufferStatic::getShortIndicesPtr()
	{
		init(DRGLBufferType::INDICES_SHORT_BUFFER);
		return static_cast<u16*>(prepareBuffer(sizeof(u16) * mElementCount));
	}

	void DRGLBufferStatic::unmap()
	{
		if (!mVBO) {
			throw DRGLBufferNotInitalized(
				"[GPU::DRGLBufferStatic::unmap] unmap cannot be called, before calling either of these: getColorPtr, getVector3Ptr, getVector2Ptr, getIndicesPtr, getShortIndicesPtr"
			);
		}
		auto type = getTargetByType();
		glBindBuffer(type, mVBO);
		glUnmapBuffer(type);
		throwOnOpenGLError();

		mRawBufferPointer = nullptr;
		mIsReadyForRender = true;
	}

	void DRGLBufferStatic::bind()
	{
		// don't check if vbo was initalized for performance reasons
		glBindBuffer(getTargetByType(), mVBO);
		throwOnOpenGLError();
	}

	void DRGLBufferStatic::init(DRGLBufferType type)
	{
		if (!mVBO) {
			glGenBuffers(1, &mVBO);
			throwOnOpenGLError();
		}
		if (mType == DRGLBufferType::NONE) {
			mType = type;
		}
		else {
			if (mType != type) {
				throw DRGLBufferWrongType("[GPU::DRGLBufferStatic] changing type isn't supported");
			}
		}
	}

	void* DRGLBufferStatic::prepareBuffer(GLsizeiptr byteSize)
	{
		if (!mRawBufferPointer) {
			auto target = getTargetByType();
			glBindBuffer(target, mVBO);
			glBufferStorage(target, byteSize, 0, GL_MAP_WRITE_BIT);
			mRawBufferPointer = glMapBuffer(target, GL_WRITE_ONLY);
			throwOnOpenGLError(); 
		}
		mIsPrepared = true;
		return mRawBufferPointer;
	}
	GLenum DRGLBufferStatic::getTargetByType()
	{
		switch (mType) {
		case DRGLBufferType::NONE: 
			throw DRGLBufferWrongType("[GPU::DRGLBufferStatic::getTargetByType] type is not set");
		case DRGLBufferType::INDICES_BUFFER: 
		case DRGLBufferType::INDICES_SHORT_BUFFER:
			return GL_ELEMENT_ARRAY_BUFFER;
		}
		return GL_ARRAY_BUFFER;
	}
}