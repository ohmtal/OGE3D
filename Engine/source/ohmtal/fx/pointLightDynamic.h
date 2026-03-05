//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//  pointLightDynamic - same as pointLight but also works when mounted
//-----------------------------------------------------------------------------
#ifndef _POINTLIGHTDYNAMIC_H_
#define _POINTLIGHTDYNAMIC_H_

#ifndef _LIGHTBASE_H_
#include "T3D/lightBase.h"
#endif
#ifndef _POINTLIGHT_H_
#include "T3D/pointLight.h"
#endif

class PointLightDynamic : public PointLight
{
   typedef PointLight Parent;

public:
   DECLARE_CONOBJECT(PointLightDynamic);

   PointLightDynamic();
};


#endif //_POINTLIGHTDYNAMIC_H_
