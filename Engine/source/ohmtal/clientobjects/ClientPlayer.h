//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#ifndef _CLIENTPLAYER_H_
#define _CLIENTPLAYER_H_

#ifndef _PLAYER_H_
#include "T3D/player.h"
#endif

#ifndef _ClientTSCtrl_H_
#include "ClientTSCtrl.h"
#endif
//=================================================================================================
class ClientPlayer : public Player
{
private:
   typedef Player  Parent;
   ClientTSCtrl* mClientTS;

public:
   ClientPlayer() {
      mClientTS = NULL;
   }
   bool onAdd();
   static void initPersistFields();
   void processTick(const Move* move);
   void interpolateTick(F32 dt);

   void setClientTS(ClientTSCtrl* lClientTS) {
      if (lClientTS)
         lClientTS->setEnableMovement(true);
      else
         if (mClientTS)
            mClientTS->setEnableMovement(false);

      mClientTS = lClientTS;
   };
   void setSkinName(const char* name);


   DECLARE_CONOBJECT(ClientPlayer);
};

//=================================================================================================
class ClientAIPlayer : public ClientPlayer {

   typedef ClientPlayer Parent;

public:
   enum MoveState {
      ModeStop,
      ModeMove,
      ModeStuck,
   };

private:
   MoveState mMoveState;
   F32 mMoveSpeed;
   F32 mMoveTolerance;                 // Distance from destination before we stop
   Point3F mMoveDestination;           // Destination for movement
   Point3F mLastLocation;              // For stuck check
   bool mMoveSlowdown;                 // Slowdown as we near the destination



   SimObjectPtr<GameBase> mAimObject; // Object to point at, overrides location
   bool mAimLocationSet;               // Has an aim location been set?
   Point3F mAimLocation;               // Point to look at
   bool mTargetInLOS;                  // Is target object visible?

   // Utility Methods
   virtual void throwCallback(const char* name);
public:
   DECLARE_CONOBJECT(ClientAIPlayer);

   ClientAIPlayer();
   ~ClientAIPlayer();

   virtual bool getAIMove(Move* move);

   // Targeting and aiming sets/gets
   void setAimObject(GameBase* targetObject);
   GameBase* getAimObject() const { return mAimObject; }
   void setAimLocation(const Point3F& location);
   Point3F getAimLocation() const { return mAimLocation; }
   void clearAim();

   // Movement sets/gets
   void setMoveSpeed(const F32 speed);
   F32 getMoveSpeed() const { return mMoveSpeed; }
   void setMoveTolerance(const F32 tolerance);
   F32 getMoveTolerance() const { return mMoveTolerance; }
   void setMoveDestination(const Point3F& location, bool slowdown);
   Point3F getMoveDestination() const { return mMoveDestination; }
   void stopMove();



};
#endif //_CLIENTPLAYER_H_
