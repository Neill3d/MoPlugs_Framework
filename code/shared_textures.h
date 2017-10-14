
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_textures.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//-- 
#include <GL\glew.h>

//
#include "algorithm\nv_math.h"

#include "graphics\OGL_Utils.h"
#include "graphics\CheckGLError.h"

#include "shared_glsl.h"
#include "shared_common.h"

#include "gpucache_types.h"


////////////////////////////////////////////////////////////////////////////////
// forward declaration
class CGPUCacheLoaderVisitorImpl;


///////////////////////////////////////////////////////////////////////////////////
//

struct CGPUImageSequencer
{
public:
	//! a constructor
	CGPUImageSequencer();
	//! a copy constructor
	CGPUImageSequencer( const CGPUImageSequencer &sequencer );
	//! a destructor
	~CGPUImageSequencer();
	
	void Clear();

	ImageSequenceHeader2 &GetHeader();
	
	void *&GetFrames();
	
public:

	//ImageHeader2			header;
	ImageSequenceHeader2	seqHeader;

	//GLint		sequenceSize;	// total size on bytes of sequence frames
	void		*frames;	// pointer to frame by frame data (compressed or uncompressed)

	// animation control over the sequencer
	int			currentFrame;
	double		currentFrameTime;
	double		mFirstRunningTime;
	
	//
	void Evaluate( const double time );

	// DONE: update should replace existing texture data (like glTexSubImage2D...)
	void UpdateTexture( GLuint *texId, const bool needCompressOnLoad, const bool needMipmapsOnLoad );

private:

	int			mLastUpdateFrame;	// store on which frame we made our last update

	GLuint		mTextureId;

};

////////////////////////////////////////////////////////////////////////////////////
//
class CTexturesReference : public CResourceGPUModel<TextureGLSL>
{
public:

	//! a constructor
	CTexturesReference();

	//! a destructor
	virtual ~CTexturesReference();

	// 
	// for all FBScene textures, get gpu pointers 
	//
	void InitializeTextures();

	void Allocate(const int numberOfSamples, const int numberOfImages);

	//
	// delete ogl textures and ogl samplers
	//
	void Free();

	//
	// free and clear all arrays (names, dimentions, samplers, etc.)
	//
	void Clear();

	//
	// go throw all loaded textures and get gpu pointers
	//
	// complex update
	void UpdateTexturePointers();

	
	//
	// work with gpu pointers, make resident and non resident
	//
	virtual void Lock() override;
	virtual void UnLock() override;

	// OR - work with cache

	const int		GetNumberOfCachedTextures() const;
	const GLint		GetTextureId(const int index) const;
	const GLuint	GetSamplerId(const int index) const;
	void			GetTextureSize(const int index, int &width, int &height) const;
	const char		*GetTextureIdName(const int index) const;

	// work with handles
	const int GetNumberOfTextureHandles() const;
	const GLuint64	*GetTexHandles() const;

	//
	//

	bool	EvaluateFrameAnimation(const double time);
	void	UpdateSequences();

protected:

	//
	// OR - local pre-cached texture storage
	std::vector<GLuint>				mResourceIds;	// source image data for textures
	std::vector<CGPUImageSequencer>	mResourceFrames;

	// each parameter per texture (sampler, source image, matrix, dimentions, names)

	std::vector<GLuint>				mTextureSamplers;		// samplers for textures (1 sampler per texture)
	std::vector<int>				mTextureResources;		// index for resource per texture
	std::vector<mat4>				mTextureMatrix;
	std::vector<vec2>				mTextureDimentions;
	std::vector<std::string>		mTextureNames;			// list of resulted textures

	friend class CGPUCacheLoaderVisitorImpl;
};
