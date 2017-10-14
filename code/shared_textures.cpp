
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_textures.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_textures.h"

#include "graphics\CheckGLError.h"
#include "graphics\OGL_Utils.h"

#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#include <limits>

const bool gGenereateMipMaps = true;


extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

bool CTexturesReference::EvaluateFrameAnimation(const double time)
{
	for (auto it=mResourceFrames.begin(); it!=mResourceFrames.end(); ++it)
	{
		if (it->GetFrames() != nullptr)
			it->Evaluate(time);
	}

	return true;
}

void CTexturesReference::UpdateSequences()
{
	for (auto it=mResourceFrames.begin(); it!=mResourceFrames.end(); ++it)
	{
		if (it->GetFrames() != nullptr)
			it->UpdateTexture(nullptr, false, false);
	}
}

CTexturesReference::CTexturesReference()
	: CResourceGPUModel<TextureGLSL>()
{
}

CTexturesReference::~CTexturesReference()
{
	Free();
}

const int CTexturesReference::GetNumberOfTextureHandles() const
{
	return (int) mGPUData.size();
}

const GLuint64 *CTexturesReference::GetTexHandles() const
{
	// not supported
	return nullptr;
	//return mTextureHandles
}


void CTexturesReference::Free()
{
	for (size_t i=0; i<mResourceIds.size(); ++i)
	{
		GLuint id = mResourceIds[i];
		if (id > 0)
			glDeleteTextures( 1, &id );
	}
	mResourceIds.clear();

	for (size_t i=0; i<mResourceFrames.size(); ++i)
	{
		mResourceFrames[i].Clear();
	}

	if (mTextureSamplers.size() > 0)
	{
		glDeleteSamplers( (int) mTextureSamplers.size(), mTextureSamplers.data() );
		mTextureSamplers.clear();
	}	
}

void CTexturesReference::Clear()
{
	mTextureResources.clear();
	
	//
	Free();
	
	//
	mTextureMatrix.clear();
	mTextureDimentions.clear();
	mTextureNames.clear();
}


void CTexturesReference::UpdateTexturePointers()
{
	//mProgress.Text = "Update textures in GPU memory";
	mGPUData.clear();

	if (mTextureResources.size() > 0)
	{
		// update pointers from pre-cached data
		mGPUData.reserve( mTextureResources.size() );

		for (size_t i=0; i<mTextureResources.size(); ++i)
		{
			const int videoIndex = mTextureResources[i];
			if (videoIndex < 0)
			{
				// TODO: apply some temp texture in that case
				//printf( "missing video source, should apply some temp texture!\n" );
				TextureGLSL texture;
				texture.address = 0;
				texture.width = 0;
				texture.height = 0;

				mGPUData.push_back(texture);
				continue;
			}
			const GLuint image = mResourceIds[ videoIndex ];
			const GLuint sampler = mTextureSamplers[i];

			GLuint64 handle = 0;
			
			if (image > 0 && sampler > 0)
			{
				handle = glGetTextureSamplerHandleARB(image, sampler);
				//handle = glGetTextureHandleARB(image);
			}

			if (handle > 0)
			{
				glMakeTextureHandleResidentARB( handle );
				glMakeTextureHandleNonResidentARB( handle );

				CHECK_GL_ERROR();
			}

			TextureGLSL	texture;
			texture.address = handle;
			texture.transform = mTextureMatrix[i];
			texture.width = mTextureDimentions[videoIndex].x;
			texture.height = mTextureDimentions[videoIndex].y;

			mGPUData.push_back(texture);

			CHECK_GL_ERROR();
		}

		CauseAGPUUpdate();
	}
}


// TODO: better to avoid  - if (handle > 0) - fill holes with blank texture

void CTexturesReference::Lock()
{
	//mProgress.Text = "Lock textures";
	GLuint64 handle=0;

	for( auto it=mGPUData.begin(); it!=mGPUData.end(); ++it )
	{
		handle = it->address;
		if (handle > 0) glMakeTextureHandleResidentARB( handle );
	}

	//printf( "Lock Textures finished\n" );
}

void CTexturesReference::UnLock()
{
	GLuint64 handle = 0;

	for( auto it=mGPUData.begin(); it!=mGPUData.end(); ++it )
	{
		handle = it->address;
		if (handle > 0) glMakeTextureHandleNonResidentARB( handle );
	}
}


//////////////////////////////////////////////////////////////////////////////////////

const int CTexturesReference::GetNumberOfCachedTextures() const
{
	return (int) mTextureResources.size();
}
const GLint CTexturesReference::GetTextureId(const int index) const
{
	return (mTextureResources[index] >=0) ? (GLint) mResourceIds[ mTextureResources[index] ] : -1;
}
void CTexturesReference::GetTextureSize(const int index, int &width, int &height) const
{
	width = (int) mTextureDimentions[index].x;
	height = (int) mTextureDimentions[index].y;
}
const GLuint CTexturesReference::GetSamplerId(const int index) const
{
	return mTextureSamplers[index];
}
const char *CTexturesReference::GetTextureIdName(const int index) const
{
	return mTextureNames[index].c_str();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IO



void CTexturesReference::Allocate(const int numberOfSamplers, const int numberOfImages)
{
	// 1 - load number of textures
	CGPUImageSequencer	defSequencer;

	mResourceIds.resize( numberOfImages, 0 );
	mResourceFrames.resize( numberOfImages, defSequencer );
	mTextureDimentions.resize( numberOfImages );
	mTextureResources.resize( numberOfSamplers );
	mTextureSamplers.resize( numberOfSamplers );
	mTextureMatrix.resize( numberOfSamplers );
	mTextureNames.resize( numberOfSamplers );

	//glGenTextures( fileHeader.numberOfImages, mResourceIds.data() );
	glGenSamplers( numberOfSamplers, mTextureSamplers.data() );
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

//! a constructor
CGPUImageSequencer::CGPUImageSequencer()
{
	//sequenceSize = 0;
	frames = nullptr;
		
	currentFrame = 0;
	mLastUpdateFrame = -1;
	mTextureId = 0;
	mFirstRunningTime = std::numeric_limits<double>::infinity();
}
//! a copy constructor
CGPUImageSequencer::CGPUImageSequencer( const CGPUImageSequencer &sequencer )
{
	//sequenceSize = sequencer.sequenceSize;
	frames = nullptr;
	currentFrame = sequencer.currentFrame;
	mTextureId = sequencer.mTextureId;
	mLastUpdateFrame = -1;
	mFirstRunningTime = std::numeric_limits<double>::infinity();
}
//! a destructor
CGPUImageSequencer::~CGPUImageSequencer()
{
	Clear();
}

void CGPUImageSequencer::Clear()
{
	if (frames)
	{
		delete [] frames;
		frames = nullptr;
	}

	currentFrame = 0;
}

ImageSequenceHeader2 &CGPUImageSequencer::GetHeader()
{
	return seqHeader;
}

void *&CGPUImageSequencer::GetFrames()
{
	return frames;
}

void CGPUImageSequencer::Evaluate( const double time )
{
	if (frames == nullptr)
		return;

	double currentTime;
	double timeStep, timeOffset;

	if (seqHeader.freeRunning > 0)
	{
		currentTime = time;
		
		if (mFirstRunningTime == std::numeric_limits<double>::infinity() )
			mFirstRunningTime = currentTime;

		currentTime = currentTime - mFirstRunningTime;
	}
	else
	{
		currentTime = time; // pInfo->GetLocalTime();
	}

	double playSpeed = seqHeader.playSpeed;
		
	if (playSpeed == 0.0) currentFrame = seqHeader.startFrame;
	else
	{
		if (seqHeader.useSystemFrameRate)
			timeStep = 1.0 / 25.0; // .SetFrame( 1 );
		else
			timeStep = 1.0 / seqHeader.frameRate;
		timeStep = timeStep * (1.0 / playSpeed);

		if (seqHeader.freeRunning == 0)
		{
			timeOffset = seqHeader.timeOffset;
			currentTime -= timeOffset;
		}

		if (seqHeader.loop > 0)
		{
			timeOffset = timeStep * (seqHeader.stopFrame - seqHeader.startFrame + 1);

			while (currentTime > timeOffset)
			{
				currentTime -= timeOffset;

				if (seqHeader.freeRunning > 0)
					mFirstRunningTime += timeOffset;
			}
		}

		currentFrameTime = currentTime;
		if (timeStep != 0.0)
			currentFrame = (int) (currentTime / timeStep);

		if (currentFrame > seqHeader.stopFrame) currentFrame = seqHeader.stopFrame;
		if (currentFrame < seqHeader.startFrame) currentFrame = seqHeader.startFrame;
	}
	

	// DONE: calculate current frame index and time
}

// DONE: update should replace existing texture data (like glTexSubImage2D...)
void CGPUImageSequencer::UpdateTexture( GLuint *texId, const bool needCompressOnLoad, const bool needMipmapsOnLoad )
{
	if (currentFrame == mLastUpdateFrame)
		return;

	BYTE *currFrameData = (BYTE*) frames;
	ImageSequenceLink *link = (ImageSequenceLink*) (frames);
	link += currentFrame;
		
	currFrameData = (BYTE*)frames + link->link;
	ImageHeader2 *frameHeader = (ImageHeader2*) currFrameData;
	currFrameData += sizeof(ImageHeader2);

	bool firstTime = false;

	if (mTextureId == 0)
	{
		firstTime = true;
		glGenTextures( 1, &mTextureId);

		if (texId != nullptr)
			*texId = mTextureId;
		//glTextureStorage2D(theTextureObject, frameHeader->numberOfLODs, frameHeader->internalFormat, frameHeader->width, frameHeader->height);
	}

	glBindTexture(GL_TEXTURE_2D, mTextureId);

	if (firstTime)
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, frameHeader->numberOfLODs-1 );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	const int uncompress_size = frameHeader->width * frameHeader->height * ((frameHeader->format == GL_RGBA) ? 4 : 3);
	bool isCompressed = (uncompress_size != frameHeader->size);

	if (isCompressed)
	{ 
		// load already compressed
		//glCompressedTextureSubImage2D(theTextureObject, 0, 0, 0, frameHeader->width, frameHeader->height, frameHeader->format, frameHeader->size, currFrameData );
		if (firstTime)
			glCompressedTexImage2D( GL_TEXTURE_2D, 0, frameHeader->internalFormat, frameHeader->width, frameHeader->height, 0, frameHeader->size, currFrameData );
		else
			glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frameHeader->width, frameHeader->height, frameHeader->internalFormat, frameHeader->size, currFrameData );
			
		currFrameData += frameHeader->size;
		ImageLODHeader2 *lodHeader = (ImageLODHeader2*) currFrameData;

		if (frameHeader->numberOfLODs == 0)
		{
			glHint( GL_GENERATE_MIPMAP_HINT, GL_FASTEST );
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			for (int i=1; i<frameHeader->numberOfLODs; ++i)
			{
				lodHeader = (ImageLODHeader2*) currFrameData;

				currFrameData += sizeof(ImageLODHeader2);
				if (firstTime)
					glCompressedTexImage2D( GL_TEXTURE_2D, i, frameHeader->internalFormat, lodHeader->width, lodHeader->height, 0, lodHeader->size, currFrameData );
				else
					glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, lodHeader->width, lodHeader->height, frameHeader->internalFormat, lodHeader->size, currFrameData );

				currFrameData += lodHeader->size;
			}
		}
	}
	else
	{
		// compressed on load
		GLint internalFormat = frameHeader->internalFormat;

		if (needCompressOnLoad)
			internalFormat = (frameHeader->format == GL_RGBA) ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB;

		if (firstTime)
			glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, frameHeader->width, frameHeader->height, 0, frameHeader->format, GL_UNSIGNED_BYTE, currFrameData );
		else
			glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, frameHeader->width, frameHeader->height, frameHeader->format, GL_UNSIGNED_BYTE, currFrameData );

		if (needCompressOnLoad)
		{
			GLint compressed = 0;
			glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed );

			if (compressed == GL_TRUE )
			{
				GLint compressed_size;

				glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );
				glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size );

				printf( "compressed size - %d\n", compressed_size );
			}
			else
			{
				printf( "failed to compress!\n" );
			}
		}

		if (needMipmapsOnLoad)
		{
			if (frameHeader->numberOfLODs == 0)
			{
				glHint( GL_GENERATE_MIPMAP_HINT, GL_FASTEST );
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else
			{
				// assign mipmaps from frame data
				// TODO:
			}
		}
	}

	glBindTexture( GL_TEXTURE_2D, 0 );

	//GLuint64 handle = glGetTextureHandleARB(texId);

	CHECK_GL_ERROR();

	//
	//

	mLastUpdateFrame = currentFrame;
}