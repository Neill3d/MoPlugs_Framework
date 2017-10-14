
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_projectors.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>

#include "shared_projectors.h"
#include "shared_misc.h"
#include "graphics\CheckGLError.h"

///////////////////////////////////////////////////////
//

void ProjectorDATA_Set(	const int mask, 
						const int channel, 
						const int blendmode, 
						const float opacity, 
						ProjectorDATA &data)
{
	data.maskLayer = (float) mask;
	data.maskChannel = channel;
	data.blendMode = (float) blendmode;
	data.blendOpacity = opacity;
}

void ProjectorDATA_Zero(ProjectorDATA &data)
{
	memset( &data, 0, sizeof(ProjectorDATA) );
	data.blendOpacity = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// CGPUProjectorsCompose

bool CGPUProjectorsCompose::InitFrameBuffer()
{
	if (mFrameBufferId == 0)
		glGenFramebuffers(1, &mFrameBufferId);
	/*
	SaveFrameBuffer();

	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	RestoreFrameBuffer();

	CHECK_GL_ERROR();
	*/
	return true;
}

void CGPUProjectorsCompose::BeginProjectorsCompose(const int w, const int h, const GLuint textureId)
{
	SaveFrameBuffer(&mFrameBufferInfo);

	if (mFrameBufferId == 0)
		InitFrameBuffer();

	CHECK_GL_ERROR();

	
	// Pass 1
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
	glViewport(0, 0, w, h);

	BindFrameBufferLayers(textureId);

	GLenum DrawBuffers[8] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};
	glDrawBuffers(8, DrawBuffers);
	CHECK_GL_ERROR();
}
void CGPUProjectorsCompose::EndProjectorsCompose()
{
	RestoreFrameBuffer(&mFrameBufferInfo);
}


CGPUProjectorsCompose::CGPUProjectorsCompose(bool initOGL)
{
	mTextureArrayId = 0;

	defaultTexId = 0;
	vbo = 0;
	vao = 0;

	if (initOGL)
	{
		PrepareDefaultTexture();
		SetupVBO();
	}
}

CGPUProjectorsCompose::~CGPUProjectorsCompose()
{
	if (mTextureArrayId > 0)
	{
		glDeleteTextures(1, &mTextureArrayId);
		mTextureArrayId = 0;
	}
	if (defaultTexId > 0)
	{
		glDeleteTextures(1, &defaultTexId);
		defaultTexId = 0;
	}
	if (vao > 0)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}
	if (vbo > 0)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}
}

bool CGPUProjectorsCompose::SetupVBO()
{
	float points[] = {
	   0.0f, 0.0f,
	   0.0f, 1.0f,
	   1.0f, 0.0f,

	   0.0f, 1.0f,
	   1.0f, 1.0f,
	   1.0f, 0.0f,
	};

	if (vbo == 0)
	{
		glGenBuffers(1, &vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData( GL_ARRAY_BUFFER, 2 * 6 * sizeof(float), &points, GL_STATIC_DRAW );
	/*
	if (vao == 0)
	{
		glGenVertexArrays (1, &vao);
	}

	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);

	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
	*/
	return true;
}

void CGPUProjectorsCompose::InitProjectionTexture(const int w, const int h)
{
	const int mipLevelCount = 1;
	const int compose_layerCount = 8;

	if (mTextureArrayId == 0)
		glGenTextures(1, &mTextureArrayId);

	glBindTexture(GL_TEXTURE_2D_ARRAY, mTextureArrayId);
	// allocate the storage
	glTexStorage3D( GL_TEXTURE_2D_ARRAY, mipLevelCount, GL_RGBA8, w, h, compose_layerCount );

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	CHECK_GL_ERROR();
}

void CGPUProjectorsCompose::PrepareDefaultTexture()
{
	if (defaultTexId == 0)
		glGenTextures( 1, &defaultTexId );

	glBindTexture(GL_TEXTURE_2D, defaultTexId);

	unsigned char *pixels = new unsigned char[8*8*4];
	memset(pixels, 255, sizeof(unsigned char) * 8 * 8 * 4 );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels );

	glBindTexture( GL_TEXTURE_2D, 0 );
}

/*
	if (mSuccess && mShaderComposite == nullptr) 
	{
		mShaderComposite = CGPUFBScene::instance().GetCompositeShader();

		if (mShaderComposite == nullptr)
		{
			mSuccess = false;
			return;
		}
	}

	const int compose_width = 1024;
	const int compose_height = 1024;

	if (mProjectorsManager == nullptr) 
	{
		mProjectorsManager = new CGPUProjectorsManager(true);
		mProjectorsManager->InitProjectionTexture(compose_width, compose_height);
	}

	ProjectorCollection		collection;

	int realNumberOfProjectors=0;
	
	for (int i=0; i<MAX_NUMBER_OF_PROJECTORS; ++i)
	{
		ProjectorData::Zero( collection[i] );
		// projector active means: 1) use flag on; 2) camera assigned; 3) foreground video clip assigned
		FBCamera* lProjector = nullptr;
		if (Projectors[i].ProjectorUse)
		{
			for(int j = 0; j < Projectors[i].Projector.GetCount(); ++j)
			{
				FBCamera* lFBCamera = FBCast<FBCamera>(Projectors[i].Projector.GetAt(j)->GetHIObject());
				if (lFBCamera)
				{
					FBVideo *pVideo = lFBCamera->ForeGroundMedia;
					if ( (Projectors[i].Texture.GetCount() > 0) || (pVideo != nullptr))
						lProjector = lFBCamera;
				}
			}
		}

		if (lProjector == nullptr) continue;

		FBTexture *lTexture = (FBTexture*) ((Projectors[i].Texture.GetCount() > 0) ? Projectors[i].Texture.GetAt(0) : nullptr);
		EBlendType blendType = Projectors[i].BlendMode;
		ProjectorData::Set( lProjector, lTexture, Projectors[i].ProjectorAspect, Projectors[i].Mask, Projectors[i].MaskChannel, (int) blendType, 0.01f * Projectors[i].BlendOpacity, collection[realNumberOfProjectors] );
		realNumberOfProjectors++;
	}
	
	if (mProjectorsManager)
	{
		mProjectorsManager->Prep( realNumberOfProjectors, collection, (Mask1.GetCount()) ? (FBTexture*)Mask1.GetAt(0) : nullptr, (Mask2.GetCount()) ? (FBTexture*)Mask2.GetAt(0) : nullptr );
		CHECK_GL_ERROR();
		if (realNumberOfProjectors > 0)
		{
			BeginProjectorsCompose( compose_width, compose_height, mProjectorsManager->GetComposeTextureID() );

			mShaderComposite->SetTechnique( nullptr, Graphics::eCompositeTechniqueProjections );
			mShaderComposite->Bind();

			CHECK_GL_ERROR();
			mProjectorsManager->Compose(compose_width, compose_height);
			CHECK_GL_ERROR();

			mShaderComposite->UnBind();

			EndProjectorsCompose();
		
			CHECK_GL_ERROR();
		}
	}
	*/

void CGPUProjectorsCompose::Compose(const int w, const int h, const ProjectorSet &collection, const GLuint _mask1, const GLuint _mask2)
{
	// bind default texture if not assigned

	int texId = 0;

	for (int i=0; i<MAX_PROJECTORS_COUNT; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);

//		texId = collection.projectors[i].textureId;
		if (texId == 0) texId = defaultTexId;

		glBindTexture(GL_TEXTURE_2D, texId);
	}

	CHECK_GL_ERROR();

	// bind masks

	glActiveTexture(GL_TEXTURE6);
	texId = _mask1;
	if (texId == 0) texId = defaultTexId;

	glBindTexture(GL_TEXTURE_2D, texId);

	glActiveTexture(GL_TEXTURE7);
	texId = _mask2;
	if (texId == 0) texId = defaultTexId;

	glBindTexture(GL_TEXTURE_2D, texId);

	CHECK_GL_ERROR();
	 
	//
	// draw quad
	//
	glDisable(GL_DEPTH_TEST);
	glEnableVertexAttribArray (0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	//glBindVertexArray (vao);

	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glDrawArrays (GL_TRIANGLES, 0, 6);
		
	//glBindVertexArray (0);
	glDisableVertexAttribArray(0);
	glEnable(GL_DEPTH_TEST);
	// finish

	for (int i=0; i<8; ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE0);

	CHECK_GL_ERROR();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// CGPUProjectorsModel


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CGPUProjectorsModelCg

#ifdef USE_CG

CGPUProjectorsModelCg::CGPUProjectorsModelCg(/*CGPUProjectorsCompose *_compose*/)
	: CGPUProjectorsModel()
	//, mCompose(_compose)
{
}

CGPUProjectorsModelCg::~CGPUProjectorsModelCg()
{
}
/*
void CGPUProjectorsModelCg::AssignCompose(CGPUProjectorsCompose *_compose)
{
	mCompose = _compose;
}
*/
bool CGPUProjectorsModelCg::InitParams(CGcontext cgcontext, CGprogram program)
{
	mParamProjSamplers[0] = cgGetNamedParameter( program, "projSampler1" );
	mParamProjSamplers[1] = cgGetNamedParameter( program, "projSampler2" );
	mParamProjSamplers[2] = cgGetNamedParameter( program, "projSampler3" );
	mParamProjSamplers[3] = cgGetNamedParameter( program, "projSampler4" );
	mParamProjSamplers[4] = cgGetNamedParameter( program, "projSampler5" );
	mParamProjSamplers[5] = cgGetNamedParameter( program, "projSampler6" );
	
	for (int i=0; i<6; ++i)
		if (mParamProjSamplers[i] == nullptr)
			return false;

	mParamMaskSamplers[0] = cgGetNamedParameter( program, "maskSampler1" );
	mParamMaskSamplers[1] = cgGetNamedParameter( program, "maskSampler2" );

	if (mParamMaskSamplers[0] == nullptr || mParamMaskSamplers[1] == nullptr)
		return false;

	return mBuffer.InitGL( cgcontext, program, "ProjectorsBuffer", sizeof(ProjectorSet) );
}

void CGPUProjectorsModelCg::UploadBuffer( const ProjectorSet &buffer )
{
	mBuffer.UpdateData( 0, sizeof(ProjectorSet), &buffer );
}

bool CGPUProjectorsModelCg::Bind(const int numberOfProjectors, const GLuint *projIds, const int numberOfMasks, const GLuint *maskIds)
{
	// DONE: bind a result of projections composition

	if (projIds != nullptr)
	{
		for (int i=0; i<numberOfProjectors; ++i)
		{
			if (projIds[i] > 0)
			{
				cgGLSetTextureParameter( mParamProjSamplers[i], projIds[i] );
				cgGLEnableTextureParameter( mParamProjSamplers[i] );
			}
		}
	}

	if (maskIds != nullptr)
	{
		for (int i=0; i<numberOfMasks; ++i)
		{
			if (maskIds[i] > 0)
			{
				cgGLSetTextureParameter( mParamMaskSamplers[i], maskIds[i] );
				cgGLEnableTextureParameter( mParamMaskSamplers[i] );
			}
		}
	}

	/*
	if (mCompose != nullptr)
	{
		const GLuint texId = mCompose->GetComposeTextureID();

		if (texId > 0)
		{
			cgGLSetTextureParameter( mParamProjTexSampler, texId );
			cgGLEnableTextureParameter( mParamProjTexSampler );

			return true;
		}
	}
	*/
	return false;
}

void CGPUProjectorsModelCg::UnBind()
{
	for (int i=0; i<6; ++i)
		cgGLDisableTextureParameter( mParamProjSamplers[i] );

	for (int i=0; i<2; ++i)
		cgGLDisableTextureParameter( mParamMaskSamplers[i] );
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CGPUProjectorsModelNv

CGPUProjectorsModelNv::CGPUProjectorsModelNv()
{
}

CGPUProjectorsModelNv::~CGPUProjectorsModelNv()
{
}

void CGPUProjectorsModelNv::Prep(const int numberOfProjectors, const ProjectorSet &collection)
{
	// DONE: !!
	//g_projectorsBlock.numProjectors = numberOfProjectors;
	
	auto dstPtr = &g_projectorsBlock.Projectors[0];
	auto srcPtr = &collection.projectors[0];

	for (int i=0; i<numberOfProjectors; ++i)
	{
		dstPtr->blendMode = (float) srcPtr->blendMode;
		dstPtr->blendOpacity = srcPtr->blendOpacity;
		dstPtr->textureLayer = (float) i;
		const float mask = (float) srcPtr->maskLayer;
		dstPtr->maskLayer = (mask < 0.0f) ? 0.0f : 6.0f + mask;
		dstPtr->maskChannel = srcPtr->maskChannel;

		dstPtr->matrix = srcPtr->matrix;

		dstPtr++;
		srcPtr++;
	}

	mBufferProjectors.UpdateData( sizeof(projectorsBlock), 1, &g_projectorsBlock );
}

void CGPUProjectorsModelNv::BindBuffer(const GLuint unitId)
{
	mBufferProjectors.Bind(unitId);
}

void CGPUProjectorsModelNv::BindAsUniform(const GLuint programId, const GLuint locationId) const
{
	mBufferProjectors.BindAsUniform(programId, locationId, 0);
}

void CGPUProjectorsModelNv::BindProjectionMapping(const int numberOfProjectors, const ProjectorSet &collection, const GLuint mask1, const GLuint mask2) const
{	
	// bind masks
	if (mask1 > 0)
	{
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, mask1 );
	}
	if (mask2 > 0)
	{
		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, mask2 );
	}

	// bind projector textures

	for (int i=0; i<numberOfProjectors; ++i)
	{
		//glActiveTexture(GL_TEXTURE10 + i);
		//glBindTexture(GL_TEXTURE_2D, (GLuint) collection.projectors[i].textureLayer );
	}

	//glActiveTexture( GL_TEXTURE0 );

	//CHECK_GL_ERROR();
	
	/*
	// bind texture2darray
	if (mProjectorsManager->GetNumberOfProjections() > 0)
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D_ARRAY, mProjectorsManager->GetComposeTextureID() );
		glActiveTexture(GL_TEXTURE0);
	}
	*/
	// DONE: bind projectors buffer
	//const GLuint projectorsUnitId = 5;
	//mProjectorsManager->BindBuffer(projectorsUnitId);
	
	// TODO:	Bind data to a shader !!
}

void CGPUProjectorsModelNv::UnBindProjectionMapping()
{
	// unbind sampler slots

	for (int i=16; i>=8; --i)
	{
		glActiveTexture(GL_TEXTURE8 + i);
		glBindTexture(GL_TEXTURE_2D, 0 );
	}

	glActiveTexture(GL_TEXTURE0);

	CHECK_GL_ERROR();
}