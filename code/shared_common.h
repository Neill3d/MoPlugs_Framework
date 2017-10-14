
#pragma once

#include "graphics\OGL_Utils.h"
#include "graphics\UniformBuffer.h"
#include <vector>
#include <map>

////////////////////////////////////////////
//

#define UPDATE_LEVEL_NONE			0
#define UPDATE_LEVEL_PARAMETERS		1
#define UPDATE_LEVEL_CONNECTIONS	2

////////////////////////////////////////////////////////////////////////////////////////////////
// common class for resources, used for gpu cache and motionbuilder scene caching

// !!! transfer all updated nodes into GPU
// if has animatable properties, mark as dirty each frame

template <typename T>
class CResourceGPUModel
{
public:

	virtual void Allocate(const int count)
	{
		mGPUData.resize(count);
		mNumberOfItems = count;
		mNumberOfBaseShaders = count;
	}

	// free memory
	virtual void Free()
	{
		mNumberOfBaseShaders = 0;
	}

	// clear all data to zero
	virtual void Clear()
	{
		mGPUData.clear();
		mNumberOfItems = 0;
		mNumberOfBaseShaders = 0;
	}

public:

	//! a constructor
	CResourceGPUModel<T>()
	{
		mNeedGPUUpdate = false;
		mNumberOfItems = 0;
		mNumberOfBaseShaders = 0;
	}

	//! a destructor
	virtual ~CResourceGPUModel<T>()
	{}

	/////////// render stage

	// ! for internal use - move data into buffer memory

	// for material I should know texture and video clip gpu buffer indices

	void PrepRender()
	{
		if (mNeedGPUUpdate)
		{
			mBuffer.UpdateData( sizeof(T), mGPUData.size(), mGPUData.data() );
			mNeedGPUUpdate = false;
		}
	}

	// for public use

	virtual void Lock()
	{}
	virtual void UnLock()
	{}

	void BindAsUniform(const GLuint location)
	{
		mBuffer.BindAsUniform(0, location, 0);
	}
	void BindAsUniform(const int program, const GLuint location)
	{
		mBuffer.BindAsUniform(program, location, 0);
	}
	void BindSpecifiedAsUniform(const GLuint program, const GLuint location, const int index)
	{
		mBuffer.BindAsUniform(program, location, sizeof(T) * index);
	}

	void BindAsAttribute(const GLuint location)
	{
		// Pass a GPU pointer to the vertex buffer for the per mesh uniform data via a vertex attribute
		mBuffer.BindAsAttribute( location, 0 );
	}
	void BindSpecifiedAsAttrib(const GLuint location, const int index)
	{
		mBuffer.BindAsAttribute( location, sizeof(T) * index );
	}

	void UnBind()
	{
		mBuffer.UnBind();
	}

	const GLuint64 GetGPUPtr(const int id) const
	{
		if (id > 0 && id >= (int)mGPUData.size() )
		{
			printf( "error\n" );
		}

		if ( mBuffer.GetGPUPtr() > 0 )
		{
			return mBuffer.GetGPUPtr() + sizeof(T) * id;
		}

		return 0;
	}

	const size_t GetBaseSize() const {
		return mNumberOfBaseShaders;
	}

	const size_t GetDataSize() const {
		return mGPUData.size();
	}

	T *GetDataPtr() {
		return mGPUData.data();
	}

	T &GetDataItem(const int index) {
		return mGPUData[index];
	}

	T &GetDataItem(const size_t index) {
		return mGPUData[index];
	}

	void AddItem(const T &data) {
		mGPUData.push_back(data);
	}

	void SetNumberOfBaseShaders(const size_t count) {
		mNumberOfBaseShaders = count;
	}

	void		CauseAGPUUpdate()
	{
		mNeedGPUUpdate = true;
	}
	const bool IsNeedAGPUUpdate() const {
		return mNeedGPUUpdate;
	}

protected:

	bool							mNeedGPUUpdate;

	size_t							mNumberOfBaseShaders;
	std::vector<T>					mGPUData;

	// final render buffer

	size_t							mNumberOfItems;
	CGPUBufferNV					mBuffer;

};