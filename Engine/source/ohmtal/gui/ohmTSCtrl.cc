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

#include "console/consoleTypes.h"
#include "T3D/gameBase/gameBase.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "T3D/gameFunctions.h"
#include "console/engineAPI.h"
#include "sim/actionMap.h" //for inputevent!
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"

#include "ohmTSCtrl.h"






//----------------------------------------------------------------------------
// Class: ohmTSCtrl
//----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(ohmTSCtrl);

ohmTSCtrl::ohmTSCtrl() {
	mProjectDistance = 70.f;	// Distance we can select things

	mCursorObject = NULL;	// Last mouse-down object
	mTrackObject = NULL;		// The object under cursor
	
	doUpdate = false;	// Limits the update rate to the tick rate
	mCanDrag = false;	// Allows dragging only if MouseDown was on this object
    mRightMouseDragged = false; //why was it true?
    

    setProcessTicks(true);// Enables processTick calls

    mMouse3DVec.zero();
    mMouse3DPos.zero();

    mLastMouseRayPoint.zero();
    mLastMouseRayNormal.zero();


	mAutoSelectObject       = false;  // autoselect or let the script decide = false (default)
	mUseObjectSelectleft    = false;  //enable this to select(highlight) (onselect) on left click, else onclick only will be pushed
	mUseObjectSelectright   = true;   //enable this to select (highlight) (onselect) on right click, else onclick only will be pushed

	mSelectedObject = NULL;


	mMousePos=Point2I(-1,-1);

	mMouseHitMask = GameBaseObjectType | StaticObjectType | PlayerObjectType | TerrainObjectType |  InteriorLikeObjectType | TerrainLikeObjectType;

   mUseMouseMovement = true;
   mMouseMovementMultiplier = 0.1f; //how fast move the cam on right mouse dragged
   mLockMouseToWindow = false;

   mEdgeMouseMovement = false;
   mPrevEdge = EdgeNone;

   //------------------ drag selecttion 
   mEnableDragSelect = false;
   mDragSelect       = false;
   mDragRect         = RectI(0,0,0,0);
   mDragRectColor    = ColorI(0, 255, 0);
   mDragStart        = Point2I(0,0);
   //mDragObjectSet    = new SimSet();
   //mDragObjectSet->registerObject();






}


void ohmTSCtrl::initPersistFields()
{

   Parent::initPersistFields();
   addGroup("MouseMovement");
      addField("UseMouseMovement", TypeBool, Offset(mUseMouseMovement, ohmTSCtrl), "Right mouse click can be used to lock cursor and move the cam");
      addField("MouseMovementMultiplier", TypeF32, Offset(mMouseMovementMultiplier, ohmTSCtrl), "Scale the movement speed mouse cursor to 3D world. (default 0.1)");
      addField("LockMouseToWindow", TypeBool, Offset(mLockMouseToWindow, ohmTSCtrl), "Lock the mouse cursor to the screen(RTS Style)");
      addField("EdgeMouseMovement", TypeBool, Offset(mEdgeMouseMovement, ohmTSCtrl), "Edge movement (RTS Style) require LockMouseToWindow");
   endGroup("MouseMovement");

   addGroup("Selection");
      addField("EnableDragSelect",       TypeBool, Offset(mEnableDragSelect, ohmTSCtrl), "Enable RTS like scale with a rectangle");
      addField("DragRectColor",          TypeColorI, Offset(mDragRectColor, ohmTSCtrl), "Color of the selection rectangle");
      
      addField("AutoSelectObject" ,      TypeBool, Offset(mAutoSelectObject,    ohmTSCtrl), "Autoselect on left click? FIXME find out what it does.");
      addField("UseObjectSelectleft" ,   TypeBool, Offset(mUseObjectSelectleft,    ohmTSCtrl), "left click to select an object");
      addField("UseObjectSelectright",   TypeBool, Offset(mUseObjectSelectright,   ohmTSCtrl), "right click to select an object");
   endGroup("Selection");
}

//---------------------------------------------------------------------------
bool ohmTSCtrl::processCameraQuery( CameraQuery *camq) {
	GameUpdateCameraFov();
	bool ret = GameProcessCameraQuery( camq);

	// Record the camera's position
	camq->cameraMatrix.getColumn( 3, &smCamPos);


	return ret;
}

//---------------------------------------------------------------------------
void ohmTSCtrl::renderWorld( const RectI &updateRect)
{
   GameRenderWorld();

}
//---------------------------------------------------------------------------
void ohmTSCtrl::make3DMouseEvent(Gui3DMouseEvent & gui3DMouseEvent, const GuiEvent & evt) {
	(GuiEvent&)(gui3DMouseEvent) = evt;

	// get the eye pos and the mouse vec from that...
	Point3F sp( evt.mousePoint.x, evt.mousePoint.y, 1);

	Point3F wp;
	unproject( sp, &wp);

	gui3DMouseEvent.pos = smCamPos;
	gui3DMouseEvent.vec = wp - smCamPos;
	gui3DMouseEvent.vec.normalize();
}

//---------------------------------------------------------------------------


void ohmTSCtrl::onMouseDown( const GuiEvent &evt)
{


	// Get a new Cursor Object
	make3DMouseEvent( mLastEvent, evt);
	on3DMouseDown( mLastEvent);

    //Set Variables for console callback:
	mMouse3DPos = mLastEvent.pos;
    mMouse3DVec = mLastEvent.vec;



	// New Track Object
	mTrackObject = (ShapeBase*)mCursorObject;
	
	if (mTrackObject) {
		setCursor( UseCursor);
	}

	sendMouseEvent("onMouseDown", evt);
}
//--------------------------------------------------------------------------------------
void ohmTSCtrl::onMouseUp( const GuiEvent &evt)
{

	make3DMouseEvent( mLastEvent, evt);
	on3DMouseUp( mLastEvent);

	if (mUseObjectSelectleft) { //XXTH
	  if (mTrackObject) {
		  // Select only objects we've tracked
		  if (mTrackObject == mCursorObject) {
			  if (mAutoSelectObject)
				setSelectedObject(mTrackObject);
		  }  else {
			  if (mAutoSelectObject)
					clearSelectedObject();
			  sendSelectionEvent("notifyDeSelect",mTrackObject);
			  mTrackObject = NULL;
		  }
	  } else {
		  if (mAutoSelectObject)
			 clearSelectedObject();
		  if (mSelectedObject)
			sendSelectionEvent("notifyDeSelect");
	  }
	}	
    //allways call onclick :P
	  
	  if (mTrackObject) {
		  // Select only objects we've tracked
		  if (mTrackObject == mCursorObject)
			  sendSelectionEvent("onClickObject",mTrackObject);
	  }




	  // Restore last cursor position
	// Keep hand cursor if we've got a Cursor Object
   if (mCursorObject)
      setCursor( HandCursor);
   else if (mUseMouseMovement && mCanDrag) {
       //do nothing!
   } else
      setCursor( DefaultCursor);

   sendMouseEvent("onMouseUp", evt);
}
//---------------------------------------------------------------------------

void ohmTSCtrl::onMouseMove( const GuiEvent &evt) {

	if (doUpdate) {
		doUpdate = false;
	
		// Get a new Cursor Object
		make3DMouseEvent( mLastEvent, evt);
		on3DMouseMove( mLastEvent);

		// Restore Default Cursor if no selectable object under the cursor		
		if (!mCursorObject)
			setCursor( DefaultCursor);
	}
    sendMouseEvent("onMouseMove", evt);
}

//---------------------------------------------------------------------------

void ohmTSCtrl::onMiddleMouseDown(const GuiEvent& evt)
{

   // Get a new Cursor Object
   make3DMouseEvent(mLastEvent, evt);
   sendMouseEvent("onMiddleMouseDown", evt);

}
void ohmTSCtrl::onMiddleMouseUp(const GuiEvent& evt)
{

   // Get a new Cursor Object
   make3DMouseEvent(mLastEvent, evt);
   sendMouseEvent("onMiddleMouseUp", evt);

}
//---------------------------------------------------------------------------
void ohmTSCtrl::onRightMouseDown( const GuiEvent &evt)
{


   if (mUseMouseMovement) {
     //use mousepoint!!  mLastCursor    = getRoot()->getCursorPos(); //getRoot is guiCanvas!
      //THIS!!!! ==>>> 
      mLastCursor = evt.mousePoint;
      mCanDrag = true;
      mRightMouseDragged = false;
      
   }

	// Get a new Cursor Object
	make3DMouseEvent( mLastEvent, evt);
    on3DRightMouseDown( mLastEvent);

	// New Track Object
	mTrackObject = (ShapeBase*)mCursorObject;
	
	if (mTrackObject)
		setCursor( UseCursor);

    sendMouseEvent("onRightMouseDown", evt);


}
//---------------------------------------------------------------------------
void ohmTSCtrl::onRightMouseUp( const GuiEvent &evt) {


	make3DMouseEvent( mLastEvent, evt);
    on3DRightMouseUp( mLastEvent);

	if (mUseObjectSelectright && ! mRightMouseDragged ) { //XXTH
	  if (mTrackObject) {
		  // Select only objects we've tracked
		  if (mTrackObject == mCursorObject) {
			  if (mAutoSelectObject)
				setSelectedObject( mTrackObject);
			  
		  }  else {
			  if (mAutoSelectObject)
					clearSelectedObject();
			  sendSelectionEvent("notifyDeSelect",mTrackObject);
			  mTrackObject = NULL;
			  
		  }
	  } else {
		  if (mAutoSelectObject)
				clearSelectedObject();
		  if (mSelectedObject)
			sendSelectionEvent("notifyDeSelect");
	  }

	  //call onclick :P
	  if (mTrackObject) {
		  // Select only objects we've tracked
		  if (mTrackObject == mCursorObject)
			  sendSelectionEvent("onRightClickObject",mTrackObject);
	  }

	} //use selection ...	



	// Restore last cursor position
   if (mUseMouseMovement) {
      if (mCanDrag) {
         getRoot()->setCursorPos(mLastCursor);
         mCanDrag = false;
      }
   }

	
	// Keep hand cursor if we've got a Cursor Object
	if (mCursorObject)
		setCursor( HandCursor);
	else
		setCursor( DefaultCursor);

   if ( !mRightMouseDragged ) //2025-04-02 dont want mouse of after dragged
    sendMouseEvent("onRightMouseUp", evt);

    mRightMouseDragged = false;
}
//---------------------------------------------------------------------------
void ohmTSCtrl::onMouseDragged( const GuiEvent &event) {

   make3DMouseEvent(mLastEvent, event);
   on3DMouseDragged(mLastEvent);

   if (mUseMouseMovement && mCanDrag) {
      onRightMouseDragged(event);
   }
}
//---------------------------------------------------------------------------
void ohmTSCtrl::onRightMouseDragged( const GuiEvent &evt)
{
//	onMouseDragged( evt);

   if (mUseMouseMovement) {
      if (mCanDrag) {

         InputEventInfo lInputEvent = getRoot()->getLastInputEvent();


         static const char* argv[2];
         // should better to use canvasPos!!!
         Point2I diff(evt.mousePoint - mLastCursor);


   // Use Move Cursor if moving
 //XXTH 24 orig:        if ((mAbs(diff.y) > 1 || mAbs(diff.x) > 1)) {
         if ((mAbs(diff.y) > 0 || mAbs(diff.x) > 0)) {
            setCursor(MoveCursor);
            mRightMouseDragged = true;
            // If you don't want to continue tracking, use the following:
            // mTrackObject = NULL;

            //<< moved here 2025-01-22
            // also set ""* 0.2f" seams much better ?!


            // Perform script-based yaw and pitch callbacks
            if (mAbs(diff.x) > 0)
            {
              argv[0] = "yaw";
              argv[1] = Con::getFloatArg((F32) diff.x * mMouseMovementMultiplier);
              Con::execute(2, argv);
            }
            if (mAbs(diff.y) > 0) {
              argv[0] = "pitch";
              argv[1] = Con::getFloatArg((F32)  diff.y * mMouseMovementMultiplier);
              Con::execute(2, argv);
            }



            getRoot()->setCursorPos(mLastCursor);

         }
/*
 * moved inside if mAbs
         // Perform script-based yaw and pitch callbacks
         if (diff.x)
         {
            argv[0] = "yaw";
            argv[1] = Con::getFloatArg(diff.x);
            Con::execute(2, argv);
         }
         if (diff.y) {
            argv[0] = "pitch";
            argv[1] = Con::getFloatArg(diff.y);
            Con::execute(2, argv);
         }


         //XXTH 25 was 1.f
         if (diff.len() > 1.f) { //DO NOT REMOVE !! XXTH
            getRoot()->setCursorPos(mLastCursor);
         }
*/
      }
   } //if (mUseMouseMovement) {

}
//---------------------------------------------------------------------------------------------------
bool ohmTSCtrl::onWake()
{
   if(!Parent::onWake())
      return(false);

   if (mLockMouseToWindow)
      Platform::setWindowLocked(true);


   return (true);
}
void ohmTSCtrl::onSleep()
{
   Parent::onSleep();

   //mLockMouseToWindow
   Platform::setWindowLocked(false);
}
//---------------------------------------------------------------------------------------------------


void ohmTSCtrl::onRender( Point2I offset, const RectI &updateRect)
{

   // check if should bother with a render
   GameConnection* con = GameConnection::getConnectionToServer();
   bool skipRender = !con || (con->getWhiteOut() >= 1.f) || (con->getDamageFlash() >= 1.f) || (con->getBlackOut() >= 1.f);

   if (!skipRender || true)
      Parent::onRender(offset, updateRect);

   //DragSelect 
   if (mEnableDragSelect && mDragSelect)
   {
      GFX->setClipRect(updateRect);
      //draw rect
      GFX->getDrawUtil()->drawRect(mDragRect, mDragRectColor);


   }

}
//---------------------------------------------------------------------------------------------------
bool ohmTSCtrl::grabCursors() {
	struct _cursorInfo {
		U32 index;
		const char * name;
	} infos[] = {
		{ DefaultCursor,	"DefaultCursor" },
		{ HandCursor,		"HandCursor" },
		{ UseCursor,		"UseCursor" },
		{ MoveCursor,		"MoveCursor" }
	};

	//
	for(U32 i = 0; i < (sizeof(infos) / sizeof(infos[0])); i++) {
		SimObject * obj = Sim::findObject(infos[i].name);
		if (!obj) {
			Con::errorf(ConsoleLogEntry::Script, "ohmTSCtrl::grabCursors: failed to find cursor '%s'.", infos[i].name);
			return(false);
		}

		GuiCursor *cursor = dynamic_cast<GuiCursor*>(obj);
		if(!cursor) {
			Con::errorf(ConsoleLogEntry::Script, "ohmTSCtrl::grabCursors: object is not a cursor '%s'.", infos[i].name);
			return(false);
		}

		//
		mCursors[infos[i].index] = cursor;
	}

	//
	mCurrentCursor = mCursors[DefaultCursor];
	
	return true;
}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::setCursor( U32 cursor) {
	AssertFatal(cursor < NumCursors, "ohmTSCtrl::setCursor: invalid cursor");

	mCurrentCursor = mCursors[cursor];
	getRoot()->setCursor(mCurrentCursor);
}
//---------------------------------------------------------------------------------------------------
ShapeBase* ohmTSCtrl::collide( const Gui3DMouseEvent &event) {
   GameConnection* conn = GameConnection::getConnectionToServer();
	if (!conn)
		return NULL; //false; XXTH cannot convert 'bool' to 'ShapeBase*' in return


	SceneObject* controlObj = conn->getControlObject();
	if (controlObj)
		controlObj->disableCollision();

	//
	Point3F startPnt = event.pos;
	Point3F endPnt = event.pos + event.vec * mProjectDistance;

	//
//   static U32 losMask = TerrainObjectType | InteriorLikeObjectType | TerrainLikeObjectType | ShapeBaseObjectType ;

    static U32 losMask = ShapeBaseObjectType;
	RayInfo ri;
	bool hit = gClientContainer.castRay( startPnt, endPnt, losMask, &ri);

	if (controlObj)
		controlObj->enableCollision();


/*
   if (hit) 
      Con::printf("HIT: %d  type:%d", ri.object->getId(), ri.object->getTypeMask());
*/

	if (hit && (ri.object->getTypeMask() & ShapeBaseObjectType))
		return (ShapeBase*)ri.object;
	else
		return NULL;
}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::queryDragSelect()
{
   static Frustum lDragFrustum;

   // Determine selected objects based on the drag box touching
   // a mesh if a drag operation has begun.
   if (mDragSelect && mDragRect.extent.x > 1 && mDragRect.extent.y > 1)
   {
      // Build the drag frustum based on the rect
      F32 wwidth;
      F32 wheight;
      F32 aspectRatio = F32(getWidth()) / F32(getHeight());

      if (!mLastCameraQuery.ortho)
      {
         wheight = mLastCameraQuery.nearPlane * mTan(mLastCameraQuery.fov / 2);
         wwidth = aspectRatio * wheight;
      }
      else
      {
         wheight = mLastCameraQuery.fov;
         wwidth = aspectRatio * wheight;
      }

      F32 hscale = wwidth * 2 / F32(getWidth());
      F32 vscale = wheight * 2 / F32(getHeight());

      F32 left = (mDragRect.point.x - getPosition().x) * hscale - wwidth;
      F32 right = (mDragRect.point.x - getPosition().x + mDragRect.extent.x) * hscale - wwidth;
      F32 top = wheight - vscale * (mDragRect.point.y - getPosition().y);
      F32 bottom = wheight - vscale * (mDragRect.point.y - getPosition().y + mDragRect.extent.y);
      lDragFrustum.set(mLastCameraQuery.ortho, left, right, top, bottom, mLastCameraQuery.nearPlane, mLastCameraQuery.farPlane, mLastCameraQuery.cameraMatrix);

      // If we're in first-person view currently, disable
      // hitting the control object.
      const bool isFirstPerson = GameConnection::getLocalClientConnection() ? GameConnection::getLocalClientConnection()->isFirstPerson() : false;
      if (isFirstPerson)
         GameConnection::getLocalClientConnection()->getControlObject()->disableCollision();

      // Find objects in the region.
      SimpleQueryList lSimpleQueryList;
      gClientContainer.findObjects(lDragFrustum, PlayerObjectType, SimpleQueryList::insertionCallback, &lSimpleQueryList);
      
      Con::executef(this, "onDragListStart");
      for (U32 i = 0; i < lSimpleQueryList.mList.size(); i++)
      {
         Con::executef(this, "onDragListObject", lSimpleQueryList.mList[i]->getId());
      }
      Con::executef(this, "onDragListEnd");
   }
}
//---------------------------------------------------------------------------------------------------
bool ohmTSCtrl::onAdd() {
	if (!Parent::onAdd())
		return false;

	// grab all the cursors
	if (!grabCursors())
		return false;

	return true;
}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::on3DMouseMove( const Gui3DMouseEvent & event)
{
	if (mCursorObject = collide( event)) {
		// Set Hand Cursor over selectable objects
		setCursor( HandCursor);
	}

   // See if we are on any of the edges or corners of the screen
   if (mEdgeMouseMovement)
   {
      Point2I localPoint = globalToLocalCoord(event.mousePoint);
      U32 edgeFlags = EdgeNone;
      if (localPoint.x <= getBounds().point.x + 5)
         edgeFlags |= EdgeLeft;
      if (localPoint.y <= getBounds().point.y + 5)
         edgeFlags |= EdgeTop;
      if (localPoint.x >= getBounds().extent.x - 5)
         edgeFlags |= EdgeRight;
      if (localPoint.y >= getBounds().extent.y - 5)
         edgeFlags |= EdgeBottom;

      // If the mouse is on an edge, send a command to scroll the display.
      if (edgeFlags & ~(EdgeNone))
      {
         Con::executef(this,"onMousePanDisplay", "1", Con::getIntArg(edgeFlags & ~(EdgeNone)));
         mPrevEdge |= edgeFlags;
      }
      else if (mPrevEdge & ~(EdgeNone))
      {
         Con::executef(this,"onMousePanDisplay", "0", Con::getIntArg(mPrevEdge & ~(EdgeNone)));
         mPrevEdge = EdgeNone;
      }
   }
}
//------------------------------------------------------------------------------
void ohmTSCtrl::on3DMouseDown(const Gui3DMouseEvent& event)
{
   if (mEnableDragSelect)
   {
      mouseLock();
      mDragSelect = false;
      //mDragObjectSet->clear();
      mDragRect.set(Point2I(event.mousePoint), Point2I(0, 0));
      mDragStart = event.mousePoint;
   }

   /* FIXME 
   if (mPlacingBuilding && mNewBuilding->getId())
      placeBuilding();
   */


}
//---------------------------------------------------------------------------------------------------
#define DRAG_PIXEL_TOLERANCE 8 //XXTH orig 4!
void ohmTSCtrl::on3DMouseDragged(const Gui3DMouseEvent& event)
{
   if (mEnableDragSelect)
   {
      if (!mDragSelect && (mDragStart.x + DRAG_PIXEL_TOLERANCE < event.mousePoint.x ||
         mDragStart.y + DRAG_PIXEL_TOLERANCE < event.mousePoint.y ||
         mDragStart.x < event.mousePoint.x + DRAG_PIXEL_TOLERANCE ||
         mDragStart.y < event.mousePoint.y + DRAG_PIXEL_TOLERANCE))
      {
         // The mouse has moved more than DRAG_PIXEL_TOLERANCE pixels since mouse down
         mDragSelect = true;
      }

      // based on drag selection scheme in the world editor
      //   see engine/editor/worldEditor.cc line 1946
      // Are we drag selecting?
      if (mDragSelect)
      {
         // Check to see if selection modifier is being pressed, if so clear selection.
         //WTF ? Con::executef(this, "checkDragSelectionModifier");

         // Drag selection is built in onRender, make sure here that there are no
         // negative extents.
         mDragRect.point.x = (event.mousePoint.x < mDragStart.x) ? event.mousePoint.x : mDragStart.x;
         mDragRect.extent.x = (event.mousePoint.x > mDragStart.x) ? event.mousePoint.x - mDragStart.x : mDragStart.x - event.mousePoint.x;
         mDragRect.point.y = (event.mousePoint.y < mDragStart.y) ? event.mousePoint.y : mDragStart.y;
         mDragRect.extent.y = (event.mousePoint.y > mDragStart.y) ? event.mousePoint.y - mDragStart.y : mDragStart.y - event.mousePoint.y;
         return;
      }

   }
}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::on3DMouseEnter(const Gui3DMouseEvent& event)
{
}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::on3DMouseLeave(const Gui3DMouseEvent& event)
{
}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::on3DRightMouseDown(const Gui3DMouseEvent& event)
{
   if (mEnableDragSelect)
   {
      mDragSelect = false;
   }

}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::on3DRightMouseUp(const Gui3DMouseEvent& event)
{
}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::on3DMouseUp(const Gui3DMouseEvent& event)
{
   if (mEnableDragSelect)
   {
      mouseUnlock();
      queryDragSelect();
      mDragSelect = false;
   }
}
//------------------------------------------------------------------------------
void ohmTSCtrl::onMouseEnter(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseEnter(mLastEvent);
}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::onMouseLeave(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseLeave(mLastEvent);

   //XXTH stop pandisplay!!!!!!!!!!!
   if (mPrevEdge & ~(EdgeNone))
   {
      Con::executef(this, "onMousePanDisplay", "0", Con::getIntArg(mPrevEdge & ~(EdgeNone)));
      mPrevEdge = EdgeNone;
   }

}
//------------------------------------------------------------------------------
// These three methods are the interface for ITickable
void ohmTSCtrl::interpolateTick( F32 delta) {
   if (!isAwake()) //2023-04-10
      return;

}

void ohmTSCtrl::processTick() {
   
   if (!isAwake()) //2023-04-10
      return;

	// Attempt to provide better performance by limiting the rate collisions are tested
	doUpdate = true;
	if (mUseMouseMovement && mCanDrag && getRoot() != nullptr && !getRoot()->getMouseControl()) {
		 mCanDrag=false;
	}

}
//---------------------------------------------------------------------------------------------------
void ohmTSCtrl::advanceTime( F32 timeDelta) {
   if (!isAwake()) //2023-04-10
      return;

}

//------------------------------------------------------------------------------
void ohmTSCtrl::setSelectedObject(ShapeBase* lObj)
{

	if (mSelectedObject != lObj) 
	{
		sendSelectionEvent("onSelectShape",lObj);
		mSelectedObject = lObj;

	}

}
//------------------------------------------------------------------------------
ShapeBase* ohmTSCtrl::getSelectedObject()
{
	return mSelectedObject;
}
//------------------------------------------------------------------------------
void ohmTSCtrl::clearSelectedObject()
{
	if (mSelectedObject) 
	{
		sendSelectionEvent("onDeSelectShape",mSelectedObject);
		mSelectedObject = NULL;

	}

}

//------------------------------------------------------------------------------
void ohmTSCtrl::sendSelectionEvent(const char * name, ShapeBase* lObj)
{
	
	if (lObj) 
		Con::executef(this,  name, lObj->getIdString());
	else
		Con::executef(this,  name, "");


}

//------------------------------------------------------------------------------
// new V2.18 (form clientTSCtrl)
bool ohmTSCtrl::hittest(const GuiEvent & event, RayInfo & ri) {

   S32 mouse_x = event.mousePoint.x;
   S32 mouse_y = event.mousePoint.y;


	// get the eye pos and the mouse vec from that...
	Point3F sp( event.mousePoint.x, event.mousePoint.y, 1);

	Point3F wp;
	unproject( sp, &wp);

	Gui3DMouseEvent gui3DMouseEvent;
	gui3DMouseEvent.pos = smCamPos;
	gui3DMouseEvent.vec = wp - smCamPos;
	gui3DMouseEvent.vec.normalize();

	mProjectDistance = 2000.f;

	Point3F startPnt = gui3DMouseEvent.pos;
	Point3F endPnt = gui3DMouseEvent.pos + gui3DMouseEvent.vec * mProjectDistance;

	// RayInfo ri;

	bool hit = gClientContainer.castRay( startPnt, endPnt, mMouseHitMask, &ri);

	if (!hit) 
	{
		ri.distance=0;
		ri.normal = Point3F(0.f,0.f,0.f);
		ri.point = Point3F(0.f,0.f,0.f);
	}

	//Con::printf("CASTRAY start: %f,%f,%f end: %f,%f,%f",startPnt.x,startPnt.y,startPnt.z,endPnt.x,endPnt.y,endPnt.z);

    return hit;
}
//------------------------------------------------------------------------------
void ohmTSCtrl::sendMouseEvent(const char * name, const GuiEvent & event)
{

   /* old style 
  char buf[5][32];
  dSprintf(buf[0], 32, "%d", event.modifier);
  dSprintf(buf[1], 32, "%d %d", event.mousePoint.x, event.mousePoint.y);
  dSprintf(buf[2], 32, "%d", event.mouseClickCount);
  if (mCursorObject) {
     dSprintf(buf[3], 32, "%d", mCursorObject->getId());
  } else {
     dSprintf(buf[3], 32, "%d", 0);
  }
   mMousePos = event.mousePoint;

   //V2.18
   RayInfo ri;
   if (hittest(event,ri)) 
		dSprintf(buf[4], 32, "%d", ri.object->getId());
   else
	   dSprintf(buf[4], 32, "%d", 0);

  char bigbuf[2][256];
  dSprintf(bigbuf[0], 256, "%f %f %f", ri.point.x,ri.point.y,ri.point.z);
  dSprintf(bigbuf[1], 256, "%f %f %f", ri.normal.x,ri.normal.y,ri.normal.z);

  mLastMouseRayPoint  = ri.point;
  mLastMouseRayNormal = ri.normal;


   Con::executef(this,  name, buf[0], buf[1], buf[2], buf[3], buf[4], bigbuf[0], bigbuf[1]);
   */


   RayInfo ri;
   bool lHit = hittest(event, ri);
   mLastMouseRayPoint = ri.point;
   mLastMouseRayNormal = ri.normal;

   Con::executef(this, name,
      event.modifier    //%modifier
      , Point2I(event.mousePoint.x, event.mousePoint.y) // %mousePoint
      , event.mouseClickCount //clickcount
      , (mCursorObject ? mCursorObject->getId() : 0)  //%cursorObject
      , (lHit ? ri.object->getId() : 0)  //%hitObject
      , ri.point  //%rayPoint
      , ri.normal //%rayNormal
   ); 


}
//------------------------------------------------------------------------------
// INPUT EVENT LIKE IN tom2D - WARNING! only works if firstresponder!
// canKeyFocus must be enabled in profile !! 
//------------------------------------------------------------------------------
bool ohmTSCtrl::onInputEvent(const InputEventInfo& event)
{
	/*
      if ( event.objType == SI_BUTTON
        || event.objType == SI_POV
        || ( ( event.objType == SI_KEY ) && !isModifierKey( event.objInst ) ) )
    */ if (true)
      {

         char deviceString[32];
         if ( !ActionMap::getDeviceName( event.deviceType, event.deviceInst, deviceString ) )
            return( false );

         const char* actionStrP = ActionMap::buildActionString( &event );
		 char actionString[256];
		 dSprintf( actionString, 256, "%s", actionStrP );

		 if (event.action == SI_MAKE ) 
			Con::executef( this,  "onInputEvent", deviceString, actionString, Con::getIntArg(mMousePos.x), Con::getIntArg(mMousePos.y), "1" );
		 else if (event.action == SI_BREAK ) 
		    Con::executef( this,  "onInputEvent", deviceString, actionString, Con::getIntArg(mMousePos.x), Con::getIntArg(mMousePos.y), "0" );
		 else {
			 //handle joystick and mouse move! (mouse on non cursor only)
            if ((event.deviceType == JoystickDeviceType || event.deviceType == MouseDeviceType)  && event.objInst == 0) {
				F32 tmpPos = (mFloor(event.fValue*1000)/1000);
				if (mFabs(tmpPos)<0.002)
					tmpPos = 0;
				Con::executef( this,  "onInputEvent", deviceString, actionString, Con::getIntArg(mMousePos.x), Con::getIntArg(mMousePos.y), Con::getFloatArg(tmpPos) );
			}
		 }

         return( false );
      }

	return false;
}


//------------------------------------------------------------------------------
// Console Mouse (orgin afx)
//------------------------------------------------------------------------------
//ConsoleMethod(ohmTSCtrl, getMouse3DVec, const char *, 2, 2, "")
DefineEngineMethod(ohmTSCtrl, getMouse3DVec, const char*,(), , "return the mouse vector")
{
  char* rbuf = Con::getReturnBuffer(256);
  const Point3F& vec = object->getMouse3DVec();
  dSprintf(rbuf, 256, "%g %g %g", vec.x, vec.y, vec.z);
  return rbuf;
}

//ConsoleMethod(ohmTSCtrl, getMouse3DPos, const char *, 2, 2, "")
DefineEngineMethod(ohmTSCtrl, getMouse3DPos, const char*, (), , "return the mouse position")
{
  char* rbuf = Con::getReturnBuffer(256);
  const Point3F& pos = object->getMouse3DPos();
  dSprintf(rbuf, 256, "%g %g %g", pos.x, pos.y, pos.z);
  return rbuf;
}


//------------------------------------------------------------------------------
// Selectons
//------------------------------------------------------------------------------
//ConsoleMethod(ohmTSCtrl, setSelectedObject, bool, 3, 3, "id of object, set the selected object!, autoselect should be off")
DefineEngineMethod(ohmTSCtrl, setSelectedObject, bool, (ShapeBase* lObj), , "id of object, set the selected object!, autoselect should be off")
{
   object->setSelectedObject(lObj);
   return true;
}
//------------------------------------------------------------------------------
//ConsoleMethod(ohmTSCtrl, clearSelectedObject, void, 2, 2, "clear selected object")
DefineEngineMethod(ohmTSCtrl, clearSelectedObject, void, (), , "clear selected object")
{
   object->clearSelectedObject();
}
//-----------------------------------------------------------------------------
//ConsoleMethod( ohmTSCtrl, getSelectedObject, S32, 2, 2, "()"
DefineEngineMethod( ohmTSCtrl, getSelectedObject, S32, (), , "Gets a selected object id")
{
   ShapeBase* obj = object->getSelectedObject();
   return obj? obj->getId(): -1;
}

//-----------------------------------------------------------------------------
// V2.18
//ConsoleMethod( ohmTSCtrl, setMouseHitMask, void, 3, 3, "( Hitmask for mousecollision  )")
DefineEngineMethod(ohmTSCtrl, setMouseHitMask, void, (U32 hitMask), , "( Hitmask for mousecollision  )")
{
	object->setMouseHitMask(hitMask);
}

//-----------------------------------------------------------------------------
// V2.18
//ConsoleMethod( ohmTSCtrl, getMouseHitMask, S32, 3, 3, "(  )")
DefineEngineMethod(ohmTSCtrl, getMouseHitMask, S32, (), , "(  )")
{
	return (S32) object->getMouseHitMask();
}
//-----------------------------------------------------------------------------
//2023-03-07
DefineEngineMethod(ohmTSCtrl, getLastMouseRayPoint, Point3F, (), , "get the last mouserayPoint")
{
   return object->getLastMouseRayPoint();
}
DefineEngineMethod(ohmTSCtrl, getLastMouseRayNormal, Point3F, (), , "get the last mouserayNormal")
{
   return object->getLastMouseRayNormal();
}

//2023-03-09
DefineEngineMethod(ohmTSCtrl, setWindowLocked, void, (bool lValue), , "manually set window Lock ignoring LockMouseToWindow")
{
   Platform::setWindowLocked(lValue);
}

//2023-12-14
DefineEngineMethod(ohmTSCtrl, getMousePos3D, Point3F, (), , "Get the last mouse pos in 3D world")
{
   return object->getLastMouseRayPoint();
}


