//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// aiEntity by t.huehn (XXTH) (c) huehn-software 2009
// converted to T3D 3.10.1 2020
//-----------------------------------------------------------------------------

#ifndef _AIENTITY_H_
#define _AIENTITY_H_


#ifndef _AIGLOBALS_H_
#include "aiGlobals.h"
#endif

#ifndef _PLAYER_H_
#include "T3D/player.h"
#endif

#ifndef _AISTEERING_H_
#include "aiSteering.h"
#endif

#ifndef _AIPATH_H_
#include "aiPath.h"
#endif

#ifndef _STATEMACHINE_H
#include "fsm/StateMachine.h"
#endif

// raycast collision but fall throught terrain problem :(
// raycast does not collide with some trees / shrunksboat and other objects :(
// good but not perfect :(  - 
// #define _entity_special_collision

#ifdef _entity_special_collision
#pragma message("!!!!WARNING!!!! _entity_special_collision enabled!!!")
#endif

/* 
Better Spell System !!! 
=======================
  Script driven:
    1.) Spell Definition (datablock ? ) 
	    (int)  id
		(enum) type: RangeAttack, CloseAttack, Vitality
		(int)  mana
		(bool) stopMovement
        (int)  Distance (optional for RangeAttack)
		(int)  CoolDown
		(bool) BlockMelee

	2.) Spellbook
	    ->addSpell
	    




!!!!!!!!!!!!!!!!!!!!!! I keep it for the Moment !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  Spell Slots
    -> Spell (definition)
	-> Inventory amount or unlimited = -1 ? 

    global conditions: Inventory Amount, Mana(Energy), Blocked by current action

	Different Spell Types:
	  * Attack
	     - Range Attack  (Hunt::enter)
		     (condition: chance, maxDistance)
		 - Close Attack  (Attack::execute)

	  * Vitality (global state - CheckVitality::execute)
	     - Heal         (condition: damage value )
		 - Fillup Mana  (condition: energy value )
		 - Anti Poison  (condition: poisoned)
		 - Anti X       (condition: X)
		 - Pushup X     (condition: chance, X not active, in combat )


   Class AISpellSlot 
   {
      AuteriaSpellType
	  Amount 
	  set/get SpellType
	  set/get Amount
   }

   

   Class Buffs
   {
      BuffType
	  started
	  ends
   }

   Tricky - add condition for vitality.
   ====================================
   We have 3 types:
     * Fillup    - low value
	 * Buffed  - buf set
	 * Pushup    - in combat and buf not set 


   Since we dont need this system at the moment we need a intermedia solution
   to keep the states clean and enhanceable for Sutaratus. 
   So I'll put the Spell Code into AIEntiy and Away from the state machine!

*/


class AISpelltype {
public:
	AISpelltype() { mId = -1; mCooldownTicks = 0; mMana = 0; mDistance = 0; mBlockMelee = false;mStopMovement=false;}
	inline AISpelltype(const char* lName, const S32 lId, const S32 lCooldownTicks, 
		                                          const S32 lMana, const F32 lDistance, const bool lBlockMelee,
												  const bool lStopMovement )
	{
		mId = lId; 
		mCooldownTicks = lCooldownTicks; 
		mMana = lMana; 
		mDistance = lDistance;
		mName = lName;
		mBlockMelee = lBlockMelee;
		mStopMovement = lStopMovement;
	}
	S32 mId;
	S32 mCooldownTicks;
	S32 mMana;
	F32 mDistance;
	bool mBlockMelee;
	bool mStopMovement;
	const char* mName;
};


static const S32 AUTERIA_AI_SPELLCOUNT = 9;

static AISpelltype gSpellIceBall    = AISpelltype("IceBall",	0,32,10,40, true, true); 
static AISpelltype gSpellPoison     = AISpelltype("Poison",		1,64,15,0, true, true);   
static AISpelltype gSpellFireBall   = AISpelltype("FireBall",	2,128,25,40 , true, true);   
static AISpelltype gSpellFireStone  = AISpelltype("FireStone",	3,64,35,0, true, true);
static AISpelltype gSpellHealth	    = AISpelltype("MagicHealthStone",4,48,12, 30, false, true); //30m distance for remote health
static AISpelltype gSpellBeer       = AISpelltype("Beer",		5,8,0,0 , false , false);
static AISpelltype gSpellAntidote   = AISpelltype("Antidode",	6,8,0,0 , false , false);

static AISpelltype gSpellCustomRange  = AISpelltype("CustomRange", 	 7,128,25,40 , true, true);   
static AISpelltype gSpellCustomArea   = AISpelltype("CustomAreaDamage",8,64,35,0, true, true);






//-------------------------------------------
class AIEntity : public Player {
   friend class AIEntityRTS;
	typedef Player Parent;
protected:
   /// Bit masks for different types of events
   // running out of bits parents eat em all 
   enum MaskBits {
      CharMask = Parent::NextFreeMask,
      NextFreeMask = Parent::NextFreeMask << 1
   };

public:
/* must be set on create!! 
   U32 mCollisionMoveMask = (TerrainObjectType |
      TerrainLikeObjectType | InteriorLikeObjectType |
      WaterObjectType | // PlayerObjectType     | 
      StaticShapeObjectType | VehicleObjectType |
      PhysicalZoneObjectType);



   U32 mServerCollisionContactMask = (mCollisionMoveMask |
      (ItemObjectType |
         TriggerObjectType |
         CorpseObjectType
         ));

   U32 mClientCollisionContactMask = mCollisionMoveMask | PhysicalZoneObjectType;
*/

   PlayerData* mDataBlock;    

   F32 mGravity; //4.0 

	bool mInLiquid;
	bool mPlayerCantMove;
	S32 mSpeedModifier;
   bool mCanSwim;
   bool mCanFly;
   S32 mBattlePoints;

    U32 mlastAnimationAction; 
	//we have that in T3D U8           anim_clip_flags;

   enum { 
      ANIM_OVERRIDDEN     = BIT(0),
      BLOCK_USER_CONTROL  = BIT(1),
   };

   NetStringHandle mGuildNameHandle;

   void setGuildName(const char* name);
   inline const char* getGuildName()
   {
      return mGuildNameHandle.getString();
   }

   S32  getBattlePoints();
   void setBattlePoints(S32 points);
   void setSpeedModifier(S32 lVal);
   S32  getSpeedModifier() { return mSpeedModifier; };
   void setPlayerCantMove(bool lVal);
   bool getPlayerCantMove();

   F32 getMaxForwardVelocity() { return (mDataBlock != NULL ? mDataBlock->maxForwardSpeed * getSpeedMulti() : 0); }
   F32  getSpeedMulti() { return (mSpeedModifier + 1.0f) / 16.0f; };

   bool getCanSwim() { return mCanSwim; };
   void setCanSwim(bool CanSwim) { mCanSwim = CanSwim; };

   bool getCanFly() { return mCanFly; };
   void setCanFly(bool CanFly) { mCanFly = CanFly; };

   /// XXHT Sets the level of gravity for this object
   /// @param   gravity   Level of gravity to assign to this object
   void setGravity(F32 gravity);

   /// XXTH Returns the amount of gravity for this object
   F32 getGravity();

   F32 floatPrec(F32 value) { return (mFloor(value * 1000) / 1000); };

   S32 getActionbyName(const char* sequence);
   virtual void setPlayerType(S32 playertype);
   virtual S32 getPlayerType();

   virtual void setFaction(S32 lFaction);
   virtual S32 getFaction();

   virtual S32 getPlayerRank();
   virtual void setPlayerRank(S32 rank);

	enum AIMoveState {
		ModeStop,
		ModeMove,
		ModeLook,
		ModeStatic
	};

	enum Personality {
		Aggressive,   //0
		Sheepish,     //1
		Dontcare,     //2
		PersoNone     //3 initial setting
	};



	enum PathUsageType {
		PathNone
		,PathWander
		,PathHunt
	};


protected:
   enum MoveDirection {
      MoveForward,
	  MoveBackward,
	  MoveSide
   };
   
   MoveDirection mMoveDirection;

	    //idle 
   S32  action_sit, 
        action_idle,
		action_sleep, //1.97.3
		action_swimidle,
        action_cready,
        action_cready1h,
        action_cready2h,
        action_death,
        action_death2,
        action_death3,
        action_death4,
        action_death5,
        action_mountsit,
		action_flyidle,


		//motion
        action_run,
        action_walk,
		action_back,
        action_1hrun,
        action_1hwalk,
        action_2hrun,
        action_2hwalk,
        action_jump,
        action_fall,
        action_land,
        action_side,
        action_swim,
        action_swim_left,
        action_swim_right,
        action_swim_back,
        action_fly,
		action_flyglide,

		//misc actions
        action_shieldblock,
        action_spellprepare,
        action_spellcast,
        action_spellcast2,

		//emotions
        action_agree,
        action_disagree,
        action_bow,
        action_wave,
        action_point,
		action_bowattack,
        action_dance,
		action_cartwheel,
		action_handstand,

		//attack actions
        action_unarmedright,
        action_unarmedleft,
        action_kick1,
        action_kick2,
        action_attack1,
        action_attack2,
        action_attack3,
        action_attack4,
        action_1hslashright,
        action_1hslash2right,
        action_1hslash3right,
        action_1hthrustright,
        action_1hslashleft,
        action_1hslash2left,
        action_1hslash3left,
        action_1hthrustleft,
        action_2hslash,
        action_2hslash2,
        action_2hslash3,
        action_2hthrust,

        action_pain1,
        action_pain2
        ;

   bool mJumping;
   S32  mJumpCnt;


   void initAnimations();

   void GuessActionAnimation();
   MatrixF * getGroundConform(const Point3F& pos,  MatrixF orgMat);


   virtual void reSkin();
   void reSkinMMO(String newSkinName);



private:
	  Point3F mTargetPosition;
	  AIMoveState mMoveState;

	  //an instance of the state machine class
	  StateMachine<AIEntity>*  mStateMachine;

     //save some load on pause ... sucks in a action game
     bool mPauseSaveLoad;

	  Personality mPersonality;

     S32  mPlayerRank;
     S32  mPlayerType;
     S32 mFaction;
     S32 mEntity;
     F32 mMana;
     F32 mMaxMana;
     F32 mMaxDamage;
     F32 mMaxEnergy;

	  S32  mWeaponHands;
	  bool mMoveBlocked;
      bool mOverwriteAnim;
	  bool  mShielded;
      bool	mShieldBlock;
      bool	mSitting;
      bool	mPainRunning;
	  bool  mDisablePainAction;
      bool	mFlying;
	  bool	mSwimming;
      bool	mCready;
      bool	mEmotion;
      bool  mBasicAnimation;
	  bool  mHumanControlled; //also added mIsAiControlled 

      bool mAttackStopMove; //2022-01-22 
      bool mAttackIgnoreMove; //2023-12-27

	  bool  mDoPauseSleepAnimaion;
	                            

	  F32 mConstantAttackDistance;
	  
      
	  Point3F mLastLocation;
	  S32 mStuckCounter;

      S32 mWalkMoveModifier; // set the current walk speed (default 15)
      S32 mRunMoveModifier; //  set the current run speed  (default 25)

      bool mAimCounterStrike;		     // Force a AimObject no matter if its out of radius!
      U32 mLastAimTime;


      SimObjectPtr<ShapeBase> mAimObject; // Object to point at, overrides location
      SimObjectPtr<ShapeBase> mAimObject2; // for interposeTest ! 
      SimObjectPtr<ShapeBase> mLeaderObject; // for offsetPursuit and Pets

      bool mAimInLOS; //2023-02-28

	  // Steering
	  SimObjectPtr<SimGroup> mObstGroup; // for Hide ! 

	  AISteering* mSteering;
	  U32 mBehaviourTypeMask;
	  F32 mMaxRotateRadiants;
	  F32 mMoveTolerance;
	  F32 mAttackDistanceAdd;
	  F32 mPanicDistance;
	  F32 mTargetDistance;

	  VectorF mFormationOffset;

	  bool mScanNeighbours;
	  SimpleQueryList	mNeighbours;
	  
	  void scanNeighbours();

	  F32 mSeparationRadius; //should be set about double of boundingbox radius; default is 2.ff //1.97.3 set to 1.5!
	  F32 mAligmentRadius; //should be same as separarion if both is used
	  F32 mCohesionRadius;
	  //multipliers. These can be adjusted to effect strength of the  
	  //appropriate behavior. Useful to get flocking the way you require
      //for example.
		F32        m_dWeightSeparation;
		F32        m_dWeightCohesion;
		F32        m_dWeightAlignment;
		F32        m_dWeightWander;
		F32        m_dWeightWallAvoidance;
		F32        m_dWeightSeek;
		F32        m_dWeightFlee;
		F32        m_dWeightArrive;
		F32        m_dWeightPursuit;
		F32        m_dWeightOffsetPursuit;
		F32        m_dWeightInterpose;
		F32        m_dWeightHide;
		F32        m_dWeightEvade;
		F32        m_dWeightFollowPath;


		//-------------------------------------------
#ifdef _entity_special_collision

		bool mRunSurface;
		VectorF mContactNormal;
		bool mOnElevator;
		SceneObject* mEleObj;
#endif
	    S32 mTickCounter;
		S32 mAttackTickCounter;
		U32 mThinkTicks;
		
		U32 mAttackTicks; 
		S32 mCounterStrikeResetCounter;
/* 1.97.3 wanted to added this but sucks with states!
		S32 mPauseTicks;
		S32 mMaxWanderTicks;
		S32 mWanderTicks; 
*/


		bool mFollowLeader;
		bool mFollowArriveNotify;

      void process_client_triggers(bool triggeredLeft, bool triggeredRight);


public:
		DECLARE_CONOBJECT( AIEntity );

	  AIEntity();
      ~AIEntity();
   
	  bool onAdd();
	  bool onNewDataBlock( GameBaseData *dptr, bool reload );

      virtual bool getAIMove( Move *move );
	  static void initPersistFields();


      virtual U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
      virtual void unpackUpdate(NetConnection *conn,           BitStream *stream);



	  bool checkShouldIgnore(ShapeBase* target);
	  bool isTouchable(SimObjectId TargetID, bool checkSheepish = false, bool usuallyIgnored = false);
	  F32 getDistanceFromSpawn(GameBase* a);
	  F32 getDistance2D(GameBase* a);
     F32 getDistance3D(GameBase* a);
     Point3F mSpawnPoint;
	  F32 mWanderRadius;
	  bool mIsWanderer; //1.99
	  S32 mTargetPlayerType ;
	  F32 mScanRadius; //used to see who's in range
      F32 mIgnoreRange;
      S32 mIgnoreBP;


	  bool mPoisoned;
	  F32  mAttack;
	  F32  mDefence;
	  F32  mMagic;
	  bool mPauseHeal;
	  F32  mPauseHealPercent;


	  // ~~~~ wall avoid hacks ~~~
	  F32  mWallSkipCounter;
	  VectorF mWallSavedForce;


	  //*** Sutaratus stats: ***
	  //Base
	  F32 mStrength;	//Staerke
	  F32 mAgility;		//Beweglichkeit
	  F32 mStamina;		//Ausdauer
	  F32 mIntellect;	//Intelligenz
	  F32 mSpirit;		//Willenskraft

	  //Secundary
	  F32 mAttackPower; //Angriffskraft
	  F32 mHaste;		//Tempo
	  F32 mCrit;		//Kritisch
	  F32 mHit;			//Trefferwertung
	  F32 mExpertise;	//Waffenkunde

	  F32 mArmour;		//Rüstung      - reduce damage calc to %
	  F32 mDodge;		//Ausweichen   - chance to doge calc to  %
	  F32 mParry;		//Parieren     - chance to parry calc to  %
	  F32 mBlock;		//Blocken	   - chance to block  calc to  %
	  F32 mResilience;  //Abhärtung    - reduce damage calc to % from players or their minions

	  F32 mSpellPower;  //Zaubermacht       - increase damage or heal of spells 
	  F32 mPenetration; //Zauberdurchschlag - reduce enemy resistance value (not percent)


	  //Resistance - reduce damge 
	  //F32 mResArcane
	  F32 mResFire;
	  F32 mResFrost;
	  F32 mResNature;
	  F32 mResShadow;


	  bool mStuckBeam;

	  PathUsageType mPathUsage; // AI wander a path instead of wander/pause states
	  Point3F mWanderPositonSet;
	  S32 mWanderTicksLeft; //on none path wander I add a ticktimeout (3 min) to force a wander stop!
	  void forceWander(Point3F lPos);

	  //dang hardcoded spells:
	  S32 mCurUpdateTickCount; //for advancing stuff like cooldown
	  AISpelltype* mCurSpell;
	  S32 mElapsedTicks;
      

	  // inventory amout of spells
      S32 mINVamt[AUTERIA_AI_SPELLCOUNT];

	  bool mAllowWater, mAllowLand, mSpawnInsideMissionArea;

	  void setSpell(AISpelltype* lSpell, ShapeBase* lTarget = NULL);
	  
	  void setWanderDestination(Point3F lTo = Point3F(0.f,0.f,0.f));

     Point3F getVariDestination(Point3F location, F32 addRad = 0.f);


     
	  
	  virtual void guessVitalitySpell();
	  virtual void guessRangeSpell();
	  virtual void guessMeleeSpell();

	  /* 1.97.3 wanted to added this but sucks with states!
	  S32 getPauseTicks()     { return mPauseTicks; }
	  S32 getWanderTicks()    { return mWanderTicks; }
	  S32 getWanderMaxTicks() { return mMaxWanderTicks; }
	  void setPauseTicks( S32 lValue )     { mPauseTicks = lValue; }
	  void setWanderTicks( S32 lValue )    { mWanderTicks = lValue; }
	  void setWanderMaxTicks( S32 lValue ) { mMaxWanderTicks = lValue; }
	  */


	  AISteering* getSteering() { return mSteering; }

      void setTargetPosition( const Point3F &location);
	  Point3F getTargetPosition( ) { return mTargetPosition; }


     void setMaxDamage(F32 maxDamage);
     F32  getMaxDamage();
     void setMaxEnergy(F32 maxEnergy);
     F32  getMaxEnergy();
     void setMaxMana(F32 maxMana);
     F32  getMaxMana();
     void setMana(F32 mana);
     F32  getMana();
     F32  getManaValue();

	  void enableRun() { setSpeedModifier(mRunMoveModifier); }
	  void enableWalk() { setSpeedModifier(mWalkMoveModifier); }

	  void setMoveState(AIMoveState newState);
	  void setMoveState(S32 newIntState) { setMoveState((AIMoveState) newIntState); }
	  AIMoveState getMoveState() { return mMoveState; }
	  S32 getIntMoveState() { return (S32)mMoveState; }

	  virtual bool setAimObject(ShapeBase*targetObject, bool lForce = false, bool lCounterStrike = false ); //return true if new aimobject set...
	  virtual void clearAim(  ) ;
	  virtual bool setAimObject2(ShapeBase*targetObject);
	  void setCounterStrike(bool lCounterStrike);
	  bool getInCounterStrike() { return mAimCounterStrike;  }

	  //2.10
	  bool getShieldBlock() { return mShieldBlock;  }


	  void setDoPauseSleepAnimaion(bool lValue) { mDoPauseSleepAnimaion = lValue; }

      // Utility Methods
      virtual void throwCallback( const char *name );

	  // Steering
	  VectorF CalculatePrioritized();
	  bool  AccumulateForce(VectorF &RunningTot, VectorF ForceToAdd);
	  F32 getMaxForce() { return this->getMaxForwardVelocity(); }
	  VectorF getTargetPos() {
			VectorF lTargetPos;
			if (mAimObject) lTargetPos = mAimObject->getPosition();
					   else	lTargetPos = mTargetPosition;
			return lTargetPos;
	  }
	  void doAttack();
	  bool validTargetAttack();
	  bool validTargetHunt();
	  bool inAttackDistance();
	  //1.97.3 added mIgnoreBP
      //void checkScanNeightbours() { mScanNeighbours = mIgnoreBP > 0 && ((mPersonality!=PersoNone) || (mPersonality!=Dontcare) || BehaviourOn(FlockBehaviourType) || BehaviourOn(SeparationBehaviourType) || BehaviourOn(AlignmentBehaviourType) || BehaviourOn(CohesionBehaviourType)); }
      // 2023-02-15 remove mIgnoreBP from the check
	  void checkScanNeightbours() {

        mScanNeighbours =
           (mPersonality != PersoNone) || (mPersonality != Dontcare)
           || BehaviourOn(FlockBehaviourType)
           || BehaviourOn(SeparationBehaviourType)
           || BehaviourOn(AlignmentBehaviourType)
           || BehaviourOn(CohesionBehaviourType)
           ;
     }
	  void setBehaviourTypeMask(U32 lMask) { 
		  mBehaviourTypeMask = lMask; 
		  checkScanNeightbours();
		  
	  }
	  bool BehaviourOn(SteeringBehaviourTypes lType) { return (mBehaviourTypeMask & lType) == lType; }
  	  bool setBehaviourOn(SteeringBehaviourTypes lType) { 
		  if (!BehaviourOn(lType))  {
			  mBehaviourTypeMask |= lType; 
			  checkScanNeightbours(); 
			  return true;
		  }
		  return false;
	  }
  	  bool setBehaviourOff(SteeringBehaviourTypes lType) { 
		  if (BehaviourOn(lType))  {
			  mBehaviourTypeMask ^= lType; 
			  checkScanNeightbours(); 
			  return true;
		  }
		  return false;
	  }



	  bool setObstancleGroup(SimGroup* lObstGroup);
	  void setFormationOffset(VectorF lOffset) { mFormationOffset = lOffset; }
	  VectorF getFormationOffset() const { return mFormationOffset; }

	  void setMaxRotateAngle(S32 lAngle);
	  void setMoveTolerance( const F32 lValue ) { mMoveTolerance = lValue; if (getSteering()) getSteering()->setMoveTolerance(mMoveTolerance); }
	  F32 getMoveTolerance() const { return mMoveTolerance; }

	  void setAttackDistanceAdd( const F32 lValue ) { mAttackDistanceAdd = lValue;  }
	  F32 getAttackDistanceAdd() const { return mAttackDistanceAdd; }

	  void setSeparationRadius ( const F32 lValue ) { mSeparationRadius = lValue;  }
	  F32 getSeparationRadius() const { return mSeparationRadius; }

	  F32 getPanicDistance() const { return mPanicDistance; }
	  F32 getIgnoreRange() const { return mIgnoreRange; }

	  void setTargetDistance( const F32 lD ) { mTargetDistance = lD; }
	  F32 getTargetDistance() const { return mTargetDistance; }

     ShapeBase* getAimObject() const  { return mAimObject; }
     ShapeBase* getAimObject2() const  { return mAimObject2; }
	  bool setLeaderObject(ShapeBase *targetObject );
     ShapeBase* getLeaderObject() const  { return mLeaderObject; }

     bool  getAimObjectInLos() { return mAimInLOS; }

	  void setScanRadius( const F32 lRad ) { mScanRadius = lRad;  }
	  F32 getScanRadius() const { return mScanRadius; }

	  void setStatic(bool activate);

	  void setPersonality(Personality lPersonality);
      Personality getPersonality() {return mPersonality;}


	  void setThinkTicks(U32 lT) {mThinkTicks = lT; }
	  U32 getThinkTicks() {return mThinkTicks; }

	  void setAttackTicks(U32 lT) {mAttackTicks = lT; }
	  U32 getAttackTicks() {return mAttackTicks; }
	  S32  getAttackDelay() { return mAttackTicks * 256; }

	  void setConstantAttackDistance(F32 lT) {mConstantAttackDistance = lT; }
	  F32 getConstantAttackDistance() {return mConstantAttackDistance; }

  	  void setTickCounter(S32 lT) {mTickCounter = lT; }
  	  void setAttackTickCounter(S32 lT) {mAttackTickCounter = lT; }
	  bool timeToAttack();

	  Point3F getSpawnPoint() { return mSpawnPoint; }
	  void setSpawnPoint(  Point3F SpawnPoint , bool lCheckExtras = true);

	  void setCready(bool lVal) {  mCready = lVal; }

     ShapeBase* getNearestTarget();
	  StateMachine<AIEntity>* getFSM()const{return mStateMachine;}


	  bool getFollowLeader() const { return mFollowLeader; }
	  bool getFollowArriveNotify() const { return mFollowArriveNotify; }
	  void setFollowArriveNotify(bool lVal) { mFollowArriveNotify = lVal; }

	  void jump() { mJumpCnt++; }


     bool isUnderWorld();
     void handleServerHumanControlledTick();
     bool handleServerAIControlledTick(const Move* move);
     void processTick(const Move* move);
	  ///Update the movement
      void updateMove(const Move *move);
#ifdef _entity_special_collision
      ///Interpolate movement
      bool updatePos(const F32 travelTime = TickSec);
#endif

	  virtual bool setActionThread(S32 action,  bool setOverwriteAnim = false, bool moveBlocked = false);
	  bool setActionThread(const char* sequence, bool setOverwriteAnim = false, bool moveBlocked = false);
	  virtual void pickActionAnimation();
	  virtual void updateActionThread();
	  bool inDeathAnim();
	  bool isDeathAnimation(S32 lAnimId);
	  S32 getRandomDeathAnimation();

	  void setOverwriteAnim(bool lVal) { mOverwriteAnim=lVal; }
      bool setAttackAnimation(S32 animId, S32 overwriteWeaponHands = -1);
      bool setPainAction( S32 id);
      bool setShieldBlock();
      bool setEmotion(S32 id);

	  bool ObjectInLos(ShapeBase* lObj, U32 mask= StaticShapeObjectType);
	  U32  playAnimation(U32 anim_id, F32 pos, F32 rate, F32 trans, bool hold, bool wait);
	  void jumpToPoint(Point3F lPos);
	  void setFollowLeader(bool doFollow , bool doSit = false);
	  bool doesFollowLeader();
	  bool getSitting() const { return mSitting; }
	  void setSitting(bool lVal)  { mSitting = lVal; }

	  bool getHumanControlled() { return mHumanControlled; }

     void setPauseSaveLoad(bool value) { mPauseSaveLoad = value; }

     bool getInLiquid() { return mInLiquid;  }
};


typedef AIEntity::PathUsageType PathUsageType;
DefineEnumType(PathUsageType);


#endif //_AIENTITY_H_
