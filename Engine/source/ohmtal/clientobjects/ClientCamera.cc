//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#include "ClientCamera.h"
#include "T3D/camera.h"
//-----------------------------------------------------------------------------------------------------------
// ClientCamera
//-----------------------------------------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ClientCamera);

// HAB ISCH :P 2023-12-12 #pragma message("ClientCamera << Testme")

//-----------------------------------------------------------------------------------------------------------
void ClientCamera::initPersistFields()
{

   Parent::initPersistFields();

   addGroup("Misc");
   //orig   addField("ClientDataBlock", TypeGameBaseDataPtr, Offset(mDataBlock, ClientCamera));
   addField("ClientDataBlock", TYPEID< GameBaseData >(), Offset(mDataBlock, ClientCamera));

   endGroup("Misc");
   addField("Yaw", TypeF32, Offset(mYaw, ClientCamera));
   addField("Pitch", TypeF32, Offset(mPitch, ClientCamera));
}
//-----------------------------------------------------------------------------------------------------------
bool ClientCamera::onAdd()
{
   mNetFlags.clear(Ghostable | ScopeAlways);
   mNetFlags.set(IsGhost);
   if (!mDataBlock) return (false);
   Parent::onNewDataBlock(mDataBlock, false); //XXTH reload ?!?!

   if (!Parent::onAdd())
      return(false);
   return(true);
}
//---------------------------------------------------------------
#define MaxPitch 1.3962
// #define MaxPitch M_HALFPI_F
void ClientCamera::processTick(const Move* move)
{
   //manual yaw and pitch 
   Move clMove;

   if (mClientTS && mClientTS->haveMove())
   {
      clMove = mClientTS->getMove();
      Parent::processTick(&clMove);
      return;
   } else if (mPitch || mYaw)
   {
      EulerF rot = mRot;
      Point3F pos = getPosition();
      rot.x = mClampF(rot.x + mPitch, -MaxPitch, MaxPitch);
      rot.z += mYaw;

      _setPosition(pos, rot);

      mPitch = 0;
      mYaw = 0;

   }
  

}

void ClientCamera::interpolateTick(F32 dt)
{
   ShapeBase::interpolateTick(dt);
   Point3F rot = mDelta.rot + mDelta.rotVec * dt;

   if (mMode == OrbitObjectMode || mMode == OrbitPointMode)
   {
      if (mMode == OrbitObjectMode && bool(mOrbitObject))
      {
         // If this is a shapebase, use its render eye transform
         // to avoid jittering.
         GameBase* castObj = mOrbitObject;
         ShapeBase* shape = dynamic_cast<ShapeBase*>(castObj);
         if (shape != NULL)
         {
            MatrixF ret;
            shape->getRenderEyeTransform(&ret);
            mPosition = ret.getPosition();
         }
         else
         {
            // Hopefully this is a static object that doesn't move,
            // because the worldbox doesn't get updated between ticks.
            mOrbitObject->getWorldBox().getCenter(&mPosition);
         }

      }
      _setRenderPosition(mPosition + mOffset, rot);
      _validateEyePoint(1.0f, &mRenderObjToWorld);

   }
   else
   {
      Point3F pos = mDelta.pos + mDelta.posVec * dt;

//      _setPosition(mPosition, rot);
      //_setRenderPosition(mPosition , rot); //2023-12-12
      //???? not in TGE : 
      _validateEyePoint(1.0f, &mRenderObjToWorld);
   }
}

DefineEngineStringlyVariadicMethod(ClientCamera, setControlClientTS, void, 3, 3, "(ClientTSCTRL object)")
{

   ClientTSCtrl* gb;
   if (!Sim::findObject(argv[2], gb))
   {
      object->setClientTS(NULL);
      return;
   }

   object->setClientTS(gb);

}
