
#pragma once

namespace Graphics
{

	// flags that helps to choose material technique and pass for drawing
	enum
	{
		eShaderFlag_LogDepth	= 1,
		eShaderFlag_EarlyZ		= 2,
		eShaderFlag_NoTextures	= 4,
		eShaderFlag_Bindless	= 8,
		eShaderFlag_CubeMapRendering	= 16,
		eShaderFlag_EyePass		= 32
	};

	// depricated!
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


	enum ECompositeTechnique
	{
		eCompositeTechniqueProjections,
		eCompositeTechniqueShadowsDisplay,
		eCompositeTechniqueCount
	};
};