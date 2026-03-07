//-----------------------------------------------------------------------------
// Copyright (c) 2009 huehn-software / Ohmtal Game Studio
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// aiEntity by t.huehn (XXTH) (c) huehn-software 2009..2024...
/*
   2023-12-30 XXTH   started changelog
   2023-12-30 XXTH   added physics
   2024-01-08 XXTH   replaced exec with callbacks 


*/

/*

 FIXME transmit stats like attack/defence/ and all this stuff


*/


// AI_USEPATH ==> to navigation addon :D search for TORQUE_NAVIGATION_ENABLED
// ===> mhh added iAiPath
// #define TGE_SPECIALPLAYERCOL need to modify playerclass OR copy all collision stuff this aiEntity :/
// ==> for now i dont care the collision

/*
Enginemodification:
   PlayerData need new variable conformToGround
     * define, default false, initPersistFields, packData, unpackData

*/

//-----------------------------------------------------------------------------
#include "aiEntity.h"
#include "aiSteering.h"
#include "aiMath.h"
#include "fsm/StateMachine.h"
#include "aiStates.h"

#include "console/consoleInternal.h"
#include "console/engineAPI.h"

#include "core/stream/bitStream.h"
#include "T3D/fx/cameraFXMgr.h"
#include "T3D/fx/particleEmitter.h"
#include "T3D/trigger.h"
#include "T3D/decal/decalManager.h"
#include "materials/materialManager.h"
#include "core/util/safeDelete.h"
#include "ts/tsShapeInstance.h"
#include "sfx/sfxSystem.h"
#include "T3D/gameBase/gameConnection.h"
#include "materials/baseMatInstance.h"

#include "math/mMatrix.h"
#include "math/mathUtils.h"

//2023-12-30 added physic
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsPlayer.h"



#ifndef	M_RAD
#define M_RAD        0.017453292519943295769236907684886
#endif

// Downward velocity at which we consider the player falling
static const F32 sFallingThreshold = -12.5f; //XXTH pre 1.99 it was: -10.0f;

// Client prediction
static F32 sMinWarpTicks = 0.5f;        // Fraction of tick at which instant warp occures
static S32 sMaxWarpTicks = 3;          // Max warp duration in ticks

static S32 sMaxPredictionTicks = 30;   // Number of ticks to predict
static F32 sAnimationTransitionTime = 0.25f;

// Choses new action animations every n ticks.
static const S32 sNewAnimationTickTime = 4;
static const F32 sMountPendingTickWait = (13.0f * 32.0f);
static const U8 JumpTriggerID = 2;
static const F32 sSwimWaterCoverage = 0.3f; //0.3f (2022-01-22) dont fix the "dolphin" must be changed in move!!
static const F32 sDolphinWaterCoverage = 0.8f; //2022-01-22 added




//-------------------------------------------


enum PlayerConstants {
   JumpSkipContactsMax = 8
};


	const char *  AIStateNames[9] = {
			"none",          //0
			"CheckVitality",  //1
			"Pause",         //2 
			"Hunt",          //3
			"Attack",        //4
			"Wander",        //5
			"Flee",          //6
			"Follow",        //7
			"Run Home"	     //8 - RunWander same as wander but enable run before
	};
//-------------------------------------------


#define BAD_ANIM_ID  999999999

#ifndef TGE_SPECIALPLAYERCOL

 

#endif


IMPLEMENT_CO_NETOBJECT_V1(AIEntity);

// --------------------------------------------------------------------------------------------
/**
 * Constructor
 */
AIEntity::AIEntity()
{
   //FIXME T3D ??!
   //mTypeMask |= AIObjectType;

   mCollisionMoveMask = (
      TerrainObjectType |
      TerrainLikeObjectType |
      InteriorLikeObjectType |
      WaterObjectType |
      // PlayerObjectType     | 
      StaticShapeObjectType |
      VehicleObjectType |
      PhysicalZoneObjectType);



   mServerCollisionContactMask = (mCollisionMoveMask |
      (ItemObjectType |
         TriggerObjectType |
         CorpseObjectType
         ));

   mClientCollisionContactMask = mCollisionMoveMask | PhysicalZoneObjectType;


   mPauseSaveLoad = false; //now default off!!! jitter running shit

   mPlayerRank = 0;
   mPlayerType = 0;
   mFaction = 0;
   mEntity = 0;
   mMana = 0.f;
   mMaxMana = 0.f;
   mMaxDamage = 100.f;
   mMaxEnergy = 0.f;
   mWeaponHands = 0; //0=barhand, 1=rhandWeapon, 2= lhandWeapon, 3 = left AND right hand,  4 = twohandWeapon
   mMoveBlocked = false;
   mShielded    = false;
   mShieldBlock = false;
   mSitting		= false;
   mPainRunning = false;
   mDisablePainAction = false;
   mAttackStopMove = false;
   mAttackIgnoreMove = false;
   mFlying		= false;
   mSwimming	= false;
   mBasicAnimation    = true; //was mIsAnimal 
   mCready      = false;
   mEmotion		= false;
   mHumanControlled = false;
   mIsAiControlled  = true; //2023-02-28
   mDoPauseSleepAnimaion = false;
   mBattlePoints = 0;

   mJumping		= false;
   mJumpCnt		= 0;
   mJumpDelay   = 0;

   mOverwriteAnim  = false; 

   mWalkMoveModifier   = 15; // set the current walk speed (default 15)
   mRunMoveModifier    = 25; //  set the current run speed  (default 25)


   mMoveDirection = MoveForward;

   mAimCounterStrike = false;		     // Force a AimObject no matter if its out of radius!
   mLastAimTime = 0;

   mLastLocation.set(9999.9f,9999.9f,9999.9f);
   mStuckCounter = 0;

   mWanderPositonSet = Point3F(0.f,0.f,0.f);


   mWallSkipCounter = 4711; //set high value by default!! 




#ifdef TGE_SPECIALPLAYERCOL
   // for testing collision is disabled for most things
 sCollisionMoveMask = (      TerrainObjectType      
                                 
#ifndef _entity_special_collision //lol walk over water !
                                 | WaterObjectType  
#else
								 | PathShapeObjectType
#endif
								 | StaticObjectType
								 | StaticShapeObjectType
								 | DynamicShapeObjectType
								 | PhysicalZoneObjectType //XXTH 2019-02-08 added ... want to play outball ?! 
								 | VehicleObjectType //XXTH 2019-02-08 added ... want to play outball ?!   <<< THIS ONE
                         | TerrainLikeObjectType | InteriorLikeObjectType 
								 
						);
 sServerCollisionContactMask = (sCollisionMoveMask |
                                          (ItemObjectType    | 
                                           TriggerObjectType |
                                           CorpseObjectType
										  ));

 sClientCollisionContactMask = sCollisionMoveMask | PhysicalZoneObjectType;
#endif

   //setup default 
   mBehaviourTypeMask = SeparationBehaviourType | Wall_AvoidanceBehaviourType;
   mTargetPosition.set( 0.0f, 0.0f, 0.0f );
   mMoveState = ModeStatic;

   mAimObject = nullptr;
   mAimObject2 = nullptr;
   mLeaderObject = nullptr;

   mAimInLOS = false;

   mMaxRotateRadiants = 75.f * (F32)M_RAD; // M_RAD = M_PI / 180; first value° in rad && KEEP IT LOW TO DENY JITTER

   mMoveTolerance = 0.5f; 
   mAttackDistanceAdd = 1.0f; //2.1 was 0.75f
   mPanicDistance = 25.f; //shoud be higher than ScanRadius!
   mTargetDistance = 3.f; //distance to target for pursuit and attack
   mConstantAttackDistance = 0.f;


   mSteering = new AISteering(this);
   mSteering->setMoveTolerance(mMoveTolerance);
  

   mFormationOffset = VectorF(0.f,0.f,0.f);

   mScanNeighbours = false;
   mScanRadius   = 20.f;
   mWanderRadius = 10.f;
   mIsWanderer	 = false;
   mSpawnPoint   = Point3F(0.f,0.f,1000.f); //z changed from 0.f to 1000.f if not set you should go on air
   mTargetPlayerType = -1; //2023-02-28 i dont want to set it by default! old:  0;
   mIgnoreRange = 50.f;
   mIgnoreBP = 0;
   mAllowWater  = true;
   mAllowLand   = true;
   mSpawnInsideMissionArea = true;

   mSeparationRadius = 2.f; //1.97.3 set to 1.5! was 2.f !! reset to 2 as default but make it changeable
   mAligmentRadius   = 2.f;
   mCohesionRadius   = 3.f;

   m_dWeightSeparation			=  1.0f; //2023-02-01 was 1
   m_dWeightCohesion			   =  2.f;
   m_dWeightAlignment			=  1.f;
   m_dWeightWander				=  1.f;
   m_dWeightWallAvoidance		=  2.1f; //2023-02-28 SET to 2.1f again old: 4.f was 10.f; prior 1.97.3 => 2.1 set to 8.f!
   m_dWeightSeek				   =  1.f;  //2023-02-01 was 1
   m_dWeightFlee				   =  1.f;
   m_dWeightArrive				=  1.f;
   m_dWeightPursuit				=  1.f;
   m_dWeightOffsetPursuit		=  1.f;
   m_dWeightInterpose			=  1.f;
   m_dWeightHide				   =  1.f;
   m_dWeightEvade				   =  1.f;  //slower
   m_dWeightFollowPath			=  1.f; 


#ifdef _entity_special_collision
   mRunSurface = false;
   mOnElevator = false;
#endif

   mPersonality = PersoNone; // 1.99 pre was: Dontcare;

   //set up state machine
   mStateMachine = new StateMachine<AIEntity>(this);

   mThinkTicks  = 32; //~1 sec
   mAttackTicks =  8; //= 6* 256 = 1536ms  => steps are 256, !! 1.97.3 new: 7 *256 = 1792 => 1.97.4 :: 8*256=2048
   mTickCounter = mThinkTicks; 
   mAttackTickCounter = mAttackTicks;




   mPoisoned = false;
   mAttack   = 0.f;
   mDefence  = 0.f;
   mMagic    = 0.f;
 
// auteria 2.0.f (sutaratus) >>
	  
	  mStrength		= 0.f;	//Staerke
	  mAgility		= 0.f;		//Beweglichkeit
	  mStamina		= 0.f;		//Ausdauer
	  mIntellect	= 0.f;	//Intelligenz
	  mSpirit		= 0.f;		//Willenskraft

	  //Secundary
	  mAttackPower	= 0.f; //Angriffskraft
	  mHaste		= 0.f;		//Tempo
	  mCrit			= 0.f;		//Kritisch
	  mHit			= 0.f;			//Trefferwertung
	  mExpertise	= 0.f;	//Waffenkunde

	  mArmour		= 0.f;		//Rüstung      - reduce damage calc to %
	  mDodge		= 0.f;		//Ausweichen   - chance to doge calc to  %
	  mParry		= 0.f;		//Parieren     - chance to parry calc to  %
	  mBlock		= 0.f;		//Blocken	   - chance to block  calc to  %
	  mResilience	= 0.f;  //Abhärtung    - reduce damage calc to % from players or their minions

	  mSpellPower	= 0.f;  //Zaubermacht       - increase damage or heal of spells 
	  mPenetration	= 0.f; //Zauberdurchschlag - reduce enemy resistance value (not percent)


	  //Resistance - reduce damge 
	  //S32 mResArcane
	  mResFire		= 0.f;
	  mResFrost		= 0.f;
	  mResNature	= 0.f;
	  mResShadow	= 0.f;


//<< auteria 2.0 (sutaratus) 

   mPathUsage = PathNone;

   mFollowLeader = false;
   mFollowArriveNotify = false;

   mPauseHeal = false; //1.97.3 default to false!
   mPauseHealPercent = 0.03f;
   mStuckBeam = false; //should be set on pets
  //SPELLS:
   mCurSpell = NULL;
   mElapsedTicks = 0;
   mCurUpdateTickCount = 32; //init! 

   for (S32 i = 0; i<AUTERIA_AI_SPELLCOUNT; i++)
	   mINVamt[i] = 0;

   mGravity = -20.f; //4.0


   mInLiquid = false;
   mPlayerCantMove = false;
   mSpeedModifier = 15;
   mlastAnimationAction = -1; //XXTH 2023-02-28 was 0!
   anim_clip_flags = 0;
   mCanSwim = false;
   mCanFly = false;

}

ImplementEnumType(PathUsageType, "@Pathusage of AIEntity\n\n" )
  { AIEntity::PathNone, "none"}
, { AIEntity::PathWander,  "wander" }  //the only difference between wander and hunt is the setPath (simpath) behaviour?
, { AIEntity::PathHunt,    "hunt" }
EndImplementEnumType;



//   static EnumTable gPathUsageTypeTable(sizeof(PathUsageTypeEnums) / sizeof(PathUsageTypeEnums[0]), &PathUsageTypeEnums[0]);

//-----------------------------------------------------------------------------
void AIEntity::initPersistFields()
{   
	Parent::initPersistFields();
    addGroup("AI");


       addField("WeaponHands",       TypeS32,      Offset(mWeaponHands, AIEntity)        );
       addField("Shielded",			 TypeBool,     Offset(mShielded, AIEntity)        );
       addField("Sitting",			 TypeBool,     Offset(mSitting, AIEntity)        );
       addField("BasicAnimation",	 TypeBool,     Offset(mBasicAnimation, AIEntity)        ); //was mIsAnimal
       addField("isHumanControlled", TypeBool,     Offset(mHumanControlled, AIEntity)        );
	   addField("Cready",			 TypeBool,     Offset(mCready, AIEntity)        );
	   

       addField("PauseHeal",	     TypeBool,     Offset(mPauseHeal, AIEntity)        );
       addField("PauseHealPercent",	  TypeF32,     Offset(mPauseHealPercent, AIEntity)        );
	   
       addField("StuckBeam",	     TypeBool,     Offset(mStuckBeam, AIEntity)        );
   

	   addField("WalkMoveModifier",  TypeS32,      Offset(mWalkMoveModifier, AIEntity)        );
	   addField("RunMoveModifier",   TypeS32,      Offset(mRunMoveModifier, AIEntity)        );

	   addField("TargetPlayerType",     TypeS32,      Offset(mTargetPlayerType, AIEntity)        );
       addField("TargetScanRadius",     TypeF32,      Offset(mScanRadius, AIEntity)        );
       addField("WanderRadius",			TypeF32,      Offset(mWanderRadius, AIEntity)        );
       addField("Wanderer",				TypeBool,      Offset(mIsWanderer, AIEntity)        );
       addField("PanicDistance",		TypeF32,      Offset(mPanicDistance, AIEntity)        );

       addField("DisablePainAction",       TypeBool,     Offset(mDisablePainAction, AIEntity)        );

       
       addField("AttackStopMove", TypeBool, Offset(mAttackStopMove, AIEntity));
       addField("AttackIgnoreMove", TypeBool, Offset(mAttackIgnoreMove, AIEntity));

	   addField("IgnoreRange",            TypeF32,      Offset(mIgnoreRange, AIEntity)        );
	   addField("IgnoreBP",               TypeS32,      Offset(mIgnoreBP, AIEntity)        );

	   addField("AllowWater",             TypeBool,     Offset(mAllowWater, AIEntity)        );
	   addField("AllowLand",              TypeBool,     Offset(mAllowLand, AIEntity)        );

//      addField("lightType", TYPEID< Item::LightType >(), Offset(lightType, ItemData), "Type of light to apply to this ItemData. Options are NoLight, ConstantLight, PulsingLight. Default is NoLight.");

	   addField("PathUsage",  TYPEID< AIEntity::PathUsageType>(),     Offset(mPathUsage, AIEntity), "");


	   
	   
    endGroup("AI");

	addGroup("Sutaratus");
	   addField("strength",				  TypeF32,     Offset(mStrength, AIEntity)        );
	   addField("agility",				  TypeF32,     Offset(mAgility, AIEntity)        );
	   addField("stamina",				  TypeF32,     Offset(mStamina, AIEntity)        );
	   addField("intellect",			  TypeF32,     Offset(mIntellect, AIEntity)        );
	   addField("spirit",				  TypeF32,     Offset(mSpirit, AIEntity)        );

	  //Secundary
	   addField("attackPower",			TypeF32,     Offset(mAttackPower, AIEntity)        );
	   addField("haste",				TypeF32,     Offset(mHaste, AIEntity)        );
	   addField("crit",				  TypeF32,     Offset(mCrit, AIEntity)        );
	   addField("hit",				  TypeF32,     Offset(mHit, AIEntity)        );
	   addField("expertise",		TypeF32,     Offset(mExpertise, AIEntity)        );

	   addField("armour",				  TypeF32,     Offset(mArmour, AIEntity)        );
	   addField("dodge",				  TypeF32,     Offset(mDodge, AIEntity)        ); //AUSWEICHEN
	   addField("parry",				  TypeF32,     Offset(mParry, AIEntity)        ); //PARIEREN 
	   addField("block",				  TypeF32,     Offset(mBlock, AIEntity)        ); // BLOCK ===> defence ?
	   addField("resilience",				  TypeF32,     Offset(mResilience, AIEntity)        );
	   addField("spellPower",				  TypeF32,     Offset(mSpellPower, AIEntity)        );
	   addField("penetration",				  TypeF32,     Offset(mPenetration, AIEntity)        );


	  //Resistance - reduce damge 
	  //S32 mResArcane
	   addField("ResFire",				  TypeF32,     Offset(mResFire, AIEntity)        );
	   addField("ResFrost",				  TypeF32,     Offset(mResFrost, AIEntity)        );
	   addField("mResNature",			  TypeF32,     Offset(mResNature, AIEntity)        );
	   addField("ResShadow",			TypeF32,     Offset(mResShadow, AIEntity)        );
	endGroup("Sutaratus");



	addGroup("Auteria");
	  
	   addField("poisoned",				  TypeBool,    Offset(mPoisoned, AIEntity)        );
	   addField("attack",				  TypeF32,     Offset(mAttack, AIEntity)        );
	   addField("defence",				  TypeF32,     Offset(mDefence, AIEntity)        );
	   addField("magic",				  TypeF32,     Offset(mMagic, AIEntity)        );

	   addField("INVIceBall",				  TypeS32,     Offset(mINVamt[0], AIEntity)        );
	   addField("INVPoison",				  TypeS32,     Offset(mINVamt[1], AIEntity)        );
	   addField("INVFireBall",				  TypeS32,     Offset(mINVamt[2], AIEntity)        );
	   addField("INVFireStone",				  TypeS32,     Offset(mINVamt[3], AIEntity)        );
	   addField("INVHealth",				  TypeS32,     Offset(mINVamt[4], AIEntity)        );
	   addField("INVBeer",					  TypeS32,     Offset(mINVamt[5], AIEntity)        );
	   addField("INVAntidote",				  TypeS32,     Offset(mINVamt[6], AIEntity)        );
	   addField("INVCustomRange",			  TypeS32,     Offset(mINVamt[7], AIEntity)        );
	   addField("INVCustomArea",			  TypeS32,     Offset(mINVamt[8], AIEntity)        );

	endGroup("Auteria");
   
   


}
//-----------------------------------------------------------------------------
void AIEntity::setPersonality(Personality lPersonality)
{
	if (mPersonality != lPersonality) {
		mPersonality = lPersonality;
		switch (mPersonality)
		{
		case PersoNone: //3
			if (mStateMachine->getCurrentState() != NULL)  
				mStateMachine->ChangeState(NULL);
			break;
		case Dontcare: //2
			
			if (mIsWanderer) 
				mStateMachine->ChangeState(AIState::Wander::Instance());
			else if (mStateMachine->getCurrentState() != NULL) 
				mStateMachine->ChangeState(NULL);
			break;
		case Aggressive: //0
			//XXTH 1.97.4 mStateMachine->ChangeState(AIState::Pause::Instance());
			if (mIsWanderer) 
				mStateMachine->ChangeState(AIState::Wander::Instance());
            else
				mStateMachine->ChangeState(AIState::Pause::Instance()); //1.99 back
   			break;

		case Sheepish: //1
			//XXTH 1.97.4 mStateMachine->ChangeState(AIState::Pause::Instance());
			if (mIsWanderer) 
				mStateMachine->ChangeState(AIState::Wander::Instance());
			/* XXTH 1.99 NOT BACK
			else 
				mStateMachine->ChangeState(AIState::Pause::Instance()); //1.99 back 
            */ 
			break;
		}
	}
	mStateMachine->SetGlobalState(AIState::CheckVitality::Instance());
	checkScanNeightbours();

}
// --------------------------------------------------------------------------------------------
/**
 * Destructor
 */
AIEntity::~AIEntity()
{
	SAFE_DELETE(mSteering);
	SAFE_DELETE(mStateMachine);
	
}

//---------------------- CalculatePrioritized ----------------------------
//
//  this method calls each active steering behavior in order of priority
//  and acumulates their forces until the max steering lVelo magnitude
//  is reached, at which time the function returns the steering lVelo 
//  accumulated to that  point
//------------------------------------------------------------------------
VectorF AIEntity::CalculatePrioritized()
{       
  VectorF lVelo;
  VectorF l_vSteeringForce(0.f, 0.f, 0.f);


  if (BehaviourOn(Wall_AvoidanceBehaviourType))  {
   // We should check to see if we are stuck...
	if ( mWallSkipCounter < 5)
	{
	  mWallSkipCounter++; //2.10 skip counter!
	  lVelo = mWallSavedForce;
	} else {
	  lVelo = getSteering()->getWallForce(mStuckCounter > 0 ) * m_dWeightWallAvoidance;
      mWallSavedForce = lVelo;
      mWallSkipCounter = 0;
	}
    if (!AccumulateForce(l_vSteeringForce, lVelo)) 
			return l_vSteeringForce;
  }


  if (BehaviourOn(EvadeBehaviourType) && mAimObject)  {
    lVelo = getSteering()->getEvadeForce(mAimObject, mPanicDistance) * m_dWeightEvade;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }
  
  if (BehaviourOn(FleeBehaviourType))  {
    lVelo = getSteering()->getFleeForce(getTargetPos(), mPanicDistance) * m_dWeightFlee;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }

  if (BehaviourOn(SeparationBehaviourType) || BehaviourOn(FlockBehaviourType) ){
	  lVelo = getSteering()->getSeparationForce(mSeparationRadius, &mNeighbours, mTargetPlayerType) * m_dWeightSeparation;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }
  if (BehaviourOn(AlignmentBehaviourType) || BehaviourOn(FlockBehaviourType) ){
    lVelo = getSteering()->getAlignmentForce(mAligmentRadius, &mNeighbours) * m_dWeightAlignment;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }

  if (BehaviourOn(CohesionBehaviourType) || BehaviourOn(FlockBehaviourType) ){
    lVelo = getSteering()->getCohesionForce(mCohesionRadius, &mNeighbours) * m_dWeightCohesion;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }

  if (BehaviourOn(SeekBehaviourType))  {
	  if (mAimObject)
		  lVelo = getSteering()->getSeekForce(getTargetPos(), false) * m_dWeightSeek;
	  else
		  lVelo = getSteering()->getSeekForce(getTargetPos(), true) * m_dWeightSeek;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }

  if (BehaviourOn(ArriveBehaviourType ))  {
    lVelo = getSteering()->getArriveForce(getTargetPos(), normal) * m_dWeightArrive;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }

  if (BehaviourOn(WanderBehaviourType))  {
	// using worldbox because its included scale
	/* 1.99 CHANGE : XXTH 1.99 WTF ?!?! run arround hole map ?  	 
	F32 lWanderRadius = mWanderRadius;  
	F32 lWanderDistance = lWanderRadius * 8.f ;
	F32 lJitter = this->getMaxForwardVelocity() *  mMaxRotateRadiants * 10.f; 
	lVelo = getSteering()->getWanderForce(lWanderRadius,lWanderDistance, lJitter) * m_dWeightWander;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
	*/
	
        F32 lBoxrad = this->getWorldBox().len_y() * 0.5;
        F32 lWanderRadius = lBoxrad * 2;
        F32 lWanderDistance = lWanderRadius * 8.f ;
        F32 lJitter = this->getMaxForwardVelocity() *  mMaxRotateRadiants * 10.f;
        lVelo = getSteering()->getWanderForce(lWanderRadius,lWanderDistance, lJitter) * m_dWeightWander;
		if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;


  }

#ifdef AI_USEPATH
  if (BehaviourOn( PursuitPathBehaviourType ) && mAimObject) {
	  if (mTickCounter % 16 == 0)
			getSteering()->updatePursuitPath(getAimObject());
    lVelo = getSteering()->getPathForce(/* removed: mTargetDistance*/) * m_dWeightFollowPath;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }
#endif

  if (BehaviourOn(PursuitBehaviourType ) && mAimObject)  {
    lVelo = getSteering()->getPursuitForce(mAimObject, mTargetDistance ) * m_dWeightPursuit;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }

  if (BehaviourOn(Offset_PursuitBehaviourType) && mLeaderObject)  {
    lVelo = getSteering()->getOffsetPursuitForce(mLeaderObject,mFormationOffset);
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }
  if (BehaviourOn( InterPoseBehaviourType ) && mAimObject && mAimObject2)  {
    lVelo = getSteering()->getInterPoseForce(mAimObject, mAimObject2) * m_dWeightInterpose;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }


  //XXTH 2021-11-08 OLD and not longer working:  NOW USING AIMOBJECT2 !!!
    /*
    if (BehaviourOn( HideBehaviourType ) && mAimObject && mObstGroup)  {
     lVelo = getSteering()->getHideForce(mAimObject,mObstGroup) * m_dWeightHide;
     if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
    }
    */
  if (BehaviourOn(HideBehaviourType) && mAimObject2 && mObstGroup) {
     lVelo = getSteering()->getHideForce(mAimObject2, mObstGroup) * m_dWeightHide;
     if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }


  if (BehaviourOn( Follow_PathBehaviourType )) {
	  // getPathForce can have parameter lStopDistance to not run into point
	  // usefull, should be added on some circumstands
    lVelo = getSteering()->getPathForce() * m_dWeightFollowPath;
    if (!AccumulateForce(l_vSteeringForce, lVelo)) return l_vSteeringForce;
  }



  return l_vSteeringForce;
}

//--------------------- AccumulateForce ----------------------------------
//
//  This function calculates how much of its max steering lVelo the 
//  vehicle has left to apply and then applies that amount of the
//  lVelo to add. Maxforce defaults to 400.f ? 
//------------------------------------------------------------------------
bool AIEntity::AccumulateForce(VectorF &RunningTot, VectorF ForceToAdd)
{
  
  //calculate how much steering lVelo the vehicle has used so far
  F32 MagnitudeSoFar = RunningTot.len();

  //calculate how much steering lVelo remains to be used by this vehicle
  F32 MagnitudeRemaining = this->getMaxForce() - MagnitudeSoFar;

  //return false if there is no more lVelo left to use
  if (MagnitudeRemaining <= 0.0) return false;

  //calculate the magnitude of the lVelo we want to add
  F32 MagnitudeToAdd = ForceToAdd.len();
  
  //if the magnitude of the sum of ForceToAdd and the running total
  //does not exceed the maximum lVelo available to this vehicle, just
  //add together. Otherwise add as much of the ForceToAdd vector is
  //possible without going over the max.
  if (MagnitudeToAdd < MagnitudeRemaining)
  {
    RunningTot += ForceToAdd;
  }

  else
  {
    //add it to the steering lVelo
	  ForceToAdd.normalize();
      RunningTot += (ForceToAdd * MagnitudeRemaining); 
  }

  return true;
}
// --------------------------------------------------------------------------------------------
bool AIEntity::doesFollowLeader()  {
  return mLeaderObject && getFSM()->getCurrentState() == AIState::Follow::Instance(); 
}

// --------------------------------------------------------------------------------------------
/**
 * This method calculates the moves for the AI player
 *
 * @param movePtr Pointer to move the move list into
 */
bool AIEntity::getAIMove(Move *move)
{

   *move = NullMove;
   VectorF lVelo(0.f, 0.f, 0.f);


   //2023-03-04 i'am dead or destroyed ... why calculate a move ?! 
   if (mDamageState != Enabled)
   {
      return true; //true because we want to transmit the NullMove! 
   }


   if (mHumanControlled) {//XXTH 2.0!
      mIsAiControlled = false; //2023-02-28
      return false;
   } 
		
	if ( mMoveState == ModeStop && mJumpCnt < 1 ) {
		return false;
	}

	switch (mMoveState) {
      case ModeMove:
     // case ModeStop: //Added 2023-03-03
         if (!(mCurSpell  && mCurSpell->mStopMovement))
				lVelo = CalculatePrioritized();

			
			if (!lVelo.isZero())
         {  
				move->yaw = getSteering()->getYawToDestination( getPosition()+lVelo+getVelocity() ,mMaxRotateRadiants );
   			move->x =  lVelo.x;
				move->y =  lVelo.y;

				
				if ((getCanSwim() && getInLiquid())  || getCanFly()) {
					if (doesFollowLeader())
						move->z = (getPosition().z > mLeaderObject->getPosition().z)? -1 : 1;
					else 
						move->z = (getPosition().z > getTargetPos().z)? -1 : 1;
				} else {
				     move->z = 0.f;
				}

				//Con::printf("XXR MOVE:%f,%f,%f YAW:%g", move->x, move->y,move->z, move->yaw);
				// Rotate the move into object space (this really only needs
				// a 2D matrix)

				Point3F rotation = getRotation();
				Point3F newMove;
				MatrixF moveMatrix;
				moveMatrix.set(EulerF(0, 0, -(rotation.z + move->yaw)));
				moveMatrix.mulV( Point3F( move->x, move->y, move->z ), &newMove );
				move->x = newMove.x;
				move->y = newMove.y;
				move->z = newMove.z;

				// Next do pitch.
				if ((mAimObject || doesFollowLeader()) && ((getCanSwim() && getInLiquid()) || getCanFly())) {
					// This should be adjusted to run from the
					// eye point to the object's center position. Though this
					// works well enough for now.
					F32 vertDist = 0.f;
					if (doesFollowLeader()) //2.10 we need to use leaderobject here !! 
					{
						vertDist =  mLeaderObject->getPosition().z - getPosition().z;
					} else {
						vertDist = getTargetPos().z - getPosition().z;
					}
					F32 newPitch = mAtan2( lVelo.len(), vertDist ) - ( M_PI / 2.0f );
					if (mFabs(newPitch) > 0.01) {
						Point3F headRotation = getHeadRotation();
						move->pitch = newPitch - headRotation.x;
					} 
				}

//				if (mStuckBeam) 
//				{
				//2.10 added mFalling! 
				if (!mFalling && !mMoveBlocked && lVelo.len() > 1.f && !mPlayerCantMove && AIMath::Distance3D(getPosition(),mLastLocation)<0.02f) { //0.031 pre 2.10 was 0.01! now 0.015!

//Con::errorf(" MOVE STUCK CHECK of %s DISTANCE: %f COUNTER: %d velo.len: %f",getShapeName(),AIMath::Distance3D(getPosition(),mLastLocation), mStuckCounter,lVelo.len());
						mStuckCounter ++;

						/* lol bad idea!!
						if (mStuckCounter > 3 && !getInLiquid()) //2.1 :D
						{
								jump();
						}
						*/

						if (mStuckCounter > 6) {
							mStuckCounter = 0;
							if (mStuckBeam)
							{
								if (doesFollowLeader())
									jumpToPoint(mLeaderObject->getPosition());
								else if ( getAimObject() )
									jumpToPoint(getTargetPos());
								
							} else {
								if (mStateMachine->getCurrentState() == AIState::Wander::Instance()) //XXTH 1.97e lol i set != must be ==
								{
									clearAim();
									mStateMachine->ChangeState(AIState::Pause::Instance());
								} else {
									throwCallback("onMoveStuck");
								}
								
								
							}
							return false;
						}
					} else {
						mStuckCounter = 0;

					}
					mLastLocation = getPosition();
//				}					 
	
			} else {
				//Con::errorf("*******************no velocity!!!!");

				if (mAimObject) {//look at aimobject if standing still 
					move->yaw = getSteering()->getYawToDestination( getTargetPos() ,mMaxRotateRadiants );
					if ((getCanSwim() && getInLiquid())  || getCanFly()) {
						move->z = (getPosition().z > getTargetPos().z)? -1 : 1;
					} else {
						move->z = 0;
					}
				}
                //NO WE NEVER STOP LOL .... setMoveState(ModeStop);  //added 2023-02.17
			}
			break;
		case ModeLook:
				move->yaw = getSteering()->getYawToDestination( getTargetPos() ,mMaxRotateRadiants );

			break;
        default: break;
	} //switch Mode

    if (mJumpCnt > 0 ) 
	{
		move->trigger[JumpTriggerID]=true;
		mJumpCnt--;

	}


	return true;
}
// --------------------------------------------------------------------------------------------
/**
 * Sets the location for the bot to run to
 *
 * @param location Point to run to
 */
void AIEntity::setTargetPosition( const Point3F &location )
{
   mTargetPosition = location;
}
// --------------------------------------------------------------------------------------------
void AIEntity::setMoveState(AIMoveState newState) {
	if (mMoveState == newState) return;
	mMoveState = newState;	

/*
    if (isServerObject()) {
     setMaskBits(AIPlayerMask);
    }
*/

}

//-----------------------------------------------------------------------------------------------------------
/**
 * Clears the aim location and sets it to the bot's
 * current destination so he looks where he's going
 */
void AIEntity::clearAim() 
{
   mAimObject = nullptr;
   mAimCounterStrike = false;
   if (mLastAimTime != 0) //do not call it multiple times!
		throwCallback("onClearAimObject");
   mLastAimTime = 0;
   
}

//-----------------------------------------------------------------------------------------------------------
void AIEntity::setCounterStrike(bool lCounterStrike)
{

	mAimCounterStrike = lCounterStrike; //overwrite  counterstrike!
	if (lCounterStrike)
		mCounterStrikeResetCounter = 480; //!! MUST BE LONGER THAN 6 sec (192) IT TAKES SOME TIME TO WALK TO TARGET IF OUT OF RANGE => 15sec!
	else
		mCounterStrikeResetCounter = 0;

}
//-----------------------------------------------------------------------------------------------------------
/**
 * Sets the object the bot is targeting
 *
 * @param targetObject The object to target
 */
bool AIEntity::setAimObject( ShapeBase *targetObject, bool lForce , bool lCounterStrike  )
{   
   	if (mDamageState != Enabled ) 
		return false;

    if (mCurSpell  && mCurSpell->mStopMovement)
		return false;

   if (!targetObject || targetObject == this) {
	   clearAim();
	   return true; //was set to empty ball out.
   }

   // we still have the same AimObject: 
   if (mAimObject && mAimObject == targetObject /* XXTH 1.99 switch to often! && (mPersonality!=Sheepish) */) {
	   setCounterStrike(lCounterStrike); //overwrite  counterstrike!
	   return false;
   }

   U32 lCurTime = Sim::getCurrentTime();
   if (!lForce && mAimObject && targetObject && mLastAimTime + 5500 > lCurTime)   //delay to prevent fast aimswitching.
	    return false; 

   mLastAimTime = lCurTime;
   mAimObject = targetObject;
   setCounterStrike(lCounterStrike);
   throwCallback("onNewAimObject");

   if (mConstantAttackDistance > 0.0f ) {
        mTargetDistance =  mConstantAttackDistance;
   } else {
		Box3F tmpBox1 = this->getWorldBox();
		Box3F tmpBox2 = targetObject->getWorldBox();
		mTargetDistance= 0 ;
		F32 tmp1 =  (tmpBox1.maxExtents.y -  tmpBox1.minExtents.y) / 2;
		F32 tmp2 =  (tmpBox1.maxExtents.x -  tmpBox1.minExtents.x) / 2;
		if (tmp1>tmp2) {
				mTargetDistance+=tmp1;
		} else {
				mTargetDistance+=tmp2; 
		}
		tmp1 =  (tmpBox2.maxExtents.y -  tmpBox2.minExtents.y) / 2;
		tmp2 =  (tmpBox2.maxExtents.x -  tmpBox2.minExtents.x) / 2;
		if (tmp1>tmp2) {
				mTargetDistance+=tmp1;
		} else {
				mTargetDistance+=tmp2; 
		}

		mTargetDistance += mAttackDistanceAdd + mMoveTolerance; 
   }

    //XXTH state check added for pet test!!! (2009-12-13)
   if (!mHumanControlled)
   {
		if (mAimCounterStrike || ( mStateMachine->getCurrentState() != AIState::Hunt::Instance() && mStateMachine->getCurrentState() != AIState::Attack::Instance()))
			mStateMachine->ChangeState(AIState::Hunt::Instance());
   }

   return true;

}
// --------------------------------------------------------------------------------------------
/**
 * Sets the second object the bot is targeting
 * Used for Interpose for example
 * @param targetObject The object to target
 */
bool AIEntity::setAimObject2(ShapeBase* targetObject )
{   
	if (!targetObject || targetObject == this || targetObject == mAimObject2) {
	   mAimObject2 = nullptr;
	   return true; //was set to empty ball out.
	}
   

   if (mAimObject2 && mAimObject2 == targetObject) return false;

   mAimObject2 = targetObject;

   throwCallback("onNewAimObject2");
   return true;
}

// --------------------------------------------------------------------------------------------
/**
 * Sets the mLeaderObject for offset pursuit 
 * and pets
 * @param targetObject The object to target
 */
bool AIEntity::setLeaderObject(ShapeBase* targetObject )
{   
	if (!targetObject || targetObject == this || targetObject == mLeaderObject) {
	   mLeaderObject = nullptr;
	   return true; //was set to empty ball out.
	}
   

   if (mLeaderObject && mLeaderObject == targetObject) 
	   return false;

   mLeaderObject = targetObject;

   throwCallback("onNewLeaderObject");
   return true;
}

// --------------------------------------------------------------------------------------------
bool AIEntity::setObstancleGroup(SimGroup* lObstGroup)
{
	if (!lObstGroup) {
	   mObstGroup = NULL;
	   return true; //was set to empty ball out.
	}
    mObstGroup = lObstGroup;

   return true;
}

// --------------------------------------------------------------------------------------------
/**
 * Utility function to throw callbacks. Callbacks always occure
 * on the datablock class.
 *
 * @param name Name of script function to call
 */
void AIEntity::throwCallback( const char *name )
{
   Con::executef(getDataBlock(), name, getIdString());
}
// --------------------------------------------------------------------------------------------
void AIEntity::scanNeighbours()
{
	if (!mScanNeighbours) 
		return;
	mNeighbours.mList.clear();


   Box3F queryBox(this->getPosition(), this->getPosition());
 	queryBox.minExtents -= Point3F(mScanRadius, mScanRadius, mScanRadius);
   queryBox.maxExtents += Point3F(mScanRadius, mScanRadius, mScanRadius);
	disableCollision(); //not scan for the object itself!
//FIXME PlayerObjectType ?!
   gServerContainer.findObjects(queryBox, PlayerObjectType, SimpleQueryList::insertionCallback, &mNeighbours);
	enableCollision();

}
// --------------------------------------------------------------------------------------------
void AIEntity::setMaxRotateAngle(S32 lAngle) {
	lAngle = mClamp(lAngle,1,180);
    mMaxRotateRadiants = lAngle * M_RAD; // M_RAD = M_PI / 180; first value° in rad && KEEP IT LOW TO DENY JITTER
}
// --------------------------------------------------------------------------------------------
bool AIEntity::isDeathAnimation(S32 lAnimId)
{
		  return  
		    lAnimId == action_death || 
			lAnimId == action_death2 || 
			lAnimId == action_death3 || 
			lAnimId == action_death4 ||
			lAnimId == action_death5;

}

bool AIEntity::inDeathAnim()
{

   if (mActionAnimation.thread && mActionAnimation.action >= 0)
		  return  isDeathAnimation(mActionAnimation.action);

   return false;
}


bool AIEntity::onAdd()
{
   ActionAnimation serverAnim = mActionAnimation;

   if(!ShapeBase::onAdd() || !mDataBlock)
      return false;

   mWorkingQueryBox.minExtents.set(-1e9f, -1e9f, -1e9f);
   mWorkingQueryBox.maxExtents.set(-1e9f, -1e9f, -1e9f);

   addToScene();

   // Make sure any state and animation passed from the server
   // in the initial update is set correctly.
   ActionState state = mState;
   mState = NullState;
   setState(state);

   if (serverAnim.action != PlayerData::NullAnimation)
   {
      setActionThread(serverAnim.action, true); //, serverAnim.holdAtEnd, true, false, true);
      if (serverAnim.atEnd)
      {
         mShapeInstance->clearTransition(mActionAnimation.thread);
         mShapeInstance->setPos(mActionAnimation.thread,
                                mActionAnimation.forward? 1: 0);
         if (inDeathAnim())
            mDeath.lastPos = 1.0f;
      }

      // We have to leave them sitting for a while since mounts don't come through right
      // away (and sometimes not for a while).  Still going to let this time out because
      // I'm not sure if we're guaranteed another anim will come through and cancel.
      if (!isServerObject() && inSittingAnim())
         mMountPending = (S32) sMountPendingTickWait;
      else
         mMountPending = 0;
   }
   //
   if (isServerObject())
   {
      scriptOnAdd();

   }
   else
   {
      U32 i;
      for( i=0; i<PlayerData::NUM_SPLASH_EMITTERS; i++ )
      {
         mSplashEmitter[i] = new ParticleEmitter;
         mSplashEmitter[i]->onNewDataBlock( mDataBlock->splashEmitterList[i], false  );


         if( !mSplashEmitter[i]->registerObject() )
         {
            Con::warnf( ConsoleLogEntry::General, "Could not register dust emitter for class: %s", mDataBlock->getName() );
            delete mSplashEmitter[i];
            mSplashEmitter[i] = NULL;
         }
      }
      mLastWaterPos = getPosition();

      // clear out all camera effects
      gCamFXMgr.clear();
   }
   initAnimations();

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


   return true;
}
bool AIEntity::onNewDataBlock(GameBaseData* dptr, bool reload)
{

   mDataBlock = dynamic_cast<PlayerData*>(dptr);
   if (!mDataBlock || !Parent::onNewDataBlock(dptr, reload))
      return false;

   initAnimations();
	return true;
}


// --------------------------------------------------------------------------------------------
// initAnimations
// --------------------------------------------------------------------------------------------
void AIEntity::initAnimations() {
    action_idle			= getActionbyName("idle");
	if (action_idle == -1) 
       action_idle			= getActionbyName("root");
	if (action_idle == -1) 	
	   Con::errorf("AIEntity - no idle or root animation!! id: %s", this->getIdString());

    //1.97.3 sleep!
	action_sleep = getActionbyName("sleep");
	if (action_sleep == -1) 
		action_sleep = action_idle;

	action_sit			= getActionbyName("sit");
	if (action_sit == -1) 
		action_sit	= getActionbyName("sit_loop");
	if (action_sit == -1) 
		action_sit	= getActionbyName("sleep");
	if (action_sit == -1) 
		action_sit = action_idle;
    action_cready		= getActionbyName("cready");
    action_cready1h		= getActionbyName("cready1h");
    action_cready2h		= getActionbyName("cready2h");
	if (action_cready == -1)  //1.97.3 we may have cready1h but no normal cready! (like on skellies!)
	{
		if (action_cready1h != -1)
			action_cready = action_cready1h;
		else
			action_cready = action_idle;
	}

    action_death		= getActionbyName("death");
	if (action_death == -1)
		action_death		= getActionbyName("die");
	if (action_death == -1)
		action_death		= getActionbyName("Death1");
	if (action_death == -1)
		Con::errorf("AIEntity - no death,die or death1 animation!! id:%s", this->getIdString());

	action_death2	= getActionbyName("Death2");
	if (action_death2 == -1)
		action_death2		= action_death;
	action_death3	= getActionbyName("Death3");
	if (action_death3 == -1)
		action_death3		= action_death;
	action_death4	= getActionbyName("Death4");
	if (action_death4 == -1)
		action_death4		= action_death;
	action_death5	= getActionbyName("Death5");
	if (action_death5 == -1)
		action_death5		= action_death;


	
    action_mountsit		= getActionbyName("mountsit");
	action_flyidle		= getActionbyName("flyidle");

	//motion
    action_run			= getActionbyName("run");
	if ( action_run  == -1)
			action_run = getActionbyName("1hrun");
	
    action_walk			= getActionbyName("walk");
	if (action_walk	 == -1)
		action_walk	   = getActionbyName("1hwalk");
	if (action_walk	 == -1)
		action_walk			= action_run;
    //fallback if we have walk but not run:
    if (action_run == -1 && action_walk >= 0)
		action_run			= action_walk;

	action_back			= getActionbyName("back");
	if (action_back	 == -1)
		action_back			= action_walk;

    action_1hrun		= getActionbyName("1hrun");
    action_1hwalk		= getActionbyName("1hwalk");
    action_2hrun		= getActionbyName("2hrun");
    action_2hwalk		= getActionbyName("2hwalk");
    action_jump			= getActionbyName("jump");
    action_fall			= getActionbyName("fall");
	if (action_fall == -1)
		action_fall		= action_run;
    action_land			= getActionbyName("land");
	if (action_land == -1)
		action_land		= action_idle;
    action_side			= getActionbyName("side");
	if (action_side == -1)
		action_side		= action_run;

    action_swim			= getActionbyName("swim");
    // T3D have swimming : swim_root, swim_forward, swim_left, swim_right, swim_backward
    if (action_swim == -1)
       action_swim      = getActionbyName("swim_forward");
	if (action_swim == -1)
		action_swim		= action_walk;

   //left/right/back unused atm 
    action_swim_left    = getActionbyName("swim_left");
    if (action_swim_left == -1)
       action_swim_left = action_swim;

    action_swim_right   = getActionbyName("swim_right");
    if (action_swim_right == -1)
       action_swim_right = action_swim;

    action_swim_back = getActionbyName("swim_backward");
    if (action_swim_back == -1)
       action_swim_back = action_swim;

	action_swimidle		= getActionbyName("swimidle");
   if (action_swimidle == -1)
      action_swimidle = getActionbyName("swim_root");
   if (action_swimidle == -1) {
     if (action_swim == action_walk)
       action_swimidle = action_idle;
     else
       action_swimidle = action_swim;
   }


	action_fly		= getActionbyName("fly");
	if (action_fly == -1)
		action_fly	= action_swim;
	if (action_fly == -1)
		action_fly		= action_walk;
	action_flyglide		= getActionbyName("flyglide");
	if (action_flyglide == -1)
		action_flyglide		= action_idle;
	if (action_flyglide == -1)
		action_flyglide		= action_swimidle;

	//misc actions
    action_shieldblock	= getActionbyName("shieldblock");
    action_spellprepare	= getActionbyName("spellprepare");
    action_spellcast	= getActionbyName("spellcast");
    action_spellcast2	= getActionbyName("spellcast2");

	//emotions
    action_agree		= getActionbyName("agree");
    action_disagree		= getActionbyName("disagree");
    action_bow			= getActionbyName("bow");
    action_wave			= getActionbyName("wave");
    action_point		= getActionbyName("point");
    action_bowattack	= getActionbyName("bowattack");
    action_dance		= getActionbyName("dance");
	action_cartwheel	= getActionbyName("cartwheel");
	action_handstand    = getActionbyName("handstand");	

	//attack actions
    action_unarmedright	= getActionbyName("unarmedright");
    action_unarmedleft	= getActionbyName("unarmedleft");
    action_kick1		= getActionbyName("kick1");
    action_kick2		= getActionbyName("kick2");
    action_attack1		= getActionbyName("attack1");
	if (action_attack1 == -1 ) 
		action_attack1		= getActionbyName("attack");

	action_attack2		= getActionbyName("attack2");
	if (action_attack2 == -1 ) 
		action_attack2 = action_attack1;
    action_attack3		= getActionbyName("attack3");
	if (action_attack3 == -1 ) 
		action_attack3 = action_attack1;
    action_attack4		= getActionbyName("attack4");
	if (action_attack4 == -1 ) 
		action_attack4 = action_attack1;
    action_1hslashright	= getActionbyName("1hslashright");
    action_1hslash2right	= getActionbyName("1hslash2right");
	if (action_1hslashright >=0 &&  action_1hslash2right==-1)
		action_1hslash2right	= action_1hslashright;
    action_1hslash3right	= getActionbyName("1hslash3right");
	if (action_1hslashright >=0 &&  action_1hslash3right==-1)
		action_1hslash3right	= action_1hslashright;
    action_1hthrustright	= getActionbyName("1hthrustright");
	if (action_1hslashright >=0 &&  action_1hthrustright == -1)
		action_1hthrustright	= action_1hslashright;
    action_1hslashleft	= getActionbyName("1hslashleft");
    action_1hslash2left	= getActionbyName("1hslash2left");
    action_1hslash3left	= getActionbyName("1hslash3left");
    action_1hthrustleft	= getActionbyName("1hthrustleft");

    action_2hslash		= getActionbyName("2hslash");
    action_2hslash2		= getActionbyName("2hslash2");
    action_2hslash3		= getActionbyName("2hslash3");
    action_2hthrust		= getActionbyName("2hthrust");

    action_pain1		= getActionbyName("pain1");
	if ( action_pain1 == -1) 
		action_pain1		= getActionbyName("pain"); //fallback
	/* thats really sucks! removed 1.97.3
	   if ( action_pain1 == -1) 
	     	action_pain1		= action_idle;
    */
    action_pain2		= getActionbyName("pain2");

	if ( action_pain2 == -1) 
		action_pain2 = action_pain1;


}
//----------------------------------------------------------------------------
S32  AIEntity::getRandomDeathAnimation()
{
   switch (gRandGen.randI(1,5))
   {
		case 1: 
				return action_death; //action_death1
				break;
		case 2: 
				return action_death2;
				break;
		case 3: 
				return action_death3;
				break;
		case 4: 
				return action_death4;
				break;
		case 5: 
				return action_death5;
				break;

   }

   return action_death;

}
//----------------------------------------------------------------------------
void AIEntity::GuessActionAnimation() {


	if (anim_clip_flags & ANIM_OVERRIDDEN) {
		return;
	}

	if (mPainRunning) {
		if (!mActionAnimation.atEnd) 
			return;
		else
			mPainRunning = false;
	}

	if (mDamageState != Enabled && !inDeathAnim() ) { 
			setActionThread(getRandomDeathAnimation(),true,true);
			return;
	}
	S32 a_run  = -1;
	S32 a_idle = -1;
	S32 a_cready = -1;
 
    bool isTwoHanded = mWeaponHands == 4;

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
    mShieldBlock = false;

	if (mWaterCoverage < sSwimWaterCoverage && !isMounted()) {
		if (mFalling && action_fall>=0) {
			a_idle = action_fall;
			a_run = action_fall;
			mJumping = false;
		} else  if (mSitting && action_sit >= 0 ) {
			a_idle = action_sit;
		} else  if (mFlying && action_flyidle >= 0 ) {
			a_idle = action_flyidle;
		} else  if (mDoPauseSleepAnimaion && mStateMachine->getCurrentState() == AIState::Pause::Instance() ) {
			a_idle = action_sleep;
		} else {
			a_idle = action_idle;
		}

	   if (mBasicAnimation) {

            if (mFlying && action_fly >=0) 
				a_run  = action_fly;
			else if (mSpeedModifier >= mRunMoveModifier && mMoveDirection == MoveForward)
				a_run  = action_run;
			else if (mMoveDirection == MoveSide && action_side>=0) 
				a_run  = action_side;
			else if (mMoveDirection == MoveBackward && action_back>=0) 
				a_run  = action_back;
			else
	            a_run  = action_walk;



	   } else if (isTwoHanded) {
            if (mFlying && action_fly >=0) 
				a_run  = action_fly;
			else if (mSpeedModifier >= mRunMoveModifier && mMoveDirection == MoveForward) 
				a_run  = action_2hrun;
			else if (mMoveDirection == MoveSide && action_side>=0) 
				a_run  = action_side;
			else
	            a_run  = action_2hwalk;
			
	   } else {
            if (mFlying && action_fly >=0) 
				a_run  = action_fly;
			else if (mSpeedModifier >= mRunMoveModifier && mMoveDirection == MoveForward) 
				a_run  = action_1hrun;
			else if (mMoveDirection == MoveSide && action_side>=0) 
				a_run  = action_side;
			else
	            a_run  = action_1hwalk;
			
	   }
	} else if (isMounted()) { //2.10 mountsit!
		a_run  = action_mountsit;
		a_idle = action_mountsit;
	} else {
       mSitting = false;
	   a_idle = action_swimidle;
	   a_run  = action_swim;
	}

	if (mBasicAnimation)
	 a_cready = action_cready;
	else if (isTwoHanded) 
	  a_cready = action_cready2h;
	else
	  a_cready = action_cready1h;
  

  Point3F vel = getVelocity();
  if (!mFalling && ( mFabs(vel.x)>0.1f ||  mFabs(vel.y)>0.1f ) ) {
	  mEmotion = false;	 //stop emotion after move!

     if (!mJumping) 
		setActionThread(a_run);
	 else 
	    setActionThread(action_jump);
	 //reset some states on run...
	 mCready=false;
	 if (mSitting) {
		mSitting = false;
	 }
  } else {
	  if (mSitting) {
		  mCready = false;
		  mEmotion = false;	 //stop emotion on sitting
	  }
      if (mEmotion  && !mActionAnimation.atEnd)  //lets finish emotion in idle
			return;
	  mEmotion = false;
	  if (mCready ) 
	  {
		  if (mFlying && action_flyidle >= 0 ) 
			  setActionThread(action_flyidle);
		  else
			  setActionThread(a_cready);
	  }
      else if (mJumping) 
  	    setActionThread(action_jump);
	 else 
		setActionThread(a_idle);
	  
  }

}
// --------------------------------------------------------------------------------------------
MatrixF * AIEntity::getGroundConform(const Point3F& pos,  MatrixF orgMat)
{

	if (mSwimming
		|| mFlying
		) return NULL; //not when swim or fly


//XXTH T3D	return Parent::getGroundConform(pos, orgMat);


      if (isServerObject()) return NULL; //NOT ON SERVER!

      static MatrixF retMat(true);

      retMat = orgMat;
      RayInfo surfaceInfo;
      //         Point3F above = Point3F(pos.x,pos.y,pos.z + 0.5f);
      //         Point3F below = Point3F(pos.x,pos.y,pos.z - 100.0f);

      Point3F above = Point3F(pos.x, pos.y, pos.z + 0.1f);
      Point3F below = Point3F(pos.x, pos.y, pos.z - 2.0f);
      disableCollision();



      U32 conformMask = (TerrainObjectType |  StaticShapeObjectType | VehicleObjectType | DynamicShapeObjectType);


      if (gClientContainer.castRay(above, below, conformMask, &surfaceInfo))
      {
         Point3F z;
         if (mDataBlock->conformToGround) { // XXTH do test on all objects!
            Point3F y, x;
            retMat.getColumn(1, &y);
            z = surfaceInfo.normal;
            z.normalize();
            mCross(y, z, &x);
            x.normalize();
            mCross(z, x, &y);
            retMat.setColumn(0, x);
            retMat.setColumn(1, y);
            retMat.setColumn(2, z);
         }
         else {
            z = surfaceInfo.normal;
            z.normalize();
         }
         // !! ONLY ADJUST SINK ON TERRAIN it does not work on other surfaces so well !! 
         if (surfaceInfo.object->getTypeMask() & TerrainObjectType) {
            // Adjust Z down to account for box offset on slope.  Figure out how
            // much we want to sink, and gradually accel to this amount.  Don't do if
            // we're conforming to stairs though
            Point3F tmpboxsize = mDataBlock->boxSize * getScale();

            F32   boxRad = (mSqrt(tmpboxsize.x * tmpboxsize.y) * 0.5f);
            F32   xy = mSqrt(z.x * z.x + z.y * z.y);
            F32   desiredSink = (boxRad * xy / z.z);
            Point3F  position(pos);

            //DO NOT MORE SINK THAN boxRad! .. mhh still to much i guess
            if (desiredSink > boxRad)
               desiredSink = boxRad;

            position.z -= desiredSink;

            retMat.setColumn(3, position);
         }

      }
      enableCollision();

      return &retMat;
   

}
// --------------------------------------------------------------------------------------------
void AIEntity::pickActionAnimation()
{
   if (!isGhost()) 
		GuessActionAnimation();
}
// --------------------------------------------------------------------------------------------
bool AIEntity::setActionThread(const char* sequence, bool setOverwriteAnim, bool moveBlocked)
{
   // AFX CODE BLOCK (anim-clip) <<
   if (anim_clip_flags & ANIM_OVERRIDDEN)
     return false;
   // AFX CODE BLOCK (anim-clip) >>

	S32 action=getActionbyName(sequence);
	if (action>=0 && action < mDataBlock->actionCount) {
		if (setActionThread(action,setOverwriteAnim, moveBlocked)) { 
         return true;
		}

	}
	return false;
}
// --------------------------------------------------------------------------------------------
bool AIEntity::setActionThread(S32 action,  bool setOverwriteAnim, bool moveBlocked)
{


   if (mlastAnimationAction == action)  //1.99 moved to to of function was after "bool fps=true" before
	   return false;


   if (action < 0 || action > mDataBlock->actionCount)
   {
#ifdef TORQUE_DEBUG
//SPAM      Con::errorf("AIEntity::setActionThread(%d): action out of range", action);
#endif
      return false;
   }

	bool forward = true;
	bool hold = false;
	bool wait = true;
	//bool forceSet = false;
	bool fsp = true; 

    

	if (mDamageState != Enabled &&  !isDeathAnimation(action))
		return false;
	if (mMoveBlocked && !moveBlocked)
      return false;

    if (action == action_jump) 
		hold = true;

   //  if (mActionAnimation.action == action )   return false;

   PlayerData::ActionAnimation &anim = mDataBlock->actionList[action];

//FIXME BUG HERE !!!! No animation when connected to dedicated server => anim.sequence is -1 !!!!!!!!!

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

      if (/*sUseAnimationTransitions &&*/ (isGhost()))
      {
         // The transition code needs the timeScale to be set in the
         // right direction to know which way to go.
         F32   transTime = sAnimationTransitionTime;

//XXTH mmo         if (mDataBlock && mDataBlock->isJumpAction(action)) transTime = 0.15f;
		 /* 2.1 so ein quatsch
		 if (action == action_jump) 
				transTime = 0.15f;
		 else if (action == action_shieldblock) //2.10 longer shieldblock
				transTime = 0.75f;
         */


         mShapeInstance->setTimeScale(mActionAnimation.thread,
            mActionAnimation.forward? 1: -1);
         mShapeInstance->transitionToSequence(mActionAnimation.thread,anim.sequence,
            mActionAnimation.forward? 0: 1, transTime, true);
      }
      else
         mShapeInstance->setSequence(mActionAnimation.thread,anim.sequence,
            mActionAnimation.forward? 0: 1);


		//char buf1[128];
		//dSprintf(buf1,sizeof(buf1),"%s",anim.name);
		//Con::executef(mDataBlock,"onNewAnimation", getIdString(),buf1);
      Con::executef(mDataBlock, "onNewAnimation", getIdString(), anim.name);

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
// --------------------------------------------------------------------------------------------
void AIEntity::updateActionThread()
{

   // Select an action animation sequence, this assumes that
   // this function is called once per tick.

	//20141221 crash here sucks!
	if (!mActionAnimation.thread)
			return;


   if(mActionAnimation.action != -1) {
      if (mActionAnimation.forward)  mActionAnimation.atEnd = mShapeInstance->getPos(mActionAnimation.thread) == 1;
      else mActionAnimation.atEnd = mShapeInstance->getPos(mActionAnimation.thread) == 0;
   }

	  


   // Only need to deal with triggers on the client
   if (isGhost())  {
      bool triggeredLeft = false;
      bool triggeredRight = false;
      F32 offset = 0.0f;
      if(mShapeInstance->getTriggerState(1) || mShapeInstance->getTriggerState(3)) { //XXTH 3 added
         triggeredLeft = true;
         offset = -mDataBlock->decalOffset;
      }
      else if(mShapeInstance->getTriggerState(2) || mShapeInstance->getTriggerState(4)) { //XXTH 4 added 
         triggeredRight = true;
         offset = mDataBlock->decalOffset;
      }
   
	  // XXTH extra Triggers ! 
			for (U32 j = 5; j < 33; j++) {
				if (mShapeInstance->getTriggerState(j)) {
					Con::executef(mDataBlock,"onAnimationTrigger", getIdString(),Con::getIntArg(j));
				}
			}


         //FIXME T3D foot sounds and puffs
         if ((triggeredLeft || triggeredRight) ) // FIXME && !noFootfallFX)
         {
            Point3F rot, pos;
            RayInfo rInfo;
            MatrixF mat = getRenderTransform();
            mat.getColumn(1, &rot);
            mat.mulP(Point3F(offset, 0.0f, 0.0f), &pos);

            if (gClientContainer.castRay(Point3F(pos.x, pos.y, pos.z + 0.01f),
               Point3F(pos.x, pos.y, pos.z - 2.0f),
                                 (U32)STATIC_COLLISION_TYPEMASK | (U32)VehicleObjectType, &rInfo))
            {
               Material* material = (rInfo.material ? dynamic_cast<Material*>(rInfo.material->getMaterial()) : 0);

               // Put footprints on surface, if appropriate for material.

               if (material && material->mShowFootprints
                  && mDataBlock->decalData ) //FIXME && !footfallDecalOverride)
               {
                  Point3F normal;
                  Point3F tangent;
                  mObjToWorld.getColumn(0, &tangent);
                  mObjToWorld.getColumn(2, &normal);
                  gDecalManager->addDecal(rInfo.point, normal, tangent, mDataBlock->decalData, getScale().y);
               }

               // Emit footpuffs.

               //FIXME if (!footfallDustOverride &&
               if (rInfo.t <= 0.5f && mWaterCoverage == 0.0f
                  && material && material->mShowDust)
               {
                  // New emitter every time for visibility reasons
                  ParticleEmitter* emitter = new ParticleEmitter;
                  emitter->onNewDataBlock(mDataBlock->footPuffEmitter, false);

                  LinearColorF colorList[ParticleData::PDC_NUM_KEYS];

                  for (U32 x = 0; x < getMin(Material::NUM_EFFECT_COLOR_STAGES, ParticleData::PDC_NUM_KEYS); ++x)
                     colorList[x].set(material->mEffectColor[x].red,
                        material->mEffectColor[x].green,
                        material->mEffectColor[x].blue,
                        material->mEffectColor[x].alpha);
                  for (U32 x = Material::NUM_EFFECT_COLOR_STAGES; x < ParticleData::PDC_NUM_KEYS; ++x)
                     colorList[x].set(1.0, 1.0, 1.0, 0.0);

                  emitter->setColors(colorList);
                  if (!emitter->registerObject())
                  {
                     Con::warnf(ConsoleLogEntry::General, "Could not register emitter for particle of class: %s", mDataBlock->getName());
                     delete emitter;
                     emitter = NULL;
                  }
                  else
                  {
                     emitter->emitParticles(pos, Point3F(0.0, 0.0, 1.0), mDataBlock->footPuffRadius,
                        Point3F(0, 0, 0), mDataBlock->footPuffNumParts);
                     emitter->deleteWhenEmpty();
                  }
               }

               // Play footstep sound.

               //FIXME if (footfallSoundOverride <= 0)
               playFootstepSound(triggeredLeft, material, rInfo.object);
            }
         }


   }
   // Mount pending variable puts a hold on the delayTicks below so players don't
   // inadvertently stand up because their mount has not come over yet.
   if (mMountPending)
      mMountPending = (isMounted() ? 0 : (mMountPending - 1));

   if (true || mActionAnimation.action == -1 ||
       ((!mActionAnimation.waitForEnd || mActionAnimation.atEnd)) &&
       !mActionAnimation.holdAtEnd && (mActionAnimation.delayTicks -= !mMountPending) <= 0)
   {
      pickActionAnimation();
   }

// AFX CODE BLOCK (anim-clip) <<
   if ( (mActionAnimation.action != -1) &&
        !(anim_clip_flags & ANIM_OVERRIDDEN)
       )
   {
      // Update action animation time scale to match ground velocity
      PlayerData::ActionAnimation &anim = 
         mDataBlock->actionList[mActionAnimation.action];
      F32 scale = 1;
      if (anim.velocityScale && anim.speed) {
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





// --------------------------------------------------------------------------------------------
/**
 * Stops movement and set to static
 */
void AIEntity::setStatic(bool activate) 
{
	if (activate) {
		setMoveState(ModeStatic);
	} else {
		setMoveState(ModeStop);
	}
}
// --------------------------------------------------------------------------------------------
void AIEntity::setSpawnPoint( Point3F SpawnPoint , bool lCheckExtras){
  mSpawnPoint=SpawnPoint;
  if (!lCheckExtras)
		return;
  mSpawnInsideMissionArea = AIMath::PointInMissionArea(SpawnPoint);
  // mSpawnInsideInterior    = PointInInterior(SpawnPoint);
  // mSpawnInsideWater    = PointInWater(SpawnPoint);
}

//---------------------------------------------------------------------------------------------------------------

F32 AIEntity::getDistanceFromSpawn(GameBase* a)
{
	return AIMath::Distance2D(a->getPosition(), mSpawnPoint);
}
//---------------------------------------------------------------------------------------------------------------
//2023-12-03 changed to  getDistance2D
F32 AIEntity::getDistance2D(GameBase* a)
{
	return AIMath::Distance2D(getPosition(), a->getPosition());
}
F32 AIEntity::getDistance3D(GameBase* a)
{
   return AIMath::Distance3D(getPosition(), a->getPosition());
}
//---------------------------------------------------------------------------------------------------------------
bool AIEntity::isTouchable(SimObjectId TargetID, bool checkSheepish, bool usuallyIgnored) {

	
 if (checkSheepish) {
	return dStrcmp(Con::executef(getDataBlock(), "onCheckSheepishTouchable", getIdString(),Con::getIntArg(TargetID),Con::getIntArg(usuallyIgnored)),"0") !=0;
 } else {
	return dStrcmp(Con::executef(getDataBlock(), "onCheckTouchable", getIdString(),Con::getIntArg(TargetID),Con::getIntArg(usuallyIgnored)),"0") !=0;
 }
}
//---------------------------------------------------------------------------------------------------------------
bool AIEntity::checkShouldIgnore(ShapeBase* target) {

#ifdef FIXME_T3D //FIXME T3D
   if (mIgnoreBP == 0 || mIgnoreBP <= target->getBattlePoints())
      return true;

   if (mLeaderObject && target == mLeaderObject || (getFaction() != 0 && getFaction()==target->getFaction()) )
		  return true;
#else
   if (mLeaderObject && target == mLeaderObject || (getFaction() != 0 && getFaction() == target->getFaction()))
      return true;
#endif
        


	  return false; 
}
//---------------------------------------------------------------------------------------------------------------
bool AIEntity::validTargetAttack()   //new 1.97.3
{
	if (!mAimObject || mAimObject->getDamageState() != Enabled)
		return false;

   //added 2023-03-04
   if (getFaction() > 0 && mAimObject->getFaction() == getFaction())
   {
      return false;
   }


	if (mAimObject->getCloakedState())
		return false;

	bool lShouldIgnore = checkShouldIgnore(getAimObject());
	if ( !mHumanControlled && lShouldIgnore && !getInCounterStrike()) //new 1.97.3 dont follow after counterstrike is over!
		return false;

    if (!isTouchable(getAimObject()->getId(),mPersonality==Sheepish, lShouldIgnore))  //1.97.3 added checkShouldIgnore AND moved to attack because of farming!
		return false;

	return true;

}

bool AIEntity::validTargetHunt() //renamed 1.97.3
{

   if (!validTargetAttack()) //pre check
		return false;

	F32 lIgnoreRange = mIgnoreRange;
	F32 lDistFromSpawn = getDistanceFromSpawn(mAimObject);
	if (getInCounterStrike())
		lIgnoreRange +=  lDistFromSpawn;
	

	bool result = lDistFromSpawn < lIgnoreRange
				  // && isTouchable(getAimObject()->getId(),mPersonality==Sheepish, checkShouldIgnore(getAimObject()))  //1.97.3 added checkShouldIgnore AND moved to attack because of farming!
				  ;
   
    //check for other params!
	if (result) {
		if (!mAllowWater && mAimObject->getInLiquid())
			return false;
		if (!mAllowLand && !mAimObject->getInLiquid())
			return false;
	}

  return (result); 
}
//---------------------------------------------------------------------------------------------------------------
void AIEntity::doAttack() {
  if (!mAimObject) return;
  

  Con::executef(getDataBlock(),  "onAttack", getIdString(), mAimObject->getIdString());
}
// --------------------------------------------------------------------------------------------
bool AIEntity::ObjectInLos(ShapeBase* lObj, U32 mask)
{

   return AIMath::CheckObjectsLos(this, lObj, mask, false);

/*

	if (!lObj) 
		return false;
  Point3F lF,lT; 
  this->getWorldBox().getCenter(&lF);
  lObj->getWorldBox().getCenter(&lT);
  RayInfo dummy;

  return !getContainer()->castRay( lF, lT, mask, &dummy);
*/
}

//---------------------------------------------------------------------------------------------------------------
bool AIEntity::timeToAttack() {
	
	
	if ((mCurSpell && ( mCurSpell->mBlockMelee || mCurSpell->mStopMovement)) || mPainRunning)
		return false;

	mAttackTickCounter --;
	if (mAttackTickCounter <=0) {
		mAttackTickCounter = mAttackTicks;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------------
bool AIEntity::inAttackDistance() {
	if (!mAimObject) return false;

   mAimInLOS = ObjectInLos(mAimObject);
	if (getDistance3D(mAimObject) <= mTargetDistance + 2.f * mMoveTolerance && mAimInLOS)
		return true;

	return false;
}
//---------------------------------------------------------------------------------------------------------------
void AIEntity::setWanderDestination(Point3F lTo)
{

	Point3F lDst;
   if (!lTo.isZero()) {
      lDst = lTo;

    } else if (getSteering()->getPathObject() && getSteering()->getPathObject()->haveNodes() &&
       !getSteering()->getPathObject()->finished()) {

       

       return;

	} else if (getDistanceFromSpawn(this) > mWanderRadius || mWanderRadius  == 0) {
		lDst = mSpawnPoint;
	} else { //wandern
	   S32  attempts= 3;
	   bool found = false;
	   bool inWater = false;
	   while (!found && attempts >0) {
		    attempts --;
			F32 zmin;
			lDst.x=gRandGen.randF()*mWanderRadius*2-mWanderRadius+mSpawnPoint.x;
			lDst.y=gRandGen.randF()*mWanderRadius*2-mWanderRadius+mSpawnPoint.y;

//1.97.3	something is wrong here it return if destination can be set, it ignore allow water/land and
//			if it failed it checks all ?!?!?!? And what does the bottom mPathUsage == AIEntity::PathHunt
//			this is never reached if it set here !!
//			Added: continue if cant set and water check!
/* 2023-11-13 removed like in auteria 2.27 
#ifdef AI_USEPATH
			if (mPathUsage != PathNone)
			{
				lDst.z=mSpawnPoint.z;
				inWater = pointInWater(lDst);  
				if ( 
					 !(!mAllowWater && inWater) && !(!mAllowLand && !inWater)
					 && getSteering()->setPathDestination(lDst)
				   )
					return;
				else 
					continue;

			}
#endif
*/

			zmin=AIMath::getTerrainHeight( lDst );
			if ((getCanSwim() && getInLiquid())  || getCanFly()) {
					lDst.z=gRandGen.randF()*mWanderRadius*2-mWanderRadius+mSpawnPoint.z;
					if (zmin>lDst.z) lDst.z=zmin;
			} else {
					lDst.z=zmin;
			}
			inWater = pointInWater(lDst);
			//CHECKS 
			if (
  			    AIMath::PointInMissionArea(lDst) == mSpawnInsideMissionArea
			    && !(!mAllowWater && inWater)
			    && !(!mAllowLand && !inWater)
				//1.97.3 added terrain check !!
				// && AIMath::simpleCheckLos(getPosition(), lDst)
				&& AIMath::checkLos(getPosition(), lDst, TerrainObjectType | TerrainLikeObjectType | InteriorLikeObjectType | StaticShapeObjectType | DynamicShapeObjectType)
				&& AIMath::simpleBoxEmpty(Point3F(lDst.x,lDst.y,lDst.z+0.7f))
				) 
				    found = true;
		
	   } //while
  
	   if (!found) 
		   lDst = mSpawnPoint;

	} //lange else wir wandern
#ifdef AI_USEPATH
	//if (mPathUsage == AIEntity::PathHunt) {
     if (mPathUsage != PathNone) {
		setTargetPosition(lDst);		
		if (!getSteering()->setPathDestination(lDst)) {
			setTargetPosition(mSpawnPoint);
			if (mSpawnPoint == lDst || !getSteering()->setPathDestination(mSpawnPoint))
			{

				//hardcore back to spawn!
				jumpToPoint(mSpawnPoint);

			}
		}
	}
	else
#endif
		setTargetPosition(lDst);
}

Point3F AIEntity::getVariDestination(Point3F location, F32 addRad)
{
   F32 tolerance = getMoveTolerance() + addRad;
   return AIMath::getVariPoint(location, tolerance);
}

//---------------------------------------------------------------------------------------------------------------

void AIEntity::forceWander(Point3F lPos) 
{
   setSpawnPoint(lPos); //2023-03-02
   mWanderPositonSet = lPos;
   mStateMachine->ChangeState(AIState::Wander::Instance());
}
//---------------------------------------------------------------------------------------------------------------
void AIEntity::jumpToPoint(Point3F lPos) 
{

   const MatrixF& tmat = getTransform();
   AngAxisF aa(tmat);
   MatrixF mat;
   aa.setMatrix(&mat);
   mat.setColumn(3,lPos);
   setTransform(mat);
}
//---------------------------------------------------------------------------------------------------------------
ShapeBase* AIEntity::getNearestTarget()
{

   ShapeBase* nearestTarget=0;

#ifdef FIXME_T3D //FIXME T3D
   if (mIgnoreBP == 0 || getPersonality() == Dontcare || getPersonality() == PersoNone) return nearestTarget;
#else
   if (getPersonality() == Dontcare || getPersonality() == PersoNone) return nearestTarget;
#endif //FIXME_T3D

   ShapeBase* tmpTarget;
   for (S32 a = 0; a < mNeighbours.mList.size(); ++a)
   {

      tmpTarget = static_cast<ShapeBase*>(&(*mNeighbours.mList[a]));

      if ((mTargetPlayerType < 0 || tmpTarget->getPlayerType() == mTargetPlayerType)
         && tmpTarget->getDamageState() == Enabled
         && !(!mAllowWater && tmpTarget->getInLiquid())
         && !(!mAllowLand && !tmpTarget->getInLiquid())
         && !checkShouldIgnore(tmpTarget)
         && getDistanceFromSpawn(tmpTarget) <= mIgnoreRange
         && !tmpTarget->getCloakedState()
         && ObjectInLos(tmpTarget) //new check added 2023-03-23
         && isTouchable(tmpTarget->getId(), mPersonality == Sheepish, checkShouldIgnore(tmpTarget))
         ) {
         if (!nearestTarget) {
            nearestTarget = tmpTarget;
         }
         else if (getDistance3D(tmpTarget) < getDistance3D(nearestTarget)) {
            nearestTarget = tmpTarget;
         }
      }

   }



	return nearestTarget;
}
// --------------------------------------------------------------------------------------------
void AIEntity::guessVitalitySpell()
{
	AIEntity* ent = this;

    if (mMagic < 0 ) //1.97.3 no magic no spell!
		return;

    F32 lCurMana = ent->getEnergyLevel();

	//check poisioned
	if (ent->mPoisoned && ent->mINVamt[gSpellAntidote.mId] > 0) {
          ent->setSpell(&gSpellAntidote);
	} 
		
	//check mana
	if (ent->getEnergyValue() < 0.5f && ent->mINVamt[gSpellBeer.mId] > 0) {
          ent->setSpell(&gSpellBeer);
	}

	// check health + remote health master
	if (!ent->mCurSpell && ent->mINVamt[gSpellHealth.mId] > 0 && lCurMana >= gSpellHealth.mMana )
	{
	   //self heal first ! 	
		if ( ent->getDamageValue() > 0.5f  )  
			ent->setSpell(&gSpellHealth, ent);
		else if (ent->getLeaderObject() 
				 && ent->getLeaderObject()->getDamageState() == Enabled 
				 && ent->getLeaderObject()->getDamageValue() > 0.5f
				 && AIMath::Distance2D(ent->getPosition(),ent->getLeaderObject()->getPosition() ) < gSpellHealth.mDistance
			)
			ent->setSpell(&gSpellHealth, ent->getLeaderObject());
	}

	

}
// --------------------------------------------------------------------------------------------
void AIEntity::guessRangeSpell()
{
	AIEntity* ent = this;

	if (!ent->mCurSpell) {
		F32 dist = AIMath::Distance2D(ent->getPosition(),ent->getTargetPos());
		
		U8 lRand = gRandGen.randI(0,4); //fire where 1=launch ..; ice = 4 is launch
         
		if (ent->mINVamt[gSpellFireBall.mId] > 0 
					&& lRand == 1
					&& gSpellFireBall.mDistance >= dist 
					&& ent->getEnergyLevel() >= gSpellFireBall.mMana + 12) {

			ent->setSpell(&gSpellFireBall, ent->getAimObject());

		}  else if (ent->mINVamt[gSpellIceBall.mId] > 0 
			&& lRand == 2
			&& gSpellIceBall.mDistance >= dist 
			&& ent->getEnergyLevel() >= gSpellIceBall.mMana + 12)  
		{
		   ent->setSpell(&gSpellIceBall, ent->getAimObject());

		}  else if (ent->mINVamt[gSpellCustomRange.mId] > 0 
			&& lRand == 3
			&& gSpellCustomRange.mDistance >= dist 
			&& ent->getEnergyLevel() >= gSpellCustomRange.mMana + 12)  
		{
		   ent->setSpell(&gSpellCustomRange, ent->getAimObject());
		}
	}

}
// --------------------------------------------------------------------------------------------
// 1.97.3 20100614 guessMeleeSpell blocks attack by random selecting spells also if ai have no spells!
//                 returning on setspell and call doAttack on the bottom not as default! 
void AIEntity::guessMeleeSpell()
{


		AIEntity* ent = this;

		switch (gRandGen.randI(0,15)) { //was 20 but let it happen more often!
			case 1:
					if (ent->mINVamt[gSpellPoison.mId] > 0 
					&& ent->getEnergyLevel() >= gSpellPoison.mMana + 12) 
					{
						ent->setSpell(&gSpellPoison, ent->getAimObject());
						return;
					}
					break;
			case 2:
					if (ent->mINVamt[gSpellFireStone.mId] > 0 
						&& ent->getEnergyLevel() >= gSpellFireStone.mMana + 12) 
					{
							ent->setSpell(&gSpellFireStone, ent->getAimObject());
						return;
					}
					break;
			case 3: //custom have much higher chance!
			case 6:
					if (ent->mINVamt[gSpellCustomArea.mId] > 0 
						&& ent->getEnergyLevel() >= gSpellCustomArea.mMana + 12) 
					{
							ent->setSpell(&gSpellCustomArea, ent->getAimObject());
						return;
					}
					break;
/*
			default:
					ent->doAttack();
*/
		}

		//when a spell is called we must return else we do a melee attack!
		ent->doAttack();
		

}

// --------------------------------------------------------------------------------------------
void AIEntity::setSpell(AISpelltype* lSpell, ShapeBase* lTarget) {
	if (mCurSpell || mINVamt[lSpell->mId] <=0)
			return;
	//SPAM! Con::printf("%d cast spell %s", this->getId(),lSpell->mName);
	S32 lId = 0;
	if (lTarget)
		lId = lTarget->getId();


	if (dStrcmp(Con::executef(mDataBlock,"castSpell", getIdString(), Con::getIntArg(lSpell->mId), Con::getIntArg(lSpell->mMana), Con::getIntArg(lId)),"0")  != 0)
   {
      mCurSpell = lSpell;
      mElapsedTicks = 0;
		mINVamt[lSpell->mId]--;
	}

}

// --------------------------------------------------------------------------------------------
// Lightweight AI stuff
// --------------------------------------------------------------------------------------------
// Called on server only!! - if true we handled unterworld
bool AIEntity::isUnderWorld()
{

   if (getMoveState() != ModeStatic && getPosition().z < -300.f)
   {
      //set stuckbeam to false no matter if it was true !!!!!!! else bouncing ncp like muurani
      mStuckBeam = false;

      Point3F newPOS = mSpawnPoint;
      newPOS.z += 1.f;

      Con::errorf("ALERT! AIEntity under terrain  pos: %f,%f,%f new: %f,%f,%f"
         , getPosition().x, getPosition().y, getPosition().z
         , newPOS.x, newPOS.y, newPOS.z);

      jumpToPoint(newPOS); //helps only if spawnpoint is set !!!!
      Con::executef(this, "wasUnderTerrain", getIdString()); //callback on function AIEntity::wasUnderTerrain(%this) NOT datablock!!
      if (getSteering()->hasPath())
         getSteering()->resetPath();


      return true;

   }
   return false;
}

void AIEntity::handleServerHumanControlledTick()
{
   if (mDamageState == Enabled)
   {
      if (getAimObject() && getFSM()->getCurrentState() != AIState::Attack::Instance())
      {
         getFSM()->ChangeState(AIState::Attack::Instance());
      }
      mTickCounter--;
      if (mTickCounter <= 0)
      {
         mTickCounter = mThinkTicks;
         mStateMachine->Update();
      }
   }

}
//if false we should return in processtick
bool AIEntity::handleServerAIControlledTick(const Move* move)
{

   //2023-03-04 make sure neighbour list is empty to prevent list corruption, which cause a crash because of invalid pointers!
   mNeighbours.mList.clear();


   if (mMoveState == ModeStatic) {
      ShapeBase::processTick(move);
      return false;
   }

   if (mDamageState == Enabled)
   {

      //we are in pain so skip one think process
      if (mPainRunning && mActionAnimation.atEnd) {
         mPainRunning = false;
      }
      if (mAimCounterStrike) {
         mCounterStrikeResetCounter--;
         if (mCounterStrikeResetCounter <= 0)
            mAimCounterStrike = false;
      }

      if (mScanNeighbours)
         scanNeighbours();

      mTickCounter--;
      if (mTickCounter <= 0) {
/* 2023-02-28 bad idea ... no idea why it's working in auteria but for behaviours we need a fresh list! MOVED UP
         if (mScanNeighbours)
            scanNeighbours();
*/
         mStateMachine->Update();

         if (mTickCounter <= 0) //maybe overwritten by statemachine 
            mTickCounter = mThinkTicks;

         mCurUpdateTickCount = mTickCounter; //save how many ticks are done until next update

      
         if (mPauseSaveLoad && mStateMachine->getCurrentState() == AIState::Pause::Instance()) //save serverload!
         {
            mTickCounter = getMax(mThinkTicks, (U32)64); //3 sec (96) pause scan time !! 1.97.3 =>  thats a damn long time :( => 2  sec (64) !
         }
      }

      //1.97.3 do pause sleep also here not only when AI tick!!
      /*
         2023-03-03 this sucks! jitter and wrong animations
      */
      if (mPauseSaveLoad &&  mStateMachine->getCurrentState() == AIState::Pause::Instance() && mTickCounter % 32 != 0) //save serverload!
      {
         ShapeBase::processTick(move);
         return false;
      }
      
   }

   return true;
}

void AIEntity::processTick(const Move* move)
{

   bool lIsGhost=isGhost();

   //1.99 under terrian ?  -300 should be ok this problem will never let me go - if spawnpoint is bad this will loop forever
   if (!lIsGhost && isUnderWorld())
   {
      return;
   }

   if (!lIsGhost && mHumanControlled)
   {
      handleServerHumanControlledTick();

   } else if (!lIsGhost && !mHumanControlled ) { //2.0 && !mHumanControlled
            
      if (!handleServerAIControlledTick(move))
         return;
   } 

   



//#ifndef _entity_special_collision
//   Parent::processTick(move);
//#else

   // If we're not being controlled by a client, let the
   // AI sub-module get a chance at producing a move.
   Move aiMove;
   if (/*!move &&*/ isServerObject() && getAIMove(&aiMove))
      move = &aiMove;

   // Manage the control object and filter moves for the player
   Move pMove,cMove;
   if (mControlObject) {
      if (!move)
         mControlObject->processTick(0);
      else {
         pMove = NullMove;
         cMove = *move;


         mControlObject->processTick((mDamageState == Enabled)? &cMove: &NullMove);
         move = &pMove;
      }
   }


   ShapeBase::processTick(move);


   // Warp to catch up to server
   if (mDelta.warpTicks > 0) {
      mDelta.warpTicks--;

      // Set new pos.

      getTransform().getColumn(3,&mDelta.pos);

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

#ifdef TGE_MELEE  //1.99 added to aientity!
	  //mhhh updateAnimationTree(true);
      // this function was previously only called on client side
	  //XXTH dont call it every tick and check if the player is dead !!!
	   if (mDoImageRayCast) { //1.94 new flag to toggle raycast no matter if we play an blended ArmThread Animation 
         for (int i = 0; i < MaxMountedImages; i++)
   		   if (  mMountedImageList[i].dataBlock  ) 
                   UpdateImageRaycastDamage( TickSec, i );
	          

	   } else if (mDamageState == Enabled && mArmThreadPlayOnce) { // 1.8:50 prior:100* => removed at 1.91 => && Sim::getCurrentTime() > mLastImageRayCast+50 
         for (int i = 0; i < MaxMountedImages; i++)
   		   if (  mMountedImageList[i].dataBlock 
				 // mMountedImageList[i].state->fire  
                 && mShapeInstance->getPos(mArmAnimation.thread) > mShapeInstance->getDuration(mArmAnimation.thread) * 0.25f  //1.92: "/ 3" :: 1.93 *0.35 but now it works! :P
			  ) {
//DEBUG cool to see how long an anim is=>
//Con::errorf("thread playing: %d, dur:%f, pos:%f ",mArmThreadPlayOnce,mShapeInstance->getDuration(mArmAnimation.thread), mShapeInstance->getPos(mArmAnimation.thread));
                   UpdateImageRaycastDamage( TickSec, i );
	            }
          // REMOVED 1.9 mLastImageRayCast  =  Sim::getCurrentTime();
               
	  }  
#endif

	  }


      //XXTH Simplification:
	  //XXTH didRenderLastRender() http://www.garagegames.com/community/blogs/view/13856/
      //orig: if(isServerObject() || (didRenderLastRender() || getControllingClient()))
	  // BLEH! if(isServerObject() ||  getControllingClient())
	  // BLEH AGAIN ok need a hack for fly and swim animation!
	  if(isServerObject() || getControllingClient())
      {
#ifndef _entity_special_collision
        if (!mPhysicsRep) //2023-12-30
        {
           updateWorkingCollisionSet();
        }
        
#endif

        updateState();
		  updateMove(move);
        updatePos();
		
      }
   }
// #endif
}

// --------------------------------------------------------------------------------------------


//Update the movement
void AIEntity::updateMove(const Move *move)
{
#ifndef _entity_special_collision
   mDelta.move = *move;

   // Trigger images ... addedg etControllingClient()
   if (getControllingClient() &&  mDamageState == Enabled) {
      setImageTriggerState(0,move->trigger[0]);
      setImageTriggerState(1,move->trigger[1]);
//XXTH ADD MORE TRIGGERS=MOUNTPOINTS if needed values: 0..5 where 2 is reserved to jump
   } 

   // Update current orientation
   if (mDamageState == Enabled) {
      F32 prevZRot = mRot.z;
      mDelta.headVec = mHead;

      F32 p = move->pitch;
      if (p > M_PI_F) p -= M_2PI_F;
      mHead.x = mClampF(mHead.x + p,mDataBlock->minLookAngle,
                        mDataBlock->maxLookAngle);

      F32 y = move->yaw;
      if (y > M_PI_F) y -= M_2PI_F;

      GameConnection* con = getControllingClient();
      if (move->freeLook && ((isMounted() && getMountNode() == 0) || (con && !con->isFirstPerson())))
      {
         mHead.z = mClampF(mHead.z + y,
                           -mDataBlock->maxFreelookAngle,
                           mDataBlock->maxFreelookAngle);
      }
      else
      {
         mRot.z += y;
         // Rotate the head back to the front, center horizontal
         // as well if we're controlling another object.
         mHead.z *= 0.5f;
         if (mControlObject)
            mHead.x *= 0.5f;
      }

      // constrain the range of mRot.z
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
   }
   MatrixF zRot;
   zRot.set(EulerF(0.0f, 0.0f, mRot.z));

   // Desired move direction & speed
   VectorF moveVec;
   F32 moveSpeed;
  // AFX CODE BLOCK (anim-clip) and XXTH playercantmove! <<

   if (mState == MoveState  && mDamageState == Enabled  && !mPlayerCantMove && !isAnimationLocked())
   {
      zRot.getColumn(0,&moveVec);
      moveVec *= move->x;
      VectorF tv;
      zRot.getColumn(1,&tv);
      moveVec += tv * move->y;

      moveSpeed = 0.0f;

      // Clamp water movement
      if (move->y > 0.0f )  
      {
		 // => Mhh something wrong with my steering ?! y is often negativ .. and on wander it does move side
		  if  (!mHumanControlled)  //XXTH mTypeMask & AIObjectType = humbug ? its always AIObjectType!
			mMoveDirection = MoveForward; 
		  else {
			mMoveDirection = MoveForward; 
		    if (mFabs(move->x) > mFabs(move->y))
			    mMoveDirection = MoveSide;
		  }

         if( mWaterCoverage >= 0.9f )
            moveSpeed = getMax(mDataBlock->maxUnderwaterForwardSpeed * getSpeedMulti() * move->y,
                               mDataBlock->maxUnderwaterSideSpeed * getSpeedMulti()  * mFabs(move->x));
         else
            moveSpeed = getMax(mDataBlock->maxForwardSpeed * getSpeedMulti()  * move->y,
                               mDataBlock->maxSideSpeed * getSpeedMulti()  * mFabs(move->x));
      }
      else //XXTH LOL if (move->y < 0.0f ) 
      {
		 
		 // => Mhh something wrong with my steering ?! y is often negativ .. and on wander it does move side
		  if  (!mHumanControlled) //mTypeMask & AIObjectType) 
			mMoveDirection = MoveForward; 
		  else {
			mMoveDirection = MoveBackward;
		    if (mFabs(move->x) > mFabs(move->y))
			    mMoveDirection = MoveSide;
		  }
         if( mWaterCoverage >= 0.9f )
            moveSpeed = getMax(mDataBlock->maxUnderwaterBackwardSpeed * getSpeedMulti()  * mFabs(move->y),
                               mDataBlock->maxUnderwaterSideSpeed * getSpeedMulti()  * mFabs(move->x));
         else
            moveSpeed = getMax(mDataBlock->maxBackwardSpeed * getSpeedMulti()  * mFabs(move->y),
                               mDataBlock->maxSideSpeed * getSpeedMulti()  * mFabs(move->x));
      }


   }
   else
   {
      moveVec.set(0.0f, 0.0f, 0.0f);
      moveSpeed = 0.0f;
   }

   // Acceleration due to gravity
   //XXTH pre 4.0 VectorF acc(0.0f,0.0f,mGravity * mGravityMod * TickSec);
   VectorF acc(0.0f, 0.0f, mGravity *  TickSec);

/*
   if (getCanFly()) {
     acc.z=0.5f*TickSec;
   } 
*/

   // Determine ground contact normal. Only look for contacts if
   // we can move.
   VectorF contactNormal(0.f,0.f,0.f); 
   bool jumpSurface = false, runSurface = false;
   if (!isMounted())
      findContact(&runSurface,&jumpSurface,&contactNormal);
   if (jumpSurface)
      mJumpSurfaceNormal = contactNormal;

   //****** FLY *****
   mFlying = false; //set default
   if (!getInLiquid() && getCanFly() /*&& delta.move.z != 0.0f*/) {

        // get the head pitch and add it to the moveVec
	    moveVec.z = mDelta.move.z;
        VectorF flyVec(0,0,-mHead.x);
        moveVec += flyVec * move->y;
        

		VectorF pv;
		pv = moveVec;

		F32 pvl = pv.len();

		if (pvl)
			 pv *= moveSpeed / pvl;
		// VectorF runAcc = pv - acc;
        VectorF runAcc = pv - mVelocity;
        F32 runSpeed = runAcc.len();

		F32 maxAcc = (mDataBlock->runForce * getSpeedMulti() / mMass) * TickSec;
		if (runSpeed > maxAcc)
			 runAcc *= maxAcc / runSpeed;
		acc += runAcc;
		mContactTimer++;

		if (mContactTimer >= 30)  //XXTH mhh we need it more different for flyidle/flyglide/fly
				mFlying = true;
		else 
				mFlying = false;
   } 
   // ***** normal running
   else 
  if (runSurface) {  // Acceleration on run surface
      mContactTimer = 0;

      // Remove acc into contact surface (should only be gravity)
      // Clear out floating point acc errors, this will allow
      // the player to "rest" on the ground.
      if (!mPhysicsRep) //2023-12-30
      {
         F32 vd = -mDot(acc, contactNormal);
         if (vd > 0.0f) {
            VectorF dv = contactNormal * (vd + 0.002f);
            acc += dv;
            if (acc.len() < 0.0001f)
               acc.set(0.0f, 0.0f, 0.0f);
         }
      } //if (!mPhysicsRep)

      // Force a 0 move if there is no energy, and only drain
      // move energy if we're moving.
      VectorF pv;
      if (mEnergy >= mDataBlock->minRunEnergy) {
         if (moveSpeed)
            mEnergy -= mDataBlock->runEnergyDrain;
         pv = moveVec;
      }
      else
         pv.set(0.0f,0.0f,0.0f);

      // Adjust the players's requested dir. to be parallel
      // to the contact surface.
      F32 pvl = pv.len();
      if (pvl) {
         VectorF nn;
         mCross(pv,VectorF(0.0f,0.0f,1.0f),&nn);
         nn *= 1.0f / pvl;
         VectorF cv = contactNormal;
         cv -= nn * mDot(nn,cv);
         pv -= cv * mDot(pv,cv);
         pvl = pv.len();
      }

      // Convert to acceleration
      if (pvl)
         pv *= moveSpeed / pvl;
      VectorF runAcc = pv - (mVelocity + acc);
      F32 runSpeed = runAcc.len();

      // Clamp acceleratin, player also accelerates faster when
      // in his hard landing recover state.
      F32 maxAcc = (mDataBlock->runForce * getSpeedMulti()  / mMass) * TickSec;
      if (mState == RecoverState)
         maxAcc *= mDataBlock->recoverRunForceScale;
      if (runSpeed > maxAcc)
         runAcc *= maxAcc / runSpeed;
      acc += runAcc;

      // If we are running on the ground, then we're not jumping
/* XXTH MMO
      if (mDataBlock->isJumpAction(mActionAnimation.action))
         mActionAnimation.action = mmoPlayerData::NullAnimation;
*/
	  mJumping=false;

   }
  // ***** Swimming
  else if (mWaterCoverage > sSwimWaterCoverage && !isMounted()  //1.93 coverage changed from 0.2 to 0.35!
	       && ( getPlayerType()==0 ||  getCanSwim() ) 
		   )  //XXTH swim starts here .. 
  {
      //updated swimming:
        //make sure its not longer jumping:
	    mJumping = false;
		mSwimming = true;


		// get the head pitch and add it to the moveVec
		// This more accurate swim vector calc comes from Matt Fairfax
		MatrixF xRot /*, zRot*/;
		xRot.set(EulerF(mHead.x, 0.0f, 0.0f));
		zRot.set(EulerF(0.0f, 0.0f, mRot.z));
		MatrixF rot;
		rot.mul(zRot, xRot);
		rot.getColumn(0,&moveVec);


		moveVec *= move->x;
		VectorF tv;
		rot.getColumn(1,&tv);
        moveVec += tv * move->y;
		rot.getColumn(2,&tv);
		
      //XXTH 2022-01-22 prevent of dolphin!!
      //ORIG: moveVec += tv * move->z;
      // move.z is always ZERO ?! ok this helps but does not remove it completly
      if (mWaterCoverage < sDolphinWaterCoverage)
      {
         
         //Con::errorf("DOLPHIN ?!?!? move->z = %f", move->z);
         moveVec += tv * -1;
      }
      else {
         //Con::errorf("mWaterCoverage:%f move->z:%f", mWaterCoverage, move->z);
         moveVec += tv * move->z;
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
			pv.set(0.0f,0.0f,0.0f);

		F32 pvl = pv.len();

		// Convert to acceleration
		if (pvl)
			pv *= moveSpeed / pvl;
		VectorF runAcc = pv - mVelocity;
		F32 runSpeed = runAcc.len();

		// Clamp acceleration, player also accelerates faster when
		// in his hard landing recover state.
		F32 maxAcc = (mDataBlock->runForce / mMass) * TickSec;
		if (mState == RecoverState)
			maxAcc *= mDataBlock->recoverRunForceScale;
		if (runSpeed > maxAcc)
			runAcc *= maxAcc / runSpeed;
		acc += runAcc;

		mContactTimer++;
  } //XXTH end of swiming
   else
      mContactTimer++;

   // Acceleration from Jumping
   //XXTH changed from hardcoded "2" to JumpTriggerID = 2
   // AFX CODE BLOCK (anim-clip) <<
   if (move->trigger[JumpTriggerID] && !isMounted() && canJump() && !isAnimationLocked())
   {
      // Scale the jump impulse base on maxJumpSpeed
      F32 zSpeedScale = mVelocity.z;
      if (zSpeedScale <= mDataBlock->maxJumpSpeed )
      {
         zSpeedScale = (zSpeedScale <= mDataBlock->minJumpSpeed )? 1.0f:
            1.0f - (zSpeedScale - mDataBlock->minJumpSpeed )  /
            (mDataBlock->maxJumpSpeed   - mDataBlock->minJumpSpeed );

         // Desired jump direction
         VectorF pv = moveVec;
         F32 len = pv.len();
         if (len > 0.0f)
            pv *= 1.0f / len;

         // We want to scale the jump size by the player size, somewhat
         // in reduced ratio so a smaller player can jump higher in
         // proportion to his size, than a larger player.
         F32 scaleZ = (getScale().z * 0.25f) + 0.75f;

         // If we are facing into the surface jump up, otherwise
         // jump away from surface.
         F32 dot = mDot(pv,mJumpSurfaceNormal);
         F32 impulse = mDataBlock->jumpForce  * getSpeedMulti() / mMass;
         if (dot <= 0.0f)
            acc.z += mJumpSurfaceNormal.z * scaleZ * impulse * zSpeedScale;
         else
         {
            acc.x += pv.x * impulse * dot;
            acc.y += pv.y * impulse * dot;
            acc.z += mJumpSurfaceNormal.z * scaleZ * impulse * zSpeedScale;
         }

         mJumpDelay = mDataBlock->jumpDelay;
         mEnergy -= mDataBlock->jumpEnergyDrain;
         
		 mJumping = true;

         mJumpSurfaceLastContact = JumpSkipContactsMax;
      }
   }
   else
      if (jumpSurface) {
         if (mJumpDelay > 0)
            mJumpDelay--;
         mJumpSurfaceLastContact = 0;
      }
      else
         mJumpSurfaceLastContact++;


   // Add in force from physical zones...
   acc += (mAppliedForce / mMass) * TickSec;

   // Adjust velocity with all the move & gravity acceleration
   // TG: I forgot why doesn't the TickSec multiply happen here...
   mVelocity += acc;

   // apply horizontal air resistance

   F32 hvel = mSqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);

   if(hvel > mDataBlock->horizResistSpeed)
   {
      F32 speedCap = hvel;
      if(speedCap > mDataBlock->horizMaxSpeed)
         speedCap = mDataBlock->horizMaxSpeed;
      speedCap -= mDataBlock->horizResistFactor * TickSec * (speedCap - mDataBlock->horizResistSpeed);
      F32 scale = speedCap / hvel;
      mVelocity.x *= scale;
      mVelocity.y *= scale;
   }
   if(mVelocity.z > mDataBlock->upResistSpeed)
   {
      if(mVelocity.z > mDataBlock->upMaxSpeed)
         mVelocity.z = mDataBlock->upMaxSpeed;
      mVelocity.z -= mDataBlock->upResistFactor * TickSec * (mVelocity.z - mDataBlock->upResistSpeed);
   }

   // Container buoyancy & drag

/* ORIG:
   // Container buoyancy & drag
   if (mBuoyancy != 0.0f)
   {     // Applying buoyancy when standing still causing some jitters-
      if (mBuoyancy > 1.0f || !mVelocity.isZero() || !runSurface)
         mVelocity.z -= mBuoyancy * mGravity * mGravityMod * TickSec;
   }
   mVelocity   -= mVelocity * mDrag * TickSec;
*/

   if (mBuoyancy != 0.0f)
   {     // Applying buoyancy when standing still causing some jitters-
	   if (mBuoyancy > 1.0f || !mVelocity.isZero() || !runSurface) {
         //XXTH pre 4.0 mVelocity.z -= mBuoyancy * mGravity * mGravityMod * TickSec;
         mVelocity.z -= mBuoyancy * mGravity  * TickSec;
      }
   }
   mVelocity   -= mVelocity * mDrag * TickSec;

   // If we are not touching anything and have sufficient -z vel,
   // we are falling.
   if (runSurface)
      mFalling = false;
   else {
      VectorF vel;
      mWorldToObj.mulV(mVelocity,&vel);
      mFalling = vel.z < sFallingThreshold;
   }

   if (!isGhost()) {
      // Vehicle Dismount
	   
      //XXTH changed from hardcoded "2" to JumpTriggerID = 2
      if(!getInLiquid() && mWaterCoverage != 0.0f) {
         Con::executef( mDataBlock, "onEnterLiquid", getIdString(), Con::getFloatArg(mWaterCoverage), mLiquidType.c_str() ); 
         mInLiquid = true;
      }
      else if(getInLiquid() && mWaterCoverage == 0.0f) {
         Con::executef( mDataBlock, "onLeaveLiquid", getIdString(), mLiquidType.c_str() ); 
         mInLiquid = false;
      }
   }
   else {
      if(!getInLiquid() && mWaterCoverage >= 1.0f) {

         mInLiquid = true;
      }
      else if(getInLiquid() && mWaterCoverage < sSwimWaterCoverage) {
		  if(getVelocity().len() >= mDataBlock->exitSplashSoundVel && !isMounted()) 
		  {
			SFX->playOnce( mDataBlock->sound[PlayerData::ExitWater], &getTransform() );                     
		  }
         mInLiquid = false;
      }
   }
#else  //_entity_special_collision
   bool lIsGhost = isGhost();

   //for backward compatibility!
   if (mDamageState == Enabled) {
      setImageTriggerState(0,move->trigger[0]);
      setImageTriggerState(1,move->trigger[1]);
//XXTH ADD MORE TRIGGERS=MOUNTPOINTS if needed values: 0..5 where 2 is reserved to jump
   } 

   delta.move = *move;


   bool outside =false;
   for (int i=0;i<getNumCurrZones();i++)
      if (getCurrZone(i)==0)
      {
         outside = true;
         break;
      }

   // Update current orientation
   if (mDamageState == Enabled) {
      F32 prevZRot = mRot.z;
      delta.headVec = mHead;

      F32 p = move->pitch;
      if (p > M_PI_F)
         p -= M_2PI_F;
      mHead.x = mClampF(mHead.x + p,mDataBlock->minLookAngle,
                        mDataBlock->maxLookAngle);

      F32 y = move->yaw;
      if (y > M_PI_F)
         y -= M_2PI_F;

      GameConnection* con = getControllingClient();
      if (move->freeLook && ((isMounted() && getMountNode() == 0) || (con && !con->isFirstPerson())))
      {
         mHead.z = mClampF(mHead.z + y,
                           -mDataBlock->maxFreelookAngle,
                           mDataBlock->maxFreelookAngle);
      }
      else
      {
         mRot.z += y;
         mHead.z *= 0.5f;
         if (mControlObject)
            mHead.x *= 0.5f;
      }

      // constrain the range of mRot.z
      if (mFabs(mRot.z)>M_2PI*10.f)
         mRot.z = 0.0f;

      while (mRot.z < 0.0f)
         mRot.z += M_2PI_F;
      while (mRot.z > M_2PI_F)
         mRot.z -= M_2PI_F;

      delta.rot = mRot;
      delta.rotVec.x = delta.rotVec.y = 0.0f;
      delta.rotVec.z = prevZRot - mRot.z;
      if (delta.rotVec.z > M_PI_F)
         delta.rotVec.z -= M_2PI_F;
      else if (delta.rotVec.z < -M_PI_F)
         delta.rotVec.z += M_2PI_F;

      delta.head = mHead;
      delta.headVec -= mHead;
   }

   // Desired move direction & speed
   MatrixF zRot;
   zRot.set(EulerF(0.0f, 0.0f, mRot.z));
   // Desired move direction & speed
   VectorF moveVec;
   F32 moveSpeed;

   if (mState == MoveState  && mDamageState == Enabled  && !mPlayerCantMove && !isAnimationLocked())
   {
      zRot.getColumn(0,&moveVec);
      moveVec *= move->x;
      VectorF tv;
      zRot.getColumn(1,&tv);
      moveVec += tv * move->y;

      // Clamp water movement
      if (move->y > 0.0f )  
      {
		 // => Mhh something wrong with my steering ?! y is often negativ .. and on wander it does move side
		  if  (!mHumanControlled) //mTypeMask & AIObjectType) 
			mMoveDirection = MoveForward; 
		  else {
			mMoveDirection = MoveForward;
		    if (mFabs(move->x) > mFabs(move->y))
			    mMoveDirection = MoveSide;
		  }

         if( mWaterCoverage >= 0.9f )
            moveSpeed = getMax(mDataBlock->maxUnderwaterForwardSpeed * getSpeedMulti() * move->y,
                               mDataBlock->maxUnderwaterSideSpeed * getSpeedMulti()  * mFabs(move->x));
         else
            moveSpeed = getMax(mDataBlock->maxForwardSpeed * getSpeedMulti()  * move->y,
                               mDataBlock->maxSideSpeed * getSpeedMulti()  * mFabs(move->x));
      }
      else
      {
		 

		 // => Mhh something wrong with my steering ?! y is often negativ .. and on wander it does move side
		  if  (!mHumanControlled) //mTypeMask & AIObjectType) 
			mMoveDirection = MoveForward; 
		  else {
			mMoveDirection = MoveBackward;
		    if (mFabs(move->x) > mFabs(move->y))
			    mMoveDirection = MoveSide;
		  }
         if( mWaterCoverage >= 0.9f )
            moveSpeed = getMax(mDataBlock->maxUnderwaterBackwardSpeed * getSpeedMulti()  * mFabs(move->y),
                               mDataBlock->maxUnderwaterSideSpeed * getSpeedMulti()  * mFabs(move->x));
         else
            moveSpeed = getMax(mDataBlock->maxBackwardSpeed * getSpeedMulti()  * mFabs(move->y),
                               mDataBlock->maxSideSpeed * getSpeedMulti()  * mFabs(move->x));
      }


   }
   else
   {
      moveVec.set(0.0f, 0.0f, 0.0f);
      moveSpeed = 0.0f;
   }

   // Acceleration due to gravity
   VectorF acc(0.0f, 0.0f, mGravity * mGravityMod * TickSec);

   // Determine ground contact normal. Only look for contacts if
   // we can move.
   VectorF contactNormal;
   bool jumpSurface = mJumping, runSurface = false;

   jumpSurface = runSurface = mRunSurface;
   contactNormal = mContactNormal;

   if (jumpSurface)
      mJumpSurfaceNormal = contactNormal;

   mFlying = false;
   mSwimming = false;
    if ( /* XXTH 2010 delta.move.z != 0.0f && */ 
		  !isMounted() 
          && ( (getCanSwim() && mWaterCoverage >= sSwimWaterCoverage) 
           || (getCanFly() && !getInLiquid()) )
	   ) 
	{
      //updated swimming:
        //make sure its not longer jumping:
	    mJumping = false;
		mJumpDelay = 0;

		if (!getInLiquid())
				mFlying = true;
		else 
				mSwimming = true;

		// Acceleration on fly or swim
        // get the head pitch and add it to the moveVec
		moveVec.z = delta.move.z;
		VectorF flyVec(0.f,0.f,0.f);

        bool luseHeadX = mFlying || 
			            mHead.x > 0.f ||   //move down ok for now we check ground later
						mWaterCoverage > sSwimWaterCoverage + 0.05f  //flipper effect
						;
		if (mHead.x > 0.f && runSurface) //deny down move if on ground
			luseHeadX = false;

		if (  luseHeadX ) 
			flyVec.z=-mHead.x;

		moveVec += flyVec * move->y;
        

		VectorF pv;
		pv = moveVec;

		F32 pvl = pv.len();

		if (pvl)
			 pv *= moveSpeed / pvl;
		// VectorF runAcc = pv - acc;
        VectorF runAcc = pv - mVelocity;
        F32 runSpeed = runAcc.len();

		F32 maxAcc = (mDataBlock->runForce * getSpeedMulti() / mMass) * TickSec;
		if (runSpeed > maxAcc)
			 runAcc *= maxAcc / runSpeed;
		acc += runAcc;
		mContactTimer++;
		/* XXTH 2010
        if (mContactTimer > 100)
          mContactTimer = 100;
	    */


	} else   if (runSurface) { 
	  // Acceleration on run surface
      
      mContactTimer = 0;

      // Remove acc into contact surface (should only be gravity)
      // Clear out floating point acc errors, this will allow
      // the player to "rest" on the ground.
      F32 vd = -mDot(acc,contactNormal);
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
         mCross(pv,VectorF(0.0f, 0.0f, 1.0f),&nn);
         nn *= 1.0f / pvl;
         VectorF cv = contactNormal;
         cv -= nn * mDot(nn,cv);
         pv -= cv * mDot(pv,cv);
         pvl = pv.len();
      }

      // Convert to acceleration
      if (pvl)
         pv *= moveSpeed / pvl;
      VectorF runAcc = pv - (mVelocity + acc);
      F32 runSpeed = runAcc.len();

      // Clamp acceleratin, player also accelerates faster when
      // in his hard landing recover state.
      F32 maxAcc = (mDataBlock->runForce / mMass) * TickSec;
      if (mState == RecoverState)
         maxAcc *= mDataBlock->recoverRunForceScale;
      if (runSpeed > maxAcc)
         runAcc *= maxAcc / runSpeed;
      acc += runAcc;

      // If we are running on the ground, then we're not jumping
      if (mDataBlock->isJumpAction(mActionAnimation.action))
         mActionAnimation.action = PlayerData::NullAnimation;
   }   else   {
      mContactTimer++;
	  /* XXTH 2010
      if (mContactTimer > 100)
         mContactTimer = 100;
	  */
   } 
   if (move->trigger[2] && !isMounted() && canJump()  && !isAnimationLocked() && mWaterCoverage < 0.35f)
   {
      // Scale the jump impulse base on maxJumpSpeed
      F32 zSpeedScale = mVelocity.z;
      if (zSpeedScale <= mDataBlock->maxJumpSpeed)
      {
         zSpeedScale = (zSpeedScale <= mDataBlock->minJumpSpeed)? 1.0f :
            1.0f - (zSpeedScale - mDataBlock->minJumpSpeed) /
            (mDataBlock->maxJumpSpeed - mDataBlock->minJumpSpeed);

         // Desired jump direction
         VectorF pv = moveVec;
         F32 len = pv.len();
         if (len > 0.0f)
            pv *= 1.0f / len;

         // We want to scale the jump size by the player size, somewhat
         // in reduced ratio so a smaller player can jump higher in
         // proportion to his size, than a larger player.
         F32 scaleZ = (getScale().z * 0.25f) + 0.75f;

         // If we are facing into the surface jump up, otherwise
         // jump away from surface.
         F32 dot = mDot(pv,mJumpSurfaceNormal);
         F32 impulse = mDataBlock->jumpForce*1.5 / mMass;
         if (dot <= 0.0f)
            acc.z += mJumpSurfaceNormal.z * scaleZ * impulse * zSpeedScale;
         else
         {
            acc.x += pv.x * impulse * dot;
            acc.y += pv.y * impulse * dot;
            acc.z += mJumpSurfaceNormal.z * scaleZ * impulse * zSpeedScale;
         }

         mJumpDelay = 4; //mDataBlock->jumpDelay;
         mEnergy -= mDataBlock->jumpEnergyDrain;

		 /*
         setActionThread((mVelocity.len() < 0.5f) ?
            PlayerData::StandJumpAnim : PlayerData::JumpAnim, true, false, true);
		*/
		 mJumping = true;
      }
   }
   else
      if (jumpSurface) {
         if (mJumpDelay > 0)
            mJumpDelay--;
         mJumpSurfaceLastContact = 0;
      }
      else
         mJumpSurfaceLastContact++;
      
   // Add in force from physical zones...
   acc += (mAppliedForce / mMass) * TickSec;

   mVelocity += acc;

   // apply horizontal air resistance

   F32 hvel = mSqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);

   if(hvel > mDataBlock->horizResistSpeed)
   {
      F32 speedCap = hvel;
      if(speedCap > mDataBlock->horizMaxSpeed)
         speedCap = mDataBlock->horizMaxSpeed;
      speedCap -= mDataBlock->horizResistFactor * TickSec * (speedCap - mDataBlock->horizResistSpeed);
      F32 scale = speedCap / hvel;
      mVelocity.x *= scale;
      mVelocity.y *= scale;
   }
   if(mVelocity.z > mDataBlock->upResistSpeed)
   {
      if(mVelocity.z > mDataBlock->upMaxSpeed)
         mVelocity.z = mDataBlock->upMaxSpeed;
      mVelocity.z -= mDataBlock->upResistFactor * TickSec * (mVelocity.z - mDataBlock->upResistSpeed);
   }

   if(mVelocity.z < -70.0f)
      mVelocity.z = -70.0f;
   
   // Container buoyancy & drag
   if (mBuoyancy != 0.0f)
   {     // Applying buoyancy when standing still causing some jitters-
      if (mBuoyancy > 1.0f || !mVelocity.isZero() || !runSurface)
         mVelocity.z -= mBuoyancy * mGravity * mGravityMod * TickSec;
   }
   mVelocity   -= mVelocity * mDrag * TickSec;

   // If we are not touching anything and have sufficient -z vel,
   // we are falling.
   if (runSurface)
      mFalling = false;
   else {
      VectorF vel;
      mWorldToObj.mulV(mVelocity,&vel);
      mFalling = vel.z < sFallingThreshold;
   }

   if (!isGhost()) {

      if(!getInLiquid() && mWaterCoverage != 0.0f) {
         Con::executef(mDataBlock,4,"onEnterLiquid",scriptThis(), Con::getFloatArg(mWaterCoverage), Con::getIntArg(mLiquidType));
         mInLiquid = true;
		 if ( !mAllowWater  ) //run back or wander somewhere on land
			 mStateMachine->ChangeState(AIState::Wander::Instance());

      }
      else if(getInLiquid() && mWaterCoverage == 0.0f) {
         Con::executef(mDataBlock,3,"onLeaveLiquid",scriptThis(), Con::getIntArg(mLiquidType));
         mInLiquid = false;
		 if ( !mAllowLand  ) //run back or wander somewhere in water
			 mStateMachine->ChangeState(AIState::Wander::Instance());
      }
   }
   else {
      if(!getInLiquid() && mWaterCoverage >= 1.0f) {

         mInLiquid = true;
      }
      else if(getInLiquid() && mWaterCoverage < sSwimWaterCoverage) {
         if(getVelocity().len() >= mDataBlock->exitSplashSoundVel && !isMounted())
            alxPlay(mDataBlock->sound[PlayerData::ExitWater], &getTransform());
         mInLiquid = false;
      }
   }
#endif

}
#ifdef _entity_special_collision
// --------------------------------------------------------------------------------------------
//Interpolate movement
bool AIEntity::updatePos(const F32 travelTime)
{
   bool lIsGhost=isGhost();
   getTransform().getColumn(3,&delta.posVec);

   if (mVelocity.len() < .01f)
      mVelocity.set(0.0f,0.0f,0.0f);




   bool lastRunSurface = mRunSurface;
   
   Point3F start;
   Point3F initialPosition;
   getTransform().getColumn(3,&start);
   initialPosition = start;
   Point3F end = start + mVelocity * travelTime;
   Point3F distance = end - start;

   F32 velZ = mVelocity.z;
   F32 initialVelLen = mVelocity.len()*travelTime;
   Point3F initialVelocity = mVelocity;
   Point3F velNorm = mVelocity;
   velNorm.normalize();
   velNorm.neg();


   SceneObject *impactObject = NULL;
   VectorF impactNormal;
   F32     impactVelocity;
   
    mOnElevator = false;


   disableCollision();
   RayInfo rinfo;
   
   
    
/* suck! better wall avoid or run though!!! or not ?! 
   but when freemove swim/fly we should check it!

*/ 
   bool lCanGoUp = (mVelocity.z >0.f && (getCanFly() || (getCanSwim() && getInLiquid()) || mJumping));

   if (true) //lCanGoUp) 
   {
	   Point3F v = mVelocity;
	   v.z = 0.f;
	   v.normalize();
	   v*=.75f;

	   // normal collision but have problems with stairs
	   //	if (getContainer()->castRay(start+Point3F(0.0f,0.0f,1.0f), end+Point3F(0.0f,0.0f,1.0f)+v, sCollisionMoveMask, &rinfo))
	   //only terrain and interior
       //  if (getContainer()->castRay(start+Point3F(0.0f,0.0f,1.0f), end+Point3F(0.0f,0.0f,1.0f)+v, TerrainObjectType  | InteriorObjectType, &rinfo))

      if (getContainer()->castRay(start+Point3F(0.0f,0.0f,1.0f), end+Point3F(0.0f,0.0f,1.0f)+v, TerrainObjectType  | InteriorObjectType, &rinfo)
          || getContainer()->castRay(start+Point3F(0.0f,0.0f,1.5f), end+Point3F(0.0f,0.0f,1.5f)+v, sCollisionMoveMask, &rinfo))
	   {
		  F32 bd = -mDot(mVelocity, rinfo.normal);
		  if ( bd >= 0.0f || (rinfo.object ))
		  {
	         

			 F32 dot = mDot(Point3F(0.0f,0.0f,1.0f), rinfo.normal);//mFabs();
			 F32 s = 0.05f;

			 if (dot<0.55f)
			 {

					mVelocity=initialVelocity;
					
					mVelocity.zero();
					mVelocity.neg();
					mVelocity*=0.75f;
					
				s = 0.65f;
			 }

			 end = rinfo.point+velNorm*s;
			 end.z+=0.02f;
	         
			 if (dot < 0.55f)
				end.z = start.z;
	            

		  }
	      
	   }
   }
   //floor
   mRunSurface = false;
   
/*
   if (getCanSwim() && getInLiquid() && mVelocity.z >0.f)
	   lDoGroundCheck = false;
*/

   if (getContainer()->castRay(end+Point3F(0.0f,0.0f,1.0f), end-Point3F(0.0f,0.0f,0.65f), sCollisionMoveMask, &rinfo))
   {
      F32 bd = -mDot(mVelocity, rinfo.normal);
      if (bd >= 0.0 || (rinfo.object ) )
      {
       mOnElevator = rinfo.object->getTypeMask()&PathShapeObjectType;

       if (mFabs(initialVelocity.z) > mDataBlock->minImpactSpeed && !mMountPending) {

            if (!isGhost() && mHumanControlled) // !(mTypeMask & AIObjectType))
            {
               //onImpact(rinfo.object, rinfo.normal*mFabs(initialVelocity.z));
               impactObject = rinfo.object;
               impactNormal = rinfo.normal;
               impactVelocity = mFabs(initialVelocity.z);

            }
               



         }

         F32 rcos = mDataBlock->runSurfaceCos;
         if (!mHumanControlled) // mTypeMask & AIObjectType)
          rcos = mCos(mDegToRad(85.0f));

         mContactNormal = rinfo.normal;

         mRunSurface  = mContactNormal.z > rcos;

         //*jump = bestVd > mDataBlock->jumpSurfaceCos;
         
		 if (mOnElevator) {
	         
				end = rinfo.point + rinfo.normal * 0.02f ;
				mVelocity=initialVelocity;
				
         
				
		 } else if ( mRunSurface || initialVelocity.z < 0.0f  )
         {

            
            if (mJumpDelay == 0)
			{
				mJumping    = false; 
				mRunSurface = true;
				
			} else {
				mRunSurface = false;
				mJumpDelay--;
			}
			if (!lCanGoUp) {
				end = rinfo.point + rinfo.normal * 0.02f ;
				mVelocity=initialVelocity;//step
				mVelocity.z = 0.0f;
			}
		 
         }
         else
         {
            end = initialPosition;
			if (!lCanGoUp)
                mVelocity.set(0.0f,0.0f,0.0f);
         }
		 
      }

   }


   enableCollision();




   Point3F vec = end-initialPosition;
   F32 d = vec.len();
   if (  d>initialVelLen && mRunSurface==lastRunSurface)
   {
      vec.normalize();
  	  end = start+vec*initialVelLen;
   }

   //falling thru world
   if (mFabs(initialPosition.z-end.z)>1.0f)
      if (getContainer()->castRay(initialPosition, end, sCollisionMoveMask, &rinfo))
      {
         end = rinfo.point+rinfo.normal * 0.1f;
      }


	if (mOnElevator) {
		end += rinfo.object->getVelocity();
	} 



   // Set new position
   // If on the client, calc delta for backstepping
   if (isClientObject())
   {
      delta.pos = end;
      delta.posVec = delta.posVec - delta.pos;
      delta.dt = 1;
   }

   setPosition(end,mRot);
   if (isServerObject())
	setMaskBits(MoveMask);
   updateContainer();

//   if (mVelocity.z > velZ)
//      mVelocity.z = 0.0f;

   if (isServerObject() && mHumanControlled) //!(mTypeMask&AIObjectType))
   {
      gServerContainer.initRadiusSearch(end, 2.0f, TriggerObjectType);
      S32 id;
      while (id = gServerContainer.containerSearchNext())
      {
         Trigger* trigger = dynamic_cast<Trigger*>(Sim::findObject(id));
         if (trigger)
            trigger->potentialEnterObject(this);
         
      }
      
      
   }

   if (impactObject)
      onImpact(impactObject, impactNormal*impactVelocity);

   return true;

}
#endif
// --------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
U32 AIEntity::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = ShapeBase::packUpdate(con, mask, stream);

   //XXTH CharMask for Modifiers speed and cant move and action Overwrite
   if (stream->writeFlag(mask  & CharMask )) {
      stream->writeFlag(mPlayerCantMove);
      stream->writeInt(mSpeedModifier,6);
	  stream->writeFlag(overrideLookAnimation);
	  stream->writeInt(getPlayerRank(),16); //2023-10-20 more bits! 

     stream->writeInt(mPlayerType, 3); //3 bits are enought for 0..7 types :P
     con->packNetStringHandleU(stream, mGuildNameHandle);
     stream->write(mGravity);

     //Entity and Faction - for client quest dedection and other nifty things
     stream->writeInt(mEntity, 16); //16 bits MAX => 65535
     stream->writeInt(mFaction, 8); //8 bits MAX => 255

       //max values
     stream->write(getMaxDamage());
     stream->write(getMaxEnergy());
     stream->write(getMaxMana());

   }
   if (stream->writeFlag((mask & ImpactMask) && !(mask & InitialUpdateMask)))
      stream->writeInt(mImpactSound, PlayerData::ImpactBits);

//XXTH ARG since all animation are server controlled i've to send this always !!??? :(
   // || (mask & InitialUpdateMask) ) hat auch nix gebracht
   if (stream->writeFlag(mask & ActionMask  &&  mActionAnimation.action != -1 )) {
      stream->writeInt(mActionAnimation.action,PlayerData::ActionAnimBits);
      //removed stream->writeFlag(mActionAnimation.holdAtEnd);
      stream->writeFlag(mActionAnimation.atEnd);
      //removed stream->writeFlag(mActionAnimation.firstPerson);
      if (!mActionAnimation.atEnd) {
         // If somewhere in middle on initial update, must send position-
         F32  where = 0;
         if (mActionAnimation.thread)
            where = mShapeInstance->getPos(mActionAnimation.thread);
         if (stream->writeFlag((mask & InitialUpdateMask) != 0 && where > 0))
            stream->writeSignedFloat(where, 6);
      }
   }

   //XXTH 1.99 added for auteria aientity player 
#ifdef TGE_MELEE
   if (stream->writeFlag(mask & ActionMask &&
         mArmAnimation.action != PlayerData::NullAnimation &&
         (!(mask & InitialUpdateMask) ||
         mArmAnimation.action != mDataBlock->lookAction))) 
   {
      stream->writeInt(mArmAnimation.action,PlayerData::ActionAnimBits);

// XXTH MELEE  ===========>
      stream->writeFlag(mArmThreadPlayOnce);
#ifdef TGE_MELEE_PLUS
	  if (mArmThreadPlayOnce) {
		  stream->write(mShapeInstance->getTimeScale(mArmAnimation.thread));
	  }
#endif
   }
#endif
// <====== XXTH MELEE 



      stream->writeFlag(mFalling);
	  stream->writeFlag(mCanFly);
	  stream->writeFlag(mCanSwim);
	  stream->writeFlag(mFlying);
	  stream->writeFlag(mSwimming);
// mmmhh maybe	  stream->writeFlag(mOnElevator);


   // The rest of the data is part of the control object packet update.
   // If we're controlled by this client, we don't need to send it.
   // we only need to send it if this is the initial update - in that case,
   // the client won't know this is the control object yet.
   if(stream->writeFlag(getControllingClient() == con && !(mask & InitialUpdateMask)))
      return(retMask);

   if (stream->writeFlag(mask & MoveMask))
   {


      stream->writeInt(mState,NumStateBits);
      if (stream->writeFlag(mState == RecoverState))
         stream->writeInt(mRecoverTicks,PlayerData::RecoverDelayBits);

      Point3F pos;
      getTransform().getColumn(3,&pos);
      stream->writeCompressedPoint(pos);
      F32 len = mVelocity.len();
      if(stream->writeFlag(len > 0.02f))
      {
         Point3F outVel = mVelocity;
         outVel *= 1.0f/len;
         stream->writeNormalVector(outVel, 10);
         len *= 32.0f;  // 5 bits of fraction
         if(len > 8191)
            len = 8191;
         stream->writeInt((S32)len, 13);
      }
      stream->writeFloat(mRot.z / M_2PI_F, 7);
      stream->writeSignedFloat(mHead.x / mDataBlock->maxLookAngle, 6);
      stream->writeSignedFloat(mHead.z / mDataBlock->maxLookAngle, 6);
      mDelta.move.pack(stream);
      stream->writeFlag(!(mask & NoWarpMask));
   }
   // Ghost need energy to predict reliably
// #pragma message("Energy update really every update?!")
   
//XXTH 1.97 bad if maxenery id ZERO!   stream->writeFloat(getEnergyLevel() / getMaxEnergy(),EnergyLevelBits);
   stream->writeFloat(getEnergyValue(),EnergyLevelBits);
   
   return retMask;
}
//------------------------------------------------------------------------------------------------
void AIEntity::unpackUpdate(NetConnection *con, BitStream *stream)
{
   ShapeBase::unpackUpdate(con,stream);

   // CharMask for Modifiers speed and cant move
   if (stream->readFlag()) {
      mPlayerCantMove=stream->readFlag();
	  S32 tmpInt=stream->readInt(6);
      setSpeedModifier(tmpInt);
	  overrideLookAnimation = stream->readFlag();

      tmpInt=stream->readInt(16); //more bits! 
	   setPlayerRank(tmpInt);

      S32 tmpplayerType = stream->readInt(3);
      setPlayerType(tmpplayerType);
      mGuildNameHandle = con->unpackNetStringHandleU(stream);

      F32 gravity; 
      stream->read(&gravity);
      setGravity(gravity);

      //Entity and Faction- for client quest dedection and other nifty things
      mEntity = stream->readInt(16); //16 bits MAX => 65535
      mFaction = stream->readInt(8); //8 bits MAX => 0..255

      //max values
      F32 tmpfloat;
      stream->read(&tmpfloat);
      setMaxDamage(tmpfloat);

      stream->read(&tmpfloat);
      setMaxEnergy(tmpfloat);

      stream->read(&tmpfloat);
      setMaxMana(tmpfloat);

   }
   if (stream->readFlag())
      mImpactSound = stream->readInt(PlayerData::ImpactBits);

   // Server specified action animation
   if (stream->readFlag()) {
      U32 action = stream->readInt(PlayerData::ActionAnimBits);
      bool atEnd = stream->readFlag();

      F32   animPos = -1.0f;
      if (!atEnd && stream->readFlag())
         animPos = stream->readSignedFloat(6);

      if (isProperlyAdded()) {
         setActionThread(action);
         bool  inDeath = inDeathAnim();
         if (atEnd)
         {
            mShapeInstance->clearTransition(mActionAnimation.thread);
            mShapeInstance->setPos(mActionAnimation.thread,
                                   mActionAnimation.forward? 1: 0);
            if (inDeath)
               mDeath.lastPos = 1.0f;
         }
         else if (animPos > 0) {
            mShapeInstance->setPos(mActionAnimation.thread, animPos);
            if (inDeath)
               mDeath.lastPos = animPos;
         }

         // mMountPending suppresses tickDelay countdown so players will sit until
         // their mount, or another animation, comes through (or 13 seconds elapses).
         mMountPending = (S32) (inSittingAnim() ? sMountPendingTickWait : 0);
      }
      else {
         mActionAnimation.action = action;
         mActionAnimation.holdAtEnd = false; //hold;
         mActionAnimation.atEnd = atEnd;
         mActionAnimation.firstPerson = true; //or better true   fsp;
      }

   } //actionmask read

   //XXTH 1.99 added for auteria aientity player 
   // Server specified arm animation
// XXTH MELEE  ===========>
#ifdef TGE_MELEE
   if (stream->readFlag()) {
      U32 action = stream->readInt(PlayerData::ActionAnimBits);
      if (isProperlyAdded())
         setArmThread(action);
      else
         mArmAnimation.action = action;


		mArmThreadPlayOnce = stream->readFlag();
#ifdef _res_motionTrail //XXTH
		mDrawImageMotionTrail =  mArmThreadPlayOnce;
#endif
		F32 tmpscale =  1.0f;
#ifdef TGE_MELEE_PLUS
		if (mArmThreadPlayOnce) 
			stream->read(&tmpscale);
#endif
		if (mArmThreadPlayOnce)
			startPlayOnce(tmpscale);
		// hth twitch fix ->
		else
			stopPlayOnce();
   }

#endif
// <====== XXTH MELEE 



    //XXTH moved here ! 
     mFalling = stream->readFlag();
	  mCanFly  = stream->readFlag();
	  mCanSwim = stream->readFlag();
	  mFlying   = stream->readFlag();
	  mSwimming = stream->readFlag();
// mmmhh maybe	  mOnElevator=stream->readFlag();

   // controlled by the client?
   if(stream->readFlag())
      return;

   if (stream->readFlag()) {
      mPredictionCount = sMaxPredictionTicks;

//mhh maybe did not test it but ....	  mOnElevator = stream->readFlag();


      ActionState actionState = (ActionState)stream->readInt(NumStateBits);
      if (stream->readFlag()) {
         mRecoverTicks = stream->readInt(PlayerData::RecoverDelayBits);
         setState(actionState, mRecoverTicks);
      }
      else
         setState(actionState);

      Point3F pos,rot;
      stream->readCompressedPoint(&pos);
      F32 speed = mVelocity.len();
      if(stream->readFlag())
      {
         stream->readNormalVector(&mVelocity, 10);
         mVelocity *= stream->readInt(13) / 32.0f;
      }
      else
         mVelocity.set(0.0f,0.0f,0.0f);
         
      rot.y = rot.x = 0.0f;
      rot.z = stream->readFloat(7) * M_2PI_F;
      mHead.x = stream->readSignedFloat(6) * mDataBlock->maxLookAngle;
      mHead.z = stream->readSignedFloat(6) * mDataBlock->maxLookAngle;
      mDelta.move.unpack(stream);

      mDelta.head = mHead;
      mDelta.headVec.set(0.0f,0.0f,0.0f);

      if (stream->readFlag() && isProperlyAdded())
      {
         // Determin number of ticks to warp based on the average
         // of the client and server velocities.
         mDelta.warpOffset = pos - mDelta.pos;
		   if (mDelta.warpOffset.len() < 0.0001f) //XXTH
            mDelta.warpOffset=Point3F(0.f,0.f,0.f);
         F32 as = (speed + mVelocity.len()) * 0.5f * TickSec;
         F32 dt = (as > 0.00001f) ? mDelta.warpOffset.len() / as: sMaxWarpTicks;
         mDelta.warpTicks = (S32)((dt > sMinWarpTicks) ? getMax(mFloor(dt + 0.5f), 1.0f) : 0.0f);

         if (mDelta.warpTicks) {
            // Setup the warp to start on the next tick.
            if (mDelta.warpTicks > sMaxWarpTicks)
               mDelta.warpTicks = sMaxWarpTicks;
            mDelta.warpOffset /= mDelta.warpTicks;

            mDelta.rotOffset = rot - mDelta.rot;
            if(mDelta.rotOffset.z < - M_PI)
               mDelta.rotOffset.z += (F32)M_2PI;
            else if(mDelta.rotOffset.z > M_PI)
               mDelta.rotOffset.z -= (F32)M_2PI;
            mDelta.rotOffset /= mDelta.warpTicks;
         }
         else {
            // Going to skip the warp, server and client are real close.
            // Adjust the frame interpolation to move smoothly to the
            // new position within the current tick.
            Point3F cp = mDelta.pos + mDelta.posVec * mDelta.dt;
            if (mDelta.dt == 0.0f) {
               mDelta.posVec.set(0.0f,0.0f,0.0f);
               mDelta.rotVec.set(0.0f,0.0f,0.0f);
            }
            else {
               F32 dti = 1.0f / mDelta.dt;
               mDelta.posVec = (cp - pos) * dti;
               mDelta.rotVec.z = mRot.z - rot.z;

               if(mDelta.rotVec.z > M_PI_F)
                  mDelta.rotVec.z -= M_2PI_F;
               else if(mDelta.rotVec.z < -M_PI_F)
                  mDelta.rotVec.z += M_2PI_F;

               mDelta.rotVec.z *= dti;
            }
            mDelta.pos = pos;
            mDelta.rot = rot;
            setPosition(pos,rot);
         }
      }
      else {
         // Set the player to the server position
         mDelta.pos = pos;
         mDelta.rot = rot;
         mDelta.posVec.set(0.0f,0.0f,0.0f);
         mDelta.rotVec.set(0.0f,0.0f,0.0f);
         mDelta.warpTicks = 0;
         mDelta.dt = 0.0f;
         setPosition(pos,rot);
      }
   }
   F32 energy = stream->readFloat(EnergyLevelBits) * getMaxEnergy();
   setEnergyLevel(energy);
}


//----------------------------------------------------------------------------
// SETAttackAnimation
//----------------------------------------------------------------------------
bool AIEntity::setAttackAnimation(S32 animId, S32 overwriteWeaponHands) 
{
  if (!mAttackIgnoreMove && !mAttackStopMove && getVelocity().len() > 1.f)
       return false;

	S32 seq = 1;
	
	if (animId<1 || animId>4) return false;
	S32 hands;
	if (overwriteWeaponHands<0) {
		hands =  mWeaponHands;
	} else {
		hands =  overwriteWeaponHands;
	}
	switch (hands) {
		case 0: //unarmed
			switch (animId) {
				case 1:
					seq= action_unarmedright;
					if (seq<0)
						seq= action_attack1;
					break;
				case 2:
					seq= action_unarmedleft;
					if (seq<0) 
						seq= action_attack2;
					break;
				case 3:
					seq= action_kick2;
					if (seq<0) {
						seq= action_attack3;
					}
					break;
				case 4:
					seq= action_kick1;
					if (seq<0) {
						seq= action_attack4;

					}
					break;
			}
			break;
		case 1: //right
			switch (animId) {
				case 1:
					seq= action_1hslashright;
					break;
				case 2:
					seq= action_1hslash2right;
					break;
				case 3:
					seq= action_1hslash3right;
					break;
				case 4:
					seq= action_1hthrustright;
					break;
			}
			break;
		case 2: //left
			switch (animId) {
				case 1:
					seq= action_1hslashleft;
					break;
				case 2:
					seq= action_1hslash2left;
					break;
				case 3:
					seq= action_1hslash3left;
					break;
				case 4:
					seq= action_1hthrustleft;
					break;
			}
			break;
		case 3: //both hands
			  return setAttackAnimation(animId, gRandGen.randI(1,2));
			break;
		case 4: //2hand
			switch (animId) {
				case 1:
					seq= action_2hslash;
					break;
				case 2:
					seq= action_2hslash2;
					break;
				case 3:
					seq= action_2hslash3;
					break;
				case 4:
					seq= action_2hthrust;
					break;
			}
			break;
	}
	
	return setActionThread(seq,true, mAttackStopMove); //mAttackStopMove added 2022-01-22
}
//----------------------------------------------------------------------------
bool AIEntity::setPainAction( S32 id) 
{
    if (mDisablePainAction)
		return false;

	if (mPainRunning) 
		return false;

	S32 action;
	if (id == 1) 
		action = action_pain1;
	else
		action = action_pain2;

	if (action == -1) 
		return false;

	mPainRunning = true;
	return setActionThread(action,true,true);
}

//----------------------------------------------------------------------------
bool AIEntity::setShieldBlock() {

   if (!mShielded) return false;
   mShieldBlock = setActionThread(action_shieldblock,true,true);
   return mShieldBlock;

}


//----------------------------------------------------------------------------
bool AIEntity::setEmotion(S32 id) {
   S32 action = -1;

   
   switch (id) {
	case 1:	action =  action_agree; break;
	case 2:	action =  action_disagree; break;
	case 3:	action =  action_bow; break;
	case 4:	action =  action_wave; break;
	case 5:	action =  action_point; break;
	case 6:	action =  action_bowattack; break;
	case 7:	action =  action_dance; break;
	case 8:	action =  action_cartwheel; break;
	case 9: action =  action_handstand; break;
   }

	if (action >= 0) {
		mEmotion  = true;
		setActionThread(action);
		return true;
	}
	return false;
}




// --------------------------------------------------------------------------------------------
// AFX Animation cause trouble!
U32 AIEntity::playAnimation(U32 anim_id, F32 pos, F32 rate, F32 trans, bool hold, bool wait)
{
	if (anim_id == BAD_ANIM_ID)
    return 0;

	if (isServerObject()) {
			return setActionThread(anim_id,true,false);
	}
	return true;
}
// --------------------------------------------------------------------------------------------
S32 AIEntity::getActionbyName(const char* sequence)
{
   //XXTH why start from 1 ?! end wtf to count 
   // for (U32 i = 1; i < mDataBlock->actionCount; i++)
   for (U32 i = 0; i < mDataBlock->actionCount; i++)
   {
      PlayerData::ActionAnimation& anim = mDataBlock->actionList[i];
      if (!dStricmp(anim.name, sequence))
      {
         return i;
      }
   }

   return -1;
}
// --------------------------------------------------------------------------------------------
void AIEntity::setFollowLeader(bool doFollow, bool doSit)
{

   if (mLeaderObject && doFollow && getFSM()->getCurrentState() != AIState::Follow::Instance() ) 
   {
	   getFSM()->ChangeState(AIState::Follow::Instance());
       clearAim();
   } else if (!doFollow && getFSM()->getCurrentState() == AIState::Follow::Instance()) {
	   if (doSit) 
		   mSitting = true; 
	   getFSM()->ChangeState(AIState::Pause::Instance());
   }

   mFollowLeader = doFollow;

}



void AIEntity::reSkinMMO(String newSkinName)
{

   // Make our own copy of the materials list from the resource if necessary
   if (mShapeInstance->ownMaterialList() == false)
      mShapeInstance->cloneMaterialList();

   TSMaterialList* pMatList = mShapeInstance->getMaterialList();
   pMatList->setTextureLookupPath(mDataBlock->mShape.getPath().getPath());
      


   char  mmoNameBuf[255];
   char  specialSkinName[255];
   dStrcpy(specialSkinName, newSkinName, 255);

   //fetch orig dts file :
   char tmpbuf[255];
   const char* resourceName = mDataBlock->mShape.getPath().getFileName();
   dStrcpy(tmpbuf, resourceName, 255);
   char* tmptoken = dStrtok(tmpbuf, "_");
   dStrcpy(mmoNameBuf, tmptoken, 255);
   tmptoken = dStrtok(NULL, "_");
   if (tmptoken) {// return;  //its maybe a one name class!!
      dStrcat(mmoNameBuf, "_", 255);
      dStrcat(mmoNameBuf, tmptoken, 255);
   }
   else {
      //i guess its an usual multi skinned skined object
      dStrcpy(tmpbuf, resourceName, 255);
      tmptoken = dStrtok(tmpbuf, ".");
      dStrcpy(mmoNameBuf, tmptoken, 255);
   }




   // Cycle through the materials
   const Vector<String>& materialNames = pMatList->getMaterialNameList();
   for (S32 i = 0; i < materialNames.size(); i++)
   {
      // Try changing base
      const String& pName = materialNames[i];
      char* token1;
      char Buf1[256];
      char Buf2[256];
      char Buf3[256];
      dStrcpy(Buf1, pName, 255);

      if (dStrcmp(pName, "special") == 0 || dStrcmp(pName, "single") == 0) {
         token1 = (char*)pName.c_str();
      }
      else {
         if (strchr(Buf1, '.') != NULL) {
            token1 = dStrtok(Buf1, ".");
            token1 = dStrtok(NULL, ".");
         }
         else {
            token1 = dStrtok(Buf1, "_");
            token1 = dStrtok(NULL, "_");
            token1 = dStrtok(NULL, "_");
         }
      }
      if (!token1) {
         Con::errorf("INVALID MMO SKIN, TOKEN NOT FOUND!");
      }

      if (dStrcmp(token1, "single") == 0) { //single NOT USING "_" name...!!
         dStrcpy(Buf2, "single", 255);

      }
      else if (dStrcmp(token1, "special") == 0) { //special is only using the base texture!
         dStrcpy(Buf2, mmoNameBuf, 255);
         dStrcat(Buf2, "_special", 255);

      }
      else if (dStrcmp(token1, "head") == 0) { //head is only using the base texture! ?!

         dStrcpy(Buf2, mmoNameBuf, 255);
         dStrcat(Buf2, "_head", 255);
      }
      else if (dStrcmp(token1, "body") == 0) {

         dStrcpy(Buf3, "", 255);
         dStrncat(Buf3, &specialSkinName[3], 2);
         //mmoNameBuf
         if (dStrcmp(Buf3, "__") == 0) {
            dStrcpy(Buf2, mmoNameBuf, 255);
            dStrcat(Buf2, "_body", 255);
         }
         else {
            dStrcpy(Buf2, "tset_", 255);
            dStrncat(Buf2, &specialSkinName[3], 2);
            dStrcat(Buf2, "_body", 255);
         }
      }
      else if (dStrcmp(token1, "legs") == 0) {

         dStrcpy(Buf3, "", 255);
         dStrncat(Buf3, &specialSkinName[5], 2);
         //mmoNameBuf
         if (dStrcmp(Buf3, "__") == 0) {
            dStrcpy(Buf2, mmoNameBuf, 255);
            dStrcat(Buf2, "_legs", 255);
         }
         else {
            dStrcpy(Buf2, "tset_", 255);
            dStrncat(Buf2, &specialSkinName[5], 2);
            dStrcat(Buf2, "_legs", 255);
         }
      }
      else if (dStrcmp(token1, "hands") == 0) {

         dStrcpy(Buf3, "", 255);
         dStrncat(Buf3, &specialSkinName[7], 2);
         //mmoNameBuf
         if (dStrcmp(Buf3, "__") == 0) {
            dStrcpy(Buf2, mmoNameBuf, 255);
            dStrcat(Buf2, "_hands", 255);
         }
         else {
            dStrcpy(Buf2, "tset_", 255);
            dStrncat(Buf2, &specialSkinName[7], 2);
            dStrcat(Buf2, "_hands", 255);
         }
      }
      else if (dStrcmp(token1, "feet") == 0) {

         dStrcpy(Buf3, "", 255);
         dStrncat(Buf3, &specialSkinName[9], 2);
         //mmoNameBuf
         if (dStrcmp(Buf3, "__") == 0) {
            dStrcpy(Buf2, mmoNameBuf, 255);
            dStrcat(Buf2, "_feet", 255);
         }
         else {
            dStrcpy(Buf2, "tset_", 255);
            dStrncat(Buf2, &specialSkinName[9], 2);
            dStrcat(Buf2, "_feet", 255);
         }
      }
      else if (dStrcmp(token1, "arms") == 0) {

         dStrcpy(Buf3, "", 255);
         dStrncat(Buf3, &specialSkinName[11], 2);
         //mmoNameBuf
         if (dStrcmp(Buf3, "__") == 0) {
            dStrcpy(Buf2, mmoNameBuf, 255);
            dStrcat(Buf2, "_arms", 255);
         }
         else {
            dStrcpy(Buf2, "tset_", 255);
            dStrncat(Buf2, &specialSkinName[11], 2);
            dStrcat(Buf2, "_arms", 255);
         }
      }

   
      pMatList->renameMaterial(i, Buf2);


   }

   mShapeInstance->initMaterialList();

}


void AIEntity::reSkin()
{
   if (isGhost() && mShapeInstance && mSkinNameHandle.isValidString())
   {
      mShapeInstance->resetMaterialList();
      Vector<String> skins;
      String(mSkinNameHandle.getString()).split(";", skins);


      //XXTH
      if (skins.size()>0 && dStrcspn(skins[0], "MMO") == 0 && dStrlen(skins[0]) == 13) {
         reSkinMMO(skins[0]);
         return;
      }


      for (S32 i = 0; i < skins.size(); i++)
      {
         String oldSkin(mAppliedSkinName.c_str());
         String newSkin(skins[i]);

         // Check if the skin handle contains an explicit "old" base string. This
         // allows all models to support skinning, even if they don't follow the 
         // "base_xxx" material naming convention.
         S32 split = newSkin.find('=');    // "old=new" format skin?
         if (split != String::NPos)
         {
            oldSkin = newSkin.substr(0, split);
            newSkin = newSkin.erase(0, split + 1);
         }
         else {
            //XXTH exchange base material!
            if (mShapeInstance->getMaterialList()->getMaterialNameList().size() == 1)
            {
               String baseMat = mShapeInstance->getMaterialList()->getMaterialName(0);
               oldSkin = baseMat;
               if (newSkin.isEmpty())
                  newSkin = baseMat;
            }
         }


         // Apply skin to both 3rd person and 1st person shape instances
         mShapeInstance->reSkin(newSkin, oldSkin);
         for (S32 j = 0; j < ShapeBase::MaxMountedImages; j++)
         {
            if (mShapeFPInstance[j])
               mShapeFPInstance[j]->reSkin(newSkin, oldSkin);
         }

         mAppliedSkinName = newSkin;
      }
   }
}
// ---------------------- Guild ----------------------------
void AIEntity::setGuildName(const char* name)
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



void AIEntity::setPlayerType(S32 playertype) {
   if (playertype < 8 && mPlayerType != playertype) {  // if you want to raise it change the bit send its currently 2 bits!
      mPlayerType = playertype;
      if (isServerObject()) {
         setMaskBits(CharMask);
      }
   }
}

S32 AIEntity::getPlayerType() {
   return (mPlayerType);
}

DefineEngineMethod(AIEntity, setPlayerType, void, (S32 value), , "(S32 Playertype (0=Player,1=Bot,2=NPC,3=Harvest, 4=Farming, 5=Misc)") {
   if (object->isServerObject())
   {
      object->setPlayerType(value);
   }
}

DefineEngineMethod(AIEntity, getPlayerType, S32, (), , "(no params returns S32)") {
   return object->getPlayerType();
}
DefineEngineMethod(AIEntity, isBot, S32, (), , "(shape is a bot)") {
   return object->getPlayerType() == 1;
}

DefineEngineMethod(AIEntity, isNpc, S32, (), , "(shape is a NPC)") {
   return object->getPlayerType() == 2;
}

DefineEngineMethod(AIEntity, isHarvest, S32, (), , "(shape is a Harvest)") {
   return object->getPlayerType() == 3;
}

DefineEngineMethod(AIEntity, isFarming, S32, (), , "(shape is Farming)") {
   return object->getPlayerType() == 4;
}

// ---------------------- XXTH faction Group----------------------------
void AIEntity::setFaction(S32 lFaction) {
   if (lFaction < 256 && mFaction != lFaction) {  // if you want to raise it change the bits
      mFaction = lFaction;
      if (isServerObject()) {
         setMaskBits(CharMask);
      }
   }
}

S32 AIEntity::getFaction() {
   return (mFaction);
}

DefineEngineMethod(AIEntity, setFaction, void, (S32 value), , "(S32 Faction (0..65535)") {
   if (object->isServerObject())
   {
      object->setFaction(value);
   }
}

DefineEngineMethod(AIEntity, getFaction, S32, (), , "(no params returns S32)") {
   return object->getFaction();
}



void AIEntity::setMaxDamage(F32 maxDamage) {
   if (mMaxDamage == maxDamage)
      return;
   mMaxDamage = maxDamage;
   updateDamageLevel();
   if (!isGhost())
      setMaskBits(CharMask);

}


F32 AIEntity::getMaxDamage() {
   return(mMaxDamage);
}

void AIEntity::setMaxEnergy(F32 maxEnergy) {
   if (mMaxEnergy == maxEnergy)
      return;

   mMaxEnergy = maxEnergy;
   if (!isGhost())
      setMaskBits(CharMask);

}

F32 AIEntity::getMaxEnergy() {
   return(mMaxEnergy);
}

void AIEntity::setMaxMana(F32 maxMana) {
   if (mMaxMana == maxMana)
      return;
   mMaxMana = maxMana;
   if (mMana > mMaxMana) setMana(mMaxMana);
   if (!isGhost())
      setMaskBits(CharMask);
}

F32 AIEntity::getMaxMana() {
   return(mMaxMana);
}

void AIEntity::setMana(F32 mana) {
   if (mMana == mana)
      return;
   if (mana > mMaxMana)
      mana = mMaxMana;
   mMana = mana;

   if (!isGhost())
      setMaskBits(DamageMask);

}

F32 AIEntity::getMana() {
   return(mMana);
}

F32 AIEntity::getManaValue() {
   return(mMana / mMaxMana);
}


DefineEngineMethod(AIEntity, setMaxDamage, void, (F32 value), , "(F32 Max Damage)") {
   if (object->isServerObject())
   {
      object->setMaxDamage(value);
      object->setMaskBits(object->DamageMask); //XXTH das isses ? 
   }
}

DefineEngineMethod(AIEntity, getMaxDamage, F32, (), , "(no params returns F32)") {
   return object->getMaxDamage();
}

DefineEngineMethod(AIEntity, setMaxEnergy, void, (F32 value), , "(F32 MaxEnergy)") {
   if (object->isServerObject())
   {
      object->setMaxEnergy(value);
   }
}

DefineEngineMethod(AIEntity, getMaxEnergy, F32, (), , "(no params returns F32)") {
   return object->getMaxEnergy();
}

DefineEngineMethod(AIEntity, setMaxMana, void, (F32 value), , "(F32 MaxMana)") {
   if (object->isServerObject())
   {
      object->setMaxMana(value);
   }
}

DefineEngineMethod(AIEntity, getMaxMana, F32, (), , "(returns F32)") {
   return object->getMaxMana();
}

DefineEngineMethod(AIEntity, setMana, void, (F32 value), , "(F32 Mana)") {
   if (object->isServerObject())
   {
      object->setMana(value);
   }
}

DefineEngineMethod(AIEntity, incMana, bool, (F32 value), , "(F32 Mana)") {
   if (object->isServerObject())
   {
      F32 curmana = object->getMana();
      F32 addmana = value;
      if (curmana == object->getMaxMana())
         return false;
      object->setMana(curmana + addmana);
      return true;
   }
   else
      return false;
}

DefineEngineMethod(AIEntity, decMana, bool, (F32 value), , "(F32 Mana)") {
   if (object->isServerObject())
   {
      F32 curmana = object->getMana();
      F32 usemana = value;
      if (usemana > curmana)
         return false;

      object->setMana(curmana - usemana);
      return true;
   }
   return false;
}


DefineEngineMethod(AIEntity, getMana, F32, (), , "(returns F32)") {
   return object->getMana();
}
//----------------------------------------------------------------------------------------
void AIEntity::setPlayerRank(S32 rank) {
   if (rank != mPlayerRank) {
      mPlayerRank = rank;
      if (isServerObject())
            setMaskBits(CharMask );  
   }
}

S32 AIEntity::getPlayerRank() {
   return(mPlayerRank);
}

DefineEngineMethod(AIEntity, setPlayerRank, void, (S32 value), , "(S32 Rank)") {
   if (object->isServerObject())
   {
      object->setPlayerRank(value);
   }
}

DefineEngineMethod(AIEntity, getPlayerRank, S32, (), , "(no params returns S32)") {
   return object->getPlayerRank();
}
//----------------------------------------------------------------------------------------
void AIEntity::setBattlePoints(S32 points) {
   if (points != mBattlePoints) {
      mBattlePoints = points;
   }
}

S32 AIEntity::getBattlePoints() {
   return(mBattlePoints);
}

DefineEngineMethod(AIEntity, setBattlePoints, void, (S32 value), , "(S32 BattlePoints)") {
   if (object->isServerObject())
   {
      object->setBattlePoints(value);
   }
}

DefineEngineMethod(AIEntity, getBattlePoints, S32, (), , "(no params returns S32)") {
   return object->getBattlePoints();
}

//----------------------------------------------------------------------------------------------------------
// Gravity
//----------------------------------------------------------------------------------------------------------
F32 AIEntity::getGravity()
{
   return mGravity;
}

void AIEntity::setGravity(F32 gravity)
{
   //1.93
   mGravity = floatPrec(gravity);
   if (!isGhost())
      setMaskBits(CharMask);
}

//--------------------------------------------------------------------------
//XXTH gravity
DefineEngineMethod(AIEntity, getGravity, F32, (), , "")
{
   return object->getGravity();
}

DefineEngineMethod(AIEntity, setGravity, void, (F32 value), , "(float gravity)")
{
   object->setGravity(value);
}


//----------------------------------------------------------------------------------------------------------
// AI SWIM
//----------------------------------------------------------------------------------------------------------

DefineEngineMethod(AIEntity, setCanSwim, void, (bool value), , "(bool CanSwim)") {
   if (object->isServerObject())
   {
      object->setCanSwim(value);
   }
}

//----------------------------------------------------------------------------------------------------------
// AI FLY
//----------------------------------------------------------------------------------------------------------
DefineEngineMethod(AIEntity, setCanFly, void, (bool value), , "(bool CanFly)") {
   if (object->isServerObject())
   {
      
      object->setCanFly(value);
      if (value)
         object->setGravity(0.f);
      else
         object->setGravity(-20.f);

   }
}

//----------------------------------------------------------------------------------------------------------
// Stop player from moving in some cases (freeze, trade, ...)
//----------------------------------------------------------------------------------------------------------
void AIEntity::setPlayerCantMove(bool CantMove) {
   if (mPlayerCantMove != CantMove) {
      mPlayerCantMove = CantMove;
      if (isServerObject()) setMaskBits(CharMask);
   }
}

bool AIEntity::getPlayerCantMove() {
   return mPlayerCantMove;
}

DefineEngineMethod(AIEntity, setPlayerCantMove, void, (bool value), , "(bool CantMove)") {
   if (object->isServerObject())
   {
      object->setPlayerCantMove(value);
   }
}

DefineEngineMethod(AIEntity, getPlayerCantMove, bool, (), , "(return bool CantMove)") {
   if (object->isServerObject())
   {
      return object->getPlayerCantMove();
   }
   return 0;
}


//----------------------------------------------------------------------------------------------------------
// XXTH Console Point in water
//----------------------------------------------------------------------------------------------------------
/*
ConsoleMethod( Player, getPointinWater, bool, 3, 3, "(Point3F scale)")
{
  if (object->isServerObject())
  {
         VectorF point(0.f,0.f,0.f);
         dSscanf(argv[2], "%g %g %g", &point.x, &point.y, &point.z);
         return object->pointInWater(point);
  }
}
*/

//----------------------------------------------------------------------------------------------------------
// XXTH SpeedModifier
//----------------------------------------------------------------------------------------------------------
void AIEntity::setSpeedModifier(S32 value) {
   if (value != mSpeedModifier && value >= 0 && value < 64) {
      mSpeedModifier = value;
      if (isServerObject())
         setMaskBits(CharMask);
   }
}


DefineEngineMethod(AIEntity, setSpeedModifier, void, (S32 value), ,
   "(S32 speedModifier default=15 is datablock speed (0..63)"
   )
{
   if (object->isServerObject())
   {
      object->setSpeedModifier(value);
   }
}

DefineEngineMethod(AIEntity, getSpeedModifier, S32, (), , "(return S32 speedModifier)") {
   if (object->isServerObject())
   {
      return object->getSpeedModifier();
   }
   return 0;
}


// --------------------------------------------------------------------------------------------
// Console Functions
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setPainAction, bool, (S32 attackAnimation), ,
   "(int AttackAnimation 1..2, 0=random, -1=only when random 0..3==1)")
{
   S32 lAction = attackAnimation;
   if (lAction == -1) {
      if (gRandGen.randI(0,3) == 1)
      lAction = gRandGen.randI(1,2);
     else
       return false;
   } else if (lAction == 0)
      lAction = gRandGen.randI(1,2);

   return object->setPainAction(lAction);
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setShieldBlock, bool, (), , "Set Shieldblock animation and flag")
{
   return object->setShieldBlock();
}

//2.10
DefineEngineMethod( AIEntity, getShieldBlock, bool, (), , "get Shieldblock is active")
{
   return object->getShieldBlock();
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setEmotion, bool, (S32 emotion), , "Emotions:1 agree,2 disagree,3 bow,4 wave,5 point,6 bowattack, 7 dance, 8 cartweel, 9 handstand")
{
   return object->setEmotion(emotion);

}
// --------------------------------------------------------------------------------------------

DefineEngineMethod(AIEntity, setAttackAnimation, bool, (S32 attackAnimation), , "(int AttackAnimation 1..4)")
{
   return object->setAttackAnimation(attackAnimation);
}

// --------------------------------------------------------------------------------------------

DefineEngineMethod( AIEntity, setTargetPosition, void, (Point3F goal), , "(Point3F goal)"
              "Tells the AI to move to the location provided.")
{
   object->setTargetPosition(goal);
}
// --------------------------------------------------------------------------------------------

DefineEngineMethod( AIEntity, getTargetPosition, const char *, (), , "()"
              "Returns the point the AI is set to move to.")
{
   Point3F movePoint = object->getTargetPosition();

   char *returnBuffer = Con::getReturnBuffer( 256 );
   dSprintf( returnBuffer, 256, "%g %g %g", movePoint.x, movePoint.y, movePoint.z );

   return returnBuffer;
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setStatic, void, (bool value), , "()"
              "true/false. Set to staticobject to prevent physics, will be canceled by move/stop/pause.")
{
   object->setStatic(value);
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setMoveState, void, (S32 moveState), , "(int MoveState)"
              "Tells the AI to move to the location provided.")
{
   object->setMoveState(moveState);
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, getMoveState, S32, (), , "()"
              "Returns the movestate the AI.")
{
   return object->getIntMoveState();
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setAimObject, void, (ShapeBase * targetObject, bool force, bool counterstrike), (true,true),
   "( ShapeBase obj, boolean force, boolean counterstrike  )"
   "Sets the bot's target object.")
{  
   
   if( targetObject ) {
      if (object->setAimObject(targetObject, force, counterstrike)) {
      }
   }
   else
      object->setAimObject( 0 );
}
// --------------------------------------------------------------------------------------------

DefineEngineMethod( AIEntity, clearAim, void, (), , "()"
              "Stop aiming at aimobject.")
{
   object->clearAim();
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, getAimObject, S32, (), , "()"
              "Gets the object the AI is targeting.")
{
   GameBase* obj = object->getAimObject();

   static S32 result;
   if ( obj )
	    result = obj->getId();
   else
		result = -1;

   return result;
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod(AIEntity, getAimObjectInLos, bool, (), , " get the current aim is in Los")
{
   return object->getAimObjectInLos();
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setAimObject2, void, (ShapeBase* targetObject), ,
   "(  ShapeBase* targetObject )"
   "Sets the bot's target object 2.")
{  
   // Find the target
   if(targetObject) {
	   if (object->setAimObject2( targetObject )) {
	   }
   }
   else
      object->setAimObject2( 0 );
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setLeaderObject, void, (ShapeBase* targetObject), ,
   "(ShapeBase* targetObject) "
   "Sets the bot's/pet's leader object.")
{  
   // Find the target
   
   if(targetObject) {
	   if (object->setLeaderObject( targetObject )) {
	   }
   }
   else
      object->setLeaderObject( 0 );
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, getLeaderObject, S32, (), , "()"
              "Gets the leader object of the AI.")
{
   GameBase* obj = object->getLeaderObject();

   static S32 result;
   if ( obj )
	    result = obj->getId();
   else
		result = -1;

   return result;
}

//Compatibility to old aiMonster class: same as getLeaderObject 
DefineEngineMethod( AIEntity, getMasterObject, S32, (), , "()"
              "Gets the leader object of the AI. same as getLeaderObject only for compatibility")
{
   GameBase* obj = object->getLeaderObject();

   static S32 result;
   if ( obj )
	    result = obj->getId();
   else
		result = -1;

   return result;
}


// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setObstancleGroup, void, (SimGroup* grpObj), , "( SimGroup obj  )"
              "Sets the bot's obstancles group.")
{  
   // Find the group
   if(grpObj) {
	   if (object->setObstancleGroup( grpObj )) {
	   }
   }
   else
      object->setObstancleGroup( 0 );
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setPath, void, (SimPath::Path* PathObj, bool looping, bool reverse, bool forceWander),(-1,false,false) ,
            "( Path obj , [bool looping], [bool reverse], [bool forceWander] )"
            "Sets the bot's path. looping is optional if not set isLooping is take from path."
			  "Reverse is also optional, forceWander start the path  "
              "note: markers msToNext let stop and wait until continue!"
            )
{  
	
   if( PathObj ) {
	   if (object->getSteering()->setPathObject( PathObj, looping, reverse )) {
            if (forceWander)
               object->getFSM()->ChangeState(AIState::Wander::Instance());
	   }
   }
   else
      object->getSteering()->clearPathObject(); //path reset
}

// --------------------------------------------------------------------------------------------
#ifdef AI_USEPATH
DefineEngineMethod(AIEntity, setiAIPath, void, (iAIPath* PathObj, bool forceWander), (false),
   "( iAIPath obj  )"
   "Sets the bot's path. Note the iAIPath is not CLEARED so you can use it again!!!!!" 
)
{
   if (PathObj) {
      if (object->getSteering()->setIAIPath(PathObj))
      {
         if (forceWander)
            object->getFSM()->ChangeState(AIState::Wander::Instance());
      }
   }
   else
      object->getSteering()->clearPathObject(); //path reset
}
#endif

// --------------------------------------------------------------------------------------------
#ifdef AI_USEPATH
DefineEngineMethod( AIEntity, setPathDestination, bool, (Point3F destination), , "( Point3F destination  )"
              "Sets the bot's AI* path.")
{


   Con::errorf("USE wanderTO, else it won't move! !!!! ");
   
   return object->getSteering()->setPathDestination(destination);
   
}
#endif
// --------------------------------------------------------------------------------------------

DefineEngineMethod( AIEntity, setBehaviourTypeMask, void, (S32 behaviourMask), ,
   "(int TypeMask)"
   "Set the typemask for behavior.")
{
   object->setBehaviourTypeMask(behaviourMask);
}

DefineEngineMethod( AIEntity, BehaviourOn, S32, (S32 behaviourMask), ,
   "(int TypeMask)"
   "Returns boolean if behavior is on.")
{
   return object->BehaviourOn((SteeringBehaviourTypes)behaviourMask);
}

DefineEngineMethod( AIEntity, setBehaviourOn, bool, (S32 behaviourMask), ,
   "(int TypeMask)"
   "Set a behavior type on.")
{
   return object->setBehaviourOn((SteeringBehaviourTypes)behaviourMask);
}

DefineEngineMethod( AIEntity, setBehaviourOff, bool, (S32 behaviourMask), ,
   "(int TypeMask)"
   "Set a behavior type on.")
{
   return object->setBehaviourOff((SteeringBehaviourTypes)behaviourMask);
}


// --------------------------------------------------------------------------------------------

DefineEngineMethod( AIEntity, setFormationOffset, void, (VectorF offset), , "(VectorF offset)")
{
   object->setFormationOffset(offset);
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setMaxRotationAngle, void, (S32 value), , "(int Angle 1..180)"
              "Set the typemask for behavior.")
{
    object->setMaxRotateAngle(value);
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setMoveTolerance, void, (F32 value), (0.5f), "( float tolerance default 0.50)"
              "Sets the move toleranze (onReachDestination) for an AI object.")
{
   object->setMoveTolerance( value );
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod(AIEntity, getMoveTolerance, F32, (), , "( get float move tolerance )" )
{
   return object->getMoveTolerance();
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setAttackDistanceAdd, void, (F32 value), (0.75), "( float tolerance default 0.75)"
              "Sets the attack distance summand.")
{
   object->setAttackDistanceAdd( value );
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setSeparationRadius, void, (F32 value), (2.f), "( float radius default 2.f)"
              "Separation radius for flock and separation")
{
   object->setSeparationRadius( value );
}


// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setScanRadius, void, (F32 value), (20.f), "( float radius default 20.f )"
              "Sets the scan Radius for an AI object.")
{
   object->setScanRadius( value );
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod(AIEntity, setIgnoreRadius, void, (F32 value), (50.f), "( float radius default 50.f )"
   "Sets the ignore Radius for an AI object, if aimobject is to far away from my spawnpoint it runs back to spawnpoint.")
{
   object->mIgnoreRange = value;
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, stopPath, void, (), , ""
              "Stop a Steering Path.")
{
	object->getSteering()->stopPath();
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, continuePath, void, (), , ""
              "Stop a Steering Path.")
{
	object->getSteering()->continuePath();
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, getClosestWayPoint, const char *, (), , "()"
              "Returns the point the AI is set to move to.")
{
   Point3F lLoc = object->getPosition();

   Point3F* movePoint = object->getSteering()->getClosestWayPoint(lLoc);
   if (!movePoint) 
	 return "";

   char *returnBuffer = Con::getReturnBuffer( 256 );
   dSprintf( returnBuffer, 256, "%g %g %g", movePoint->x, movePoint->y, movePoint->z );

   return returnBuffer;
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setPersonality, void, (S32 value), , "(0=Aggressive, 1=Sheepish, 2=None)"
              "Set the typemask for behavior.")
{
	object->setPersonality((AIEntity::Personality)value);
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, getPersonality, S32, (), , "Returns the id of the current Personality.")
{
   return (S32) object->getPersonality();
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setThinkTime, void, (S32 value), (1000), "(Value in ms, default 1000)"
              "Set thinktime for active personality")
{
	U32 lT = mFloor( (F32)value * 0.032f );
	object->setThinkTicks(lT);
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setAttackdelay, void, (S32 value), (2048), "(Value in ms, default 2048, only possitble in 256ms steps => 2000 would be 1792 => mfloor(2000 / 256)*256))"
              "Set attackticks for active personality")
{
	//attack ticks are not the same as think ticks!
	// when attack state it think every 8 ticks (256ms) instead of thinktime
	U32 lT = mCeil( (F32)value / 256.f );
	object->setAttackTicks(lT);
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setAttackDistance, void, (F32 value), ,
   "(F32 Attackdistance)"
   "set the attack distance")
{
	object->setConstantAttackDistance(value);
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod(AIEntity, getAttackDistance, F32, (), ,
   "()"
   "get the attack distance")
{
   F32 result = object->getConstantAttackDistance();
   if (object->getTargetDistance() > result)
      result = object->getTargetDistance();

   return result;
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, playRandAttack, void, (), , "(play a random attack animation)")
{
	object->setAttackAnimation((gRandGen.randI(1,4))); 
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setSpawnPoint, void, (Point3F value), , "(Point3F spawnpoint)"
              "Tells the AI where the spawnpoint is.")
{
   object->setSpawnPoint( value);
}

DefineEngineMethod( AIEntity, getSpawnPoint, const char*, (), , "Returns the spawnpoint.")
{
   char *returnBuffer = Con::getReturnBuffer(256);
   Point3F lSP = object->getSpawnPoint();

   dSprintf(returnBuffer,256,"%g %g %g", lSP.x, lSP.y, lSP.z);
   return returnBuffer;
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, getCurrentState, S32, (), ,
   "Returns the id of the current State.")
{
	if (object->getFSM() && object->getFSM()->getCurrentState())
	{

		return object->getFSM()->getCurrentState()->getStateId();

	} else {
		return 0;
	}
}

DefineEngineMethod( AIEntity, getCurrentStateName, const char*, (), ,
   "Returns the name of the current State.")
{
	if (object->getFSM() &&  object->getFSM()->getCurrentState())
	{

		char *returnBuffer = Con::getReturnBuffer(256);
		dSprintf(returnBuffer,256,"%s", AIStateNames[object->getFSM()->getCurrentState()->getStateId()]);
		return returnBuffer;
	} else {
		return "none";
	}
}


// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, resetActionThread, void, (), ,
   "Reset overwriteanim for looping animations")
{
  object->setOverwriteAnim(false);
}
// --------------------------------------------------------------------------------------------
//THIS!!! 
DefineEngineMethod( AIEntity, setActionThread, bool, (const char * sequenceName, bool overwrite, bool blocked), (true, false),
   "(string sequenceName, bool setOverwriteAnimation (until animation ends) - default true, bool move blocked - default false")
{
   return object->setActionThread(sequenceName,overwrite,blocked);
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, WanderTo, void, (Point3F wanderDestination), , "(Point3F wanderDestination)"
              "Tells the AI to wander somewhere, this also set the spawnpoint new! Makes a callback onWanderDone")
{
   object->forceWander(wanderDestination);
   
}

DefineEngineMethod(AIEntity, wander, void, (), , "()"
   "Time to take a wander...")
{
   object->getFSM()->ChangeState(AIState::Wander::Instance());
}


DefineEngineMethod(AIEntity, Pause, void, (), , "()"
   "Time to take a break...")
{
   object->getFSM()->ChangeState(AIState::Pause::Instance());
}


// --------------------------------------------------------------------------------------------
DefineEngineMethod(AIEntity, WanderToObject, bool, (GameBase* lObject), , "(Object)"
   "Tells the AI to wander somewhere, this also set the spawnpoint new! Makes a callback onWanderDone")
{
   if (!lObject)
      return false;
   Point3F wanderDestination = AIMath::getPositionCloseToObject(object, lObject);
   if (wanderDestination.isZero())
      return false;

   object->forceWander(wanderDestination);

   return true;

}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, followLeader, void, (bool doFollow, bool sitdown), (false), "(bool doFollow, [bool sitdown])"
              "Tells the AI to follow it's leader, sitdown only works if doFollow = false")
{
	object->setFollowLeader(doFollow, sitdown);
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, clearPathWanderUsage, void, (), , "Reset the current wander path, empty it and set state to pause")
{
	if (object->mPathUsage == AIEntity::PathWander) 
	{
		object->getSteering()->clearPathObject(); //path reset
		object->getFSM()->ChangeState(AIState::Pause::Instance()); // force pause
		object->mPathUsage = AIEntity::PathNone;
	}
  
}
// --------------------------------------------------------------------------------------------
DefineEngineMethod(AIEntity,jump,void,(), ,
			  "Jump :D")
{
	object->jump();
}

//----------------------------------------------------------------------------
DefineEngineMethod( AIEntity, setDoPauseSleepAnimaion, void, (bool value), , "bool value")
{
   object->setDoPauseSleepAnimaion(value);
}
// --------------------------------------------------------------------------------------------
// compat to script source added in V2.20
// ConsoleMethod(AIEntity, setTargetPlayerType, void, 3, 3, "U8 playertype")
DefineEngineMethod(AIEntity, setTargetPlayerType, void, (S32 playertype), , "(S32 playertype)"
   "Set the playertype same as TargetPlayerType=")

{
   if (object->isServerObject())
      object->mTargetPlayerType = playertype;
}

// --------------------------------------------------------------------------------------------
DefineEngineMethod( AIEntity, getTargetPlayerType, S32, (), , "()" "Gets Monster target Playertype.")
{
   return object->mTargetPlayerType;
}

//----------------------------------------------------------------------------
//DefineEngineMethod(ShapeBase, getSkinName, const char*, (), ,
//DefineEngineMethod( ShapeBase, setSkinName, void, ( const char* name ),,

DefineEngineMethod(AIEntity, setGuildName, void, (const char* name), , "")
{
   object->setGuildName(name);
}

DefineEngineMethod(AIEntity, getGuildName, const char*, (), ,"")
{
   return object->getGuildName();
}


//----------------------------------------------------------------------------
DefineEngineMethod(AIEntity, setPauseSaveLoad, void, (bool value), , " save load on pause - was default in auteria")
{
   return object->setPauseSaveLoad(value);
}

DefineEngineMethod(AIEntity, getVelocity, Point3F, (), , "get the current Velocity of an AIEntity")
{
   return object->getVelocity();
}

DefineEngineMethod(AIEntity, getVariDestination, Point3F, (Point3F location, F32 addRad), (0.f), "get a variable (random) position from a given pos")
{
   return object->getVariDestination(location, addRad);
}

