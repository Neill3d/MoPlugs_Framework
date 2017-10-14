
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: Shader.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "Shader.h"
//#include "ShaderModelInfo.h"

#ifdef _WIN32
	#include <windows.h>
#endif

#include <math.h>
#include <map>
#include <algorithm>
#include "algorithm\math3d.h"

#include "graphics\CheckGLError.h"

#include "glm\gtc\matrix_transform.hpp"

namespace Graphics
{
	
	char	gResourcesPath[256];

	static bool firstQuery = true;

	const char * ShaderBlendNames[] = {

		"BlendModes::BlendNormal", // 0
		"BlendModes::BlendLighten", 
		"BlendModes::BlendDarken",
		"BlendModes::BlendMultiply",
		"BlendModes::BlendAverage",
		"BlendModes::BlendAdd",
		"BlendModes::BlendSubstract",
		"BlendModes::BlendDifference",
		"BlendModes::BlendNegation",
		"BlendModes::BlendExclusion",
		"BlendModes::BlendScreen",
		"BlendModes::BlendOverlay",
		"BlendModes::BlendSoftLight",
		"BlendModes::BlendHardLight",
		"BlendModes::BlendColorDodge",
		"BlendModes::BlendColorBurn",
		"BlendModes::BlendLinearDodge",
		"BlendModes::BlendLinearBurn",
		"BlendModes::BlendLinearLight",
		"BlendModes::BlendVividLight",
		"BlendModes::BlendPinLight",
		"BlendModes::BlendHardMix",
		"BlendModes::BlendReflect",
		"BlendModes::BlendGlow",
		"BlendModes::BlendPhoenix"
	};

	//------------------------------------------------------------------------------
void errorCallbackFunc(const char *errMsg)
{
#ifdef WIN32
    OutputDebugString(errMsg);
#endif
    printf(errMsg);
}
void includeCallbackFunc(const char *incName, FILE *&fp, const char *&buf)
{
    char fullpath[200];
    fopen_s(&fp, incName, "r");
    if(fp) return;

    sprintf_s(fullpath, 200, "%s\\%s", gResourcesPath, incName);
    fopen_s(&fp, fullpath, "r");
    if(fp) return;
}


///////////////////////////////////////////////////////////////////////////////////
//

const char *CustomEffectShaderLocations::GetVertexLocationName(const CustomVertexShaderLocation location)
{
	switch(location)
	{
		//
		// vertex locations
		
	case eCustomVertexLocationAllTheShaders:
		return "allTheShaders";
	case eCustomVertexLocationAllTheMeshes:
		return "allTheMeshes";
	case eCustomVertexLocationAllTheModels:
		return "allTheModels";

	case eCustomVertexLocationIrisSize:
		return "iris_size";
	case eCustomVertexLocationCorneaBumpAmount:
		return "cornea_bump_amount";
	case eCustomVertexLocationCorneaBumpRadiusMult:
		return "cornea_bump_radius_mult";
	}

	return "None";
}

const char *CustomEffectShaderLocations::GetFragmentLocationName(const CustomFragmentShaderLocation location)
{

	switch(location)
	{
		//
		// fragment locations
		
	case eCustomLocationAmbient:
		return "ambientSampler";
	case eCustomLocationDiffuse:
		return "diffuseSampler";
	case eCustomLocationTransparency:
		return "transparencySampler";
	case eCustomLocationSpecularity:
		return "specularitySampler";
	case eCustomLocationReflectivity:
		return "reflectivitySampler";
	case eCustomLocationNormalMap:
		return "normalMapSampler";
	case eCustomLocationCubeMap:
		return "cubeMapSampler";

		// global scene data needed for shader

	case eCustomLocationAllTheTextures:
		return "allTheTextures";
	case eCustomLocationAllTheMaterials:
		return "allTheMaterials";
	case eCustomLocationAllTheShaders:
		return "allTheShaders";
	case eCustomLocationAllTheProjectors:
		return "allTheProjectors";

	case eCustomLocationClusterGrid:
		return "clusterGrid";
	case eCustomLocationClusterIndex:
		return "clusterIndex";
	case eCustomLocationDirLights:
		return "dirLights";
	case eCustomLocationLights:
		return "lights";

	case eCustomLocationShaderDirLights:
		return "shaderDirLights";
	case eCustomLocationShaderLights:
		return "shaderLights";

			// 2d array textures (projections, shadows)
	case eCustomLocationShadows:
		return "shadowsSampler";
	case eCustomLocationLightMatrices:
		return "lightMatrices";

			// array of samplers + mask samplers (8 samplers in total)
	case eCustomLocationProjectors:
		return "projectorsSampler";
	case eCustomLocationMaskA:
		return "projectorMaskA";
	case eCustomLocationMaskB:
		return "projectorMaskB";

	case eCustomLocationBackgroundSampler:
		return "backgroundSampler";
	case eCustomLocationMatCapSampler:
		return "matCapSampler";
	case eCustomLocationRimSampler:			// texture that could define rim color in uv-space
		return "rimSampler";

	case eCustomLocationDiffuseLightingSampler:
		return "DiffuseLightingSampler";
	case eCustomLocationSpecularLightingSampler:
		return "SpecularLightingSampler";
	case eCustomLocationBrdfSampler:
		return "brdfSampler";

	case eCustomLocationReflCubeSampler:
		return "reflectionCubeSampler";
	case eCustomLocationMainDepthSampler:	// used for soft particles
		return "mainDepthSampler";

		//
		// eye shader uniforms

	case eCustomLocationEyeEnvReflectionSampler:
		return "texEnvRfl";
	case eCustomLocationEyeEnvDiffuseSampler:
		return "texEnvDif";
	case eCustomLocationEyeEnvRefractionSampler:
		return "texEnvRfr";

	case eCustomLocationPupilSize:
		return "pupil_size";
	case eCustomLocationIrisTexStart:
		return "iris_tex_start";
	case eCustomLocationIrisTexEnd:
		return "iris_tex_end";
	case eCustomLocationIrisBorder:
		return "iris_border";
	case eCustomLocationIrisSize:
		return "iris_size";
	case eCustomLocationIrisEdgeFade:
		return "iris_edge_fade";
	case eCustomLocationIrisInsetDepth:
		return "iris_inset_depth";
	case eCustomLocationScleraTexScale:
		return "sclera_tex_scale";
	case eCustomLocationScleraTexOffset:
		return "sclera_tex_offset";
	case eCustomLocationIor:
		return "ior";
	case eCustomLocationRefractEdgeSoftness:
		return "refract_edge_softness";

	case eCustomLocationIrisTextureCurvature:
		return "iris_texture_curvature";
	case eCustomLocationArgIrisShadingCurvature:
		return "arg_iris_shading_curvature";
		
	case eCustomLocationTexUOffset:
		return "tex_U_offset";
	case eCustomLocationIrisNormalOffset:
		return "iris_normal_offset";
	case eCustomLocationCorneaDensity:
		return "cornea_density";
	case eCustomLocationBumpTexture:
		return "bump_texture";
	case eCustomLocationCatShape:
		return "catshape";
	case eCustomLocationCybShape:
		return "cybshape";
	case eCustomLocationColTexture:
		return "col_texture";
	}

	return "None";
}

int CustomEffectShaderLocations::PrepVertexLocations()
{
	if (vertex < 0)
		return 0;

	int count=0;
	for (int i=0; i<eCustomVertexLocationCount; ++i)
	{
		GLint loc = glGetUniformLocation(vertex, GetVertexLocationName( (CustomVertexShaderLocation) i) );

		if (loc >= 0)
			count += 1;
		vertexLocations[i] = loc;
	}

	return count;
}

int CustomEffectShaderLocations::PrepFragmentLocations()
{
	if (fragment <= 0)
		return 0;

	int count = 0;
	for (int i=0; i<eCustomFragmentLocationCount; ++i)
	{
		GLint loc = glGetUniformLocation(fragment, GetFragmentLocationName( (CustomFragmentShaderLocation) i) );

		if (loc >= 0)
			count += 1;
		fragmentLocations[i] = loc;
	}

	return count;

		
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// SHADER BASE


ShaderBase::ShaderBase()
{
	fx_EffectMaterial = nullptr;

	fx_TechCurrent = nullptr;
	fx_pass = nullptr;

	fx_ScreenSize = nullptr;
	/*
	fx_ScreenHeight = nullptr;
	fx_ScreenHeight = nullptr;
	*/
}



ShaderBase::~ShaderBase()
{
    Free();
}

void ShaderBase::ReSize(const int W, const int H)
{
	nvFX::getResourceRepositorySingleton()->setParams(0,0,W,H,1,0,NULL );
    bool failed = nvFX::getResourceRepositorySingleton()->validateAll() ? false : true;
    if(failed)
        assert(!"Oops");
}

void ShaderBase::Free()
{
	if (fx_EffectMaterial)
	{
		nvFX::IContainer::destroy(fx_EffectMaterial);
		fx_EffectMaterial = NULL;
	}
}



//-----------------------------------------------------------------------------
// Load scene effect
//-----------------------------------------------------------------------------
bool ShaderBase::InitializeEffectParams()
{
	//
    // Let's keep track in interface pointers everything, for purpose of clarity
    //
	/*
	fx_ScreenWidth = fx_EffectMaterial->findUniform("SCREEN_WIDTH");
	if (fx_ScreenWidth)
		fx_ScreenWidth->setValue1i(800);
	fx_ScreenHeight = fx_EffectMaterial->findUniform("SCREEN_HEIGHT");
	if (fx_ScreenHeight)
		fx_ScreenHeight->setValue1i(600);
		*/

	fx_ScreenSize = fx_EffectMaterial->findUniform("SCREEN_SIZE");
	if (fx_ScreenSize)
		fx_ScreenSize->setValue4f(0.0f, 0.0f, 1.0f/800.0f, 1.0f/600.0f);

	return true;
}




bool ShaderBase::loadMaterialEffect(const char *effectFileName, const double globalScale)
{
    if(fx_EffectMaterial)
    {
        LOGI("Desroying previous material Effect\n");
        //fx_EffectMaterial->destroy();
        //or
        LOGI("=========> Destroying effect\n");
        nvFX::IContainer::destroy(fx_EffectMaterial);
        fx_EffectMaterial = NULL;
    }
    LOGI("Creating Effect (material)\n");
    fx_EffectMaterial = nvFX::IContainer::create("material");
    bool bRes = nvFX::loadEffectFromFile(fx_EffectMaterial, effectFileName);
        
    if(!bRes)
    {
        LOGE("Failed\n");
        return false;
    }
    LOGOK("Loaded\n");

	if (InitializeEffectParams() == false)
	{
		LOGE("Failed to initialized uniforms\n");
		return false;
	}

    return true;
}



bool ShaderBase::Initialize( const char* resourcePath, const char *effectName, const int W, const int H, const double globalScale )
{
    sprintf_s( gResourcesPath, 256, "%s", resourcePath );

	char szMaterialEffectPath[256];

	sprintf_s( szMaterialEffectPath, 256, "%s\\%s", resourcePath, effectName );

	//
    // Effects
    //
    nvFX::setErrorCallback(errorCallbackFunc);
    nvFX::setIncludeCallback(includeCallbackFunc);
	if (false == loadMaterialEffect( szMaterialEffectPath, globalScale ) ) return false;

    return true;
}

const GLuint ShaderBase::GetFragmentProgramId()
{
	return mFragmentProgram;
}

const GLuint ShaderBase::FindFragmentProgramLocation(const char *name)
{
	GLuint loc = glGetUniformLocation( mFragmentProgram, name );
	return loc;
}

const GLuint ShaderBase::nvGetFragmentProgramId(nvFX::IPass *pass, const int programPipeline, const int shaderProgram)
{
	if (pass == nullptr)
		return 0;
	
	GLuint fragmentId = 0;	

	nvFX::IProgram *glslProgram = nullptr;
    
	nvFX::IProgramPipeline *glslProgramPipeline = pass->getExInterface()->getProgramPipeline(programPipeline);
    if(glslProgramPipeline)
    {
		glslProgram = glslProgramPipeline->getShaderProgram(shaderProgram);	// Fragment glsl program !
    
    } else {
		glslProgram = pass->getExInterface()->getProgram(shaderProgram);
    
    }
    fragmentId = (glslProgram != nullptr) ? glslProgram->getProgram() : 0;

	return fragmentId;
}

bool ShaderBase::PrepCommonLocations()
{
	// Compile GLSL shader using sampler uniform <u>.  The shader itself
    // needs no special #extension directive as long as <u> is a uniform in
    // the default partition.  Link the program, and query the location of
    // <u>, which we will store in <location>.
	
	CHECK_GL_ERROR();

	bool status = true;

	mFragmentProgram = ShaderBase::nvGetFragmentProgramId(fx_pass, 0, 1);

	return status;
}

void ShaderBase::BindShaderPrograms()
{
	CHECK_GL_ERROR();

	// TODO: what to do with id buffer ?
	/*
	if (pRenderOptions->IsIDBufferRendering() )
	{
		fx_TechCurrent = fx_TechFlat;
	}
	else
	{
		fx_TechCurrent = fx_TechPhong;
	}
	*/
	//
	//
	if (fx_TechCurrent) fx_pass = fx_TechCurrent->getPass(0);
	if (fx_pass) fx_pass->execute();

	CHECK_GL_ERROR();
}

void ShaderBase::UnBindShaderPrograms()
{

    if(fx_pass)
        fx_pass->unbindProgram();

    fx_pass = nullptr;
}


void ShaderBase::UnsetTextures()
{

    // TODO: Deactivate all textures.

	glActiveTexture( GL_TEXTURE3 );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glActiveTexture( GL_TEXTURE2 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	
	glActiveTexture( GL_TEXTURE1 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, 0 );

}




void ShaderBase::Bind()
{
	// bind effect
	BindShaderPrograms();
}


void ShaderBase::UnBind(const bool unlockTextures)
{
	/*
	FBCamera *pCamera = FBSystem::TheOne().Renderer->CurrentCamera;
	const double nearPlane = pCamera->NearPlaneDistance;
	const double farPlane = pCamera->FarPlaneDistance;

	FBMatrix mv, mp;
	pCamera->GetCameraMatrix(mv, kFBModelView);
	pCamera->GetCameraMatrix(mp, kFBProjection);

	mat4 projection, modelview;
	for (int i=0; i<16; ++i)
	{
		projection.mat_array[i] = (float) mp[i];
		modelview.mat_array[i] = (float) mv[i];
	}

	*/

	if (fx_pass)
		fx_pass->unbindProgram();

	// TODO: RenderModel::renderFinish();




	//CGPUFBScene::instance().GetLightsManagerPtr()->DrawCameraFrustum( nearPlane, farPlane, projection, modelview );
}

void ShaderBase::SetWindowSize( const int offsetX, const int offsetY, const int width, const int height )
{
	if (fx_ScreenSize)
		fx_ScreenSize->setValue4f( (float)offsetX, (float)offsetY, 1.0f/(float)width, 1.0f/(float)height);

	/*
	if (fx_ScreenWidth) 
		fx_ScreenWidth->setValue1i(width);
	if (fx_ScreenHeight)
		fx_ScreenHeight->setValue1i(height);
		*/
}

void ShaderBase::ShowError(const char* pText)
{
	// TODO: output nvFX error

    //const char* lErrStr = ( pText ) ? pText : cgGetErrorString(cgGetError());
    //FBMessageBox("Cg Error", (char*)lErrStr,  "OK");
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////// SHADER EFFECT

ShaderEffect::ShaderEffect()
	: ShaderBase()
{
	/*
	fx_TechPhong = nullptr;
	fx_TechPhongLog = nullptr;
	fx_TechPhongBindless = nullptr;
	fx_TechPhongBindlessLog = nullptr;
	fx_TechPhongCubeMap = nullptr;
	fx_TechPhongBindlessCubeMap = nullptr;
	*/
	fx_TechWallMaterial = nullptr;
	fx_TechWallMaterialLog = nullptr;
	//fx_TechWallCubeMap = nullptr;
	fx_TechCharacter = nullptr;
	fx_TechDepthOverrides = nullptr;
	fx_TechShadow = nullptr;
	fx_TechCulling = nullptr;
	fx_TechNormals = nullptr;
	fx_TechNormalsLog = nullptr;
	fx_TechSimple = nullptr;

	// Cst buffers
	fx_transfBlock1 = nullptr;
	fx_transfBlock2 = nullptr;
	fx_frustumBlock = nullptr;
	fx_cubeMapBlock = nullptr;

	fx_lightBlock = nullptr;
	fx_projectorsBlock = nullptr;
	
	//
	
	fx_Fcoef = nullptr;
	fx_depthDisplacement = nullptr;
	fx_numberOfShadows = nullptr;
	fx_normalsLength= nullptr;
	fx_AlphaPass = nullptr;
	fx_numberOfProjectors = nullptr;

	fx_meshIndex = nullptr;

	fx_textureOffset = nullptr;
	fx_textureScaling = nullptr;
	//fx_shaderMask = nullptr;

	fx_softParticles = nullptr;

	//fx_rimOptions = nullptr;
	//fx_rimColor = nullptr;

	mLogDepth = false;
	mOldLogDepth= false;

	mCurrentTech = eEffectTechniqueShading;

	mNoTextures = false;
	mEarlyZ = false;
	mBindless = false;
	mCubeMapRendering = false;
	mEyePass = false;
	mAlpha = 0.0f;

	ConstructorUniformBuffers();
}

ShaderEffect::~ShaderEffect()
{
	Free();
}

void ShaderEffect::Free()
{
	ShaderBase::Free();

	FreeUniformBuffers();
}


void ShaderEffect::SetTechnique(const EEffectTechnique technique)
{
	mCurrentTech = technique;
	PrepCurrentTech();
}
/*
void ShaderEffect::SetNormalsParameters(FBRenderOptions *pOptions, const float length)
{
	if (fx_normalsLength)
		fx_normalsLength->setValue1f(length);
}
*/
void ShaderEffect::SetLogarithmicDepthHint(const EEffectDepthHint value)
{
	mLogarithmicDepthHint = value;
}

const EEffectDepthHint ShaderEffect::GetLogarithmicDepthHint() const
{
	return mLogarithmicDepthHint;
}

void ShaderEffect::SetLogarithmicDepth(const bool value)
{
	mLogDepth = value;
	PrepLogDepth();

	/*
	bool newValue = value;
	switch( mLogarithmicDepthHint )
	{
	case eEffectDepthUser:
		newValue = value;
		break;
	case eEffectDepthLinear:
		newValue = false;
		break;
	case eEffectDepthLogarithmic:
		newValue = true;
		break;
	}
	
	if (newValue != mLogarithmicDepth)
	{
		mLogarithmicDepth = newValue;
		PrepLogDepth();
	}
	*/
}

void ShaderEffect::SetEarlyZ(const bool value)
{
	mEarlyZ = value;

	//PrepCurrent();
}
void ShaderEffect::SetEyePass( const bool value )
{
	mEyePass = value;
}
void ShaderEffect::NoTextures(const bool value)
{
	mNoTextures = value;

	//PrepCurrent();
}

void ShaderEffect::SetBindless(const bool value)
{
	mBindless = value;

	//PrepCurrent();
}

void ShaderEffect::SetCubeMapRendering(const bool value)
{
	mCubeMapRendering = value;

	//PrepCurrent();
}

void ShaderEffect::PrepCurrentTech()
{

	switch(mCurrentTech)
	{
	case eEffectTechniqueIBL:
		PrepCharacter_PassAndLocations();
		break;

	case eEffectTechniqueWallMaterial:
	case eEffectTechniqueWallMaterialLog:
	case eEffectTechniqueShading:
		PrepWall_PassAndLocations();
		break;

	case eEffectTechniqueCulling:
		fx_TechCurrent = fx_TechCulling;
		break;
	case eEffectTechniqueNormals:
		fx_TechCurrent = (mLogDepth) ? fx_TechNormalsLog : fx_TechNormals;
		break;
	case eEffectTechniqueShadows:
		mCurrentLoc = &mShadowLoc;
		fx_TechCurrent = fx_TechShadow;
		break;
	case eEffectTechniqueSimple:
		fx_TechCurrent = fx_TechSimple;
		break;
	default:
		fx_TechCurrent = nullptr;
	}

	if (fx_TechCurrent) fx_pass = fx_TechCurrent->getPass(0);
}

void ShaderEffect::PrepWall_PassAndLocations()
{
	int currentPassId = eTechWallPass_EarlyZNoTextures;
	
	if (false == mEarlyZ)
	{
		currentPassId = eTechWallPass_BindedTextures;

		if (mNoTextures)
			currentPassId = eTechWallPass_NoTextures;
		else if (mBindless)
			currentPassId = eTechWallPass_BindlessTextures;
	}
	else
	{
		currentPassId = eTechWallPass_BindedEarlyZ;

		if (mNoTextures)
			currentPassId = eTechWallPass_EarlyZNoTextures;
		else if (mBindless)
			currentPassId = eTechWallPass_BindlessEarlyZ;
	}
	
	//
	if (nullptr != fx_TechWallMaterial)
	{
		mCurrentLoc = &mWallLocations[currentPassId];
		fx_pass = fx_TechWallMaterial->getPass(currentPassId);
	}
	else
	if (mLogDepth && nullptr != fx_TechWallMaterialLog)
	{
		mCurrentLoc = &mWallLogLocations[currentPassId];
		fx_pass = fx_TechWallMaterialLog->getPass(currentPassId);
	} 
}

void ShaderEffect::PrepCharacter_PassAndLocations()
{
	ETechCharacterPasses passId = eTechCharacterPass_IBL;
	if (true == mEyePass)
		passId = eTechCharacterPass_Eye;

	if (mEarlyZ)
		passId = eTechCharacterPass_Simple;
	else if (mNoTextures)
		passId = eTechCharacterPass_NoTextures;

	mCurrentLoc = &mCharacterLocations[passId];
	fx_pass = fx_TechCharacter->getPass(passId);
}

bool ShaderEffect::PrepLogDepth()
{
	return true;
	// !!!!
	bool result = true;

	if (mLogDepth == mOldLogDepth)
		return true;

	int Sz = 1; // fx_EffectMaterial->getNumTechniques();
	nvFX::ITechnique** techs = new nvFX::ITechnique*[Sz];
	//for(int t=0; t<Sz ; t++)
	//	techs[t] = fx_EffectMaterial->findTechnique(t);
	techs[0] = fx_TechWallMaterial;

	if (mLogDepth)
	{
		result = fx_TechDepthOverrides->getPass( eTechDepthOverride_Log )->setupOverrides( techs, 1 );
	}
	else
	{
		result = fx_TechDepthOverrides->getPass( eTechDepthOverride_Linear )->setupOverrides( techs, 1 );
	}

	if (result)
	{
		
		//result = InitializeEffectParams();
		result = techs[0]->validate();
		//PrepCurrent();
	}

	delete [] techs;
	techs = nullptr;

	mOldLogDepth = mLogDepth;



	return result;
}

void ShaderEffect::SetNumberOfShadows( const int numberOfShadows )
{
	if (fx_numberOfShadows)
	{
		fx_numberOfShadows->setValue1i( numberOfShadows );
	}
}
/*
void ShaderEffect::SetDepthDisplacement(const float value)
{
	if (fx_depthDisplacement)
	{
		fx_depthDisplacement->setValue1f( value );
	}
}

void ShaderEffect::UpdateDepthDisplacement(const float value)
{
	if (fx_depthDisplacement && fx_pass)
	{
		fx_depthDisplacement->updateValue1f( value, fx_pass );
	}
}
*/
void ShaderEffect::SetMeshIndex( const int index )
{
	if (fx_meshIndex)
	{
		fx_meshIndex->setValue1i( index );
	}
}

void ShaderEffect::UpdateMeshIndex( const int index )
{
	if (fx_meshIndex && fx_pass)
	{
		fx_meshIndex->updateValue1i( index, fx_pass );
	}
}

void ShaderEffect::SetTextureOffset(const vec4 &v)
{
	if (fx_textureOffset)
	{
		fx_textureOffset->setValue4fv( (float*) v.vec_array );
	}
}

void ShaderEffect::UpdateTextureOffset(const vec4 &v)
{
	if (fx_textureOffset && fx_pass)
	{
		fx_textureOffset->updateValue4fv( (float*) v.vec_array, fx_pass );
	}
}

void ShaderEffect::SetTextureScaling(const vec4 &v)
{
	if (fx_textureScaling)
	{
		fx_textureScaling->setValue4fv( (float*) v.vec_array );
	}
}

void ShaderEffect::UpdateTextureScaling(const vec4 &v)
{
	if (fx_textureScaling && fx_pass)
	{
		fx_textureScaling->updateValue4fv( (float*) v.vec_array, fx_pass );
	}
}

void ShaderEffect::SetSoftParticles(const float value)
{
	if (fx_softParticles)
	{
		fx_softParticles->setValue1f( value );
	}
}

void ShaderEffect::UpdateSoftParticles(const float value )
{
	if (fx_softParticles && fx_pass)
	{
		fx_softParticles->updateValue1f( value, fx_pass );
	}
}

/*
void ShaderEffect::SetRimParameters( const double useRim, const double rimPower, const bool useRimTexture, const vec4 &rimColor )
{
	if (fx_rimOptions)
		fx_rimOptions->setValue4f( (float) useRim, (float) rimPower, (useRimTexture) ? 1.0f : 0.0f, 0.0f );
	if (fx_rimColor)
		fx_rimColor->setValue4fv( (float*) rimColor.vec_array );
}

void ShaderEffect::UpdateRimParameters( const double useRim, const double rimPower, const bool useRimTexture, const vec4 &rimColor )
{
	if (fx_pass)
	{
		if (fx_rimOptions)
			fx_rimOptions->updateValue4f( (float) useRim, (float) rimPower, (useRimTexture) ? 1.0f : 0.0f, 0.0f, fx_pass );
		if (fx_rimColor)
			fx_rimColor->updateValue4fv( (float*) rimColor.vec_array, fx_pass );
	}
}


void ShaderEffect::SetShaderMask(const vec4 &mask)
{
	if (fx_shaderMask)
	{
		fx_shaderMask->setValue4fv( (float*) mask.vec_array );
	}
}

void ShaderEffect::UpdateShaderMask(const vec4 &mask)
{
	if (fx_shaderMask && fx_pass)
	{
		fx_shaderMask->updateValue4fv( (float*) mask.vec_array, fx_pass );
	}
}
*/
void ShaderEffect::SetNumberOfProjectors( const int numberOfProjectors )
{
	if (fx_numberOfProjectors)
	{
		fx_numberOfProjectors->setValue1i( numberOfProjectors );
	}
}

void ShaderEffect::UpdateNumberOfProjectors( const int numberOfProjectors )
{
	if (fx_numberOfProjectors && fx_pass)
	{
		fx_numberOfProjectors->updateValue1i( numberOfProjectors, fx_pass );
	}
}


//-----------------------------------------------------------------------------
// scene instances, depending on the scene level needs
//-----------------------------------------------------------------------------
bool ShaderEffect::validateAndCreateSceneInstances()
{
    bool failed = false;
    //
    // gather techniques of materials that we want to instanciate
    // depending on scene effects
    //
    int Sz = fx_EffectMaterial->getNumTechniques();
    nvFX::ITechnique** techs = new nvFX::ITechnique*[Sz];
    for(int t=0; t<Sz ; t++)
        techs[t] = fx_EffectMaterial->findTechnique(t);
    //
    // validate scene-level techniques
    //
	
    nvFX::ITechnique* scTech;
    for(int t=0; ; t++)
    {
		scTech = fx_EffectMaterial->findTechnique(t);
        if(scTech == NULL)
            break;
        bool bRes = scTech->validate();
        if(!bRes)
        {
            printf("Error>> couldn't validate the scene technique %d\n", t);
            failed  = true;
        }
        int np = scTech->getNumPasses();
        for(int i=0; i<np; i++)
        {
            // in this special example, only one 'material' technique is used...
            nvFX::IPass* p  = scTech->getPass(i);
            //
            // instanciate what is inside the array
            //
            bool bRes       = p->setupOverrides(techs, Sz);
			
        }
    }
    //
    // Let's find all the techniques and validates them
    // subsequent passes will get validated and related shaders/programs
    // will be created
    //
    for(int t=0; t<Sz ; t++)
    {
        bool bRes = techs[t]->validate();
        if(!bRes)
        {
            printf("Error>> couldn't validate the material technique %s\n", techs[t]->getName());
            failed  = true;
        }
    }
    delete [] techs;
    return failed;
}

bool ShaderEffect::InitializeEffectParams()
{
	if (false == ShaderBase::InitializeEffectParams() )
		return false;
	
	

	/*
	//
	fx_TechPhongNoTextures = fx_EffectMaterial->findTechnique("PhongNoTextures");
	if(fx_TechPhongNoTextures && (!fx_TechPhongNoTextures->validate()))
        return false;

	fx_TechPhong = fx_EffectMaterial->findTechnique("Phong");
	if(fx_TechPhong && (!fx_TechPhong->validate()))
        return false;
	fx_TechPhongLog = fx_EffectMaterial->findTechnique("PhongLog");
	if(fx_TechPhongLog && (!fx_TechPhongLog->validate()))
        return false;
	
	fx_TechPhongBindless = fx_EffectMaterial->findTechnique("PhongBindless");
	if(fx_TechPhongBindless && (!fx_TechPhongBindless->validate()))
        return false;
	fx_TechPhongBindlessLog = fx_EffectMaterial->findTechnique("PhongBindlessLog");
	if(fx_TechPhongBindlessLog && (!fx_TechPhongBindlessLog->validate()))
        return false;

	fx_TechPhongCubeMap = fx_EffectMaterial->findTechnique("PhongCubeMap");
	if(fx_TechPhongCubeMap && (!fx_TechPhongCubeMap->validate()))
        return false;

	fx_TechPhongBindlessCubeMap = fx_EffectMaterial->findTechnique("PhongBindlessCubeMap");
	if(fx_TechPhongBindlessCubeMap && (!fx_TechPhongBindlessCubeMap->validate()))
        return false;
		*/

	//
	fx_TechWallMaterial = fx_EffectMaterial->findTechnique("WallMaterial");
	if(fx_TechWallMaterial && (!fx_TechWallMaterial->validate()))
        return false;

	fx_TechWallMaterialLog = fx_EffectMaterial->findTechnique("WallMaterialLog");
	if(fx_TechWallMaterialLog && (!fx_TechWallMaterialLog->validate()))
        return false;

	fx_TechDepthOverrides = fx_EffectMaterial->findTechnique("DepthOverrides");
	if(fx_TechDepthOverrides && (!fx_TechDepthOverrides->validate()))
        return false;
		
	fx_TechCharacter = fx_EffectMaterial->findTechnique("CharacterT");
	if(fx_TechCharacter && (!fx_TechCharacter->validate()))
        return false;

	fx_TechShadow = fx_EffectMaterial->findTechnique("ShadowT");
	if(fx_TechShadow && (!fx_TechShadow->validate()))
        return false;

	if (false == PrepLogDepth() )
		return false;

	//if (false == validateAndCreateSceneInstances() )
	//	return false;

	/*
	fx_TechCulling = fx_EffectMaterial->findTechnique("Culling");
	if(fx_TechCulling && (!fx_TechCulling->validate()))
        return false;
	fx_TechNormals = fx_EffectMaterial->findTechnique("Normals");
	if(fx_TechNormals && (!fx_TechNormals->validate()))
        return false;
	fx_TechNormalsLog = fx_EffectMaterial->findTechnique("NormalsLog");
	if(fx_TechNormalsLog && (!fx_TechNormalsLog->validate()))
        return false;
	fx_TechSimple = fx_EffectMaterial->findTechnique("Simple");
	if(fx_TechSimple && (!fx_TechSimple->validate()))
        return false;
	*/

	//
	
	if (mBufferTransfBlock[0] == 0)
		CreateUniformBuffers();

    fx_transfBlock1 = fx_EffectMaterial->findCstBuffer("transfBlock1");
	if (fx_transfBlock1)
		fx_transfBlock1->setGLBuffer(mBufferTransfBlock[0]); // ask nvFx to take care of the UBO creation
	

	fx_transfBlock2 = fx_EffectMaterial->findCstBuffer("transfBlock2");
	if (fx_transfBlock2)
		fx_transfBlock2->setGLBuffer(mBufferTransfBlock[1]);
	
	fx_frustumBlock = fx_EffectMaterial->findCstBuffer("frustumBlock");
	if (fx_frustumBlock)
		fx_frustumBlock->setGLBuffer(mBufferFrustumBlock);

	fx_lightBlock = fx_EffectMaterial->findCstBuffer("lightBlock");
	if (fx_lightBlock)
		fx_lightBlock->setGLBuffer(mBufferLightBlock);

	fx_cubeMapBlock = fx_EffectMaterial->findCstBuffer("cubeMapBlock");
	if (fx_cubeMapBlock)
		fx_cubeMapBlock->setGLBuffer(mBufferCubeMapBlock);
	//

	fx_Fcoef = fx_EffectMaterial->findUniform("Fcoef");
	if (fx_Fcoef)
		fx_Fcoef->setValue1f( 1.0f );
	fx_depthDisplacement = fx_EffectMaterial->findUniform("depthDisplacement");
	if (fx_depthDisplacement)
		fx_depthDisplacement->setValue1f( 0.0f );

	fx_meshIndex = fx_EffectMaterial->findUniform( "gMeshIndex" );
	fx_textureOffset = fx_EffectMaterial->findUniform("textureOffset");
	fx_textureScaling = fx_EffectMaterial->findUniform("textureScaling");
	fx_softParticles = fx_EffectMaterial->findUniform("softParticles");
	//fx_shaderMask = fx_EffectMaterial->findUniform("shaderMask");

	//fx_rimOptions = fx_EffectMaterial->findUniform("gRimOptions" );
	//fx_rimColor = fx_EffectMaterial->findUniform("gRimColor");

	fx_normalsLength = fx_EffectMaterial->findUniform("normal_length");
	fx_AlphaPass = fx_EffectMaterial->findUniform("AlphaPass");
	if (fx_AlphaPass)
		fx_AlphaPass->setValue1f( 0.0f );
	fx_numberOfProjectors = fx_EffectMaterial->findUniform("numProjectors");
	fx_numberOfShadows = fx_EffectMaterial->findUniform("numberOfShadows");

	//
	if (nullptr != fx_TechWallMaterial)
	{
		for (int i=0; i<eTechWallPass_Count; ++i)
		{
			nvFX::IPass *pPass = fx_TechWallMaterial->getPass(i);
			mWallLocations[i].SetShadersId( ShaderBase::nvGetFragmentProgramId( pPass, 0, 0 ),
											ShaderBase::nvGetFragmentProgramId( pPass, 0, 1 ) );
		}
	}

	if (nullptr != fx_TechWallMaterialLog)
	{
		for (int i=0; i<eTechWallPass_Count; ++i)
		{
			nvFX::IPass *pPass = fx_TechWallMaterialLog->getPass(i);
			mWallLogLocations[i].SetShadersId( ShaderBase::nvGetFragmentProgramId( pPass, 0, 0 ),
											ShaderBase::nvGetFragmentProgramId( pPass, 0, 1 ) );
		}
	}

	if (nullptr != fx_TechCharacter)
	{
		for (int i=0; i<eTechCharacterPass_Count; ++i)
		{
			nvFX::IPass *pPass = fx_TechCharacter->getPass(i);
			mCharacterLocations[i].SetShadersId( ShaderBase::nvGetFragmentProgramId( pPass, 0, 0 ),
											ShaderBase::nvGetFragmentProgramId( pPass, 0, 1 ) );
		}
	}

	if (nullptr != fx_TechShadow)
	{
		nvFX::IPass *pPass = fx_TechShadow->getPass(0);
		mShadowLoc.SetShadersId( ShaderBase::nvGetFragmentProgramId( pPass, 0, 0 ),
											ShaderBase::nvGetFragmentProgramId( pPass, 0, 2 ) );
	}

	mCurrentLoc = nullptr;

	if (false == PrepCommonLocations() )
		return false;

	return true;
}

nvFX::IUniform	*ShaderEffect::FindUniform(const char *name)
{
	if (nullptr != fx_EffectMaterial)
		return fx_EffectMaterial->findUniform(name);

	return nullptr;
}

bool ShaderEffect::PrepCommonLocations()
{
	if (false == ShaderBase::PrepCommonLocations() )
		return false;
	
	auto find_locations = [] (CustomEffectShaderLocations &loc) -> bool
	{
		int vertexCount = loc.PrepVertexLocations();
		int fragmentCount = loc.PrepFragmentLocations();
		int samplersCount = loc.PrepDefaultSamplerSlots();

		if (vertexCount == 0 || fragmentCount == 0)
			return false;

		return true;
	};

	if (nullptr != fx_TechWallMaterial)
	{
		for (int i=0; i<eTechWallPass_Count; ++i)
		{
			find_locations( mWallLocations[i] );
		}
	}

	if (nullptr != fx_TechWallMaterialLog)
	{
		for (int i=0; i<eTechWallPass_Count; ++i)
		{
			find_locations( mWallLogLocations[i] );
		}
	}

	if (nullptr != fx_TechCharacter)
	{
		for (int i=0; i<eTechCharacterPass_Count; ++i)
		{
			find_locations( mCharacterLocations[i] );
		}
	}

	find_locations( mShadowLoc );

	return true;
}

void ShaderEffect::UploadLightingInformation(const bool mapOnGPU, const vec4 &ambientLightColor, const int numDirLights, const int numPointLights)
{
	g_lightBlock.globalAmbientLight = ambientLightColor;
	g_lightBlock.numDirLights = numDirLights;
	g_lightBlock.numPointLights = numPointLights;

	// copy the block to OGL
	if(true == mapOnGPU && nullptr != fx_lightBlock)
    {

		const int bufferId = fx_lightBlock->getGLBuffer();
		const size_t size = sizeof(lightBlock);
		if (bufferId > 0 && size > 0)
		{
			const void *data = &g_lightBlock;

			glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
			glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else
		{
			printf( "empty buffer data\n" );
		}
		/*
        void* p;
        if (true == fx_lightBlock->mapBuffer(&p) )
		{
			memcpy(p, &g_lightBlock, sizeof(lightBlock));
			fx_lightBlock->unmapBuffer();
		}
		*/
    }

	CHECK_GL_ERROR();
}

void ShaderEffect::UploadLightingInformation(const int numDirLights, const int numPointLights)
{
	g_lightBlock.numDirLights = numDirLights;
	g_lightBlock.numPointLights = numPointLights;

	// copy the block to OGL
	if(fx_lightBlock)
    {
		const int bufferId = fx_lightBlock->getGLBuffer();
		const size_t size = sizeof(lightBlock);
		if (bufferId > 0 && size > 0)
		{
			const void *data = &g_lightBlock;

			glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
			glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else
		{
			printf( "empty buffer data\n" );
		}
		/*
        void* p;
        if (true == fx_lightBlock->mapBuffer(&p) )
		{
			memcpy(p, &g_lightBlock, sizeof(lightBlock));
			fx_lightBlock->unmapBuffer();
		}
		*/
    }

	CHECK_GL_ERROR();
}

void ShaderEffect::UploadFrustumPlanes(CFrustum &frustum)
{
	memcpy( g_frustumBlock.planes[0].vec_array, frustum.GetFrustumPlane(0), sizeof(float)*4*6 );
	

	// copy the block to OGL
	if(fx_frustumBlock)
    {
		const int bufferId = fx_frustumBlock->getGLBuffer();
		const size_t size = sizeof(frustumBlock);
		if (bufferId > 0 && size > 0)
		{
			const void *data = &g_frustumBlock;

			glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
			glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else
		{
			printf( "empty buffer data\n" );
		}
		/*
        void* p;
        if (true == fx_frustumBlock->mapBuffer(&p) )
		{
			memcpy(p, &g_frustumBlock, sizeof(frustumBlock));
			fx_frustumBlock->unmapBuffer();
		}
		*/
    }

	CHECK_GL_ERROR();
}

void generate_projections(const float zmin, const float zmax, glm::mat4 *cubeMapProjections) {
    
	glm::mat4 projMatrix = glm::perspective(90.0f, 1.0f, zmin, zmax);

	//Negative X
	cubeMapProjections[0] = projMatrix * glm::lookAt(glm::vec3(0), glm::vec3(1,0,0),glm::vec3(0,-1,0));
	//Positive X
    cubeMapProjections[1] = projMatrix * glm::lookAt(glm::vec3(0), glm::vec3(-1,0,0),glm::vec3(0,-1,0));
	//Positive Y
    cubeMapProjections[2] = projMatrix * glm::lookAt(glm::vec3(0), glm::vec3(0,1,0),glm::vec3(0,0,1));
	//Negative Y
    cubeMapProjections[3] = projMatrix * glm::lookAt(glm::vec3(0), glm::vec3(0,-1,0),glm::vec3(0,0,-1));
	//Positive Z
    cubeMapProjections[4] = projMatrix * glm::lookAt(glm::vec3(0), glm::vec3(0,0,1),glm::vec3(0,-1,0));
	//Negative Z
    cubeMapProjections[5] = projMatrix * glm::lookAt(glm::vec3(0), glm::vec3(0,0,-1),glm::vec3(0,-1,0));
    
}

void ShaderEffect::UploadCubeMapUniforms(const float zmin, const float zmax, const glm::mat4 &worldToLocal, const glm::vec3 &position, const glm::vec3 &max, const glm::vec3 &min, const float useParallax)
{
	generate_projections(zmin, zmax, g_cubeMapBlock.cubeMapProjections);

	g_cubeMapBlock.cubeMapWorldToLocal = worldToLocal;
	g_cubeMapBlock.cubeMapPosition = glm::vec4( position, 1.0 );
	g_cubeMapBlock.cubeMapMax = glm::vec4( max, 1.0 );
	g_cubeMapBlock.cubeMapMin = glm::vec4( min, 1.0 );
	g_cubeMapBlock.cubeMapParallaxCorrection = useParallax;

	// copy the block to OGL
    if(fx_cubeMapBlock)
    {
		const int bufferId = fx_cubeMapBlock->getGLBuffer();
		const size_t size = sizeof(cubeMapBlock);
		if (bufferId > 0 && size > 0)
		{
			const void *data = &g_cubeMapBlock;

			glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
			glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else
		{
			printf( "empty buffer data\n" );
		}
		/*
        void* p;
        if (true == fx_cubeMapBlock->mapBuffer(&p) )
		{
			memcpy(p, &g_cubeMapBlock, sizeof(cubeMapBlock));
			fx_cubeMapBlock->unmapBuffer();
		}
		*/
    }

	CHECK_GL_ERROR();
}

void ShaderEffect::UploadCameraUniforms(const CCameraInfoCache &cacheInfo)
{
//
	// Pre render - setup global scene matrix
	//
	
	float Fcoef = (float) (2.0 / log2(cacheInfo.farPlane + 1.0));
	if (fx_Fcoef)
		fx_Fcoef->setValue1f( Fcoef );

	/*
	if (fx_ScreenWidth) fx_ScreenWidth->setValue1i(cacheInfo.width);
	if (fx_ScreenHeight) fx_ScreenHeight->setValue1i(cacheInfo.height);
	*/
	if (fx_ScreenSize)
		fx_ScreenSize->setValue4f((float)cacheInfo.offsetX, (float)cacheInfo.offsetY,
							1.0f/(float)cacheInfo.width, 1.0f/(float)cacheInfo.height);

	CHECK_GL_ERROR();

	// g_transfBlock1.m4_Proj already done
	g_transfBlock1.m4_Proj = cacheInfo.p4;
	g_transfBlock1.m4_ViewProjI = cacheInfo.proj2d;

	g_transfBlock1.m4_View = cacheInfo.mv4;
	g_transfBlock1.m4_ViewIT = cacheInfo.mvInv4;
    /*
	for (int i=0; i<16; ++i)
	{
		g_transfBlock1.m4_Proj.mat_array[i] = (float) projection[i];
		g_transfBlock1.m4_View.mat_array[i] = (float) modelview[i];
		g_transfBlock1.m4_ViewIT.mat_array[i] = (float) viewIT[i];
	}
	*/
	//
    // setup the block1 with base matrices
    //
    
    //g_transfBlock1.m4_ViewIT = ...todo
    g_transfBlock1.m4_ViewProj = g_transfBlock1.m4_Proj * g_transfBlock1.m4_View;
	//g_transfBlock1.eyePos.x = (float) cameraPos[0];
	//g_transfBlock1.eyePos.y = (float) cameraPos[1];
	//g_transfBlock1.eyePos.z = (float) cameraPos[2];
	//g_transfBlock1.farPlane = (realFarPlane) ? *realFarPlane : (float)farPlane;	// this is for lights clusters
	
	g_transfBlock1.eyePos = cacheInfo.pos;
	g_transfBlock1.farPlane = (float) cacheInfo.farPlane;

    // copy the block to OGL
    if(fx_transfBlock1)
    {
		const int bufferId = fx_transfBlock1->getGLBuffer();
		const size_t size = sizeof(transfBlock1);
		if (bufferId > 0 && size > 0)
		{
			const void *data = &g_transfBlock1;

			glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
			glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else
		{
			printf( "empty buffer data\n" );
		}
		/*
        void* p;
        if (true == fx_transfBlock1->mapBuffer(&p) )
		{
			memcpy(p, &g_transfBlock1, sizeof(transfBlock1));
			fx_transfBlock1->unmapBuffer();
		}
		*/
    }

	CHECK_GL_ERROR();
	/*
	//
    // default values for second block made of World...
    //
    g_transfBlock2.m4_World.identity();
    g_transfBlock2.m4_WorldView = g_transfBlock1.m4_View * g_transfBlock2.m4_World;
    g_transfBlock2.m4_WorldViewProj = g_transfBlock1.m4_Proj * g_transfBlock2.m4_WorldView;
    //g_transfBlock2.m4_WorldIT = ... todo;
    if(fx_transfBlock2)
    {
        void* p;
		if (true == fx_transfBlock2->mapBuffer(&p) )
		{
			memcpy(p, &g_transfBlock2, sizeof(transfBlock2));
			fx_transfBlock2->unmapBuffer();
		}
    }
	*/
}
/*
void ShaderEffect::UploadCameraUniforms(const mat4 &modelview, const mat4 &projection, const mat4 &viewIT, 
			const vec3 &cameraPos, const int width, const int height, const float farPlane, const float *realFarPlane)
{
//
	// Pre render - setup global scene matrix
	//
	
	float Fcoef = (float) (2.0 / log2(farPlane + 1.0));
	if (fx_Fcoef)
		fx_Fcoef->setValue1f( Fcoef );

	if (fx_ScreenWidth) fx_ScreenWidth->setValue1i(width);
	if (fx_ScreenHeight) fx_ScreenHeight->setValue1i(height);

	CHECK_GL_ERROR();

	// g_transfBlock1.m4_Proj already done
	g_transfBlock1.m4_Proj = projection;
	g_transfBlock1.m4_View = modelview;
	g_transfBlock1.m4_ViewIT = viewIT;
   
	//
    // setup the block1 with base matrices
    //
    
    //g_transfBlock1.m4_ViewIT = ...todo
    g_transfBlock1.m4_ViewProj = g_transfBlock1.m4_Proj * g_transfBlock1.m4_View;
	//g_transfBlock1.eyePos.x = (float) cameraPos[0];
	//g_transfBlock1.eyePos.y = (float) cameraPos[1];
	//g_transfBlock1.eyePos.z = (float) cameraPos[2];
	//g_transfBlock1.farPlane = (realFarPlane) ? *realFarPlane : (float)farPlane;	// this is for lights clusters
	
	g_transfBlock1.eyePos = cameraPos;
	g_transfBlock1.farPlane = (float) farPlane;

    // copy the block to OGL
    if(fx_transfBlock1)
    {
        void* p;
        if (true == fx_transfBlock1->mapBuffer(&p) )
		{
			memcpy(p, &g_transfBlock1, sizeof(transfBlock1));
			fx_transfBlock1->unmapBuffer();
		}
    }

	CHECK_GL_ERROR();
	
}
*/
void ShaderEffect::UploadCameraUniforms(const float *realFarPlane)
{
    // copy the block to OGL
    if(fx_transfBlock1 && nullptr != realFarPlane)
    {

		g_transfBlock1.farPlane = *realFarPlane;

		const int bufferId = fx_transfBlock1->getGLBuffer();
		const size_t size = sizeof(transfBlock1);
		if (bufferId > 0 && size > 0)
		{
			const void *data = &g_transfBlock1;

			glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
			glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else
		{
			printf( "empty buffer data\n" );
		}

		/*
        void* p;
        if (true == fx_transfBlock1->mapBuffer(&p) )
		{
			memcpy(p, &g_transfBlock1, sizeof(transfBlock1));
			fx_transfBlock1->unmapBuffer();
		}
		*/
    }
}

void ShaderEffect::UploadModelTransform(const mat4 &m)
{
	//
    // default values for second block made of World...
    //
	/*
    g_transfBlock2.m4_World.identity();

	for (int i=0; i<16; ++i)
		g_transfBlock2.m4_World.mat_array[i] = (float) m[i];
		*/
	g_transfBlock2.m4_World = m;

    g_transfBlock2.m4_WorldView = g_transfBlock1.m4_View * g_transfBlock2.m4_World;
    g_transfBlock2.m4_WorldViewProj = g_transfBlock1.m4_Proj * g_transfBlock2.m4_WorldView;
    //g_transfBlock2.m4_WorldIT = ... todo;

	invert(g_transfBlock2.m4_WorldIT, g_transfBlock2.m4_WorldView);
	transpose(g_transfBlock2.m4_WorldIT);

	const int bufferId = fx_transfBlock2->getGLBuffer();
	const size_t size = sizeof(transfBlock2);
	if (bufferId > 0 && size > 0)
	{
		const void *data = &g_transfBlock2;

		glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	else
	{
		printf( "empty buffer data\n" );
	}
	/*
    if(fx_transfBlock2)
    {
        void* p;
		if (true == fx_transfBlock2->mapBuffer(&p) )
		{
			memcpy(p, &g_transfBlock2, sizeof(transfBlock2));
			fx_transfBlock2->unmapBuffer();
		}
    }
	*/
}

void ShaderEffect::UploadModelTransform(const double *m)
{
	//
    // default values for second block made of World...
    //
	
    g_transfBlock2.m4_World.identity();

	for (int i=0; i<16; ++i)
		g_transfBlock2.m4_World.mat_array[i] = (float) m[i];
	
    g_transfBlock2.m4_WorldView = g_transfBlock1.m4_View * g_transfBlock2.m4_World;
    g_transfBlock2.m4_WorldViewProj = g_transfBlock1.m4_Proj * g_transfBlock2.m4_WorldView;
    //g_transfBlock2.m4_WorldIT = ... todo;

	invert(g_transfBlock2.m4_WorldIT, g_transfBlock2.m4_WorldView);
	transpose(g_transfBlock2.m4_WorldIT);

	const int bufferId = fx_transfBlock2->getGLBuffer();
	const size_t size = sizeof(transfBlock2);
	if (bufferId > 0 && size > 0)
	{
		const void *data = &g_transfBlock2;

		glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	else
	{
		printf( "empty buffer data\n" );
	}
	/*
    if(fx_transfBlock2)
    {
        void* p;
		if (true == fx_transfBlock2->mapBuffer(&p) )
		{
			memcpy(p, &g_transfBlock2, sizeof(transfBlock2));
			fx_transfBlock2->unmapBuffer();
		}
    }
	*/
}

void ShaderEffect::UploadLightTransform(const mat4 &proj, const mat4 &view, const mat4 &m)
{
	//
    // default values for second block made of World...
    //
	/*
    g_transfBlock2.m4_World.identity();

	for (int i=0; i<16; ++i)
	{
		g_transfBlock1.m4_Proj.mat_array[i] = (float) proj[i];
		g_transfBlock1.m4_View.mat_array[i] = (float) view[i];
		g_transfBlock2.m4_World.mat_array[i] = (float) m[i];
	}
	*/

	g_transfBlock1.m4_Proj = proj;
	g_transfBlock1.m4_View = view;
	g_transfBlock1.m4_ViewProj = g_transfBlock1.m4_Proj * g_transfBlock1.m4_View;
	g_transfBlock2.m4_World = m;

    g_transfBlock2.m4_WorldView = g_transfBlock1.m4_View * g_transfBlock2.m4_World;
    g_transfBlock2.m4_WorldViewProj = g_transfBlock1.m4_Proj * g_transfBlock2.m4_WorldView;
    //g_transfBlock2.m4_WorldIT = ... todo;

	invert(g_transfBlock2.m4_WorldIT, g_transfBlock2.m4_WorldView);
	transpose(g_transfBlock2.m4_WorldIT);

	g_transfBlock1.eyePos = vec3(-view.x, -view.y, -view.z);
	g_transfBlock1.farPlane = 4000.0; // (float) cacheInfo.farPlane;


    // copy the block to OGL

    if(fx_transfBlock1)
    {
		const int bufferId = fx_transfBlock1->getGLBuffer();
		const size_t size = sizeof(transfBlock1);
		if (bufferId > 0 && size > 0)
		{
			const void *data = &g_transfBlock1;

			glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
			glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else
		{
			printf( "empty buffer data\n" );
		}
		/*
        void* p;
        if (true == fx_transfBlock1->mapBuffer(&p) )
		{
			memcpy(p, &g_transfBlock1, sizeof(transfBlock1));
			fx_transfBlock1->unmapBuffer();
		}
		*/
    }

    if(fx_transfBlock2)
    {
		const int bufferId = fx_transfBlock2->getGLBuffer();
		const size_t size = sizeof(transfBlock2);
		if (bufferId > 0 && size > 0)
		{
			const void *data = &g_transfBlock2;

			glBindBuffer(GL_UNIFORM_BUFFER, bufferId);

			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
			glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);

			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		else
		{
			printf( "empty buffer data\n" );
		}
		/*
        void* p;
		if (true == fx_transfBlock2->mapBuffer(&p) )
		{
			memcpy(p, &g_transfBlock2, sizeof(transfBlock2));
			fx_transfBlock2->unmapBuffer();
		}
		*/
    }
}

void ShaderEffect::UpdateAlphaPass(const float value)
{
	if (fx_AlphaPass)
	{
		fx_AlphaPass->updateValue1f(value, fx_pass);
	}
}

const GLuint ShaderEffect::GetVertexProgramId() const
{
	if (mCurrentLoc)
		return mCurrentLoc->GetVertexId();
	return 0;
}

const GLuint ShaderEffect::GetFragmentProgramId() const
{
	if (mCurrentLoc)
		return mCurrentLoc->GetFragmentId();
	return 0;
}

const GLuint ShaderEffect::FindFragmentProgramLocation(const char *name)
{
	GLuint loc = 0;
	if (mCurrentLoc)
		loc = glGetUniformLocation( mCurrentLoc->GetFragmentId(), name );
	return loc;
}

void ShaderEffect::UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount)
{
    //cgSetBufferSubData(mParamModelViewArrayBuffer, mParamModelViewArrayBufferOffset, 4 * 4 * sizeof (double) * pCount, pModelViewMatrixArray);
}


/*
void Shader::ModelRender( FBRenderOptions *pOptions, FBShaderModelInfo *pInfo )
{
	FBModelVertexData	*lVertexData = pInfo->GetFBModel()->ModelVertexData;



	// bind VAO or VBUM
	SuperShaderModelInfo *lSuperInfo = (SuperShaderModelInfo*) pInfo;
	lSuperInfo->Bind();

	if (firstQuery)
	{
		CGPUVertexData::QueryAttributes(0);	// query current program
		firstQuery = false;
	}
	
	// assign attribute with per mesh information
	const GLuint perMeshLocation = 4;
	GLuint64 perMeshUniformsGPUPtr = mPerMeshUniformsGPUPtr + sizeof(PerMeshInfo) * mModelIndex;
	glVertexAttribI2iEXT( perMeshLocation, (int)(perMeshUniformsGPUPtr & 0xFFFFFFFF), (int) (perMeshUniformsGPUPtr>>32) & (0xFFFFFFFF) );
	
	if (mMaterialsManager != nullptr)
	{
		if (pInfo->GetSubRegionIndex() == -1)
		{
			for (int i=0; i<lVertexData->GetSubPatchCount(); ++i)
			{
				if (lVertexData->GetSubPatchPrimitiveType(i) != kFBGeometry_TRIANGLES)
				{
					continue;
				}

				FBMaterial *pMaterial = lVertexData->GetSubPatchMaterial(i);
			
				// assign attribute with pointer to the material information
				if (pMaterial && mMaterialLoc < 4096 && mMaterialLoc > 0)
				{
					mMaterialsManager->BindSpecifiedMaterialAsUniform( mFragmentProgram, mMaterialLoc, pMaterial );
				}
			
				const int indexOffset = lVertexData->GetSubPatchIndexOffset(i);
				const int indexSize = lVertexData->GetSubPatchIndexSize(i);

				glDrawElements( GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, (void*) (indexOffset * sizeof(unsigned int)) );
			}
		}
		else
		{
			printf( "unsupported rendering condition\n" );
		}
	}

	lSuperInfo->UnBind();

	CHECK_GL_ERROR();

	// !!!!!!!!!!!!!
	mModelIndex++;
}
*/

void ShaderEffect::Bind()
{
	PrepCurrentTech();

	//PrepLogDepth();
	ShaderBase::Bind();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////// COMPOSITE EFFECT

ShaderComposite::ShaderComposite()
	: ShaderBase()
{
	
	fx_TechCompose = nullptr;
	fx_TechShadowsDiplay = nullptr;

	fx_MVP = nullptr;

	mShadowsDisplaySamplerLoc = 0;
}

ShaderComposite::~ShaderComposite()
{
	Free();
}

bool ShaderComposite::InitializeEffectParams()
{
	if (false == ShaderBase::InitializeEffectParams() )
		return false;

	//

	fx_TechCompose = fx_EffectMaterial->findTechnique("Projections");
	if(fx_TechCompose && (!fx_TechCompose->validate()))
        return false;
	
	fx_TechShadowsDiplay = fx_EffectMaterial->findTechnique("ShadowsDisplay");
	if(fx_TechShadowsDiplay && (!fx_TechShadowsDiplay->validate()))
        return false;

	fx_MVP = fx_EffectMaterial->findUniform("MVP");
	fx_numberOfShadows = fx_EffectMaterial->findUniform("numberOfShadows");

	fx_composeSampler0 = fx_EffectMaterial->findUniform("sampler0");
	fx_composeSampler1 = fx_EffectMaterial->findUniform("sampler1");
	fx_composeSampler2 = fx_EffectMaterial->findUniform("sampler2");
	fx_composeSampler3 = fx_EffectMaterial->findUniform("sampler3");
	fx_composeSampler4 = fx_EffectMaterial->findUniform("sampler4");
	fx_composeSampler5 = fx_EffectMaterial->findUniform("sampler5");
	fx_composeSampler6 = fx_EffectMaterial->findUniform("sampler6");
	fx_composeSampler7 = fx_EffectMaterial->findUniform("sampler7");

	if (fx_composeSampler0) fx_composeSampler0->setSamplerUnit(0);
	if (fx_composeSampler1) fx_composeSampler1->setSamplerUnit(1);
	if (fx_composeSampler2) fx_composeSampler2->setSamplerUnit(2);
	if (fx_composeSampler3) fx_composeSampler3->setSamplerUnit(3);
	if (fx_composeSampler4) fx_composeSampler4->setSamplerUnit(4);
	if (fx_composeSampler5) fx_composeSampler5->setSamplerUnit(5);
	if (fx_composeSampler6) fx_composeSampler6->setSamplerUnit(6);
	if (fx_composeSampler7) fx_composeSampler7->setSamplerUnit(7);

	PrepCommonLocations();

	return true;
}

bool ShaderComposite::PrepCommonLocations()
{
	if (false == ShaderBase::PrepCommonLocations() )
		return false;

	// sampler for displaing shadow textures
	fx_pass = fx_TechShadowsDiplay->getPass(0);
	const GLuint fragment = ShaderBase::nvGetFragmentProgramId(fx_pass, 0, 1);
	if (fragment > 0)
	{
		mShadowsDisplaySamplerLoc = glGetUniformLocation( fragment, "shadowsSampler" );

		if (mShadowsDisplaySamplerLoc >= 0)
			glProgramUniform1i( fragment, mShadowsDisplaySamplerLoc, 0 );
	}

	return true;
}

void ShaderComposite::SetTechnique(const ECompositeTechnique technique)
{
	switch(technique)
	{
	case eCompositeTechniqueProjections:
		fx_TechCurrent = fx_TechCompose;
		break;
	case eCompositeTechniqueShadowsDisplay:
		fx_TechCurrent = fx_TechShadowsDiplay;
		break;
	default:
		fx_TechCurrent = nullptr;
	}

	if (fx_TechCurrent) fx_pass = fx_TechCurrent->getPass(0);
}

void ShaderComposite::SetMVP( float *mvp )
{
	if (fx_MVP)
	{
		fx_MVP->setMatrix4f( mvp );
	}
}

void ShaderComposite::SetNumberOfShadows( const int count )
{
	if (fx_numberOfShadows)
	{
		fx_numberOfShadows->setValue1i(count);
	}
}

} // Graphics namespace
