//-----------------------------------------------------------------------------
// Ohmtal Game Engine
//-----------------------------------------------------------------------------
// MyCreature2D public tom2DSprite
//-----------------------------------------------------------------------------

#ifndef _MYCREATURE_H_
#define _MYCREATURE_H_

#ifndef _TOM2D_H_
#include "tom2DCtrl.h"
#endif
#ifndef _TOM2DSPRITE_H_
#include "tom2DSprite.h"
#endif

//=================================================================================================
// MyCreature2D
//=================================================================================================
class MyCreature2D : public tom2DSprite
{
private:
	typedef tom2DSprite Parent;

protected:
	S32 mFaction;
	S32 mSex;
	F32 mAge;
	F32 mMaxAge;
	F32 mStrength;
	F32 mMaxStrength;
	F32 mHealth;
	F32 mMaxHealth;
	F32 mFittness;
	F32 mMaxFittness;
	bool mSelected;


   //did we change a parameter which should call updateStuff
   S32 mChangeMask;
   

	S32 mStateId; //we use S32 as state since statemachine is scripted at the moment 
	S32 mTargetId; //id of the target


   ColorI mInnerColor;
   ColorI mOuterColor;

public:
	DECLARE_CONOBJECT(MyCreature2D);
	MyCreature2D();
	~MyCreature2D();
	static void initPersistFields();
   void updateStuff();

   virtual void onUpdate(F32 fDt);
	virtual void onRender(U32 dt, Point3F lOffset);
   void renderPrim();
   void renderBatch();


protected:
   void setChangeMask();
   S32 getChangeMask() { return mChangeMask; }
   bool needUpdateStuff();
};
#endif //_MYCREATURE_H_
