//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
// Copyright (c) 2023 Ohmtal Game Studio
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------
// 2023-02-18 (XXTH) changed the aiMove based on eye to getPosition
//
// 2023-02-19 (XXTH) added walk Sim::path
// 
// 2023-02-19 (XXTH) added sidesteps => stepLeft, stepRight, stepBack, stepForward
//                   checkInLos should be called with disabled check
//                   NO changed aimLocation mAimLocation = mAimObject->getPosition() + mAimOffset;
//                        to mAimLocation = mAimObject->getWorldBox().getCenter() + mAimOffset;
//
// 2023-02-20 (XXTH) added getAimObjectInLos
// 2023-02-22 (XXTH) * replaced mMoveState with _mMoveState and added get/set to make it easier
//                    to find out why the units stop while they have a movedestination (path).
//                    Only because the collided with an other unit.
//                   * removed the repath on movestuck !! let the script decide whats going on
//                   * added random to movedesination by movetolerance
// 2023-02-23 (XXTH) * long path is stopped - limit of nodes ?!! changed that onReach to repath
//                     if not close to destination.
//                   ~~~~~
//                   first i extended aiPlayer here but after to many changes i copied
//                   every from aiPlayer to aiEnhancedPlayer and restored the original aiPlayer.
// 
// 
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// Arcane-FX for MIT Licensed Open Source version of Torque 3D from GarageGames
// Copyright (C) 2015 Faust Logic, Inc.
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//

#include "platform/platform.h"
#include "math/mMatrix.h"
#include "console/engineAPI.h"
#include "console/consoleInternal.h"
#include "core/stream/bitStream.h"
#include "T3D/gameBase/moveManager.h"
#include <cfloat>

#include "aiEnhancedPlayer.h"
#include <ohmtal/ai/aiMath.h>

static U32 sAIPlayerLoSMask = TerrainObjectType | StaticShapeObjectType | StaticObjectType;

IMPLEMENT_CO_NETOBJECT_V1(aiEnhancedPlayer);

ConsoleDocClass(aiEnhancedPlayer,
   "@brief A Player object not controlled by conventional input, but by an AI engine.\n\n"

   "The AIPlayer provides a Player object that may be controlled from script.  You control "
   "where the player moves and how fast.  You may also set where the AIPlayer is aiming at "
   "-- either a location or another game object.\n\n"

   "The AIPlayer class does not have a datablock of its own.  It makes use of the PlayerData "
   "datablock to define how it looks, etc.  As the AIPlayer is an extension of the Player class "
   "it can mount objects and fire weapons, or mount vehicles and drive them.\n\n"

   "While the PlayerData datablock is used, there are a number of additional callbacks that are "
   "implemented by AIPlayer on the datablock.  These are listed here:\n\n"

   "void onReachDestination(AIPlayer obj) \n"
   "Called when the player has reached its set destination using the setMoveDestination() method.  "
   "The actual point at which this callback is called is when the AIPlayer is within the mMoveTolerance "
   "of the defined destination.\n\n"

   "void onMoveStuck(AIPlayer obj) \n"
   "While in motion, if an AIPlayer has moved less than moveStuckTolerance within a single tick, this "
   "callback is called.  From here you could choose an alternate destination to get the AIPlayer moving "
   "again.\n\n"

   "void onTargetEnterLOS(AIPlayer obj) \n"
   "When an object is being aimed at (following a call to setAimObject()) and the targeted object enters "
   "the AIPlayer's line of sight, this callback is called.  The LOS test is a ray from the AIPlayer's eye "
   "position to the center of the target's bounding box.  The LOS ray test only checks against interiors, "
   "statis shapes, and terrain.\n\n"

   "void onTargetExitLOS(AIPlayer obj) \n"
   "When an object is being aimed at (following a call to setAimObject()) and the targeted object leaves "
   "the AIPlayer's line of sight, this callback is called.  The LOS test is a ray from the AIPlayer's eye "
   "position to the center of the target's bounding box.  The LOS ray test only checks against interiors, "
   "statis shapes, and terrain.\n\n"

   "@tsexample\n"
   "// Create the demo player object\n"
   "%player = new AiPlayer()\n"
   "{\n"
   "  dataBlock = DemoPlayer;\n"
   "  path = \"\";\n"
   "};\n"
   "@endtsexample\n\n"

   "@see Player for a list of all inherited functions, variables, and base description\n"

   "@ingroup AI\n"
   "@ingroup gameObjects\n");

/**
 * This method calculates the moves for the AI player
 * if !mIsAiControlled we return false here 
 * @param movePtr Pointer to move the move list into
 */

void aiEnhancedPlayer::moveToNode(S32 node)
{
   if (mPathData.path.isNull())
      return;

   // -1 is shorthand for 'last path node'.
   if (node == -1)
      node = mPathData.path->size() - 1;

   // Consider slowing down on the last path node.
   setMoveDestination(mPathData.path->getNode(node), false);

   // Check flags for this segment.
   if (mPathData.index)
   {
      U16 flags = mPathData.path->getFlags(node - 1);
      // Jump if we must.
      if (flags & LedgeFlag)
         mJump = Ledge;
      else if (flags & JumpFlag)
         mJump = Now;
      else
         // Catch pathing errors.
         mJump = None;
   }

   // Store current index.
   mPathData.index = node;

}

void aiEnhancedPlayer::onReachDestination()
{
   //XXTH reset strafestep we need no script callback here  ?!
   if (mStrafeMode) {
      mStrafeMode = false;
      throwCallback("onStepDone");
      return;
   }


#ifdef TORQUE_NAVIGATION_ENABLED
   if (!mPathData.path.isNull())
   {
      if (mPathData.index == mPathData.path->size() - 1)
      {
         //XXTH path limited ?! 
         if (AIMath::Distance2D(getPosition(), mPathData.path->mTo) > mMoveTolerance)
         {
            repath();
            return;
         }


         // Handle looping paths.
         if (mPathData.path->mIsLooping)
            moveToNode(0);
         // Otherwise end path.
         else
         {
            clearPath();
            throwCallback("onReachDestination");
         }
      }
      else
      {
         moveToNode(mPathData.index + 1);
         // Throw callback every time if we're on a looping path.
         //if(mPathData.path->mIsLooping)
            //throwCallback("onReachDestination");
      }
   }
   else
#endif
      throwCallback("onReachDestination");

}

void aiEnhancedPlayer::onStuck()
{
   /* XXTH
#ifdef TORQUE_NAVIGATION_ENABLED
   if(!mPathData.path.isNull())
      repath();
   else
#endif
   */
   throwCallback("onMoveStuck");

}

bool aiEnhancedPlayer::getAIMove(Move* movePtr)
{
   if (mIsAiControlled)
   {
      *movePtr = NullMove;

      // Use the eye as the current position.
      /* XXTH Same old stupid glitch like in good old TGE was ..
      * This make the aiPlayer turn arround like crazy !!!! =>

      MatrixF eye;
      getEyeTransform(&eye);
      Point3F location = eye.getPosition();

      * Much better =>
      */
      Point3F location = getPosition();
      Point3F rotation = getRotation();

#ifdef TORQUE_NAVIGATION_ENABLED
      if (mDamageState == Enabled)
      {
         if (getMoveState() != ModeStop)
            updateNavMesh();
         if (!mFollowData.object.isNull())
         {
            if (mPathData.path.isNull())
            {
               if ((getPosition() - mFollowData.object->getPosition()).len() > mFollowData.radius // XXTH we dont hunt!also if not in LoS
                  || !mTargetInLOS
                  )
                  followObject(mFollowData.object, mFollowData.radius);
            }
            else
            {
               if ((mPathData.path->mTo - mFollowData.object->getPosition()).len() > mFollowData.radius)
                  repath();
               else if ((getPosition() - mFollowData.object->getPosition()).len() < mFollowData.radius //XXTH he we stop! also if not in LoS
                        && mTargetInLOS
                       ) 
               {
                  clearPath();
                  setMoveState(ModeStop);
                  throwCallback("onTargetInRange");
               }
               else if ((getPosition() - mFollowData.object->getPosition()).len() < mAttackRadius)
               {
                  throwCallback("onTargetInFiringRange");
               }
            }
         }
      }
#endif // TORQUE_NAVIGATION_ENABLED

      // Orient towards the aim point, aim object, or towards
      // our destination.

      if (mStrafeMode && getMoveState() != ModeStop) //XXTH
      {
         //strafe baby
      }
      else if (mAimObject || mAimLocationSet || getMoveState() != ModeStop)
      {
         // Update the aim position if we're aiming for an object
         //XXTH orig:

         if (mAimObject)
            mAimLocation = mAimObject->getPosition() + mAimOffset;
         else if (!mAimLocationSet)
            mAimLocation = mMoveDestination;

         F32 xDiff = mAimLocation.x - location.x;
         F32 yDiff = mAimLocation.y - location.y;

         if (!mIsZero(xDiff) || !mIsZero(yDiff))
         {
            // First do Yaw
            // use the cur yaw between -Pi and Pi
            F32 curYaw = rotation.z;
            while (curYaw > M_2PI_F)
               curYaw -= M_2PI_F;
            while (curYaw < -M_2PI_F)
               curYaw += M_2PI_F;

            // find the yaw offset
            F32 newYaw = mAtan2(xDiff, yDiff);
            F32 yawDiff = newYaw - curYaw;

            // make it between 0 and 2PI
            if (yawDiff < 0.0f)
               yawDiff += M_2PI_F;
            else if (yawDiff >= M_2PI_F)
               yawDiff -= M_2PI_F;

            // now make sure we take the short way around the circle
            if (yawDiff > M_PI_F)
               yawDiff -= M_2PI_F;
            else if (yawDiff < -M_PI_F)
               yawDiff += M_2PI_F;

            movePtr->yaw = yawDiff;

            // Next do pitch.
            if (!mAimObject && !mAimLocationSet)
            {
               // Level out if were just looking at our next way point.
               Point3F headRotation = getHeadRotation();
               movePtr->pitch = -headRotation.x;
            }
            else
            {
               // This should be adjusted to run from the
               // eye point to the object's center position. Though this
               // works well enough for now.
               F32 vertDist = mAimLocation.z - location.z;
               F32 horzDist = mSqrt(xDiff * xDiff + yDiff * yDiff);
               F32 newPitch = mAtan2(horzDist, vertDist) - (M_PI_F / 2.0f);
               if (mFabs(newPitch) > 0.01f)
               {
                  Point3F headRotation = getHeadRotation();
                  movePtr->pitch = newPitch - headRotation.x;
               }
            }
         }
      }
      else
      {
         // Level out if we're not doing anything else
         Point3F headRotation = getHeadRotation();
         movePtr->pitch = -headRotation.x;
      }

      // Move towards the destination
      if (getMoveState() != ModeStop)
      {
         F32 xDiff = mMoveDestination.x - location.x;
         F32 yDiff = mMoveDestination.y - location.y;

         // Check if we should mMove, or if we are 'close enough'
         if (mFabs(xDiff) < mMoveTolerance && mFabs(yDiff) < mMoveTolerance)
         {
            setMoveState(ModeStop);
            onReachDestination();
         }
         else
         {
            // Build move direction in world space
            if (mIsZero(xDiff))
               movePtr->y = (location.y > mMoveDestination.y) ? -1.0f : 1.0f;
            else
               if (mIsZero(yDiff))
                  movePtr->x = (location.x > mMoveDestination.x) ? -1.0f : 1.0f;
               else
                  if (mFabs(xDiff) > mFabs(yDiff))
                  {
                     F32 value = mFabs(yDiff / xDiff);
                     movePtr->y = (location.y > mMoveDestination.y) ? -value : value;
                     movePtr->x = (location.x > mMoveDestination.x) ? -1.0f : 1.0f;
                  }
                  else
                  {
                     F32 value = mFabs(xDiff / yDiff);
                     movePtr->x = (location.x > mMoveDestination.x) ? -value : value;
                     movePtr->y = (location.y > mMoveDestination.y) ? -1.0f : 1.0f;
                  }

            // Rotate the move into object space (this really only needs
            // a 2D matrix)
            Point3F newMove;
            MatrixF moveMatrix;
            moveMatrix.set(EulerF(0.0f, 0.0f, -(rotation.z + movePtr->yaw)));
            moveMatrix.mulV(Point3F(movePtr->x, movePtr->y, 0.0f), &newMove);
            movePtr->x = newMove.x;
            movePtr->y = newMove.y;

            // Set movement speed.  We'll slow down once we get close
            // to try and stop on the spot...
            if (mMoveSlowdown)
            {
               F32 speed = mMoveSpeed;
               F32 dist = mSqrt(xDiff * xDiff + yDiff * yDiff);
               F32 maxDist = mMoveTolerance * 2;
               if (dist < maxDist)
                  speed *= dist / maxDist;
               movePtr->x *= speed;
               movePtr->y *= speed;

               setMoveState(ModeSlowing);
            }
            else
            {
               movePtr->x *= mMoveSpeed;
               movePtr->y *= mMoveSpeed;

               setMoveState(ModeMove);
            }

            // Don't check for ai stuckness if animation during
            // an anim-clip effect override.
            if (mDamageState == Enabled && !(anim_clip_flags & ANIM_OVERRIDDEN) && !isAnimationLocked()) {
               if (mMoveStuckTestCountdown > 0)
                  --mMoveStuckTestCountdown;
               else
               {
                  // We should check to see if we are stuck...
                  F32 locationDelta = (location - mLastLocation).len();
                  if (locationDelta < mMoveStuckTolerance && mDamageState == Enabled)
                  {
                     // If we are slowing down, then it's likely that our location delta will be less than
                     // our move stuck tolerance. Because we can be both slowing and stuck
                     // we should TRY to check if we've moved. This could use better detection.
                     if (getMoveState() != ModeSlowing || locationDelta == 0)
                     {
                        setMoveState(ModeStuck);
                        onStuck();
                     }
                  }
               }
            }
         }
      }

      // Test for target location in sight if it's an object. The LOS is
      // run from the eye position to the center of the object's bounding,
      // which is not very accurate.
      if (mAimObject)
      {
         //XXTH orig:
         //if (checkInLos(mAimObject.getPointer()))
         if (checkInLos(mAimObject.getPointer(), false, true))
         {
            if (!mTargetInLOS)
            {
               throwCallback("onTargetEnterLOS");
               mTargetInLOS = true;
            }
         }
         else if (mTargetInLOS)
         {
            throwCallback("onTargetExitLOS");
            mTargetInLOS = false;
         }
      }

      Pose desiredPose = mPose;

      if (mSwimming)
         desiredPose = SwimPose;
      else if (mAiPose == 1 && canCrouch())
         desiredPose = CrouchPose;
      else if (mAiPose == 2 && canProne())
         desiredPose = PronePose;
      else if (mAiPose == 3 && canSprint())
         desiredPose = SprintPose;
      else if (canStand())
         desiredPose = StandPose;

      setPose(desiredPose);

      // Replicate the trigger state into the move so that
      // triggers can be controlled from scripts.
      for (U32 i = 0; i < MaxTriggerKeys; i++)
         movePtr->trigger[i] = getImageTriggerState(i);

#ifdef TORQUE_NAVIGATION_ENABLED
      if (mJump == Now)
      {
         movePtr->trigger[2] = true;
         mJump = None;
      }
      else if (mJump == Ledge)
      {
         // If we're not touching the ground, jump!
         RayInfo info;
         if (!getContainer()->castRay(getPosition(), getPosition() - Point3F(0, 0, 0.4f), StaticShapeObjectType, &info))
         {
            movePtr->trigger[2] = true;
            mJump = None;
         }
      }
#endif // TORQUE_NAVIGATION_ENABLED

      if (!(anim_clip_flags & ANIM_OVERRIDDEN) && !isAnimationLocked())
         mLastLocation = location;

      return true;

   }
   return false;
}

void aiEnhancedPlayer::updateMove(const Move* move)
{
   if (!getControllingClient() && isGhost())
      return;

   Parent::updateMove(move);

}

void aiEnhancedPlayer::clearPath()
{
   // Only delete if we own the path.
   if (!mPathData.path.isNull() && mPathData.owned)
      mPathData.path->deleteObject();
   // Reset path data.
   mPathData = PathData();

}

void aiEnhancedPlayer::clearCover()
{
   // Notify cover that we are no longer on our way.
   if (!mCoverData.cover.isNull())
      mCoverData.cover->setOccupied(false);
   mCoverData = CoverData();

}

void aiEnhancedPlayer::clearFollow()
{
   mFollowData = FollowData();
}

/**
 * Sets the object the bot is targeting
 *
 * @param targetObject The object to target
 */
void aiEnhancedPlayer::setAimObject(GameBase* targetObject)
{
   mAimObject = targetObject;
   mTargetInLOS = false;
   mAimOffset = Point3F(0.0f, 0.0f, 0.0f);

}
/**
 * Sets the object the bot is targeting and an offset to add to target location
 *
 * @param targetObject The object to target
 * @param offset       The offest from the target location to aim at
 */
void aiEnhancedPlayer::setAimObject(GameBase* targetObject, const Point3F& offset)
{
   mAimObject = targetObject;
   mTargetInLOS = false;
   mAimOffset = offset;
}

/**
 * Sets the location for the bot to aim at
 *
 * @param location Point to aim at
 */

void aiEnhancedPlayer::setAimLocation(const Point3F& location)
{
   mAimObject = 0;
   mAimLocationSet = true;
   mAimLocation = location;
   mAimOffset = Point3F(0.0f, 0.0f, 0.0f);

}

/**
 * Clears the aim location and sets it to the bot's
 * current destination so he looks where he's going
 */
void aiEnhancedPlayer::clearAim()
{
   mAimObject = 0;
   mAimLocationSet = false;
   mAimOffset = Point3F(0.0f, 0.0f, 0.0f);

}

/**
 * Sets the correct aim for the bot to the target
 */
void aiEnhancedPlayer::getMuzzleVector(U32 imageSlot, VectorF* vec)
{
   MatrixF mat;
   getMuzzleTransform(imageSlot, &mat);

   MountedImage& image = mMountedImageList[imageSlot];

   if (image.dataBlock->correctMuzzleVector)
   {
      disableHeadZCalc();
      if (getCorrectedAim(mat, vec))
      {
         enableHeadZCalc();
         return;
      }
      enableHeadZCalc();

   }
   mat.getColumn(1, vec);
}

bool aiEnhancedPlayer::checkInLos(GameBase* target, bool _useMuzzle, bool _checkEnabled)
{
   if (!isServerObject()) return false;
   if (!target)
   {
      target = mAimObject.getPointer();
      if (!target)
         return false;
   }
   if (_checkEnabled)
   {
      if (target->getTypeMask() & ShapeBaseObjectType)
      {
         ShapeBase* shapeBaseCheck = static_cast<ShapeBase*>(target);
         if (shapeBaseCheck)
            if (shapeBaseCheck->getDamageState() != Enabled) return false;
      }
      else
         return false;
   }

   RayInfo ri;

   disableCollision();

   S32 mountCount = target->getMountedObjectCount();
   for (S32 i = 0; i < mountCount; i++)
   {
      target->getMountedObject(i)->disableCollision();
   }

   Point3F checkPoint;
   if (_useMuzzle)
      getMuzzlePointAI(0, &checkPoint);
   else
   {
      MatrixF eyeMat;
      getEyeTransform(&eyeMat);
      eyeMat.getColumn(3, &checkPoint);
   }

   bool hit = !gServerContainer.castRay(checkPoint, target->getBoxCenter(), sAIPlayerLoSMask, &ri);
   enableCollision();

   for (S32 i = 0; i < mountCount; i++)
   {
      target->getMountedObject(i)->enableCollision();
   }
   return hit;
}



bool aiEnhancedPlayer::checkInFoV(GameBase* target, F32 camFov, bool _checkEnabled)
{
   if (!isServerObject()) return false;
   if (!target)
   {
      target = mAimObject.getPointer();
      if (!target)
         return false;
   }
   if (_checkEnabled)
   {
      if (target->getTypeMask() & ShapeBaseObjectType)
      {
         ShapeBase* shapeBaseCheck = static_cast<ShapeBase*>(target);
         if (shapeBaseCheck)
            if (shapeBaseCheck->getDamageState() != Enabled) return false;
      }
      else
         return false;
   }

   MatrixF cam = getTransform();
   Point3F camPos;
   VectorF camDir;

   cam.getColumn(3, &camPos);
   cam.getColumn(1, &camDir);

   camFov = mDegToRad(camFov) / 2;

   Point3F shapePos = target->getBoxCenter();
   VectorF shapeDir = shapePos - camPos;
   // Test to see if it's within our viewcone, this test doesn't
   // actually match the viewport very well, should consider
   // projection and box test.
   shapeDir.normalize();
   F32 dot = mDot(shapeDir, camDir);
   return (dot > mCos(camFov));
}

F32 aiEnhancedPlayer::getTargetDistance(GameBase* target, bool _checkEnabled)
{
   if (!isServerObject()) return false;
   if (!target)
   {
      target = mAimObject.getPointer();
      if (!target)
         return F32_MAX;
   }

   if (_checkEnabled)
   {
      if (target->getTypeMask() & ShapeBaseObjectType)
      {
         ShapeBase* shapeBaseCheck = static_cast<ShapeBase*>(target);
         if (shapeBaseCheck)
            if (shapeBaseCheck->getDamageState() != Enabled) return false;
      }
      else
         return F32_MAX;
   }

   return (getPosition() - target->getPosition()).len();
}


/**
 * Sets the speed at which this AI moves
 *
 * @param speed Speed to move, default player was 10
 */
void aiEnhancedPlayer::setMoveSpeed(F32 speed)
{
   mMoveSpeed = getMax(0.0f, getMin(1.0f, speed));
}

/**
 * Stops movement for this AI
 */
void aiEnhancedPlayer::stopMove()
{
   setMoveState(ModeStop);
#ifdef TORQUE_NAVIGATION_ENABLED
   clearPath();
   clearCover();
   clearFollow();
#endif
}

/**
 * Sets how far away from the move location is considered
 * "on target"
 *
 * @param tolerance Movement tolerance for error
 */
void aiEnhancedPlayer::setMoveTolerance(const F32 tolerance)
{
   mMoveTolerance = getMax(0.1f, tolerance);
}

/**
 * Sets the location for the bot to run to
 *
 * @param location Point to run to
 */
void aiEnhancedPlayer::setMoveDestination(const Point3F& location, bool slowdown)
{
   //XXTH 2023-02-22 adding random to x+y destination with usage of tolerance


   // AssertFatal(location != Point3F(0.f, 0.f, 0.f), "aiEnhancedPlayer::setMoveDestination is 0 0 0?! really ?");


   mMoveDestination = Point3F(
      location.x + gRandGen.randF(mMoveTolerance * -1, mMoveTolerance)
      , location.y + gRandGen.randF(mMoveTolerance * -1, mMoveTolerance)
      , location.z
   );

   //XXTH orig: mMoveDestination = location;
   setMoveState(ModeMove);
   mMoveSlowdown = slowdown;
   mMoveStuckTestCountdown = mMoveStuckTestDelay;
}



// These changes coordinate with anim-clip mods to parent class, Player.

// New method, restartMove(), restores the AIPlayer to its normal move-state
// following animation overrides from AFX. The tag argument is used to match
// the latest override and prevents interruption of overlapping animation
// overrides. See related anim-clip changes in Player.[h,cc].
void aiEnhancedPlayer::restartMove(U32 tag)
{
   if (tag != 0 && tag == last_anim_tag)
   {
      if (mMoveState_saved != -1)
      {
         setMoveState((MoveState)mMoveState_saved);
         mMoveState_saved = -1;
      }

      bool is_death_anim = ((anim_clip_flags & IS_DEATH_ANIM) != 0);

      last_anim_tag = 0;
      anim_clip_flags &= ~(ANIM_OVERRIDDEN | IS_DEATH_ANIM);

      if (mDamageState != Enabled)
      {
         if (!is_death_anim)
         {
            // this is a bit hardwired and desperate,
            // but if he's dead he needs to look like it.
            setActionThread("death10", false, false, false);
         }
      }
   }
}

// New method, saveMoveState(), stores the current movement state
// so that it can be restored when restartMove() is called.
void aiEnhancedPlayer::saveMoveState()
{
   if (mMoveState_saved == -1)
      mMoveState_saved = (S32)getMoveState();
}


//------------------------------------------------------------------------------
void aiEnhancedPlayer::strafeStep(aiEnhancedPlayer::StrafeMode lMode, F32 distance)
{
   VectorF moveVector(0.f, 0.f, 0.f);
   VectorF tmpVector(0.f, 0.f, 0.f);

   const F32 LEFTrad = mDegToRad(270.f);
   const F32 RIGHTrad = mDegToRad(90.f);

   switch (lMode)
   {
   case StrafeMode::left:
      tmpVector = AIMath::RotateAroundOrigin(getTransform().getForwardVector(), LEFTrad);
      moveVector = tmpVector * distance;
      break;
   case StrafeMode::right:
      tmpVector = AIMath::RotateAroundOrigin(getTransform().getForwardVector(), RIGHTrad);
      moveVector = tmpVector * distance;
      break;
   case StrafeMode::back:
      moveVector = getTransform().getForwardVector() * distance * -1.f;
      break;

   case StrafeMode::forward:
   default:
      moveVector = getTransform().getForwardVector() * distance;
      break;

   }


   setMoveDestination(getPosition() + moveVector, false);
   mStrafeMode = true;




}

void aiEnhancedPlayer::setAiPose(S32 pose)
{
   if (!getControllingClient() && isGhost())
      return;
   mAiPose = pose;
}

S32 aiEnhancedPlayer::getAiPose()
{
   return mAiPose;
}

/**
 * Set the state of a movement trigger.
 *
 * @param slot The trigger slot to set
 * @param isSet set/unset the trigger
 */
void aiEnhancedPlayer::setMoveTrigger(U32 slot, const bool isSet)
{
   if (slot >= MaxTriggerKeys)
   {
      Con::errorf("Attempting to set an invalid trigger slot (%i)", slot);
   }
   else
   {
      mMoveTriggers[slot] = isSet;   // set the trigger
      setMaskBits(NoWarpMask);         // force the client to updateMove
   }
}

/**
 * Get the state of a movement trigger.
 *
 * @param slot The trigger slot to query
 * @return True if the trigger is set, false if it is not set
 */
bool aiEnhancedPlayer::getMoveTrigger(U32 slot) const
{
   if (slot >= MaxTriggerKeys)
   {
      Con::errorf("Attempting to get an invalid trigger slot (%i)", slot);
      return false;
   }
   else
   {
      return mMoveTriggers[slot];
   }
}

/**
 * Clear the trigger state for all movement triggers.
 */
void aiEnhancedPlayer::clearMoveTriggers()
{
   for (U32 i = 0; i < MaxTriggerKeys; i++)
      setMoveTrigger(i, false);
}


void aiEnhancedPlayer::initPersistFields()
{
   addGroup("AI");

   addField("mMoveTolerance", TypeF32, Offset(mMoveTolerance, aiEnhancedPlayer),
      "@brief Distance from destination before stopping.\n\n"
      "When the aiEnhancedPlayer is moving to a given destination it will move to within "
      "this distance of the destination and then stop.  By providing this tolerance "
      "it helps the aiEnhancedPlayer from never reaching its destination due to minor obstacles, "
      "rounding errors on its position calculation, etc.  By default it is set to 0.25.\n");

   addField("moveStuckTolerance", TypeF32, Offset(mMoveStuckTolerance, aiEnhancedPlayer),
      "@brief Distance tolerance on stuck check.\n\n"
      "When the aiEnhancedPlayer is moving to a given destination, if it ever moves less than "
      "this tolerance during a single tick, the aiEnhancedPlayer is considered stuck.  At this point "
      "the onMoveStuck() callback is called on the datablock.\n");

   addField("moveStuckTestDelay", TypeS32, Offset(mMoveStuckTestDelay, aiEnhancedPlayer),
      "@brief The number of ticks to wait before testing if the aiEnhancedPlayer is stuck.\n\n"
      "When the aiEnhancedPlayer is asked to move, this property is the number of ticks to wait "
      "before the aiEnhancedPlayer starts to check if it is stuck.  This delay allows the aiEnhancedPlayer "
      "to accelerate to full speed without its initial slow start being considered as stuck.\n"
      "@note Set to zero to have the stuck test start immediately.\n");

   addField("AttackRadius", TypeF32, Offset(mAttackRadius, aiEnhancedPlayer),
      "@brief Distance considered in firing range for callback purposes.");

   endGroup("AI");

#ifdef TORQUE_NAVIGATION_ENABLED
   addGroup("Pathfinding");

   addField("allowWalk", TypeBool, Offset(mLinkTypes.walk, aiEnhancedPlayer),
      "Allow the character to walk on dry land.");
   addField("allowJump", TypeBool, Offset(mLinkTypes.jump, aiEnhancedPlayer),
      "Allow the character to use jump links.");
   addField("allowDrop", TypeBool, Offset(mLinkTypes.drop, aiEnhancedPlayer),
      "Allow the character to use drop links.");
   addField("allowSwim", TypeBool, Offset(mLinkTypes.swim, aiEnhancedPlayer),
      "Allow the character to move in water.");
   addField("allowLedge", TypeBool, Offset(mLinkTypes.ledge, aiEnhancedPlayer),
      "Allow the character to jump ledges.");
   addField("allowClimb", TypeBool, Offset(mLinkTypes.climb, aiEnhancedPlayer),
      "Allow the character to use climb links.");
   addField("allowTeleport", TypeBool, Offset(mLinkTypes.teleport, aiEnhancedPlayer),
      "Allow the character to use teleporters.");

   endGroup("Pathfinding");
#endif // TORQUE_NAVIGATION_ENABLED

   Parent::initPersistFields();

}

bool aiEnhancedPlayer::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Use the eye as the current position (see getAIMove)
   /* XXTH dont use eye!
   MatrixF eye;
   getEyeTransform(&eye);
   mLastLocation = eye.getPosition();
   */
   mLastLocation = getPosition();

   return true;

}

void aiEnhancedPlayer::onRemove()
{
#ifdef TORQUE_NAVIGATION_ENABLED
   clearPath();
   clearCover();
   clearFollow();
#endif
   Parent::onRemove();

}

/**
 * Utility function to throw callbacks. Callbacks  occure
 * on player class here !! 
 *
 * @param name Name of script function to call
 */
void aiEnhancedPlayer::throwCallback(const char* name)
{
   

   // lazy and crazy
   if (mIsSimPathWalking && strcmp(name, "onReachDestination") == 0)
   {
      MoveToNextSimPathWayPoint();
   }
   else {
     // Nah dont break it:  Con::executef(this, name);
      Con::executef(getDataBlock(), name, getIdString());
   }

   
}



//-----------------------------------------------------------------------------
// Simple SimPath following:
// I can do that better !!! would need the direction .... 
//-----------------------------------------------------------------------------
Marker* aiEnhancedPlayer::getClosestSimPathMarker()
{
   //Marker* lMarker = static_cast<Marker*>((*mSimPathObject)[mCurSimPathNodeIndex]);
   


   Marker* result = 0;
   if (!mSimPathObject || mSimPathObject->size() < 1 )
      return result;

   Marker* tmpMarker = 0;
   F32 DistToClosest = F32_MAX;
   for ( S32 i = 0; i < mSimPathObject->size(); i++)
   {
      tmpMarker = static_cast<Marker*>((*mSimPathObject)[i]);
      F32 Dist = AIMath::Distance2D(getPosition(), tmpMarker->getPosition());
      if (Dist < DistToClosest) {
         result = tmpMarker;
         mCurSimPathNodeIndex = i;
         DistToClosest = Dist;
      }
   }
   //	Con::printf("aiEnhancedPlayer::getClosestSimPathMarker: %f,%f,%f",result->x,result->y,result->z);
   return result;
}
//-----------------------------------------------------------------------------
bool aiEnhancedPlayer::MoveToSimPathWayMarker(Marker* lMarker)
{
   if (!lMarker) //huh=?!
      return false;
   setMoveDestination(lMarker->getPosition(), false);
   return true;
}
//-----------------------------------------------------------------------------
bool aiEnhancedPlayer::MoveToNextSimPathWayPoint()
{
   if (!getIsSimPathWalking() || !mSimPathObject)
      return false;

   if (mSimPathReverse)
   {
      mCurSimPathNodeIndex--;
      if (mCurSimPathNodeIndex < 0) //reached first
      {
         stopSimPath();
         throwCallback("onReachDestination");
         return false;
      }

   } else {
      mCurSimPathNodeIndex++;
      if (mCurSimPathNodeIndex >= mSimPathObject->size()) //reached last!
      {
         stopSimPath();
         throwCallback("onReachDestination");
         return false;
      }
   }

   Marker* lMarker = static_cast<Marker*>((*mSimPathObject)[mCurSimPathNodeIndex]);

   return MoveToSimPathWayMarker(lMarker);

}
//-----------------------------------------------------------------------------
void aiEnhancedPlayer::setSimPathObject(SimPath::Path* lPath)
{
   if (lPath && lPath->size() > 1) {
      mSimPathObject = lPath;
      mCurSimPathNodeIndex = -1;
   }
      

}
//-----------------------------------------------------------------------------
bool aiEnhancedPlayer::startSimPath(bool reverse)
{
   if (mSimPathObject && mSimPathObject->size() > 1 )
   {

      mIsSimPathWalking = true;
      mSimPathReverse = reverse;
      Marker* lMarker = getClosestSimPathMarker();
      return MoveToSimPathWayMarker(lMarker);
   }
   return false;
}


//-----------------------------------------------------------------------------
void aiEnhancedPlayer::stopSimPath()
{
   mIsSimPathWalking = false;
   stopMove();
}
//-----------------------------------------------------------------------------
// Engine methods
//-----------------------------------------------------------------------------
DefineEngineMethod(aiEnhancedPlayer, stop, void, (), ,
   "@brief Tells the AIPlayer to stop moving.\n\n")
{
   object->stopMove();
}

DefineEngineMethod(aiEnhancedPlayer, clearAim, void, (), ,
   "@brief Use this to stop aiming at an object or a point.\n\n"

   "@see setAimLocation()\n"
   "@see setAimObject()\n")
{
   object->clearAim();
}

DefineEngineMethod(aiEnhancedPlayer, setMoveSpeed, void, (F32 speed), ,
   "@brief Sets the move speed for an AI object.\n\n"

   "@param speed A speed multiplier between 0.0 and 1.0.  "
   "This is multiplied by the AIPlayer's base movement rates (as defined in "
   "its PlayerData datablock)\n\n"

   "@see getMoveDestination()\n")
{
   object->setMoveSpeed(speed);
}

DefineEngineMethod(aiEnhancedPlayer, getMoveSpeed, F32, (), ,
   "@brief Gets the move speed of an AI object.\n\n"

   "@return A speed multiplier between 0.0 and 1.0.\n\n"

   "@see setMoveSpeed()\n")
{
   return object->getMoveSpeed();
}

DefineEngineMethod(aiEnhancedPlayer, setMoveDestination, void, (Point3F goal, bool slowDown), (true),
   "@brief Tells the AI to move to the location provided\n\n"

   "@param goal Coordinates in world space representing location to move to.\n"
   "@param slowDown A boolean value. If set to true, the bot will slow down "
   "when it gets within 5-meters of its move destination. If false, the bot "
   "will stop abruptly when it reaches the move destination. By default, this is true.\n\n"

   "@note Upon reaching a move destination, the bot will clear its move destination and "
   "calls to getMoveDestination will return \"0 0 0\"."

   "@see getMoveDestination()\n")
{
   object->setMoveDestination(goal, slowDown);
}

DefineEngineMethod(aiEnhancedPlayer, getMoveDestination, Point3F, (), ,
   "@brief Get the AIPlayer's current destination.\n\n"

   "@return Returns a point containing the \"x y z\" position "
   "of the AIPlayer's current move destination. If no move destination "
   "has yet been set, this returns \"0 0 0\"."

   "@see setMoveDestination()\n")
{
   return object->getMoveDestination();
}

DefineEngineMethod(aiEnhancedPlayer, setAimLocation, void, (Point3F target), ,
   "@brief Tells the AIPlayer to aim at the location provided.\n\n"

   "@param target An \"x y z\" position in the game world to target.\n\n"

   "@see getAimLocation()\n")
{
   object->setAimLocation(target);
}

DefineEngineMethod(aiEnhancedPlayer, getAimLocation, Point3F, (), ,
   "@brief Returns the point the AIPlayer is aiming at.\n\n"

   "This will reflect the position set by setAimLocation(), "
   "or the position of the object that the bot is now aiming at.  "
   "If the bot is not aiming at anything, this value will "
   "change to whatever point the bot's current line-of-sight intercepts."

   "@return World space coordinates of the object AI is aiming at. Formatted as \"X Y Z\".\n\n"

   "@see setAimLocation()\n"
   "@see setAimObject()\n")
{
   return object->getAimLocation();
}

DefineEngineMethod(aiEnhancedPlayer, setAimObject, void, (const char* objName, Point3F offset), (Point3F::Zero), "( GameBase obj, [Point3F offset] )"
   "Sets the bot's target object. Optionally set an offset from target location."
   "@hide")
{

   // Find the target
   GameBase* targetObject;
   if (Sim::findObject(objName, targetObject))
   {

      object->setAimObject(targetObject, offset);
   }
   else
      object->setAimObject(0, offset);
}

DefineEngineMethod(aiEnhancedPlayer, getAimObject, S32, (), ,
   "@brief Gets the object the AIPlayer is targeting.\n\n"

   "@return Returns -1 if no object is being aimed at, "
   "or the SimObjectID of the object the AIPlayer is aiming at.\n\n"

   "@see setAimObject()\n")
{
   GameBase* obj = object->getAimObject();
   return obj ? obj->getId() : -1;
}


DefineEngineMethod(aiEnhancedPlayer, checkInLos, bool, (ShapeBase* obj, bool useMuzzle, bool checkEnabled), (nullAsType<ShapeBase*>(), false, false),
   "@brief Check whether an object is in line of sight.\n"
   "@obj Object to check. (If blank, it will check the current target).\n"
   "@useMuzzle Use muzzle position. Otherwise use eye position. (defaults to false).\n"
   "@checkEnabled check whether the object can take damage and if so is still alive.(Defaults to false)\n")
{
   return object->checkInLos(obj, useMuzzle, checkEnabled);
}

//XXTH getAimObjectInLos
DefineEngineMethod(aiEnhancedPlayer, getAimObjectInLos, bool, (), , " get the current aim is in Los")
{
   return object->getAimObjectInLos();
}



DefineEngineMethod(aiEnhancedPlayer, checkInFoV, bool, (ShapeBase* obj, F32 fov, bool checkEnabled), (nullAsType<ShapeBase*>(), 45.0f, false),
   "@brief Check whether an object is within a specified veiw cone.\n"
   "@obj Object to check. (If blank, it will check the current target).\n"
   "@fov view angle in degrees.(Defaults to 45)\n"
   "@checkEnabled check whether the object can take damage and if so is still alive.(Defaults to false)\n")
{
   return object->checkInFoV(obj, fov, checkEnabled);
}




DefineEngineMethod(aiEnhancedPlayer, setMoveTrigger, void, (U32 slot), ,
   "@brief Sets a movement trigger on an AI object.\n\n"
   "@param slot The trigger slot to set.\n"
   "@see getMoveTrigger()\n"
   "@see clearMoveTrigger()\n"
   "@see clearMoveTriggers()\n")
{
   object->setMoveTrigger(slot, true);
}

DefineEngineMethod(aiEnhancedPlayer, clearMoveTrigger, void, (U32 slot), ,
   "@brief Clears a movement trigger on an AI object.\n\n"
   "@param slot The trigger slot to set.\n"
   "@see setMoveTrigger()\n"
   "@see getMoveTrigger()\n"
   "@see clearMoveTriggers()\n")
{
   object->setMoveTrigger(slot, false);
}

DefineEngineMethod(aiEnhancedPlayer, getMoveTrigger, bool, (U32 slot), ,
   "@brief Tests if a movement trigger on an AI object is set.\n\n"
   "@param slot The trigger slot to check.\n"
   "@return a boolean indicating if the trigger is set/unset.\n"
   "@see setMoveTrigger()\n"
   "@see clearMoveTrigger()\n"
   "@see clearMoveTriggers()\n")
{
   return object->getMoveTrigger(slot);
}

DefineEngineMethod(aiEnhancedPlayer, clearMoveTriggers, void, (), ,
   "@brief Clear ALL movement triggers on an AI object.\n"
   "@see setMoveTrigger()\n"
   "@see getMoveTrigger()\n"
   "@see clearMoveTrigger()\n")
{
   object->clearMoveTriggers();
}


//------------------------------------------------------------------------------

DefineEngineMethod(aiEnhancedPlayer, getTargetDistance, F32, (ShapeBase* obj, bool checkEnabled), (nullAsType<ShapeBase*>(), false),
   "@brief The distance to a given object.\n"
   "@obj Object to check. (If blank, it will check the current target).\n"
   "@checkEnabled check whether the object can take damage and if so is still alive.(Defaults to false)\n")
{
   return object->getTargetDistance(obj, checkEnabled);
}

DefineEngineMethod(aiEnhancedPlayer, setAiPose, void, (S32 pose), ,
   "@brief Sets the AiPose for an AI object.\n"
   "@param pose StandPose=0, CrouchPose=1, PronePose=2, SprintPose=3.\n"
   "Uses the new AiPose variable from shapebase (as defined in its PlayerData datablock).\n")
{
   object->setAiPose(pose);
}

DefineEngineMethod(aiEnhancedPlayer, getAiPose, S32, (), ,
   "@brief Get the object's current AiPose.\n"
   "@return StandPose=0, CrouchPose=1, PronePose=2, SprintPose=3.\n")
{
   return object->getAiPose();
}

//------------------------------------------------------------------------------
//
// 
//-------------------------- XXTH  move ----------------------------------
DefineEngineMethod(aiEnhancedPlayer, getMoveState, S32, (), , "get the int value of current movestate"
   "0=ModeStop,1=ModeMove,2=ModeStuck,3=ModeSlowing"
)
{
   return (S32)object->getMoveState();
}

DefineEngineMethod(aiEnhancedPlayer, setMoveTolerance, void, (F32 lTolerance), (0.25f), "set the movetolerance (default is 0.25)"

)
{
   return object->setMoveTolerance(lTolerance);
}



DefineEngineMethod(aiEnhancedPlayer, continuePath, bool, (), , "continue the move after stuck happens")
{
   if (object->getMoveState() != aiEnhancedPlayer::ModeMove)
   {
      if (object->havePath()) {
         object->setMoveState(aiEnhancedPlayer::ModeMove);
         return true;
      }


   }
   return false;
}

//--------------- SimPath
DefineEngineMethod(aiEnhancedPlayer, setSimPath, void, (SimPath::Path* lPath), , "set a simpath for walking")
{
   return object->setSimPathObject(lPath);
}
DefineEngineMethod(aiEnhancedPlayer, startSimPath, bool, (bool reverse),(false), "start/continue the simpath")
{
   return object->startSimPath(reverse);
}

DefineEngineMethod(aiEnhancedPlayer, stopSimPath, void, (), , "stop the simpath")
{
   object->stopSimPath();
}

//-------------------------- XXTH  strafeStep ----------------------------------
DefineEngineMethod(aiEnhancedPlayer, stepLeft, void, (F32 lDist), (2.f), "Sidestep left")
{
   object->strafeStep(object->StrafeMode::left, lDist);
}

DefineEngineMethod(aiEnhancedPlayer, stepRight, void, (F32 lDist), (2.f), "Sidestep right")
{
   object->strafeStep(object->StrafeMode::right, lDist);
}

DefineEngineMethod(aiEnhancedPlayer, stepBack, void, (F32 lDist), (2.f), "step back")
{
   object->strafeStep(object->StrafeMode::back, lDist);
}

DefineEngineMethod(aiEnhancedPlayer, stepFor, void, (F32 lDist), (2.f), "step forward")
{
   object->strafeStep(object->StrafeMode::forward, lDist);
}

bool aiEnhancedPlayer::setPathDestination(const Point3F& pos)
{
   // Pathfinding only happens on the server.
   if (!isServerObject())
      return false;

   if (!getNavMesh())
      updateNavMesh();
   // If we can't find a mesh, just move regularly.
   if (!getNavMesh())
   {
      //setMoveDestination(pos);
      throwCallback("onPathFailed");
      return false;
   }

   // Create a new path. XXTH renamed to lPath
   NavPath* lPath = new NavPath();

   lPath->mMesh = getNavMesh();
   lPath->mFrom = getPosition();
   lPath->mTo = pos;
   lPath->mFromSet = lPath->mToSet = true;
   lPath->mAlwaysRender = true;
   lPath->mLinkTypes = mLinkTypes;
   lPath->mXray = true;
   // lPath->mIsSliced = true; //do it once!! 
   // Paths plan automatically upon being registered.
   if (!lPath->registerObject())
   {
      delete lPath;
      return false;
   }

   clearPath();
   clearCover();
   clearFollow();

   //XXTH check size
   if (lPath->success() && lPath->size() > 0)
   {
      // Clear any current path we might have.
      //XXTH MOVED UP also if it fail we delete the old one!!!
      /*
      clearPath();
      clearCover();
      clearFollow();
      */
      // Store new path.
      mPathData.path = lPath;
      mPathData.owned = true;
      // Skip node 0, which we are currently standing on.
      //XXTH can have only one point grrrr
      if (lPath->size() == 1)
      {
         moveToNode(0);
      }
      else {
         moveToNode(1);
      }
      
      throwCallback("onPathSuccess");
      return true;
   }
   else
   {
      // Just move normally if we can't path.
      //setMoveDestination(pos, true);
      //return;
      throwCallback("onPathFailed");
      lPath->deleteObject();
      return false;
   }
}


Point3F aiEnhancedPlayer::getPathDestination() const
{
   if (havePath())
      return mPathData.path->mTo;
   Con::errorf("aiEnhancedPlayer::getPathDestination PATH is NULL returning 0 0 0 destination ...:(");
   return Point3F(0, 0, 0);
}


void aiEnhancedPlayer::followNavPath(NavPath* path)
{
   if (!isServerObject())
      return;

   // Get rid of our current path.
   clearPath();
   clearCover();
   clearFollow();

   // Follow new path.
   mPathData.path = path;
   mPathData.owned = false;
   // Start from 0 since we might not already be there.
   moveToNode(0);
}


void aiEnhancedPlayer::followObject(SceneObject* obj, F32 radius)
{
   if (!isServerObject())
      return;

   if ((mFollowData.lastPos - obj->getPosition()).len() < mMoveTolerance)
      return;

   if (setPathDestination(obj->getPosition()))
   {
      clearCover();
      mFollowData.object = obj;
      mFollowData.radius = radius;
      mFollowData.lastPos = obj->getPosition();
   }
}


void aiEnhancedPlayer::repath()
{
   // Ineffectual if we don't have a path, or are using someone else's.

   //XXTH if(mPathData.path.isNull() || !mPathData.owned)
   if (!havePath())
      return;

   // If we're following, get their position.
   if (!mFollowData.object.isNull())
      mPathData.path->mTo = mFollowData.object->getPosition();
   // Update from position and replan.
   mPathData.path->mFrom = getPosition();
   mPathData.path->plan();

   //XXTH size <1 is bad!! 
   if (mPathData.path->size() < 1)
   {
      
      throwCallback("onPathFailed");
      return;

   }
   else if (mPathData.path->size() == 1) {
      //XXTH we have only ONE node to go to 0!
      moveToNode(0);
   }
   else {
      // Move to first node (skip start pos).
      moveToNode(1);
   }

   

   
}


struct CoverSearch
{
   Point3F loc;
   Point3F from;
   F32 dist;
   F32 best;
   CoverPoint* point;
   CoverSearch() : loc(0, 0, 0), from(0, 0, 0)
   {
      best = -FLT_MAX;
      point = NULL;
      dist = FLT_MAX;
   }
};

static void findCoverCallback(SceneObject* obj, void* key)
{
   CoverPoint* p = dynamic_cast<CoverPoint*>(obj);
   if (!p || p->isOccupied())
      return;
   CoverSearch* s = static_cast<CoverSearch*>(key);
   Point3F dir = s->from - p->getPosition();
   dir.normalizeSafe();
   // Score first based on angle of cover point to enemy.
   F32 score = mDot(p->getNormal(), dir);
   // Score also based on distance from seeker.
   score -= (p->getPosition() - s->loc).len() / s->dist;
   // Finally, consider cover size.
   score += (p->getSize() + 1) / CoverPoint::NumSizes;
   score *= p->getQuality();
   if (score > s->best)
   {
      s->best = score;
      s->point = p;
   }
}

bool aiEnhancedPlayer::findCover(const Point3F& from, F32 radius)
{
   if (radius <= 0)
      return false;

   // Create a search state.
   CoverSearch s;
   s.loc = getPosition();
   s.dist = radius;
   // Direction we seek cover FROM.
   s.from = from;

   // Find cover points.
   Box3F box(radius * 2.0f);
   box.setCenter(getPosition());
   getContainer()->findObjects(box, MarkerObjectType, findCoverCallback, &s);

   // Go to cover!
   if (s.point)
   {
      // Calling setPathDestination clears cover...
      bool foundPath = setPathDestination(s.point->getPosition());
      // Now store the cover info.
      mCoverData.cover = s.point;
      s.point->setOccupied(true);
      return foundPath;
   }
   return false;
}


NavMesh* aiEnhancedPlayer::findNavMesh() const
{
   // Search for NavMeshes that contain us entirely with the smallest possible
   // volume.
   NavMesh* mesh = NULL;
   SimSet* set = NavMesh::getServerSet();
   for (U32 i = 0; i < set->size(); i++)
   {
      NavMesh* m = static_cast<NavMesh*>(set->at(i));
      if (m->getWorldBox().isContained(getWorldBox()))
      {
         // Check that mesh size is appropriate.
         if (mMount.object) // Should use isMounted() but it's not const. Grr.
         {
            if (!m->mVehicles)
               continue;
         }
         else
         {
            if ((getNavSize() == Small && !m->mSmallCharacters) ||
               (getNavSize() == Regular && !m->mRegularCharacters) ||
               (getNavSize() == Large && !m->mLargeCharacters))
               continue;
         }
         if (!mesh || m->getWorldBox().getVolume() < mesh->getWorldBox().getVolume())
            mesh = m;
      }
   }
   return mesh;
}


void aiEnhancedPlayer::updateNavMesh()
{
   NavMesh* old = mNavMesh;
   if (mNavMesh.isNull())
      mNavMesh = findNavMesh();
   else
   {
      if (!mNavMesh->getWorldBox().isContained(getWorldBox()))
         mNavMesh = findNavMesh();
   }
   // See if we need to update our path.
   if (mNavMesh != old && !mPathData.path.isNull())
   {
      setPathDestination(mPathData.path->mTo);
   }
}

DefineEngineMethod(aiEnhancedPlayer, getNavMesh, S32, (), ,
   "@brief Return the NavMesh this AIPlayer is using to navigate.\n\n")
{
   NavMesh* m = object->getNavMesh();
   return m ? m->getId() : 0;
}

DefineEngineMethod(aiEnhancedPlayer, setNavSize, void, (const char* size), ,
   "@brief Set the size of NavMesh this character uses. One of \"Small\", \"Regular\" or \"Large\".")
{
   if (!dStrcmp(size, "Small"))
      object->setNavSize(aiEnhancedPlayer::Small);
   else if (!dStrcmp(size, "Regular"))
      object->setNavSize(aiEnhancedPlayer::Regular);
   else if (!dStrcmp(size, "Large"))
      object->setNavSize(aiEnhancedPlayer::Large);
   else
      Con::errorf("AIPlayer::setNavSize: no such size '%s'.", size);
}

DefineEngineMethod(aiEnhancedPlayer, getNavSize, const char*, (), ,
   "@brief Return the size of NavMesh this character uses for pathfinding.")
{
   switch (object->getNavSize())
   {
   case aiEnhancedPlayer::Small:
      return "Small";
   case aiEnhancedPlayer::Regular:
      return "Regular";
   case aiEnhancedPlayer::Large:
      return "Large";
   }
   return "";
}
//FIXME ?! #endif // TORQUE_NAVIGATION_ENABLED

DefineEngineMethod(aiEnhancedPlayer, setPathDestination, bool, (Point3F goal), ,
   "@brief Tells the AI to find a path to the location provided\n\n"

   "@param goal Coordinates in world space representing location to move to.\n"
   "@return True if a path was found.\n\n"

   "@see getPathDestination()\n"
   "@see setMoveDestination()\n")
{
   return object->setPathDestination(goal);
}
DefineEngineMethod(aiEnhancedPlayer, getPathDestination, Point3F, (), ,
   "@brief Get the AIPlayer's current pathfinding destination.\n\n"

   "@return Returns a point containing the \"x y z\" position "
   "of the AIPlayer's current path destination. If no path destination "
   "has yet been set, this returns \"0 0 0\"."

   "@see setPathDestination()\n")
{
   return object->getPathDestination();
}

DefineEngineMethod(aiEnhancedPlayer, followNavPath, void, (SimObjectId obj), ,
   "@brief Tell the AIPlayer to follow a path.\n\n"

   "@param obj ID of a NavPath object for the character to follow.")
{
   NavPath* path;
   if (Sim::findObject(obj, path))
      object->followNavPath(path);
}

DefineEngineMethod(aiEnhancedPlayer, followObject, void, (SimObjectId obj, F32 radius), ,
   "@brief Tell the AIPlayer to follow another object.\n\n"

   "@param obj ID of the object to follow.\n"
   "@param radius Maximum distance we let the target escape to.")
{
   SceneObject* follow;
   object->clearPath();
   object->clearCover();
   object->clearFollow();

   if (Sim::findObject(obj, follow))
      object->followObject(follow, radius);
}
DefineEngineMethod(aiEnhancedPlayer, clearFollow, void, (), ,
   "@brief Tell the AIPlayer not longer to follow another object.\n\n"
   )
{
   object->clearFollow();
}

DefineEngineMethod(aiEnhancedPlayer, repath, void, (), ,
   "@brief Tells the AI to re-plan its path. Does nothing if the character "
   "has no path, or if it is following a mission path.\n\n")
{
   object->repath();
}

DefineEngineMethod(aiEnhancedPlayer, findCover, S32, (Point3F from, F32 radius), ,
   "@brief Tells the AI to find cover nearby.\n\n"

   "@param from   Location to find cover from (i.e., enemy position).\n"
   "@param radius Distance to search for cover.\n"
   "@return Cover point ID if cover was found, -1 otherwise.\n\n")
{
   if (object->findCover(from, radius))
   {
      CoverPoint* cover = object->getCover();
      return cover ? cover->getId() : -1;
   }
   else
   {
      return -1;
   }
}

DefineEngineMethod(aiEnhancedPlayer, findNavMesh, S32, (), ,
   "@brief Get the NavMesh object this AIPlayer is currently using.\n\n"

   "@return The ID of the NavPath object this character is using for "
   "pathfinding. This is determined by the character's location, "
   "navigation type and other factors. Returns -1 if no NavMesh is "
   "found.")
{
   NavMesh* mesh = object->getNavMesh();
   return mesh ? mesh->getId() : -1;
}
