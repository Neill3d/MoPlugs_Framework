#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_rendering.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "glm\glm.hpp"

struct CubeMapRenderingData
{
	int			cubeMapSize;

	int			outputWidth;
	int			outputHeight;

	int			maxCubeMapSize;
	int			maxOutputSize;

	// main texture objects
	bool			useStaticCubeMap;
	int				staticCubeMapSize;
	unsigned		cubeMapId;
	unsigned		depthForCubeMapId; // 2d texture for each cubemap face rendering

	unsigned		outputId;
	unsigned		depthId;	// needed for rendering into the camera
	bool			needUpdateOutput;

	// option for exporting dynamic generated cubemap
	// filename we can take from pComponent->FileName
	bool				saveOutputMap;
	bool				saveDynamicCubeMap;

	// add-on flags
	bool			renderGeomCache;

	float			zmin;
	float			zmax;

	float			useParallax;

	// pointer to the scene reflection object (FBComponent)
	void		*pComponent;		

	glm::mat4			worldToLocal;
	glm::vec3			position;
	glm::vec3			max;
	glm::vec3			min;

	CubeMapRenderingData()
	{
		useStaticCubeMap = false;
		staticCubeMapSize = 1;
		cubeMapSize = 1;
		cubeMapId = 0;
		depthForCubeMapId = 0;

		outputId = 0;
		depthId = 0;

		maxCubeMapSize = 2048;
		maxOutputSize = 8192;
	}
};

struct RenderingStats
{
	int			totalDrawNodeCount;
	int			totalGeometryCacheCount;
	
	int			opaqueModels;
	int			transparentModels;

	int			totalShadersCount;
	int			opaqueShaders;
	int			transparentShaders;

	double		totalUpdateDuration;
	double		updateLocalTime;			// FBTime

	RenderingStats()
	{
		Reset();
	}

	void Reset()
	{
		totalDrawNodeCount = 0;
		totalGeometryCacheCount = 0;
		
		opaqueModels = 0;
		transparentModels = 0;

		totalShadersCount = 0;
		opaqueShaders = 0;
		transparentShaders = 0;

		totalUpdateDuration = 0.0;
		updateLocalTime = 0.0;
	}

	void CountOneShader() {
		totalShadersCount += 1;
	}
	void CountOneOpaqueShader() {
		opaqueShaders += 1;
	}
	void CountOpaqueShaders(const int count) {
		opaqueShaders += count;
	}
	void CountOneTransparencyShader() {
		transparentShaders += 1;
	}
	void CountTransparencyShaders(const int count) {
		transparentShaders += count;
	}

	void CountOneObject() {
		totalDrawNodeCount += 1;
	}
	void CountOpaqueObjects(const int count) {
		opaqueModels += count;
	}
	void CountOneOpaqueObject() {
		opaqueModels += 1;
	}
	void CountOneTransparentModel() {
		transparentModels += 1;
	}
	void CountTransparentModels(const int count) {
		transparentModels += count;
	}
	void CountOneGeometryCache() {
		totalGeometryCacheCount += 1;
	}
};