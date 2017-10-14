
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_camera.h
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

#include "algorithm\nv_math.h"

struct CCameraInfoCache
{
	//FBCamera *pCamera;
	void				*pUserData;

	//
	int					offsetX;
	int					offsetY;
	int					width;
	int					height;

	//
	double				fov;

	double				farPlane;
	double				nearPlane;
	double				realFarPlane;

	vec4				pos;	// camera eye pos

	mat4				mv4;
	mat4				mvInv4; // mv inverse
	mat4				p4;	// projection matrix
	mat4				proj2d;

	double				mv[16];

	/*
	// pre-loaded data from camera
	FBMatrix			mv;
	FBMatrix			mvInv;
	FBMatrix			p;

	static void Prep(FBCamera *pCamera, CCameraInfoCache &cache);
	*/
};