#ifdef _DISABLED_
dont use me !!!

TODO: REMOVE CODE AGAIN

//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
// Copyright (c) 2025 Ohmtal Game Studio
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

#include "ohmtal/components/animation/baseAnimation.h"
#include "ohmtal/components/animation/baseAnimation_ScriptBinding.h"

#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "ts/tsShapeInstance.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "gfx/gfxTransformSaver.h"
#include "console/engineAPI.h"
#include "lighting/lightQuery.h"
#include "gfx/sim/debugDraw.h" 
#include "T3D/shapeBase.h"
#include "T3D/tsStatic.h"

extern bool gEditingMission;

//////////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////////

// IMPLEMENT_CALLBACK( BaseAnimation, onAnimationStart, void, ( Component* obj, const String& animName ), ( obj, animName ),
IMPLEMENT_CALLBACK(BaseAnimation, onAnimationStart, void, (const String& animName), (animName),
      "@brief Called when we collide with another object.\n\n"
                   "@param obj The ShapeBase object\n"
                   "@param collObj The object we collided with\n"
                   "@param vec Collision impact vector\n"
                   "@param len Length of the impact vector\n" );

IMPLEMENT_CALLBACK(BaseAnimation, onAnimationEnd, void, (const char* animName), (animName),
                   "@brief Called when we collide with another object.\n\n"
                   "@param obj The ShapeBase object\n"
                   "@param collObj The object we collided with\n"
                   "@param vec Collision impact vector\n"
                   "@param len Length of the impact vector\n" );

IMPLEMENT_CALLBACK(BaseAnimation, onAnimationTrigger, void, (const String& animName, S32 triggerID), (animName, triggerID),
                   "@brief Called when we collide with another object.\n\n"
                   "@param obj The ShapeBase object\n"
                   "@param collObj The object we collided with\n"
                   "@param vec Collision impact vector\n"
                   "@param len Length of the impact vector\n" );



//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
BaseAnimation::BaseAnimation() 
{


   // :(( only with scopealways!!
   //mNetFlags.set(Ghostable);

   mNetFlags.set(Ghostable| ScopeAlways);

   mShapeInstance = NULL;
   mSceneObject = NULL;




   for (U32 i = 0; i < MaxScriptThreads; i++)
   {
      mAnimationThreads[i].sequence = -1;
      mAnimationThreads[i].thread = 0;
      mAnimationThreads[i].sound = 0;
      mAnimationThreads[i].state = Thread::Stop;
      mAnimationThreads[i].atEnd = false;
      mAnimationThreads[i].timescale = 1.f;
      mAnimationThreads[i].position = -1.f;
      mAnimationThreads[i].transition = true;
   }
}

BaseAnimation::~BaseAnimation()
{
}

IMPLEMENT_CO_NETOBJECT_V1(BaseAnimation);

bool BaseAnimation::onAdd()
{
   if (!Parent::onAdd())
      return false;

   //we need at least one layer
   for (U32 i = 0; i < MaxScriptThreads; i++) 
   {
      Thread& st = mAnimationThreads[i];

      if (st.sequence != -1) 
      {
         // TG: Need to see about suppressing non-cyclic sounds
         // if the sequences were activated before the object was
         // ghosted.
         // TG: Cyclic animations need to have a random pos if
         // they were started before the object was ghosted.

         // If there was something running on the old shape, the thread
         // needs to be reset. Otherwise we assume that it's been
         // initialized either by the constructor or from the server.
         bool reset = st.thread != 0;
         st.thread = 0;

         if (st.sequence != -1)
         {
            setThreadSequence(i, st.sequence, reset);
         }
      }

      if (st.thread)
         updateThread(st);
   }

   return true;
}

void BaseAnimation::onRemove()
{
   Parent::onRemove();

}


bool BaseAnimation::setSceneObject(SceneObject* lObject)
{

   if (!lObject)
   {
      setShapeInstance(NULL);
      mSceneObject = NULL;
      if (isServerObject())
         setMaskBits(SceneObjectMask);
      return true;
   }
   

   bool success = false;
   ShapeBase* lShapeBase = dynamic_cast<ShapeBase*>(lObject);
   if (lShapeBase) {
      setShapeInstance(lShapeBase->getShapeInstance());
      mSceneObject = lObject;
      success = true;
   }

   if (!success)
   {
      TSStatic* ltsStatic = dynamic_cast<TSStatic*>(lObject);
      if (ltsStatic) {
         setShapeInstance(ltsStatic->getShapeInstance());
         success = true;
      }
   }

   if (success)
   {
      mSceneObject = lObject;
      setTransform(mSceneObject->getTransform());
      if (isServerObject())
         setMaskBits(SceneObjectMask);
   }

   return success;
}

void BaseAnimation::setShapeInstance(TSShapeInstance* lShapeInstance)
{
   mShapeInstance = lShapeInstance;

   if (!mShapeInstance)
      return;

   for (U32 i = 0; i < MaxScriptThreads; i++)
   {
      Thread& st = mAnimationThreads[i];

      st.thread = mShapeInstance->addThread();
   }
}



void BaseAnimation::initPersistFields()
{
   Parent::initPersistFields();
}

U32 BaseAnimation::packUpdate(NetConnection* con, U32 mask, BitStream* stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);


   if (stream->writeFlag(mask & SceneObjectMask))
   {
      S32 shapeGhost = con->getGhostIndex(mSceneObject);
      if (shapeGhost < 0) {
         shapeGhost = 0;
         setMaskBits(SceneObjectMask);
      }
      stream->writeRangedU32(U32(shapeGhost), 0, NetConnection::MaxGhostCount);
   }


   for (int i = 0; i < MaxScriptThreads; i++)
   {
      Thread& st = mAnimationThreads[i];
      if (stream->writeFlag((st.sequence != -1 || st.state == Thread::Destroy) && (mask & (ThreadMaskN << i))))
      {
         stream->writeInt(st.sequence, ThreadSequenceBits);
         stream->writeInt(st.state, 2);
         stream->write(st.timescale);
         stream->write(st.position);
         stream->writeFlag(st.atEnd);
         stream->writeFlag(st.transition);
      }
   }

   return retMask;
}

void BaseAnimation::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag())
   {
      S32 ghost_idx = stream->readRangedU32(0, NetConnection::MaxGhostCount);
            
      if (ghost_idx == 0 || ghost_idx  > NetConnection::MaxGhostCount)
      {
         setSceneObject(NULL);
         if (ghost_idx > NetConnection::MaxGhostCount)
            Con::errorf("BaseAnimation::unpackUpdate => GhostIndex > MaxGhostCount = %d!!", ghost_idx);
      }
      else {
         SceneObject* lSceneObject = dynamic_cast<SceneObject*>(con->resolveGhost(ghost_idx));
         setSceneObject(lSceneObject);
      }
   }


   for (S32 i = 0; i < MaxScriptThreads; i++) 
   {
      if (stream->readFlag()) 
      {
         Thread& st = mAnimationThreads[i];
         U32 seq = stream->readInt(ThreadSequenceBits);
         st.state = Thread::State(stream->readInt(2));
         stream->read( &st.timescale );
         stream->read( &st.position );
         st.atEnd = stream->readFlag();
         bool transition = stream->readFlag();

         if (!st.thread || st.sequence != seq && st.state != Thread::Destroy)
            setThreadSequence(i, seq, false, transition);
         else
            updateThread(st);
      }
   }
}

void BaseAnimation::processTick()
{
   //Parent::processTick();

   if (!getShape())
      return;

   if (isServerObject()) 
   {
      // Server only...
      advanceThreads(TickSec);
   }
}


void BaseAnimation::advanceTime(F32 dt)
{

   Parent::advanceTime(dt);

   if (isClientObject() && mSceneObject)
   {
      setTransform(mSceneObject->getTransform());
   }

   // On the client, the shape threads and images are
   // advanced at framerate.
   advanceThreads(dt);
}
//
const char *BaseAnimation::getThreadSequenceName(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence == -1)
   {
      // Invalid Animation.
      return "";
   }

   // Name Index
   TSShape* shape = getShape();

   if (shape)
   {
      const U32 nameIndex = shape->sequences[st.sequence].nameIndex;

      // Return Name.
      return shape->getName(nameIndex);
   }

   return "";
}

bool BaseAnimation::setThreadSequence(U32 slot, S32 seq, bool reset, bool transition, F32 transTime)
{
   Thread& st = mAnimationThreads[slot];
   if (st.thread && st.sequence == seq && st.state == Thread::Play && !reset)
      return true;

   // Handle a -1 sequence, as this may be set when a thread has been destroyed.
   if (seq == -1)
      return true;

   if (seq < MaxSequenceIndex)
   {
      setMaskBits(ThreadMaskN << slot);
      st.sequence = seq;
      st.transition = transition;

      if (reset)
      {
         st.state = Thread::Play;
         st.atEnd = false;
         st.timescale = 1.f;
         st.position = 0.f;
      }

      if (mShapeInstance)
      {
         if (!st.thread)
            st.thread = mShapeInstance->addThread();

         if (transition)
         {
            mShapeInstance->transitionToSequence(st.thread, seq, st.position, transTime, true);
         }
         else
         {
            mShapeInstance->setSequence(st.thread, seq, 0);
            stopThreadSound(st);
         }

         updateThread(st);
      }
      return true;
   }
   return false;
}

S32 BaseAnimation::getThreadSequenceID(S32 slot)
{
   if (slot >= 0 && slot < BaseAnimation::MaxScriptThreads)
   {
      return mAnimationThreads[slot].sequence;
   }
   else
   {
      return -1;
   }
}

void BaseAnimation::updateThread(Thread& st)
{
   if (!getShape())
      return;

   switch (st.state)
   {
      case Thread::Stop:
      {
         mShapeInstance->setTimeScale(st.thread, 1.f);
         mShapeInstance->setPos(st.thread, (st.timescale > 0.f) ? 0.0f : 1.0f);
      } // Drop through to pause state

      case Thread::Pause:
      {
         if (st.position != -1.f)
         {
            mShapeInstance->setTimeScale(st.thread, 1.f);
            mShapeInstance->setPos(st.thread, st.position);
         }

         mShapeInstance->setTimeScale(st.thread, 0.f);
         stopThreadSound(st);
      } break;

      case Thread::Play:
      {
         if (st.atEnd)
         {
            mShapeInstance->setTimeScale(st.thread, 1);
            mShapeInstance->setPos(st.thread, (st.timescale > 0.f) ? 1.0f : 0.0f);
            mShapeInstance->setTimeScale(st.thread, 0);
            stopThreadSound(st);
            st.state = Thread::Stop;
         }
         else
         {
            if (st.position != -1.f)
            {
               mShapeInstance->setTimeScale(st.thread, 1.f);
               mShapeInstance->setPos(st.thread, st.position);
            }

            mShapeInstance->setTimeScale(st.thread, st.timescale);
            if (!st.sound)
            {
               startSequenceSound(st);
            }
         }
      } break;

      case Thread::Destroy:
      {
         stopThreadSound(st);
         st.atEnd = true;
         st.sequence = -1;
         if (st.thread)
         {
            mShapeInstance->destroyThread(st.thread);
            st.thread = 0;
         }
      } break;
   }
}

bool BaseAnimation::stopThread(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1 && st.state != Thread::Stop) 
   {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Stop;
      updateThread(st);
      return true;
   }
   return false;
}

bool BaseAnimation::destroyThread(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1 && st.state != Thread::Destroy) 
   {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Destroy;
      updateThread(st);
      return true;
   }
   return false;
}

bool BaseAnimation::pauseThread(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1 && st.state != Thread::Pause) 
   {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Pause;
      updateThread(st);
      return true;
   }
   return false;
}

bool BaseAnimation::playThread(U32 slot)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1 && st.state != Thread::Play)
   {
      setMaskBits(ThreadMaskN << slot);
      st.state = Thread::Play;
      updateThread(st);
      return true;
   }
   return false;
}

bool BaseAnimation::playThread(U32 slot, const char* name, bool transition, F32 transitionTime)
{
   if (slot < BaseAnimation::MaxScriptThreads)
   {
      if (!dStrEqual(name, ""))
      {
         if (TSShape* shape = getShape())
         {
            S32 seq = shape->findSequence(name);
            if (seq != -1 && setThreadSequence(slot, seq, true, transition, transitionTime))
            {
               return true;
            }
            else if (seq == -1)
            {
               //We tried to play a non-existaint sequence, so stop the thread just in case
               destroyThread(slot);
               return false;
            }
         }
      }
      else
      {
         if (playThread(slot))
            return true;
      }
   }

   return false;
}

bool BaseAnimation::setThreadAnimation(U32 slot, const char* name)
{
   if (slot < BaseAnimation::MaxScriptThreads)
   {
      if (!dStrEqual(name, ""))
      {
         if (TSShape* shape = getShape())
         {
            S32 seq = shape->findSequence(name);
            if (seq != -1 && setThreadSequence(slot, seq, false, false))
            {
               Thread& st = mAnimationThreads[slot];
               if (st.position == -1)
                  st.position = 0;
               //st.state = Thread::Pause;
               return true;
            }
            else if (seq == -1)
            {
               //We tried to play a non-existaint sequence, so stop the thread just in case
               destroyThread(slot);
               return false;
            }
         }
      }
      else
      {
         if (playThread(slot))
            return true;
      }
   }

   return false;
}

bool BaseAnimation::setThreadPosition(U32 slot, F32 pos)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1)
   {
      setMaskBits(ThreadMaskN << slot);
      st.position = pos;
      st.atEnd = false;
      updateThread(st);

      return true;
   }
   return false;
}

bool BaseAnimation::setThreadDir(U32 slot, bool forward)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1)
   {
      if ((st.timescale >= 0.f) != forward)
      {
         setMaskBits(ThreadMaskN << slot);
         st.timescale *= -1.f;
         st.atEnd = false;
         updateThread(st);
      }
      return true;
   }
   return false;
}

bool BaseAnimation::setThreadTimeScale(U32 slot, F32 timeScale)
{
   Thread& st = mAnimationThreads[slot];
   if (st.sequence != -1)
   {
      if (st.timescale != timeScale)
      {
         setMaskBits(ThreadMaskN << slot);
         st.timescale = timeScale;
         updateThread(st);
      }
      return true;
   }
   return false;
}

void BaseAnimation::stopThreadSound(Thread& thread)
{
   return;
}

void BaseAnimation::startSequenceSound(Thread& thread)
{
   return;
}

void BaseAnimation::advanceThreads(F32 dt)
{

   if (mShapeInstance == nullptr || !getShape())
      return;

   for (U32 i = 0; i < MaxScriptThreads; i++)
   {
      Thread& st = mAnimationThreads[i];
      if (st.thread && st.sequence != -1)
      {
         if (!getShape()->sequences[st.sequence].isCyclic() &&
            !st.atEnd &&
            ((st.timescale > 0.f) ? mShapeInstance->getPos(st.thread) >= 1.0 : mShapeInstance->getPos(st.thread) <= 0))
         {
            st.atEnd = true;
            updateThread(st);

            if (!isClientObject())
            {
               Con::executef(this, "onAnimationEnd", st.thread->getSequenceName());
            }
         }

         // Make sure the thread is still valid after the call to onEndSequence_callback().
         // Someone could have called destroyThread() while in there.
         if (st.thread)
         {
            mShapeInstance->advanceTime(dt, st.thread);
         }

         if (mShapeInstance && !isClientObject())
         {
            for (U32 stateIDx = 1; stateIDx < 32; stateIDx++)
            {
               if (mShapeInstance->getTriggerState(stateIDx))
               {
                  const char* animName = st.thread->getSequenceName().c_str();
                  onAnimationTrigger_callback(animName, stateIDx);
               }
            }
         }

         if (isClientObject())
         {
            mShapeInstance->animate();
         }
      }
   }
}



TSShape* BaseAnimation::getShape()
{
   if (mShapeInstance == NULL)
      return NULL;
   return mShapeInstance->getShape();
}

S32 BaseAnimation::getAnimationCount()
{
   if (getShape())
      return getShape()->sequences.size();
   else
      return 0;
}

S32 BaseAnimation::getAnimationIndex(const char* name)
{
   if (getShape())
      return getShape()->findSequence(name);
   else
      return -1;
}

const char* BaseAnimation::getAnimationName(S32 index)
{
   if (getShape())
   {
      if (index >= 0 && index < getShape()->sequences.size())
         return getShape()->getName(getShape()->sequences[index].nameIndex);
   }

   return "";
}
#endif
