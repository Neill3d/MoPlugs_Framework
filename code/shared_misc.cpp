
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_misc.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_misc.h"
#include <math.h>


///////////////////////////////////////////////////////////////////////////////////

double VectorLength(double *v1, double *v2)
{
	return sqrt( (v2[0]-v1[0]) * (v2[0]-v1[0]) + (v2[1]-v1[1]) * (v2[1]-v1[1]) + (v2[2]-v1[2]) * (v2[2]-v1[2]) );
}

void VectorCenter(const double *v1, const double *v2, double *result)
{
	for (int i=0; i<3; ++i)
		result[i] = v1[i] + 0.5 * (v2[i] - v1[i]);
}