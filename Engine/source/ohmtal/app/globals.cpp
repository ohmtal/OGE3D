//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
/**
  * globals by t.huehn (XXTH) (c) ohmtal game studio 2021
  * @since: 2021-03-04
  * @static stuff
  */
//-----------------------------------------------------------------------------
#include "globals.h"
#include "console/engineAPI.h"

bool OhmtalGlobals::mDevelMode = false;

void OhmtalGlobals::init()
{
#ifdef OGE_ENVCACHE
   Con::setBoolVariable("$OGE_ENVCACHE", true);
#endif

#ifdef TORQUE_INTERIOR_ENABLED
   Con::setBoolVariable("$TORQUE_INTERIOR_ENABLED", true);
#endif

}

DefineEngineFunction(setDevelmode, void, (bool lDevelMode), , "set to development mode for cache handling")
{
   OhmtalGlobals::setDevelMode(lDevelMode);
}
DefineEngineFunction(getDevelmode, bool, (), , "get development mode")
{
   return OhmtalGlobals::getDevelMode();
}
