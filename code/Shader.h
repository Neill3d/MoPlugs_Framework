
#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// wrapper around nvFX effects with implementing 2d composition technique 
//	and 3d gpu geometry cache, super lighting shader shading
//
//	Author Sergey Solokhin (Neill3d) 2014
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
// This shader is to fully take control in our hands for rendering in the MoBu

#   define  LOGI(...)  printf(__VA_ARGS__)
#   define  LOGW(...)  printf(__VA_ARGS__)
#   define  LOGE(...)  printf(__VA_ARGS__)
#   define  LOGOK(...)  printf(__VA_ARGS__)

//-- 
#include <GL\glew.h>
#include <vector>
//#include "nv_math.h"
#include <map>

//-------------- SDK include
//#include <fbsdk/fbsdk.h>

//-------------- Effect system
#include "FxParser.h"

//-------------- MCL
#include "Types.h"
#include "shared_models.h"
#include "shared_camera.h"

#include "glm\glm.hpp"

#define kMaxDrawInstancedSize  100

namespace Graphics
{

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Shader class

	class ShaderBase
	{
	public:
		//! a constructor
		ShaderBase();
		//! a destructor
		virtual ~ShaderBase();

        // Call after contruction.
        virtual bool	Initialize(const char* resourcePath, const char *effectName, const int W, const int H, const double globalScale);
		// on destructor
		virtual void	Free();
		// not used
		void	ReSize(const int w, const int h);

	
		// bind and unbind a zero pass of a current technique
		virtual void		Bind();
		virtual void		UnBind(const bool unlockTextures=true);

		void	SetWindowSize( const int offsetX, const int offsetY, const int width, const int height );

		void UnsetTextures();

		virtual const GLuint		GetFragmentProgramId();
		virtual const GLuint		FindFragmentProgramLocation(const char *name);

	protected:

		static void ShowError(const char* pText);

		//
		// stuff effect, tech and pass interfaces
		//
		nvFX::IContainer	*fx_EffectMaterial;

		nvFX::ITechnique	*fx_TechCurrent;
		nvFX::IPass			*fx_pass;

		//nvFX::IUniform		*fx_ScreenWidth;
		//nvFX::IUniform		*fx_ScreenHeight;
		nvFX::IUniform		*fx_ScreenSize;

		std::vector<nvFX::IResource*>	fx_resources;

		// Call those two functions around rendering.
		virtual void BindShaderPrograms();
        virtual void UnBindShaderPrograms(); 		
		
		virtual bool	InitializeEffectParams();
		virtual bool	PrepCommonLocations();

		static const GLuint	nvGetFragmentProgramId(nvFX::IPass *pass, const int programPipeline, const int shaderProgram);

		bool loadMaterialEffect(const char *effectFileName, const double globalScale);

	protected:

		int			mVertexProgram;
		int			mFragmentProgram;

	protected:

		ShaderBase(const ShaderBase&);
        ShaderBase& operator = (const ShaderBase&);
	};


	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//

	enum ECompositeTechnique
	{
		eCompositeTechniqueProjections,
		eCompositeTechniqueShadowsDisplay,
		eCompositeTechniqueCount
	};

	class ShaderComposite : public ShaderBase
	{
	public:

		//! a constructor
		ShaderComposite();
		//! a destructor
		virtual ~ShaderComposite();

		//
		void	SetTechnique( const ECompositeTechnique technique );
		
		void	SetMVP( float *mvp );
		void	SetNumberOfShadows( const int count );

	protected:
		// compose shader

		nvFX::ITechnique	*fx_TechCompose;
		nvFX::ITechnique	*fx_TechShadowsDiplay;

		nvFX::IUniform		*fx_MVP;
		nvFX::IUniform		*fx_numberOfShadows;

		nvFX::IUniform		*fx_composeSampler0;
		nvFX::IUniform		*fx_composeSampler1;
		nvFX::IUniform		*fx_composeSampler2;
		nvFX::IUniform		*fx_composeSampler3;
		nvFX::IUniform		*fx_composeSampler4;
		nvFX::IUniform		*fx_composeSampler5;
		nvFX::IUniform		*fx_composeSampler6;
		nvFX::IUniform		*fx_composeSampler7;

		GLint				mShadowsDisplaySamplerLoc;

		virtual bool	InitializeEffectParams();
		virtual bool	PrepCommonLocations();
	};


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ShaderEffect

	enum EEffectTechnique
	{
		eEffectTechniqueShading,
		eEffectTechniqueSimple,
		eEffectTechniqueCulling,
		eEffectTechniqueShadows,
		eEffectTechniqueNormals,
		eEffectTechniqueWallMaterial,
		eEffectTechniqueWallMaterialLog,
		eEffectTechniqueIBL,
		eEffectTechniqueCount
	};

	//
		enum ETechWallPasses
		{
			eTechWallPass_EarlyZNoTextures,
			eTechWallPass_BindedEarlyZ,
			eTechWallPass_BindlessEarlyZ,
			eTechWallPass_NoTextures,
			eTechWallPass_BindedTextures,
			eTechWallPass_BindlessTextures,
			eTechWallPass_Count
		};

		enum ETechCharacterPasses
		{
			eTechCharacterPass_Simple,
			eTechCharacterPass_NoTextures,
			eTechCharacterPass_IBL,
			eTechCharacterPass_Eye,
			eTechCharacterPass_Skin,
			eTechCharacterPass_Count
		};

	enum EEffectDepthHint
	{
		eEffectDepthUser,			// depends on a shader choise
		eEffectDepthLinear,			// force specified value
		eEffectDepthLogarithmic
	};

	enum CustomVertexShaderLocation
	{
		//
		// vertex locations
		
		eCustomVertexLocationAllTheShaders,
		eCustomVertexLocationAllTheMeshes,
		eCustomVertexLocationAllTheModels,

		//
		// eye shader

		eCustomVertexLocationIrisSize,
		eCustomVertexLocationCorneaBumpAmount,
		eCustomVertexLocationCorneaBumpRadiusMult,

		//
		eCustomVertexLocationCount
	};

	enum CustomFragmentShaderLocation
	{
		//
		// fragment locations
		
		eCustomLocationAmbient,
		eCustomLocationDiffuse,
		eCustomLocationTransparency,
		eCustomLocationSpecularity,
		eCustomLocationReflectivity,
		eCustomLocationNormalMap,
		eCustomLocationCubeMap,

		// global scene data needed for shader

		eCustomLocationAllTheTextures,
		eCustomLocationAllTheMaterials,
		eCustomLocationAllTheShaders,
		eCustomLocationAllTheProjectors,

		eCustomLocationClusterGrid,
		eCustomLocationClusterIndex,
		eCustomLocationDirLights,
		eCustomLocationLights,

		eCustomLocationShaderDirLights,
		eCustomLocationShaderLights,

			// 2d array textures (projections, shadows)
		eCustomLocationShadows,
		eCustomLocationLightMatrices,

			// array of samplers + mask samplers (8 samplers in total)
		eCustomLocationProjectors,
		eCustomLocationMaskA,
		eCustomLocationMaskB,

		eCustomLocationBackgroundSampler,
		eCustomLocationMatCapSampler,
		eCustomLocationRimSampler,			// texture that could define rim color in uv-space

		eCustomLocationDiffuseLightingSampler,
		eCustomLocationSpecularLightingSampler,
		eCustomLocationBrdfSampler,

		eCustomLocationReflCubeSampler,
		eCustomLocationMainDepthSampler,	// used for soft particles

		//
		// eye shader uniforms

		eCustomLocationEyeEnvReflectionSampler,
		eCustomLocationEyeEnvDiffuseSampler,
		eCustomLocationEyeEnvRefractionSampler,

		eCustomLocationPupilSize,
		eCustomLocationIrisTexStart,
		eCustomLocationIrisTexEnd,
		eCustomLocationIrisBorder,
		eCustomLocationIrisSize,
		eCustomLocationIrisEdgeFade,
		eCustomLocationIrisInsetDepth,
		eCustomLocationScleraTexScale,
		eCustomLocationScleraTexOffset,
		eCustomLocationIor,
		eCustomLocationRefractEdgeSoftness,

		eCustomLocationIrisTextureCurvature,
		eCustomLocationArgIrisShadingCurvature,
		
		eCustomLocationTexUOffset,
		eCustomLocationIrisNormalOffset,
		eCustomLocationCorneaDensity,
		eCustomLocationBumpTexture,
		eCustomLocationCatShape,
		eCustomLocationCybShape,
		eCustomLocationColTexture,

		eCustomFragmentLocationCount

	};

	struct CustomEffectShaderLocations
	{
	public:

		//! a constructor
		CustomEffectShaderLocations()
		{
			vertex = -1;
			fragment = -1;

			for (int i=0; i<eCustomVertexLocationCount; ++i)
				vertexLocations[i] = -1;
			for (int i=0; i<eCustomFragmentLocationCount; ++i)
				fragmentLocations[i] = -1;

			// default sampler slots
			samplers.insert( std::make_pair( eCustomLocationMainDepthSampler, 18 ) );
			samplers.insert( std::make_pair( eCustomLocationReflCubeSampler, 17 ) );
			samplers.insert( std::make_pair( eCustomLocationRimSampler, 16 ) );
			samplers.insert( std::make_pair( eCustomLocationBackgroundSampler, 7 ) );

			samplers.insert( std::make_pair( eCustomLocationMatCapSampler, 6 ) );
			samplers.insert( std::make_pair( eCustomLocationDiffuseLightingSampler, 10 ) );
			samplers.insert( std::make_pair( eCustomLocationSpecularLightingSampler, 11 ) );
			samplers.insert( std::make_pair( eCustomLocationBrdfSampler, 12 ) );

			samplers.insert( std::make_pair( eCustomLocationMaskA, 8 ) );
			samplers.insert( std::make_pair( eCustomLocationMaskB, 9 ) );
			samplers.insert( std::make_pair( eCustomLocationShadows, 5 ) );

			samplers.insert( std::make_pair( eCustomLocationAmbient, 6 ) );
			samplers.insert( std::make_pair( eCustomLocationDiffuse, 0 ) );
			samplers.insert( std::make_pair( eCustomLocationTransparency, 1 ) );
			samplers.insert( std::make_pair( eCustomLocationSpecularity, 2 ) );
			samplers.insert( std::make_pair( eCustomLocationReflectivity, 3 ) );
			samplers.insert( std::make_pair( eCustomLocationNormalMap, 4 ) );
			//samplers.insert( std::make_pair( eCustomLocationCubeMap, 3 ) );

			samplers.insert( std::make_pair( eCustomLocationEyeEnvDiffuseSampler, 13 ) );
			samplers.insert( std::make_pair( eCustomLocationEyeEnvReflectionSampler, 14 ) );
			samplers.insert( std::make_pair( eCustomLocationEyeEnvRefractionSampler, 15 ) );
		}

		void SetShadersId(const int _vertex, const int _fragment)
		{
			vertex = _vertex;
			fragment = _fragment;
		}

		const GLint	GetVertexId() const {
			return vertex;
		}
		const GLint GetFragmentId() const {
			return fragment;
		}

		const GLint GetVertexLocation(const CustomVertexShaderLocation location) const {
			return vertexLocations[location];
		}
		const GLint GetFragmentLocation(const CustomFragmentShaderLocation location) const {
			return fragmentLocations[location];
		}

		// return number of assigned locations
		int PrepVertexLocations();
		int PrepFragmentLocations();
		int PrepDefaultSamplerSlots()
		{
			int count = 0;
			if (fragment < 0)
				return count;

			for (auto iter=begin(samplers); iter!=end(samplers); ++iter)
			{
				GLint loc = fragmentLocations[iter->first];
				if ( loc >= 0 )
				{
					glProgramUniform1i( fragment, loc, iter->second );
					count += 1;
				}
			}

			GLint loc = fragmentLocations[eCustomLocationProjectors];
			if (loc >= 0)
			{
				GLint samplersArray[6] = {10, 11, 12, 13, 14, 15};
				glProgramUniform1iv( fragment, loc, 6, samplersArray );
				count += 1;
			}

			return count;
		}

		bool SetUniform1i(const CustomFragmentShaderLocation location, const GLint value) const {
			if ( fragment >= 0 && fragmentLocations[location] >= 0)
				glProgramUniform1i( fragment, fragmentLocations[location], value );
			return true;
		}
		bool SetUniform1f(const CustomFragmentShaderLocation location, const float value) const {
			if ( fragment >= 0 && fragmentLocations[location] >= 0)
				glProgramUniform1f( fragment, fragmentLocations[location], value );
			return true;
		}
		bool VertexUniform1f(const CustomVertexShaderLocation location, const float value) const {
			if ( vertex >= 0 && vertexLocations[location] >= 0)
				glProgramUniform1f( vertex, vertexLocations[location], value );
			return true;
		}
		const GLint GetSamplerSlot(const CustomFragmentShaderLocation location) const
		{
			auto iter = samplers.find(location);
			if (iter != end(samplers) )
				return iter->second;
			return -1;
		}

	protected:
		//
		int					vertex;
		int					fragment;

		// custom shader location (extracted manualy)
		GLint		vertexLocations[eCustomVertexLocationCount];
		GLint		fragmentLocations[eCustomFragmentLocationCount];

		// store default samplers values for fragment locations
		std::map<CustomFragmentShaderLocation, GLint>	samplers;

		const char *GetVertexLocationName(const CustomVertexShaderLocation location);
		const char *GetFragmentLocationName(const CustomFragmentShaderLocation location);

	};

	/////////////////////////////////////////////////////////////////////////////////////////////
	//
	class ShaderEffect : public ShaderBase
	{
	protected:

		// data structure reflecting buffers in shaders
		struct transfBlock1
		{
			mat4	m4_ViewProj;
			mat4	m4_ViewProjI;
			mat4	m4_Proj;
			mat4	m4_View;
			mat4	m4_ViewIT;
			vec3	eyePos;
			float	farPlane;
		};
		transfBlock1 g_transfBlock1;
		struct transfBlock2
		{
			mat4	m4_World;
			mat4	m4_WorldView;
			mat4	m4_WorldViewProj;
			mat4	m4_WorldIT;
		};
		transfBlock2 g_transfBlock2;
		vec4 scaleBias;	

		struct frustumBlock
		{
			vec4	planes[6];
		};
		frustumBlock	g_frustumBlock;

		struct cubeMapBlock
		{
			glm::mat4	cubeMapProjections[6];
			glm::mat4	cubeMapWorldToLocal;
			glm::vec4	cubeMapPosition;
			glm::vec4	cubeMapMax;
			glm::vec4	cubeMapMin;
			float		cubeMapParallaxCorrection;
			vec3		cubeMapTemp;
		};
		cubeMapBlock	g_cubeMapBlock;

		struct lightBlock
		{
			vec4	globalAmbientLight;
			int		numDirLights;
			int		numPointLights;
			float	dummy1;
			float	dummy2;
		};
		lightBlock	g_lightBlock;

		GLuint		mBufferTransfBlock[2];
		GLuint		mBufferFrustumBlock;
		GLuint		mBufferCubeMapBlock;
		GLuint		mBufferLightBlock;

		void	ConstructorUniformBuffers()
		{
			mBufferTransfBlock[0] = mBufferTransfBlock[1] = 0;
			mBufferFrustumBlock = 0;
			mBufferCubeMapBlock = 0;
			mBufferLightBlock = 0;
		}
		void	CreateUniformBuffers()
		{
			FreeUniformBuffers();

			glGenBuffers(2, &mBufferTransfBlock[0]);
			glBindBuffer(GL_UNIFORM_BUFFER, mBufferTransfBlock[0]);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(transfBlock1), NULL, GL_STREAM_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, mBufferTransfBlock[1]);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(transfBlock2), NULL, GL_STREAM_DRAW);

			glGenBuffers(1, &mBufferFrustumBlock);
			glBindBuffer(GL_UNIFORM_BUFFER, mBufferFrustumBlock);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(frustumBlock), NULL, GL_STREAM_DRAW);

			glGenBuffers(1, &mBufferCubeMapBlock);
			glBindBuffer(GL_UNIFORM_BUFFER, mBufferCubeMapBlock);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(cubeMapBlock), NULL, GL_STREAM_DRAW);

			glGenBuffers(1, &mBufferLightBlock);
			glBindBuffer(GL_UNIFORM_BUFFER, mBufferLightBlock);
			glBufferData(GL_UNIFORM_BUFFER, sizeof(lightBlock), NULL, GL_STREAM_DRAW);
		}
		void	FreeUniformBuffers()
		{
			if (mBufferTransfBlock[0] > 0)
				glDeleteBuffers(2, &mBufferTransfBlock[0] );
			if (mBufferFrustumBlock > 0)
				glDeleteBuffers(1, &mBufferFrustumBlock);
			if (mBufferCubeMapBlock > 0)
				glDeleteBuffers(1, &mBufferCubeMapBlock);
			if (mBufferLightBlock > 0)
				glDeleteBuffers(1, &mBufferLightBlock);

			ConstructorUniformBuffers();
		}

	

	protected:


		enum ETechDepthOverridesPasses
		{
			eTechDepthOverride_Linear,
			eTechDepthOverride_Log,
			eTechDepthOverride_Count
		};

		// customize tech wall material
		//	change depth algorithm ( linear or logarithmic )

		EEffectTechnique	mCurrentTech;

		//
		nvFX::ITechnique	*fx_TechWallMaterial;
		nvFX::ITechnique	*fx_TechWallMaterialLog;
		//nvFX::ITechnique	*fx_TechWallCubeMap;
		nvFX::ITechnique	*fx_TechDepthOverrides;

		nvFX::ITechnique	*fx_TechCharacter;

		nvFX::ITechnique	*fx_TechShadow;
		nvFX::ITechnique	*fx_TechCulling;
		nvFX::ITechnique	*fx_TechNormals;
		nvFX::ITechnique	*fx_TechNormalsLog;		
		nvFX::ITechnique	*fx_TechSimple;

		//
		nvFX::ICstBuffer	*fx_transfBlock1;
		nvFX::ICstBuffer	*fx_transfBlock2;
		nvFX::ICstBuffer	*fx_projectorsBlock;
		
		nvFX::ICstBuffer	*fx_frustumBlock;
		nvFX::ICstBuffer	*fx_lightBlock;

		nvFX::ICstBuffer	*fx_cubeMapBlock;

		//
		nvFX::IUniform		*fx_Fcoef;
		nvFX::IUniform		*fx_depthDisplacement;
		nvFX::IUniform		*fx_numberOfShadows;
		nvFX::IUniform		*fx_normalsLength;
		nvFX::IUniform		*fx_AlphaPass;

		nvFX::IUniform		*fx_meshIndex;

		nvFX::IUniform		*fx_textureOffset;
		nvFX::IUniform		*fx_textureScaling;

		nvFX::IUniform		*fx_softParticles;

		//nvFX::IUniform		*fx_rimOptions;
		//nvFX::IUniform		*fx_rimColor;

		//nvFX::IUniform		*fx_shaderMask;

		nvFX::IUniform		*fx_numberOfProjectors;

		//

		virtual bool	InitializeEffectParams();
		virtual bool	PrepCommonLocations();

		bool validateAndCreateSceneInstances();

	private:

		CustomEffectShaderLocations		mWallLocations[eTechWallPass_Count];
		CustomEffectShaderLocations		mWallLogLocations[eTechWallPass_Count];
		//CustomEffectShaderLocations		mWallCubeMapLoc[eTechWallPass_Count];
		CustomEffectShaderLocations		mCharacterLocations[eTechCharacterPass_Count];
		/*
		CustomEffectShaderLocations		mPhongNoTexturesLoc;
		CustomEffectShaderLocations		mPhongLoc;
		CustomEffectShaderLocations		mPhongLogLoc;
		CustomEffectShaderLocations		mPhongBindlessLoc;
		CustomEffectShaderLocations		mPhongBindlessLogLoc;
		CustomEffectShaderLocations		mPhongCubeMapLoc;
		CustomEffectShaderLocations		mPhongBindlessCubeMapLoc;
		*/
		CustomEffectShaderLocations		mShadowLoc;

		CustomEffectShaderLocations		*mCurrentLoc;

		//

		float				mAlpha;
		EEffectDepthHint	mLogarithmicDepthHint;
		
		bool				mLogDepth;
		bool				mOldLogDepth;

		bool				mEarlyZ;
		bool				mNoTextures;
		bool				mBindless;
		bool				mCubeMapRendering;
		bool				mEyePass;
		
		bool	PrepLogDepth();

	public:

		//! a constructor
		ShaderEffect();

		// a destructor
		virtual ~ShaderEffect();

		// on destructor
		virtual void	Free();

		virtual void		Bind();
		
		void	UploadFrustumPlanes(CFrustum &frustum);
		// pass nullptr for realFarPlane to use camera farPlane value
		void	UploadCubeMapUniforms(const float zmin, const float zmax, const glm::mat4 &worldToLocal, const glm::vec3 &position, const glm::vec3 &max, const glm::vec3 &min, const float useParallax);
		/*
		void	UploadCameraUniforms(const mat4 &modelview, const mat4 &projection, const mat4 &viewIT, 
			const vec3 &cameraPos, const int width, const int height, const float farPlane, const float *realFarPlane);
		*/
		void	UploadCameraUniforms(const CCameraInfoCache &cacheInfo);
		void	UploadCameraUniforms(const float *realFarPlane);

		void	UploadModelTransform(const mat4 &m);
		void	UploadModelTransform(const double *m);
		void	UploadLightTransform(const mat4 &proj, const mat4 &view, const mat4 &m);
		void	UploadLightingInformation(const bool mapOnGPU, const vec4 &ambientLightColor, const int numDirLights, const int numPointLights);
		void	UploadLightingInformation(const int numDirLights, const int numPointLights);
		//void	SetDepthDisplacement( const float value );
		//void	UpdateDepthDisplacement( const float value );
		
		// per model uniforms !
		void	SetMeshIndex( const int index );
		void	UpdateMeshIndex( const int index );

		void	SetTextureOffset( const vec4 &v );
		void	UpdateTextureOffset( const vec4 &v );
		void	SetTextureScaling( const vec4 &v );
		void	UpdateTextureScaling( const vec4 &v );

		void	SetSoftParticles( const float value );
		void	UpdateSoftParticles( const float value );

		//void	SetRimParameters( const double useRim, const double rimPower, const bool useRimTexture, const vec4 &rimColor );
		//void	UpdateRimParameters( const double useRim, const double rimPower, const bool useRimTexture, const vec4 &rimColor );

		//void	SetShaderMask( const vec4 &mask );
		//void	UpdateShaderMask( const vec4 &mask );

		//
		void	SetTechnique( const EEffectTechnique technique );

		void	PrepCurrentTech();

		void	PrepWall_PassAndLocations();
		void	PrepCharacter_PassAndLocations();

		void	SetNormalsParameters( const float length );
	
		void	SetNumberOfProjectors( const int numberOfProjectors );
		void	UpdateNumberOfProjectors( const int numberOfProjectors );
	
		void	SetNumberOfShadows( const int numberOfShadows );
		void	SetShadowsMVP( float *mvp );

		//!! Technique should be set up before
		void	SetLogarithmicDepthHint(const EEffectDepthHint value);
		const EEffectDepthHint GetLogarithmicDepthHint() const;

		void	SetLogarithmicDepth(const bool value);
		const bool	IsLogarithmicDepth() const { return mLogDepth; }

		//
		void SetEarlyZ( const bool value );
		const bool IsEarlyZ() { return mEarlyZ; }

		void SetEyePass( const bool value );
		const bool IsEyePass() const {return mEyePass; }

		const bool IsWallTechnique() { 
			return (mCurrentTech != eEffectTechniqueIBL);
		}

		void	NoTextures( const bool value );
		const bool IsNoTextures() { return mNoTextures; }

		void	SetBindless( const bool value );
		const bool  IsBindless() { return mBindless; }

		void	SetCubeMapRendering( const bool value );
		const bool	IsCubeMapRendering() { return mCubeMapRendering; }

		void	UpdateAlphaPass(const float value);

		// Upload ModelView Matrix Array for Draw Instanced.
        void UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount);
	
		// return set of locations according to current bindless and logarithmic state
		const CustomEffectShaderLocations *GetCustomEffectShaderLocationsPtr() const
		{
			return mCurrentLoc;
		}
		const CustomEffectShaderLocations *GetIBLPassLocationsPtr(const ETechCharacterPasses passId) const
		{
			return &mCharacterLocations[passId];
		}

		virtual const GLuint		GetVertexProgramId() const;
		virtual const GLuint		GetFragmentProgramId() const;
		virtual const GLuint		FindFragmentProgramLocation(const char *name);

		nvFX::IUniform	*FindUniform(const char *name);

	};

}
