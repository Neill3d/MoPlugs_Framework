
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_model.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucachemodel.h"

#ifdef _DEBUG
	extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
//
CGPUCacheModel::CGPUCacheModel()
{
	// internal model resources
	mTextures = nullptr;
	mMaterials = nullptr;
	mShaders = nullptr;
	mModelRender = nullptr;
	mVertexData = nullptr;

	// external assignment
	mMaterialShader = nullptr;
	mCameraCache = nullptr;

	// properties
	OverrideShading = false;
	ShadingType = eShadingTypeFlat;
	LogarithmicDepth = false;
	SampleAlphaToCoverage = false;
	AlphaPass = 0.25f;

	//
	mParentTransform.identity();

	mNeedUpdateTexturePtr = false;
	mLastReceiveShadows = 1;
}

CGPUCacheModel::~CGPUCacheModel()
{
	Free();
}

void CGPUCacheModel::Allocate()
{
	Free();

	mTextures = new CTexturesReference();
	mMaterials = new CMaterialsReference(mTextures);
	mShaders = new CShadersReference();

	mVertexData = new CGPUVertexData();
	mModelRender = new CGPUModelRenderCached(mVertexData);
}

void CGPUCacheModel::Free()
{
	if (mTextures)
	{
		delete mTextures;
		mTextures = nullptr;
	}
	if (mMaterials)
	{
		delete mMaterials;
		mMaterials = nullptr;
	}
	if (mShaders)
	{
		delete mShaders;
		mShaders = nullptr;
	}
	if (mModelRender)
	{
		delete mModelRender;
		mModelRender = nullptr;
	}
	if (mVertexData)
	{
		delete mVertexData;
		mVertexData = nullptr;
	}
}

void CGPUCacheModel::Render( const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pMaterialShader, 
					//CGPUShaderLights	*const pShaderLights,
					const bool cubemapSetup, 
					const CubeMapRenderingData *data )
{
	if (pMaterialShader == nullptr)
		return;

	mMaterialShader = pMaterialShader;
	mCameraCache = &cameraCache;
	//mLights = pShaderLights;
	/*
	if (mTextures)
	{
		mTextures->EvaluateFrameAnimation();
		mTextures->UpdateSequences();
	}
	*/

	if (mTextures)
	{
		if (mNeedUpdateTexturePtr)
		{
			mTextures->UpdateTexturePointers();
			mNeedUpdateTexturePtr = false;
		}

		mTextures->PrepRender();
	}
	if (mMaterials)
		mMaterials->PrepRender();
	if (mShaders)
		mShaders->PrepRender();

	if (mModelRender)
	{
		mat4 m4_parent (mParentTransform);
		mModelRender->UpdateGPUBuffer(&m4_parent, &mCameraCache->mv4);
	}

	PassPreRender(cubemapSetup, data);
	PassLighted(cubemapSetup, true, false);
}

void CGPUCacheModel::PrepRender( const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pUberShader, 
					//CGPUShaderLights	*const pShaderLights,
					const bool cubemapSetup, 
					const CubeMapRenderingData *data )
{
	if (pUberShader == nullptr)
		return;

	mMaterialShader = pUberShader;
	mCameraCache = &cameraCache;
	//mLights = pShaderLights;
	/*
	if (mTextures)
	{
		mTextures->EvaluateFrameAnimation();
		mTextures->UpdateSequences();
	}
	*/

	if (mTextures)
	{
		if (mNeedUpdateTexturePtr)
		{
			mTextures->UpdateTexturePointers();
			mNeedUpdateTexturePtr = false;
		}

		mTextures->PrepRender();
	}
	if (mMaterials)
		mMaterials->PrepRender();
	if (mShaders)
	{
		if (mShaders->IsNeedAGPUUpdate() )
		{

		}

		mShaders->PrepRender();
	}

	if (mModelRender)
	{
		mat4 m4_parent (mParentTransform);
		mModelRender->UpdateGPUBuffer(&m4_parent, &mCameraCache->mv4);
	}
}

void CGPUCacheModel::UpdateColorId(const vec3 &colorId)
{
	if (mModelRender)
	{
		if (colorId.x != mLastColorId.x || colorId.y != mLastColorId.y || colorId.z != mLastColorId.z)
		{
			mModelRender->UpdateColorId(colorId);
			mLastColorId = colorId;
		}
	}
}

void CGPUCacheModel::UpdateReceiveShadows(const int flag)
{
	if (mModelRender)
	{
		if (flag != mLastReceiveShadows)
		{
			mModelRender->UpdateReceiveShadows(flag);
			mLastReceiveShadows = flag;
		}
	}
}

void CGPUCacheModel::RenderBegin( const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pUberShader, 
					//CGPUShaderLights	*const pShaderLights,
					const bool isEarlyZ,
					const bool cubemapSetup, 
					const CubeMapRenderingData *cubemap )
{
	if (nullptr == pUberShader || nullptr == mModelRender)
		return;

	mMaterialShader = pUberShader;

	//mat4 m; 
	//m.identity();
	mat4 m4_parent (mParentTransform);

	const float realfarplane = (float) mCameraCache->realFarPlane;

	//
	/*
	if (cubemapSetup && cubemap)
	{
		vec3 cameraPos(cubemap->position.x, cubemap->position.y, cubemap->position.z);

		mat4 cameraMV;
		cameraMV.identity();
		cameraMV.set_translation( -cameraPos );
		
		mUberShader->UploadCubeMapUniforms(	cubemap->zmin, 
											cubemap->zmax, 
											cubemap->position, 
											cubemap->max, 
											cubemap->min, 
											cubemap->useParallax);
		mUberShader->SetCubeMapRendering(true);

		// TODO: 
		
		// mUberShader->UploadCameraUniforms(cameraMV, cameraPRJ, viewIT, vEyePos, width, height, farPlane, nullptr);

		//mUberShader->UploadCameraUniforms(cameraMV, mat4_id, mat4_id, cameraPos, cubemap->cubeMapSize, cubemap->cubeMapSize,
		//	cubemap->zmax, nullptr );
	}
	else
	{
		mUberShader->SetCubeMapRendering(false);
		mUberShader->UploadCameraUniforms(&realfarplane);	
	}
	*/

	mMaterialShader->UploadModelTransform(m4_parent);
	
	mMaterialShader->ModifyShaderFlags( Graphics::eShaderFlag_Bindless, true );
	mMaterialShader->ModifyShaderFlags( Graphics::eShaderFlag_EarlyZ, isEarlyZ );

	//mUberShader->SetCubeMapRendering(cubemapSetup);
	//mUberShader->SetLogarithmicDepth( LogarithmicDepth );
	//mMaterialShader->SetNumberOfProjectors(0);
	
	BindUberShader(mMaterialShader, true, mTextures, mMaterials, mShaders, vec4(0.1f, 0.1f, 0.1f, 0.0f), false);
	
	// DONE: subroutines for blending modes
	// set subroutine values
	// TODO: this is a part of shaderFX bind method
	/*
	if (false == isEarlyZ)
	{
		GLuint index[30];
		for (int i=0; i<30; ++i)
			index[i] = i;

		if (OverrideShading || cubemapSetup)
		{
			EShadingType shading = ShadingType;

			//if (cubemapSetup)
			//	shading = eShadingTypeFlat;

			const int indexOffset = 25;

			for (int i=0; i<eShadingTypeCount; ++i)
				index[indexOffset + i] = indexOffset + (int) shading;

			// TODO: bind an override shader with specified parameters
			//mUberShader->SetToonParameters( (float) ToonSteps, (float) ToonDistribution, (float) ToonShadowPosition );
		}

		glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, GLsizei(30), &index[0] );

		CHECK_GL_ERROR();
	}
	*/
	if (mModelRender->RenderBegin() )
	{
		auto locPtr = mMaterialShader->GetCurrentEffectLocationsPtr()->vptr();
		mModelRender->BindModelInfoAsUniform( locPtr->GetShaderId(), locPtr->GetLocation(Graphics::eCustomVertexLocationAllTheModels) );
		mModelRender->BindMeshInfoAsUniform( locPtr->GetShaderId(), locPtr->GetLocation(Graphics::eCustomVertexLocationAllTheMeshes) );	
	}
}

void CGPUCacheModel::RenderOpaque( const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pUberShader )
{
	// opaque
	mMaterialShader = pUberShader;
	if ( mNumberOfOpaque > 0)
	{
		//mUberShader->UpdateAlphaPass(0.0f);
		mModelRender->RenderOpaque();
	}
		
}

void CGPUCacheModel::RenderTransparent( const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pUberShader )
{
	// draw transparency (sort objects ?!)
	mMaterialShader = pUberShader;
	if ( mNumberOfTransparency > 0)
	{

		//float alphapass = (float) AlphaPass;
		//mUberShader->UpdateAlphaPass(alphapass);
		mModelRender->RenderTransparency();
	}
}

void CGPUCacheModel::RenderEnd( const CCameraInfoCache &cameraCache, 
					Graphics::BaseMaterialShaderFX *const pUberShader )
{
	if (nullptr == pUberShader || nullptr == mModelRender)
		return;

	mMaterialShader = pUberShader;

	//
	mModelRender->RenderEnd();
	CHECK_GL_ERROR();

	UnBindUberShader(mMaterialShader, true, mTextures, false);

	CHECK_GL_ERROR();
}

void CGPUCacheModel::PassPreRender(const bool cubemapSetup, const CubeMapRenderingData *cubemap)
{
	//
	
	if (cubemapSetup && cubemap)
	{
		/*
		mat4 cameraMV;
		cameraMV.identity();
		cameraMV.set_translation( -cubemap->position );
		*/
		mMaterialShader->UploadCubeMapUniforms(	cubemap->zmin, 
											cubemap->zmax, 
											cubemap->worldToLocal,
											cubemap->position,
											cubemap->max, 
											cubemap->min, 
											cubemap->useParallax);
		mMaterialShader->ModifyShaderFlags( Graphics::eShaderFlag_CubeMapRendering, true );

		// TODO: 
		/*
		viewIT = cameraMV;
		viewIT[12] = 0.0;
		viewIT[13] = 0.0;
		viewIT[14] = 0.0;
		FBMatrixInverse( viewIT, viewIT );
		FBMatrixTranspose( viewIT, viewIT );
		cameraPRJ.Identity();
		
		width = cubemap->cubeMapSize;
		height = cubemap->cubeMapSize;
		*/
		// mUberShader->UploadCameraUniforms(cameraMV, cameraPRJ, viewIT, vEyePos, width, height, farPlane, nullptr);
	}
	else
	{
		mMaterialShader->ModifyShaderFlags( Graphics::eShaderFlag_CubeMapRendering, false );
	}

	mMaterialShader->UploadCameraUniforms( *mCameraCache );
}


bool CGPUCacheModel::BindUberShader(	Graphics::BaseMaterialShaderFX *pMaterialShader,
						const bool lockTextures, 
									CResourceGPUModel<TextureGLSL> *customTextures, 
									CResourceGPUModel<MaterialGLSL> *customMaterials, 
									CResourceGPUModel<ShaderGLSL> *customShaders,
									//CGPUShaderLights *pShaderLights,
									vec4 globalAmbientColor,
									const bool shadowTechnique)
{
	if (pMaterialShader == nullptr) 
		return false;
	/*
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	*/
	// make textures resident for a shader
	if (lockTextures)
	{
		if (customTextures)
			customTextures->Lock();
	}
	/*
	if (pShaderLights != nullptr && shadowTechnique == false )
	{
		pUberShader->UploadLightingInformation(globalAmbientColor, 
			pShaderLights->GetNumberOfDirLights(), 
			pShaderLights->GetNumberOfLights() );
	}
	*/
	// bind effect
	/*
	if (shadowTechnique == true)
	{
		pUberShader->SetTechnique(Graphics::eEffectTechniqueShadows);
	}
	else
	{
		pUberShader->SetTechnique(Graphics::eEffectTechniqueShading);
	}
	*/
	pMaterialShader->Bind();

	const GLuint fragmentProgram = pMaterialShader->GetFragmentProgramId();
	if (fragmentProgram > 0)
	{
		auto locPtr = pMaterialShader->GetCurrentEffectLocationsPtr()->fptr();
		/*
		if (pShaderLights != nullptr && shadowTechnique == false )
			pShaderLights->Bind(fragmentProgram, locPtr->dirLights, locPtr->lights );
			//pShaderLights->Bind(fragmentProgram, locPtr->clusterGrid, locPtr->clusterIndex, locPtr->dirLights, locPtr->lights, locPtr->lightMatrices);
*/
		if (customShaders)
		{
			//customShaders->BindAsUniform(locPtr->vertex, locPtr->allTheShadersV);
			customShaders->BindAsUniform(fragmentProgram, locPtr->GetLocation(Graphics::eCustomLocationAllTheShaders));
		}
		
		// DONE: check what is it !??!

		if (customTextures)
			customTextures->BindAsUniform(fragmentProgram, locPtr->GetLocation(Graphics::eCustomLocationAllTheTextures));
		
		if (customMaterials)
			customMaterials->BindAsUniform(fragmentProgram, locPtr->GetLocation(Graphics::eCustomLocationAllTheMaterials));
	}

	//
	if (shadowTechnique == false)
	{
		//BindShadowsTexture(5, true);
	}
	
	CHECK_GL_ERROR();
	return true;
}

void CGPUCacheModel::UnBindUberShader( Graphics::BaseMaterialShaderFX *pUberShader,
										const bool unlockTextures, 
										CResourceGPUModel<TextureGLSL> *customTextures, 
										const bool shadowTechnique)
{
	if (pUberShader == nullptr) 
		return;

	pUberShader->UnBind();

	//
	if (shadowTechnique == false)
	{
		//UnBindShadowsTexture(5);
	}

	// make textures non resident
	if (unlockTextures)
	{
		if (customTextures)
			customTextures->UnLock();
	}
}

void CGPUCacheModel::PassLighted(const bool cubemapSetup, const bool opaque, const bool transparency)
{
	if (!mMaterialShader || !mModelRender)
		return;

	//mat4 m; 
	//m.identity();
	mat4 m4_parent (mParentTransform);

	const float realfarplane = (float) mCameraCache->realFarPlane;

	//

	mMaterialShader->UploadCameraUniforms(&realfarplane);
	mMaterialShader->UploadModelTransform(m4_parent);
	
	mMaterialShader->ModifyShaderFlags( Graphics::eShaderFlag_Bindless, true );
	mMaterialShader->ModifyShaderFlags( Graphics::eShaderFlag_EarlyZ, false );
	mMaterialShader->ModifyShaderFlags( Graphics::eShaderFlag_CubeMapRendering, cubemapSetup );

	//mUberShader->SetLogarithmicDepth( LogarithmicDepth );
	//mMaterialShader->SetNumberOfProjectors(0);

	BindUberShader(mMaterialShader, true, mTextures, mMaterials, mShaders, vec4(0.1f, 0.1f, 0.1f, 0.0f), false);

	// DONE: subroutines for blending modes
	// set subroutine values
	GLuint index[30];
	for (int i=0; i<30; ++i)
		index[i] = i;

	if (OverrideShading || cubemapSetup)
	{
		EShadingType shading = ShadingType;

		//if (cubemapSetup)
		//	shading = eShadingTypeFlat;

		const int indexOffset = 25;

		for (int i=0; i<eShadingTypeCount; ++i)
			index[indexOffset + i] = indexOffset + (int) shading;

		// TODO: bind an override shader with specified parameters
		//mUberShader->SetToonParameters( (float) ToonSteps, (float) ToonDistribution, (float) ToonShadowPosition );
	}

	glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, GLsizei(30), &index[0] );

	CHECK_GL_ERROR();

		
	if (mModelRender->RenderBegin() )
	{
		const int numberOfOpaque = mNumberOfOpaque; // mShaders->GetNumberOfOpaqueShaders();
		const int numberOfTransparency = mNumberOfTransparency; // mShaders->GetNumberOfTransparencyShaders();
		
		auto locPtr = mMaterialShader->GetCurrentEffectLocationsPtr()->vptr();
		mModelRender->BindModelInfoAsUniform( locPtr->GetShaderId(), locPtr->GetLocation(Graphics::eCustomVertexLocationAllTheModels));
		mModelRender->BindMeshInfoAsUniform( locPtr->GetShaderId(), locPtr->GetLocation(Graphics::eCustomVertexLocationAllTheMeshes));

		// opaque
		if (true == opaque && numberOfOpaque > 0)
		{
			mMaterialShader->UpdateAlphaPass(0.0f);
			mModelRender->RenderOpaque();
		}
		
		// draw transparency (sort objects ?!)
		if (true == transparency && numberOfTransparency > 0)
		{

			float alphapass = (float) AlphaPass;
			mMaterialShader->UpdateAlphaPass(alphapass);
			mModelRender->RenderTransparency();

			/*
			//glEnable(GL_BLEND);
			//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			bool IsMultisampling = (glIsEnabled(GL_MULTISAMPLE) == GL_TRUE);

			if (SampleAlphaToCoverage)
			{
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				//glEnable(GL_ALPHA_TEST);
				//glAlphaFunc(GL_GEQUAL, 0.25f);

				//glEnable(GL_SAMPLE_MASK);
				//glSampleCoverage(0.75, GL_FALSE);
			}
			else
			{
				glEnable(GL_BLEND);
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			}

			float alphapass = (float) AlphaPass;
			mUberShader->UpdateAlphaPass(alphapass);
			mModelRender->RenderTransparency();


			if (SampleAlphaToCoverage)
			{
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				//glDisable(GL_ALPHA_TEST);
				//glDisable(GL_SAMPLE_MASK);
			}
			else
			{
				glDisable(GL_BLEND);
			}
			*/
		}

		mModelRender->RenderEnd();
	}

	UnBindUberShader(mMaterialShader, true, mTextures, false);

	CHECK_GL_ERROR();
}

const char *CGPUCacheModel::GetSourceFilename() const
{
	return mSourceFilename.c_str();
}

const int CGPUCacheModel::GetNumberOfSubModels() const
{
	return mModelRender->GetNumberOfSubModels();
}

const int CGPUCacheModel::GetNumberOfMaterials() const
{
	return mMaterials->GetNumberOfMaterials();
}

const int CGPUCacheModel::GetNumberOfBaseShaders() const
{
	return (int) mShaders->GetBaseSize();
}

const int CGPUCacheModel::GetNumberOfShaders() const
{
	return (int) mShaders->GetDataSize();
}

const int CGPUCacheModel::GetNumberOfOpaqueCommands() const
{
	return mNumberOfOpaque;
}

const int CGPUCacheModel::GetNumberOfTransparencyCommands() const
{
	return mNumberOfTransparency;
}

const char *CGPUCacheModel::GetSubModelName(const int index) const
{
	return mModelRender->GetSubModelName(index);
}

const char *CGPUCacheModel::GetMaterialName(const int index) const
{
	return mMaterials->GetMaterialName(index);
}

const char *CGPUCacheModel::GetShaderName(const int index) const
{
	return mShaders->GetShaderName(index);
}


void CGPUCacheModel::DrawGeometry( const CCameraInfoCache &cameraCache )
{

	mat4 m4_parent (mParentTransform);
	const float realfarplane = (float) mCameraCache->realFarPlane;

	if (mModelRender->RenderBegin() )
	{
		const int numberOfOpaque = mNumberOfOpaque; // mShaders->GetNumberOfOpaqueShaders();
		const int numberOfTransparency = mNumberOfTransparency; // mShaders->GetNumberOfTransparencyShaders();
		
		auto locPtr = mMaterialShader->GetCurrentEffectLocationsPtr()->vptr();
		mModelRender->BindModelInfoAsUniform( locPtr->GetShaderId(), locPtr->GetLocation(Graphics::eCustomVertexLocationAllTheModels));
		mModelRender->BindMeshInfoAsUniform( locPtr->GetShaderId(), locPtr->GetLocation(Graphics::eCustomVertexLocationAllTheMeshes) );

		// opaque
		if (numberOfOpaque > 0)
		{
			mMaterialShader->UpdateAlphaPass(0.0f);
			mModelRender->RenderOpaque();
		}
		
		// draw transparency (sort objects ?!)
		if (numberOfTransparency > 0)
		{
			//glEnable(GL_BLEND);
			//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			bool IsMultisampling = (glIsEnabled(GL_MULTISAMPLE) == GL_TRUE);

			if (SampleAlphaToCoverage)
			{
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				//glEnable(GL_ALPHA_TEST);
				//glAlphaFunc(GL_GEQUAL, 0.25f);

				//glEnable(GL_SAMPLE_MASK);
				//glSampleCoverage(0.75, GL_FALSE);
			}
			else
			{
				glEnable(GL_BLEND);
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			}

			float alphapass = (float) AlphaPass;
			mMaterialShader->UpdateAlphaPass(alphapass);
			mModelRender->RenderTransparency();


			if (SampleAlphaToCoverage)
			{
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
				//glDisable(GL_ALPHA_TEST);
				//glDisable(GL_SAMPLE_MASK);
			}
			else
			{
				glDisable(GL_BLEND);
			}
		}

		mModelRender->RenderEnd();
	}

}