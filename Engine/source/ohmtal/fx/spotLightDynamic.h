//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//  SpotLightDynamic - same as spotLight but also works when mounted
//-----------------------------------------------------------------------------
#ifndef _SPOTLIGHTDYNAMIC_H_
#define _SPOTLIGHTDYNAMIC_H_

#ifndef _LIGHTBASE_H_
#include "T3D/lightBase.h"
#endif
#ifndef _SPOTLIGHT_H_
#include "T3D/spotLight.h"
#endif

class SpotLightDynamic : public SpotLight
{
   typedef SpotLight Parent;

public:
   DECLARE_CONOBJECT(SpotLightDynamic);

   SpotLightDynamic();
};


#endif //_SPOTLIGHTDYNAMIC_H_
