
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_model.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_common.h"
#include "shared_textures.h"
#include "shared_materials.h"
#include "shared_shaders.h"
#include "shared_models.h"
#include "shared_rendering.h"
#include "shared_camera.h"
#include "shared_lights.h"

#include "ShaderFX.h"

#include <string>

/// forward declaration
class CGPUCacheLoaderVisitorImpl;

/////////////////////////////////////////////////////////////////////////////////////////
//

class CGPUCacheModel
{
public:

	//! a constructor
	CGPUCacheModel();

	//! a destructor
	virtual ~CGPUCacheModel();

	void Allocate();

	// free resources like textures, materials, shaders, etc.
	void Free();

	// load from xml
	bool	Load(const char *filename);
	void	ReloadShader();

	void	ShadeModel();
	void	CustomDisplay();
	void	CustomRayPick();
	void	CustomRectPick();

	// TODO: Depricated!
	void Render(	const CCameraInfoCache &cameraCache, 
				Graphics::BaseMaterialShaderFX *const pMaterialShader, 
					//CGPUShaderLights	*const pShaderLights,
					const bool cubemapSetup, 
					const CubeMapRenderingData *data );

	void PrepRender(	const CCameraInfoCache &cameraCache, 
				Graphics::BaseMaterialShaderFX *const pMaterialShader, 
					//CGPUShaderLights	*const pShaderLights,
					const bool cubemapSetup, 
					const CubeMapRenderingData *data );

	void UpdateColorId(const vec3 &colorId);
	void UpdateReceiveShadows(const int flag);

	void RenderBegin(	const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pMaterialShader, 
					//CGPUShaderLights	*const pShaderLights,
					const bool isEarlyZ,
					const bool cubemapSetup, 
					const CubeMapRenderingData *data );
	void RenderOpaque(	const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pMaterialShader );

	void RenderTransparent(	const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pMaterialShader );

	void RenderEnd( 	const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pMaterialShader );

	void DrawGeometry( const CCameraInfoCache &cameraCache );

public:

	bool			OverrideShading;
	EShadingType	ShadingType;
	bool			LogarithmicDepth;
	bool			SampleAlphaToCoverage;
	float			AlphaPass;
	
	inline void GetBoundingBox(float *bmin, float *bmax)
	{
		mModelRender->GetBoundingBox(bmin, bmax);
	}

	void		SetCacheMatrix(float *const matrix)
	{
		memcpy( mParentTransform.mat_array, matrix, sizeof(float) * 16 );
	}
	void		SetCacheMatrix( const double *matrix )
	{
		for (int i=0; i<16; ++i)
			mParentTransform.mat_array[i] = (float) matrix[i];
	}

	void	NeedUpdateTexturePtr()
	{
		mNeedUpdateTexturePtr = true;
	}

public:

	const char *GetSourceFilename() const;

	const int GetNumberOfSubModels() const;
	const int GetNumberOfMaterials() const;
	const int GetNumberOfBaseShaders() const;
	const int GetNumberOfShaders() const;	// populated count with combinations

	const int GetNumberOfOpaqueCommands() const;
	const int GetNumberOfTransparencyCommands() const;

	const char *GetSubModelName(const int index) const;
	const char *GetMaterialName(const int index) const;
	const char *GetShaderName(const int index) const;

	CGPUModelRenderCached *GetModelRenderPtr() const
	{
		return mModelRender;
	}

protected:

	mat4						mParentTransform;

	std::string					mSourceFilename;
	std::string					mFilename;	// imported xml file

	// cached resources
	CTexturesReference			*mTextures;
	CMaterialsReference			*mMaterials;
	CShadersReference			*mShaders;

	int							mNumberOfOpaque;
	int							mNumberOfTransparency;

	//
	bool						mNeedUpdateTexturePtr;

	int							mLastReceiveShadows;
	vec3						mLastColorId;

	// pre-cached model information 
	CGPUModelRenderCached		*mModelRender;
	CGPUVertexData				*mVertexData;

	// pre-cached scene information
	const CCameraInfoCache			*mCameraCache;
	Graphics::BaseMaterialShaderFX	*mMaterialShader;
	//Graphics::ShaderEffect			*mUberShader;
	//CGPUShaderLights				*mLights;

	void PassPreRender(const bool cubemapSetup, const CubeMapRenderingData *data=nullptr);
	void PassLighted(const bool cubemapSetup, const bool opaque, const bool transparency);

	static bool BindUberShader(	Graphics::BaseMaterialShaderFX *pMaterialShader,
						const bool lockTextures, 
									CResourceGPUModel<TextureGLSL> *customTextures, 
									CResourceGPUModel<MaterialGLSL> *customMaterials, 
									CResourceGPUModel<ShaderGLSL> *customShaders,
									//CGPUShaderLights *pShaderLights,
									vec4 globalAmbientColor,
									const bool shadowTechnique);

	static void UnBindUberShader( Graphics::BaseMaterialShaderFX *pMaterialShader,
							const bool unlockTextures, 
							CResourceGPUModel<TextureGLSL> *customTextures, 
							const bool shadowTechnique);

	friend class CGPUCacheLoaderVisitorImpl;
};