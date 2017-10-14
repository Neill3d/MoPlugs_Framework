
/**
	Declare common engine types and macros.
    Copyright (C) 2010 Sergey Solohin (neill), e-mail to: Neill.Solow<at>gmail.com
	home page - neill3d.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _TYPES_H_
#define _TYPES_H_

#include <Windows.h>

// property base class
template <class Object, class Type>
class Property
{
public:
	typedef	Type &(Object::*getproc) ();
	typedef void (Object::*setproc) (Type const&);

	//! a constructor
	Property(Object	*object, getproc get, setproc set)
		: mObject(object)
		, mGet(get)
		, mSet(set)
	{}
	//! a constructor with default value
	Property(const Type defval, Object	*object, getproc get, setproc set)
		: mObject(object)
		, mGet(get)
		, mSet(set)
	{
		set(defval);
	}
	
	operator Type &()
	{
		return get();
	}
	inline operator Type () const
	{
		return get();
	}
	Type const& operator = (Type const& value)
	{
		set(value);
		return value;
	}
	Property const& operator = (Property const& value)
	{
		set(value.get());
		return *this;
	}

private:
	Object		*mObject;
	getproc		mGet;
	setproc		mSet;

	inline Type &get() const
	{
		return (mObject->*mGet) ();
	}
	inline void set(Type const& value)
	{
		(mObject->*mSet)(value);
	}

};


//! class for singleton pattern
template<typename T> class Singleton
{
public:
	
	static T&	instance()
	{
		static T theSingletonInstance;
		return theSingletonInstance;
	}
};

// memory macros
#define FREEANDNIL(p)	if(p) { delete p; p=nullptr; }
#define ARRAYFREE(p)	if(p) { delete [] p; p=nullptr; }

#define	nullptr	NULL

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

//////////////////////////////////////////////////////////
// replicate opengl unsigned int vectors

struct uint2
{
	unsigned int x;
	unsigned int y;

	static uint2 zero()
	{
		uint2 elem;
		elem.x=0;
		elem.y=0;
		return elem;
	}
};

struct uint3
{
	unsigned int x;
	unsigned int y;
	unsigned int z;

	static uint3 zero()
	{
		uint3 elem;
		elem.x = elem.y = elem.z = 0;
		return elem;
	}

	static uint3 make_vector( uint2 v, unsigned int u )
	{
		uint3 result;
		result.x = v.x;
		result.y = v.y;
		result.z = u;
		return result;
	}
};

inline uint2 operator / (const uint2 &a, const uint2 &b)
{
	uint2 result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	return result;
}

inline uint2 operator + (const uint2 &a, const uint2 &b)
{
	uint2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

inline uint2 operator - (const uint2 &a, const unsigned int b)
{
	uint2 result;
	result.x = a.x - b;
	result.y = a.y - b;
	return result;
}

#endif // _TYPES_H_
