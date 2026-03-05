//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// AIStates by T.Huehn 2009 (XXTH) (c) huehn-software 2009....2023 still working on

// 2023-03-03 setBehaviourTypeMask is BAD !!! TAG: TMBAD

// !! WHEN ADDING NEW STATE, DONT FORGET TO ADD THE NAME TO AIStateNames in aiEntity.cc, too !!
// * Spells moved to aiEntity for future enhancements

//-----------------------------------------------------------------------------

#include "aiEntity.h"
#include "aiSteering.h"
#include "aiStates.h"
#include "aiMath.h"

namespace AIState {
//-----------------------------------------------------------------------------
// HumanState - an empty state ...
//-----------------------------------------------------------------------------
HumanState* HumanState::Instance()
{
  static HumanState instance;
  return &instance;
}
//---------------------------------------------------------
void HumanState::Enter(AIEntity* ent)
{
}
//---------------------------------------------------------
void HumanState::Execute(AIEntity* ent)
{

}
//---------------------------------------------------------
void HumanState::Exit(AIEntity* ent)
{
}

//-----------------------------------------------------------------------------
// CheckVitality
//-----------------------------------------------------------------------------
CheckVitality* CheckVitality::Instance()
{
  static CheckVitality instance;
  return &instance;
}
//---------------------------------------------------------
void CheckVitality::Enter(AIEntity* ent)
{
}
//---------------------------------------------------------
void CheckVitality::Execute(AIEntity* ent)
{
	ent->guessVitalitySpell();

	if (ent->mCurSpell) {
		ent->mElapsedTicks += ent->mCurUpdateTickCount;
		if (ent->mElapsedTicks >= ent->mCurSpell->mCooldownTicks)
			ent->mCurSpell = NULL;
	}

}
//---------------------------------------------------------
void CheckVitality::Exit(AIEntity* ent)
{
}
//-----------------------------------------------------------------------------
// Pause
//-----------------------------------------------------------------------------
Pause* Pause::Instance()
{
  static Pause instance;
  return &instance;
}
//---------------------------------------------------------
void Pause::Enter(AIEntity* ent)
{
   ent->mElapsedTicks = 0; //2023-03-04
	if (ent->getAimObject())
			ent->clearAim();
	ent->setMoveState(AIEntity::ModeStop);
//TMBAD	ent->setBehaviourTypeMask(0);

	if ((ent->mPauseHeal || ent->getSitting() ) && ent->mPauseHealPercent > 0) {
		//!! we only tick all 3 seconds! raise values! 31.25 is the magic value
		// FIXME manareg should be from spirit!!!
		if ( ent->mPauseHealPercent > 1.f ) 
				ent->mPauseHealPercent = 1.f;
		F32 lmul = (F32) ent->mPauseHealPercent / 31.25f;
		ent->setRechargeRate(ent->getMaxEnergy() * lmul ); //0.096f == 3 mana / sec !! 2.0 5% per sec
		ent->setRepairRate(ent->getMaxDamage() * lmul); //0.192 == 6 live / sec !! 2.0 10% per sec
		
	}
	
/* 1.97.3 wanted to added this but sucks with states!
	ent->setPauseTicks( gRandGen.randI(5,100) ) ;
*/

}
//---------------------------------------------------------
void Pause::Execute(AIEntity* ent)
{

   ent->mElapsedTicks += ent->mCurUpdateTickCount; //2023-03-04
	//Look for oponents
	if (ShapeBase* tgt = ent->getNearestTarget()) {
		ent->setTickCounter(1);
		ent->setAimObject(tgt);
		switch (ent->getPersonality()) {
			case AIEntity::Aggressive:
				ent->getFSM()->ChangeState(Hunt::Instance());
				break;
			case AIEntity::Sheepish:
				ent->getFSM()->ChangeState(Flee::Instance());
				break;
			case AIEntity::Dontcare:
				//nothing do to ;)
				break;
			default:
				Con::errorf("PAUSE FOUND TARGET BUT UNKNOWN PERSONALITY STATE!!!");
		}
		return;
	}

   //2023-03-04
   if (!ent->mWanderPositonSet.isZero()) {
      if (ent->mElapsedTicks > 64) //FIXME hardcoded ?! 256 ~ is about 5 sec , CHANGED to 2 sec => 64
      {
         ent->getFSM()->ChangeState(Wander::Instance());
         return;
      }
   } else 
    // 1.97.3 was gRandGen.randI(1, 20) but since it is to often we do   gRandGen.randI(1, 50)
	// pre 2023-03-04 if (!ent->getSitting() && (ent->mPathUsage == AIEntity::PathWander || gRandGen.randI(1, 50) == 5)) {
      if (!ent->getSitting() &&
            // mhh only with pathWander ? (ent->mPathUsage == AIEntity::PathWander || (ent->mElapsedTicks > 256 && gRandGen.randI(0, 10) == 0))
            (ent->mElapsedTicks > 256 && gRandGen.randI(0, 10) == 0)
         )
      {
         ent->getFSM()->ChangeState(Wander::Instance());
         return;
      }

}
//---------------------------------------------------------
void Pause::Exit(AIEntity* ent)
{
   ent->mElapsedTicks = 0; //2023-03-04 
	ent->setMoveState(AIEntity::ModeMove);
	if (ent->mPauseHeal) {
		ent->setRechargeRate(0.f); 
		ent->setRepairRate(0.f); 
	}
	if (ent->getSitting())
		ent->setSitting(false);
}
//-----------------------------------------------------------------------------
// Hunt
//-----------------------------------------------------------------------------
Hunt* Hunt::Instance()
{
  static Hunt instance;
  return &instance;
}
//---------------------------------------------------------
void Hunt::Enter(AIEntity* ent)
{
	ent->setMoveState(AIEntity::ModeMove);

   /* TMBAD
	if (ent->mPathUsage == AIEntity::PathHunt)
		ent->setBehaviourTypeMask(SeparationBehaviourType | Wall_AvoidanceBehaviourType | PursuitPathBehaviourType);
	else
		ent->setBehaviourTypeMask(SeparationBehaviourType | Wall_AvoidanceBehaviourType | PursuitBehaviourType);
    */
   if (ent->mPathUsage == AIEntity::PathHunt)
      ent->setBehaviourOn(PursuitPathBehaviourType);
   else
      ent->setBehaviourOn(PursuitBehaviourType);



	ent->guessRangeSpell();

	ent->enableRun();

}
//---------------------------------------------------------
void Hunt::Execute(AIEntity* ent)
{
   if (!ent->validTargetHunt())
   {
      ent->clearAim();
      if (ent->getFollowLeader()) {
         ent->getFSM()->ChangeState(Follow::Instance());
      }	else {

         //2023-03-24 continue the path if enemy is out of range 
         if (!ent->mWanderPositonSet.isZero()) {
            ent->getFSM()->ChangeState(Wander::Instance());
         }
         else {
            ent->getFSM()->ChangeState(Pause::Instance());
         }
      }

		return;
	}
    
	//FIXME remoteSpells

	if (ent->inAttackDistance()) {
		ent->setTickCounter(1);
		ent->getFSM()->ChangeState(Attack::Instance());
		return;
	}

}
//---------------------------------------------------------
void Hunt::Exit(AIEntity* ent)
{
   //TMBAD added:
   if (ent->mPathUsage == AIEntity::PathHunt)
      ent->setBehaviourOff(PursuitPathBehaviourType);
   else
      ent->setBehaviourOff(PursuitBehaviourType);

	ent->enableWalk();
}
//-----------------------------------------------------------------------------
// Attack
//-----------------------------------------------------------------------------
Attack* Attack::Instance()
{
  static Attack instance;
  return &instance;
}
//---------------------------------------------------------
void Attack::Enter(AIEntity* ent)
{

//XXTH wall_avoid make them jitter?!	ent->setBehaviourTypeMask(Wall_AvoidanceBehaviourType | SeparationBehaviourType);
    //neee dont rotate to target ... ent->setMoveState(AIEntity::ModeStop); //2023-03-03
	//TMBAD ent->setBehaviourTypeMask(SeparationBehaviourType);
	ent->setAttackTickCounter(0); //no delay until attack!
	
}
//---------------------------------------------------------
void Attack::Execute(AIEntity* ent)
{
    

	if ( !ent->validTargetAttack() )
	{
		ent->clearAim();
		if (ent->getFollowLeader())
			ent->getFSM()->ChangeState(Follow::Instance());
		else 
		{
			if (ent->getHumanControlled())
			{
				ent->getFSM()->ChangeState(HumanState::Instance());
			} else {
               ent->getFSM()->ChangeState(Pause::Instance());
               //2023-03-02 looks stupid switching too fast
               // ent->getFSM()->ChangeState(Wander::Instance());
			}
		}
	} else if (ent->inAttackDistance())
	{
		ent->setCready(true); 
		if (ent->timeToAttack())
		{
			ent->guessMeleeSpell();
		}
    
		//XXTH changed to 256 ms think about poor performance!! ent->setTickCounter(3);  //96ms
		ent->setTickCounter(8); //256ms

		return;
	} else if (!ent->getHumanControlled()) {
		ent->setTickCounter(1);
		ent->getFSM()->ChangeState(Hunt::Instance());
		return;
	}
}
//---------------------------------------------------------
void Attack::Exit(AIEntity* ent)
{
	ent->setCready(false);
   // compat to aiPlayer
    Con::executef(ent->getDataBlock(), "onTargetExitLOS", ent->getIdString());
 
}
//-----------------------------------------------------------------------------
// Wander
//-----------------------------------------------------------------------------
Wander* Wander::Instance()
{
  static Wander instance;
  return &instance;
}
//---------------------------------------------------------
void Wander::Enter(AIEntity* ent)
{
	ent->setMoveState(AIEntity::ModeMove);
	ent->clearAim();
	if (ent->mPathUsage == AIEntity::PathWander ) {
	  //TMBAD ent->setBehaviourTypeMask(Wall_AvoidanceBehaviourType | SeparationBehaviourType | Follow_PathBehaviourType);
      ent->setBehaviourOn(Follow_PathBehaviourType);
      if (!ent->mWanderPositonSet.isZero()) //2023-03-08 for setPath!!!!!!
         ent->setWanderDestination(ent->mWanderPositonSet);
	} else if (ent->mPathUsage == AIEntity::PathHunt) {
      //TMBAD ent->setBehaviourTypeMask(Wall_AvoidanceBehaviourType | SeparationBehaviourType | Follow_PathBehaviourType);
      ent->setBehaviourOn(Follow_PathBehaviourType);
	  ent->setWanderDestination(ent->mWanderPositonSet); 
	  
	} else {
      //TMBAD ent->setBehaviourTypeMask(Wall_AvoidanceBehaviourType | ArriveBehaviourType );

	  ent->setWanderDestination(ent->mWanderPositonSet); 

	  // we do a normal wander so lets add a wander timeout
	  ent->mWanderTicksLeft = 180; // ticks each sec so its = 3 minutes
	  
	}

}
//---------------------------------------------------------
void Wander::Execute(AIEntity* ent)
{

	//Look for oponents
	if (ShapeBase* tgt = ent->getNearestTarget()) {
		ent->setTickCounter(1);
		ent->setAimObject(tgt);

		switch (ent->getPersonality()) {
			case AIEntity::Aggressive:
				ent->getFSM()->ChangeState(Hunt::Instance());
				break;
			case AIEntity::Sheepish:
				ent->getFSM()->ChangeState(Flee::Instance());
				break;
			default:
				Con::errorf("WANDER FOUND TARGET BUT UNKNOWN PERSONALITY STATE!!!");
		}
		return;
	}


	if (ent->mPathUsage != AIEntity::PathWander) 
	{
        /* XXTH 2023-03-09 only if we have no aimobject!! */
		if (!ent->getAimObject() && !ent->mWanderPositonSet.isZero()) {
			//update SpawnPoint on forced Wander
			ent->setSpawnPoint(ent->getPosition(),false);
		}
      


		//This is strange ZERO Destination is bad!
		if (ent->getTargetPos().isZero())
		{
			ent->getFSM()->ChangeState(Pause::Instance());
			return;
		}

		// 1.97.3 dunno why but pathed wander stop sometimes ?! i must check this here!
		bool lforceWanderDone=false;
		if (ent->BehaviourOn( Follow_PathBehaviourType ))
		{
			if (ent->getSteering()->getPath()->isStopped() && ent->getSteering()->getPath()->inProcess())
			{
#ifdef TORQUE_DEBUG
				Con::warnf("WARNING: Entity (%d) stopped it's path and does still wander !! thats bad, i handle it here!!! ", ent->getId());
#endif
				lforceWanderDone = true;
			}
		} else { //normal wander without path
			ent->mWanderTicksLeft--;
			if (ent->mWanderTicksLeft < 0)
			{
#ifdef TORQUE_DEBUG
				Con::warnf("WARNING: Entity (%d) wandered longer than 3 minutes!! thats bad, i handle it here!!! ", ent->getId());
#endif
				lforceWanderDone = true;
			}
		}


		//double check ... this is also in steering ... bad! mhhh!
		//mhh whats up if this position is unreachable and we get stuck ?! 
		// => stuck should be handled by aientiy .. hopefully
		if (lforceWanderDone || AIMath::Distance2D(ent->getPosition(),ent->getTargetPos()) <= ent->getMoveTolerance()) {
			ent->getFSM()->ChangeState(Pause::Instance());
			ent->mWanderPositonSet = Point3F(0.f,0.f,0.f);
			ent->throwCallback("onWanderDone");
		}
		 
	} else {
			
         
         //update SpawnPoint on Path :: 2023-03-09 only if we have no aimobject!! 
         if (!ent->getAimObject()) {
            ent->setSpawnPoint(ent->getPosition(), false);
         }
         if (!ent->BehaviourOn(Follow_PathBehaviourType))
         {
            //this can happen when a wander is running and wander is set!  
            //TMBAD ent->setBehaviourTypeMask(Wall_AvoidanceBehaviourType | SeparationBehaviourType | Follow_PathBehaviourType);
            ent->setBehaviourOn(Follow_PathBehaviourType);
         } 

	}
}
//---------------------------------------------------------
void Wander::Exit(AIEntity* ent)
{
//TMBAD addeD:
   if (ent->mPathUsage != AIEntity::PathNone) {
      //TMBAD ent->setBehaviourTypeMask(Wall_AvoidanceBehaviourType | SeparationBehaviourType | Follow_PathBehaviourType);
      ent->setBehaviourOff(Follow_PathBehaviourType);
   }


}
//-----------------------------------------------------------------------------
// Flee
//-----------------------------------------------------------------------------
Flee* Flee::Instance()
{
  static Flee instance;
  return &instance;
}
//---------------------------------------------------------
void Flee::Enter(AIEntity* ent)
{
	ent->setMoveState(AIEntity::ModeMove);
	ent->enableRun();
	//TMBAD ent->setBehaviourTypeMask(SeparationBehaviourType | Wall_AvoidanceBehaviourType | EvadeBehaviourType);
   ent->setBehaviourOn(EvadeBehaviourType);
}
//---------------------------------------------------------
void Flee::Execute(AIEntity* ent)
{
	//check states, ai, ai dead, water or land state **1.97.3 we got out of ignore range let wander back but wander is slow :(!

 	if ( ent->getDistanceFromSpawn(ent)>=ent->mIgnoreRange  ) 
	{
			ent->setTickCounter(1);
			ent->clearAim();
			//2019-02-12 jitter bad on switch to wander if ignorerange same as scan range?! 
			ent->getFSM()->ChangeState(RunWander::Instance());
			return;
	}

	if (!ent->getAimObject() 
		|| ent->getAimObject()->getDamageState() != ent->Enabled
		|| (ent->mInLiquid  && !ent->mAllowWater)
		|| (!ent->mInLiquid && !ent->mAllowLand)
		) {

			ent->setTickCounter(1);
			ent->clearAim();
			ent->getFSM()->ChangeState(Wander::Instance());
			return;
	}


	F32 dist = AIMath::Distance2D(ent->getPosition(),ent->getTargetPos());
	if (dist > ent->getPanicDistance()) {
		ent->clearAim();
		ent->setTickCounter(1);
		ent->getFSM()->ChangeState(Pause::Instance());
		return;
	}

}
//---------------------------------------------------------
void Flee::Exit(AIEntity* ent)
{
   //TMBAD added:
   ent->setBehaviourOff(EvadeBehaviourType);
	ent->enableWalk();
}
//-----------------------------------------------------------------------------
// Follow
//-----------------------------------------------------------------------------
Follow* Follow::Instance()
{
  static Follow instance;
  return &instance;
}
//---------------------------------------------------------
void Follow::Enter(AIEntity* ent)
{
	ent->setMoveState(AIEntity::ModeMove);
	ent->enableRun();
/*TMBAD
	if (ent->mStuckBeam) //not if we want stuckbeam!
		ent->setBehaviourTypeMask( Offset_PursuitBehaviourType );
	else
		ent->setBehaviourTypeMask(Wall_AvoidanceBehaviourType | Offset_PursuitBehaviourType);
*/
   ent->setBehaviourOn(Offset_PursuitBehaviourType);
      
	ent->setFollowArriveNotify(true);

}
//---------------------------------------------------------
void Follow::Execute(AIEntity* ent)
{
	//Look for oponents (1.97.3 also on follow!
	if (ShapeBase* tgt = ent->getNearestTarget()) {
		ent->setTickCounter(1);
		ent->setAimObject(tgt);

		switch (ent->getPersonality()) {
			case AIEntity::Aggressive:
				ent->getFSM()->ChangeState(Hunt::Instance());
				break;
			case AIEntity::Sheepish:
				ent->getFSM()->ChangeState(Flee::Instance());
				break;
			default:
				Con::errorf("FOLLOW FOUND TARGET BUT UNKNOWN PERSONALITY STATE!!!");
		}
		return;
	}

	if (!ent->getLeaderObject() ) {
      ent->setLeaderObject(NULL);
      ent->setTickCounter(1);
      ent->getFSM()->ChangeState(Pause::Instance());
      return;
	}


	ent->setSpawnPoint(ent->getPosition(),false);

	if (ent->getFollowArriveNotify() && 
		AIMath::Distance2D(ent->getLeaderObject()->getPosition(), ent->getPosition()) <= ent->getFormationOffset().len()*1.2)
	{
		ent->setFollowArriveNotify(false);
		ent->throwCallback("onLeaderReached");
	}
		
		
}
//---------------------------------------------------------
void Follow::Exit(AIEntity* ent)
{
	ent->enableWalk();
	ent->setFollowArriveNotify(false);
}


//-----------------------------------------------------------------------------
// RunWander
//-----------------------------------------------------------------------------
RunWander* RunWander::Instance()
{
  static RunWander instance;
  return &instance;
}
//---------------------------------------------------------
void RunWander::Enter(AIEntity* ent)
{
	ent->enableRun();
}
//---------------------------------------------------------
void RunWander::Execute(AIEntity* ent)
{
  ent->setTickCounter(1);
  ent->clearAim();
  ent->getFSM()->ChangeState(Wander::Instance());

}
//---------------------------------------------------------
void RunWander::Exit(AIEntity* ent)
{
}

}; //namespace AIState 

/* template for new state:
//-----------------------------------------------------------------------------
// Pause
//-----------------------------------------------------------------------------
Pause* Pause::Instance()
{
  static Pause instance;
  return &instance;
}
//---------------------------------------------------------
void Pause::Enter(AIEntity* ent)
{
}
//---------------------------------------------------------
void Pause::Execute(AIEntity* ent)
{
}
//---------------------------------------------------------
void Pause::Exit(AIEntity* ent)
{
}
*/

