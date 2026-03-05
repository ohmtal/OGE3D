#pragma message("********************************************************")
#pragma message("**** tomEntity untested and loooooots of fixme .... ****")
#pragma message("********************************************************")

//-----------------------------------------------------------------------------
// Copyright (c) 2009/2023 huehn-software / Ohmtal Game Studio
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
// ToMEntity by t.huehn (XXTH) (c) huehn-software 2009
//-----------------------------------------------------------------------------

#include "tomEntity.h"
#include "ohmtal/ai/aiSteering.h"
#include "ohmtal/ai/aiMath.h"
#include "ohmtal/ai/fsm/StateMachine.h"
#include "ohmtal/ai/aiStates.h"
#include "T3D/projectile.h"
#include "sim/netconnection.h"
#include "scene/simPath.h"


// Client prediction
static F32 sMinWarpTicks =  3.5f; //XXTH 0.5f; dont know why but need more here, else rotation is out of sync ?!       // Fraction of tick at which instant warp occures
static S32 sMaxWarpTicks = 3;          // Max warp duration in ticks
static S32 sMaxPredictionTicks = 30;   // Number of ticks to predict
static F32 sAnimationTransitionTime = 0.25f;
static const S32 sNewAnimationTickTime = 4;


IMPLEMENT_CO_DATABLOCK_V1(ToMEntityData);
IMPLEMENT_CO_NETOBJECT_V1(ToMEntity);
//----------------------------------------------------------------------------
// ****************************** ToMEntityData ******************************
//----------------------------------------------------------------------------
ToMEntityData::ToMEntityData()
{


   mDisplayname = StringTable->insert(""); 
   mMinipic     = StringTable->insert(""); 
   mHint        = StringTable->insert(""); 
   mDummy       = StringTable->insert(""); 
   mBaseTex     = "";

   mMoveSpeed   = 0;

   mEntityId    = -1; 
   mIsBuilding  = false;
   mIsTower     = false;
   mTowerShootSpell = ""; //Name of spell.. spell object will be looked up in ToMHandler::onTowerTarget
   mCachedTowerShootSpell = NULL;
   mLingerSpell = "";
   mCachedLingerSpell = NULL;


   mStartDamage = 0;
   mStartAttackDelay = 0;
   mStartRange  = 0;

   mTowerAddDamage = 0;
   mTowerReduceDelay = 0;
   mTowerAddRange = 0;

   mUnitMaxDamageQuotient = 0;

   mParams.init();

   mMaxUpgrades = 0;

   for (U8 i = 0; i < MAX_TowerUpgradeType; i++)
   {
	   for (U8 j = 0; j < TOM_MAXTOWERUPGRADES; j++)
			mUpgradeCosts[i][j].init();
	  mTowerLevel[i] = 0;
   }
   mUnitLevel = 0;


}
//----------------------------------------------------------------------------
void ToMEntityData::updateUnitLevel(U8 lLevel)
{
    //geo progression for maxdamage
	maxDamage  = mRound((F32)mStartDamage * mPow(mUnitMaxDamageQuotient,lLevel));
	mUnitLevel = lLevel;
}
//----------------------------------------------------------------------------
ToMEntityParams* ToMEntityData::getUpgradeParams(TowerUpgradeType lType)
{
  U8 lNextLevel = mTowerLevel[lType]+1;
  if ( lNextLevel > mMaxUpgrades) //TOM_MAXTOWERUPGRADES )
	  return NULL;

  return &mUpgradeCosts[lType][lNextLevel-1];


}
//----------------------------------------------------------------------------
bool ToMEntityData::updateTowerLevel(TowerUpgradeType lType)
{

	U8 lNextLevel = mTowerLevel[lType]+1;
    if ( lNextLevel > mMaxUpgrades ) //TOM_MAXTOWERUPGRADES )
	  return NULL;
	updateTowerLevel(lType,lNextLevel);
	return true;
}
//----------------------------------------------------------------------------
void ToMEntityData::updateTowerLevel(TowerUpgradeType lType, U8 lLevel)
{
	switch (lType)
	{
	case TowerDamage:
			mBaseDamage  = mStartDamage + lLevel * mTowerAddDamage;
		break;
	case TowerRange:
			mRange       = mStartRange + lLevel * mTowerAddRange;
			mVision	     = mRange;
			mAttackRange = mRange;
		break;
	case TowerAttackDelay:
			mAttackDelay = mRound((F32)(mStartAttackDelay - lLevel * mTowerReduceDelay) * 0.032f);
			if (mAttackDelay < 8)
				mAttackDelay = 8; //minmum of 8 ticks! ==> 250ms
		break;

	}
	mTowerLevel[lType]=lLevel;
}
//----------------------------------------------------------------------------

void ToMEntityData::buildHint()
{
//FIXME 	mHint      = StringTable->insert(gToMHandler->GetEnityBuildParamsHint(&mParams));
}

//-----------------------------------------------------------------------------
void ToMEntityData::initPersistFields()
{   
#ifdef TORQUE_DEBUG //hide fields on release!
	Parent::initPersistFields();
#endif
/* FIXME 
	addProtectedField("miniPic",		TypeString, Offset(mMinipic, ToMEntityData), setDummy, staticGetMinipic,"" );
	addProtectedField("DisplayName",	TypeString, Offset(mDisplayname, ToMEntityData), setDummy, staticGetDisplayName,"" );
	addProtectedField("DisplayInfo",	TypeString, Offset(mDummy, ToMEntityData), setDummy, staticGetDisplayInfo,"" );
	addProtectedField("hint",			TypeString, Offset(mHint, ToMEntityData), setDummy, staticGetHint,"" );
	addProtectedField("isBuilding",		TypeBool,	Offset(mIsBuilding, ToMEntityData), setDummy, staticGetIsBuilding, ""   );
*/
}
/*
char* ToMEntityData::getDisplayInfo()
{
   static char buf[256];
   buf[255] = NULL;
   if (mIsBuilding)
   {
	   if (!mIsTower)
			return "";

	   dSprintf(buf,sizeof(buf),"<just:left>%s<just:right>%d (%d)<br><just:left>%s<just:right>%d (%d)<br><just:left>%s<just:right>%d (%d)<br>",
		   gToMHandler->T("Damage"),(S32)mBaseDamage, (S32)mTowerLevel[0]+1,
		   gToMHandler->T("Range"),(S32)mVision,(S32)mTowerLevel[1]+1,
		   gToMHandler->T("Interval"),(S32)(mAttackDelay*3.2f), (S32)mTowerLevel[2]+1
	   );

	   return buf;
   } else {
	   dSprintf(buf,sizeof(buf),"<just:left>%s<just:right>%d<br><just:left>%s<just:right>%d<br><just:left>%s<just:right>%d<br>",
		   gToMHandler->T("Health"),(S32)maxDamage,gToMHandler->T("Gold"),(S32)mParams.a_money,gToMHandler->T("Damage"),mBaseDamage);

       return buf;
   }

  return "";

}
*/

//----------------------------------------------------------------------------
// ****************************** ToMEntity  ******************************
//----------------------------------------------------------------------------
ToMEntity::ToMEntity()
{
	 mMoveBlocked	= false;
     mOverwriteAnim = false;

	 mIsPathWalking = false;
	 mPathObject    = NULL;
	 mCurPathNodeIndex = -1;

	 mWaveManager   = NULL;

}
//----------------------------------------------------------------------------
ToMEntity::~ToMEntity()
{
}
//-----------------------------------------------------------------------------
void ToMEntity::initPersistFields()
{   
	Parent::initPersistFields();
	/*
    addGroup("ToM");
		addField("isTower",			 TypeBool,     Offset(mIsTower, ToMEntity)        );
    endGroup("ToM");
	*/
}

//------------------------------------------------------------------------------------------------------
bool ToMEntity::onNewDataBlock(GameBaseData* dptr, bool reload)
{
   mDataBlock = static_cast<ToMEntityData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   return true;
}

//------------------------------------------------------------------------------------------------------
bool ToMEntity::onAdd()
{
   if (!Parent::onAdd())
      return false;

   initAnimations();

	if (mDataBlock->mIsBuilding) 
	{
		if (action_idle >=0) 
			setActionThread(action_idle);
	}
	//FIXME overrideOptions = false; //we use ambiente !!!

	//back to datablock maxdamage!
   //FIXME setMaxDamage(mDataBlock->maxDamage);

   return true;
}

// --------------------------------------------------------------------------------------------
S32 ToMEntity::getActionbyName(const char* sequence)
{
   for (U32 i = 1; i < mDataBlock->actionCount; i++)
   {
      PlayerData::ActionAnimation& anim = mDataBlock->actionList[i];
      if (!dStricmp(anim.name, sequence))
      {
         return i;
      }
   }

   return -1;
}

//----------------------------------------------------------------------------
void ToMEntity::initAnimations() {
    action_idle			= getActionbyName("idle");
    action_walk			= getActionbyName("walk");
    action_attack		= getActionbyName("attack");
    action_death		= getActionbyName("death");
}
//----------------------------------------------------------------------------
void ToMEntity::GuessActionAnimation() {

	if (mDataBlock->mIsBuilding) {
		return;
	}
	if (mDamageState != Enabled ) { 
			setActionThread(action_death,true,true);
			return;
	}

	if (mOverwriteAnim ) {
		if (mActionAnimation.atEnd) {
			mOverwriteAnim = false;
		} else {
		  return;
		}
	}

	if (mMoveBlocked) {
		mMoveBlocked = false;
		setPlayerCantMove(false);
	}


  Point3F vel = getVelocity();
  if (mFabs(vel.x)>0.1f ||  mFabs(vel.y)>0.1f) {
		setActionThread(action_walk);
  } else {
		setActionThread(action_idle);
  }

}
// --------------------------------------------------------------------------------------------
void ToMEntity::pickActionAnimation()
{
   //XXTH blah! if (!isGhost()) 
   if (isGhost()) 
		GuessActionAnimation();
}
// --------------------------------------------------------------------------------------------
bool ToMEntity::setActionThread(const char* sequence, bool setOverwriteAnim, bool moveBlocked)
{

	S32 action=getActionbyName(sequence);
	if (action>=0 && action < mDataBlock->actionCount) {
		if (setActionThread(action,setOverwriteAnim, moveBlocked)) { 
         return true;
		}

	}
	return false;
}
// --------------------------------------------------------------------------------------------
bool ToMEntity::setActionThread(S32 action,  bool setOverwriteAnim, bool moveBlocked)
{

   if (action < 0 || action > mDataBlock->actionCount)
   {
#ifdef TORQUE_DEBUG
      Con::errorf("ToMEntity::setActionThread(%d): ToMEntity action out of range", action);
#endif
      return false;
   }

	bool forward = true;
	bool hold = false;
	bool wait = true;
	//bool forceSet = false;
	bool fsp = true; 

    if (mlastAnimationAction == action) return false;

	if (mDamageState != Enabled &&  action != action_death)
			return false;
	if (mMoveBlocked && !moveBlocked) return false;


   PlayerData::ActionAnimation &anim = mDataBlock->actionList[action];
   if (anim.sequence != -1)
   {
      mActionAnimation.action          = action;
      mActionAnimation.forward         = forward;
      mActionAnimation.firstPerson     = fsp;
      mActionAnimation.holdAtEnd       = hold;
      mActionAnimation.waitForEnd      = hold? true: wait;
      mActionAnimation.animateOnServer = fsp;
      mActionAnimation.atEnd           = false;
      mActionAnimation.delayTicks      = sNewAnimationTickTime;
      mActionAnimation.atEnd           = false;

      if ( (isGhost()))
      {
         // The transition code needs the timeScale to be set in the
         // right direction to know which way to go.
         F32   transTime = sAnimationTransitionTime;



         mShapeInstance->setTimeScale(mActionAnimation.thread,
            mActionAnimation.forward? 1: -1);
         mShapeInstance->transitionToSequence(mActionAnimation.thread,anim.sequence,
            mActionAnimation.forward? 0: 1, transTime, true);
      }
      else
         mShapeInstance->setSequence(mActionAnimation.thread,anim.sequence,
            mActionAnimation.forward? 0: 1);


		char buf1[128];
		dSprintf(buf1,sizeof(buf1),"%s",anim.name);
		Con::executef(mDataBlock,"onNewAnimation", getIdString(),buf1);

        mlastAnimationAction = action;
		mOverwriteAnim = setOverwriteAnim;
	    if (moveBlocked && mOverwriteAnim) {
			if (!mMoveBlocked) {
				setPlayerCantMove(true);
			}
			mMoveBlocked = true; //must be allways called after that movemodifier
		}
        if (!isGhost()) 
			setMaskBits(ActionMask);

        return true;
   }
   return false;
}
//------------------------------------------------------------------------------------------------------
void ToMEntity::stopMove()
{
   if (getMoveState() == ModeStop)
      return;

   
   setMoveState(ModeStop);
//   pickActionAnimation();

  if (isServerObject()) 
	   setMaskBits(CharMask | MoveMask);

   mVelocity.set(0.0f, 0.0f, 0.0f);

}
//------------------------------------------------------------------------------------------------------
void ToMEntity::setMoveDestination( const Point3F &location  )
{
	if (!isServerObject()) return;
   // Check if we're still alive.
   if(getDamageState() == Enabled)
   {
      
      setMoveDestination(location);
      setMoveState(ModeMove);
//	  pickActionAnimation();
	  setMaskBits(CharMask);
   } else 
   {
      stopMove();
//	  pickActionAnimation(); 
   }
}

//------------------------------------------------------------------------------------------------------
void ToMEntity::beginProcessTick(const Move *move)
{
   // In fact, RTSUnits don't do moves at all. ;)
   resetWorldBox();

   if (isServerObject() && mDataBlock->maxDamage != getMaxDamage())
		setMaxDamage(mDataBlock->maxDamage);

   //--------------------------------------------------
   // Shapebase Stuff
   //--------------------------------------------------
   // Energy management
   if (mDamageState == Enabled && mDataBlock->inheritEnergyFromMount == false) {
      F32 store = mEnergy;
      mEnergy += mRechargeRate;
      if (mEnergy > mDataBlock->maxEnergy)
         mEnergy = mDataBlock->maxEnergy;
      else
         if (mEnergy < 0)
            mEnergy = 0;

      if (mEnergy != store)
         setEnergyLevel(mEnergy);
   }

   // Repair management
   if (mDataBlock->isInvincible == false) 
   {
      F32 store = mDamage;
      mDamage -= mRepairRate;
      mDamage = mClampF(mDamage, 0.f, mDataBlock->maxDamage);

      if (mRepairReserve > mDamage)
         mRepairReserve = mDamage;
      if (mRepairReserve > 0.0)
      {
         F32 rate = getMin(mDataBlock->repairRate, mRepairReserve);
         mDamage -= rate;
         mRepairReserve -= rate;
      }

      if (store != mDamage)
      {
         updateDamageLevel();
         if (isServerObject()) {
			 /*
            char delta[100];
            dSprintf(delta,sizeof(delta),"%f",mDamage - store);
            setMaskBits(DamageMask);
            Con::executef(mDataBlock,"onDamage",scriptThis(),delta);
			*/
         }
      }
   }


   if (isServerObject()) {
      // Server only...
	   //---------------
	   // tower power ;)
		if (getIsTower()) {
			processTower();
		}
		//---------------

      advanceThreads(TickSec);
      updateServerAudio();
      if(mFading)
      {
         F32 dt = TickMs / 1000.0f;
         F32 newFadeET = mFadeElapsedTime + dt;
         if(mFadeElapsedTime < mFadeDelay && newFadeET >= mFadeDelay)
            setMaskBits(CharMask); //orig; SpecialPowerMask);
         mFadeElapsedTime = newFadeET;
         if(mFadeElapsedTime > mFadeTime + mFadeDelay)
         {
            mFadeVal = F32(!mFadeOut);
            mFading = false;
         }
      }

   }
   //--------------------- < shapebase stuff -------------------------

   ///XXTH dont need this here!
   /*
   if (mDataBlock->mDoLookAnimation)
   {
      updateLookAnimation();
   }
   updateDeathOffsets();
   */
   if (!isGhost())
      updateAnimation(TickSec);
}
//------------------------------------------------------------------------------------------------------
void ToMEntity::processTick(const Move *move)
{
	if (mDataBlock->mIsBuilding && !getIsTower())
		return;
	Parent::processTick(move);
}
//------------------------------------------------------------------------------------------------------
void ToMEntity::doMove(VectorF lTargetLocation,Point3F &location)
{

	Point3F posXY(location.x, location.y, 0);
	lTargetLocation.z=0;
	VectorF goalDelta = lTargetLocation - posXY;
	F32 lCloseDist;
	lCloseDist = 0.1f;
		

	if (goalDelta.len() <= lCloseDist) {

		stopMove();
		mVelocity.set(0,0,0);

      if (getIsPathWalking())
      {
         if (isServerObject())
         {
            Con::executef(this,  "onReachDestination");
         }
         else {
            Con::executef(this, "ClientUnitReachDestination", this->getIdString());
         }

      }

		return;
	} //close

	if (isServerObject() && getMoveState() == ModeMove) {
		// Cap speed...
		F32 v = 0;
		v = getMaxForwardVelocity() * TickSec;
                
			if(goalDelta.len() > v)
				{
					goalDelta.normalize();
					goalDelta *= v;
				}
				// Set velocity to make the animation system play stuff
				mVelocity = goalDelta / TickSec;
				location += goalDelta;

//  	    if (mVelocity.len() > 0.01f) 
//		   pickActionAnimation();

		setMaskBits(MoveMask);
		 
	} //isServer
			
}

//------------------------------------------------------------------------------------------------------

U32 ToMEntity::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = GameBase::packUpdate(con, mask, stream);

   if (mask & InitialUpdateMask)
   {
      // mask off sounds that aren't playing
      S32 i;
      for (i = 0; i < MaxSoundThreads; i++)
         if (!mSoundThread[i].play)
            mask &= ~(SoundMaskN << i);

      // mask off threads that aren't running
      for (i = 0; i < MaxScriptThreads; i++)
         if (mScriptThread[i].sequence == -1)
            mask &= ~(ThreadMaskN << i);

      // mask off images that aren't updated
      for(i = 0; i < MaxMountedImages; i++)
         if(!mMountedImageList[i].dataBlock)
            mask &= ~(ImageMaskN << i);
   }


   if(!stream->writeFlag(mask & (DamageMask | SoundMask | ThreadMask | SkinMask |
      ActionMask | CharMask | InitialUpdateMask |  ImpactMask | 
      MoveMask )))
      return retMask;

   // ShapeBase stuff
   if (stream->writeFlag((mask & ThreadMask) && (mDamageState != Destroyed)))
   {
      for (int i = 0; i < MaxScriptThreads; i++)
      {
         Thread& st = mScriptThread[i];
         if (stream->writeFlag(st.sequence != -1 && (mask & (ThreadMaskN << i))))
         {
            stream->writeInt(st.sequence,ThreadSequenceBits);
            stream->writeInt(st.state,2);
            //??? stream->writeFlag(st.forward);
            stream->writeFlag(st.atEnd);
         }
      }
   }

   if (stream->writeFlag(mask & DamageMask))
   {
      stream->writeFloat(mClampF(mDamage / mDataBlock->maxDamage, 0.f, 1.f), DamageLevelBits);
      stream->writeInt(mDamageState,NumDamageStateBits);
   }

   if (stream->writeFlag(mask & SoundMask))
   {
   }

   if(stream->writeFlag(mask & SkinMask))
      con->packNetStringHandleU(stream, mSkinNameHandle);

   //added 2023-01-03
   if (stream->writeFlag(mask & CharMask)) {
       stream->writeInt(getPlayerType(), 3); //3 bits are enought for 0..7 types :P
   }

   // Player stuff
   if (stream->writeFlag((mask & ImpactMask) && !(mask & InitialUpdateMask)))
      stream->writeInt(mImpactSound, PlayerData::ImpactBits);

   if (stream->writeFlag(mask & ActionMask &&
      mActionAnimation.action != PlayerData::NullAnimation &&
      mActionAnimation.action >= PlayerData::NumTableActionAnims))
   {
      stream->writeInt(mActionAnimation.action,PlayerData::ActionAnimBits);
      stream->writeFlag(mActionAnimation.holdAtEnd);
      stream->writeFlag(mActionAnimation.atEnd);
      stream->writeFlag(mActionAnimation.firstPerson);
      if (!mActionAnimation.atEnd)
      {
         // If somewhere in middle on initial update, must send position-
         F32 where = mShapeInstance->getPos(mActionAnimation.thread);
         if (stream->writeFlag((mask & InitialUpdateMask) != 0 && where > 0))
            stream->writeSignedFloat(where, 5);
      }
   }

   if (stream->writeFlag(mask & ActionMask &&
      mArmAnimation.action != PlayerData::NullAnimation &&
      (!(mask & InitialUpdateMask) ||
      mArmAnimation.action != mDataBlock->lookAction)))
   {
      stream->writeInt(mArmAnimation.action,PlayerData::ActionAnimBits);
   }

   if (stream->writeFlag(mask & MoveMask))
   {
      stream->writeFlag(mFalling);

      stream->writeInt(mState,NumStateBits);

      Point3F pos;
      getTransform().getColumn(3,&pos);
      stream->writeCompressedPoint(pos, 0.1f);
//FIXME      setDirty();


      F32 len = mVelocity.len();
      if(stream->writeFlag(len > 0.02))
      {
         Point3F outVel = mVelocity;
         outVel *= 1/len;
         stream->writeNormalVector(outVel, 8);
         len *= 32.0;  // 5 bits of fraction
         if(len > 8191)
            len = 8191;
         stream->writeInt((S32)len, 13);
      }
//XXTH stream->writeFloat(mRot.z / M_2PI_F, 7); //6);
	  stream->write(mRot.z); 

     mDelta.move.pack(stream);
      stream->writeFlag(!(mask & NoWarpMask));
   }

   // Ghost needs energy to predict reliably (but not in RTS -- BJG)
   //   stream->writeFloat(getEnergyLevel() / mDataBlock->maxEnergy,EnergyLevelBits);

   // Transmit AI state.
   if (stream->writeFlag(mask & CharMask))
   {
/* XXTH FIXME sucks ?!
      stream->writeInt(getMoveState(), 2);

      if(true ) //mMoveState !=  ModeStop )//XXTH allways else it get out of sync! bleh ! 
      {
         // If we didn't previously write a position, send it over.
         stream->writeCompressedPoint(getMoveDestination(), 0.1f);

      }

      if ( getAimObject())
      {
         S32 ghostId = con->getGhostIndex(getAimObject());
         if (stream->writeFlag(ghostId != -1))
            stream->writeInt(ghostId, NetConnection::GhostIdBitSize);
      }
      else
         stream->writeFlag(false);

      if(stream->writeFlag(mAimLocationSet))
      {
         stream->writeCompressedPoint(mAimLocation, 0.1f);
      }
*/
   }

/* FIXME ?!? !
   if (stream->writeFlag(mask & ModifierMask))
   {
      AssertISV(mModifierList.size() < 32, "Max number of active modifiers on a unit is 16!");
      stream->writeInt(mModifierList.size(), 5);
      for (U32 k = 0; k < mModifierList.size(); k++)
      {
         stream->write(mModifierList[k]->getId());
      }
   }
*/
   // Team and class
   if(stream->writeFlag(mask & InitialUpdateMask))
   {
/* FIXME ?!?! 
      AssertISV(getTeam() < 64, "Can only pack 64 unique teams!");
      stream->writeInt(getTeam(), 5);
*/
   }

   // Projectile datablock
/* FIXME 
   if(stream->writeFlag((mask & ProjectileMask) && (mCurrentProjectile != NULL)))
   {
      stream->writeRangedU32(mCurrentProjectile->getId(),
         DataBlockObjectIdFirst,
         DataBlockObjectIdLast);
   }
*/

   // Fade out on death
   if (stream->writeFlag(mask & CharMask)) 
   {
      // cloaking
      stream->writeFlag( mCloaked );

      // fading
      if(stream->writeFlag(mFading && mFadeElapsedTime >= mFadeDelay)) {
         stream->writeFlag(mFadeOut);
         stream->write(mFadeTime);
      }
      else
         stream->writeFlag(mFadeVal == 1.0f);
   }

   return retMask;
}
//--------------------------------------------------------------------------------------------------
void ToMEntity::unpackUpdate(NetConnection *con, BitStream *stream)
{
   GameBase::unpackUpdate(con,stream);

   mLastRenderFrame = sLastRenderFrame; // make sure we get a process after the event...

   if(!stream->readFlag())
      return;

   // ShapeBase stuff
   if (stream->readFlag())
   {
      for (S32 i = 0; i < MaxScriptThreads; i++)
      {
         if (stream->readFlag())
         {
            Thread& st = mScriptThread[i];
            U32 seq = stream->readInt(ThreadSequenceBits);
            st.state = Thread::State(stream->readInt(2));
            //XXTH unknown ?! st.forward = stream->readFlag();
            st.atEnd = stream->readFlag();
            if (st.sequence != seq)
               setThreadSequence(i,seq,false);
            else
               updateThread(st);
         }
      }
   }

   if (stream->readFlag())
   {
      mDamage = mClampF(stream->readFloat(DamageLevelBits) * mDataBlock->maxDamage, 0.f, mDataBlock->maxDamage);
	  //Entity need it from maxDamage of datablock mDamage = mClampF(stream->readFloat(DamageLevelBits) * mMaxDamage, 0.f, mMaxDamage);
      DamageState prevState = mDamageState;
      mDamageState = DamageState(stream->readInt(NumDamageStateBits));

      // Don't need dir for RTS.
      //stream->readNormalVector( &damageDir, 8 );

      if (prevState != Destroyed && mDamageState == Destroyed && isProperlyAdded())
         blowUp();

      updateDamageLevel();
      updateDamageState();
   }

   if (stream->readFlag())
   {
      // Don't need this for an RTS -- BJG
      /*
      for (S32 i = 0; i < MaxSoundThreads; i++)
      {
      if (stream->readFlag())
      {
      Sound& st = mSoundThread[i];
      if ((st.play = stream->readFlag()) == true)
      st.profile = (AudioProfile*) stream->readRangedU32(DataBlockObjectIdFirst,
      DataBlockObjectIdLast);
      if (isProperlyAdded())
      updateAudioState(st);
      }
      }*/
   }

   if (stream->readFlag())  // SkinMask
   {
      NetStringHandle skinDesiredNameHandle = con->unpackNetStringHandleU(stream);;
      if (mSkinNameHandle != skinDesiredNameHandle)
      {
         mSkinNameHandle = skinDesiredNameHandle;
         reSkin();
      }

   }
   //added 2023-01-03 auteriatypemask
   if (stream->readFlag())
   {
       S32 tmpplayerType = stream->readInt(3);
       setPlayerType(tmpplayerType);
   }


   // Player stuff
   if (stream->readFlag())
      mImpactSound = stream->readInt(PlayerData::ImpactBits);

   // Server specified action animation
   if (stream->readFlag())
   {
      U32 action = stream->readInt(PlayerData::ActionAnimBits);
      bool hold  = stream->readFlag();
      bool atEnd = stream->readFlag();
      bool fsp   = stream->readFlag();

      F32   animPos = -1.0;
      if (!atEnd && stream->readFlag())
         animPos = stream->readSignedFloat(5);

      if (isProperlyAdded())
      {
         setActionThread(action);
         bool  inDeath = inDeathAnim();
         if (atEnd)
         {
            mShapeInstance->clearTransition(mActionAnimation.thread);
            mShapeInstance->setPos(mActionAnimation.thread,
               mActionAnimation.forward ? 1 : 0);
            if (inDeath)
               mDeath.lastPos = 1.0;
         }
         else if (animPos > 0)
         {
            mShapeInstance->setPos(mActionAnimation.thread, animPos);
            if (inDeath)
               mDeath.lastPos = animPos;
         }

      }
      else
      {
         mActionAnimation.action      = action;
         mActionAnimation.holdAtEnd   = hold;
         mActionAnimation.atEnd       = atEnd;
         mActionAnimation.firstPerson = fsp;
      }
   }

   // Server specified arm animation
   if (stream->readFlag())
   {
      U32 action = stream->readInt(PlayerData::ActionAnimBits);
      if (isProperlyAdded())
         setArmThread(action);
      else
         mArmAnimation.action = action;
   }

   if (stream->readFlag())
   {
      mPredictionCount = sMaxPredictionTicks;
      mFalling = stream->readFlag();

      ActionState actionState = (ActionState)stream->readInt(NumStateBits);
      setState(actionState);

      Point3F pos,rot;
      stream->readCompressedPoint(&pos, 0.1f);

      F32 speed = mVelocity.len();
      if(stream->readFlag())
      {
         stream->readNormalVector(&mVelocity, 8);
         mVelocity *= stream->readInt(13) / 32.0f;
      }
      else
         mVelocity.set(0,0,0);


      rot.y = rot.x = 0;
      //XXTH rot.z = stream->readFloat(6) * M_2PI;
	  //XXTH  rot.z = stream->readFloat(7) * M_2PI_F;
	  stream->read(&rot.z);

      mDelta.move.unpack(stream);

      mDelta.head = mHead;
      mDelta.headVec.set(0,0,0);

      if (stream->readFlag() && isProperlyAdded())
      {
         // Determine number of ticks to warp based on the average
         // of the client and server velocities.
         mDelta.warpOffset = pos - mDelta.pos;
         F32 as = (speed + mVelocity.len()) * 0.5f * TickSec;
         F32 dt = (as > 0.00001f) ? mDelta.warpOffset.len() / as: sMaxWarpTicks;
         mDelta.warpTicks = (S32)((dt > sMinWarpTicks) ? getMax(mFloor(dt + 0.5), 1.0f) : 0.0f);

         if (mDelta.warpTicks )
         {
            // Setup the warp to start on the next tick.
            if (mDelta.warpTicks > sMaxWarpTicks)
               mDelta.warpTicks = sMaxWarpTicks;
            mDelta.warpOffset /= mDelta.warpTicks;

            mDelta.rotOffset = rot - mDelta.rot;
            if(mDelta.rotOffset.z < - M_PI)
               mDelta.rotOffset.z += M_2PI;
            else if(mDelta.rotOffset.z > M_PI)
               mDelta.rotOffset.z -= M_2PI;
            mDelta.rotOffset /= mDelta.warpTicks;
         }
         else
         {
            // Going to skip the warp, server and client are real close.
            // Adjust the frame interpolation to move smoothly to the
            // new position within the current tick.
            Point3F cp = mDelta.pos + mDelta.posVec * mDelta.dt;
            if (mDelta.dt == 0)
            {
               mDelta.posVec.set(0,0,0);
               mDelta.rotVec.set(0,0,0);
            }
            else
            {
               F32 dti = 1 / mDelta.dt;
               mDelta.posVec = (cp - pos) * dti;
               mDelta.rotVec.z = mRot.z - rot.z;

               if(mDelta.rotVec.z > M_PI)
                  mDelta.rotVec.z -= M_2PI;
               else if(mDelta.rotVec.z < -M_PI)
                  mDelta.rotVec.z += M_2PI;

               mDelta.rotVec.z *= dti;
            }
            mDelta.pos = pos;
            mDelta.rot = rot;
            setPosition(pos,rot);
         }
      }
      else
      {
         // Set the player to the server position
         mDelta.pos = pos;
         mDelta.rot = rot;
         mDelta.posVec.set(0,0,0);
         mDelta.rotVec.set(0,0,0);
         mDelta.warpTicks = 0;
         mDelta.dt = 0;
         setPosition(pos,rot);
      }
      //FIXME ?!?! gClientVisManager->setDirty();
   }

   // Ghost needs energy to predict reliably (but not in RTS -- BJG)
   //F32 energy = stream->readFloat(EnergyLevelBits) * mDataBlock->maxEnergy;
   //setEnergyLevel(energy);

   // Transmit AI state.
   if (stream->readFlag())
   {
/* sucks ?! 
      mMoveState = (RTSUnit::MoveState)stream->readInt(2);

      if(true ) // mMoveState != ModeStop ) //XXTH allways else it get out of sync!
      {
         stream->readCompressedPoint(&mMoveDestination, 0.1f);
         //XXTH unused in RTS !!! mMoveTolerance = stream->readFloat(5) * 4.f;
      }

      if(mAimObjectSet = stream->readFlag())
      {
         S32 aimObjId = stream->readInt(NetConnection::GhostIdBitSize);
         mAimObject = (GameBase*)con->resolveGhost(aimObjId);
      }

      if(mAimLocationSet = stream->readFlag())
      {
         stream->readCompressedPoint(&mAimLocation, 0.1f);
      }

      //XXTH unused ! mMoveVelocity = stream->readFloat(8) * 32.f;

	  // mPathRunning = stream->readFlag();
      */
   }

/* FIXME ?! 
   if (stream->readFlag())
   {
      // Clear the modifier list...
      U32 size = mModifierList.size();
      for (U32 k = 0; k < size; k++)
         removeModifier((RTSUnitModifierData*)mModifierList[size-1-k]);

      AssertFatal(mModifierList.empty(), "Modifier list not empty!!");

      size = stream->readInt(5);
      for (U32 k = 0; k < size; k++)
      {
         U32 id;
         stream->read(&id);
         if (RTSUnitModifierData* mod = static_cast<RTSUnitModifierData*>(Sim::findObject(id)))
            addModifier(mod);
         else
            AssertFatal(false, "BAD MODIFIER ON CLIENT!!!");
      }
   }
*/
   // Team
   if(stream->readFlag())
   {
//FIXME?!      setTeam(stream->readInt(5));
   }

   // Projectile datablock
/* FIXME ?!
   if(stream->readFlag())
   {
      ProjectileData* dptr = NULL;
      SimObjectId id = stream->readRangedU32(DataBlockObjectIdFirst, DataBlockObjectIdLast);

      if (!Sim::findObject(id,dptr))
         con->setLastError("Failed to load projectile datablock.");

      mCurrentProjectile = dptr;
   }
*/

   // Fade status...
   if(stream->readFlag())
   {
      setCloakedState(stream->readFlag());

      if (( mFading = stream->readFlag()) == true) {
         mFadeOut = stream->readFlag();
         if(mFadeOut)
            mFadeVal = 1.0f;
         else
            mFadeVal = 0;
         stream->read(&mFadeTime);
         mFadeDelay = 0;
         mFadeElapsedTime = 0;
      }
      else
         mFadeVal = F32(stream->readFlag());


   }
}
//----------------------------------------------------------------------------------------------------------
void ToMEntity::updateAnimation(F32 dt)
{
	
   if ((isGhost() || mActionAnimation.animateOnServer) && mActionAnimation.thread)
      mShapeInstance->advanceTime(dt,mActionAnimation.thread);
/*
   if (mRecoilThread)
      mShapeInstance->advanceTime(dt,mRecoilThread);

   // AFX CODE BLOCK (anim-clip) <<
   // update any active blend clips
   if (isGhost())
      for (S32 i = 0; i < blend_clips.size(); i++)
        mShapeInstance->advanceTime(dt, blend_clips[i].thread);
   // AFX CODE BLOCK (anim-clip) >>
*/

/*
   // If we are the client's player on this machine, then we need
   // to make sure the transforms are up to date as they are used
   // to setup the camera.
   if (isGhost())
   {
      if (getControllingClient())
      {
         updateAnimationTree(isFirstPerson());
         mShapeInstance->animate();
      }
      else
      {
         updateAnimationTree(false);
         // AFX CODE BLOCK (anim-clip) <<
         // This addition forces recently visible players to animate their
         // skeleton now rather than in pre-render so that constrained effects
         // get up-to-date node transforms.
         if (didRenderLastRender())
           mShapeInstance->animate();
         // AFX CODE BLOCK (anim-clip) >>
      }
   }
*/

}
//----------------------------------------------------------------------------------------------------------
void ToMEntity::updateActionThread()
{

	if (mDataBlock->mIsBuilding)
		return;

   // Select an action animation sequence, this assumes that
   // this function is called once per tick.

   if(mActionAnimation.action != PlayerData::NullAnimation)
      if (mActionAnimation.forward)
         mActionAnimation.atEnd = mShapeInstance->getPos(mActionAnimation.thread) == 1;
      else
         mActionAnimation.atEnd = mShapeInstance->getPos(mActionAnimation.thread) == 0;

/*
   if (mActionAnimation.action == PlayerData::NullAnimation ||
      ((!mActionAnimation.waitForEnd || mActionAnimation.atEnd)) &&
      !mActionAnimation.holdAtEnd && (mActionAnimation.delayTicks -= !mMountPending) <= 0)
   {
      //The scripting language will get a call back when a script animation has finished...
      //  example: When the chat menu animations are done playing...
      if ( isServerObject() && mActionAnimation.action >= PlayerData::NumTableActionAnims )
         Con::executef(mDataBlock,"animationDone",scriptThis());
      pickActionAnimation();
   }
*/
   if (!isServerObject())
		pickActionAnimation();

   if ( (mActionAnimation.action != PlayerData::LandAnim) &&
      (mActionAnimation.action != PlayerData::NullAnimation) )
   {
      // Update action animation time scale to match ground velocity
      PlayerData::ActionAnimation &anim = mDataBlock->actionList[mActionAnimation.action];
      F32 scale = 1;
      if (anim.velocityScale && anim.speed)
      {
         VectorF vel;
         mWorldToObj.mulV(mVelocity,&vel);
         scale = mFabs(mDot(vel, anim.dir) / anim.speed);

         if (scale > mDataBlock->maxTimeScale)
            scale = mDataBlock->maxTimeScale;
      }

      mShapeInstance->setTimeScale(mActionAnimation.thread,
         mActionAnimation.forward? scale: -scale);
   }
   
}
//----------------------------------------------------------------------------------------------------------
void ToMEntity::doCheckTarget(Player* target,Point3F location) {
	// Check that our aim object is or is derived from RTSUnit
	if(target)
	{
		Point3F posXY(location.x, location.y, 0);
		Point3F goalXY(target->getPosition().x,target->getPosition().y, 0);
		Point3F goalDelta = goalXY - posXY;

/*
if (isGhost())
	Con::printf("CLIENT:len to target: %f", goalDelta.len() );
else
	Con::printf("SERVER:len to target: %f", goalDelta.len());
*/
        F32 lcheckRange = mDataBlock->mRange;
		if (isGhost()) //crazy ghost/server difference!
			lcheckRange += 0.8f; 

		// Check if we're within range to fire
		if(goalDelta.len() < lcheckRange)
		{
			if(getMoveState() == ModeMove)
				stopMove();
                  
			if(mAttackDelayCounter >= mDataBlock->mAttackDelay)
			{
				mAttackDelayCounter = 0;
				ShapeBase* target = dynamic_cast<ShapeBase*>(&(*getAimObject()));
				if (!target || target->getDamageState() == ShapeBase::Enabled)
				{
					if(isServerObject())
					{
						// Do a proper method callback...
                        Con::executef(mDataBlock,  "onAttack", getIdString(), getAimObject()->getIdString());
						//gToMHandler->applyDamage(this,static_cast<RTSUnit*>(&(*mAimObject)));
					} else {
                        // Call a goofy global function to do the client side animation.
						setActionThread(action_attack, true, true);
                    }
                 }
			} else {
					mAttackDelayCounter++;
			}
		} else {
			setMoveState(ModeMove);
		}

	}
}
//------------------------------------------------------------------------------------------------------
void ToMEntity::doLook(VectorF lTargetLocation,Point3F &location, Point3F &rotation)
{
	//XXTH we only need to look at server bleh!
	if (isGhost()) return;
	//XXTH 2012-11-07 do not rotate a tower ;)
	if (getIsTower())
		return;

/* FIXME really!!! 

         // Orient towards the aim point, aim object, or towards
         // our destination.
         if (mAimObjectSet || mAimLocationSet || mMoveState == ModeMove)
         {
            // Update the aim position if we're aiming for an object
            if(mAimObjectSet)
            {
				if(mAimObject) {
                    mAimLocation = mAimObject->getPosition();

				}  else {
                  mAimObjectSet = false;
				}
			} else {
               mAimLocation = mMoveDestination;
			}

            F32 xDiff = mAimLocation.x - location.x;
            F32 yDiff = mAimLocation.y - location.y;
            if (!isZero(xDiff) || !isZero(yDiff))
            {
               // First do Yaw
               // use the cur yaw between -Pi and Pi
               F32 curYaw = rotation.z;
               while (curYaw > M_2PI)
                  curYaw -= M_2PI;
               while (curYaw < -M_2PI)
                  curYaw += M_2PI;

               // find the yaw offset
               F32 newYaw = mAtan(xDiff, yDiff);
               F32 yawDiff = newYaw - curYaw;

               // make it between 0 and 2PI
               if(yawDiff < 0.0f)
                  yawDiff +=M_2PI;
               else if(yawDiff >= M_2PI)
                  yawDiff -= M_2PI;

               // now make sure we take the short way around the circle
               if(yawDiff > M_PI)
                  yawDiff -= M_2PI;
               else if(yawDiff <-M_PI)
                  yawDiff += M_2PI;

               mRot.z += yawDiff;

			   // make sure that the clients rotation gets updated if necessary?
               if (!isZero(yawDiff))
                 setMaskBits(MoveMask);
            }
         }
*/
}

//------------------------------------------------------------------------------------------------------
void ToMEntity::dropToTerrain(Point3F &location) 
{

	if (mDataBlock->mIsBuilding) 
		return;

   // Move us to sit on the ground...
   TerrainBlock* block = NULL;
   block = AIMath::getTerrainBlockATpos(location, true);
   
    
   if(block)
   {
      Point3F terrPos = location;
      block->getWorldTransform().mulP(terrPos);
      terrPos.convolveInverse(block->getScale());

      F32 height;
      bool res = block->getHeight(Point2F(terrPos.x, terrPos.y), &height);
      if(res)
      {
         terrPos.z = height;
         terrPos.convolve(block->getScale());
         block->getTransform().mulP(terrPos);
      }

	  height += 0.3f; //bit higher
      if (mFabs(location.z - height) > 0.01)
      {
         location.z = height;
      }
   }

}


// --------------------------------------------------------------------------------------------
bool ToMEntity::ObjectInLos(ShapeBase* lObj, U32 mask) 
{

	if (!lObj) 
		return false;
  Point3F lF,lT; 
  this->getWorldBox().getCenter(&lF);
  lObj->getWorldBox().getCenter(&lT);
  RayInfo dummy;

  disableCollision();
  bool result = !getContainer()->castRay( lF, lT, mask, &dummy);
  
  enableCollision();

  return result || dummy.object == lObj;
  

}

//---------------------------------------------------------------------------------------
void ToMEntity::processTower()
{
/* FIXME ?!? 
   if(mAttackDelayCounter++ < mDataBlock->mAttackDelay)
   {
		return;
   }

   mAttackDelayCounter = 0;
   scanNeighbours();
   
   if (mNeighbours.mList.size() == 0)
	   return;

   RTSUnit* nearestTarget=NULL;
   RTSUnit* tmpTarget;

// XXTH 2012-11-07 stay on old target while in range and a valid target
   if (mAimObjectSet && mAimObject )
   {
	   RTSUnit* lAimObject = static_cast<RTSUnit*>(&(*mAimObject));
	   if ( lAimObject->getDamageState() == ShapeBase::Enabled && ObjectInLos(lAimObject))
	   {
			for (S32 a = 0; a<mNeighbours.mList.size(); ++a) {
					tmpTarget = static_cast<RTSUnit*>(&(*mNeighbours.mList[a]));
					if (tmpTarget->getId() == mAimObject->getId()){
						gToMHandler->onTowerTarget(this,lAimObject);
						return;
					}
			}
	   }
   }

   for (S32 a = 0; a<mNeighbours.mList.size(); ++a) {
		tmpTarget = static_cast<RTSUnit*>(&(*mNeighbours.mList[a]));
		if (tmpTarget->getDamageState() == ShapeBase::Enabled && tmpTarget->getTeam() != this->getTeam() && ObjectInLos(tmpTarget))
		{
			if (!nearestTarget) {
				nearestTarget=tmpTarget;
		    } else if ( getDistance(tmpTarget) <  getDistance(nearestTarget)) {
                nearestTarget=tmpTarget;
		    }
		}
   }

   if (nearestTarget) {
	   
	   gToMHandler->onTowerTarget(this,nearestTarget);
	   //Con::executef(mDataBlock,"onTowerTarget",scriptThis(),nearestTarget->getIdString()); 
   }


*/
}
//---------------------------------------------------------------------------------------
/* used from AIEntity!
void ToMEntity::insertionNeighbourCallback(SceneObject* obj, void *key)
{
   SimpleQueryList* pList = (SimpleQueryList*)key;
   ToMEntity* lEntity = static_cast<ToMEntity*>(obj);
   if (lEntity && !lEntity->mDataBlock->mIsBuilding)
		pList->insertObject(obj);
}

void ToMEntity::scanNeighbours()
{
	//if (!mScanNeighbours) return;
	SimObjectId tmpID;
	ShapeBase* tmpObject;
	mNeighbours.mList.clear();

   F32 mScanRadius = mDataBlock->mVision; //FIXME ?!?  *mNetModifier.mVision;

    Box3F queryBox(this->getPosition(), this->getPosition());
    queryBox.min -= Point3F(mScanRadius, mScanRadius, mScanRadius);
    queryBox.max += Point3F(mScanRadius, mScanRadius, mScanRadius);
	disableCollision(); //not scan for the object itself!
    gServerContainer.findObjects(queryBox, PlayerObjectType, ToMEntity::insertionNeighbourCallback, &mNeighbours);
	enableCollision();

}
*/

//---------------------------------------------------------------------------------------
F32 ToMEntity::getDistance(GameBase* a)
{
	return AIMath::Distance2D(getPosition(), a->getPosition());
}

//-----------------------------------------------------------------------------
/* FIXME 
void ToMEntity::renderObject(SceneRenderState* state, SceneRenderImage* image)
{

	Parent::renderObject(state, image);

	if (!mDataBlock->mIsBuilding)
			return;
   
	bool lDoVision = false; 
	
	if (gGuiRTSCtrl && !gGuiRTSCtrl->getPlacingBuilding() && gGuiRTSCtrl->getBuildingVisionFlag() > 0)
	{
		if (gGuiRTSCtrl->isObjectInSelection(this) ) {
				lDoVision = true;
		} else if (this == gGuiRTSCtrl->getHitObject() || gGuiRTSCtrl->getDragSelection().objInSet(this)) {
			if (gGuiRTSCtrl->getBuildingVisionFlag() > 1)
				lDoVision = true;

		} 
	}

	if (lDoVision) 
	{
		
		RenderBuildingVision(state, getTransform(), getCombinedVision(), ColorF (0.2f,0.2f,1.f), getWorldBox().len_z() ); // / 2.f);
	}


}
*/
//-----------------------------------------------------------------------------
// Ondisabled here!
//-----------------------------------------------------------------------------

void ToMEntity::updateDamageState()
{
   // Become a corpse when we're disabled (dead).
   if (mDamageState == Enabled) {
      mTypeMask &= ~CorpseObjectType;
      mTypeMask |= PlayerObjectType;
   }
   else {
      mTypeMask &= ~PlayerObjectType;
      mTypeMask |= CorpseObjectType;
	  if (isServerObject()) {
		//mmhhh gToMHandler->onUnitDisabled(this,mDataBlock);
	  }
   }

   Parent::updateDamageState();
}


//-----------------------------------------------------------------------------
// Simple Path following:
//-----------------------------------------------------------------------------
bool ToMEntity::MoveToNextWayPoint()
{
	if (!getIsPathWalking() || !mPathObject)
		return false;

	mCurPathNodeIndex++;
   
	if (mCurPathNodeIndex >= mPathObject->size()) //reached last!
	{
		stopPath();
		return false;
	}

	Marker* lMarker = static_cast<Marker*>((*mPathObject)[mCurPathNodeIndex]);
	if (!lMarker) //huh=?!
		return false;
	setMoveDestination(lMarker->getPosition());
	return true;

}
//-----------------------------------------------------------------------------
void ToMEntity::setPathObject(SimPath::Path* lPath)
{
	if (lPath && lPath->size() > 1) 
		mPathObject = lPath;

}
//-----------------------------------------------------------------------------
void ToMEntity::startPath()
{
	if (mPathObject && mPathObject->size() > 1) {
		mIsPathWalking = true;
		mCurPathNodeIndex = -1; //will be set 0 in MoveToNext!
		MoveToNextWayPoint();   
	}
}
//-----------------------------------------------------------------------------
void ToMEntity::stopPath()
{
  mIsPathWalking = false;
  stopMove();
}

