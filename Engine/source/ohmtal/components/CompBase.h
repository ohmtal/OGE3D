/**
  CompBase

  @since 2024-01-01
  @author XXTH 

  License at: ohmtal/misc/ohmtalMIT.h

  -----------------------------------------------------------------------------
   *** FIXME PROBLEM FIXME ***
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   copy paste complete parent object call unmount on the object which is copied
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


  -----------------------------------------------------------------------------
  I like the idea of the components
  dont know why it was stopped and try to make it a bit different.
 
  Idea behind it:
  The Component is mounted to the object we want to modify.
  We lookup at the "mount parent" and work with it.
  it may use dynamic_cast onMount (cached) to set the class we
  want to modify.
  ~~~
  Maybe it would be better if i can mount something to an DataBlock
  but would look like synced ! 
  -----------------------------------------------------------------------------

  If you want it networked you need:
    // see also SimpleNetObject

   1.) In Object constructor:
      mNetFlags.set(ScopeAlways | Ghostable);

   2.) Add methods, where you send/receive changes:
      virtual U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream);
      virtual void unpackUpdate(NetConnection* conn, BitStream* stream);

*/


#ifndef _COMPBASE_H_
#define _COMPBASE_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif


class CompBase : public SceneObject, public ITickable
{
public:
   typedef SceneObject Parent;


   // Set up any fields that we want to be editable (like position)
   static void initPersistFields();

   // Networking masks
   enum MaskBits
   {
      UpdateMask = Parent::NextFreeMask << 0,
      NextFreeMask = Parent::NextFreeMask << 1
   };

   StringTableEntry mIdent;
   CompBase():
      mMountParent(NULL)
   {
      mIdent = StringTable->insert("CompBase");
   }

   // handle mount signals:
   virtual void onMount(SceneObject* obj, S32 node);
   virtual void onUnmount(SceneObject* obj, S32 node);

   SceneObject* getMountParent() { return mMountParent; }


   //iTickable interface
   virtual void interpolateTick(F32 dt) {}
   virtual void processTick() {}
   virtual void advanceTime(F32 dt) {}

   // NetObject.
   U32 packUpdate(NetConnection* conn, U32 mask, BitStream* stream) { return Parent::packUpdate(conn, mask, stream); }
   void unpackUpdate(NetConnection* conn, BitStream* stream) { Parent::unpackUpdate(conn, stream); }



private:
      SceneObject* mMountParent;

};

#endif // _COMPBASE_H_
