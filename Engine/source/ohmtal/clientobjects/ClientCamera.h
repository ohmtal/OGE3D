//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#ifndef _ClientCamera_H_
#define _ClientCamera_H_

#ifndef _ClientTSCtrl_H_
#include "ClientTSCtrl.h"
#endif
#ifndef _CAMERA_H_
#include "T3D/camera.h"
#endif
//=================================================================================================
class ClientCamera : public Camera 
{
private:
   typedef Camera Parent;
	ClientTSCtrl* mClientTS;

	F32 mYaw,mPitch;

public:
	ClientCamera() {
      mYaw = 0.f;
      mPitch = 0.f;
      mClientTS = NULL;
   }
   bool onAdd();
   static void initPersistFields();
   void processTick(const Move* move);
   void interpolateTick(F32 dt);
   
	void setClientTS(ClientTSCtrl* lClientTS) { 
	   if (lClientTS) 
		   lClientTS->setEnableMovement(true);
	   else
		   if ( mClientTS ) 
				mClientTS->setEnableMovement(false);

  	   mClientTS = lClientTS; 

	};
   

   DECLARE_CONOBJECT(ClientCamera);
};


//=================================================================================================


#endif
