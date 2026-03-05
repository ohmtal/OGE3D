//-----------------------------------------------------------------------------
// FX GrassObject
//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------

#ifndef _fxGrassObject_H_
#define _fxGrassObject_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif


#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif


#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

#ifndef _TERRDATA_H_
#include "terrain/terrData.h"
#endif



/* added to mConstans.h
#define M_PIDIV180			0.01745329251994329576923690768488f		// PI / 180
#define RADIANS(a)	((a)*M_PIDIV180)
*/
//------------------------------------------------------------------------------
// Class: fxGrassObject
//------------------------------------------------------------------------------
class fxGrassObject : public SceneObject
{
private:
	typedef SceneObject		Parent;

protected:


	enum {	fxGrassObjectMask		= (1 << 0),
			fxGrassObjectAnother	= (1 << 1) };

   GFXTexHandle					mTextureHandle;

	// Fields.
	F32 mMinWidth;
	F32 mMaxWidth;
	F32 mMinHeight;
	F32 mMaxHeight;

	F32 mOffsetZ;
	bool mRandomFlip;
	F32	mSwayMulti;

   bool mFlat;
   bool mFlatGround;
   S32  mSeed;


    StringTableEntry				mTextureName;

	// VBO
    // GFXVertexBufferHandle<GFXVertexPCT>		mVBO;
    
    Vector< GFXVertexBufferHandle<GFXVertexPNT>* > mVBO;

	S32 mCount;
	F32 mLodDistance; 
	bool mIsSquare;
	F32 mInnerRadX;
	F32 mOuterRadX;
	F32 mInnerRadY;
	F32 mOuterRadY;
   bool mOnlyInBoxZ;

	S32			   mMaterialGrassIndex;

   bool            mAllowOnTerrain;
   bool            mAllowOnInteriors;
   bool            mAllowStatics;
   bool            mAllowUnderWater;
   bool            mAllowOnlyUnderWater;
   bool            mAllowWaterSurface;


	F32 mDistance;
	F32 mObjBoxZ;
	void updateWorldBox();


	MRandomLCG              RandomGen;
	F32						mLastCheckSum;

	TerrainBlock* mTerrain;  //XXTH
//	MaterialPropertyMap* mMatMap; //XXTH


	F32 mRatio;
	

   //2.30 replaced with gEditingMission ! bool	mClientReplicationStarted;

public:
	fxGrassObject();
	~fxGrassObject();

	bool initVBO();

	// SceneObject


   virtual void prepRenderImage(SceneRenderState* state);
   void renderObject(SceneRenderState* state);


	// SimObject      
	bool onAdd();
	void onRemove();
	void onEditorEnable();
	void onEditorDisable();
	void inspectPostApply();

	// NetObject
	U32 packUpdate(NetConnection *, U32, BitStream *);
	void unpackUpdate(NetConnection *, BitStream *);

	// ConObject.
	static void initPersistFields();

	// Declare Console Object.
	DECLARE_CONOBJECT(fxGrassObject);

	
};


#endif // _fxGrassObject_H_
