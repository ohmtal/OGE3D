//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#ifndef _TOMMATHTYPES_H_
#define _TOMMATHTYPES_H_


#ifndef _SIMBASE_H_
#include "console/simObject.h"
#endif
#ifndef _TYPES_H_
#include "platform/types.h"
#endif
#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif
#ifndef _MRECT_H_
#include "math/mRect.h"
#endif


class tomRectF : public SimObject
{
	typedef SimObject Parent;

public:
	tomRectF();
	DECLARE_CONOBJECT(tomRectF);
	RectF mRectF;
	static void initPersistFields();
}; //class

#endif //#ifndef _TOMMATHTYPES_H_
