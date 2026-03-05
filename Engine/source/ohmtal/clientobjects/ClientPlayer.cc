//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------

#include "console/consoleInternal.h"
#include "console/engineAPI.h"

#include "ClientPlayer.h"


//-----------------------------------------------------------------------------------------------------------
// ClientPlayer
//-----------------------------------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ClientPlayer);

bool ClientPlayer::onAdd()
{
   mNetFlags.clear(Ghostable | ScopeAlways);
   mNetFlags.set(IsGhost);

#ifdef TGE_SPECIALPLAYERCOL 
   sClientCollisionContactMask = sServerCollisionContactMask;
#endif

   if (!mDataBlock) return (false);
   PlayerData* newDataBlock = mDataBlock;
   Parent::onNewDataBlock(newDataBlock, true);


   if (!Parent::onAdd())
      return(false);

   return true;
}
//-----------------------------------------------------------------------------------------------------------
void ClientPlayer::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("Misc");
   //ORIG addField("ClientDataBlock", TypeGameBaseDataPtr, Offset(mDataBlock, ClientPlayer));
   addField("ClientDataBlock", TYPEID< GameBaseData >(), Offset(mDataBlock, ClientPlayer));
   
   endGroup("Misc");


}
//-----------------------------------------------------------------------------------------------------------

void ClientPlayer::processTick(const Move* move)
{
   // We may have a Move from ClientTS
   Move aiMove, clMove;

   if (mClientTS && mClientTS->haveMove())
   {
      clMove = mClientTS->getMove();
      move = &clMove;
   }


   if (!move && getAIMove(&aiMove))
      move = &aiMove;


   // Manage the control object and filter moves for the player
   Move pMove, cMove;
   if (mControlObject) {
      if (!move)
         mControlObject->processTick(0);
      else {
         pMove = NullMove;
         cMove = *move;

         if (move->freeLook) {
            // Filter yaw/picth/roll when freelooking.

            pMove.yaw = move->yaw;
            pMove.pitch = move->pitch;

            pMove.roll = move->roll;
            pMove.freeLook = true;
            cMove.freeLook = false;
            cMove.yaw = cMove.pitch = cMove.roll = 0.0f;
         }
         mControlObject->processTick((mDamageState == Enabled) ? &cMove : &NullMove);
         move = &pMove;
      }
   }

   ShapeBase::processTick(move);

   // If there is no move, the player is either an
   // unattached player on the server, or a player's
   // client ghost.
   if (!move) {
      if (isGhost()) {
         /*
           // If we haven't run out of prediction time,
           // predict using the last known move.
           if (mPredictionCount-- <= 0)
           {
              return;
           }
        */
         move = &mDelta.move;
      }
      else
         move = &NullMove;
   }
   if (!isGhost()) {
      updateAnimation(TickSec);

   }
   //if(isServerObject() || (didRenderLastRender() || getControllingClient()))
   if (true) //XXTH for clientplayer
   {
      updateWorkingCollisionSet();

      updateState();
      updateMove(move);
      updateLookAnimation();
      updateDeathOffsets();
      updatePos();
      notifyCollision(); //extra!
   }

   if (!isGhost())
   {
      // Animations are advanced based on frame rate on the
      // client and must be ticked on the server.
      updateActionThread();
      updateAnimationTree(true);
   }

}

void ClientPlayer::interpolateTick(F32 dt)
{
   // Client side interpolation
   ShapeBase::interpolateTick(dt);

   mHead = mDelta.head;
   setRenderPosition(mDelta.pos, mDelta.rot, 0);
   updateLookAnimation();
   mDelta.dt = dt;

}

void ClientPlayer::setSkinName(const char* name)
{
   /* ???
   StringHandle     mSkinNameHandle;
   if (name[0] != '\0') {

      // Use tags for better network performance
      // Should be a tag, but we'll convert to one if it isn't.
      if (name[0] == StringTagPrefixByte) {
         mSkinNameHandle = StringHandle(U32(dAtoi(name + 1)));
      }
      else {
         mSkinNameHandle = StringHandle(name);
      }
   }
   else {
      mSkinNameHandle = StringHandle();
   }
   mShapeInstance->reSkin(mSkinNameHandle);
   */

   String mSkinNameHandle;
   mSkinNameHandle = name;
   mShapeInstance->reSkin(mSkinNameHandle);
}



/*
DefineEngineStringlyVariadicMethod(ClientPlayer, setSpeedModifier, void, 3, 3, "(S32 speedModifier default=15 is datablock speed (0..63)") {
   object->setSpeedModifier(dAtoi(argv[2]));
}

DefineEngineStringlyVariadicMethod(ClientPlayer, setPlayerCantMove, void, 3, 3, "(bool CantMove)") {
   object->setPlayerCantMove(dAtob(argv[2]));
}

DefineEngineStringlyVariadicMethod(ClientPlayer, getPlayerCantMove, bool, 2, 2, "(return bool CantMove)") {
   return object->getPlayerCantMove();
}

DefineEngineStringlyVariadicMethod(ClientPlayer, getSpeedModifier, S32, 2, 2, "(return S32 speedModifier)") {
   return object->getSpeedModifier();
}

//1.98 added at client
DefineEngineStringlyVariadicMethod(ClientPlayer, setCanFly, void, 3, 3, "(bool CanFly)") {
   bool enable = dAtob(argv[2]);
   object->setCanFly(enable);
   if (enable)
   {
      object->setGravity(0.f);
      Con::printf("Happy flight mode enabled");
   }
   else {
      object->setGravity(-20.f);
      Con::printf("Flight mode disabled");
   }
}
*/

DefineEngineStringlyVariadicMethod(ClientPlayer, setControlClientTS, void, 3, 3, "(ClientTSCTRL object)")
{
   ClientTSCtrl* gb;
   if (!Sim::findObject(argv[2], gb))
   {
      object->setClientTS(NULL);
      return;
   }

   object->setClientTS(gb);

}
//==============================================================================================================
IMPLEMENT_CO_NETOBJECT_V1(ClientAIPlayer);

/**
 * Constructor
 */
ClientAIPlayer::ClientAIPlayer()
{
   mMoveDestination.set(0.0f, 0.0f, 0.0f);
   mMoveSpeed = 1.0f;
   mMoveTolerance = 0.25f;
   mMoveSlowdown = true;

   mAimObject = 0;
   mAimLocationSet = false;
   mTargetInLOS = false;


}

/**
 * Destructor
 */
ClientAIPlayer::~ClientAIPlayer()
{
}

/**
 * Sets the speed at which this AI moves
 *
 * @param speed Speed to move, default player was 10
 */
void ClientAIPlayer::setMoveSpeed(F32 speed)
{
   mMoveSpeed = getMax(0.0f, getMin(1.0f, speed));
}

/**
 * Stops movement for this AI
 */
void ClientAIPlayer::stopMove()
{
   mMoveState = ModeStop;
}

/**
 * Sets how far away from the move location is considered
 * "on target"
 *
 * @param tolerance Movement tolerance for error
 */
void ClientAIPlayer::setMoveTolerance(const F32 tolerance)
{
   mMoveTolerance = getMax(0.1f, tolerance);
}

/**
 * Sets the location for the bot to run to
 *
 * @param location Point to run to
 */
void ClientAIPlayer::setMoveDestination(const Point3F& location, bool slowdown)
{
   mMoveDestination = location;
   mMoveState = ModeMove;
   mMoveSlowdown = slowdown;
}

/**
 * Sets the object the bot is targeting
 *
 * @param targetObject The object to target
 */
void ClientAIPlayer::setAimObject(GameBase* targetObject)
{
   mAimObject = targetObject;
   mTargetInLOS = false;
}

/**
 * Sets the location for the bot to aim at
 *
 * @param location Point to aim at
 */
void ClientAIPlayer::setAimLocation(const Point3F& location)
{
   mAimObject = 0;
   mAimLocationSet = true;
   mAimLocation = location;
}

/**
 * Clears the aim location and sets it to the bot's
 * current destination so he looks where he's going
 */
void ClientAIPlayer::clearAim()
{
   mAimObject = 0;
   mAimLocationSet = false;
}

/**
 * This method calculates the moves for the AI player
 *
 * @param movePtr Pointer to move the move list into
 */
bool ClientAIPlayer::getAIMove(Move* movePtr)
{
   *movePtr = NullMove;

   Point3F location = getPosition();
   Point3F rotation = getRotation();

   // Orient towards the aim point, aim object, or towards
   // our destination.
   if (mAimObject || mAimLocationSet || mMoveState == ModeMove) {

      // Update the aim position if we're aiming for an object
      if (mAimObject)
         mAimLocation = mAimObject->getPosition();
      else
         if (!mAimLocationSet)
            mAimLocation = mMoveDestination;

      F32 xDiff = mAimLocation.x - location.x;
      F32 yDiff = mAimLocation.y - location.y;
      if (!mIsZero(xDiff) || !mIsZero(yDiff)) {

         // First do Yaw
         // use the cur yaw between -Pi and Pi
         F32 curYaw = rotation.z;
         while (curYaw > M_2PI)
            curYaw -= M_2PI;
         while (curYaw < -M_2PI)
            curYaw += M_2PI;

         // find the yaw offset
         F32 newYaw = mAtan2(xDiff, yDiff);
         F32 yawDiff = newYaw - curYaw;

         // make it between 0 and 2PI
         if (yawDiff < 0.0f)
            yawDiff += M_2PI;
         else if (yawDiff >= M_2PI)
            yawDiff -= M_2PI;

         // now make sure we take the short way around the circle
         if (yawDiff > M_PI)
            yawDiff -= M_2PI;
         else if (yawDiff < -M_PI)
            yawDiff += M_2PI;

         movePtr->yaw = yawDiff;

         // Next do pitch.
         if (!mAimObject && !mAimLocationSet) {
            // Level out if were just looking at our next way point.
            Point3F headRotation = getHeadRotation();
            movePtr->pitch = -headRotation.x;
         }
         else {
            // This should be adjusted to run from the
            // eye point to the object's center position. Though this
            // works well enough for now.
            F32 vertDist = mAimLocation.z - location.z;
            F32 horzDist = mSqrt(xDiff * xDiff + yDiff * yDiff);
            F32 newPitch = mAtan2(horzDist, vertDist) - (M_PI / 2.0f);
            if (mFabs(newPitch) > 0.01) {
               Point3F headRotation = getHeadRotation();
               movePtr->pitch = newPitch - headRotation.x;
            }
         }
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

      // Check if we should mMove, or if we are 'close enough'
      if (mFabs(xDiff) < mMoveTolerance && mFabs(yDiff) < mMoveTolerance) {
         mMoveState = ModeStop;
         throwCallback("onReachDestination");
      }
      else {
         // Build move direction in world space
         if (mIsZero(xDiff))
            movePtr->y = (location.y > mMoveDestination.y) ? -1 : 1;
         else
            if (mIsZero(yDiff))
               movePtr->x = (location.x > mMoveDestination.x) ? -1 : 1;
            else
               if (mFabs(xDiff) > mFabs(yDiff)) {
                  F32 value = mFabs(yDiff / xDiff);
                  movePtr->y = (location.y > mMoveDestination.y) ? -value : value;
                  movePtr->x = (location.x > mMoveDestination.x) ? -1 : 1;
               }
               else {
                  F32 value = mFabs(xDiff / yDiff);
                  movePtr->x = (location.x > mMoveDestination.x) ? -value : value;
                  movePtr->y = (location.y > mMoveDestination.y) ? -1 : 1;
               }

         // Rotate the move into object space (this really only needs
         // a 2D matrix)
         Point3F newMove;
         MatrixF moveMatrix;
         moveMatrix.set(EulerF(0, 0, -(rotation.z + movePtr->yaw)));
         moveMatrix.mulV(Point3F(movePtr->x, movePtr->y, 0), &newMove);
         movePtr->x = newMove.x;
         movePtr->y = newMove.y;

         // Set movement speed.  We'll slow down once we get close
         // to try and stop on the spot...
         if (mMoveSlowdown) {
            F32 speed = mMoveSpeed;
            F32 dist = mSqrt(xDiff * xDiff + yDiff * yDiff);
            F32 maxDist = 5;
            if (dist < maxDist)
               speed *= dist / maxDist;
            movePtr->x *= speed;
            movePtr->y *= speed;
         }
         else {
            movePtr->x *= mMoveSpeed;
            movePtr->y *= mMoveSpeed;
         }

         // We should check to see if we are stuck...
         if (location == mLastLocation) {
            throwCallback("onMoveStuck");
            mMoveState = ModeStop;
         }
      }
   }

   // Test for target location in sight if it's an object. The LOS is
   // run from the eye position to the center of the object's bounding,
   // which is not very accurate.
   if (mAimObject) {
      MatrixF eyeMat;
      getEyeTransform(&eyeMat);
      eyeMat.getColumn(3, &location);
      Point3F targetLoc = mAimObject->getBoxCenter();

      // This ray ignores non-static shapes. Cast Ray returns true
      // if it hit something.
      RayInfo dummy;
      if (getContainer()->castRay(location, targetLoc,
          StaticShapeObjectType | StaticObjectType | TerrainLikeObjectType | InteriorLikeObjectType |
         TerrainObjectType, &dummy)) {
         if (mTargetInLOS) {
            throwCallback("onTargetExitLOS");
            mTargetInLOS = false;
         }
      }
      else
         if (!mTargetInLOS) {
            throwCallback("onTargetEnterLOS");
            mTargetInLOS = true;
         }
   }

   // Replicate the trigger state into the move so that
   // triggers can be controlled from scripts.
   for (int i = 0; i < MaxTriggerKeys; i++)
      movePtr->trigger[i] = getImageTriggerState(i);

   return true;
}

/**
 * Utility function to throw callbacks. Callbacks always occure
 * on the datablock class.
 *
 * @param name Name of script function to call
 */
void ClientAIPlayer::throwCallback(const char* name)
{
   Con::executef(getDataBlock(),  name, getIdString());
}


// --------------------------------------------------------------------------------------------
// Console Functions
// --------------------------------------------------------------------------------------------

DefineEngineStringlyVariadicMethod(ClientAIPlayer, stop, void, 2, 2, "()"
   "Stop moving.")
{
   object->stopMove();
}

DefineEngineStringlyVariadicMethod(ClientAIPlayer, clearAim, void, 2, 2, "()"
   "Stop aiming at anything.")
{
   object->clearAim();
}

DefineEngineStringlyVariadicMethod(ClientAIPlayer, setMoveSpeed, void, 3, 3, "( float speed )"
   "Sets the move speed for an AI object.")
{
   object->setMoveSpeed(dAtof(argv[2]));
}

DefineEngineStringlyVariadicMethod(ClientAIPlayer, setMoveDestination, void, 3, 4, "(Point3F goal, bool slowDown=true)"
   "Tells the AI to move to the location provided.")
{
   Point3F v(0.0f, 0.0f, 0.0f);
   dSscanf(argv[2], "%f %f %f", &v.x, &v.y, &v.z);
   bool slowdown = (argc > 3) ? dAtob(argv[3]) : true;
   object->setMoveDestination(v, slowdown);
}

DefineEngineStringlyVariadicMethod(ClientAIPlayer, getMoveDestination, const char*, 2, 2, "()"
   "Returns the point the AI is set to move to.")
{
   Point3F movePoint = object->getMoveDestination();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%f %f %f", movePoint.x, movePoint.y, movePoint.z);

   return returnBuffer;
}

DefineEngineStringlyVariadicMethod(ClientAIPlayer, setAimLocation, void, 3, 3, "( Point3F target )"
   "Tells the AI to aim at the location provided.")
{
   Point3F v(0.0f, 0.0f, 0.0f);
   dSscanf(argv[2], "%f %f %f", &v.x, &v.y, &v.z);

   object->setAimLocation(v);
}

DefineEngineStringlyVariadicMethod(ClientAIPlayer, getAimLocation, const char*, 2, 2, "()"
   "Returns the point the AI is aiming at.")
{
   Point3F aimPoint = object->getAimLocation();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer, 256, "%f %f %f", aimPoint.x, aimPoint.y, aimPoint.z);

   return returnBuffer;
}

DefineEngineStringlyVariadicMethod(ClientAIPlayer, setAimObject, void, 3, 3, "( GameBase obj )"
   "Sets the bot's target object.")
{
   // Find the target
   GameBase* targetObject;
   if (Sim::findObject(argv[2], targetObject))
      object->setAimObject(targetObject);
   else
      object->setAimObject(0);
}

DefineEngineStringlyVariadicMethod(ClientAIPlayer, getAimObject, S32, 2, 2, "()"
   "Gets the object the AI is targeting.")
{
   GameBase* obj = object->getAimObject();
   return obj ? obj->getId() : -1;
}

