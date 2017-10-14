
#pragma once

/*
	Author Sergey Solokhin (Neill3d)

    GitHub page - https://github.com/Neill3d/MoPlugs_Framework
	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
*/

#include "nv_math.h"


//
//

struct BSphere
{
	vec3		center;
	float		radius;
};

void computeFrustumBSphere(	const float nearPlane, 
							const float farPlane, 
							const float Fov, 
							const float fWidth2, 
							const float fHeight2, 
							const mat4 &modelview, 
							const vec3 &eyepos, 
							BSphere &bb);


//
class CFrustum
{
public:

	void NormalizePlane(float ppf_frustum[6][4], int i_side);
	void CalculateFrustum(const double *projection, const double *modelview);
	void CalculateFrustum(const float *projection, const float *modelview);

	bool PointInFrustum( float f_x, float f_y, float f_z );
	bool SphereInFrustum( float f_x, float f_y, float f_z, float f_radius ) const;
	bool BoxInFrustum( float f_min_x, float f_min_y, float f_min_z, float f_max_x, float f_max_y, float f_max_z);

	const float *GetFrustumPlane(const int index)
	{
		return &m_ppfFrustum[index][0];
	}

private:
	float m_ppfFrustum[6][4];
};