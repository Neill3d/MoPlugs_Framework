
/**	\file	TextUtils.cpp
*
*	functions and classes for work with text
*		such as parsing, etc.
*
*	Author Sergey Solokhin, 2003-2010
*		e-mail to: s@neill3d.com
*/

#include <assert.h>
#include <string>
#include "TextUtils.h"
//#include "DefString.h"

// memory macros
#define FREEANDNIL(p)	if(p) { delete p; p=nullptr; }
#define ARRAYFREE(p)	if(p) { delete [] p; p=nullptr; }

//! pack two integers into one
inline int packValue(const int startPos, const int len)
{
	return (startPos << 16) | (len & 0xFFFF);
}
//! unpack two integers from one
inline void unpackValue(const int value, int &startPos, int &len)
{
	len = (value & 0xFFFF);
	startPos = value >> 16;
}


using namespace CEUtils;

#ifdef _MSC_VER
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#endif
#include <crtdbg.h>
#endif
#if defined(_MSC_VER) && defined(_DEBUG)
#define new new(_NORMAL_BLOCK, THIS_FILE, __LINE__)
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////
// String Tokens class

bool	IsDelimeter( const char ch, const char *delimeter, const int len )
{
	for (int i=0; i<len; i++)
	{
		if (ch == delimeter[i]) return true;
	}
	return false;
}

StringTokens::StringTokens(const char *text, const char *_delimeter)
		: mText(text)
		, delimeter(_delimeter)
{
	nTokens = NULL;
	nCount = GetNumberOfTokens(text, delimeter);
	if (nCount)
	{
		nTokens = new int[nCount];

		int i=0;
		bool inToken=false;
		int	count=0;
		int strLength = (int) strlen(text);
		int delLen = (int) strlen(delimeter);

		int startPos=0, len=0;

		while( IsDelimeter(text[i], delimeter, delLen) && (i<strLength) )
		{
			i++;
		}

		len = 0;
		startPos = i;
		//nTokens[count++] = i;		// we start first token
		inToken = true;
		for ( ; i<strLength; i++)
		{
			if ( !IsDelimeter(text[i], delimeter, delLen) ) {
				if (!inToken) startPos = i;
				inToken=true;
				len++;
			} 
			else
			{
				if (inToken)
				{
					nTokens[count++] = packValue( startPos, len);
					startPos = i;
					len = 0;
				}
				inToken = false;
			}
		}
		// catch last token
		if ( (len > 0) && (count < nCount))
		{
			nTokens[count] = packValue( startPos,len);
		}
	}
}

StringTokens::~StringTokens()
{
	mText = NULL;
	ARRAYFREE(nTokens);
}

void StringTokens::GetToken(int n, char *buffer, int size)
{
	int startPos, len;
	unpackValue( nTokens[n], startPos, len);

	//strset(buffer, 0);
	memset(buffer, 0, sizeof(char) * size);
	memcpy( buffer, (mText+startPos), sizeof(char)*len );
}

/////////////////////////////////////////////////////////////////////// static func
int StringTokens::GetNumberOfTokens(const char *s, const char *delimeter)
{
	if (!s || !delimeter)
		return 0;

	int res=0;
	int len= (int) strlen(s);
	int dlen = (int) strlen(delimeter);
	bool text=false;
	
	for (int i=0; i<len; i++) {
		if ( IsDelimeter(s[i], delimeter, dlen) || (s[i] == '\n') ) {
			if (text) { res++; text=false; }
		} else text = true;
	}

	if (text) res++;
	return res;
}

////////////////////////////////////////////////////////////////////////////////////////////////
/*
//-- перводим список аргументов в единую целостною строку
const char *unitedstr (const char *text, ...) {

	static DefString<char, LONG_STRING_LIMIT>  szUnitedText;

	//Compile string to output
	va_list argList;
	va_start(argList, text);

	//Write the text
	_strset_s(&szUnitedText[0], LONG_STRING_LIMIT, 0);
	vsprintf_s( &szUnitedText[0], LONG_STRING_LIMIT, text, argList );
	va_end(argList);

	return szUnitedText.c_str();
}
*/

int GetTextParts(const char *s)
{
	int res=0;
	int len= (int) strlen(s);
	bool text=false;

	for (int i=0; i<len; i++) {
		if (s[i]==' ' || s[i]==',' || s[i]=='\n' || s[i]=='(' || s[i] == ')' ) {
			if (text) { res++; text=false; }
		} else text = true;
	}

	if (text) res++;

	return res;
}



// n from 1 to ...
int GetTextPartLen(const char *s, int n)
{
	int l = 0;
	int res=0;
	int len= (int) strlen(s);
	bool text=false;

	for (int i=0; i<len; i++) {
		if (s[i]==' ' || s[i]==',' || s[i]=='\n' || s[i]=='(' || s[i] == ')' ) {
			if (text) { res++; text=false; }
		} else {
			text = true;

			if (res == (n-1)) l++;
		}
	}

	return l;
}

void GetTextPart(const char *s, int n, char *part)
{
	int l = 0;
	int res=0;
	int len= (int) strlen(s);
	bool text=false;

	for (int i=0; i<len; i++) {
		if (s[i]==' ' || s[i]==',' || s[i]=='\n' || s[i]=='(' || s[i] == ')' ) {
			if (text) { res++; text=false; }
		} else {
			text = true;
			if (res == (n-1))
				part[l++]=s[i];
		}
	}
}

char *GetTextPart(const char *s, int n)
{
	/*int l = 0;
	int res=0;
	int len=strlen(s);
	bool text=false;

	char *part=new char[GetTextPartLen(s, n)];

	for (int i=0; i<len; i++) {
		if (s[i]==' ' || s[i]==',' || s[i]=='\n' || s[i]=='(' || s[i] == ')' ) {
			if (text) { res++; text=false; }
		} else {
			text = true;

			if (res == (n-1) && isprint(s[i]))
				part[l++]=s[i];
		}
	}

	return part;*/

	int m=0;
	int res=0;


	int	count=0;
	char	*part = new char[80];

	for (size_t i=0; i<strlen(s); i++)
		if (!isspace(s[i])) {
			if (m == n) part[count++] = s[i];
			res=1;
		} else if (res == 1) {
				m++;
				res=0;
			}

	part[count] = '\0';

	return part;
}

char *CutTextPart(const char *s, int n)
{
	int l = GetTextPartLen(s, n);
	int res=0;
	int len = (int) strlen(s);
	bool text=false;
	char *part = new char[len-l];

	l=0;

	for (int i=0; i<len; i++) {
		if (s[i]==' ' || s[i]==',' || s[i]=='(' || s[i]==')' ) {
			if (text) { text=false; res++; part[l++]=s[i];}
		} else {
			text = true;

			if (res != (n-1))
				part[l++]=s[i];
		}
	}

	return part;
}

char *TranslateString (const char *buf)
{
	static	char	buf2[32768];
	int		i, l;
	char	*out;

	l = (int) strlen(buf);
	out = buf2;
	for (i=0 ; i<l ; i++)
	{
		if (buf[i] == '\n')
		{
			*out++ = '\r';
			*out++ = '\n';
		}
		else
			*out++ = buf[i];
	}
	*out++ = 0;

	return buf2;
}
