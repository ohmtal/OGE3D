#ifndef _STATEMASCHINE_H
#define _STATEMASCHINE_H
//------------------------------------------------------------------------
//
//  Name:   State.h
//
//  Desc:   abstract base class to define an interface for a state
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//  Modification for TGE T.Huehn (XXTH)
//
//------------------------------------------------------------------------
//FIXME struct Telegram;

template <class entity_type>
class State
{
public:

  U16 mStateId; //used to identify State to get a name or something like that!

  virtual ~State(){ mStateId=0; }

  //this will execute when the state is entered
  virtual void Enter(entity_type*)=0;

  //this is the states normal update function
  virtual void Execute(entity_type*)=0;

  //this will execute when the state is exited. 
  virtual void Exit(entity_type*)=0;

  virtual U16 getStateId() { return mStateId; }

  //this executes if the agent receives a message from the 
  //message dispatcher
//XXTH FIXME   virtual bool OnMessage(entity_type*, const Telegram&)=0;
};

#endif