
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_projectors.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


/*

	CProjectedData

	support for up to 6 projecto textures
	and 2 dynamic masks

	features
	- compose 6 projectors and 2 masks into one 2d texture array
	- one model class for upload data to the Cg shader
	- one model class for upload data to the Nv bindless uniform buffer

	Data-Model

	Data prepare from MoBu needed information about projections
	Model can work with Cg or Nv uniform buffer to upload data into the GPU

*/

#pragma once

#include <GL\glew.h>
//
#include "algorithm\nv_math.h"

#ifdef	USE_CG
#include <Cg/cg.h>
#endif

#include "graphics\OGL_Utils.h"
#include "graphics\glslShader.h"

#include "graphics\UniformBuffer.h"

#ifndef MAX_PROJECTORS_COUNT
#define MAX_PROJECTORS_COUNT	6
#endif 

////////////////////////////////////////////////////////////////////////////

// projectors data
typedef struct 
{
		mat4	matrix;
	
		float	textureLayer;	// layer in 2d array
		float	maskLayer;		// layer in 2d array
		int		maskChannel;	// x,y,z,w from a mask sampler
		float	blendOpacity;
		
		float	blendMode;
		
		float	dummyPRJ1;
		float	dummyPRJ2;
		float	dummyPRJ3;
} ProjectorDATA;

typedef struct
{
	ProjectorDATA		projectors[MAX_PROJECTORS_COUNT];
	int					numberOfProjectors;		// binded number of projectors for that shader
} ProjectorSet;

void ProjectorDATA_Set(	const int mask, 
						const int channel, 
						const int blendmode, 
						const float opacity, 
						ProjectorDATA &data);

void ProjectorDATA_Zero(ProjectorDATA &data);

////////////////////////////////////////////////////////////////////////////
// render all projectors and masks into one texture

class CGPUProjectorsCompose
{
public:
	//! a constructor
	CGPUProjectorsCompose(bool initOGL=true);
	//~ a destructor
	~CGPUProjectorsCompose();

	void Free();


	void Compose(	const int w, const int h, 
					const ProjectorSet &collection, 
					const GLuint _mask1, 
					const GLuint _mask2);


	GLuint		GetComposeTextureID() const
	{
		return mTextureArrayId;
	}

	void InitProjectionTexture(const int w, const int h);

protected:

	// 
	GLuint								mFrameBufferId;

	bool InitFrameBuffer();
	void BeginProjectorsCompose(const int w, const int h, const GLuint textureId);
	void EndProjectorsCompose();

private:

	FrameBufferInfo		mFrameBufferInfo;	// info for storing/restoring state of a framebuffer

	GLuint		defaultTexId;

	GLuint		mTextureArrayId;

	GLuint		vbo;
	GLuint		vao;

	static	GLSLShader		*mShader;
	static	int				mShaderCounter;

	void PrepareDefaultTexture();
	bool SetupVBO();
};


////////////////////////////////////////////////////////////////////////////////
// MODEL - use data in specified model

// approached to use projectors on gpu (by Cg or by Nv bindless uniform buffer)
class CGPUProjectorsModel
{
public:
	//! a constructor
	CGPUProjectorsModel()
	{}
	//! a destructor
	virtual ~CGPUProjectorsModel()
	{}

	virtual void Prep(const int numberOfProjectors, const ProjectorSet &collection)
	{}

protected:

};

///////////////////////////////////////////////////////////////////////////////////
//
#ifdef USE_CG

class CGPUProjectorsModelCg : public CGPUProjectorsModel
{
public:

	//! a constructor
	CGPUProjectorsModelCg(/*CGPUProjectorsCompose *_compose*/);
	//
	virtual ~CGPUProjectorsModelCg();

	//void	AssignCompose(CGPUProjectorsCompose *_compose);

	bool	InitParams(CGcontext context, CGprogram program);

	// upload to gpu shader all needed projector parameters
	virtual void UploadBuffer( const ProjectorSet &buffer );

	bool	Bind(const int numberOfProjectors, const GLuint *projIds, const int numberOfMasks, const GLuint *maskIds);
	void	UnBind();

protected:

	UniformBufferCG				mBuffer;

	//
	
	CGparameter					mParamProjSamplers[6];
	CGparameter					mParamMaskSamplers[2];

private:

	//CGPUProjectorsCompose		*mCompose;	// compose up to 6 projector textures and 2 masks into one 2d array

};

#endif

///////////////////////////////////////////////////////////////////////////////////
//
class CGPUProjectorsModelNv : public CGPUProjectorsModel
{
public:

	//! a constructor
	CGPUProjectorsModelNv();
	//
	~CGPUProjectorsModelNv();

	virtual void Prep( const int numberOfProjectors, const ProjectorSet &collection );

public:

	// bind masks and projectors to texture units from TEXTURE7 to TEXTURE15 (8 units in total)
	void	BindProjectionMapping(const int numberOfProjectors, const ProjectorSet &collection, const GLuint mask1, const GLuint mask2) const;
	static void	UnBindProjectionMapping();

	void BindBuffer(const GLuint unitId);
	void BindAsUniform(const GLuint programId, const GLuint locationId) const;

	void ChangeContext() {
		mBufferProjectors.Free();
	}

protected:

	struct projectorDATA
	{
		mat4		matrix;				// texture projection matrix

		float		textureLayer;		// index in texture2darray
		float		maskLayer;			// index in texture2darray
		int			maskChannel;		// x,y,z,w channel of a mask layer
		float		blendOpacity;		// projected texture opacity
		float		blendMode;			// photoshop blend mode

		vec3		dummy;
	};

	struct projectorsBlock
	{
		projectorDATA		Projectors[MAX_PROJECTORS_COUNT];
	};
	projectorsBlock			g_projectorsBlock;

	CGPUBufferNV			mBufferProjectors;
};