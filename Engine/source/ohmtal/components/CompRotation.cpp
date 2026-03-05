/**
  CompRotation

  @since 2024-01-01
  @author XXTH

  License at: ohmtal/misc/ohmtalMIT.h
  -----------------------------------------------------------------------------

  This is a minimal example of a component when we get mounted we rotated our
  mount-parent on server side (only).
  
  I did not use a header file this time to make the overhead as small as possible.

  Example:
   MyObject.add(new CompRotation());

*/

#include "scene/sceneObject.h"
#include "console/consoleObject.h"
#include "core/stream/bitStream.h"

#include "CompBase.h"

//-----------------------------------------------------------------------------
// Header
//-----------------------------------------------------------------------------
class CompRotation : public CompBase
{
   typedef CompBase Parent;
   DECLARE_CONOBJECT(CompRotation);

   // Set up any fields that we want to be editable (like position)
   static void initPersistFields();

public:
   CompRotation() :
      mRotationSpeed(1.f)
   {
      mIdent = StringTable->insert("CompRotation");
   }
   // UpdateInterface stuff: 
   virtual void advanceTime(F32 dt);

protected:
   F32          mRotationSpeed;
};

//-----------------------------------------------------------------------------
// Source: 
//-----------------------------------------------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(CompRotation);

void CompRotation::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("CompRotation");
      addField("speed", TypeF32, Offset(mRotationSpeed, CompRotation), "rotation speed");
   endGroup("CompRotation");

}
//-----------------------------------------------------------------------------

void CompRotation::advanceTime(F32 dt)
{
   if (isServerObject() && getMountParent())
   {
      //FIXME hardcoded speed 
      F32 r = mRotationSpeed * 0.01f * dt * M_2PI;
      Point3F pos = getMountParent()->getTransform().getPosition();
      MatrixF rotMatrix;
      rotMatrix.set(EulerF(0.0, 0.0, r));
      MatrixF mat = getMountParent()->getTransform();
      mat.setPosition(pos);
      mat.mul(rotMatrix);
      getMountParent()->setTransform(mat);

   }
}

