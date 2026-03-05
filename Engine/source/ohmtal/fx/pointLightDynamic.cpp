//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//  pointLightDynamic - same as pointLight but also works when mounted
//-----------------------------------------------------------------------------
#include "platform/platform.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxDrawUtil.h"

#include "pointLightDynamic.h"


IMPLEMENT_CO_NETOBJECT_V1(PointLightDynamic);

PointLightDynamic::PointLightDynamic()
{
   
  mNetFlags.clear(Ghostable | ScopeAlways);
   mNetFlags.set(Ghostable);

}


