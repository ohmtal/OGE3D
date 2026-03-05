//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// ClientTSCtrl for serverless rendering
// Created by Ohmtal Game Studio 2007~202X
//-----------------------------------------------------------------------------
#ifndef _ClientTSCtrl_H_
#define _ClientTSCtrl_H_

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _MOVEMANAGER_H_
#include "T3D/gameBase/moveManager.h"
#endif

#ifndef _GUITSCONTROL_H_
#include "gui/3d/guiTSControl.h"
#endif

#ifndef _COLLISION_H_
#include "collision/collision.h"
#endif


class ClientCamera;

class ClientTSCtrl : public GuiTSCtrl //, public GuiMouseEventCtrl
{

   typedef GuiTSCtrl Parent;


protected:
   // ClientTSCtrl
   void make3DMouseEvent(Gui3DMouseEvent& gui3Devent, const GuiEvent& event);
   void getCursor(GuiCursor*& cursor, bool& showCursor, const GuiEvent& lastGuiEvent);

   void onMouseUp(const GuiEvent& event);
   void onMouseDown(const GuiEvent& event);
   void onMouseMove(const GuiEvent& event);
   void onMouseDragged(const GuiEvent& event);
   void onMouseEnter(const GuiEvent& event);
   void onMouseLeave(const GuiEvent& event);
   void onRightMouseDown(const GuiEvent& event);
   void onRightMouseUp(const GuiEvent& event);
   void onRightMouseDragged(const GuiEvent& event);
   void onMiddleMouseDown(const GuiEvent& event);
   void onMiddleMouseUp(const GuiEvent& event);

   bool onInputEvent(const InputEventInfo& event);


   //      void selection(const GuiEvent & event);
   bool hittest(const GuiEvent& event, RayInfo& ri);

   virtual void updateGuiInfo() {};
   virtual void renderScene(const RectI&) {};
   //void renderMissionArea();


   // GuiTSCtrl
   void renderWorld(const RectI& updateRect);

protected:
   Gui3DMouseEvent mLastEvent;

   GFXStateBlockDesc mSelectedCubeDesc;

   Point3F             mLastMouseRayPoint;
   Point3F             mLastMouseRayNormal;


   ClientCamera* mCamera;

   bool      mEnableMovement;
   bool		getNextMove(Move& curMove);


   U32		mMouseHitMask;

   SceneObject* mSelectedObj;
   /*
         //XXTH iTickable interface
         virtual void interpolateTick( F32 delta );
         virtual void processTick();
         virtual void advanceTime( F32 timeDelta );
   */
private:
   void sendMouseEvent(const char* name, const GuiEvent& event);

   //void renderObjectBox(SceneObject* obj, const ColorI& col = ColorI(255, 255, 0));
   
public:

   ClientTSCtrl();
   ~ClientTSCtrl();

   // SimObject
   bool onAdd();


   static void initPersistFields();

   static void consoleInit();

   //
   bool              mRightMousePassThru;
   bool				mMiddleMousePassThru;

   // all editors will share a camera
   Point3F    smCamPos;
   EulerF     smCamRot;
   MatrixF    smCamMatrix;
   F32        smVisibleDistance;

   F32 mCamOffset; //for camera observer

   // GuiTSCtrl
   bool processCameraQuery(CameraQuery* query);

   void setCamera(ClientCamera* lCamera) { mCamera = lCamera; }
   ClientCamera* getCamera() { return mCamera; }

   void setEnableMovement(bool lEnable) { mEnableMovement = lEnable; }
   bool haveMove() { return (mEnableMovement); }
   Move getMove();

   void setMouseHitMask(U32 lMask) { mMouseHitMask = lMask; }
   U32 getMouseHitMask() { return mMouseHitMask; }

   void setSelectedObject(SceneObject* lObj) { mSelectedObj = lObj; }
   SceneObject* getSelectedObject() { return mSelectedObj; }

   // guiControl
   virtual void onRender(Point2I offset, const RectI& updateRect);

   virtual void on3DMouseUp(const Gui3DMouseEvent&) {};
   virtual void on3DMouseDown(const Gui3DMouseEvent&) {};
   virtual void on3DMouseMove(const Gui3DMouseEvent&) {};
   virtual void on3DMouseDragged(const Gui3DMouseEvent&) {};
   virtual void on3DMouseEnter(const Gui3DMouseEvent&) {};
   virtual void on3DMouseLeave(const Gui3DMouseEvent&) {};
   virtual void on3DRightMouseDown(const Gui3DMouseEvent&) {};
   virtual void on3DRightMouseUp(const Gui3DMouseEvent&) {};
   virtual void on3DRightMouseDragged(const Gui3DMouseEvent&) {};
   virtual void get3DCursor(GuiCursor*& cursor, bool& visible, const Gui3DMouseEvent&);


   Point3F getLastMouseRayPoint() { return  mLastMouseRayPoint; }
   Point3F getLastMouseRayNormal() { return  mLastMouseRayNormal; }

   DECLARE_CONOBJECT(ClientTSCtrl);
};



#endif
