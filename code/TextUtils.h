
#pragma once

/**	\file	TextUtils.h
*
*	functions and classes for work with text
*		such as parsing, etc.
*
*	Author Sergey Solokhin, 2003-2010
*		e-mail to: s@neill3d.com
*/

//#include "Types.h"
/*
#define floatToStr( value ) unitedstr( "%f", value )
#define boolToStr( value ) unitedstr( "%d", (int)value )
#define intToStr( value ) unitedstr( "%d", value )
*/
namespace CEUtils
{

//! a string token parser class
/*!
	Class is for split text into the token array with a space delimeter
	U can pass several delimeters as string, each string character in the delimeter is a delimeter
*/
class StringTokens
{
public:
	//! a constructor
	StringTokens(const char *text, const char *_delimeter=" ");
	//! a destructor
	~StringTokens();

	//! GetCount
	/*>
		/return - number of tokens in a string
	*/
	int GetCount() {
		return nCount;
	}

	//! GetToken
	/*>
		/param n - token index (0..count)
		/param buffer - output token buffer
		/param size - buffer size (check bounds)
		/return - token string
	*/
	void GetToken(int n, char *buffer, int size);

	//! GetNumberOfTokens
	/*>
		/param s - string input value
		/param delimeter - string, where each char is a separate delimeter (by default space delimeter only)
		/return - number of tokens in s string (with space delimeter)
	*/
	static	int GetNumberOfTokens(const char *s, const char *delimeter=" ");

private:
	const char	*delimeter;	//> char for separating tokens from each other
	const char	*mText;		//> pointer to a parsing string
	int nCount;			//> number of tokens
	int	*nTokens;		//> stride for each token in string
};

};

//-- format string and variable convertions
//const char *unitedstr (const char *text, ...);

//! GetTextParts
/*>
	/param s - string input value
	/return - number of tokens in s string (with space delimeter)
*/
int GetTextParts(const char *s);


//! GetTextPartLen
/*>
	/param s - string input value
	/param n - number of token in string
	/return - toker string length
*/
int GetTextPartLen(const char *s, int n);


//! GetTextPart
/*>
	/param s - string input value
	/param n - number of token in string
	/param part - pointer to destination token
*/
void GetTextPart(const char *s, int n, char *part);
char *GetTextPart(const char *s, int n);

//! CutTextPart
/*>
	/param s - string input value
	/param n - number of token in string
	/return - extruded token n from string s
*/
char *CutTextPart(const char *s, int n);

//! TranslateString
/*>
	/param but - string text buffer
	/return - translated string text
*/
char *TranslateString (const char *buf);
