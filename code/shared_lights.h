#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_loader.h
//
// light manager and clustered assignment technique
//  with bindless unifroms and SSBO we actually don't have any limits in number of lights !
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

// STL
#include <vector>
#include <map>

//
#include "algorithm\nv_math.h"

#include "graphics\GlBufferObject.h"
#include "graphics\OGL_Utils.h"
#include "graphics\UniformBuffer.h"

#include "shared_camera.h"

#include "Types.h"

////////////////////////////////////////////////////////////

#define	MAX_NUMBER_OF_LIGHTS			16
#define MAX_NUMBER_OF_CASCADED_SPLITS	4

#define	LIGHT_TYPE_DIRECTION			0.0f
#define	LIGHT_TYPE_POINT				1.0f
#define LIGHT_TYPE_SPOT					2.0f
#define LIGHT_TYPE_AREA					3.0f	// TODO: add support for that

// bind indices for glsl shader
#define	BIND_CLUSTERS_INDEX_DATA		1
#define BIND_CLUSTERS_GRID_DATA			2

struct globalSettings
{
	int		width;		// window width
	int		height;		// window height
	float	fov;
	float	farPlane;
	float	nearPlane;
};

struct ScreenRect
{
	uint2		min;
	uint2		max;
};

struct ScreenRect3D
{
	uint3		min;
	uint3		max;
	uint32_t	index;
};

struct CLightFrustum
{
	float neard;
	float fard;
	float fov;
	float ratio;
	vec3 point[8];
};

////////////////////////////////////////////////////////////


// light data should be computer in eye space, means for specified camera
//
struct LightDATA
{
	vec3 		position; 
	float		type;
	
    vec3 		dir; 
	float		spotAngle;
	
    vec3 		color;
	float		radius;
	
    vec4 		attenuations;
	
	float		shadowMapLayer;
	float		shadowMapSize;
	float		shadowPCFKernelSize;
	float		castSpecularOnObject;
	
	//mat4		shadowVP;	// view projection matrix of a shadow map
	vec4		shadowIndex;	// index and count in the shadow matrix array
	vec4		normalizedFarPlanes;	// for cascaded shadows
};


typedef std::vector<LightDATA>			lights_vector;

struct Cluster3d
{
	vec3		min;
	vec3		max;
};

struct ClusterLights
{
	int			count;
	int			offset;
};

/////////////////////////////////////

struct ShadowSettings
{
	int		shadowMapSize; 
	int		shadowPCFKernelSize;
	float	offsetScale;
	float	offsetBias;

	vec3	shadowColor;
	float	shadowIntensity;

	//! a constructor
	ShadowSettings()
	{
		shadowMapSize = 2048;
		shadowPCFKernelSize = 9;
		offsetScale = 4.0f;
		offsetBias = 400.0f;
		shadowColor = vec3(0.0f, 0.0f, 0.0f);
		shadowIntensity = 0.5f;
	}

	ShadowSettings(const int pShadowMapSize, const int pShadowPCFKernelSize, const float pOffsetScale, const float pOffsetBias, const vec3 pShadowColor, const float pShadowIntensity)
		: shadowMapSize( pShadowMapSize )
		, shadowPCFKernelSize( pShadowPCFKernelSize )
		, offsetScale ( pOffsetScale )
		, offsetBias ( pOffsetBias )
		, shadowColor ( pShadowColor )
		, shadowIntensity ( pShadowIntensity )
	{}
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// this is a light version of lights manager !

class CGPUShaderLights
{

public:
	//! a constructor
	CGPUShaderLights();
	//! a destructor
	~CGPUShaderLights();

	//void Build (CCameraInfoCache &cameraCache, std::vector<LightDATA> &lights);

	void MapOnGPU();
	void PrepGPUPtr();

	void Bind( const GLuint programId, const GLuint dirLightsLoc, const GLuint lightsLoc ) const;
	void UnBind() const;

	const int GetNumberOfDirLights() const
	{
		return (int) mDirLights.size();
	}
	const int GetNumberOfLights() const
	{
		return (int) mLights.size();
	}

	lights_vector		&GetLightsVector()
	{
		return mLights;
	}
	lights_vector		&GetDirLightsVector()
	{
		return mDirLights;
	}

	lights_vector		&GetTransformedLightsVector()
	{
		return mTransformedLights;
	}
	lights_vector		&GetTransformedDirLightsVector()
	{
		return mTransformedDirLights;
	}

	// prepare lights in a view space
	void UpdateTransformedLights(const mat4 &modelview, const mat4 &rotation);

protected:
	//
	// lights scene data

	lights_vector					mDirLights;	// dir lights stored in viewSpace !!
	lights_vector					mLights;	// point/spot lights stored in viewSpace !!

	lights_vector					mTransformedDirLights;
	lights_vector					mTransformedLights;

	/// vars for parallel evaluation
	//int								mNumberOfDirLights;
	//int								mNumberOfLights;

	//
	// uniform buffer object (UBO) for lights data
	
	CGPUBufferNV			mBufferLights;
	CGPUBufferNV			mBufferDirLights;

};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// collect lights from FBScene or locally from gpu cache
//
class CGPULightsManager
{
public:
	//! a constructor
	CGPULightsManager();

	//! a destructor
	~CGPULightsManager();

	//
	// process lights - One time per frame for each camera
	void Initialize();
	
	void	Free();
	void	Clear();

	void Bind(const GLuint programId, const GLuint clusterGridLoc, const GLuint clusterIndexLoc, const GLuint dirLightsLoc, const GLuint lightsLoc, const GLuint lightMatricesLoc);
	void UnBind();

	void Prep(const CCameraInfoCache &cameraCache, const float *realFarPlane);
	void MapOnGPU();
	void PrepGPUPtr();

	const int	GetNumberOfDirLights() const;
	const int	GetNumberOfPointLights() const;

	void DrawCameraFrustum(const float nearPlane, const float farPlane, const mat4 &projection, const mat4 &modelview);

	void SetDebugDisplay(const bool value);

	const size_t GetNumberOfShadowCasters() const;

	void BindLightMatrices(const GLuint attribIndex) const;

	mat4	&GetLightViewMatrix(const int index);
	mat4	&GetLightProjMatrix(const int index);
	mat4	&GetLightInvTM(const int index);

public:

	bool PrepShadowMaps( CCameraInfoCache *pCache, 
		const vec3 &worldMin, 
		const vec3 &worldMax, 
		const int shadowMapSize,
		const LightDATA &lightData, 
		const int cascadedSplits, 
		const float cascadedCorrection,
		const float cascadedNearPlane,
		const float cascadedFarPlane
		);
	
	void PostRenderingOverviewCam(const int width, const int height);

public:

	//
	// lights scene data

	lights_vector					mDirLights;	// dir lights stored in viewSpace !!
	lights_vector					mLights;	// point/spot lights stored in viewSpace !!

	/// vars for parallel evaluation
	int								mNumberOfDirLights;
	int								mNumberOfLights;

	//
	// uniform buffer object (UBO) for lights data
	
	CGPUBufferDoubleNV			mBufferLights;
	CGPUBufferDoubleNV			mBufferDirLights;

	CGPUBufferDoubleNV			mBufferClusterIndex;
	CGPUBufferDoubleNV			mBufferClusterGrid;

	//GLuint						mClusterFlagTexture;
	//GLuint						mClusterGridTexture;
	//GlBufferObject<uint32_t>	mClusterFlagBuffer;
	//GlBufferObject<uint2>		mClusterGridBuffer;
	//GLuint						mClusterLightIndexListsTexture;
	//GlBufferObject<int>			mClusterLightIndexListsBuffer;

	
	// Clustering debug data.
	uint32_t					mTotalus;
	std::vector<int>			mIndexesHost;
	std::vector<uint2>			mClusterLights;		// HOST clusters grid uint2 (holding offset and count)
	std::vector<Cluster3d>		mClusters;			// for displaying clusters in 3d
	mat4						mPrevProjection;
	mat4						mPrevModelView;

	bool						mDebugDisplay;

	void bindClusteredForwardConstants(const GLuint programId, const GLuint clusterGridLoc, const GLuint clusterIndexLoc, const GLuint dirLightsLoc, const GLuint lightsLoc, const GLuint lightMatricesLoc);

	
	void assignLightsToClusters( const int numLights, 
		LightDATA *data, 
		const float nearPlane, 
		const float farPlane, 
		const mat4 &projection, 
		const mat4 &modelview, 
		const vec3 eyepos);
	
	// make snapshot of proj and mv matrices to see result of clusters calculations
	void	DebugMatrixSnapshot(const CCameraInfoCache &cameraCache);
	
public:
	
	CGPUBufferNV				mLightMatrices;

	std::vector<void*>			mLightCasters; // custom (FBLight*) user data
	std::vector<LightDATA*>		mLightCastersDataPtr;

	//GLint			mShadowMapSize;
	//GLint			mShadowPCFKernelSize;

	//GLuint			mShadowTexArray;	// 2d array - result of shadow composition

	// DONE: should be assigned from CGPUScene
	vec4		mWorldMin;
	vec4		mWorldMax;

	mat4		mLightViewMatrix[MAX_NUMBER_OF_LIGHTS];
    mat4		mLightProjMatrix[MAX_NUMBER_OF_LIGHTS];
	mat4		mLightInvTM[MAX_NUMBER_OF_LIGHTS];

	// TODO: initialize that value!
	int				mFrustumSegmentCount;
	float			mSplitWeight;
	float			mFarPlanes[MAX_NUMBER_OF_CASCADED_SPLITS];
	float			mNormalizedFarPlanes[MAX_NUMBER_OF_CASCADED_SPLITS];
	mat4			mLightSegmentVPSBMatrices[MAX_NUMBER_OF_CASCADED_SPLITS];
	vec4			mLightViewports[MAX_NUMBER_OF_CASCADED_SPLITS];
	CLightFrustum	mFrustums[MAX_NUMBER_OF_CASCADED_SPLITS];
	vec3			mCameraPos;
	vec3			mCameraUp;
	vec3			mCameraDir;
	vec3			mCameraRight;

	void UpdateSplitDist(CLightFrustum *f, float nd, float fd);
	float ApplyCropMatrix(const int index, CLightFrustum &f, mat4 &lightViewMatrix, mat4 &lightProjMatrix);

	bool ComputeLightMatrices( const LightDATA &lightData, mat4 &pLightView, mat4 &pLightProj, mat4 &lightInvTM );
	bool ComputeSpotLightMatrices( const LightDATA &lightData, mat4 &pLightView, mat4 &pLightProj, mat4 &lightInvTM );
	bool ComputeInfiniteLightMatrices( const LightDATA &lightData, mat4 &pLightView, mat4 &pLightProj, mat4 &lightInvTM );

	void BindLightViewports();

	void	UpdateFrustumSegmentFarPlanes(CCameraInfoCache *pCache, const float fructumSplitCorrection, const float nearPlane, const float farPlane);
	void	FrustumBoundingBoxLightViewSpace(CCameraInfoCache *pCache, mat4 &lightViewMatrix, float nearPlane, float farPlane, vec4 &min, vec4 &max);
	void	UpdateFrustumPoints(CCameraInfoCache *pCache, CLightFrustum &f);
	void	UpdateCascadedLightProjAndViewports(CCameraInfoCache *pCache, mat4 &lightViewMatrix, mat4 &lightProjMatrix);

	void GetWorldBounds( const LightDATA &pLight, double& pRadius, vec4& pPos );
	
};