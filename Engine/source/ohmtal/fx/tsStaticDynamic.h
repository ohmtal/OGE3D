//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//  TSStaticDynamic - same as TSStatic but also works when mounted
//-----------------------------------------------------------------------------
#ifndef _TSSTATICDYNAMIC_H_
#define _TSSTATICDYNAMIC_H_

#ifndef _POINTLIGHT_H_
#include "T3D/tsStatic.h"
#endif

class TSStaticDynamic : public TSStatic
{
   typedef TSStatic Parent;

public:
   DECLARE_CONOBJECT(TSStaticDynamic);

   TSStaticDynamic();
};


#endif //_TSSTATICDYNAMIC_H_
