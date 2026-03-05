//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#include "platform/platform.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"

#include "terrain/terrData.h"
#include "aiEntityRTS.h"
#include "aiMath.h"


//---------------- CLASS AIEntityRTS -------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(AIEntityRTS);

//-----------------------------------------------------------------------------
void AIEntityRTS::processTick(const Move* move)
{
   bool lIsGhost = isGhost();


   if (!lIsGhost && !mHumanControlled) {
      if (!handleServerAIControlledTick(move))
         return;

   } // !lIsGhost


   // If we're not being controlled by a client, let the
   // AI sub-module get a chance at producing a move.
   Move aiMove;
   if (/*!move &&*/ isServerObject() && getAIMove(&aiMove)) {
      move = &aiMove;
   }
      



   ShapeBase::processTick(move);


   // Warp to catch up to server
   if (mDelta.warpTicks > 0) {
      mDelta.warpTicks--;

      // Set new pos.

      getTransform().getColumn(3, &mDelta.pos);

      mDelta.pos += mDelta.warpOffset;
      mDelta.rot += mDelta.rotOffset;
      setPosition(mDelta.pos, mDelta.rot);
      setRenderPosition(mDelta.pos, mDelta.rot);
      updateLookAnimation();

      // Backstepping
      mDelta.posVec.x = -mDelta.warpOffset.x;
      mDelta.posVec.y = -mDelta.warpOffset.y;
      mDelta.posVec.z = -mDelta.warpOffset.z;
      mDelta.rotVec.x = -mDelta.rotOffset.x;
      mDelta.rotVec.y = -mDelta.rotOffset.y;
      mDelta.rotVec.z = -mDelta.rotOffset.z;

   }
   else {
      // If there is no move, the player is either an
      // unattached player on the server, or a player's
      // client ghost.
      if (!move) {
         if (isGhost()) {
            // If we haven't run out of prediction time,
            // predict using the last known move.
            if (mPredictionCount-- <= 0)
            {
               return;
            }
            move = &mDelta.move;
         }
         else
            move = &NullMove;
      }
      if (!isGhost()) {
         updateAnimation(TickSec);
         updateActionThread();


      }


      if (isServerObject() || getControllingClient())
      {
//         updateWorkingCollisionSet();
         updateState();
         updateMove(move);
         updatePos();

      }
   }
   // #endif


}
//-----------------------------------------------------------------------------

//Update the movement
void AIEntityRTS::updateMove(const Move* move)
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

   mDelta.move = *move;


   // Update current orientation
   if (mDamageState == Enabled) {
      F32 prevZRot = mRot.z;
      mDelta.headVec = mHead;

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
      if (mControlObject)
         mHead.x *= 0.5f;
   

      // constrain the range of mRot.z
      if (mFabs(mRot.z) > M_2PI * 10.f)
         mRot.z = 0.0f;

      while (mRot.z < 0.0f)
         mRot.z += M_2PI_F;
      while (mRot.z > M_2PI_F)
         mRot.z -= M_2PI_F;

      mDelta.rot = mRot;
      mDelta.rotVec.x = mDelta.rotVec.y = 0.0f;
      mDelta.rotVec.z = prevZRot - mRot.z;
      if (mDelta.rotVec.z > M_PI_F)
         mDelta.rotVec.z -= M_2PI_F;
      else if (mDelta.rotVec.z < -M_PI_F)
         mDelta.rotVec.z += M_2PI_F;

      mDelta.head = mHead;
      mDelta.headVec -= mHead;
   } // if (mDamageState == Enabled)

   // Desired move direction & speed
   MatrixF zRot;
   zRot.set(EulerF(0.0f, 0.0f, mRot.z));
   // Desired move direction & speed
   VectorF moveVec;
   F32 moveSpeed;

   if (mState == MoveState && mDamageState == Enabled && !mPlayerCantMove && !isAnimationLocked())
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
         if (!mHumanControlled) //mTypeMask & AIObjectType) 
            mMoveDirection = MoveForward;
         else {
            mMoveDirection = MoveForward;
            if (mFabs(move->x) > mFabs(move->y))
               mMoveDirection = MoveSide;
         }

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
         if (!mHumanControlled) //mTypeMask & AIObjectType) 
            mMoveDirection = MoveForward;
         else {
            mMoveDirection = MoveBackward;
            if (mFabs(move->x) > mFabs(move->y))
               mMoveDirection = MoveSide;
         }
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

   mContactTimer = 0;

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

   // Force a 0 move if there is no energy, and only drain
   // move energy if we're moving.
   VectorF pv;
   if (mEnergy >= mDataBlock->minRunEnergy) {
      if (moveSpeed)
         mEnergy -= mDataBlock->runEnergyDrain;
      pv = moveVec;
   }
   else
      pv.set(0.0f, 0.0f, 0.0f);

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

   // Clamp acceleratin, player also accelerates faster when
   // in his hard landing recover state.
   F32 maxAcc = (mDataBlock->runForce * getSpeedMulti() / mMass) * TickSec;
   if (mState == RecoverState)
      maxAcc *= mDataBlock->recoverRunForceScale;
   if (runSpeed > maxAcc)
      runAcc *= maxAcc / runSpeed;
   acc += runAcc;

   mJumping = false;
   //<<<<<<<<<<<<<<<< running: 

   mVelocity += acc;

}

//-----------------------------------------------------------------------------
bool AIEntityRTS::updatePos(const F32 travelTime)
{

   
   getTransform().getColumn(3, &mDelta.posVec);

   if (mVelocity.len() < .01f)
      mVelocity.set(0.0f, 0.0f, 0.0f);


   Point3F start;
   Point3F initialPosition;
   getTransform().getColumn(3, &start);
   initialPosition = start;
   Point3F end = start + mVelocity * travelTime;
   Point3F distance = end - start;

   dropToTerrain(end);
   //dropToGround(end);

   if (isClientObject())
   {
      mDelta.pos = end;
      mDelta.posVec = mDelta.posVec - mDelta.pos;
      mDelta.dt = 1;
   }

   setPosition(end, mRot);
   if (isServerObject())
      setMaskBits(MoveMask);
   updateContainer();


   return true;
//   return Player::updatePos(travelTime);
}
//-----------------------------------------------------------------------------
void AIEntityRTS::dropToTerrain(Point3F& location)
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
      location.z = surfaceInfo.point.z;

   }

}

void AIEntityRTS::dropToGround(Point3F& location)
{
   bool lIsGhost = isGhost();
   /*
   if (mDataBlock->mIsBuilding)
      return;
   */


   
   U32 conformMask = (StaticObjectType | TerrainObjectType | InteriorLikeObjectType | TerrainLikeObjectType);
   RayInfo surfaceInfo;

   Point3F above = Point3F(location.x, location.y, location.z + 2.5f);
   Point3F below = Point3F(location.x, location.y, location.z - 500.f);

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
      location.z = surfaceInfo.point.z;

   }
}

void AIEntityRTS::throwCallback(const char* name)
{

   Con::executef(this, name);
}
