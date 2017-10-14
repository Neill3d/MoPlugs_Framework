
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_misc.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <windows.h>
#include <GL\glew.h>

/////////////////////////////////////////////////////////////


void				SetMainContext(HGLRC context);
HGLRC				GetMainContext();



//////////////////////////////////////////////////////////////////////////

double VectorLength(double *v1, double *v2);
void VectorCenter(const double *v1, const double *v2, double *result);


//////////////////////////////////////////////////////////////////////////


class	EvalRenderSync
{
public:
	//! a constructor
	EvalRenderSync()
	{
		mEvaluateIndex = 0;
		mRenderIndex = 1;

		mNeedEvaluation = false;
		mNeedSync = false;
		mNeedPrepRender = false;

		SyncUpdatePointers();
	}
	//! a destructor
	virtual ~EvalRenderSync()
	{}

	// processing stages
	
	virtual void	Evaluate()
	{
		EvaluationDone();
	}
	virtual void	Sync()
	{
		SyncSwap();
		SyncDone();
	}
	virtual	void	PrepRender()
	{
		PrepRenderDone();
	}
	
	//

	void	NeedEvaluation()
	{
		mNeedEvaluation = true;
	}
	const bool IsNeedEvaluation() {
		return mNeedEvaluation;
	}

	void	NeedSync()
	{
		mNeedSync = true;
	}
	const bool IsNeedSync() {
		return mNeedSync;
	}

	void	NeedPrepRender()
	{
		mNeedPrepRender = true;
	}
	const bool IsNeedPrepRender() {
		return mNeedPrepRender;
	}

protected:

	int			mEvaluateIndex;
	int			mRenderIndex;

	// flags to determine update stage
	bool		mNeedEvaluation;
	bool		mNeedSync;
	bool		mNeedPrepRender;

	
	virtual void	SyncClear()
	{
		mEvaluateIndex = 0;
		mRenderIndex = 1;

		SyncUpdatePointers();
	}

	virtual void SyncSwap()
	{
		mEvaluateIndex = 1 - mEvaluateIndex;
		mRenderIndex = 1 - mRenderIndex;

		SyncUpdatePointers();
	}

	// override with some code with double buffer data swap
	virtual void	SyncUpdatePointers()
	{
//		printf ("base function\n");
	}

	//
	// we pass some stages
	virtual void	EvaluationDone()
	{
		mNeedEvaluation = false;
		mNeedSync = true;
	}
	virtual void	SyncDone()
	{
		mNeedSync = false;
		mNeedPrepRender = true;
	}
	virtual void	PrepRenderDone()
	{
		mNeedPrepRender = false;
	}
};