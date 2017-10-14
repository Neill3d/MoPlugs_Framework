
/*
	Sergey Solokhin (Neill3d)
		
	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE

*/

#include "math3d.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void computeFrustumBSphere(	const float nearPlane, 
							const float farPlane, 
							const float Fov, 
							const float fWidth2, 
							const float fHeight2, 
							const mat4 &modelview, 
							const vec3 &eyepos, 
							BSphere &bb )
{
	// get the look vector of the camera from the view matrix
	vec3 right(modelview.mat_array[0], modelview.mat_array[4], modelview.mat_array[8]);
	vec3 up(modelview.mat_array[1], modelview.mat_array[5], modelview.mat_array[9]);
	vec3 d(modelview.mat_array[2], modelview.mat_array[6], modelview.mat_array[10]);
	
	const float ratio = fWidth2 / fHeight2;
	
	float Hnear = 2.0f * tan( nv_to_rad * Fov * 0.5f) * nearPlane;
	float  Wnear = Hnear * ratio;

	float Hfar = 2.0f * tan( nv_to_rad * Fov * 0.5f) * farPlane;
	float Wfar = Hfar * ratio;

	normalize(d);
	normalize(up);
	normalize(right);
	
	scale(d, -1.0);
	
	vec3 fc, ftl;
	add(fc, eyepos, vec3(d[0]*farPlane, d[1]*farPlane, d[2]*farPlane) );
	add(ftl, fc, vec3(0.5f*up[0]*Hfar, 0.5f*up[1]*Hfar, 0.5f*up[2]*Hfar) );
	sub(ftl, ftl, vec3(0.5f*right[0]*Wfar, 0.5f*right[1]*Wfar, 0.5f*right[2]*Wfar));

	// calculate the center of the sphere
	add(bb.center, eyepos, vec3(0.5f*d[0]*farPlane, 0.5f*d[1]*farPlane, 0.5f*d[2]*farPlane));

	// the vector between P and Q
	vec3 vDiff;
	sub(vDiff, ftl, bb.center);

	// the radius becomes the length of this vector
	bb.radius = vDiff.sq_norm();	
}

/////////////////////////////////////////////////////////////

// We create an enum of the sides so we don't have to call each side 0 or 1.
// This way it makes it more understandable and readable when dealing with frustum sides.
// Definuj hodnoty šesti stran pomyslného komolého jehlanu zorného pole kamery. 
enum FrustumSide
{
	// The RIGHT side of the frustum
	// Pravá strana zorného pole kamery
	RIGHT,
	
	// The LEFT	 side of the frustum
	// Levá strana zorného pole kamery
	LEFT,
	
	// The BOTTOM side of the frustum
	// Spodní strana zorného pole kamery
	BOTTOM,
	
	// The TOP side of the frustum
	// Horní strana zorného pole kamery
	TOP,
	
	// The BACK	side of the frustum
	// Zadní strana zorného pole kamery
	BACK,	
	
	// The FRONT side of the frustum
	// Pøední strana zorného pole kamery
	FRONT			
}; 

//----------------------------------------------------------------------------------------------------//

// Like above, instead of saying a number for the ABC and D of the plane, we
// want to be more descriptive.
// Definuj hodnoty (A B C) normálového vektoru  plochy stran zorného pole kamera a ( D ) vzdálenost  
// bodu  od plochy kdy dochází k detekci.
enum PlaneData
{
	// The X value of the plane's normal
	// X hodnota normály plochy
	A,				
	
	// The Y value of the plane's normal
	// Y hodnota normály plochy
	B,

	// The Z value of the plane's normal
	// Z hodnota normály plochy
	C,
	
	// The distance the plane is from the origin
	// Vzdálenost bodu  od plochy kdy dochází k detekci
	D				
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
//								  NormalizePlane
//////////////////////////////////////////////////////////////////////////////////////////////////////// 

// This normalizes a plane (A side) from a given frustum.
void CFrustum::NormalizePlane(float ppf_frustum[6][4], int i_side)
{
	// Here we calculate the magnitude of the normal to the plane (point A B C)
	// Remember that (A, B, C) is that same thing as the normal's (X, Y, Z).
	// To calculate magnitude you use the equation:  magnitude = sqrt( x^2 + y^2 + z^2)
	//
	
	float i_magnitude = (float)sqrt( ppf_frustum[i_side][A] * ppf_frustum[i_side][A] + 
									 ppf_frustum[i_side][B] * ppf_frustum[i_side][B] + 
								     ppf_frustum[i_side][C] * ppf_frustum[i_side][C] );

	// Then we divide the plane's values by it's magnitude.
	// This makes it easier to work with.
	
	ppf_frustum[i_side][A] /= i_magnitude;
	ppf_frustum[i_side][B] /= i_magnitude;
	ppf_frustum[i_side][C] /= i_magnitude;
	ppf_frustum[i_side][D] /= i_magnitude; 
}


void CFrustum::CalculateFrustum(const double *pf_proj, const double *pf_modl)
{    
	// This will hold our projection matrix
	
	// This will hold the clipping planes
	
	double   pf_clip[16];								

	// Now that we have our modelview and projection matrix, if we combine these 2 matrices,
	// it will give us our clipping planes.  To combine 2 matrices, we multiply them.


	pf_clip[ 0] = pf_modl[ 0] * pf_proj[ 0] + pf_modl[ 1] * pf_proj[ 4] + pf_modl[ 2] * pf_proj[ 8] + pf_modl[ 3] * pf_proj[12];
	pf_clip[ 1] = pf_modl[ 0] * pf_proj[ 1] + pf_modl[ 1] * pf_proj[ 5] + pf_modl[ 2] * pf_proj[ 9] + pf_modl[ 3] * pf_proj[13];
	pf_clip[ 2] = pf_modl[ 0] * pf_proj[ 2] + pf_modl[ 1] * pf_proj[ 6] + pf_modl[ 2] * pf_proj[10] + pf_modl[ 3] * pf_proj[14];
	pf_clip[ 3] = pf_modl[ 0] * pf_proj[ 3] + pf_modl[ 1] * pf_proj[ 7] + pf_modl[ 2] * pf_proj[11] + pf_modl[ 3] * pf_proj[15];

	pf_clip[ 4] = pf_modl[ 4] * pf_proj[ 0] + pf_modl[ 5] * pf_proj[ 4] + pf_modl[ 6] * pf_proj[ 8] + pf_modl[ 7] * pf_proj[12];
	pf_clip[ 5] = pf_modl[ 4] * pf_proj[ 1] + pf_modl[ 5] * pf_proj[ 5] + pf_modl[ 6] * pf_proj[ 9] + pf_modl[ 7] * pf_proj[13];
	pf_clip[ 6] = pf_modl[ 4] * pf_proj[ 2] + pf_modl[ 5] * pf_proj[ 6] + pf_modl[ 6] * pf_proj[10] + pf_modl[ 7] * pf_proj[14];
	pf_clip[ 7] = pf_modl[ 4] * pf_proj[ 3] + pf_modl[ 5] * pf_proj[ 7] + pf_modl[ 6] * pf_proj[11] + pf_modl[ 7] * pf_proj[15];

	pf_clip[ 8] = pf_modl[ 8] * pf_proj[ 0] + pf_modl[ 9] * pf_proj[ 4] + pf_modl[10] * pf_proj[ 8] + pf_modl[11] * pf_proj[12];
	pf_clip[ 9] = pf_modl[ 8] * pf_proj[ 1] + pf_modl[ 9] * pf_proj[ 5] + pf_modl[10] * pf_proj[ 9] + pf_modl[11] * pf_proj[13];
	pf_clip[10] = pf_modl[ 8] * pf_proj[ 2] + pf_modl[ 9] * pf_proj[ 6] + pf_modl[10] * pf_proj[10] + pf_modl[11] * pf_proj[14];
	pf_clip[11] = pf_modl[ 8] * pf_proj[ 3] + pf_modl[ 9] * pf_proj[ 7] + pf_modl[10] * pf_proj[11] + pf_modl[11] * pf_proj[15];

	pf_clip[12] = pf_modl[12] * pf_proj[ 0] + pf_modl[13] * pf_proj[ 4] + pf_modl[14] * pf_proj[ 8] + pf_modl[15] * pf_proj[12];
	pf_clip[13] = pf_modl[12] * pf_proj[ 1] + pf_modl[13] * pf_proj[ 5] + pf_modl[14] * pf_proj[ 9] + pf_modl[15] * pf_proj[13];
	pf_clip[14] = pf_modl[12] * pf_proj[ 2] + pf_modl[13] * pf_proj[ 6] + pf_modl[14] * pf_proj[10] + pf_modl[15] * pf_proj[14];
	pf_clip[15] = pf_modl[12] * pf_proj[ 3] + pf_modl[13] * pf_proj[ 7] + pf_modl[14] * pf_proj[11] + pf_modl[15] * pf_proj[15];
	
	// Now we actually want to get the sides of the frustum.  To do this we take
	// the clipping planes we received above and extract the sides from them.
	
	// This will extract the RIGHT side of the frustum
	
	m_ppfFrustum[RIGHT][A] = (float) (pf_clip[ 3] - pf_clip[ 0]);
	m_ppfFrustum[RIGHT][B] = (float) (pf_clip[ 7] - pf_clip[ 4]);
	m_ppfFrustum[RIGHT][C] = (float) (pf_clip[11] - pf_clip[ 8]);
	m_ppfFrustum[RIGHT][D] = (float) (pf_clip[15] - pf_clip[12]);

	// Now that we have a normal (A,B,C) and a distance (D) to the plane,
	// we want to normalize that normal and distance.
	
	// Normalize the RIGHT side
	NormalizePlane(m_ppfFrustum, RIGHT);

	// This will extract the LEFT side of the frustum
	m_ppfFrustum[LEFT][A] = (float) (pf_clip[ 3] + pf_clip[ 0]);
	m_ppfFrustum[LEFT][B] = (float) (pf_clip[ 7] + pf_clip[ 4]);
	m_ppfFrustum[LEFT][C] = (float) (pf_clip[11] + pf_clip[ 8]);
	m_ppfFrustum[LEFT][D] = (float) (pf_clip[15] + pf_clip[12]);

	// Normalize the LEFT side
	NormalizePlane(m_ppfFrustum, LEFT);

	// This will extract the BOTTOM side of the frustum
	m_ppfFrustum[BOTTOM][A] = (float) (pf_clip[ 3] + pf_clip[ 1]);
	m_ppfFrustum[BOTTOM][B] = (float) (pf_clip[ 7] + pf_clip[ 5]);
	m_ppfFrustum[BOTTOM][C] = (float) (pf_clip[11] + pf_clip[ 9]);
	m_ppfFrustum[BOTTOM][D] = (float) (pf_clip[15] + pf_clip[13]);

	// Normalize the BOTTOM side
	NormalizePlane(m_ppfFrustum, BOTTOM);

	// This will extract the TOP side of the frustum
	m_ppfFrustum[TOP][A] = (float) (pf_clip[ 3] - pf_clip[ 1]);
	m_ppfFrustum[TOP][B] = (float) (pf_clip[ 7] - pf_clip[ 5]);
	m_ppfFrustum[TOP][C] = (float) (pf_clip[11] - pf_clip[ 9]);
	m_ppfFrustum[TOP][D] = (float) (pf_clip[15] - pf_clip[13]);

	// Normalize the TOP side
	NormalizePlane(m_ppfFrustum, TOP);

	// This will extract the BACK side of the frustum
	m_ppfFrustum[BACK][A] = (float) (pf_clip[ 3] - pf_clip[ 2]);
	m_ppfFrustum[BACK][B] = (float) (pf_clip[ 7] - pf_clip[ 6]);
	m_ppfFrustum[BACK][C] = (float) (pf_clip[11] - pf_clip[10]);
	m_ppfFrustum[BACK][D] = (float) (pf_clip[15] - pf_clip[14]);

	// Normalize the BACK side
	NormalizePlane(m_ppfFrustum, BACK);

	// This will extract the FRONT side of the frustum
	m_ppfFrustum[FRONT][A] = (float) (pf_clip[ 3] + pf_clip[ 2]);
	m_ppfFrustum[FRONT][B] = (float) (pf_clip[ 7] + pf_clip[ 6]);
	m_ppfFrustum[FRONT][C] = (float) (pf_clip[11] + pf_clip[10]);
	m_ppfFrustum[FRONT][D] = (float) (pf_clip[15] + pf_clip[14]);

	// Normalize the FRONT side
	NormalizePlane(m_ppfFrustum, FRONT);
}

void CFrustum::CalculateFrustum(const float *pf_proj, const float *pf_modl)
{    
	// This will hold our projection matrix
	
	// This will hold the clipping planes
	
	float   pf_clip[16];								

	// Now that we have our modelview and projection matrix, if we combine these 2 matrices,
	// it will give us our clipping planes.  To combine 2 matrices, we multiply them.


	pf_clip[ 0] = pf_modl[ 0] * pf_proj[ 0] + pf_modl[ 1] * pf_proj[ 4] + pf_modl[ 2] * pf_proj[ 8] + pf_modl[ 3] * pf_proj[12];
	pf_clip[ 1] = pf_modl[ 0] * pf_proj[ 1] + pf_modl[ 1] * pf_proj[ 5] + pf_modl[ 2] * pf_proj[ 9] + pf_modl[ 3] * pf_proj[13];
	pf_clip[ 2] = pf_modl[ 0] * pf_proj[ 2] + pf_modl[ 1] * pf_proj[ 6] + pf_modl[ 2] * pf_proj[10] + pf_modl[ 3] * pf_proj[14];
	pf_clip[ 3] = pf_modl[ 0] * pf_proj[ 3] + pf_modl[ 1] * pf_proj[ 7] + pf_modl[ 2] * pf_proj[11] + pf_modl[ 3] * pf_proj[15];

	pf_clip[ 4] = pf_modl[ 4] * pf_proj[ 0] + pf_modl[ 5] * pf_proj[ 4] + pf_modl[ 6] * pf_proj[ 8] + pf_modl[ 7] * pf_proj[12];
	pf_clip[ 5] = pf_modl[ 4] * pf_proj[ 1] + pf_modl[ 5] * pf_proj[ 5] + pf_modl[ 6] * pf_proj[ 9] + pf_modl[ 7] * pf_proj[13];
	pf_clip[ 6] = pf_modl[ 4] * pf_proj[ 2] + pf_modl[ 5] * pf_proj[ 6] + pf_modl[ 6] * pf_proj[10] + pf_modl[ 7] * pf_proj[14];
	pf_clip[ 7] = pf_modl[ 4] * pf_proj[ 3] + pf_modl[ 5] * pf_proj[ 7] + pf_modl[ 6] * pf_proj[11] + pf_modl[ 7] * pf_proj[15];

	pf_clip[ 8] = pf_modl[ 8] * pf_proj[ 0] + pf_modl[ 9] * pf_proj[ 4] + pf_modl[10] * pf_proj[ 8] + pf_modl[11] * pf_proj[12];
	pf_clip[ 9] = pf_modl[ 8] * pf_proj[ 1] + pf_modl[ 9] * pf_proj[ 5] + pf_modl[10] * pf_proj[ 9] + pf_modl[11] * pf_proj[13];
	pf_clip[10] = pf_modl[ 8] * pf_proj[ 2] + pf_modl[ 9] * pf_proj[ 6] + pf_modl[10] * pf_proj[10] + pf_modl[11] * pf_proj[14];
	pf_clip[11] = pf_modl[ 8] * pf_proj[ 3] + pf_modl[ 9] * pf_proj[ 7] + pf_modl[10] * pf_proj[11] + pf_modl[11] * pf_proj[15];

	pf_clip[12] = pf_modl[12] * pf_proj[ 0] + pf_modl[13] * pf_proj[ 4] + pf_modl[14] * pf_proj[ 8] + pf_modl[15] * pf_proj[12];
	pf_clip[13] = pf_modl[12] * pf_proj[ 1] + pf_modl[13] * pf_proj[ 5] + pf_modl[14] * pf_proj[ 9] + pf_modl[15] * pf_proj[13];
	pf_clip[14] = pf_modl[12] * pf_proj[ 2] + pf_modl[13] * pf_proj[ 6] + pf_modl[14] * pf_proj[10] + pf_modl[15] * pf_proj[14];
	pf_clip[15] = pf_modl[12] * pf_proj[ 3] + pf_modl[13] * pf_proj[ 7] + pf_modl[14] * pf_proj[11] + pf_modl[15] * pf_proj[15];
	
	// Now we actually want to get the sides of the frustum.  To do this we take
	// the clipping planes we received above and extract the sides from them.
	
	// This will extract the RIGHT side of the frustum
	
	m_ppfFrustum[RIGHT][A] = (float) (pf_clip[ 3] - pf_clip[ 0]);
	m_ppfFrustum[RIGHT][B] = (float) (pf_clip[ 7] - pf_clip[ 4]);
	m_ppfFrustum[RIGHT][C] = (float) (pf_clip[11] - pf_clip[ 8]);
	m_ppfFrustum[RIGHT][D] = (float) (pf_clip[15] - pf_clip[12]);

	// Now that we have a normal (A,B,C) and a distance (D) to the plane,
	// we want to normalize that normal and distance.
	
	// Normalize the RIGHT side
	NormalizePlane(m_ppfFrustum, RIGHT);

	// This will extract the LEFT side of the frustum
	m_ppfFrustum[LEFT][A] = (float) (pf_clip[ 3] + pf_clip[ 0]);
	m_ppfFrustum[LEFT][B] = (float) (pf_clip[ 7] + pf_clip[ 4]);
	m_ppfFrustum[LEFT][C] = (float) (pf_clip[11] + pf_clip[ 8]);
	m_ppfFrustum[LEFT][D] = (float) (pf_clip[15] + pf_clip[12]);

	// Normalize the LEFT side
	NormalizePlane(m_ppfFrustum, LEFT);

	// This will extract the BOTTOM side of the frustum
	m_ppfFrustum[BOTTOM][A] = (float) (pf_clip[ 3] + pf_clip[ 1]);
	m_ppfFrustum[BOTTOM][B] = (float) (pf_clip[ 7] + pf_clip[ 5]);
	m_ppfFrustum[BOTTOM][C] = (float) (pf_clip[11] + pf_clip[ 9]);
	m_ppfFrustum[BOTTOM][D] = (float) (pf_clip[15] + pf_clip[13]);

	// Normalize the BOTTOM side
	NormalizePlane(m_ppfFrustum, BOTTOM);

	// This will extract the TOP side of the frustum
	m_ppfFrustum[TOP][A] = (float) (pf_clip[ 3] - pf_clip[ 1]);
	m_ppfFrustum[TOP][B] = (float) (pf_clip[ 7] - pf_clip[ 5]);
	m_ppfFrustum[TOP][C] = (float) (pf_clip[11] - pf_clip[ 9]);
	m_ppfFrustum[TOP][D] = (float) (pf_clip[15] - pf_clip[13]);

	// Normalize the TOP side
	NormalizePlane(m_ppfFrustum, TOP);

	// This will extract the BACK side of the frustum
	m_ppfFrustum[BACK][A] = (float) (pf_clip[ 3] - pf_clip[ 2]);
	m_ppfFrustum[BACK][B] = (float) (pf_clip[ 7] - pf_clip[ 6]);
	m_ppfFrustum[BACK][C] = (float) (pf_clip[11] - pf_clip[10]);
	m_ppfFrustum[BACK][D] = (float) (pf_clip[15] - pf_clip[14]);

	// Normalize the BACK side
	NormalizePlane(m_ppfFrustum, BACK);

	// This will extract the FRONT side of the frustum
	m_ppfFrustum[FRONT][A] = (float) (pf_clip[ 3] + pf_clip[ 2]);
	m_ppfFrustum[FRONT][B] = (float) (pf_clip[ 7] + pf_clip[ 6]);
	m_ppfFrustum[FRONT][C] = (float) (pf_clip[11] + pf_clip[10]);
	m_ppfFrustum[FRONT][D] = (float) (pf_clip[15] + pf_clip[14]);

	// Normalize the FRONT side
	NormalizePlane(m_ppfFrustum, FRONT);
}

// This determines if a point is inside of the frustum
bool CFrustum::PointInFrustum( float f_x, float f_y, float f_z )
{
	// Go through all the sides of the frustum
	for(int i = 0; i < 6; i++ )
	{
		// Calculate the plane equation and check if the point is behind a side of the frustum
		if(m_ppfFrustum[i][A] * f_x + m_ppfFrustum[i][B] * f_y + m_ppfFrustum[i][C] * f_z + m_ppfFrustum[i][D] <= 0)
		{
			// The point was behind a side, so it ISN'T in the frustum
			return false;
		}
	}

	// The point was inside of the frustum (In front of ALL the sides of the frustum)
	return true;
}


bool CFrustum::SphereInFrustum( float f_x, float f_y, float f_z, float f_radius ) const
{
	
	// Go through all the sides of the frustum
	for(int i = 0; i < 6; i++ )	
	{
		// If the center of the sphere is farther away from the plane than the radius
		if( m_ppfFrustum[i][A] * f_x + m_ppfFrustum[i][B] * f_y + m_ppfFrustum[i][C] * f_z + m_ppfFrustum[i][D] <= -f_radius )
		{
			// The distance was greater than the radius so the sphere is outside of the frustum
			return false;
		}
	}
	
	// The sphere was inside of the frustum!
	return true;
}

// This determines if a BOX is in or around our frustum by it's min and max points
bool CFrustum::BoxInFrustum( float f_min_x, float f_min_y, float f_min_z, float f_max_x, float f_max_y, float f_max_z)
{
	
	// Go through all of the corners of the box and check then again each plane
	// in the frustum.  If all of them are behind one of the planes, then it most
	// like is not in the frustum.
	for(int i = 0; i < 6; i++ )
	{
		if(m_ppfFrustum[i][A] * f_min_x + m_ppfFrustum[i][B] * f_min_y + m_ppfFrustum[i][C] * f_min_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_max_x + m_ppfFrustum[i][B] * f_min_y + m_ppfFrustum[i][C] * f_min_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_min_x + m_ppfFrustum[i][B] * f_max_y + m_ppfFrustum[i][C] * f_min_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_max_x + m_ppfFrustum[i][B] * f_max_y + m_ppfFrustum[i][C] * f_min_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_min_x + m_ppfFrustum[i][B] * f_min_y + m_ppfFrustum[i][C] * f_max_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_max_x + m_ppfFrustum[i][B] * f_min_y + m_ppfFrustum[i][C] * f_max_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_min_x + m_ppfFrustum[i][B] * f_max_y + m_ppfFrustum[i][C] * f_max_z + m_ppfFrustum[i][D] > 0)  continue;
		if(m_ppfFrustum[i][A] * f_max_x + m_ppfFrustum[i][B] * f_max_y + m_ppfFrustum[i][C] * f_max_z + m_ppfFrustum[i][D] > 0)  continue;

		// If we get here, it isn't in the frustum
		return false;
	}

	// Return a true for the box being inside of the frustum
	return true;
}