
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_lights.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_lights.h"

#include "Config_LightClusters.h"
#include <math.h>
#include <array>

#include "algorithm\math3d.h"

static bool g_enableGpuClustering = true; /**< Controls whether the GPU is used to find required 
                                               clusters by performing a flag pass. Requires 
                                               GLEW_EXT_shader_image_load_store, and is automatically
                                               set to false if this extension is missing. */

/////////////////////////////////////////////////////////////////
/*
double sDefaultAttenuationNone[3] = { 1.0, 0.0, 0.0 };
double sDefaultAttenuationLinear[3] = { 0.0, 0.01, 0.0 };
double sDefaultAttenuationQuadratic[3] = { 0.0, 0.0, 0.0001 };
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CGPULightsManager

CGPULightsManager::CGPULightsManager()
{
	/*
	mClusterFlagTexture = 0;
	mClusterGridTexture = 0;
	mClusterLightIndexListsTexture = 0;
	*/
	mNumberOfLights = 0;
	mNumberOfDirLights = 0;

	mDebugDisplay = false;

	mFrustumSegmentCount = 4;
	mSplitWeight = 0.8f;
}

CGPULightsManager::~CGPULightsManager()
{
	Free();
}

void CGPULightsManager::Initialize()
{
	/*
	// TODO: this one buffer depends on resolution, do we need to tweak it on resize ?!
	mClusterFlagBuffer.init( LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y * LIGHT_GRID_MAX_DIM_Z );
	//mClusterGridBuffer.init( LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y * LIGHT_GRID_MAX_DIM_Z );
	//mFullClusterFlagsDebug.resize( LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y * LIGHT_GRID_MAX_DIM_Z, 0 );

	glGenTextures(1, &mClusterFlagTexture);
	glBindTexture(GL_TEXTURE_BUFFER, mClusterFlagTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, mClusterFlagBuffer);
	
	glGenTextures(1, &mClusterGridTexture);
	glBindTexture(GL_TEXTURE_BUFFER, mClusterGridTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32UI, mClusterGridBuffer);
	
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	glGenTextures(1, &mClusterLightIndexListsTexture);
	// initial size is 1, because glTexBuffer failed if it was empty, we'll shovel in data later.

	// TODO: 1024 is a limit of max number of lights, do we need to resize this buffer depends on the number of lights ?!
	
	mClusterLightIndexListsBuffer.init(4 * 1024 * 1024);
	glBindTexture(GL_TEXTURE_BUFFER, mClusterLightIndexListsTexture);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, mClusterLightIndexListsBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	CHECK_GL_ERROR();
	
	// clear the flag buffer for first time... (done here because CUDA barfs when mapped if done before...);
	uint32_t *p = mClusterFlagBuffer.beginMap();
	memset(p, 0, sizeof(p[0]) * LIGHT_GRID_MAX_DIM_X * LIGHT_GRID_MAX_DIM_Y * LIGHT_GRID_MAX_DIM_Z);
	mClusterFlagBuffer.endMap();
	*/
	//
	//
}

void CGPULightsManager::Free()
{
	
}

void CGPULightsManager::Clear()
{
	mDirLights.clear();
	mLights.clear();

	mNumberOfDirLights = 0;
	mNumberOfLights = 0;

	mClusterLights.clear();
	mClusters.clear();
}

void CGPULightsManager::Bind(const GLuint programId, const GLuint clusterGridLoc, const GLuint clusterIndexLoc, const GLuint dirLightsLoc, const GLuint lightsLoc, const GLuint lightMatricesLoc)
{
	bindClusteredForwardConstants(programId, clusterGridLoc, clusterIndexLoc, dirLightsLoc, lightsLoc, lightMatricesLoc);
}

void CGPULightsManager::UnBind()
{
}

void CGPULightsManager::DebugMatrixSnapshot(const CCameraInfoCache &cameraCache)
{
	mPrevModelView = cameraCache.mv4;
	mPrevProjection = cameraCache.p4;
}

//////////////////////////////////////////////////////////////////
// Main function to prepare scene lights in rendering frame
//////////////////////////////////////////////////////////////////

void CGPULightsManager::Prep(const CCameraInfoCache &cameraCache, const float *realFarPlane)
{
	/*
	if (mClusterFlagTexture == 0 || mClusterGridTexture == 0)
		Initialize();
*/
	
	//updateLightsFromFBScene( pCamera );

	//if (pCamera == nullptr) return;
	// TODO: add gpu clustering step to find which clusters contains geometry which not !

	mTotalus = 0u;

	if (mLights.size() == 0) 
	{
		return; // no lights to proceed
	}

	globalSettings settings;
	settings.farPlane = (realFarPlane) ? *realFarPlane : (float)cameraCache.farPlane;
	settings.nearPlane = cameraCache.nearPlane;
	settings.fov = cameraCache.fov;
	settings.width = cameraCache.width;
	settings.height = cameraCache.height;
	
	// TODO: this should be an optional feature, but at the moment cluster lights are depricated !
	//assignLightsToClusters( (const int) mLights.size(), mLights.data(), settings.nearPlane, settings.farPlane, cameraCache.p4, cameraCache.mv4, cameraCache.pos );
	
#ifdef _DEBUG
	if (mDebugDisplay)
	{
		BSphere bs;
		computeFrustumBSphere( settings.nearPlane, settings.farPlane, settings.fov, settings.width, settings.height, cameraCache.mv4, cameraCache.pos, bs );
		
		glPushMatrix();

		glTranslated( bs.center[0], bs.center[1], bs.center[2] );
		//glutWireSphere(bs.radius, 12, 12);

		glPopMatrix();

		DrawCameraFrustum( settings.nearPlane, settings.farPlane, cameraCache.p4, cameraCache.mv4 );
	}
#endif

	//
	//
	mat4 modelrotation(cameraCache.mv4);
	modelrotation.set_translation( vec3(0.0f, 0.0f, 0.0f) );

	for (size_t i=0; i<mLights.size(); ++i)
	{
		mLights[i].position = cameraCache.mv4 * mLights[i].position;
		mLights[i].dir = modelrotation * mLights[i].dir;
	}
}

void CGPULightsManager::MapOnGPU()
{
	// cluster indixes
	//mClusterLightIndexListsBuffer.copyFromHost( data, mTotalus );
	mBufferClusterIndex.UpdateData( mTotalus, sizeof(int), mIndexesHost.data() );
	

	// cluster grid
	//mClusterGridBuffer.copyFromHost( mClusterLights.data(), mClusterLights.size() );
	mBufferClusterGrid.UpdateData( mClusterLights.size(), sizeof(uint2), mClusterLights.data() );
	

	// dir lights
	mBufferDirLights.UpdateData( mDirLights.size(), sizeof(LightDATA), mDirLights.data() );
	

	// point / spot lights
	mBufferLights.UpdateData( mLights.size(), sizeof(LightDATA), mLights.data() );
	
}

void CGPULightsManager::PrepGPUPtr()
{
	// clusters
	if ( mBufferClusterGrid.GetCount() > 0 )
		mBufferClusterGrid.UpdateGPUPtr();

	if ( mBufferClusterIndex.GetCount() > 0 )
		mBufferClusterIndex.UpdateGPUPtr();

	// dir lights
	if ( mBufferDirLights.GetCount() > 0 )
		mBufferDirLights.UpdateGPUPtr();
	
	// point / spot
	if ( mBufferLights.GetCount() > 0 )
		mBufferLights.UpdateGPUPtr();
	

	mNumberOfDirLights = (int) mBufferDirLights.GetCount();
	mNumberOfLights = (int) mBufferLights.GetCount();
}


// helper to bind texture...
static void bindTexture(GLenum type, int texUnit, int textureId)
{
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(type, textureId);
}

void CGPULightsManager::bindClusteredForwardConstants(const GLuint programId, const GLuint clusterGridLoc, const GLuint clusterIndexLoc, const GLuint dirLightsLoc, const GLuint lightsLoc, const GLuint lightMatricesLoc)
{
	//std::vector<int> balh(10 * 1024 * 1024, 0);
	//g_clusterLightIndexListsBuffer.copyFromHost(&balh[0], balh.size());
	//glBindTexture(GL_TEXTURE_BUFFER, g_clusterLightIndexListsTexture);
	//glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, g_clusterLightIndexListsBuffer);
	//glBindTexture(GL_TEXTURE_BUFFER, 0);

	//bindTexture(GL_TEXTURE_BUFFER, BIND_CLUSTERS_INDEX_DATA, mClusterLightIndexListsTexture);
	//bindTexture(GL_TEXTURE_BUFFER, BIND_CLUSTERS_GRID_DATA, mClusterGridTexture);

	//mClusterLightIndexListsBuffer.bindSlot( GL_SHADER_STORAGE_BUFFER, 3 );
	//mClusterGridBuffer.bindSlot( GL_SHADER_STORAGE_BUFFER, 4 );

	// bind dir lights uniforms
	if (programId > 0)
	{
		mBufferClusterGrid.BindAsUniform( programId, clusterGridLoc, 0 );
		mBufferClusterIndex.BindAsUniform( programId, clusterIndexLoc, 0 );
		mBufferDirLights.BindAsUniform( programId, dirLightsLoc, 0 );
		mBufferLights.BindAsUniform( programId, lightsLoc, 0 );	
		mLightMatrices.BindAsUniform( programId, lightMatricesLoc, 0 );
	}
}



float clamp(const float f, const float a, const float b)
{
	if (f<a) return a;
	else
	if (f>b) return b;
	else
		return f;
}

uint32_t clamp(const uint32_t f, const uint32_t a, const uint32_t b)
{
	if (f<a) return a;
	else
	if (f>b) return b;
	else
		return f;
}

vec4 clamp(const vec4 v, const vec4 a, const vec4 b)
{
	vec4 result;
	result.x = clamp(v.x, a.x, b.x);
	result.y = clamp(v.y, a.y, b.y);
	result.z = clamp(v.z, a.z, b.z);
	result.w = clamp(v.w, a.w, b.w);
	return result;
}

uint3 clamp(const uint3 v, const uint3 a, const uint3 b)
{
	uint3 result;
	result.x = clamp(v.x, a.x, b.x);
	result.y = clamp(v.y, a.y, b.y);
	result.z = clamp(v.z, a.z, b.z);
	return result;
}

void vec4Mult( const vec4 &src, const mat4 &mat, vec4 &dst )
{
	dst = mat * src;

	dst.x /= dst.w;
	dst.y /= dst.w;
	dst.z /= dst.w;
	dst.w = 1.0f;
}

void vec3Mult( const mat4 &proj, const mat4 &InvMVP, const vec4 &src, vec3 &dst )
{
	vec4 dst4( proj * vec4(0.0, 0.0, -src.z, 1.0) );
	if (dst4.w == 0.0f) dst4.w = 1.0f;
	dst4 = InvMVP * vec4(src.x, src.y, dst4.z / dst4.w, 1.0);

	dst.x = dst4.x / dst4.w;
	dst.y = dst4.y / dst4.w;
	dst.z = dst4.z / dst4.w;
}

void DrawCluster(const vec3 &vmin, const vec3 &vmax)
{
	glVertex3fv(vmin.vec_array);
	glVertex3f(vmax.x, vmin.y, vmin.z);
	
	glVertex3f(vmax.x, vmin.y, vmin.z);
	glVertex3f(vmax.x, vmax.y, vmin.z);

	glVertex3f(vmax.x, vmax.y, vmin.z);
	glVertex3f(vmin.x, vmax.y, vmin.z);

	glVertex3f(vmin.x, vmax.y, vmin.z);
	glVertex3fv(vmin.vec_array);

	//
	glVertex3fv(vmax.vec_array);
	glVertex3f(vmin.x, vmax.y, vmax.z);
	
	glVertex3f(vmin.x, vmax.y, vmax.z);
	glVertex3f(vmin.x, vmin.y, vmax.z);

	glVertex3f(vmin.x, vmin.y, vmax.z);
	glVertex3f(vmax.x, vmin.y, vmax.z);

	glVertex3f(vmax.x, vmin.y, vmax.z);
	glVertex3fv(vmax.vec_array);
}


bool calculateLightRegionAndCheck(const vec3 &pos, const float radius, const float farPlane, const mat4 &modelview, const mat4 &projection, const vec3 &eyepos, vec3 &vmin, vec3 &vmax, float &minZ, float &maxZ)
{
	vmin = vec3(10.0f, 10.0f, 10.0f);
	vmax = vec3(-10.0f, -10.0f, -10.0f);

	/*
	vec4 verts[8] =
	{
		vec4( pos.x+radius, pos.y-radius, pos.z-radius, 1.0 ),
		vec4( pos.x+radius, pos.y+radius, pos.z-radius, 1.0 ),
		vec4( pos.x+radius, pos.y+radius, pos.z+radius, 1.0 ),
		vec4( pos.x+radius, pos.y-radius, pos.z+radius, 1.0 ),

		vec4( pos.x-radius, pos.y-radius, pos.z-radius, 1.0 ),
		vec4( pos.x-radius, pos.y+radius, pos.z-radius, 1.0 ),
		vec4( pos.x-radius, pos.y+radius, pos.z+radius, 1.0 ),
		vec4( pos.x-radius, pos.y-radius, pos.z+radius, 1.0 ),
	};
	*/

	for (int j=0; j<8; ++j)
	{
		double x = (j & 1) ? pos.x-radius : pos.x+radius;
		double y = (j & 2) ? pos.y-radius : pos.y+radius;
		double z = (j & 4) ? pos.z-radius : pos.z+radius;

		vec4 vert(x, y, z, 1.0);

		//verts[j] = modelview * verts[j];
		//verts[j] = projection * verts[j];

		vert = modelview * vert;
		vert = projection * vert;

		const float invw = 1.0f / vert.w;

		vert.x *= invw;
		vert.y *= invw;
		vert.z *= invw;
		vert.w = 1.0f;
			
		vmin.x = std::min(vmin.x, vert.x);
		vmin.y = std::min(vmin.y, vert.y);
		vmin.z = std::min(vmin.z, vert.z);

		vmax.x = std::max(vmax.x, vert.x);
		vmax.y = std::max(vmax.y, vert.y);
		vmax.z = std::max(vmax.z, vert.z);
	}
	/*
	vmin.x = -1.0f;
	vmin.y = -1.0f;
	vmax.x = 1.0f;
	vmax.y = 1.0f;
	*/
	if (vmin.x > 1.0f || vmin.y > 1.0f || vmax.x < -1.0f || vmax.y < -1.0f )
		return false;
		
	vec3 dir = pos - eyepos;
	float len = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);

	minZ = (len-radius) / farPlane * CLUSTERS_DEPTH_COUNT;
	maxZ = (len+radius) / farPlane * CLUSTERS_DEPTH_COUNT;
	/*
	glPushMatrix();

	glTranslatef( pos.x, pos.y, pos.z );
	glutWireSphere( radius, 12, 12 );

	glPopMatrix();
	*/
	//
	//vmin.z = minZ;
	//vmax.z = maxZ;

	return true;
}

void CGPULightsManager::assignLightsToClusters( const int numLights, LightDATA *data, const float nearPlane, const float farPlane, const mat4 &projection, const mat4 &modelview, const vec3 eyepos)
{
	mat4 temp, invMVP, invModelView, transposedMV;
	temp = modelview * projection;
	
	invert(invMVP, temp);
	invert(invModelView, modelview);
	transpose(transposedMV, modelview);

	CFrustum frustum;
	frustum.CalculateFrustum( projection.mat_array, modelview.mat_array );

	float xStep = 2.0 / CLUSTERS_WIDTH_COUNT;
	float yStep = 2.0 / CLUSTERS_HEIGHT_COUNT;
	float zStep = 2.0 / CLUSTERS_DEPTH_COUNT;

	mClusterLights.resize(CLUSTERS_TOTAL_COUNT);
	for (int i=0; i<CLUSTERS_TOTAL_COUNT; ++i)
	{
		mClusterLights[i].x = 0;	// count
		mClusterLights[i].y = 0;	// offset
	}

	mTotalus = 0u;

	// draw lights clusters in viewport
	std::vector<ScreenRect3D>	rects;
	rects.reserve(numLights);

	for (int i=0; i<numLights; ++i)
	{
		// DONE: light region
		vec3 pos = data[i].position;
		//float radius = data[i].attenuations[3];
		float radius = data[i].radius;

		// DONE: prepare pos and radius for spot light
		//
		if (data[i].type == LIGHT_TYPE_SPOT)
		{
			const float angle = data[i].spotAngle;

			mat4	M;
			look_at( M, pos, pos + data[i].dir, vec3(0.0f, -1.0f, 0.0f) );

			BSphere bs;
			computeFrustumBSphere( 0.1f, radius, angle, 512.0f, 512.0f, M, pos, bs );

			pos = bs.center;
			radius = bs.radius;
			/*
			glPushMatrix();

			glTranslated( bs.center[0], bs.center[1], bs.center[2] );
			glutWireSphere(bs.radius, 12, 12);

			glPopMatrix();
			*/
		}

		if ( frustum.SphereInFrustum( pos.x, pos.y, pos.z, radius ) == false )
		{
			continue;	// skip to processing that light
		}

		vec3 vmin, vmax;
		float minZ, maxZ;
		if (calculateLightRegionAndCheck(pos, radius, farPlane, modelview, projection, eyepos, vmin, vmax, minZ, maxZ) == false )
		{
			continue; // skip if light is out of window rect
		}

		ScreenRect3D rect;
		rect.index = i;
		rect.min.x = (uint32_t) std::max(0.0f, floor((vmin.x + 1.0f) / xStep));	// shift to [0; 2] and find step index
		rect.min.y = (uint32_t) std::max(0.0f, floor((vmin.y + 1.0f) / yStep));
		rect.min.z = (uint32_t) std::max(0.0f, floor(minZ));
		rect.max.x = (uint32_t) std::min( ceil((vmax.x + 1.0f) / xStep), (float) CLUSTERS_WIDTH_COUNT-1.0f );
		rect.max.y = (uint32_t) std::min( ceil((vmax.y + 1.0f) / yStep), (float) CLUSTERS_HEIGHT_COUNT-1.0f );
		rect.max.z = (uint32_t) std::min( ceil(maxZ), (float)CLUSTERS_DEPTH_COUNT-1.0f );
		rects.push_back(rect);

		// determine clusters with this light

		for (uint32_t x=rect.min.x; x<=rect.max.x; ++x)
			for (uint32_t y=rect.min.y; y<=rect.max.y; ++y)
				for (uint32_t z=rect.min.z; z<=rect.max.z; ++z)	
				{
					//const uint32_t index = x*(CLUSTERS_HEIGHT_COUNT*CLUSTERS_DEPTH_COUNT) + y*CLUSTERS_DEPTH_COUNT + z;
					const uint32_t index = x + CLUSTERS_WIDTH_COUNT * (y + z * CLUSTERS_HEIGHT_COUNT);
					if (index > CLUSTERS_TOTAL_COUNT)
						printf( "error\n" );

					mClusterLights[index].x += 1;
					++mTotalus;
				}

		//
		// draw
#ifdef _DEBUG
		if (mDebugDisplay)
		{
			vec4 vmin4 = invMVP * vec4(vmin.x, vmin.y, vmin.z, 1.0);
			vec4 vmax4 = invMVP * vec4(vmax.x, vmax.y, vmax.z, 1.0);

			vmin.x = vmin4.x / vmin4.w;
			vmin.y = vmin4.y / vmin4.w;
			vmin.z = vmin4.z / vmin4.w;

			vmax.x = vmax4.x / vmax4.w;
			vmax.y = vmax4.y / vmax4.w;
			vmax.z = vmax4.z / vmax4.w;

			glColor3f(0.0f, 0.0f, 1.0f);
			glBegin(GL_LINES);
				DrawCluster( vmin, vmax );
			glEnd();
		}
#endif
	}


	// 1 pass - prepare offsets

	uint32_t offset = 0u;

	uint32_t maxTileLightsCount = 0u;

	for (int x=0; x<CLUSTERS_WIDTH_COUNT; ++x)
		for (int y=0; y<CLUSTERS_HEIGHT_COUNT; ++y)
			for (int z=0; z<CLUSTERS_DEPTH_COUNT; ++z)
			{
				//const uint32_t index = i*(CLUSTERS_HEIGHT_COUNT*CLUSTERS_DEPTH_COUNT) + j*CLUSTERS_DEPTH_COUNT + k;
				const uint32_t index = x + CLUSTERS_WIDTH_COUNT * (y + z * CLUSTERS_HEIGHT_COUNT);
				if (index > CLUSTERS_TOTAL_COUNT)
					printf( "error\n" );

				uint32_t count = mClusterLights[index].x;
				mClusterLights[index].y = offset + count;
				offset += count;

				maxTileLightsCount = std::max(maxTileLightsCount, count);
			}

	// 2 pass - write offsets
	if (rects.size() && mTotalus > 0 )
	{
		mIndexesHost.resize(mTotalus);
		int *data = &mIndexesHost[0];

		for (size_t i=0; i<rects.size(); ++i)
		{
			ScreenRect3D rect = rects[i];

			for (uint32_t x=rect.min.x; x<=rect.max.x; ++x)
				for (uint32_t y=rect.min.y; y<=rect.max.y; ++y)
					for (uint32_t z=rect.min.z; z<=rect.max.z; ++z)	
					{
						//const uint32_t index = x*(CLUSTERS_HEIGHT_COUNT*CLUSTERS_DEPTH_COUNT) + y*CLUSTERS_DEPTH_COUNT + z;
						const uint32_t index = x + CLUSTERS_WIDTH_COUNT * (y + z * CLUSTERS_HEIGHT_COUNT);
						if (index > CLUSTERS_TOTAL_COUNT)
							printf( "error\n" );

						uint32_t offset = mClusterLights[index].y-1;
						data[offset] = rect.index;
						mClusterLights[index].y = offset;
					}
		}
	}
}

void CGPULightsManager::DrawCameraFrustum(const float nearPlane, const float farPlane, const mat4 &projection, const mat4 &modelview)
{
	mat4 temp, inv, invModelView;
	temp = projection * modelview;
	inv = invert(inv, temp);
	invModelView = invert(invModelView, modelview);

	vec4 fr[8]= {
		// near
		vec4(-1, -1, -1, 1), vec4(1, -1, -1, 1), vec4(1,  1, -1, 1), vec4(-1,  1, -1, 1),
		// far
		vec4(-1, -1, 1, 1),	vec4(1, -1, 1, 1), vec4(1, 1, 1, 1), vec4(-1, 1, 1, 1)
	};

	vec4 tfr[8];

	// transform all vertices
	for (int i=0; i<8; ++i)
	{
		tfr[i] = inv * fr[i];

		tfr[i].x /= tfr[i].w;
		tfr[i].y /= tfr[i].w;
		tfr[i].z /= tfr[i].w;
		tfr[i].w = 1.0f;
	}

	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex4fv(tfr[0].vec_array);
	glVertex4fv(tfr[1].vec_array);

	glVertex4fv(tfr[1].vec_array);
	glVertex4fv(tfr[2].vec_array);

	glVertex4fv(tfr[2].vec_array);
	glVertex4fv(tfr[3].vec_array);

	glVertex4fv(tfr[3].vec_array);
	glVertex4fv(tfr[0].vec_array);

	glVertex4fv(tfr[4].vec_array);
	glVertex4fv(tfr[5].vec_array);

	glVertex4fv(tfr[5].vec_array);
	glVertex4fv(tfr[6].vec_array);

	glVertex4fv(tfr[6].vec_array);
	glVertex4fv(tfr[7].vec_array);

	glVertex4fv(tfr[7].vec_array);
	glVertex4fv(tfr[4].vec_array);

	glVertex4fv(tfr[0].vec_array);
	glVertex4fv(tfr[4].vec_array);

	glVertex4fv(tfr[1].vec_array);
	glVertex4fv(tfr[5].vec_array);

	glVertex4fv(tfr[2].vec_array);
	glVertex4fv(tfr[6].vec_array);

	glVertex4fv(tfr[3].vec_array);
	glVertex4fv(tfr[7].vec_array);
	glEnd();


	
	//
	//

	if (mClusterLights.size() == CLUSTERS_TOTAL_COUNT)
	{

		mClusters.resize(CLUSTERS_TOTAL_COUNT);

		vec4 vMult(2.0, 2.0, 2.0, 1.0);
		vec4 vOffset(0.5, 0.5, 0.5, 0.0);

		float xStep = 2.0 / CLUSTERS_WIDTH_COUNT;
		float yStep = 2.0 / CLUSTERS_HEIGHT_COUNT;
		float zStep = 2.0 / CLUSTERS_DEPTH_COUNT;


		for (int x=0; x<CLUSTERS_WIDTH_COUNT; ++x)
			for (int y=0; y<CLUSTERS_HEIGHT_COUNT; ++y)
				for (int z=0; z<CLUSTERS_DEPTH_COUNT; ++z)
				{
					const float realZ = (nearPlane+(farPlane-nearPlane)) / (CLUSTERS_DEPTH_COUNT) * z;

					vec4 vmin( -1.0f+xStep*x, -1.0f+yStep*y, realZ, 1.0f );
					vec4 vmax( -1.0f+xStep*(x+1), -1.0f+yStep*(y+1), realZ, 1.0f );

					//const uint32_t index = i*(CLUSTERS_HEIGHT_COUNT*CLUSTERS_DEPTH_COUNT) + j*CLUSTERS_DEPTH_COUNT + k;
					const uint32_t index = x + CLUSTERS_WIDTH_COUNT * (y + z * CLUSTERS_HEIGHT_COUNT);
					if (index > CLUSTERS_TOTAL_COUNT)
						printf( "error\n" );

				
					vec3Mult( projection, inv, vmin, mClusters[index].min );
					vec3Mult( projection, inv, vmax, mClusters[index].max );
				}

		// debug draw all clusters

		glBegin(GL_LINES);
		for (size_t i=0; i<mClusters.size(); ++i)
		{
			if (mClusterLights[i].x > 0)
			{
				glColor3f( 1.0f, 0.0f, 0.0f);
			}
			else
			{
				glColor3f( 1.0f, 1.0f, 0.0f );
				continue;
			
			}
			DrawCluster( mClusters[i].min, mClusters[i].max );
		}
		glEnd();
	}
}

/////////////////////////////////////////////////////////////////////////////
// shadows


bool CGPULightsManager::PrepShadowMaps( CCameraInfoCache *pCache, const vec3 &worldMin, const vec3 &worldMax, const int shadowMapSize,
	const LightDATA &lightData, const int cascadedSplits, const float cascadedCorrection, const float cascadedNearPlane, const float cascadedFarPlane)
{

	if (mLightCasters.size() == 0 || pCache == nullptr)
		return false;

	CHECK_GL_ERROR();
	
	mWorldMin = vec4( worldMin[0], worldMin[1], worldMin[2], 1.0f );
	mWorldMax = vec4( worldMax[0], worldMax[1], worldMax[2], 1.0f );

	// Initialize shadow buffers
	const int shadowPCFKernelSize = 3;

	mCameraPos = pCache->pos;
	//mShadowPCFKernelSize = shadowPCFKernelSize;
	
	CHECK_GL_ERROR();

	// Compute shadow maps
	mat4	vp;
	std::array<mat4, MAX_NUMBER_OF_LIGHTS+MAX_NUMBER_OF_CASCADED_SPLITS>		m4_vp; // [MAX_NUMBER_OF_LIGHTS + MAX_NUMBER_OF_CASCADED_SPLITS];
	vec4		normalizedFarPlanes(1.0f, 1.0f, 1.0f, 1.0f);

	int dataIndex = 0;

	int lLightCount = std::min( MAX_NUMBER_OF_LIGHTS, (int) mLightCasters.size() );
	for( int i = 0; i < lLightCount; i++ )
	{
		// Calculate the light matrices
		//FBLight		*pLight = mLightCasters[i];
		LightDATA	*pLightData = mLightCastersDataPtr[i];

		// use first infinite light as a cascaded light for testing
		if (i==0) //(pLight == pCascadedLight)
		{
			// DONE: prepare cascaded matrices
			
			// RESULT: mFarPlanes, mNormalizedFarPlanes !
			UpdateFrustumSegmentFarPlanes(pCache, cascadedCorrection, cascadedNearPlane, cascadedFarPlane);

			// compute light view matrix
			mat4 lightViewMatrix;
			mat4 lightProjMatrix;

			lightViewMatrix.identity();
			lightProjMatrix.identity();

			ComputeInfiniteLightMatrices(*pLightData, mLightViewMatrix[i], mLightProjMatrix[i], mLightInvTM[i]);

			lightViewMatrix = mLightViewMatrix[i]; 
			lightProjMatrix = mLightProjMatrix[i];
			
			//GLfloat light_dir[4] = {0.2f, 0.99f, 0.0f , 0.0f};
			//look_at( lightViewMatrix, vec3(0.0f, 0.0f, 0.0f), vec3(-light_dir[0], -light_dir[1], -light_dir[2]), vec3(-1.0f, 0.0f, 0.0f) );

			// RESULT: mLightSegmentVPSBMatrices !
			//UpdateCascadedLightProjAndViewports(pCache, lightViewMatrix, lightProjMatrix);

			
			// compute the z-distances for each split as seen in camera space
			//UpdateSplitDist( mFrustums, 1.0f, 4000.0f );

			for (int j=0; j<mFrustumSegmentCount; ++j)
			{
				// note that fov is in radians here and in OpenGL it is in degrees.
				// the 0.2f factor is important because we might get artifacts at
				// the screen borders.
				mFrustums[j].fov = nv_to_rad * pCache->fov + 0.2f;
				mFrustums[j].ratio = static_cast<float>(pCache->width) / pCache->height;
				mFrustums[j].neard = (j==0) ? pCache->nearPlane : mFrustums[j-1].fard;
				mFrustums[j].fard = mFarPlanes[j];
				// compute the camera frustum slice boundary points in world space
				UpdateFrustumPoints(pCache, mFrustums[j]);

				// adjust the view frustum of the light, so that it encloses the camera frustum slice fully.
				// note that this function sets the projection matrix as it sees best fit
				// minZ is just for optimization to cull trees that do not affect the shadows
				float minZ = ApplyCropMatrix(j, mFrustums[j], lightViewMatrix, lightProjMatrix);

			}
			

			// assign result
			for (int j=0; j<mFrustumSegmentCount; ++j)
			{
				// store each light segment into struct
				m4_vp[dataIndex+j] = mLightSegmentVPSBMatrices[j];
			}

			normalizedFarPlanes = vec4(mNormalizedFarPlanes[0], mNormalizedFarPlanes[1], mNormalizedFarPlanes[2], mNormalizedFarPlanes[3]);

			if (pLightData)
			{
				pLightData->shadowMapLayer = (float) dataIndex;
				pLightData->shadowMapSize = (float) shadowMapSize;
				pLightData->shadowPCFKernelSize = (float) 9.0;
				pLightData->shadowIndex = vec4( (float) mFrustumSegmentCount, 0.0f, 0.0f, 0.0f );
				pLightData->normalizedFarPlanes = normalizedFarPlanes;
			}

			dataIndex += mFrustumSegmentCount;
		}
		else
		if ( true == ComputeLightMatrices(*pLightData, mLightViewMatrix[i], mLightProjMatrix[i], mLightInvTM[i]) )
		{
			// Upload model independent shadow parameters
			
			mult( vp, mLightProjMatrix[i], mLightViewMatrix[i] );

			m4_vp[dataIndex] = vp;
			
			/*
			mat4 clip2Tex;
			clip2Tex.identity();
			clip2Tex.set_scale(vec3(0.5f, 0.5f, 0.5f));
			clip2Tex.set_translation(vec3(0.5f, 0.5f, 0.5f));
			*/
			// DONE: put in a lights structure
			if (pLightData)
			{
				pLightData->shadowMapLayer = (float) dataIndex;
				pLightData->shadowMapSize = (float) shadowMapSize;
				pLightData->shadowPCFKernelSize = (float) 9.0;
				//pLightData->shadowVP = clip2Tex * m4_vp[i];
				pLightData->shadowIndex = vec4( 0.0f, 0.0f, 0.0f, 0.0f );
				pLightData->normalizedFarPlanes = normalizedFarPlanes;
			}

			dataIndex += 1;
		}
	}

	mLightMatrices.UpdateData( sizeof(mat4), m4_vp.size(), m4_vp.data() );

	return true;
}

/*
bool CGPULightsManager::RenderShadowMaps( FBRenderOptions* pRenderOptions, const ShadowSettings &settings, FBArrayTemplate<FBLight*>* pAffectingLightList, FBArrayTemplate<FBModel*>* pShadowCasters )
{
	if( !pAffectingLightList || pAffectingLightList->GetCount() == 0 )
	{
		return false;
	}

	//g_lightBlock.ShadowColor = vec4( settings.shadowColor[0], settings.shadowColor[1], settings.shadowColor[2], settings.shadowIntensity );

	//if (onceUsed) return false;

	// Initialize shadow buffers
	InitializeBuffers( settings.shadowMapSize, pAffectingLightList );
	mShadowPCFKernelSize = settings.shadowPCFKernelSize;

	CHECK_GL_ERROR();

	// Store current frame buffer
	SaveFrameBuffer();

	// Compute the world bounds for infinite light adjustment.
	//ComputeWorldBounds();

	// Save the viewport
	glPushAttrib( GL_VIEWPORT_BIT );

	// Set the viewport to the proper size
	glViewport( 0, 0, settings.shadowMapSize, settings.shadowMapSize );

	glPolygonOffset( settings.offsetScale, settings.offsetBias );
	glEnable( GL_POLYGON_OFFSET_FILL );

	// Set the actual shaders 
	BindShadowPrograms();

	// TODO: use MRT to write several textures at once

	// Clear the depth buffer
	glClearDepth( 1.0 );
	glClear( GL_DEPTH_BUFFER_BIT );

	// Upload model independent shadow parameters
	UploadShadowParameters( i, pRenderOptions );

	// Draw the shadow models
	RenderShadowModels( pShadowCasters );

	CHECK_GL_ERROR();

	// Compute shadow maps
	int lLightCount = min( MAX_NUMBER_OF_LIGHTS, pAffectingLightList->GetCount() );
	for( int i = 0; i < lLightCount; i++ )
	{
		// Calculate the light matrices
		FBLight* light = pAffectingLightList->GetAt(i);
		if ( (mFrameBuffer[i] > 0) && light->CastShadows && true == ComputeLightMatrices(light, mLightViewMatrix[i], mLightProjMatrix[i], mLightInvTM[i]) )
		{

			// Setup to render into the shadow buffer ...
			glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, mFrameBuffer[i] );

			//glDepthRangedNV(0.0, 1.0);

			// Clear the depth buffer
			glClearDepth( 1.0 );
			glClear( GL_DEPTH_BUFFER_BIT );

			// Upload model independent shadow parameters
			UploadShadowParameters( i, pRenderOptions );

			// Draw the shadow models
			RenderShadowModels( pShadowCasters );

			CHECK_GL_ERROR();
		}
	}

	// Set the actual shaders 
	UnbindShadowPrograms();

	// Set our frame buffer back to the default one ...
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPopAttrib(); // restore the viewport 
		
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	RestoreFrameBuffer();

	//onceUsed = true;
	
	CHECK_GL_ERROR();

	return true;
}
*/
/*
void CGPULightsManager::RenderShadowModels( FBArrayTemplate<FBModel*>* pShadowCasters )
{
	if( pShadowCasters )
	{
		for( int i = 0; i < pShadowCasters->GetCount(); i++ )
		{
			FBModel* lModel = pShadowCasters->GetAt(i);
			if( !lModel )
			{
				continue;
			}

			// Set the model matrix
			FBMatrix lModelMatrix;
			lModel->GetMatrix( lModelMatrix );
			
			if (fx_ShadowM)
			{
				mat4 m4_m;
				for (int ii=0; ii<16; ++ii)
					m4_m.mat_array[ii] = (float) lModelMatrix[ii];

				fx_ShadowM->updateMatrix4f( &m4_m.mat_array[0], fx_shadowPass );
			}
			
			// Draw the model
			RenderShadowModel( lModel );
		}
	}
	else
	{
		FBRenderer* lRenderer = FBSystem().Renderer;
		if( lRenderer )
		{
			for( int i = 0; i < lRenderer->DisplayableGeometryCount; i++ )
			{
				FBModel* lModel = lRenderer->GetDisplayableGeometry(i);
				if( !lModel )
				{
					continue;
				}

				// Set the model matrix
				FBMatrix lModelMatrix;
				lModel->GetMatrix( lModelMatrix );

				if (fx_ShadowM)
				{
					mat4 m4_m;
					for (int ii=0; ii<16; ++ii)
						m4_m.mat_array[ii] = (float) lModelMatrix[ii];

					fx_ShadowM->updateMatrix4f( &m4_m.mat_array[0], fx_shadowPass );
				}

				// Draw the model
				RenderShadowModel( lModel );
			}
		}
	}
}

*/



void CGPULightsManager::UpdateFrustumSegmentFarPlanes(CCameraInfoCache *pCache, const float frustumSplitCorrection, const float nearPlane, const float farPlane)
{
	for (unsigned int i=1; i <= mFrustumSegmentCount; ++i)
	{
		const float distFactor = static_cast<float>(i) / mFrustumSegmentCount;
		const float stdTerm = nearPlane * std::pow( (double) farPlane / nearPlane, (double) distFactor );
		const float corrTerm = nearPlane + distFactor * (farPlane - nearPlane);
		const float viewDepth = frustumSplitCorrection * stdTerm + (1.0f - frustumSplitCorrection) * corrTerm;

		mFarPlanes[i-1] = viewDepth;
		vec4 projectedDepth = pCache->p4 * vec4(0.0f, 0.0f, -viewDepth, 1.0f);
		// Normalized to [0, 1] depth range.
		mNormalizedFarPlanes[i-1] = (projectedDepth.z / projectedDepth.w) * 0.5f + 0.5f;
	}
}

void CGPULightsManager::FrustumBoundingBoxLightViewSpace(CCameraInfoCache *pCache, mat4 &lightViewMatrix, float nearPlane, float farPlane, vec4 &min, vec4 &max)
{
	const float floatMax = std::numeric_limits<float>::max();
	const float floatMin = std::numeric_limits<float>::lowest();
	vec4 frustumMin( floatMax, floatMax, floatMax, floatMax );
	vec4 frustumMax( floatMin, floatMin, floatMin, floatMin );

	float fov = nv_to_rad * pCache->fov;

	const float nearHeight = 2.0f * tan(fov * 0.5f) * nearPlane;
	const float nearWidth = nearHeight * static_cast<float>(pCache->width) / pCache->height;
	const float farHeight = 2.0f * tan(fov * 0.5f) * farPlane;
	const float farWidth = farHeight * static_cast<float>(pCache->width) / pCache->height;
	vec4 cameraPos = vec4(pCache->pos.x, pCache->pos.y, pCache->pos.z, 1.0f);// * vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	mat3 invRot;
	pCache->mvInv4.get_rot(invRot);
	vec4 viewDir = invRot * vec4(0.0f, 0.0f, -1.0f, 0.0f);
	vec4 upDir = invRot * vec4(0.0f, 1.0f, 0.0f, 0.0f);
	vec4 rightDir = invRot * vec4(1.0f, 0.0f, 0.0f, 0.0f);
	vec4 nc = cameraPos + nearPlane * viewDir; // near center
	vec4 fc = cameraPos + farPlane * viewDir; // far center

	upDir = normalize(upDir);
	rightDir = normalize(rightDir);

	// vertices in world space
	vec4 vertices[8] = {
		nc - 0.5f * nearHeight * upDir - 0.5f * nearWidth * rightDir, // nbl (near, bottom, left)
		nc - 0.5f * nearHeight * upDir + 0.5f * nearWidth * rightDir, // nbr
		nc + 0.5f * nearHeight * upDir + 0.5f * nearWidth * rightDir, // ntr
		nc + 0.5f * nearHeight * upDir - 0.5f * nearWidth * rightDir, // ntl

		nc - 0.5f * farHeight * upDir - 0.5f * farWidth * rightDir, // fbl (far, bottom, left)
		nc - 0.5f * farHeight * upDir + 0.5f * farWidth * rightDir, // fbr
		nc + 0.5f * farHeight * upDir + 0.5f * farWidth * rightDir, // ftr
		nc + 0.5f * farHeight * upDir - 0.5f * farWidth * rightDir, // ftl
	};

	for (unsigned int vertId = 0; vertId < 8; ++vertId)
	{
		// Light view space.
		vertices[vertId] = lightViewMatrix * vertices[vertId];
		// update bounding box
		nv_min(frustumMin, frustumMin, vertices[vertId]);
		nv_max(frustumMax, frustumMax, vertices[vertId]);
	}

	min = frustumMin;
	max = frustumMax;
}

void CGPULightsManager::UpdateFrustumPoints(CCameraInfoCache *pCache, CLightFrustum &f)
{
	mat3 invRot;
	pCache->mvInv4.get_rot(invRot);
	vec3 view_dir = invRot * vec3(0.0f, 0.0f, -1.0f);
	//vec3 view_dir(-0.7f, 0.0f, 0.7f);
	vec3 up = invRot * vec3(0.0f, 1.0f, 0.0f);
	//vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 right = invRot * vec3(1.0f, 0.0f, 0.0f);
	//vec3 right;
	//cross(right, view_dir, up);

	mCameraDir = view_dir;
	mCameraUp = up;
	mCameraRight = right;

	vec3 center = pCache->pos;
	//center = vec3(0.0f, 0.0f, 0.0f);
	//f.fard = pCache->farPlane;
	//f.neard = pCache->nearPlane;

	vec3 fc = center + f.fard*view_dir;
	vec3 nc = center + f.neard*view_dir;

	//right = normalize(right);
	//cross(up, right, view_dir);
	//up = normalize(up);

	// these heights and widths are half the heights and widths of
	// the near and far plane rectangles
	float near_height = tan(f.fov/2.0f) * f.neard;
	float near_width = near_height * f.ratio;
	float far_height = tan(f.fov/2.0f) * f.fard;
	float far_width = far_height * f.ratio;
	/*
	f.point[0] = nc - 0.5f*near_height*up - 0.5f*near_width*right;
	f.point[1] = nc + 0.5f*near_height*up - 0.5f*near_width*right;
	f.point[2] = nc + 0.5f*near_height*up + 0.5f*near_width*right;
	f.point[3] = nc - 0.5f*near_height*up + 0.5f*near_width*right;

	f.point[4] = fc - 0.5f*far_height*up - 0.5f*far_width*right;
	f.point[5] = fc + 0.5f*far_height*up - 0.5f*far_width*right;
	f.point[6] = fc + 0.5f*far_height*up + 0.5f*far_width*right;
	f.point[7] = fc - 0.5f*far_height*up + 0.5f*far_width*right;
	*/

	f.point[0] = nc - near_height*up - near_width*right;
	f.point[1] = nc + near_height*up - near_width*right;
	f.point[2] = nc + near_height*up + near_width*right;
	f.point[3] = nc - near_height*up + near_width*right;

	f.point[4] = fc - far_height*up - far_width*right;
	f.point[5] = fc + far_height*up - far_width*right;
	f.point[6] = fc + far_height*up + far_width*right;
	f.point[7] = fc - far_height*up + far_width*right;
}

// updateSplitDist computes the near and far distances for every frustum slice
// in camera eye space - that is, at what distance does a slice start and end
void CGPULightsManager::UpdateSplitDist(CLightFrustum *f, float nd, float fd)
{
	float lambda = mSplitWeight;
	float ratio = fd/nd;
	f[0].neard = nd;

	for(int i=1; i<mFrustumSegmentCount; i++)
	{
		float si = i / (float)mFrustumSegmentCount;

		f[i].neard = lambda*(nd*powf(ratio, si)) + (1-lambda)*(nd + (fd - nd)*si);
		f[i-1].fard = f[i].neard * 1.005f;
	}
	f[mFrustumSegmentCount-1].fard = fd;
}

// this function builds a projection matrix for rendering from the shadow's POV.
// First, it computes the appropriate z-range and sets an orthogonal projection.
// Then, it translates and scales it, so that it exactly captures the bounding box
// of the current frustum slice
float CGPULightsManager::ApplyCropMatrix(const int index, CLightFrustum &f, mat4 &shad_modelview, mat4 &lightProjMatrix)
{
	const float floatMax = std::numeric_limits<float>::max();
	const float floatMin = std::numeric_limits<float>::lowest();

	//mat4 shad_modelview;
	mat4 shad_crop;

	float shad_proj[16];
	float shad_mvp[16];

	float maxX = floatMin;
    float maxY = floatMin;
	float maxZ;
    float minX = floatMax;
    float minY = floatMax;
	float minZ;

	mat4 nv_mvp;
	vec4 transf;	
	
	// find the z-range of the current frustum as seen from the light
	// in order to increase precision
	
	nv_mvp = shad_modelview;

	// note that only the z-component is need and thus
	// the multiplication can be simplified
	// transf.z = shad_modelview[2] * f.point[0].x + shad_modelview[6] * f.point[0].y + shad_modelview[10] * f.point[0].z + shad_modelview[14];
	transf = nv_mvp*vec4(f.point[0],  1.0f);
	minZ = transf.z;
	maxZ = transf.z;
	for(int i=1; i<8; i++)
	{
		transf = nv_mvp*vec4(f.point[i], 1.0f);
		if(transf.z > maxZ) maxZ = transf.z;
		if(transf.z < minZ) minZ = transf.z;
	}
	// make sure all relevant shadow casters are included
	// note that these here are dummy objects at the edges of our scene
	maxZ += 100.0;
	minZ -= 100.0;
	/*
	for(int i=0; i<NUM_OBJECTS; i++)
	{
		transf = nv_mvp*vec4f(obj_BSphere[i].center, 1.0f);
		if(transf.z + obj_BSphere[i].radius > maxZ) maxZ = transf.z + obj_BSphere[i].radius;
	//	if(transf.z - obj_BSphere[i].radius < minZ) minZ = transf.z - obj_BSphere[i].radius;
	}
	*/


	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	// set the projection matrix with the new z-bounds
	// note the inversion because the light looks at the neg. z axis
	// gluPerspective(LIGHT_FOV, 1.0, maxZ, minZ); // for point lights
	mat4 projLight;
	projLight.identity();
	ortho(projLight, -1.0f, 1.0f, -1.0f, 1.0f, -maxZ, -minZ);
	nv_mvp = projLight * shad_modelview;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// set the projection matrix with the new z-bounds
	// note the inversion because the light looks at the neg. z axis
	// gluPerspective(LIGHT_FOV, 1.0, maxZ, minZ); // for point lights
	glOrtho(-1.0, 1.0, -1.0, 1.0, -maxZ, -minZ);
	glGetFloatv(GL_PROJECTION_MATRIX, shad_proj);
	glPushMatrix();
	glMultMatrixf(shad_modelview.mat_array);
	glGetFloatv(GL_PROJECTION_MATRIX, shad_mvp);
	glPopMatrix();

	for (int i=0; i<16; ++i)
	{
		nv_mvp.mat_array[i] = shad_mvp[i];
		projLight.mat_array[i] = shad_proj[i];
	}

	// find the extends of the frustum slice as projected in light's homogeneous coordinates
	//nv_mvp.set_value(shad_mvp);
	for(int i=0; i<8; i++)
	{
		transf = nv_mvp*vec4(f.point[i], 1.0f);

		transf.x /= transf.w;
		transf.y /= transf.w;

		if(transf.x > maxX) maxX = transf.x;
		if(transf.x < minX) minX = transf.x;
		if(transf.y > maxY) maxY = transf.y;
		if(transf.y < minY) minY = transf.y;
	}

	float scaleX = 2.0f/(maxX - minX);
	float scaleY = 2.0f/(maxY - minY);
	float offsetX = -0.5f*(maxX + minX)*scaleX;
	float offsetY = -0.5f*(maxY + minY)*scaleY;

	// apply a crop matrix to modify the projection matrix we got from glOrtho.
	shad_crop.identity();
	shad_crop.set_scale( vec3( scaleX, scaleY, 1.0f ) );
	shad_crop.set_translation( vec3(offsetX, offsetY, 0.0f) );

	//
	glLoadMatrixf(shad_crop.mat_array);
	glMultMatrixf(shad_proj);

	glGetFloatv(GL_PROJECTION_MATRIX, projLight.mat_array);

	mLightSegmentVPSBMatrices[index] = projLight * shad_modelview;
	
	return minZ;
}

void CGPULightsManager::UpdateCascadedLightProjAndViewports(CCameraInfoCache *pCache, mat4 &lightViewMatrix, mat4 &lightProjMatrix)
{
	// Find a bounding box of whole camera frustum in light view space.
	const float floatMax = std::numeric_limits<float>::max();
	const float floatMin = std::numeric_limits<float>::lowest();
	vec4 frustumMin( floatMax, floatMax, floatMax, floatMax );
	vec4 frustumMax( floatMin, floatMin, floatMin, floatMin );

	FrustumBoundingBoxLightViewSpace(pCache, lightViewMatrix, pCache->nearPlane, pCache->farPlane, frustumMin, frustumMax);

	// update light projection matrix to only cover the area viewable by the camera
	ortho(lightProjMatrix, frustumMin.x, frustumMax.x, frustumMin.y, frustumMax.y, frustumMin.z, frustumMax.z);

	// find a boudning box of segment in light view space.
	float nearSegmentPlane = 0.0f;
	const int frustumSegmentCount = 4;
	for (unsigned int i=0; i<frustumSegmentCount; ++i)
	{
		vec4 segmentMin( floatMax, floatMax, floatMax, floatMax );
		vec4 segmentMax( floatMin, floatMin, floatMin, floatMin );

		FrustumBoundingBoxLightViewSpace(pCache, lightViewMatrix, nearSegmentPlane, mFarPlanes[i], segmentMin, segmentMax);

		// update viewports
		vec2 frustumSize(frustumMax.x - frustumMin.x, frustumMax.y - frustumMin.y);
		const float segmentSizeX = segmentMax.x - segmentMin.x;
		const float segmentSizeY = segmentMax.y - segmentMin.y;
		const float segmentSize = segmentSizeX < segmentSizeY ? segmentSizeY : segmentSizeX;
		const vec2 offsetBottomLeft(segmentMin.x - frustumMin.x, segmentMin.y - frustumMin.y);
		const vec2 offsetSegmentSizeRatio(offsetBottomLeft.x / segmentSize, offsetBottomLeft.y / segmentSize);
		const vec2 frustumSegmentSizeRatio(frustumSize.x / segmentSize, frustumSize.y / segmentSize);
		
		// NEXT CODE IS ONLY NEEDED FOR MULTI_VIEWPORT RENDERING !
		
		const int LIGHT_TEXTURE_SIZE = 512;
		vec2 pixelOffsetTopLeft( (nv_scalar) LIGHT_TEXTURE_SIZE * offsetSegmentSizeRatio);
		vec2 pixelFrustumSize( (nv_scalar) LIGHT_TEXTURE_SIZE * frustumSegmentSizeRatio);

		// Scale factor that helps if frustum size is supposed to be bigger
		// than maximum viewport size.
		
		float viewportDims[2];
		glGetFloatv( GL_MAX_VIEWPORT_DIMS, &viewportDims[0] );

		vec2 scaleFactor(
			viewportDims[0] < pixelFrustumSize.x ? viewportDims[0] / pixelFrustumSize.x : 1.0f,
			viewportDims[1] < pixelFrustumSize.y ? viewportDims[1] / pixelFrustumSize.y : 1.0f );
			
		pixelOffsetTopLeft = pixelOffsetTopLeft * scaleFactor;
		pixelFrustumSize = pixelFrustumSize * scaleFactor;

		mLightViewports[i] = vec4(-pixelOffsetTopLeft.x, -pixelOffsetTopLeft.y, pixelFrustumSize.x, pixelFrustumSize.y);
		glViewportIndexedfv(i, mLightViewports[i].vec_array);
		/*
		const float scaleX = 2.0f / (segmentMax.x - segmentMin.x);
		const float scaleY = 2.0f / (segmentMax.y - segmentMin.y);
		const float offsetX = -0.5f * (segmentMax.x + segmentMin.x) * scaleX;
		const float offsetY = -0.5f * (segmentMax.y + segmentMin.y) * scaleY;

		mat4 viewportMatrix;
		viewportMatrix.identity();
		viewportMatrix(0,0) = scaleX;
		viewportMatrix(1,1) = scaleY;
		viewportMatrix(0,3) = offsetX;
		viewportMatrix(1,3) = offsetY;
		transpose(viewportMatrix);
		*/
		// update light view-projection matrices per segment.
		mat4 lightProj;
		lightProj.identity();
		ortho(lightProj, segmentMin.x, segmentMin.x + segmentSize, segmentMin.y, segmentMin.y + segmentSize, 0.0f, frustumMin.z);
		mat4 lightScale;
		lightScale.identity();
		lightScale.set_scale( vec3(0.5f * scaleFactor.x, 0.5f * scaleFactor.y, 0.5f) );
		mat4 lightBias;
		lightBias.identity();
		lightBias.set_translation( vec3(0.5f * scaleFactor.x, 0.5f * scaleFactor.y, 0.5f) );
		mLightSegmentVPSBMatrices[i] = lightBias * lightScale * lightProj * lightViewMatrix;

		nearSegmentPlane = mNormalizedFarPlanes[i];
	}
	/*
	// Set remaining viewports to some kind of standard state.
	for (unsigned int i = frustumSegmentCount; i<MAX_CAMERA_FRUSTUM_SPLIT_COUNT; ++i)
		glViewportIndexedf(i, 0, 0, LIGHT_TEXTURE_SIZE, LIGHT_TEXTURE_SIZE);
		*/
}

void CGPULightsManager::BindLightViewports()
{
	for (int i=0; i<4; ++i)
		glViewportIndexedfv(i, mLightViewports[i].vec_array);
}

bool CGPULightsManager::ComputeLightMatrices( const LightDATA &lightData, mat4& pLightView, mat4& pLightProj, mat4 &lightInvTM )
{
	bool status = false;
	if( lightData.type == LIGHT_TYPE_SPOT )
	{
		status = ComputeSpotLightMatrices( lightData, pLightView, pLightProj, lightInvTM );
	}
	else if( lightData.type == LIGHT_TYPE_DIRECTION )
	{
		status = ComputeInfiniteLightMatrices( lightData, pLightView, pLightProj, lightInvTM );
	}
	return status;
}

bool CGPULightsManager::ComputeSpotLightMatrices( const LightDATA &lightData, mat4 &pLightView, mat4 &pLightProj, mat4 &lightInvTM )
{
	if( lightData.type != LIGHT_TYPE_SPOT )
	{
		return false;
	}

	pLightView.identity();
	pLightProj.identity();

	// Spotlight support

	// Get all the information necessary to setup the lighting matrix
	// Will need to create a MODELVIEWPROJ matrix using:
	//		- Transformation matrix of light
	//		- Custom projection matrix based on light

	// We need a base matrix because the transformation matrix doesn't take into account that lights
	// start out with transforms that are not represented ... grr ...

	float base[16]	=
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
	mat4 baseMat( base );

	// Factor out the scale, because we don't want it ...
	//FBVector3d v;
	/*
	FBVector3d scl;
	FBMatrix rotationMat;
	FBMatrix transformationMat;
	pLight->GetMatrix( rotationMat, kModelRotation, true );
	FBMatrixMult( transformationMat, rotationMat, baseMat );
	pLight->GetVector(scl, kModelScaling);
	*/
	look_at( pLightView, lightData.position, lightData.dir, vec3(0.0f, 1.0f, 0.0f) );
	invert(lightInvTM, pLightView);
	//pLight->GetVector( v );
	/*
	//transformationMat.Identity();
	transformationMat(3,0) = ((FBVector3d)pLight->Translation)[0];
	transformationMat(3,1) = ((FBVector3d)pLight->Translation)[1];
	transformationMat(3,2) = ((FBVector3d)pLight->Translation)[2];
	transformationMat(3,3) = 1.0f;

	FBMatrixInverse( pLightView, transformationMat );
	//pLight->GetMatrix( pLightView, kModelTransformation );
	
	pLight->GetMatrix( lightInvTM, kModelInverse_Transformation );
	*/
	// Ok .. now we just need a projection matrix ...
	float fov = 1.2f * lightData.spotAngle / 2.0f;
	float fFar = lightData.radius * 2.0f; // scl[1] 
	float fNear = 1.0f;
	float top = tan(fov*3.14159f/360.0f) * fNear;
	float bottom = -top;
	float left = bottom;
	float right = top;
	double perspectiveValues[16] =
        {
            (2.0*fNear)/(right-left),   0,                          0,                          0,
            0,                         (2.0*fNear)/(top-bottom),    0,                          0,
            0,                         0,                           -(fFar+fNear)/(fFar-fNear), -(2.0f*fFar*fNear)/(fFar-fNear),
            0,                         0,                           -1.0f,                      0
        };

	

	perspective( pLightProj, lightData.spotAngle, 1.0, fNear, fFar );

	return true;
}

bool CGPULightsManager::ComputeInfiniteLightMatrices( const LightDATA &lightData, mat4 &pLightView, mat4 &pLightProj, mat4 &lightInvTM )
{
	if( lightData.type != LIGHT_TYPE_DIRECTION )
	{
		return false;
	}

	pLightView.identity();
	pLightProj.identity();

	// Directional light support

	// Get all the information necessary to setup the lighting matrix
	// Will need to create a MODELVIEWPROJ matrix using:
	//		- Transformation matrix of light
	//		- Custom projection matrix based on light

	// We need a base matrix because the transformation matrix doesn't take into account that lights
	// start out with transforms that are not represented ... grr ...
	float base[16]	=
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
	mat4 baseMat( base );
	/*
	// Factor out the scale, because we don't want it ...
	FBMatrix rotationMat;
	FBMatrix transformationMat;
	pLight->GetMatrix( rotationMat, kModelRotation, true );
	FBMatrixMult( transformationMat, rotationMat, baseMat );

	double radius;
	FBVector4d newPos;
	GetWorldBounds( pLight, radius, newPos );

	transformationMat(3,0) = newPos[0];
	transformationMat(3,1) = newPos[1];
	transformationMat(3,2) = newPos[2];
	transformationMat(3,3) = 1.0f;

	FBMatrixInverse( pLightView, transformationMat );

	pLight->GetMatrix( lightInvTM, kModelInverse_Transformation );
	*/

	look_at( pLightView, -lightData.dir, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f) );
	invert( lightInvTM, pLightView );
	float radius = lightData.radius;
	

	// Ok .. now we just need a projection matrix ...
	float left		= -radius;
	float right	=  radius;
	float top		=  radius;
	float bottom	= -radius;

	float fNear	=  0.0f;
	float fFar		=  radius * 2.0f;

	float diffRL	= 1.0f / (right - left);
	float diffTB	= 1.0f / (top - bottom);
	float diffFN	= 1.0f / (fFar - fNear);

	float orthoValues[16] =
        {
            2.0f * diffRL,	0.0f,			0.0f,                       0.0f,
            0.0f,           2.0f * diffTB,	0.0f,                       0.0f,
            0.0f,           0.0f,           -2.0f * diffFN,				0.0f,
            0.0f,           0.0f,           -(fFar + fNear) * diffFN,	1.0f
        };

	pLightProj = orthoValues;

	return true;
}


void CGPULightsManager::GetWorldBounds( const LightDATA &lightData, double& pRadius, vec4& pPos )
{
	// TODO:
	/*
	// Default light position is pointing down the -Y axis
	float lBaseLightDir[4] = { 0.0f, -1.0f, 0.0f, 0.0f };
	vec4 lDir( lBaseLightDir );
	
	pLight->GetMatrix( rotationMat, kModelRotation, true );

	// Compute transformation matrix.
	double base[16]	=
        {
            1.0, 0.0, 0.0, 0.0,
            0.0, 0.0, -1.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        };
	FBMatrix baseMat( base );
	FBMatrix transformMat, invTransformMat;
	FBMatrixMult( transformMat, rotationMat, baseMat );
	FBMatrixInverse( invTransformMat, transformMat );

	// Compute bounds in lightView space.
	FBVector4d lBounds[2];
	FBVectorMatrixMult( lBounds[0], invTransformMat, mWorldMin );
	FBVectorMatrixMult( lBounds[1], invTransformMat, mWorldMax );
	double dblMin[4] = { DBL_MIN, DBL_MIN, DBL_MIN, 1.0 };
	double dblMax[4] = { DBL_MAX, DBL_MAX, DBL_MAX, 1.0 };
	FBVector4d lLightMin, lLightMax;
	lLightMin.Set( dblMax );
	lLightMax.Set( dblMin );
	for( int i = 0; i < 3; i++ )
	{
		if( lBounds[0][i] < lLightMin[i] ) { lLightMin[i] = lBounds[0][i]; }
		if( lBounds[1][i] < lLightMin[i] ) { lLightMin[i] = lBounds[1][i]; }
		if( lBounds[0][i] > lLightMax[i] ) { lLightMax[i] = lBounds[0][i]; }
		if( lBounds[1][i] > lLightMax[i] ) { lLightMax[i] = lBounds[1][i]; }
	}

	// Compute the box center (in world space)
	FBVector4d center;
	center[0] = 0.5 * (lLightMin[0] + lLightMax[0]);
	center[1] = 0.5 * (lLightMin[1] + lLightMax[1]);
	center[2] = 0.5 * (lLightMin[2] + lLightMax[2]);
	center[3] = 1.0;
	FBVectorMatrixMult( center, transformMat, center );

	double w = lLightMax[0] - lLightMin[0];
	double h = lLightMax[1] - lLightMin[1];
	double d = lLightMax[2] - lLightMin[2];

	// Expand the radius by 10%
	double sceneRadius = 0.5 * std::max( w, std::max(h, d) );
	sceneRadius *= 1.1f;

	FBVectorMatrixMult( lDir, rotationMat, lDir );
	lDir[0] *= sceneRadius;
	lDir[1] *= sceneRadius;
	lDir[2] *= sceneRadius;

	FBVector4d newPos;
	newPos[0] = center[0] - lDir[0];
	newPos[1] = center[1] - lDir[1];
	newPos[2] = center[2] - lDir[2];
	newPos[3] = 1.0;

	// Return values
	pRadius = sceneRadius;
	pPos = newPos;
	*/
}

void CGPULightsManager::PostRenderingOverviewCam(const int width, const int height)
{
	const float FAR_DIST = 4000.0;

	glViewport(width - 129, 0, 128, 128);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)width/(double)height, mFrustums[0].neard, mFrustums[mFrustumSegmentCount-1].fard);

	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glDisable(GL_LIGHTING);
	glPointSize(10);
	glColor3f(1.0f, 1.0f, 0.0f);
	gluLookAt(0, FAR_DIST/2, 0, 0, 0, 0, 0, 0, 1.0f);

	glPushMatrix();
	glScalef(0.2f, 0.2f, 0.2f);
	glRotatef(20, 1, 0, 0);
	for(int i=0; i<mFrustumSegmentCount; i++)
	{
		glBegin(GL_LINE_LOOP);
		for(int j=0; j<4; j++)
			glVertex3f(mFrustums[i].point[j].x, mFrustums[i].point[j].y, mFrustums[i].point[j].z);

		glEnd();
		glBegin(GL_LINE_LOOP);
		for(int j=4; j<8; j++)
			glVertex3f(mFrustums[i].point[j].x, mFrustums[i].point[j].y, mFrustums[i].point[j].z);
		glEnd();
	}
	for(int j=0; j<4; j++)
	{
		glBegin(GL_LINE_STRIP);
		glVertex3fv(mCameraPos.vec_array);
		for(int i=0; i<mFrustumSegmentCount; i++)
			glVertex3f(mFrustums[i].point[j].x, mFrustums[i].point[j].y, mFrustums[i].point[j].z);
		glVertex3f(mFrustums[mFrustumSegmentCount-1].point[j+4].x, mFrustums[mFrustumSegmentCount-1].point[j+4].y, mFrustums[mFrustumSegmentCount-1].point[j+4].z);
		glEnd();
	}
	glPopMatrix();

	//glTranslatef( mCameraPos.x, mCameraPos.y, mCameraPos.z );
	glLineWidth(4.0f);
	glScalef(1000.0f, 1000.0f, 1000.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, .0f, .0f);
	glVertex3f(.0f, .0f, .0f);
	glVertex3fv(mCameraRight.vec_array);
	glColor3f(.0f, 1.0f, .0f);
	glVertex3f(.0f, .0f, .0f);
	glVertex3fv(mCameraUp.vec_array);
	glColor3f(.0f, .0f, 1.0f);
	glVertex3f(.0f, .0f, .0f);
	glVertex3fv(mCameraDir.vec_array);

	glEnd();

	glLineWidth(1.0f);
}

const int CGPULightsManager::GetNumberOfDirLights() const 
{
	return mNumberOfDirLights;
}

const int CGPULightsManager::GetNumberOfPointLights() const 
{
	return mNumberOfLights;
}

void CGPULightsManager::SetDebugDisplay(const bool value) 
{
	mDebugDisplay = value;
}

const size_t CGPULightsManager::GetNumberOfShadowCasters() const
{
	return mLightCasters.size();
}

void CGPULightsManager::BindLightMatrices(const GLuint attribIndex) const
{
	mLightMatrices.BindAsAttribute( attribIndex, 0 );
}

mat4 &CGPULightsManager::GetLightViewMatrix(const int index)
{
	return mLightViewMatrix[index];
}
mat4 &CGPULightsManager::GetLightProjMatrix(const int index)
{
	return mLightProjMatrix[index];
}
mat4 &CGPULightsManager::GetLightInvTM(const int index)
{
	return mLightInvTM[index];
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FBShaderLights

CGPUShaderLights::CGPUShaderLights()
{
	//mNumberOfLights = 0;
	//mNumberOfDirLights = 0;
}

CGPUShaderLights::~CGPUShaderLights()
{
}

void CGPUShaderLights::UpdateTransformedLights(const mat4 &modelview, const mat4 &rotation)
{
	mTransformedLights.resize( mLights.size() );
	mTransformedDirLights.resize( mDirLights.size() );

	auto srcLight = begin(mLights);
	auto dstLight = begin(mTransformedLights);

	for ( ; srcLight != end(mLights); ++srcLight, ++dstLight)
	{
		dstLight->position = modelview * srcLight->position;
		dstLight->dir = rotation * srcLight->dir;

		dstLight->type = srcLight->type;
		dstLight->spotAngle = srcLight->spotAngle;

		dstLight->color = srcLight->color;
		dstLight->radius = srcLight->radius;

		dstLight->attenuations = srcLight->attenuations;
	}

	auto srcDirLight = begin(mDirLights);
	auto dstDirLight = begin(mTransformedDirLights);

	for ( ; srcDirLight != end(mDirLights); ++srcDirLight, ++dstDirLight)
	{
		dstDirLight->position = srcDirLight->position;
		dstDirLight->dir = rotation * srcDirLight->dir;

		dstDirLight->type = srcDirLight->type;
		dstDirLight->spotAngle = srcDirLight->spotAngle;

		dstDirLight->color = srcDirLight->color;
		dstDirLight->radius = srcDirLight->radius;

		dstDirLight->attenuations = srcDirLight->attenuations;
	}
}

void CGPUShaderLights::MapOnGPU()
{
	// dir lights
	mBufferDirLights.UpdateData( sizeof(LightDATA), mTransformedDirLights.size(), mTransformedDirLights.data() );
	
	// point / spot lights
	mBufferLights.UpdateData( sizeof(LightDATA), mTransformedLights.size(), mTransformedLights.data() );

	//
	//mNumberOfDirLights = (int) mTransformedDirLights.size();
	//mNumberOfLights = (int) mTransformedLights.size();
}



void CGPUShaderLights::PrepGPUPtr()
{
	/*
	// dir lights
	if ( mBufferDirLights.GetCount() > 0 )
		mBufferDirLights.UpdateGPUPtr();
	
	// point / spot
	if ( mBufferLights.GetCount() > 0 )
		mBufferLights.UpdateGPUPtr();
	
	mNumberOfDirLights = (int) mBufferDirLights.GetCount();
	mNumberOfLights = (int) mBufferLights.GetCount();
	*/

	
}

/*
void CGPUShaderLights::Build(CCameraInfoCache &cameraCache, std::vector<FBLight*> &lights)
{
	if (lights.size() == 0)
	{
		mDirLights.clear();
		mLights.clear();
	
		return;
	}

	FBMatrix pCamMatrix(cameraCache.mv);
	//pCamera->GetCameraMatrix( pCamMatrix, kFBModelView );
	
	FBMatrix lViewMatrix( pCamMatrix );
	
    FBRVector lViewRotation;
    FBMatrixToRotation(lViewRotation, lViewMatrix);

    FBMatrix lViewRotationMatrix;
    FBRotationToMatrix(lViewRotationMatrix, lViewRotation);

	//
	const int numLights = (int) lights.size();

	// process scene lights and enable clustering if some point/spot light exist

	int numDirLights = 0;
	int numPointLights = 0;
	//int numLightCasters = 0;

	for (int i=0; i<numLights; ++i)
	{
		FBLight *pLight = lights[i];

		if (pLight->CastLightOnObject)
		{
			if (pLight->LightType == kFBLightTypeInfinite) numDirLights++;
			else numPointLights++;
		}
			
	}


	mDirLights.resize(numDirLights);
	mLights.resize(numPointLights);
	
	numDirLights = 0;
	numPointLights = 0;
	//numLightCasters = 0;

	for (int i=0; i<numLights; ++i)
	{
		FBLight *pLight = lights[i];
		LightDATA *pLightData = nullptr;

		if (pLight->CastLightOnObject)
		{
			if (pLight->LightType != kFBLightTypeInfinite)
			{
				LightDATA::ConstructFromFBLight( false, lViewMatrix, lViewRotationMatrix, pLight, mLights[numPointLights] );
				pLightData = &mLights[numPointLights];
				numPointLights++;
			}
			else
			{
				LightDATA::ConstructFromFBLight( true, lViewMatrix, lViewRotationMatrix, pLight, mDirLights[numDirLights] );
				pLightData = &mDirLights[numDirLights];
				numDirLights++;
			}
		}
			
	}
	
	//
	//
	
	mat4 modelrotation;

	for (int i=0; i<16; ++i)
	{
		modelrotation.mat_array[i] = (float) lViewRotationMatrix[i];
	}

	for (size_t i=0; i<mLights.size(); ++i)
	{
		mLights[i].position = cameraCache.mv4 * mLights[i].position;
		mLights[i].dir = modelrotation * mLights[i].dir;
	}
}
*/
void CGPUShaderLights::Bind(const GLuint programId, const GLuint dirLightsLoc, const GLuint lightsLoc) const
{
	// bind dir lights uniforms
	if (programId > 0)
	{
		mBufferDirLights.BindAsUniform( programId, dirLightsLoc, 0 );
		mBufferLights.BindAsUniform( programId, lightsLoc, 0 );	
	}
}

void CGPUShaderLights::UnBind() const
{
}