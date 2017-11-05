
#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// wrapper around nvFX effects with implementing 2d composition technique 
//	and 3d gpu geometry cache, super lighting shader shading
//
//	Author Sergey Solokhin (Neill3d) 2014-2017
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

//-------------- Effect system
#include "FxParser.h"

//-------------- MCL
#include "Types.h"
#include "shared_models.h"
#include "shared_camera.h"

#include "glm\glm.hpp"

#include "ShaderFX_enums.h"

#define kMaxDrawInstancedSize  100

namespace Graphics
{

	

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Base nvFX Shader class

	class BaseShaderFX
	{
	public:
		//! a constructor
		BaseShaderFX();
		//! a destructor
		virtual ~BaseShaderFX();

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

		BaseShaderFX(const BaseShaderFX&);
        BaseShaderFX& operator = (const BaseShaderFX&);
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	//

	class ShaderComposite : public BaseShaderFX
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

	
	////////////////////////////////////////////////////////////////////////////////////////////
	// query a vertex or fragment locations (sampler, uniform, buffer)
	struct CustomShaderLocations
	{
	public:

		//! a constructor
		CustomShaderLocations();

		// return a new location enum id
		GLint RegisterNewLocation(const char *locationName);

		// method stores default sampler value for a specified location
		// enumId - registred location enum id
		
		GLint RegisterDefaultSampler( const GLint enumId, const GLint value );

		void SetShaderId( const GLint shaderId );
		const GLint GetShaderId() const;

		// return a number of prepared/finded (glGetUniformLocation) locations
		int Prep();

		virtual int PrepDefaultSamplerSlots();

		bool SetUniform1i(const CustomFragmentShaderLocation location, const GLint value) const;
		bool SetUniform1f(const CustomFragmentShaderLocation location, const float value) const;
		
		const GLint GetSamplerSlot(const CustomFragmentShaderLocation location) const;

	protected:
		//
		GLint						mShader;
		
		std::vector<GLint>			mLocations; // for a new location, it's id in shader
		std::vector<std::string>	mLocationNames;

		// store default samplers values for fragment locations
		std::map<GLint, GLint>		mSamplers;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////
	// BaseMaterialShaderFX

	class BaseMaterialShaderFX : public BaseShaderFX
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

		void	ConstructorUniformBuffers();
		void	CreateUniformBuffers();
		void	FreeUniformBuffers();

	protected:

		enum ETechDepthOverridesPasses
		{
			eTechDepthOverride_Linear,
			eTechDepthOverride_Log,
			eTechDepthOverride_Count
		};

		//	change depth algorithm ( linear or logarithmic )

		EEffectTechnique	mCurrentTech;

		//
		nvFX::ITechnique	*fx_TechMaterialLinear;	// with a linear depth
		nvFX::ITechnique	*fx_TechMaterialLog;	// with a log depth

		//
		nvFX::ICstBuffer	*fx_transfBlock1;
		nvFX::ICstBuffer	*fx_transfBlock2;
		
		
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

		//

		virtual bool	InitializeEffectParams();
		virtual bool	PrepCommonLocations();

		bool validateAndCreateSceneInstances();

		virtual bool	OnFindTechnique();

	protected:
		//

		float				mAlpha;
		unsigned int		mShaderFlags;

		struct EffectLocations
		{
			CustomShaderLocations	*vertex;
			CustomShaderLocations	*fragment;
		};

		EffectLocations			mCurrentLoc;

		bool		PrepLogDepth();

	public:

		//! a constructor
		BaseMaterialShaderFX();

		// a destructor
		virtual ~BaseMaterialShaderFX();

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
		void	SetNumberOfShadows( const int numberOfShadows );
		void	SetShadowsMVP( float *mvp );

		void	UpdateAlphaPass(const float value);

		//
		// Shader Flags - helps to select technique pass and settings

		void	DefaultShaderFlags();
		void	SetShaderFlags( unsigned int flags );
		void	ModifyShaderFlags( unsigned int flags, bool activate );
		
		void	SetTechnique( const EEffectTechnique technique );
		void	PrepCurrentTech();


		// Upload ModelView Matrix Array for Draw Instanced.
        void UploadModelViewMatrixArrayForDrawInstanced(const double* pModelViewMatrixArray, int pCount);
	
		// return set of locations according to current bindless and logarithmic state
		const EffectLocations GetCurrentEffectLocationsPtr() const
		{
			return mCurrentLoc;
		}
		

		virtual const GLuint		GetVertexProgramId() const;
		virtual const GLuint		GetFragmentProgramId() const;
		virtual const GLuint		FindFragmentProgramLocation(const char *name);

		nvFX::IUniform	*FindUniform(const char *name);
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// phong, toon, color correction, up to 6 projected textures

	struct ProjectorsVertexLocations : public CustomShaderLocations
	{
	public:
		// a constructor
		ProjectorsVertexLocations();
	};

	struct ProjectorsFragmentLocations : public CustomShaderLocations
	{
	public:
		// a constructor
		ProjectorsFragmentLocations();

		virtual int PrepDefaultSamplerSlots() override;
	};

	class ProjectorsShaderFX : public BaseMaterialShaderFX
	{
	protected:

		CustomEffectShaderLocations		mWallLocations[eTechWallPass_Count];
		CustomEffectShaderLocations		mWallLogLocations[eTechWallPass_Count];

		// TODO: depricated ?!
		nvFX::ITechnique	*fx_TechShadow;
		nvFX::ITechnique	*fx_TechCulling;
		nvFX::ITechnique	*fx_TechNormals;
		nvFX::ITechnique	*fx_TechNormalsLog;		
		nvFX::ITechnique	*fx_TechSimple;

		//
		nvFX::ICstBuffer	*fx_projectorsBlock;

		//
		nvFX::IUniform		*fx_numberOfProjectors;

		virtual bool	OnFindTechnique() override;

	protected:

		// depricated
		CustomEffectShaderLocations		mShadowLoc;


	public:

		void	PrepWall_PassAndLocations();
		

		void	SetNumberOfProjectors( const int numberOfProjectors );
		void	UpdateNumberOfProjectors( const int numberOfProjectors );
	

		void	SetBindless( const bool value );
		const bool  IsBindless() { return mBindless; }
	};

	////////////////////////////////////////////////////////////////////////////////////////////////
	// IBL PBS, eye shader, eye lashes, skin shader

	class IBLShaderFX : public BaseMaterialShaderFX
	{
	protected:

		CustomEffectShaderLocations		mCharacterLocations[eTechCharacterPass_Count];

		virtual bool	OnFindTechnique() override;

	public:

		void	PrepCharacter_PassAndLocations();

		void SetEyePass( const bool value );
		const bool IsEyePass() const {return mEyePass; }


		const CustomEffectShaderLocations *GetIBLPassLocationsPtr(const ETechCharacterPasses passId) const
		{
			return &mCharacterLocations[passId];
		}
	};
}
