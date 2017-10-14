
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_glsl.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//-- 
#include <GL\glew.h>

//
#include "algorithm\nv_math.h"

// !!! TODO: add information about shader alpha type values

enum EShadingType {
		eShadingTypeDynamic,	// Phong shading
		eShadingTypeFlat,		
		eShadingTypeToon,		// Toon phong shading
		eShadingTypeMatte,
		eShadingTypeMatCap,
		eShadingTypeCount
		//eShadingTypeCubeMap
	};

enum	EShaderType
{
	eShaderTypeDefault,
	eShaderTypeSuperLighting,
	eShaderTypeColorCorrection,
	eShaderTypeShading,
	eShaderTypeProjections,
	eShaderTypeCount
};

//////////////////////////////////////////////////////////////////

// wrapper around textures
struct TextureGLSL
{
	mat4			transform;
	GLuint64		address;	// source sampler data (if we use bindless textures)
	float			width;
	float			height;
	//vec4			format;
};

struct MaterialGLSL
{
	// pointers to textures
	int			ambient;
	int			diffuse;
	int			specular;
	int			emissive;
	int			transparency;
	int			normalmap;
	int			reflect;

	float		specexp;

	float		useAmbient;
	float		useDiffuse;
	float		useSpecular;
	float		useEmissive;
	float		useTransparency;
	float		useNormalmap;
	float		useReflect;
		
	// for IBL 
	float		roughness;
	float		metal;
	float		dummy3;
	float		dummy1;
	float		dummy2;

	// material common colors
	vec4		emissiveColor;
    vec4     	diffuseColor;
    vec4     	ambientColor;
	vec4		reflectColor;
    vec4     	transparencyColor;
    vec4     	specularColor;
};


//
struct ShaderGLSL
{
	int			shadingType;

	// hold combination here
	int			shaderType;		// shader type for making combinations (override part of lower level shader)
	int			shader1;
	int			shader2;

	int			shader3;
	int			shader4;
	int			shader5;
	float		transparency;

	float		toonEnabled;
	float		toonSteps;
	float		toonDistribution;
	float		toonShadowPosition;
	
	vec4		customColor;	
	
	float		contrast;
	float		saturation;
	float		brightness;
	float		gamma;

	float		depthDisplacement;
	float		alphaFromColor;
	float		dummy2;
	float		applyColorCorrection;

	vec4		mask;
	vec4		rimOptions;
	vec4		rimColor;

	// TODO: move them to lights
	vec4		shadow;		// rgb - color, w-intensity	
};


struct ModelGLSL
{
	mat4		transform;		// local mesh matrix
	mat4		normalMatrix;	// transpose(inverse(WorldViewIT))
	// total 128 bytes
};


struct MeshGLSL
{
	int				material;		// index to the mesh material

	// temproary we could use to store shadow receive flag
	int				lightmap;	// index to the lightmap texture (if needed)

	int				model;
	int				shader;

	// 16
		
	vec4		color;			// flat color id (buffer id rendering)
		
	// total - 32 bytes
};


struct ProjectorGLSL
{
	mat4		matrix;				// texture projection matrix

	float		textureLayer;		// index in texture2darray
	float		maskLayer;			// index in texture2darray
	int			maskChannel;		// x,y,z,w channel of a mask layer
	float		blendOpacity;		// projected texture opacity
	float		blendMode;			// photoshop blend mode

	vec3		dummy;
};

/////////////////////////////////////////////////
//

void	DefaultMaterial( MaterialGLSL &mat );
void	DefaultShader( ShaderGLSL &shader );

void	MergeShader( const ShaderGLSL &inShader, ShaderGLSL &shader );