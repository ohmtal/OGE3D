//-----------------------------------------------------------------------------
// Copyright (c) 2009 huehn-software / Ohmtal Game Studio
// Copyright (c) 2012 GarageGames, LLC
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

#ifndef _OHMTSCTRL_H_
#define _OHMTSCTRL_H_

#ifndef _GUITSCONTROL_H_
#include "gui/3d/guiTSControl.h"
#endif

#ifndef _COLLISION_H_
#include "collision/collision.h"
#endif

#ifndef _ITICKABLE_H_
#include "core/iTickable.h"
#endif



class ShapeBase;

//----------------------------------------------------------------------------
class ohmTSCtrl : public GuiTSCtrl, public virtual ITickable {
private:
   typedef GuiTSCtrl Parent;


	// cursor constants
	enum {
		DefaultCursor = 0,
		HandCursor,
		UseCursor,
		MoveCursor,
		//
		NumCursors
	};

	Point3F	smCamPos;
	Gui3DMouseEvent mLastEvent;
    Point2I	mLastCursor;


	bool doUpdate;	// Limits the update rate to the tick rate
	bool mCanDrag;	// Allows dragging only if MouseDown was on this object
	bool mRightMouseDragged; // we did drag...
	ShapeBase* mTrackObject;	// Last mouse-down object
	ShapeBase* mCursorObject;	// The object under cursor

	ShapeBase* mSelectedObject;	// The object which is selected!

    Point3F             mMouse3DVec;
    Point3F             mMouse3DPos;

    Point3F             mLastMouseRayPoint;
    Point3F             mLastMouseRayNormal;


	void sendMouseEvent(const char * name, const GuiEvent &);

   bool mUseMouseMovement;
   F32  mMouseMovementMultiplier; //added 2025-01-22 default 0.1f
   bool mLockMouseToWindow; //capture mouse to screen

   bool mEdgeMouseMovement; //we send event on edge require  mLockMouseToWindow !
   enum PanEdge {
      EdgeNone = BIT(0),
      EdgeLeft = BIT(1),
      EdgeRight = BIT(2),
      EdgeTop = BIT(3),
      EdgeBottom = BIT(4)
   };
   U32 mPrevEdge;


protected:
	Point2I mMousePos;
	U32		mMouseHitMask; //V2.18
	bool onInputEvent(const InputEventInfo& event); //XXTH from tom2D

    // These three methods are the interface for ITickable
    virtual void interpolateTick( F32 delta);
    virtual void processTick( void);
    virtual void advanceTime( F32 timeDelta);

	
public:
	F32	mProjectDistance;	// Distance we can select things

	GuiCursor*	mCursors[NumCursors];
	GuiCursor*	mCurrentCursor;

	ohmTSCtrl( void);

	void setSelectedObject(ShapeBase* lObj);
	ShapeBase* getSelectedObject();
	void clearSelectedObject();
	void sendSelectionEvent(const char * name, ShapeBase* lObj = NULL);

   static void initPersistFields();

	bool processCameraQuery( CameraQuery *query);
	void renderWorld( const RectI &updateRect);
	void onMouseDown( const GuiEvent &event);
    void onMouseDragged( const GuiEvent &event);
	void onMouseUp( const GuiEvent &event);
	void onMouseMove( const GuiEvent &evt);
    void onMiddleMouseDown(const GuiEvent& event);
    void onMiddleMouseUp(const GuiEvent& event);
    void onRightMouseDown( const GuiEvent &event);
	void onRightMouseUp( const GuiEvent &event);
    void onRightMouseDragged( const GuiEvent &event);
    void onMouseEnter(const GuiEvent &event);
    void onMouseLeave(const GuiEvent &event);


	void onRender( Point2I offset, const RectI &updateRect);
	//XXTH 
	bool onWake();
    void onSleep();

	//XXTH V2.18
	bool hittest(const GuiEvent & event, RayInfo & ri);
    void setMouseHitMask(U32 lMask) { mMouseHitMask = lMask; }
	U32 getMouseHitMask() { return mMouseHitMask; }


	DECLARE_CONOBJECT(ohmTSCtrl);

	bool onAdd( void);

	bool grabCursors( void);
	void setCursor( U32 cursor);

	void make3DMouseEvent( Gui3DMouseEvent &gui3DMouseEvent, const GuiEvent &event);
	void on3DMouseMove( const Gui3DMouseEvent &event);
   void on3DMouseDown(const Gui3DMouseEvent& event);
   void on3DMouseUp(const Gui3DMouseEvent& event);
   void on3DMouseDragged(const Gui3DMouseEvent& event);
   void on3DMouseEnter(const Gui3DMouseEvent& event);
   void on3DMouseLeave(const Gui3DMouseEvent& event);
   void on3DRightMouseDown(const Gui3DMouseEvent& event);
   void on3DRightMouseUp(const Gui3DMouseEvent& event);




	ShapeBase* collide( const Gui3DMouseEvent &event);

    Point3F             getMouse3DVec() {return mMouse3DVec;};   
    Point3F             getMouse3DPos() {return mMouse3DPos;};

private:
	bool mAutoSelectObject; // autoselect or let the script decide = false (default)
	bool mUseObjectSelectleft;  //enable this to select(highlight) (onselect) on left click, else onclick only will be pushed
	bool mUseObjectSelectright; //enable this to select (highlight) (onselect) on right click, else onclick only will be pushed

   //------------------ drag selecttion 
   bool        mEnableDragSelect;
   bool        mDragSelect;
   RectI       mDragRect;
   ColorI      mDragRectColor;
   Point2I     mDragStart;
   void        queryDragSelect();

public:
   Point3F getLastMouseRayPoint()  { return  mLastMouseRayPoint; }
   Point3F getLastMouseRayNormal() { return  mLastMouseRayNormal; }

}; //class

#endif //#ifndef _OHMTSCTRL_H_
