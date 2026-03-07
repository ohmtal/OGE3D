//-----------------------------------------------------------------------------
//  Licen at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// LiteUnit
// @author T.Huehn (XXTH)
// @desc Attempt to add a minimal player class
//
// Goal is to only send simple commands from client to server to get the move
// instead of move updates each tick.
// This should reduce the ghost updates and allow to push more units ...
//
// !!! Events are called on Object not on Datablock !!!!
//
// FIXME:  Projectile does not collide with LiteUnit - do i need this here ?!
// FIXME:  Test: Does triggers work ?!
//
// commands would be:
//    moveTo x,y
//    stop
//    lookat x,y
// a problem is the lag to the client and how to sync it ...
//
//    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//       WARNING NOT PERFECT CLIENT/SERVER SYNCED
//    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// FIXME flying: funny move like a wave , and heading!! 
//
// Testing: 
// $mrBox = new LiteUnit(){ datablock = DefaultPlayerData; position=baseMarker1.getPosition();};
//
//
// 
//-----------------------------------------------------------------------------
#include "aiSteering.h"
#include "aiMath.h"
#include "LiteUnit.h"

#include "platform/platform.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "core/stream/bitStream.h"
#include "math/mathIO.h"
#include "math/mathUtils.h"
#include "sim/netConnection.h"
#include "T3D/player.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsPlayer.h"
#include <T3D/trigger.h>


//---------------- CLASS LiteUnit -------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(LiteUnit);


void LiteUnit::initPersistFields()
{

      addField( "useDatablockCalls", TypeBool, Offset(mUseDatablockCalls, LiteUnit),
                "@brief Use server callbacks on datablock instead on object \n");

      Parent::initPersistFields();
}


bool LiteUnit::onAdd()
{
   mStrafeMode = false; //did set in on constructor but was true on client ?!
   if (!Parent::onAdd() || !mDataBlock)
      return false;
   addToScene();
   if (isServerObject())
   {
         scriptOnAdd();
   }

   //2025-03-23 >>>
   if (PHYSICSMGR)
   {
      PhysicsWorld* world = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");

      mPhysicsRep = PHYSICSMGR->createPlayer();
      mPhysicsRep->init(mDataBlock->physicsPlayerType,
         mDataBlock->boxSize,
         mDataBlock->runSurfaceCos,
         mDataBlock->maxStepHeight,
         this,
         world);
      mPhysicsRep->setTransform(getTransform());
   }
   //2025-03-23 <<<<


   return true;
}

bool LiteUnit::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   //unused ?! PlayerData* prevData = mDataBlock;
   mDataBlock = dynamic_cast<PlayerData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   // Player requires a shape instance.
   if (mShapeInstance == NULL)
      return false;


   //worldbox / objbox
   mObjBox.maxExtents.x = mDataBlock->boxSize.x * 0.5f;
   mObjBox.maxExtents.y = mDataBlock->boxSize.y * 0.5f;
   mObjBox.maxExtents.z = mDataBlock->boxSize.z;
   mObjBox.minExtents.x = -mObjBox.maxExtents.x;
   mObjBox.minExtents.y = -mObjBox.maxExtents.y;
   mObjBox.minExtents.z = 0.0f;

   //if (!mInitAnimationDone)
   initAnimations();

   resetWorldBox();
   scriptOnNewDataBlock();
   return true;

}

void LiteUnit::onRemove()
{
   scriptOnRemove();
   removeFromScene();

   
   SAFE_DELETE(mPhysicsRep); //2025-03-23

   Parent::onRemove();
}


void LiteUnit::processTick(const Move* move)
{
   

   Move aiMove;
   if ( getAIMove(&aiMove)) {
      move = &aiMove;
   }
   ShapeBase::processTick(move); //do NOT remove this :P

   if (move)
   {
      updateMove(move);
      updatePos();

   }

   if (getDamageValue() >= 1.f && getDamageState() != Disabled )
   {
      if (isServerObject())
         setDamageState(Disabled);
      /* 2023-10-28 let the script do this !! 
      if (isClientObject())
         startAnimation(action_death);
      */
   }

   /* done in onNewDataBlock
   if (!mInitAnimationDone)
      initAnimations();
   */

   if (isClientObject() && getDamageState() != Disabled)
   {
      if (mMoveState == ModeStop)
         startIdleAnim();
      else if (mMoveState == ModeMove)
      {
         if ( mMoveSpeed < 0.5f ) {
            if (mAnimationId != action_walk)
               startAnimation(action_walk);
         }
         else {
            if (mAnimationId != action_run)
               startAnimation(action_run);
         }
      }
      else if (mMoveState == ModeRotate) {
         if (mAnimationId == action_idle)
            startAnimation(action_rotate);
      }
   }



}



void LiteUnit::setTransform(const MatrixF& mat)
{
   ShapeBase::setTransform(mat);
   setMaskBits(PositionMask);
}


void LiteUnit::interpolateTick(F32 dt)
{
   // Client side interpolation
   ShapeBase::interpolateTick(dt);

}


//----------------------------------------------------------------------------

U32 LiteUnit::packUpdate(NetConnection* connection, U32 mask, BitStream* stream)
{
   U32 retMask = ShapeBase::packUpdate(connection, mask, stream);
   if (stream->writeFlag(mask & PositionMask ))
   {
      mathWrite(*stream, mObjToWorld);
   }
   if (stream->writeFlag(mask & ExtendedInfoMask))
   {
      mathWrite(*stream, mObjScale);
      stream->writeFloat(mMoveSpeed, 6); //like DamageLevelBits 0f..1f
      //stream->writeFloat(mMoveTolerance, 6); //like DamageLevelBits ok?!
      //stream->writeFloat(mTerrainOffsetZ, 6);
      stream->write(mMoveTolerance);
      stream->write(mTerrainOffsetZ);
      stream->writeFlag(mFlying);

   }
   if (stream->writeFlag(mask & MoveMask))
   {
      stream->writeCompressedPoint(mMoveDestination);
      stream->writeInt(mMoveState, 3); //3 bits for 0..7 types
      // stream->writeFlag(mMoveSlowdown);
      stream->writeCompressedPoint(mLookAtPosition);
      stream->writeFlag(mStrafeMode);

   }

   //~~~ CharMask : mFaction
   if (stream->writeFlag(mask & CharMask))
   {
      stream->writeInt(mFaction, 8); //8 bits MAX => 255
      stream->writeInt(mUnitType, 8); //8 bits MAX => 255
      connection->packNetStringHandleU(stream, mGuildNameHandle);
   }

   //~~~ AnimMask
   if (stream->writeFlag(mask & AnimMask))
   {
      // BAD_ANIM_ID_U8
      if (mAnimationId > BAD_ANIM_ID_U8)
         mAnimationId = BAD_ANIM_ID_U8;
      stream->writeInt(mAnimationId , 8); //8 bits MAX => 255
      stream->writeFlag(mCready);
   }
   


   
      

   return retMask;
}
//----------------------------------------------------------------------------
void LiteUnit::unpackUpdate(NetConnection* connection, BitStream* stream)
{
   ShapeBase::unpackUpdate(connection, stream);
   if (stream->readFlag())
   {
      MatrixF mat;
      mathRead(*stream, &mat);
      ShapeBase::setTransform(mat);
      ShapeBase::setRenderTransform(mat);
   }
   if (stream->readFlag())
   {
      VectorF scale;
      mathRead(*stream, &scale);
      setScale(scale);
      mMoveSpeed = stream->readFloat(6); //like DamageLevelBits 0f..1f
      //mMoveTolerance = stream->readFloat(6); //like DamageLevelBits ok?!
      //mTerrainOffsetZ = stream->readFloat(6);
      stream->read(&mMoveTolerance);
      stream->read(&mTerrainOffsetZ);
      mFlying = stream->readFlag();
      
   }
   //MoveMask
   if (stream->readFlag())
   {
      stream->readCompressedPoint(&mMoveDestination);
      mMoveState     = (MoveState)stream->readInt(3);
      //mMoveSlowdown  = stream->readFlag();
      stream->readCompressedPoint(&mLookAtPosition);
      mStrafeMode = stream->readFlag();
   }

   //~~~ CharMask : mFaction
   if (stream->readFlag())
   {
      mFaction          = stream->readInt(8); //8 bits MAX => 0..255
      mUnitType         = stream->readInt(8); //8 bits MAX => 0..255
      mGuildNameHandle  = connection->unpackNetStringHandleU(stream);
   }

   //~~~ AnimMask 
   if (stream->readFlag())
   {
      //mAnimationId = stream->readInt(8); //8 bits MAX => 0..255
      startAnimation(stream->readInt(8));
      mCready = stream->readFlag();
   }

}



//----------------------------------------------------------------------------
/**
 * This method calculates the moves for the AI player
 *
 * @param movePtr Pointer to move the move list into
 */
bool LiteUnit::getAIMove(Move* movePtr)
{
   *movePtr = NullMove;

   Point3F location = getPosition();
   Point3F rotation = getRotation();


   if (mStrafeMode && getMoveState() != ModeStop) //XXTH
   {
         //strafe baby - No rotation
         // Con::printf("REMOVE ME !! Strafe Baby");
   }
   else if ( mMoveState == ModeMove || !mLookAtPosition.isZero())
   {
      // Orient towards the aim point, aim object, or towards
      // our destination.

      
      F32 xDiff, yDiff;
      bool lRotateState = mMoveState != ModeMove && !mLookAtPosition.isZero();
      if (lRotateState)
      {
         xDiff = mLookAtPosition.x - location.x;
         yDiff = mLookAtPosition.y - location.y;
      }
      else {
         xDiff = mMoveDestination.x - location.x;
         yDiff = mMoveDestination.y - location.y;
      }
      if (!mIsZero(xDiff) || !mIsZero(yDiff)) {

         // First do Yaw
         // use the cur yaw between -Pi and Pi
         F32 curYaw = rotation.z;
         while (curYaw > (F32)M_2PI)
            curYaw -= (F32)M_2PI;
         while (curYaw < -(F32)M_2PI)
            curYaw += (F32)M_2PI;

         // find the yaw offset
         F32 newYaw = mAtan2(xDiff, yDiff);
         F32 yawDiff = newYaw - curYaw;

         // make it between 0 and 2PI
         if (yawDiff < 0.0f)
            yawDiff += (F32)M_2PI;
         else if (yawDiff >= (F32)M_2PI)
            yawDiff -= (F32)M_2PI;

         // now make sure we take the short way around the circle
         if (yawDiff > (F32)M_PI)
            yawDiff -= (F32)M_2PI;
         else if (yawDiff < -(F32)M_PI)
            yawDiff += (F32)M_2PI;

         const F32 cMaxYaw = 0.1415f; // 0.3925f;
         movePtr->yaw = mClampF(yawDiff, -cMaxYaw, cMaxYaw);
  //       movePtr->yaw = yawDiff;

         
        if (lRotateState && mAbs(movePtr->yaw) > 0.001f) {
            mMoveState = ModeRotate;
         }
         else if (mMoveState == ModeRotate) {
            mMoveState = ModeStop;
         }



         // Next do pitch.
         // Level out if were just looking at our next way point.
         Point3F headRotation = getHeadRotation();
         movePtr->pitch = -headRotation.x;

      }
   }
   else {
      // Level out if we're not doing anything else
      Point3F headRotation = getHeadRotation();
      movePtr->pitch = -headRotation.x;
   }

   // Move towards the destination
   if (mMoveState == ModeMove) {
      F32 xDiff = mMoveDestination.x - location.x;
      F32 yDiff = mMoveDestination.y - location.y;
      F32 zDiff = mMoveDestination.z - location.z;

      // Check if we should mMove, or if we are 'close enough'
      bool zIsClose = true;
      if (getFlying())
      {
         zIsClose = mFabs(zDiff) < mMoveTolerance;

      }

      if (mFabs(xDiff) < mMoveTolerance && mFabs(yDiff) < mMoveTolerance && zIsClose)
      {
         //2025-04-03 extra function
         this->onDestinationReached();
            
      }
      else {
         // Build move direction in world space
         if (mIsZero(xDiff))
            movePtr->y = (location.y > mMoveDestination.y) ? -1.f : 1.f;
         else
            if (mIsZero(yDiff))
               movePtr->x = (location.x > mMoveDestination.x) ? -1.f : 1.f;
            else
               if (mFabs(xDiff) > mFabs(yDiff)) {
                  F32 value = mFabs(yDiff / xDiff);
                  movePtr->y = (location.y > mMoveDestination.y) ? -value : value;
                  movePtr->x = (location.x > mMoveDestination.x) ? -1.f : 1.f;
               }
               else {
                  F32 value = mFabs(xDiff / yDiff);
                  movePtr->x = (location.x > mMoveDestination.x) ? -value : value;
                  movePtr->y = (location.y > mMoveDestination.y) ? -1.f : 1.f;
               }
         // using mMoveTolerance / 2.f helps a lot againt up/down movement
         if (getFlying() && !mIsZero(zDiff, mMoveTolerance / 2.f))
         {
            if (mFabs(zDiff) < 1.f)
               movePtr->z =  zDiff;
            else
               movePtr->z = zDiff < 0.f ? -1.f : 1.f;
         }
         // Rotate the move into object space 
         Point3F newMove;
         MatrixF moveMatrix;
         moveMatrix.set(EulerF(0, 0, -(rotation.z + movePtr->yaw)));
         moveMatrix.mulV(Point3F(movePtr->x, movePtr->y, movePtr->z), &newMove);
         movePtr->x = newMove.x;
         movePtr->y = newMove.y;
         movePtr->z = newMove.z;

         // Set movement speed.  We'll slow down once we get close
         // to try and stop on the spot...
         if (false ) //mMoveSlowdown) 
         {
            F32 speed = mMoveSpeed;
            F32 dist = mSqrt(xDiff * xDiff + yDiff * yDiff);
            F32 maxDist = 5;
            if (dist < maxDist)
               speed *= dist / maxDist;
            movePtr->x *= speed;
            movePtr->y *= speed;
            movePtr->z *= speed;
         }
         else {
            movePtr->x *= mMoveSpeed;
            movePtr->y *= mMoveSpeed;
            movePtr->z *= mMoveSpeed;
         }

         // We should check to see if we are stuck...
/* 2025-03-19 liteUnit Cant stuck! :P
         if (location == mLastLocation) {
            throwCallback("onMoveStuck");
#if TORQUE_DEBUG
            Con::errorf("LiteUnit %d onMoveStuck! ", getId());
#endif // TORQUE_DEBUG

//2024-19-03            mMoveState = ModeStop;
         }
         */
      }
      mLastLocation = location; //added missing set 2024-19-03
   } //if modeMove

   // Replicate the trigger state into the move so that
   // triggers can be controlled from scripts.
   for (int i = 0; i < MaxTriggerKeys; i++)
      movePtr->trigger[i] = getImageTriggerState(i);

   
   return true;
}
// --------------------------------------------------------------------------------------------
void  LiteUnit::onDestinationReached()
{
   // path movement
   if (pathIsReady() && getSteering()->getPathObject()->setNextWayPoint())
   {
      setMoveDestination(getSteering()->getPathObject()->getCurrentWayPoint()->pos, false); //fixme slowdown!! 
   }
   else {
      stopMove();

      if (mStrafeMode)
      {
         mStrafeMode = false; //reset strafe mode
         throwCallback("onStepDone");
      }
      else {
         throwCallback("onReachDestination");
      }
   }
}
// --------------------------------------------------------------------------------------------
//Update the movement
void LiteUnit::updateMove(const Move* move)
{

   //for backward compatibility!
   if (mDamageState == Enabled) {
      //XXTH nah setImageTriggerState(0, move->trigger[0]);
      //XXTH nah setImageTriggerState(1, move->trigger[1]);
      //XXTH ADD MORE TRIGGERS=MOUNTPOINTS if needed values: 0..5 where 2 is reserved to jump
   }
   else {
      setImageTriggerState(0, 0);
      setImageTriggerState(1, 0);

   }

//   mDelta.move = *move;


   // Update current orientation
   if (mDamageState == Enabled) {
      //unused F32 prevZRot = mRot.z;
//      mDelta.headVec = mHead;

      F32 p = move->pitch;
      if (p > M_PI_F)
         p -= M_2PI_F;
      mHead.x = mClampF(mHead.x + p, mDataBlock->minLookAngle,
         mDataBlock->maxLookAngle);

      F32 y = move->yaw;
      if (y > M_PI_F)
         y -= M_2PI_F;

      mRot.z += y;
      mHead.z *= 0.5f;

      // constrain the range of mRot.z
      if (mFabs(mRot.z) > M_2PI * 10.f)
         mRot.z = 0.0f;

      while (mRot.z < 0.0f)
         mRot.z += M_2PI_F;
      while (mRot.z > M_2PI_F)
         mRot.z -= M_2PI_F;

   } // if (mDamageState == Enabled)

   // Desired move direction & speed
   MatrixF zRot;
   zRot.set(EulerF(0.0f, 0.0f, mRot.z));
   // Desired move direction & speed
   VectorF moveVec;
   F32 moveSpeed;

   if (mMoveState == ModeMove && mDamageState == Enabled
       && !isAnimationLocked())
   {
      zRot.getColumn(0, &moveVec);
      moveVec *= move->x;
      VectorF tv;
      zRot.getColumn(1, &tv);
      moveVec += tv * move->y;



      // Clamp water movement
      if (move->y > 0.0f)
      {
         // => Mhh something wrong with my steering ?! y is often negativ .. and on wander it does move side
         mMoveDirection = MoveForward;
         if (mFabs(move->x) > mFabs(move->y))
            mMoveDirection = MoveSide;

         if (mWaterCoverage >= 0.9f)
            moveSpeed = getMax(mDataBlock->maxUnderwaterForwardSpeed * getSpeedMulti() * move->y,
               mDataBlock->maxUnderwaterSideSpeed * getSpeedMulti() * mFabs(move->x));
         else
            moveSpeed = getMax(mDataBlock->maxForwardSpeed * getSpeedMulti() * move->y,
               mDataBlock->maxSideSpeed * getSpeedMulti() * mFabs(move->x));
      }
      else
      {


         // => Mhh something wrong with my steering ?! y is often negativ .. and on wander it does move side
         mMoveDirection = MoveBackward;
         if (mFabs(move->x) > mFabs(move->y))
            mMoveDirection = MoveSide;
         if (mWaterCoverage >= 0.9f)
            moveSpeed = getMax(mDataBlock->maxUnderwaterBackwardSpeed * getSpeedMulti() * mFabs(move->y),
               mDataBlock->maxUnderwaterSideSpeed * getSpeedMulti() * mFabs(move->x));
         else
            moveSpeed = getMax(mDataBlock->maxBackwardSpeed * getSpeedMulti() * mFabs(move->y),
               mDataBlock->maxSideSpeed * getSpeedMulti() * mFabs(move->x));
      }


   }
   else
   {
      moveVec.set(0.0f, 0.0f, 0.0f);
      moveSpeed = 0.0f;
   }
   VectorF acc(0.0f, 0.0f, 0.f);
   VectorF contactNormal(0.f, 0.f, 0.f);

   //running: >>>>>>>>>>>>>>><

   //mContactTimer = 0;

   // Remove acc into contact surface (should only be gravity)
   // Clear out floating point acc errors, this will allow
   // the player to "rest" on the ground.
   F32 vd = -mDot(acc, contactNormal);
   if (vd > 0.0f) {
      VectorF dv = contactNormal * (vd + 0.002f);
      acc += dv;
      if (acc.len() < 0.0001f)
         acc.set(0.0f, 0.0f, 0.0f);
   }


   if (getFlying())
   {
      moveVec.z = move->z;
      //FIXME mHead.x always zero! 
      VectorF flyVec(0, 0, -mHead.x);
      moveVec += flyVec * move->y;
   } //if (getFlying())

   VectorF pv;
   pv = moveVec;


   // Adjust the players's requested dir. to be parallel
   // to the contact surface.
   F32 pvl = pv.len();
   if (pvl) {
      VectorF nn;
      mCross(pv, VectorF(0.0f, 0.0f, 1.0f), &nn);
      nn *= 1.0f / pvl;
      VectorF cv = contactNormal;
      cv -= nn * mDot(nn, cv);
      pv -= cv * mDot(pv, cv);
      pvl = pv.len();
   }

   // Convert to acceleration
   if (pvl)
      pv *= moveSpeed / pvl;



   VectorF runAcc = pv - (mVelocity + acc);
   F32 runSpeed = runAcc.len();

   F32 maxAcc = (mDataBlock->runForce * getSpeedMulti() / mMass) * TickSec;
   if (runSpeed > maxAcc)
      runAcc *= maxAcc / runSpeed;
   acc += runAcc;
   //<<<<<<<<<<<<<<<< running: 

   mVelocity += acc;

}

//-----------------------------------------------------------------------------
bool LiteUnit::updatePos(const F32 travelTime)
{


   // getTransform().getColumn(3, &mDelta.posVec);

   if (mVelocity.len() < .01f)
      mVelocity.set(0.0f, 0.0f, 0.0f);


   Point3F start;
   Point3F initialPosition;
   getTransform().getColumn(3, &start);
   initialPosition = start;
   Point3F end = start + mVelocity * travelTime;
   Point3F distance = end - start;

   if (!getFlying())
   {
      dropToTerrain(end);
   }

/*
   if (mVelocity.len() > 0.f)
      Con::printf("pos: %f,%f,%f velo:%f,%f,%f",
         end.x, end.y, end.z,
         mVelocity.x, mVelocity.y, mVelocity.z);
*/



   setPosition(end, mRot);

   if (mPhysicsRep)
      mPhysicsRep->setTransform(getTransform()); //2025-03-23

   updateContainer();

   //2025-03-31 check triggers on server
   if (isServerObject()) 
   {
      gServerContainer.initRadiusSearch(end, 2.0f, TriggerObjectType);
      S32 id;
      while ((id = gServerContainer.containerSearchNext()))
      {
         Trigger* trigger = dynamic_cast<Trigger*>(Sim::findObject(id));
         if (trigger)
            trigger->potentialEnterObject(this);
      }
   }



   return true;
 
}


//-----------------------------------------------------------------------------
// required for trigger 2025-03-31
static MatrixF IMat(1);
bool LiteUnit::buildPolyList(PolyListContext, AbstractPolyList* polyList, const Box3F&, const SphereF&)
{
   // Collision with the player is always against the player's object
   // space bounding box axis aligned in world space.
   Point3F pos;
   getTransform().getColumn(3, &pos);
   IMat.setColumn(3, pos);
   polyList->setTransform(&IMat, Point3F(1.0f, 1.0f, 1.0f));
   polyList->setObject(this);
   polyList->addBox(mObjBox);
   return true;
}

//-----------------------------------------------------------------------------

void LiteUnit::setPosition(const Point3F& pos, const Point3F& rot)
{
   MatrixF mat;

   if (false && getFlying()) //<< XXTH 2024-03-19 added false again, this sucks! 
   {
      VectorF fwd = pos;
      fwd.normalize();
      mat = MathUtils::createOrientFromDir(fwd);

      //mat.set(EulerF(0.0f, 0.0f, rot.z));
      mat.setColumn(3, pos);
   }
   else {
      mat.set(EulerF(0.0f, 0.0f, rot.z));
      mat.setColumn(3, pos);
   }


   Parent::setTransform(mat);
   mRot = rot;
}

//-----------------------------------------------------------------------------
void LiteUnit::dropToTerrain(Point3F& location)
{
   bool lIsGhost = isGhost();
   /*
   if (mDataBlock->mIsBuilding)
      return;
   */

   U32 conformMask = (TerrainObjectType | TerrainLikeObjectType);
   RayInfo surfaceInfo;

   Point3F above = Point3F(location.x, location.y, location.z + 1000.f);
   Point3F below = Point3F(location.x, location.y, location.z - 1000.f);

   bool found = false;
   if (!lIsGhost) {
      if (gServerContainer.castRay(above, below, conformMask, &surfaceInfo))
         found = true;
   }
   else {
      if (gClientContainer.castRay(above, below, conformMask, &surfaceInfo))
         found = true;
   }

   if (found) {
      location.z = surfaceInfo.point.z + mTerrainOffsetZ;
   }

}

//=============================================================================
//=============================================================================

void LiteUnit::throwCallback(const char* name)
{
   //2025-03-21 Double Call sucks !!! added flag mUseDatablockCalls
   if (isServerObject())
   {
      if (mUseDatablockCalls)
            Con::executef(getDataBlock(), name, getIdString());
      else
            Con::executef(this, name);
   }
      
}


/**
 * Sets the location for the bot to run to
 *
 * @param location Point to run to
 */
bool LiteUnit::setMoveDestination(const Point3F& location, bool slowdown)
{

   //dont update the move when we are already on the way
   if (
        (mMoveState == ModeMove && (mMoveDestination - location).len() < 0.01)
        ||
        ((location - getPosition()).len() <= mMoveTolerance)
      )
   {
      onDestinationReached(); //added 2025-04-03 else path does stop if close to a point.
      return true;
   }

   // if we strafe we should cancel it.
   if (mStrafeMode)
   {
         mStrafeMode = false;
         throwCallback("onStepDone");
   }


   if (mMoveState == ModeStop)
      throwCallback("onStartMove");

   mMoveDestination     = location;
   mMoveState           = ModeMove;
   //mMoveSlowdown        = slowdown;

   if (isServerObject())
      setMaskBits(MoveMask);

  return true;
}


bool LiteUnit::setMoveToObject(GameBase* lObject, F32 lStopDistance)
{
   if (!lObject)
      return false;
   VectorF lVector = getSteering()->getPursuitVector(lObject, lStopDistance); // , true);
   if (lVector.isZero())
         return false;
   return setMoveDestination(lVector, false);
}

/**
 * Sets the speed at which this AI moves
 * values allowed: 0.f .. 1.f
 * @param speed Speed to move, default player was 10
 */
void LiteUnit::setMoveSpeed(F32 speed)
{
   mMoveSpeed = getMax(0.0f, getMin(1.0f, speed));
   if (isServerObject())
      setMaskBits(ExtendedInfoMask);

}

/**
 * Stops movement for this AI
 */
void LiteUnit::stopMove()
{

   if (mMoveState == ModeStop)
      return;

   mMoveState = ModeStop;
   if (isServerObject()) {
      //path
      if (pathIsReady())
         getSteering()->getPathObject()->Stop();


      throwCallback("onStopMove");
      setMaskBits(MoveMask);
      //mhh same jitter setMaskBits(PositionMask); //2024-01-14 sync position
   }
      
}

void LiteUnit::ContinueMove()
{
   if (mMoveState == ModeMove)
      return;

   mMoveState = ModeMove;
   if (isServerObject())
   {
      //path
      if (pathIsReady())
         getSteering()->getPathObject()->Resume();

      throwCallback("onContinueMove");
      setMaskBits(MoveMask);
   }
      
}

void LiteUnit::setLookPosition(const Point3F& location)
{
   if (mLookAtPosition == location)
      return;

   mLookAtPosition = location;
   if (isServerObject())
      setMaskBits(MoveMask);
}

void LiteUnit::clearLook()
{
   if (mLookAtPosition.isZero())
      return;
   mLookAtPosition.zero();
   if (isServerObject())
      setMaskBits(MoveMask);
}

void LiteUnit::initAnimations()
{
   if (mInitAnimationDone)
      return;

   //init animations on client and server ?
 // ~~~ idle ~~~~
   action_idle = getAnimationID("idle");
   if (action_idle == BAD_ANIM_ID)
      action_idle = getAnimationID("root");
   if (action_idle == BAD_ANIM_ID)
      Con::warnf("LiteUnit - no idle or root animation!! id: %s", this->getIdString());
   // ~~~ idle ~~~~
   action_cready = getAnimationID("cready");
   if (action_cready == BAD_ANIM_ID)
      action_cready = action_idle;
   // ~~~ run ~~~~
   action_run = getAnimationID("run");
   if (action_run == BAD_ANIM_ID)
      action_run = getAnimationID("walk");
   if (action_run == BAD_ANIM_ID)
      action_run = getAnimationID("1hrun");
   if (action_run == BAD_ANIM_ID)
      Con::warnf("LiteUnit - no run,walk or 1hrun animation!! id: %s", this->getIdString());

   // ~~~ walk ~~~~
   action_walk = getAnimationID("walk");
   if (action_walk == BAD_ANIM_ID)
      action_walk = action_run;

   // ~~~ rotate ~~~~
   action_rotate = getAnimationID("rotate");
   if (action_rotate == BAD_ANIM_ID)
      action_rotate = action_walk;


   // ~~~ death ~~~~
   action_death = getAnimationID("death");
   if (action_death == BAD_ANIM_ID)
      action_death = getAnimationID("die");
   if (action_death == BAD_ANIM_ID)
      action_death = getAnimationID("Death1");
   if (action_death == BAD_ANIM_ID)
      Con::warnf("LiteUnit - no death,die or death1 animation!! id:%s", this->getIdString());

   action_death2 = getAnimationID("Death2");
   if (action_death2 == BAD_ANIM_ID)
      action_death2 = action_death;
   action_death3 = getAnimationID("Death3");
   if (action_death3 == BAD_ANIM_ID)
      action_death3 = action_death;
   action_death4 = getAnimationID("Death4");
   if (action_death4 == BAD_ANIM_ID)
      action_death4 = action_death;
   action_death5 = getAnimationID("Death5");
   if (action_death5 == BAD_ANIM_ID)
      action_death5 = action_death;


   // ~~~ attack ~~~~
   action_attack1 = getAnimationID("attack1");
   if (action_attack1 == BAD_ANIM_ID)
      action_attack1 = getAnimationID("attack");
   action_attack2 = getAnimationID("attack2");
   if (action_attack2 == BAD_ANIM_ID)
      action_attack2 = action_attack1;
   action_attack3 = getAnimationID("attack3");
   if (action_attack3 == BAD_ANIM_ID)
      action_attack3 = action_attack1;
   action_attack4 = getAnimationID("attack4");
   if (action_attack4 == BAD_ANIM_ID)
      action_attack4 = action_attack1;
   if (action_attack1 == BAD_ANIM_ID)
      Con::warnf("LiteUnit - no attack animation!! id:%s", this->getIdString());


   mInitAnimationDone = true;


}

void LiteUnit::startIdleAnim()
{
   if (isServerObject())
      return;

   ShapeBase::Thread& st = getScriptThread(0);

   S32 idleAnim = mCready ? action_cready : action_idle;

   //we are already using idle or are in death anim
   if (st.sequence == idleAnim || st.sequence == action_death)
      return;

   if (st.sequence == -1 || st.sequence == action_run || st.sequence == action_walk)
   {
      startAnimation(idleAnim);
      return;
   }


   // ok we are playing something else and need to check if on end
   if (st.atEnd || st.thread->getSequence()->isCyclic()) {

      startAnimation(idleAnim);
      return;
   }

   // we should so nothing then ...

}

bool LiteUnit::playAttackAnim(S32 id)
{
   if (id == 0)
      id = gRandGen.randI(1, 4);
   
   S32 action = action_attack1;
   switch (id)
   {
   case 2:
      action = action_attack2;
      break;
   case 3:
      action = action_attack3;
      break;
   case 4:
      action = action_attack4;
      break;
   default:
      action = action_attack1;
      break;
   }

   return startAnimation(action);

}


bool LiteUnit::playDeathAnim(S32 id)
{
   if (id == 0)
      id = gRandGen.randI(1, 4);

   S32 action = action_death;
   switch (id)
   {
   case 2:
      action = action_death2;
      break;
   case 3:
      action = action_death3;
      break;
   case 4:
      action = action_death4;
      break;
   case 5:
      action = action_death5;
      break;
   }

   return startAnimation(action);
}

void LiteUnit::setCready(bool lValue)
{
   if (lValue == mCready)
      return;
   mCready = lValue;
   setMaskBits(AnimMask);
}

bool LiteUnit::startAnimation(S32 action)
{
   if (action == BAD_ANIM_ID || action == BAD_ANIM_ID_U8)
      return false;




   //no idea who calles this ... but I need to prevent it! 
   if (mDamageState != Enabled )
   {
      if (!isDeathAnimation(action))
         return false;

      if (isDeathAnimation(mDoubleDeathCalled)) {
#ifdef TORQUE_DEBUG
         Con::errorf("[SERVER:%d] DEATH ANIM DOUBLE CALL!! called unit: %d , anim: %d ", isServerObject(), getId(), action);
#endif // TORQUE_DEBUG
         return false;
      }
      
      mDoubleDeathCalled = action;

   }


   mAnimationId = action;
   if (isServerObject())
   {
      setMaskBits(AnimMask);
      return false;
   }

//2025-03-08 RESET!! since i added transitions to shapebase anmation did not work as is should. This fix it :D bad hack ? 
// 2025-03-13 not always .... mhhh
   setThreadSequence(0, action_idle);


   // 2025-03-07 trans, hold and wait are unused !!!!!
   //U32 ShapeBase::playAnimationByID(U32 anim_id, F32 pos, F32 rate, F32 trans, bool hold, bool wait, bool is_death_anim)
   return 0 != playAnimationByID(action, 0.f, 1.f, 0.5f, false, false, action == action_death);
}


// --------------------------------------------------------------------------------------------
bool LiteUnit::isDeathAnimation(S32 lAnimId)
{
   return
      lAnimId == action_death ||
      lAnimId == action_death2 ||
      lAnimId == action_death3 ||
      lAnimId == action_death4 ||
      lAnimId == action_death5;

}
// --------------------------------------------------------------------------------------------


bool LiteUnit::startAnimation(const char* name)
{
   S32 aniId = getAnimationID(name);
   if (aniId == BAD_ANIM_ID)
      return false;

   return startAnimation(aniId);
}

//let the castRay hit me :P
bool LiteUnit::castRay(const Point3F& start, const Point3F& end, RayInfo* info)
{
   F32 fst;
   if (!mObjBox.collideLine(start, end, &fst, &info->normal))
      return false;

   info->t = fst;
   info->object = this;
   info->point.interpolate(start, end, fst);
   info->material = NULL;
   return true;
}


Point3F LiteUnit::getVelocity() const
{
   return mVelocity;
}

void LiteUnit::setFlying(bool value)
{
   if (mFlying != value)
   {
      mFlying = value;
      if (isServerObject())
         setMaskBits(ExtendedInfoMask);
   }

}

//-----------------------------------------------------------------------------
// PATH 2024-01-11 !! created like in CompMove
//-----------------------------------------------------------------------------

bool LiteUnit::openPath(bool looping)
{
   if (mPathOpen)
   {
      Con::errorf("LiteUnit Close the CompMove Path before you open it again !!!");
      return false;
   }
   if (getSteering() == NULL || getSteering()->getPathObject() == NULL)
   {
      Con::errorf("LiteUnit Steering not initialized!!!!");
      return false;
   }
   getSteering()->getPathObject()->clearPoints();
   getSteering()->getPathObject()->setLooping(looping);
   mPathOpen = true;

   return true;
}
//-----------------------------------------------------------------------------
bool LiteUnit::addPathPoint(Point3F position)
{
   if (!mPathOpen)
   {
      Con::errorf("LiteUnit::addPathPoint => Path not open!");
      return false;
   }
   getSteering()->getPathObject()->addPoint(position);
   return true;
}
bool LiteUnit::pathIsReady()
{
   return (getSteering() &&
      getSteering()->getPathObject() &&
      !mPathOpen &&
      getSteering()->getPathObject()->haveNodes());
   
}
//-----------------------------------------------------------------------------
bool LiteUnit::closePath(bool splineIze)
{
   if (!mPathOpen)
   {
      Con::errorf("LiteUnit::closePath => Path not open!");
      return false;
   }

   mPathOpen = false;
   if (splineIze)
   {
      //fixme splinePrec parameter ?! is 0.1f as default
      getSteering()->getPathObject()->splineize();
   }

   return true;
}
//-----------------------------------------------------------------------------
bool LiteUnit::startPath(bool slowDown)
{
   if (!pathIsReady())
   {
      Con::errorf("LiteUnit::startPath() => Path not prepared!! open/add/close");
      return false;

   }
   getSteering()->getPathObject()->init(getPosition());

   setMoveDestination(getSteering()->getPathObject()->getCurrentWayPoint()->pos, slowDown);

   return true;
}
//-----------------------------------------------------------------------------
bool LiteUnit::stopPath()
{
   if (!pathIsReady())
   {
      Con::errorf("LiteUnit::stopPath() => Path not prepared!! open/add/close");
      return false;

   }
   // done in stopMove getSteering()->getPathObject()->Stop();
   stopMove();
   return true;
}
//-----------------------------------------------------------------------------
bool LiteUnit::continuePath()
{
   if (!pathIsReady())
   {
      Con::errorf("LiteUnit::continuePath() => Path not prepared!! open/add/close");
      return false;

   }
   // done in ContinueMove getSteering()->getPathObject()->Resume();
   ContinueMove();
   return true;
}
//-----------------------------------------------------------------------------
// PATH script
//-----------------------------------------------------------------------------

DefineEngineMethod(LiteUnit, openPath, bool, (bool looping), (true),
   "open a new path"
)
{
   return object->openPath(looping);
}

DefineEngineMethod(LiteUnit, addPathPoint, bool, (Point3F pos), ,
   "add a point to an open path"
)
{
   return object->addPathPoint(pos);
}


DefineEngineMethod(LiteUnit, closePath, bool, (bool splineize), (false),
   "close the path, optional splineize"
)
{
   return object->closePath(splineize);
}

DefineEngineMethod(LiteUnit, startPath, bool, (bool slowdown), (false),
   "(bool slowdown (not implemented !!)"
   "start the path, first it must be initilized"
)
{
   return object->startPath(slowdown);
}

// *** reduntant with use stop ***
DefineEngineMethod(LiteUnit, stopPath, void, (), ,
   "stop the current path - same as move"
)
{
   object->stopPath();
}
 
DefineEngineMethod(LiteUnit, continuePath, void, (), ,
   "continue the current path - same as move"
)
{
   object->continuePath();
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


/**
 * Sets how far away from the move location is considered
 * "on target"
 *
 * @param tolerance Movement tolerance for error
 */
void LiteUnit::setMoveTolerance(const F32 tolerance)
{
   mMoveTolerance = getMax(0.1f, tolerance);
   getSteering()->setMoveTolerance(tolerance);
   if (isServerObject()) {
      setMaskBits(ExtendedInfoMask);
   }
      

}


// --------------------------------------------------------------------------------------------
// Console Functions
// --------------------------------------------------------------------------------------------

DefineEngineMethod(LiteUnit, stop, void, (), , "Stop the player.")
{
   object->stopMove();
}

DefineEngineMethod(LiteUnit, move, void, (), , "restart the move of the player.")
{
   object->ContinueMove();
}



DefineEngineMethod(LiteUnit, setMoveSpeed, void, (F32 lSpeed), , "set the move speed ")
{
   object->setMoveSpeed(lSpeed);
}
DefineEngineMethod(LiteUnit, getMoveSpeed, F32, (), , "get the move speed ")
{
   return object->getMoveSpeed();
}


DefineEngineMethod(LiteUnit, setMoveDestination, void, (Point3F lDest, bool lSlowdown), (  false),
   "(Point3F goal, [UNUSED] bool slowDown=true)"
   "Tells the AI to move to the location provided."
   )
{
   object->setMoveDestination(lDest, lSlowdown);
}

DefineEngineMethod(LiteUnit, setMoveToObject, void, (GameBase* lObject, F32 lStopDistance), (0.f),
   "(GameBase object, F32 stopDistance)"
   "Pursuit to an Object."
)
{
   if (!lObject)
      return;
   object->setMoveToObject(lObject, lStopDistance);
}



DefineEngineMethod(LiteUnit, getMoveDestination, Point3F, (), ,
   "Returns the point the AI is set to move to.")
{
   return object->getMoveDestination();
}

DefineEngineMethod(LiteUnit, isMoving, bool, (), ,
   "are we in ModeMove state ?")
{
   return object->getMoveState() == LiteUnit::ModeMove;
}

DefineEngineMethod(LiteUnit, setMoveTolerance, void, (F32 value), ,
   "set the movetolerance default=0.25")
{
   object->setMoveTolerance(value);
}
DefineEngineMethod(LiteUnit, getMoveTolerance, F32, (), ,
   "set the movetolerance default=0.25")
{
   return object->getMoveTolerance();
}


DefineEngineMethod(LiteUnit, LookAt, void, (Point3F lDest), ,
   "(Point3F goal)"
   "Tells the AI to look at a point in modeStop."
)
{
   object->setLookPosition(lDest);
}
DefineEngineMethod(LiteUnit, clearLook, void, (), ,
   "(Point3F goal)"
   "Tells the AI to look at a point in modeStop."
)
{
   object->clearLook();
}


//-----------------------------------------------------------------------------
S32 LiteUnit::getFaction()
{
   return mFaction;
}

bool LiteUnit::setFaction(S32 lFaction)
{
   if (lFaction > 255) {
      Con::errorf("Allowed faction id's from 0 to 255");
      return false;
   }
   if (mFaction != lFaction)
   {
      mFaction = lFaction;
      if (isServerObject())
         setMaskBits(CharMask);
   }
   return true;

}
//-----------------------------------------------------------------------------
S32 LiteUnit::getUnitType()
{
   return mUnitType;
}


bool LiteUnit::setUnitType(S32 lUnitType)
{
   if (lUnitType > 255) {
      Con::errorf("Allowed unitTypes from 0 to 255");
      return false;
   }
   if ( mUnitType != lUnitType)
   {
      mUnitType = lUnitType;
      if (isServerObject())
         setMaskBits(CharMask);
   }
   return true;
}
// ---------------------- TerrainOffsetZ for "flying" units ---------------------
F32 LiteUnit::getTerrainOffsetZ()
{
   return mTerrainOffsetZ;
}

bool LiteUnit::setTerrainOffsetZ(F32 lTerrainOffsetZ)
{
   if ( mTerrainOffsetZ != lTerrainOffsetZ)
   {
      mTerrainOffsetZ = lTerrainOffsetZ;
      if (isServerObject())
         setMaskBits(ExtendedInfoMask);
   }
   return true;
}

// ---------------------- Guild ----------------------------
void LiteUnit::setGuildName(const char* name)
{
   if (!isGhost()) {
      if (name[0] != '\0') {
         // Use tags for better network performance
         // Should be a tag, but we'll convert to one if it isn't.
         if (name[0] == StringTagPrefixByte)
            mGuildNameHandle = NetStringHandle(U32(dAtoi(name + 1)));
         else
            mGuildNameHandle = NetStringHandle(name);
      }
      else {
         mGuildNameHandle = NetStringHandle();
      }
      setMaskBits(CharMask);
   }
}

//------------------------------------------------------------------------------
//Strafe
void LiteUnit::strafeStep(StrafeMode lMode, F32 lDistance, VectorF lCustomVector)
{
      VectorF moveVector(0.f, 0.f, 0.f);
      VectorF tmpVector(0.f, 0.f, 0.f);

      const F32 LEFTrad = mDegToRad(270.f);
      const F32 RIGHTrad = mDegToRad(90.f);

      switch (lMode)
      {
            case StrafeMode::left:
                  tmpVector = AIMath::RotateAroundOrigin(getTransform().getForwardVector(), LEFTrad);
                  moveVector = tmpVector * lDistance;
                  break;
            case StrafeMode::right:
                  tmpVector = AIMath::RotateAroundOrigin(getTransform().getForwardVector(), RIGHTrad);
                  moveVector = tmpVector * lDistance;
                  break;
            case StrafeMode::back:
                  moveVector = getTransform().getForwardVector() * lDistance * -1.f;
                  break;

            case StrafeMode::forward:
            default:
                  moveVector = getTransform().getForwardVector() * lDistance;
                  break;

            case StrafeMode::customVector:
                  lCustomVector.normalize(); //better :P
                  moveVector = lCustomVector * lDistance;
                  break;
      }

      setMoveDestination(getPosition() + moveVector, false);
      mStrafeMode = true;




}


//-----------------------------------------------------------------------------
// Engine methods
//-----------------------------------------------------------------------------
//--------------- faction
DefineEngineMethod(LiteUnit, setFaction, bool, (S32 lFaction), , "get the faction of the unit")
{
   return object->setFaction(lFaction);
}


DefineEngineMethod(LiteUnit, getFaction, S32, (), , "get the faction of the unit")
{
   return object->getFaction();
}

//--------------- unitType
DefineEngineMethod(LiteUnit, setUnitType, bool, (S32 lUnitType), , "get the unitType of the unit")
{
   return object->setUnitType(lUnitType);
}


DefineEngineMethod(LiteUnit, getUnitType, S32, (), , "get the UnitType of the unit")
{
   return object->getUnitType();
}

//--------------- TerrainOffsetZ
DefineEngineMethod(LiteUnit, setTerrainOffsetZ, bool, (F32 lTerrainOffsetZ), , "set the TerrainOffsetZ of the unit")
{
   return object->setTerrainOffsetZ(lTerrainOffsetZ);
}


DefineEngineMethod(LiteUnit, getTerrainOffsetZ, S32, (), , "get the TerrainOffsetZ of the unit")
{
   return object->getTerrainOffsetZ();
}


//--------------- GuildName
DefineEngineMethod(LiteUnit, setGuildName, void, (const char* name), , "")
{
   object->setGuildName(name);
}

DefineEngineMethod(LiteUnit, getGuildName, const char*, (), , "")
{
   return object->getGuildName();
}





DefineEngineMethod(LiteUnit, getState, const char*, (), ,
   "@brief Get the name of the player's current state.\n\n"
   "@return The current state; one of: \"Dead\", \"Mounted\", \"Move\", \"Recover\"\n")
{
   if (object->getDamageValue() < 1.0f)
      return "Move";

   return "Dead";
}

DefineEngineMethod(LiteUnit, setActionThread, bool, (const char* name),  ,
   "Start an animation"
)
{
   return object->startAnimation(name);
}

DefineEngineMethod(LiteUnit, playAttackAnim, bool, (S32 id),(0),
   "Start an attack animation - 1..4 or none parameter for random"
)
{
   return object->playAttackAnim(id);
}

DefineEngineMethod(LiteUnit, playDeathAnim, bool, (S32 id), (0),
   "Start an attack animation - 1..5 or none parameter for random"
)
{
   return object->playDeathAnim(id);
}

DefineEngineMethod(LiteUnit, setCready, void, (bool lValue), ,
   "playing cready animation instead of idle"
)
{
   object->setCready(lValue);
}

DefineEngineMethod(LiteUnit, setFlying, void, (bool lValue), ,
   "flying enabled ignore ground"
)
{
   object->setFlying(lValue);
}

DefineEngineMethod(LiteUnit, getFlying, bool, (), ,
   "is flying enabled?"
)
{
   return object->getFlying();
}

//-------------------------- XXTH  strafeStep ----------------------------------
DefineEngineMethod(LiteUnit, stepLeft, void, (F32 lDist), (2.f), "Sidestep left")
{
      object->strafeStep(object->StrafeMode::left, lDist);
}

DefineEngineMethod(LiteUnit, stepRight, void, (F32 lDist), (2.f), "Sidestep right")
{
      object->strafeStep(object->StrafeMode::right, lDist);
}

DefineEngineMethod(LiteUnit, stepBack, void, (F32 lDist), (2.f), "step back")
{
      object->strafeStep(object->StrafeMode::back, lDist);
}

DefineEngineMethod(LiteUnit, stepFor, void, (F32 lDist), (2.f), "step forward")
{
      object->strafeStep(object->StrafeMode::forward, lDist);
}

DefineEngineMethod(LiteUnit, stepWithVector, void, (VectorF lVector, F32 lDist), (2.f), "step with Vector")
{
      object->strafeStep(object->StrafeMode::customVector, lDist, lVector);
}
//---- << strafeStep

DefineEngineMethod(LiteUnit, getDiameter, F32, (), , "get object diameter")
{
      return object->getWorldBox().len_y();

}

