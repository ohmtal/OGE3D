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
#ifndef _ENHANCEDPLAYER_H_
#define _ENHANCEDPLAYER_H_

#ifndef _PLAYER_H_
#include "T3D/player.h"
#endif

class enhancedPlayer : public Player
{
protected:
   typedef Player Parent;

   /// Bit masks for different types of events
   enum MaskBits {
      CharMask = Parent::NextFreeMask << 0,
      NextFreeMask = Parent::NextFreeMask << 1
   };

   virtual U32  packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   virtual void unpackUpdate(NetConnection* conn, BitStream* stream);

   S32 mFaction;

public:
   DECLARE_CONOBJECT(enhancedPlayer);

   enhancedPlayer();
   virtual S32  getFaction();
   virtual bool setFaction(S32 lFaction);


};
#endif //_ENHANCEDPLAYER_H_
