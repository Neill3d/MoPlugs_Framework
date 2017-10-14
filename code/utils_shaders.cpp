
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: utils_shaders.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "utils_shaders.h"

const int AddShaderCombination( CResourceGPUModel<ShaderGLSL> *shaders, 
									const int shader1, 
									const int shader2, 
									const int shader3, 
									const int shader4, 
									const int shader5,
									std::vector<int> *alpha_vector)
{
	const int numberOfShaders = (int) shaders->GetDataSize();
	// no possibility to make combinations
	if (numberOfShaders < 2)
		return 0;

	// 1 - try to find a combination
		
	size_t basecount = shaders->GetBaseSize();
	size_t count = shaders->GetDataSize();

	for (size_t i=basecount; i<count; ++i)
	{
		ShaderGLSL &lShader = shaders->GetDataItem(i);
		if (shader1 == lShader.shader1 && shader2 == lShader.shader2 && shader3 == lShader.shader3 
				&& shader4 == lShader.shader4 && shader5 == lShader.shader5)
		{
			// we find that this combination is already exist
			return (int)i;
		}
	}

	// 2 - add a new combination

	ShaderGLSL newShader = shaders->GetDataItem(shader1);
	if (shader2 >= 0) MergeShader( shaders->GetDataItem(shader2), newShader );
	if (shader3 >= 0) MergeShader( shaders->GetDataItem(shader3), newShader );
	if (shader4 >= 0) MergeShader( shaders->GetDataItem(shader4), newShader );
	if (shader5 >= 0) MergeShader( shaders->GetDataItem(shader5), newShader );
		
	int shaderId = (int) shaders->GetDataSize();
		
	newShader.shader1 = shader1;
	newShader.shader2 = shader2;
	newShader.shader3 = shader3;
	newShader.shader4 = shader4;
	newShader.shader5 = shader5;
		
	shaders->AddItem( newShader );
		
	if (alpha_vector && alpha_vector->size() > 0)
		alpha_vector->push_back( alpha_vector->at(shader1) );	// get alpha mode from base shader
	
	shaders->CauseAGPUUpdate();

	return shaderId;
}