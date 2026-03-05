//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//
//
// class EnhancedPlayer 
// 
// @desc Like aiEnhancedPlayer but different collision
//
// @created 2023-02-22
// @author  T.Huehn XXTH
//
// For humancontrolled i use the existing flag: mIsAiControlled
// 
//-----------------------------------------------------------------------------
#include "console/engineAPI.h"
#include "console/consoleInternal.h"
#include "core/stream/bitStream.h"


#include "enhancedPlayer.h"


IMPLEMENT_CO_NETOBJECT_V1(enhancedPlayer);

//-----------------------------------------------------------------------------
// --------------------- NETWORK SEND RECEIVE ---------------------------------
//-----------------------------------------------------------------------------
U32 enhancedPlayer::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   //~~~ CharMask : mFaction
   if (stream->writeFlag(mask & CharMask))
   {
      stream->writeInt(mFaction, 8); //8 bits MAX => 255
   }

   return retMask;
}

void enhancedPlayer::unpackUpdate(NetConnection* conn, BitStream* stream)
{
   Parent::unpackUpdate(conn, stream);

   //~~~ CharMask : mFaction
   if (stream->readFlag())
   {
      mFaction = stream->readInt(8); //8 bits MAX => 0..255
   }


}
//-----------------------------------------------------------------------------
enhancedPlayer::enhancedPlayer()
{
   mCollisionMoveMask =
      TerrainObjectType
      | WaterObjectType
      | StaticShapeObjectType
      | VehicleObjectType
      // dont want so stuck the others or me | PlayerObjectType
      | TerrainLikeObjectType   //XXTH 
      | InteriorLikeObjectType  //XXTH 
      | PhysicalZoneObjectType
      ;

   mServerCollisionContactMask = mCollisionMoveMask |
      ItemObjectType |
      TriggerObjectType |
      CorpseObjectType;

   mClientCollisionContactMask = mCollisionMoveMask |
      TriggerObjectType;

   //default human controlled
   mIsAiControlled = false;

   mFaction = 0;

}
//-----------------------------------------------------------------------------
S32 enhancedPlayer::getFaction()
{
   return mFaction;
}

bool enhancedPlayer::setFaction(S32 lFaction)
{
   if (lFaction > 255) {
      Con::errorf("Allowed faction id's from 0 to 255");
      return false;
   }
   if (mFaction != lFaction)
   {
      mFaction = lFaction;
      setMaskBits(CharMask);
   }
   return true;

}
//-----------------------------------------------------------------------------
// Engine methods
//-----------------------------------------------------------------------------
//--------------- faction
DefineEngineMethod(enhancedPlayer, setFaction, bool, (S32 lFaction), , "get the faction of the unit")
{
   return object->setFaction(lFaction);
}


DefineEngineMethod(enhancedPlayer, getFaction, S32, (), , "get the faction of the unit")
{
   return object->getFaction();
}
