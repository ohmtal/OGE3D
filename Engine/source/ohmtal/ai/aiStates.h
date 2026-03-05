//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// AIStates by T.Huehn 2009 (XXTH) (c) huehn-software 2009
//-----------------------------------------------------------------------------
#ifndef _AISTATES_H_
#define _AISTATES_H_

#ifndef _STATEMASCHINE_H
#include "fsm/State.h"
#endif

class AIEntity;

namespace AIState {

//-------------------------------------------
class HumanState : public State<AIEntity>
{
  HumanState(){ mStateId = 1; }

  //copy ctor and assignment should be private
  HumanState(const HumanState&);
  HumanState& operator=(const HumanState&);

public:

  //this is a singleton
  static HumanState* Instance();

  virtual void Enter(AIEntity* ent);
  virtual void Execute(AIEntity* ent);
  virtual void Exit(AIEntity* ent);
};
//-------------------------------------------
class CheckVitality : public State<AIEntity>
{
  CheckVitality(){ mStateId = 1; }

  //copy ctor and assignment should be private
  CheckVitality(const CheckVitality&);
  CheckVitality& operator=(const CheckVitality&);

public:

  //this is a singleton
  static CheckVitality* Instance();

  virtual void Enter(AIEntity* ent);
  virtual void Execute(AIEntity* ent);
  virtual void Exit(AIEntity* ent);
};
//-------------------------------------------
class Pause : public State<AIEntity>
{
  Pause(){ mStateId = 2; }

  //copy ctor and assignment should be private
  Pause(const Pause&);
  Pause& operator=(const Pause&);

public:

  //this is a singleton
  static Pause* Instance();

  virtual void Enter(AIEntity* ent);
  virtual void Execute(AIEntity* ent);
  virtual void Exit(AIEntity* ent);


};
//-------------------------------------------
class Hunt : public State<AIEntity>
{
  Hunt(){ mStateId = 3; }

  //copy ctor and assignment should be private
  Hunt(const Hunt&);
  Hunt& operator=(const Hunt&);

public:

  //this is a singleton
  static Hunt* Instance();

  virtual void Enter(AIEntity* ent);
  virtual void Execute(AIEntity* ent);
  virtual void Exit(AIEntity* ent);
};
//-------------------------------------------
class Attack : public State<AIEntity>
{
  Attack(){ mStateId = 4; }

  //copy ctor and assignment should be private
  Attack(const Attack&);
  Attack& operator=(const Attack&);

public:

  //this is a singleton
  static Attack* Instance();

  virtual void Enter(AIEntity* ent);
  virtual void Execute(AIEntity* ent);
  virtual void Exit(AIEntity* ent);


};
//-------------------------------------------
class Wander : public State<AIEntity>
{
  Wander(){ mStateId = 5; }

  //copy ctor and assignment should be private
  Wander(const Wander&);
  Wander& operator=(const Wander&);

public:

  //this is a singleton
  static Wander* Instance();

  virtual void Enter(AIEntity* ent);
  virtual void Execute(AIEntity* ent);
  virtual void Exit(AIEntity* ent);


};
//-------------------------------------------
class Flee : public State<AIEntity>
{
  Flee(){ mStateId = 6; }

  //copy ctor and assignment should be private
  Flee(const Flee&);
  Flee& operator=(const Flee&);

public:

  //this is a singleton
  static Flee* Instance();

  virtual void Enter(AIEntity* ent);
  virtual void Execute(AIEntity* ent);
  virtual void Exit(AIEntity* ent);


};
//-------------------------------------------
class Follow : public State<AIEntity>
{
  Follow(){ mStateId = 7; }

  //copy ctor and assignment should be private
  Follow(const Follow&);
  Follow& operator=(const Follow&);

public:

  //this is a singleton
  static Follow* Instance();

  virtual void Enter(AIEntity* ent);
  virtual void Execute(AIEntity* ent);
  virtual void Exit(AIEntity* ent);
};
//-------------------------------------------
// flee out of range we need to run back fast 
//-------------------------------------------
class RunWander : public State<AIEntity>
{
  RunWander(){ mStateId = 8; }

  //copy ctor and assignment should be private
  RunWander(const RunWander&);
  RunWander& operator=(const RunWander&);

public:

  //this is a singleton
  static RunWander* Instance();

  virtual void Enter(AIEntity* ent);
  virtual void Execute(AIEntity* ent);
  virtual void Exit(AIEntity* ent);
};

}; //namespace AIState
//-------------------------------------------

#endif //_AISTATES_H_
