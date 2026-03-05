#ifdef _DISABLED_

crash and make no sense :P  disabled
you may use baseAnimation without CompAnimation => NOT there is the crash ...
... dont need this i fixed shapebase for the animation ....




/**
  CompAnimation

  @since 2025-03-06
  @author XXTH

  License at: ohmtal/misc/ohmtalMIT.h
  -----------------------------------------------------------------------------

  FIXME animation sometimes cant be changed, need to be reset with root !

  USAGE anitest is this object:
      echo(anitest.playthread(0,"run"));
  or
      echo(anitest.getAnimationObject().playthread(0,"run"));

*/

#include "scene/sceneObject.h"
#include "console/consoleObject.h"
#include "core/stream/bitStream.h"
#include "core/util/safeDelete.h"


#include "CompBase.h"
#include "ohmtal/components/animation/baseAnimation.h"

//-----------------------------------------------------------------------------
// Header
//-----------------------------------------------------------------------------
class CompAnimation : public CompBase
{
   typedef CompBase Parent;
   DECLARE_CONOBJECT(CompAnimation);

   // Set up any fields that we want to be editable (like position)
   static void initPersistFields();

public:
   CompAnimation() :
      mAnimation("") //FIXME: UNUSED AT THE MOMENT!!!! since i dont know how to start it .. onmount does not work! 
   {
      mIdent = StringTable->insert("CompAnimation");
      mBaseAnimation = new BaseAnimation();
      mBaseAnimation->registerObject();
   }
   ~CompAnimation()
   {
      mBaseAnimation->unregisterObject();
      SAFE_DELETE(mBaseAnimation);
   }

   BaseAnimation* getAnimation() { return mBaseAnimation; }

   // handle mount signals:
   virtual void onMount(SceneObject* obj, S32 node);
   virtual void onUnmount(SceneObject* obj, S32 node);

   // U32  packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
   // void unpackUpdate(NetConnection* conn, BitStream* stream);

   void inspectPostApply();


protected:
   StringTableEntry  mAnimation;
   BaseAnimation* mBaseAnimation;
};

//-----------------------------------------------------------------------------
// Source: 
//-----------------------------------------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(CompAnimation);

void CompAnimation::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("CompAnimation");
 //FIXME  addField("animation", TypeCaseString, Offset(mAnimation, CompAnimation));
   endGroup("CompAnimation");

}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CompAnimation::inspectPostApply()
{
   if (isServerObject()) {
//FIXME       mBaseAnimation->playThread(0, mAnimation, true, 0.25);
   }
}
//-----------------------------------------------------------------------------
void CompAnimation::onMount(SceneObject* obj, S32 node)
{
   Parent::onMount(obj, node);
   

   if (!mBaseAnimation->setSceneObject(obj))
   {
      Con::errorf("CompAnimation:: Cast to ShapeBase or tsStatic failed. Can't use animations ! :(");
   }


}

void CompAnimation::onUnmount(SceneObject* obj, S32 node)
{
   Parent::onUnmount(obj, node);
   mBaseAnimation->setSceneObject(NULL);

}


DefineEngineMethod(CompAnimation, getAnimationObject, S32,(), , "get the animation (BaseAnimation) handler")
{
   return object->getAnimation()->getId();
}


DefineEngineMethod(CompAnimation, playThread, bool, (S32 slot, const char* name, bool transition, F32 transitionTime), (-1, "", true, 0.25), "")
{

  return object->getAnimation()->playThread(0, name, transition, transitionTime);
}

#endif
