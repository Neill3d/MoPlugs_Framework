
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_glsl.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_glsl.h"

/////////////////////////////////////////////////////////////////////////////////////////////

void	MergeShader( const ShaderGLSL &inShader, ShaderGLSL &shader )
{
	switch(inShader.shaderType)
	{
	case eShaderTypeDefault:
	case eShaderTypeSuperLighting:
		shader = inShader;
		break;
	case eShaderTypeColorCorrection:
		shader.brightness = inShader.brightness;
		shader.contrast = inShader.contrast;
		shader.customColor = inShader.customColor;
		shader.gamma = inShader.gamma;
		shader.saturation = inShader.saturation;
		break;
	case eShaderTypeShading:
		shader.shadingType = inShader.shadingType;
		shader.toonDistribution = inShader.toonDistribution;
		shader.toonEnabled = inShader.toonEnabled;
		shader.toonShadowPosition = inShader.toonShadowPosition;
		shader.toonSteps = inShader.toonSteps;
		break;
	case eShaderTypeProjections:
		break;
	}
}

void DefaultMaterial( MaterialGLSL &mat )
{
	memset( &mat, 0, sizeof(MaterialGLSL) );
}

void DefaultShader( ShaderGLSL &shader )
{
	shader.shadingType = eShadingTypeDynamic;
	shader.shaderType = eShaderTypeDefault;

	shader.shader1 = -1;
	shader.shader2 = -1;
	shader.shader3 = -1;
	shader.shader4 = -1;
	shader.shader5 = -1;
	
	shader.transparency = 1.0f;
	shader.alphaFromColor = 0.0f;

	shader.toonEnabled = 0.0f;
	shader.toonDistribution = 1.0f;
	shader.toonShadowPosition = 0.5f;
	shader.toonSteps = 4.0f;

	shader.customColor = vec4(1.0, 1.0, 1.0, 3.0);

	shader.contrast = 1.0f;
	shader.saturation = 1.0f;
	shader.brightness = 1.0f;
	shader.gamma = 1.0f;

	shader.depthDisplacement = 0.0f;
	shader.applyColorCorrection = 0.0f;

	shader.rimColor = vec4(1.0, 1.0, 1.0, 1.0);
	shader.rimOptions = vec4(0.0, 0.0, 0.0, 0.0);
	shader.mask = vec4(0.0, 0.0, 0.0, 0.0);
}