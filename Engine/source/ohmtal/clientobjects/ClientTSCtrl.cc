//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------

/*
* 
   ClientTSCtrl based on guiTSControl and guiEvent
   old ClientTSCtrl was based on EditorTSCtrl by t.huehn
   =============================================
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   Did not test this so far no idea if its work!!!!






   TSCtrl for rendering clickable Objects without Serverconnection.


   - additional ideas from fxRenderObject and NeHeGL's Lession32

   Current Shapes Inheritance :
   ============================
   * SimObject
     * ClientShapeObject : a simple shape with a render function
        * ClientDTSShape : a DTS shape





======
MOUSEDOWN:

   // New Track Object
   mTrackObject = (ShapeBase*)mCursorObject;



MOUSEUP:
    //allways call onclick :P
     if (mTrackObject) {
        // Select only objects we've tracked
        if (mTrackObject == mCursorObject)
           ShapeBase::onClick( mTrackObject);
     }



MOUSEMOVE:

   if (mCursorObject = collide( event)) {
      // Set Hand Cursor over selectable objects
      if (mCursorObject->IsSelectable())
         setCursor( HandCursor);
      else
         mCursorObject = NULL;
   }
***
ShapeBase* GameTSCtrl::collide( const Gui3DMouseEvent &event) {
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return false;

   SceneObject* controlObj = conn->getControlObject();
   if (controlObj)
      controlObj->disableCollision();

   //
   Point3F startPnt = event.pos;
   Point3F endPnt = event.pos + event.vec * mProjectDistance;

   //
   static U32 losMask = TerrainObjectType | In	teriorObjectType | ShapeBaseObjectType ;
   RayInfo ri;
   bool hit = gClientContainer.castRay( startPnt, endPnt, losMask, &ri);

   if (controlObj)
      controlObj->enableCollision();

   if (hit && (ri.object->getTypeMask() & ShapeBaseObjectType))
      return (ShapeBase*)ri.object;
   else
      return NULL;
}

!!!!!!!!!!!!!!!!!!!!!!!!!!
bool TSShapeInstance::castRay(const Point3F & a, const Point3F & b, RayInfo * rayInfo, S32 dl)
!!!!!!!!!!!!!!!!!!!!!!!!!!

2023-10-29 XXTH :
   \o/ after alter lightmanager conn is NULL,
   i got both working and can set camera manually.
   Problems:
      * Shadows are flickering in BOTH views!! 
      * tsDynamic not working 

2023-12-16 left right (pitch?!) is not smooth
   ??? 


*/

#include "T3D/gameFunctions.h"
#include "ClientTSCtrl.h"
#include "ClientCamera.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"



IMPLEMENT_CONOBJECT(ClientTSCtrl);


static Point3F BoxPnts[] = {
   Point3F(0,0,0),
   Point3F(0,0,1),
   Point3F(0,1,0),
   Point3F(0,1,1),
   Point3F(1,0,0),
   Point3F(1,0,1),
   Point3F(1,1,0),
   Point3F(1,1,1)
};

static U32 BoxVerts[][4] = {
   {0,2,3,1},     // -x
   {7,6,4,5},     // +x
   {0,1,5,4},     // -y
   {3,2,6,7},     // +y
   {0,4,6,2},     // -z
   {3,7,5,1}      // +z
};

static Point3F BoxNormals[] = {
   Point3F(-1, 0, 0),
   Point3F(1, 0, 0),
   Point3F(0,-1, 0),
   Point3F(0, 1, 0),
   Point3F(0, 0,-1),
   Point3F(0, 0, 1)
};

//------------------------------------------------------------------------------

//Point3F  ClientTSCtrl::smCamPos;
//EulerF   ClientTSCtrl::smCamRot;
//MatrixF  ClientTSCtrl::smCamMatrix;
//F32      ClientTSCtrl::smVisibleDistance = 2100.f;

ClientTSCtrl::ClientTSCtrl() //:mEditManager(0)
{
   mRightMousePassThru = false;
   mMiddleMousePassThru = false;


   // Set up our camera
   smCamMatrix.identity();
   smCamRot.set(0.0f, 0.0f, 0.0f);
   smCamPos.set(0.0f, -20.0f, 0.0f); // board object	
   smCamMatrix.setColumn(3, smCamPos);



   mCamera = NULL;
   mCamOffset = 3.f;

   mEnableMovement = false;


   mMouseHitMask = GameBaseObjectType | StaticObjectType | PlayerObjectType | TerrainObjectType | TerrainLikeObjectType | InteriorLikeObjectType  ;

   mSelectedObj = NULL; //For special editor

   mLastMouseRayPoint.zero();
   mLastMouseRayNormal.zero();

   mSelectedCubeDesc.setZReadWrite(true, false);
   mSelectedCubeDesc.setBlend(true);
   mSelectedCubeDesc.fillMode = GFXFillWireframe;


}
//-----------------------------------------------------------------------------------------------------------
ClientTSCtrl::~ClientTSCtrl()
{
}

//------------------------------------------------------------------------------

bool ClientTSCtrl::onAdd()
{
   if (!Parent::onAdd())
      return(false);

   // give all derived access to the fields
   Parent::setModStaticFields(true);
   return true;
}

//------------------------------------------------------------------------------
Move ClientTSCtrl::getMove()
{
   Move lMove = NullMove;

   if (!haveMove())
      return lMove;

   getNextMove(lMove);
   return lMove;
}

bool ClientTSCtrl::getNextMove(Move& curMove)
{
   /*
   if(mMoveList.size() > MaxMoveQueueSize)
      return false;
     */


   F32 pitchAdd   = MoveManager::mPitchUpSpeed - MoveManager::mPitchDownSpeed;
   F32 yawAdd     = MoveManager::mYawLeftSpeed - MoveManager::mYawRightSpeed;
   F32 rollAdd    = MoveManager::mRollRightSpeed - MoveManager::mRollLeftSpeed;

   curMove.pitch  = MoveManager::mPitch + pitchAdd;
   curMove.yaw    = MoveManager::mYaw + yawAdd;
   curMove.roll   = MoveManager::mRoll + rollAdd;

   MoveManager::mPitch  = 0.f;
   MoveManager::mYaw    = 0.f;
   MoveManager::mRoll   = 0.f;

   curMove.x = MoveManager::mRightAction - MoveManager::mLeftAction;
   curMove.y = MoveManager::mForwardAction - MoveManager::mBackwardAction;
   curMove.z = MoveManager::mUpAction - MoveManager::mDownAction;

   curMove.freeLook = MoveManager::mFreeLook;

   for (U32 i = 0; i < MaxTriggerKeys; i++)
   {
      curMove.trigger[i] = false;
      if (MoveManager::mTriggerCount[i] & 1)
         curMove.trigger[i] = true;
      else if (!(MoveManager::mPrevTriggerCount[i] & 1) && MoveManager::mPrevTriggerCount[i] != MoveManager::mTriggerCount[i])
         curMove.trigger[i] = true;
      MoveManager::mPrevTriggerCount[i] = MoveManager::mTriggerCount[i];
   }
   //XXTH 2023-12-16 we have not traffic here !! curMove.clamp();  // clamp for net traffic
   
   return true;
}

//------------------------------------------------------------------------------
void ClientTSCtrl::onRender(Point2I offset, const RectI& updateRect)
{
   Parent::onRender(offset, updateRect);

}

//------------------------------------------------------------------------------

void ClientTSCtrl::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("Camera");
   addField("CamPos", TypePoint3F, Offset(smCamPos, ClientTSCtrl));
   addField("CamRot", TypePoint3F, Offset(smCamRot, ClientTSCtrl));
   addField("CamOffset", TypeF32, Offset(mCamOffset, ClientTSCtrl));
   endGroup("Camera");

   addField("RightMousePassThru", TypeBool, Offset(mRightMousePassThru, ClientTSCtrl));
   addField("MiddleMousePassThru", TypeBool, Offset(mMiddleMousePassThru, ClientTSCtrl));


}

//------------------------------------------------------------------------------
void ClientTSCtrl::consoleInit()
{
   Parent::consoleInit();
}

//------------------------------------------------------------------------------

void ClientTSCtrl::make3DMouseEvent(Gui3DMouseEvent& gui3DMouseEvent, const GuiEvent& event)
{
   (GuiEvent&)(gui3DMouseEvent) = event;

   // get the eye pos and the mouse vec from that...
   Point3F sp(event.mousePoint.x, event.mousePoint.y, 1);

   Point3F wp;
   unproject(sp, &wp);

   gui3DMouseEvent.pos = smCamPos;
   gui3DMouseEvent.vec = wp - smCamPos;
   gui3DMouseEvent.vec.normalize();
}

//------------------------------------------------------------------------------

void ClientTSCtrl::getCursor(GuiCursor*& cursor, bool& visible, const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   get3DCursor(cursor, visible, mLastEvent);
}
//------------------------------------------------------------------------------
void ClientTSCtrl::get3DCursor(GuiCursor*& cursor, bool& visible, const Gui3DMouseEvent& event)
{
   event;
   cursor = NULL;
   visible = true;
}


//------------------------------------------------------------------------------
void ClientTSCtrl::onMouseUp(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseUp(mLastEvent);

   sendMouseEvent("onMouseUp", event);
}

//------------------------------------------------------------------------------
void ClientTSCtrl::onMouseDown(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseDown(mLastEvent);

   //XXTH for selections
   //	selection(event);

   sendMouseEvent("onMouseDown", event);
}



//------------------------------------------------------------------------------
void ClientTSCtrl::onMouseMove(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseMove(mLastEvent);
   sendMouseEvent("onMouseMove", event);
}

//------------------------------------------------------------------------------
void ClientTSCtrl::onMouseDragged(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseDragged(mLastEvent);
   sendMouseEvent("onMouseDragged", event);

}
//------------------------------------------------------------------------------
void ClientTSCtrl::onMouseEnter(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseEnter(mLastEvent);
   sendMouseEvent("onMouseEnter", event);
}
//------------------------------------------------------------------------------
void ClientTSCtrl::onMouseLeave(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DMouseLeave(mLastEvent);
   sendMouseEvent("onMouseLeave", event);
}
//------------------------------------------------------------------------------
void ClientTSCtrl::onRightMouseDown(const GuiEvent& event)
{
   // always process the right mouse event first...

   make3DMouseEvent(mLastEvent, event);
   on3DRightMouseDown(mLastEvent);

   if (mRightMousePassThru && mProfile->mCanKeyFocus)
   {
      // ok, gotta disable the mouse
      // script functions are lockMouse(true); Canvas.cursorOff();
      GuiCanvas* Canvas = getRoot();
      Platform::setWindowLocked(true);
      Canvas->setCursorON(false);
      setFirstResponder();
   }
   sendMouseEvent("onRightMouseDown", event);
}
//------------------------------------------------------------------------------
void ClientTSCtrl::onRightMouseUp(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DRightMouseUp(mLastEvent);
   sendMouseEvent("onRightMouseUp", event);
}
//------------------------------------------------------------------------------
void ClientTSCtrl::onRightMouseDragged(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   on3DRightMouseDragged(mLastEvent);
}
//------------------------------------------------------------------------------
void ClientTSCtrl::onMiddleMouseDown(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   // on3DRightMouseDown(mLastEvent);

   if (mMiddleMousePassThru && mProfile->mCanKeyFocus)
   {
      // ok, gotta disable the mouse
      // script functions are lockMouse(true); Canvas.cursorOff();
      Platform::setWindowLocked(true);
      GuiCanvas* Canvas = getRoot();
      Canvas->setCursorON(false);
      setFirstResponder();
   }
   sendMouseEvent("onMiddleMouseDown", event);
}
//------------------------------------------------------------------------------
void ClientTSCtrl::onMiddleMouseUp(const GuiEvent& event)
{
   make3DMouseEvent(mLastEvent, event);
   // on3DRightMouseUp(mLastEvent);
   sendMouseEvent("onMiddleMouseUp", event);
}

//------------------------------------------------------------------------------
bool ClientTSCtrl::onInputEvent(const InputEventInfo& event)
{
   
   if (event.deviceType == MouseDeviceType && event.action == SI_BREAK
      && (
         (mRightMousePassThru && event.objInst == KEY_BUTTON1)
         ||
         (mMiddleMousePassThru && event.objInst == KEY_BUTTON2)
         )
      )
   {
      // if the right mouse pass thru is enabled,
      // we want to reactivate mouse on a right mouse button up
      Platform::setWindowLocked(false);
      GuiCanvas* Canvas = Parent::getRoot();
      Canvas->setCursorON(true);
   }
   // we return false so that the canvas can properly process the right mouse button up...
   return false;
}
//------------------------------------------------------------------------------
bool ClientTSCtrl::hittest(const GuiEvent& event, RayInfo& ri) {



   // get the eye pos and the mouse vec from that...
   Point3F sp(event.mousePoint.x, event.mousePoint.y, 1);

   Point3F wp;
   unproject(sp, &wp);

   Gui3DMouseEvent gui3DMouseEvent;
   gui3DMouseEvent.pos = smCamPos;
   gui3DMouseEvent.vec = wp - smCamPos;
   gui3DMouseEvent.vec.normalize();

   F32 mProjectDistance = 2000.f;

   Point3F startPnt = gui3DMouseEvent.pos;
   Point3F endPnt = gui3DMouseEvent.pos + gui3DMouseEvent.vec * mProjectDistance;

   // RayInfo ri;

   bool hit = gClientContainer.castRay(startPnt, endPnt, mMouseHitMask, &ri);

   if (!hit)
   {
      ri.distance = 0;
      ri.normal = Point3F(0.f, 0.f, 0.f);
      ri.point = Point3F(0.f, 0.f, 0.f);
   }

   //Con::printf("CASTRAY start: %f,%f,%f end: %f,%f,%f",startPnt.x,startPnt.y,startPnt.z,endPnt.x,endPnt.y,endPnt.z);

   return hit;

}

//------------------------------------------------------------------------------

void ClientTSCtrl::sendMouseEvent(const char* name, const GuiEvent& event)
{
/* old style: 
   char buf[4][32];
   dSprintf(buf[0], 32, "%d", event.modifier);
   dSprintf(buf[1], 32, "%d %d", event.mousePoint.x, event.mousePoint.y);
   dSprintf(buf[2], 32, "%d", event.mouseClickCount);

   RayInfo ri;
   if (hittest(event, ri))
      dSprintf(buf[3], 32, "%d", ri.object->getId());
   else
      dSprintf(buf[3], 32, "%d", 0);

   

   char bigbuf[2][256];
   dSprintf(bigbuf[0], 256, "%f %f %f", ri.point.x, ri.point.y, ri.point.z);
   dSprintf(bigbuf[1], 256, "%f %f %f", ri.normal.x, ri.normal.y, ri.normal.z);

   // CALL EVENT WITH: Modifier, MOUSEPOINT(x y), %clickcount, hitobject, hitPoint(x y z), hitNormal(x y z)
     //function ClientTSCtrl::onMouseDown(%this,%modifier,%mousePos,%clickCount,%hitObj,%hitPos,%hitNormal)

     //FIXME stupid new stigg look at GuiMouseEventCtrl== > 
     //ORIG: Con::executef(this, 7, name, buf[0], buf[1], buf[2], buf[3], bigbuf[0], bigbuf[1]);
   Con::executef(this,  name, buf[0], buf[1], buf[2], buf[3], bigbuf[0], bigbuf[1]);
*/

   RayInfo ri;
   bool lHit = hittest(event, ri);
   mLastMouseRayPoint = ri.point;
   mLastMouseRayNormal = ri.normal;

   Con::executef(this, name,
      event.modifier    //%modifier
      , Point2I(event.mousePoint.x, event.mousePoint.y) // %mousePoint
      , event.mouseClickCount //clickcount
      , (lHit ? ri.object->getId() : 0)  //%hitObject
      , ri.point  //%rayPoint
      , ri.normal //%rayNormal
   );

}

//------------------------------------------------------------------------------
void ClientTSCtrl::renderWorld(const RectI& updateRect)
{

   PROFILE_START(GameRenderClientWorld);
   FrameAllocator::setWaterMark(0);

   //i need more scene graph's ... but too much work! 
   gClientSceneGraph->renderScene(SPT_Diffuse);


   if (mSelectedObj)
   {
      Box3F lBox = mSelectedObj->getWorldBox();
      GFX->getDrawUtil()->drawCube(mSelectedCubeDesc, lBox, ColorI(230, 250, 60, 255));
   }

   // renderScene leaves some states dirty, which causes problems if GameTSCtrl is the last Gui object rendered
   GFX->updateStates();

   AssertFatal(FrameAllocator::getWaterMark() == 0,
      "Error, someone didn't reset the water mark on the frame allocator!");
   FrameAllocator::setWaterMark(0);
   PROFILE_END();
//   GameRenderWorld();
}


//------------------------------------------------------------------------------

bool ClientTSCtrl::processCameraQuery(CameraQuery* query)
{
   if (getCamera())
   {
      // Provide some default values
      query->stereoTargets[0] = 0;
      query->stereoTargets[1] = 0;
      query->eyeOffset[0] = Point3F::Zero;
      query->eyeOffset[1] = Point3F::Zero;
      query->hasFovPort = false;
      query->hasStereoTargets = false;
      query->displayDevice = NULL;

      query->eyeTransforms[0] = query->cameraMatrix;
      query->eyeTransforms[1] = query->cameraMatrix;
      query->headMatrix = query->cameraMatrix;


      F32 lPos = mCamOffset;
      getCamera()->getCameraTransform(&lPos, &query->cameraMatrix);
      query->nearPlane = 0.1f;

      
      query->farPlane = gClientSceneGraph->getVisibleDistance();
      query->fov = mDegToRad(getCamera()->getCameraFov());
      query->ortho = false;
      query->cameraMatrix.getColumn(3, &smCamPos); // for mouse handling!
      return true;

   }



   MatrixF xRot, zRot;
   xRot.set(EulerF(smCamRot.x, 0, 0)); //x=yaw ? 
   zRot.set(EulerF(0, 0, smCamRot.z)); //z=pitch

   smCamMatrix.mul(zRot, xRot);
   query->nearPlane = 0.1f;
   query->farPlane = 2100.0f;
   if (mCamera)
      query->fov = mDegToRad(mCamera->getCameraFov());

   smCamMatrix.setColumn(3.f, smCamPos);
   query->cameraMatrix = smCamMatrix;

   return true;

}
// --------------------------------------------------------------------------------------------
//DefineEngineStringlyVariadicMethod(ClientTSCtrl, setSelectedObject, void, 3, 3, "( SceneObject obj  )")
DefineEngineMethod(ClientTSCtrl, setSelectedObject, void, (SceneObject* obj), ,"(SceneObject obj)")
{
      object->setSelectedObject(obj);
}
// --------------------------------------------------------------------------------------------
DefineEngineStringlyVariadicMethod(ClientTSCtrl, getSelectedObject, S32, 2, 2, "()")
{
   SceneObject* lObj = object->getSelectedObject();
   return lObj ? lObj->getId() : 0;
}
// --------------------------------------------------------------------------------------------
DefineEngineStringlyVariadicMethod(ClientTSCtrl, setCamera, void, 3, 3, "( ClientCamera obj  )")
{
   // Find the Cam
   ClientCamera* obj;
   if (Sim::findObject(argv[2], obj)) {
      object->setCamera(obj);
   }
   else
      object->setCamera(NULL);
}

DefineEngineStringlyVariadicMethod(ClientTSCtrl, setMouseHitMask, void, 3, 3, "( Hitmask for mousecollision  )")
{
   object->setMouseHitMask((U32)dAtoi(argv[2]));
}

DefineEngineStringlyVariadicMethod(ClientTSCtrl, getMouseHitMask, S32, 3, 3, "(  )")
{
   return (S32)object->getMouseHitMask();
}

//2023-12-14
DefineEngineMethod(ClientTSCtrl, getMousePos3D, Point3F, (), , "Get the last mouse pos in 3D world")
{
   return object->getLastMouseRayPoint();
}

DefineEngineMethod(ClientTSCtrl, getLastMouseRayPoint, Point3F, (), , "get the last mouserayPoint")
{
   return object->getLastMouseRayPoint();
}
DefineEngineMethod(ClientTSCtrl, getLastMouseRayNormal, Point3F, (), , "get the last mouserayNormal")
{
   return object->getLastMouseRayNormal();
}


