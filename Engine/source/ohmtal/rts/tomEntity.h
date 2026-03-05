
#ifndef _TOMENTITY_H
#define _TOMENTITY_H


#ifndef _AIENTITY_H_
#include "ohmtal/ai/aiEntity.h"
#endif



#ifndef _AFX_MAGIC_SPELL_H_
#include "afx/afxMagicSpell.h"
#endif

#ifndef _SIMPATH_H_
#ifdef TORQUE_SHADERGEN
#include "scene/simPath.h"
#else
#include "sim/simPath.h"
#endif
#endif


class ToMWave;
//------------------------------------------------------------------------------------------------------------
struct ToMEntityParams
{



   //Consum - when build it's reduced
	//need money to build 
   U32 c_money;
   U8 c_unit; //from houses
   U8 c_food; //from farms

   U8 c_field; //from fields
   U8 c_craft; //from blacksmith 
   U8 c_lore;  //from school 


   //Add Stuff - Money per Round or Kill or units
   S32 a_money; 
   //Add per Build
   U8 a_unit; 
   U8 a_food;  

   U8 a_field;   
   U8 a_craft; 
   U8 a_lore;


   //------------------------------
   void init() {
      c_money  = 0;
	  c_unit   = 0;
	  c_food   = 0;
      c_field  = 0;
	  c_craft  = 0;
	  c_lore   = 0;

	  a_money  = 0;
	  a_unit   = 0;
	  a_food   = 0;
	  a_field  = 0;
	  a_craft  = 0;
	  a_lore   = 0;

   }
};


#define TOM_MAXTOWERUPGRADES 20
enum TowerUpgradeType 
{
	  TowerDamage
	, TowerRange
	, TowerAttackDelay
	, MAX_TowerUpgradeType

};

//------------------------------------------------------------------------------------------------------------
class ToMEntityData : public PlayerData
{
   typedef PlayerData Parent;

public:
   StringTableEntry  mDisplayname;
   StringTableEntry  mMinipic;
   StringTableEntry  mHint;
   StringTableEntry  mDummy;
   char*  mBaseTex;


   S32 mBaseDamage;
   S32 mAttackDelay;
   S32 mDamagePlus;

   S32 mArmor;

   F32 mMoveSpeed;
   F32 mRange;
   F32 mVision;
   F32 mAttackRange;

   F32 mRingScale;
   F32 mMaxRotateRadiants;


   S32    mEntityId; 
   bool   mIsBuilding;
   bool   mIsTower;
   char*  mTowerShootSpell; //Name of spell.. spell object will be looked up in ToMHandler::onTowerTarget
   afxMagicSpellData* mCachedTowerShootSpell;

   char*  mLingerSpell; //Name of spell.. spell object will be looked up when object is created
   afxMagicSpellData* mCachedLingerSpell;


   S32 mStartDamage;
   S32 mStartAttackDelay; //in milisec! => lDb->mAttackDelay = (F32)lAttackDelay * 0.032f;
   F32 mStartRange;

   S32 mTowerAddDamage;
   S32 mTowerReduceDelay;
   F32 mTowerAddRange;
   F32 mUnitMaxDamageQuotient;

   ToMEntityData();

   static void initPersistFields();

   void updateUnitLevel(U8 lLevel);
   bool updateTowerLevel(TowerUpgradeType lType); 
   void updateTowerLevel(TowerUpgradeType lType, U8 lLevel); 
   

   void buildHint();


   //Build requirements
   ToMEntityParams mParams;

   U8 mMaxUpgrades; 
   void setMaxUpgrades(U8 lMax )
   {
	   if (lMax > TOM_MAXTOWERUPGRADES)
		   mMaxUpgrades = TOM_MAXTOWERUPGRADES;
	   else
		   mMaxUpgrades = lMax;
   }

   ToMEntityParams mUpgradeCosts[MAX_TowerUpgradeType][TOM_MAXTOWERUPGRADES];
   ToMEntityParams* getUpgradeParams(TowerUpgradeType lType);
  
   void setUpgradeCostsMoney(U8 lLevel, U32 lTowerDamage, U32 lTowerRange, U32 lTowerAttackDelay)
   {
	  mUpgradeCosts[TowerDamage][lLevel].c_money      = lTowerDamage;
      mUpgradeCosts[TowerRange][lLevel].c_money		  = lTowerRange;
      mUpgradeCosts[TowerAttackDelay][lLevel].c_money = lTowerAttackDelay;
   }
   void setUpgradeCostsLore(U8 lLevel, U8 lLore)
   {
	  mUpgradeCosts[TowerDamage][lLevel].c_lore		 = lLore;
      mUpgradeCosts[TowerRange][lLevel].c_lore		 = lLore;
      mUpgradeCosts[TowerAttackDelay][lLevel].c_lore = lLore;
   }

   U8 mTowerLevel[MAX_TowerUpgradeType];
   U8 mUnitLevel;


//   char* getDisplayInfo();
   /*
   virtual void packData(BitStream* stream);
   virtual void unpackData(BitStream* stream);
   */

   static const char* staticGetIsBuilding(void* obj, const char* data) {return static_cast<ToMEntityData*>(obj)->mIsBuilding ? "1" : "0";}
   static const char* staticGetMinipic(void* obj, const char* data) {return static_cast<ToMEntityData*>(obj)->mMinipic;}
   static const char* staticGetDisplayName(void* obj, const char* data) {return static_cast<ToMEntityData*>(obj)->mDisplayname;}
   static const char* staticGetHint(void* obj, const char* data) {return static_cast<ToMEntityData*>(obj)->mHint;}
//   static const char* staticGetDisplayInfo(void* obj, const char* data) {return static_cast<ToMEntityData*>(obj)->getDisplayInfo();};
   static bool setDummy(void* obj, const char* data) {   return false;  }

   DECLARE_CONOBJECT(ToMEntityData);
};

//------------------------------------------------------------------------------------------------------------

class ToMEntity : public AIEntity
{
private:
   typedef AIEntity Parent;
public:
	ToMEntity();
	~ToMEntity();

	bool onNewDataBlock(GameBaseData* dptr, bool reload);
protected:
	ToMEntityData *mDataBlock;
	virtual void updateDamageState();
//    static void insertionNeighbourCallback(SceneObject* obj, void *key);
//    virtual void scanNeighbours();

private:
      
      S32 mAttackDelayCounter;
	  bool mMoveBlocked;
      bool mOverwriteAnim;
	  S32	action_walk,
			action_idle,
			action_attack,
			action_death;


	  F32 getDistance(GameBase* a);
	  bool ObjectInLos(ShapeBase* lObj, U32 mask = PlayerObjectType | StaticShapeObjectType  );

	  bool  mIsPathWalking;
      SimPath::Path* mPathObject;
	  S32   mCurPathNodeIndex;
	  bool  MoveToNextWayPoint();

	  ToMWave* mWaveManager;

public:
	DECLARE_CONOBJECT( ToMEntity );
	static void initPersistFields();
	bool onAdd();
	virtual void pickActionAnimation();
	void initAnimations();
	void GuessActionAnimation();
    virtual bool setActionThread(S32 action,  bool setOverwriteAnim = false, bool moveBlocked = false);
	bool setActionThread(const char* sequence, bool setOverwriteAnim = false, bool moveBlocked = false);

	void stopMove();
	void setMoveDestination( const Point3F &location );
    U32  packUpdate  (NetConnection *conn, U32 mask, BitStream *stream);
	void unpackUpdate(NetConnection *conn,           BitStream *stream);
	void updateActionThread();
	void updateAnimation(F32 dt);
	virtual void doCheckTarget(Player* target,Point3F location);
	virtual void doLook(VectorF lTargetLocation,Point3F &location, Point3F &rotation);
	virtual void dropToTerrain(Point3F &location);

 
	void setOverwriteAnim(bool lVal) { mOverwriteAnim=lVal; }

	bool getIsTower() { return mDataBlock->mIsTower; }
	void processTower();

	void setPathObject(SimPath::Path* lPath);
	bool getIsPathWalking()  { return mIsPathWalking; }
	void startPath();
	void stopPath();

	void setWaveManager(ToMWave* lWave) { mWaveManager = lWave; }
	ToMWave* getWaveManager() {return mWaveManager; }


   S32 getActionbyName(const char* sequence);

	void processTick(const Move *move);
protected:
	virtual void doMove(VectorF lTargetLocation,Point3F &location);
	virtual void beginProcessTick(const Move *move);
//FIXME placement rendering	void renderObject(SceneRenderState* state, SceneRenderImage* image);

};
#endif //_TOMENTITY_H
