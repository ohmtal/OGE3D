//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// LiteUnit
// @author T.Huehn (XXTH)
// @desc Attempt to add a minimal player class
//
// * Bases on Shapebase
// * NO Collisions
// * Drop to Terrain
// * Animations
// * Movement
// * Callbacks against Object NOT datablock
//
// * mhh aiEntityRts should be better to do this :P but the aiEntity getAIMove is
//    heavy stuff^^ and i need it lite here!
// 
//-----------------------------------------------------------------------------

#ifndef _LITEUNIT_H_
#define _LITEUNIT_H_

#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif
#ifndef _PLAYER_H_
#include "T3D/player.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif
#ifndef _DYNAMIC_CONSOLETYPES_H_
#include "console/dynamicTypes.h"
#endif
#ifndef _AISTEERING_H_
#include "aiSteering.h"
#endif


//---------------- CLASS LiteUnit -------------------------------------------
class PhysicsPlayer;


class LiteUnit : public ShapeBase
{
public:
   typedef ShapeBase Parent;
protected:

   const S32 BAD_ANIM_ID_U8 = 255;

   enum MaskBits {
      PositionMask = Parent::NextFreeMask << 0, 
      CharMask     = Parent::NextFreeMask << 1,
      MoveMask     = Parent::NextFreeMask << 2,
      AnimMask     = Parent::NextFreeMask << 3,
      NextFreeMask = Parent::NextFreeMask << 4,
   };

   //like aiEntiy prefetch but really light :P

   S32
      action_run
      , action_idle
      , action_cready
      , action_walk
      , action_rotate
      , action_death
      , action_death2
      , action_death3
      , action_death4
      , action_death5
      , action_attack1
      , action_attack2
      , action_attack3
      , action_attack4
      ;
   bool mInitAnimationDone;
   S32 mAnimationId;
public:
   enum MoveState {
      ModeStop,
      ModeMove,
      ModeRotate,
      ModeStuck
   };


   enum MoveDirection {
      MoveForward,
      MoveBackward,
      MoveSide
   };

   enum StrafeMode {
      left,
      right,
      back,
      forward,
      customVector
   };


   PlayerData* mDataBlock;    ///< ...datablock...

   MoveDirection mMoveDirection;

   MoveState mMoveState;
   F32 mMoveSpeed;
   F32 mMoveTolerance;                 // Distance from destination before we stop
   Point3F mMoveDestination;           // Destination for movement
   Point3F mLastLocation;              // For stuck check
   //unused bool mMoveSlowdown;                 // Slowdown as we near the destination
   Point3F mLookAtPosition;            // look at something only works in ModeStop

   S32 mFaction;

   bool mCready;                    // i'am are cready

   Point3F mHead;                   ///< Head rotation, uses only x & z
   Point3F mRot;                    ///< Body rotation, uses only z
   VectorF mVelocity;

   AISteering* mSteering;

   PhysicsPlayer* mPhysicsRep; //2025-03-23 for castray !! ... receive collsions from afx or bullets when physics is enabled


   F32 mTerrainOffsetZ;
   S32 mUnitType;
   bool mFlying; //ignore ground 2024-01-08

   bool mStrafeMode;                   // we strafe so no rotation

   bool mUseDatablockCalls;            // like the direct calls more but it should be possible

public:
   DECLARE_CONOBJECT(LiteUnit);



   LiteUnit() :
      mDataBlock(0),
      mMoveState(ModeStop),
      mMoveDirection(MoveForward),
      mMoveSpeed(1.f),
      mMoveTolerance(0.25f),
      //mMoveSlowdown(false),
      mLookAtPosition(0.f, 0.f, 0.f),
      mFaction(0),
      mRot(0.f, 0.f, 0.f),
      mHead(0.f, 0.f, 0.f),
      mVelocity(0.f, 0.f, 0.f)
      , action_idle(BAD_ANIM_ID)
      , action_walk(BAD_ANIM_ID)
      , action_cready(BAD_ANIM_ID)
      , action_rotate(BAD_ANIM_ID)
      , action_run(BAD_ANIM_ID)
      , action_death(BAD_ANIM_ID)
      , action_death2(BAD_ANIM_ID)
      , action_death3(BAD_ANIM_ID)
      , action_death4(BAD_ANIM_ID)
      , action_death5(BAD_ANIM_ID)
      , action_attack1(BAD_ANIM_ID)
      , action_attack2(BAD_ANIM_ID)
      , action_attack3(BAD_ANIM_ID)
      , action_attack4(BAD_ANIM_ID)
      , mInitAnimationDone(false)
      , mAnimationId(BAD_ANIM_ID) //init with bad! 
      , mCready(false)
      , mDoubleDeathCalled(-1)
      , mTerrainOffsetZ(0.f)
      , mUnitType(0)
      , mFlying(false)
      , mPathOpen(false)
      , mStrafeMode(false)
      , mUseDatablockCalls(false)
      , mPhysicsRep(NULL)

   

  
   {
      mTypeMask |=  PlayerObjectType | DynamicShapeObjectType;
      mSteering = new AISteering(this);
      mSteering->setMoveTolerance(mMoveTolerance);

   }

   static void initPersistFields();


   bool onAdd();
   bool onNewDataBlock(GameBaseData* dptr, bool reload);
   void onRemove();

   void processTick(const Move* move);

   void setTransform(const MatrixF& mat);
   void interpolateTick(F32 dt);

   U32 packUpdate(NetConnection* connection, U32 mask, BitStream* stream);
   void unpackUpdate(NetConnection* connection, BitStream* stream);

   void startIdleAnim();
   bool playAttackAnim(S32 id = 0);
   bool playDeathAnim(S32 id = 0);
   void setCready(bool lValue);


   bool getAIMove(Move* movePtr);


   //faction
   virtual S32  getFaction();
   virtual bool setFaction(S32 lFaction);

   //UnitType
   virtual S32  getUnitType();
   virtual bool setUnitType(S32 lUnitType);


   //terrainOffset
   virtual F32 getTerrainOffsetZ();
   virtual bool setTerrainOffsetZ(F32 lTerrainOffsetZ);




   NetStringHandle mGuildNameHandle;

   void setGuildName(const char* name);
   inline const char* getGuildName()
   {
      return mGuildNameHandle.getString();
   }


   void updateMove(const Move* move);
   bool updatePos(const F32 travelTime = TickSec);
   void setPosition(const Point3F& pos, const Point3F& rot);
   void dropToTerrain(Point3F& location);
   const Point3F& getHeadRotation() { return mHead; }
   F32  getSpeedMulti() { return 1.f; } // (mSpeedModifier + 1.0f) / 16.0f;

   


   // Utility Methods
   void throwCallback(const char* name);



   // Movement sets/gets
   void setMoveSpeed(const F32 speed);
   F32 getMoveSpeed() const { return mMoveSpeed; }
   void setMoveTolerance(const F32 tolerance);
   F32 getMoveTolerance() const { return mMoveTolerance; }
   bool  setMoveDestination(const Point3F& location, bool slowdown);
   bool  setMoveToObject(GameBase* lObject, F32 lStopDistance); //pursuit to an object
   Point3F getMoveDestination() const { return mMoveDestination; }
   MoveState getMoveState() const { return mMoveState; }
   void stopMove();

   void ContinueMove();

   void setLookPosition(const Point3F& location);
   void clearLook();


   const Point3F getRotation() {
      Point3F pos, vec;
      getTransform().getColumn(1, &vec);
      Point3F lResult(0.0f, 0.0f, -mAtan2(-vec.x, vec.y));
      return lResult;
   }

   // Animations
   void initAnimations(); 
   bool startAnimation(S32 action);
   bool isDeathAnimation(S32 lAnimId);
   bool startAnimation(const char* name);

   //collitions for castRay
   bool castRay(const Point3F& start, const Point3F& end, RayInfo* info);
   bool buildPolyList(PolyListContext context, AbstractPolyList* polyList, const Box3F& box, const SphereF& sphere);



   Point3F getVelocity() const;
   AISteering* getSteering() { return mSteering; }

   bool getFlying() { return (mFlying); }
   void setFlying(bool value);


   S32 mDoubleDeathCalled;


   //XXTH strafeStep 2025-03-13
   virtual void strafeStep(StrafeMode lMode, F32 lDistance, VectorF lCustomVector = VectorF(0.f,0.f,0.f));


public:
   // path 
   bool openPath(bool looping = true);
   bool addPathPoint(Point3F position);
   bool pathIsReady();
   bool closePath(bool splineIze);
   bool startPath(bool slowDown);

   bool stopPath();
   bool continuePath();

protected:
   // ... path ...
   // AIPath* mPath;
   bool        mPathOpen;

   void  onDestinationReached();


};

#endif // _LITEUNIT_H_
