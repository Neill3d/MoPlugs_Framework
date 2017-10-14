
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_visitorImpl.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucache_loader.h"
#include "gpucache_model.h"

///////////////////////////////////////////////////////////////////////////
//
class CGPUCacheLoaderVisitorImpl : public CGPUCacheLoaderVisitor
{
public:
	
	//! a constructor
	CGPUCacheLoaderVisitorImpl(CGPUCacheModel *pModel);

	//! a destructor
	virtual ~CGPUCacheLoaderVisitorImpl();

	// main header
	void OnReadHeader(const char *xmlFilename, const char *sourceFilename) override;
	
	// textures
	bool OnReadTexturesBegin( const char *textures_filename, const int numberOfSamplers, const int numberOfImages ) override;
	void OnReadTexturesImage1(const ImageHeader *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData) override;
	void OnReadTexturesImage2(const BYTE type, const ImageHeader2 *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData) override;
	void OnReadEmptyImage() override;
	void OnReadTexturesSampler(const char *sampler_name, const char *sampler_file, const SamplerHeader *header, const size_t fileSamplerOffset, const size_t samplerSize, const BYTE *samplerData) override;
	void OnReadTexturesError(const char *what) override;
	void OnReadTexturesEnd() override;
	
	// materials, shaders, etc.
	bool OnReadMaterialsBegin(const int numberOfMaterials) override;
	void OnReadMaterial(const char *material_name, const MaterialGLSL &material ) override;
	void OnReadMaterialsEnd() override;

	bool OnReadShadersBegin(const int numberOfShaders) override;
	void OnReadShader(const char *shader_name, const int alphatype, const ShaderGLSL &shaderData ) override;
	void OnReadShadersEnd() override;

	// geometry, models
	bool OnReadModelsBegin(const int numberOfModels, const int numberOfMeshes, const double *bounding_min, const double *bounding_max) override;
	void OnReadVertexData( FileGeometryHeader *const pHeader, const BYTE *data ) override;
	void OnReadModel(const char *name, 
						const double *translation, 
						const double *rotation, 
						const double *scaling, 
						const double *bounding_min,
						const double *bounding_max,
						const int numberOfShaders,
						const int *shaders,
						const VertexDataHeader *pheader, 
						const BYTE *data) override;
	void OnReadModelPatch(const int offset, const int size, const int materialId) override;
	void OnReadModelFinish() override;
	void OnReadModelsEnd() override;

protected:

	CGPUCacheModel			*mModel;
	CTexturesReference		*mTextures;
	CMaterialsReference		*mMaterials;
	CShadersReference		*mShaders;

	int						mImageIndex;
	int						mSamplerIndex;

	int						mShaderIndex;
	int						mMaterialIndex;

	CGPUModelRenderCached	*mModelRender;
	CGPUVertexData			*mVertexData;

	int						mSubmodelIndex;
	int						mModelShaderId;
	float					mOpaqueModel;
	vec4					mBSphere;
	unsigned int			mNumberOfIndices;
	unsigned int			mAccumNumberOfIndices;

	static bool LoadImageData( int fh, GLuint &texId, vec2 &dimentions, BYTE *localImageBuffer, bool &isComporessed );
	static bool LoadImageData2( int fh, GLuint &texId, vec2 &dimentions, BYTE *localImageBuffer, CGPUImageSequencer &sequencer, bool &isComporessed );

	//static bool SaveSampler( int fh, FBTexture *pTexture, const int videoIndex );
	static bool LoadSampler( int fh, const GLuint samplerId, float *matrix, int &videoIndex );

};