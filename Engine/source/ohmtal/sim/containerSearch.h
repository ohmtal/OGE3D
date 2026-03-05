//-----------------------------------------------------------------------------
// class containerSearch public SimSet
// replacement for InitContainerRadiusSearch which is not safe! 
// @since 2025-11-22
//-----------------------------------------------------------------------------
#ifndef _CONTAINERSEARCH_H_
#define _CONTAINERSEARCH_H_

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _TYPES_H_
#include "platform/types.h"
#endif

#ifndef _OBJECTTYPES_H_
#include "T3D/objectTypes.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif


class containerSearch : public SimSet
{
private:
	typedef SimSet Parent;

protected:
	Point3F mPosition;
	F32 mRadius; 
	U32 mMask; 
	bool mUseClientContainer;
	S32 mTimeStamp; //use unixTimeStamp
	
public: 
	containerSearch() :
		mRadius(10.f),
		mPosition(0.f,0.f,0.f),
		mMask(PlayerObjectType),
		mUseClientContainer(false),
		mTimeStamp(0)
	{	}
	DECLARE_CONOBJECT(containerSearch);
	static void initPersistFields();
	S32 search();

}; //class ContainerSearch 

#endif //_CONTAINERSEARCH_H_
