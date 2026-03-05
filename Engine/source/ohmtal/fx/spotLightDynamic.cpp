#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"

#include "spotLightDynamic.h"


IMPLEMENT_CO_NETOBJECT_V1(SpotLightDynamic);


SpotLightDynamic::SpotLightDynamic()
{
   mNetFlags.clear(Ghostable | ScopeAlways);
   mNetFlags.set(Ghostable);

}
