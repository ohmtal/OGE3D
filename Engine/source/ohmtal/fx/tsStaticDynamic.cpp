#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "core/stream/bitStream.h"

#include "tsStaticDynamic.h"

IMPLEMENT_CO_NETOBJECT_V1(TSStaticDynamic);

TSStaticDynamic::TSStaticDynamic()
{
   mNetFlags.clear(Ghostable | ScopeAlways);
   mNetFlags.set(Ghostable);

}
