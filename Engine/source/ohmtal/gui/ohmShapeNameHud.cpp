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

#include "platform/platform.h"

#include "gui/core/guiControl.h"
#include "gui/3d/guiTSControl.h"
#include "console/consoleTypes.h"
#include "scene/sceneManager.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "ohmtal/ai/aiEntity.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"
//----------------------------------------------------------------------------
// Modified for Auteria style gameplay

//----------------------------------------------------------------------------
/// Displays name & damage above shape objects.
///
/// This control displays the name and damage value of all named
/// ShapeBase objects on the client.  The name and damage of objects
/// within the control's display area are overlayed above the object.
///
/// This GUI control must be a child of a TSControl, and a server connection
/// and control object must be present.
///
/// This is a stand-alone control and relies only on the standard base GuiControl.
class OhmShapeNameHud : public GuiControl {
   typedef GuiControl Parent;

   // field data
   LinearColorF   mFillColor;
   LinearColorF   mFrameColor;
   LinearColorF   mTextColor;
   LinearColorF   mLabelFillColor;
   LinearColorF   mLabelFrameColor;

   F32      mVerticalOffset;
   F32      mDistanceFade;
   bool     mShowFrame;
   bool     mShowFill;
   bool     mShowLabelFrame;
   bool     mShowLabelFill;

   Point2I  mLabelPadding;

   //XXTH Ohm added:
   F32  mVisDistance;
   
   bool	mShowDamage;
   bool mShowMySelf; //XXTH
   bool mOnlyPlayerObjects; //2023-03-23 only loop playerobjects! 
   bool mCustomDrawDamageName; //XXTH for drawing with tom2d
   bool mDrawDamageName; //XXTH V2.20
   bool	mUseStaticDisplay; //XXTH 1.98 animation do not move the text on players!

   U32 mLosMask;  // default  TerrainObjectType | InteriorLikeObjectType  | StaticShapeObjectType | StaticObjectType;

   LinearColorF   mDamageFillColor;
   LinearColorF   mDamageFriendlyFillColor;
   LinearColorF   mDamageFrameColor;
   Point2I  mDamageRectSize;
   Point2I  mDamageOffset;

   bool       mShowSelectedObject;



protected:
//XXTH unused   void drawName( Point2I offset, const char *buf, F32 opacity);
   void drawDamageName(Point2I offset, const char* name, const char* guild, U32 playerrank, F32 damagepercent, F32 damagelevel, F32 maxdamage, F32 opacity, S32 playerType, bool showSelection);

   bool onInputEvent(const InputEventInfo& event); //XXTH from tom2D


public:
   OhmShapeNameHud();

   //XXTH >>
   bool mSeekNewSelectedObject;
   bool mBackwardSeekObject; //XXTH 1.98 added for selecting the previous!
   bool mSeekPlayerType[8];

   ShapeBase* mSelectedObject;

   // object selection additions
   virtual void onMouseDown(const GuiEvent& evt);
   virtual void onMouseDragged(const GuiEvent& event);
   virtual void onMouseUp(const GuiEvent& event);
   virtual void onMouseMove(const GuiEvent& evt);
   virtual void onRightMouseDown(const GuiEvent& event);
   virtual void onRightMouseUp(const GuiEvent& event);
   virtual void onRightMouseDragged(const GuiEvent& event);

   void onMiddleMouseDown(const GuiEvent& event);

   void onMiddleMouseUp(const GuiEvent& event);

   void onMiddleMouseDragged(const GuiEvent& event);


   void setLosMask(U32 lLosMask) { mLosMask = lLosMask; }
   //<< XXTH

   // GuiControl
   virtual void onRender(Point2I offset, const RectI &updateRect);

   static void initPersistFields();
   DECLARE_CONOBJECT( OhmShapeNameHud );
   DECLARE_CATEGORY( "Gui Game" );
   DECLARE_DESCRIPTION( "Displays name and damage of AIEntity objects in its bounds.\n"
      "Must be a child of a GuiTSCtrl and a server connection must be present." );
};


//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(OhmShapeNameHud);

ConsoleDocClass( OhmShapeNameHud,
   "@brief Displays name and damage of ShapeBase objects in its bounds. Must be a child of a GuiTSCtrl and a server connection must be present.\n\n"
   "This control displays the name and damage value of all named ShapeBase objects on the client. "
   "The name and damage of objects within the control's display area are overlayed above the object.\n\n"
   "This GUI control must be a child of a TSControl, and a server connection and control object must be present. "
   "This is a stand-alone control and relies only on the standard base GuiControl.\n\n"
   
   "@tsexample\n"
		"\n new OhmShapeNameHud()"
		"{\n"
		"	fillColor = \"0.0 1.0 0.0 1.0\"; // Fills with a solid green color\n"
		"	frameColor = \"1.0 1.0 1.0 1.0\"; // Solid white frame color\n"
		"	textColor = \"1.0 1.0 1.0 1.0\"; // Solid white text Color\n"
		"	showFill = \"true\";\n"
		"	showFrame = \"true\";\n"
		"	labelFillColor = \"0.0 1.0 0.0 1.0\"; // Fills with a solid green color\n"
		"	labelFrameColor = \"1.0 1.0 1.0 1.0\"; // Solid white frame color\n"
		"	showLabelFill = \"true\";\n"
		"	showLabelFrame = \"true\";\n"
		"	verticalOffset = \"0.15\";\n"
		"	distanceFade = \"15.0\";\n"
		"};\n"
   "@endtsexample\n\n"
   
   "@ingroup GuiGame\n"
);

OhmShapeNameHud::OhmShapeNameHud()
{
   mFillColor.set( 0.25f, 0.25f, 0.25f, 0.25f );
   mFrameColor.set( 0, 1, 0, 1 );
   mLabelFillColor.set( 0.25f, 0.25f, 0.25f, 0.25f );
   mLabelFrameColor.set( 0, 1, 0, 1 );
   mTextColor.set( 0, 1, 0, 1 );
   mShowFrame = mShowFill = true;
   mShowLabelFrame = mShowLabelFill = false;
   mVerticalOffset = 0.5f;
   mDistanceFade = 0.1f;
   mLabelPadding.set(0, 0);
//XXTH >>
   mVisDistance = gClientSceneGraph->getVisibleDistance();
   mShowDamage = true;
   mShowMySelf = false;
   mCustomDrawDamageName = false;
   mOnlyPlayerObjects = true;
   mUseStaticDisplay = false;
   mVerticalOffset = 0.5;
   mDamageFillColor.set(0.f, 1.f, 0.f, 1.f);
   mDamageFriendlyFillColor.set(0.7f, 0.2f, 0.2f, 1.f);
   mDamageFrameColor.set(1.f, 0.6f, 0.f, 1.f);
   mDamageRectSize.set(50, 4);
   mDamageOffset.set(0, 32);
   mShowSelectedObject = false;
   mSelectedObject = NULL;
   mSeekNewSelectedObject = false;
   mBackwardSeekObject = false;
   for (U32 i = 0; i < 8; i++) {
      mSeekPlayerType[i] = false;
   }
   mSeekPlayerType[0] = true;
   mSeekPlayerType[1] = true;
   mSeekPlayerType[3] = true;
   mSeekPlayerType[4] = true;

   mLosMask = TerrainObjectType | InteriorLikeObjectType | StaticShapeObjectType | StaticObjectType;
//<< XXTH
}

void OhmShapeNameHud::initPersistFields()
{
   addGroup("Colors");     
   addField( "fillColor",  TypeColorF, Offset( mFillColor, OhmShapeNameHud ), "Standard color for the background of the control." );
   addField( "frameColor", TypeColorF, Offset( mFrameColor, OhmShapeNameHud ), "Color for the control's frame."  );
   addField( "textColor",  TypeColorF, Offset( mTextColor, OhmShapeNameHud ), "Color for the text on this control." );
   addField( "labelFillColor",  TypeColorF, Offset( mLabelFillColor, OhmShapeNameHud ), "Color for the background of each shape name label." );
   addField( "labelFrameColor", TypeColorF, Offset( mLabelFrameColor, OhmShapeNameHud ), "Color for the frames around each shape name label."  );
   endGroup("Colors");     

   addGroup("ohmtal");
   addField("visibleDistance", TypeF32, Offset(mVisDistance, OhmShapeNameHud));
   addField("showMySelf", TypeBool, Offset(mShowMySelf, OhmShapeNameHud));
   addField("UseStaticDisplay", TypeBool, Offset(mUseStaticDisplay, OhmShapeNameHud));
   addField("customDrawDamageName", TypeBool, Offset(mCustomDrawDamageName, OhmShapeNameHud));
   addField("onlyPlayerObjects", TypeBool, Offset(mOnlyPlayerObjects, OhmShapeNameHud));
   endGroup("ohmtal");

   addGroup("Damage");
   addField("showDamage", TypeBool, Offset(mShowDamage, OhmShapeNameHud));
   addField("drawName", TypeBool, Offset(mDrawDamageName, OhmShapeNameHud));
   addField("damageFillColor", TypeColorF, Offset(mDamageFillColor, OhmShapeNameHud));
   addField("damageFriendlyFillColor", TypeColorF, Offset(mDamageFriendlyFillColor, OhmShapeNameHud));
   addField("damageFrameColor", TypeColorF, Offset(mDamageFrameColor, OhmShapeNameHud));
   addField("damageRect", TypePoint2I, Offset(mDamageRectSize, OhmShapeNameHud));
   addField("damageOffset", TypePoint2I, Offset(mDamageOffset, OhmShapeNameHud));
   endGroup("Damage");

   addGroup("Selection");
   addField("seekPlayerType0", TypeBool, Offset(mSeekPlayerType[0], OhmShapeNameHud), "Human or Friendly (green)");
   addField("seekPlayerType1", TypeBool, Offset(mSeekPlayerType[1], OhmShapeNameHud), "Enemy Bot - red");
   addField("seekPlayerType2", TypeBool, Offset(mSeekPlayerType[2], OhmShapeNameHud), "NPC - yellow");
   addField("seekPlayerType3", TypeBool, Offset(mSeekPlayerType[3], OhmShapeNameHud), "Harvest - magenta");
   addField("seekPlayerType4", TypeBool, Offset(mSeekPlayerType[4], OhmShapeNameHud), "Farming - blue");
   addField("seekPlayerType5", TypeBool, Offset(mSeekPlayerType[5], OhmShapeNameHud), "Misc - lightgray");
   addField("seekPlayerType6", TypeBool, Offset(mSeekPlayerType[6], OhmShapeNameHud));
   addField("seekPlayerType7", TypeBool, Offset(mSeekPlayerType[7], OhmShapeNameHud));

   endGroup("Selection");




   addGroup("Misc");       
   addField( "showFill",   TypeBool, Offset( mShowFill, OhmShapeNameHud ), "If true, we draw the background color of the control." );
   addField( "showFrame",  TypeBool, Offset( mShowFrame, OhmShapeNameHud ), "If true, we draw the frame of the control."  );
   addField( "showLabelFill",  TypeBool, Offset( mShowLabelFill, OhmShapeNameHud ), "If true, we draw a background for each shape name label." );
   addField( "showLabelFrame", TypeBool, Offset( mShowLabelFrame, OhmShapeNameHud ), "If true, we draw a frame around each shape name label."  );
   addField( "labelPadding", TypePoint2I, Offset( mLabelPadding, OhmShapeNameHud ), "The padding (in pixels) between the label text and the frame." );
   addField( "verticalOffset", TypeF32, Offset( mVerticalOffset, OhmShapeNameHud ), "Amount to vertically offset the control in relation to the ShapeBase object in focus." );
   addField( "distanceFade", TypeF32, Offset( mDistanceFade, OhmShapeNameHud ), "Visibility distance (how far the player must be from the ShapeBase object in focus) for this control to render." );
   endGroup("Misc");
   Parent::initPersistFields();
}


//----------------------------------------------------------------------------
/// Core rendering method for this control.
///
/// This method scans through all the current client ShapeBase objects.
/// If one is named, it displays the name and damage information for it.
///
/// Information is offset from the center of the object's bounding box,
/// unless the object is a PlayerObjectType, in which case the eye point
/// is used.
///
/// @param   updateRect   Extents of control.
/*
void OhmShapeNameHud::onRender( Point2I, const RectI &updateRect)
{
   // Background fill first
   if (mShowFill)
      GFX->getDrawUtil()->drawRectFill(updateRect, mFillColor.toColorI());

   // Must be in a TS Control
   GuiTSCtrl *parent = dynamic_cast<GuiTSCtrl*>(getParent());
   if (!parent) return;

   // Must have a connection and control object
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn) return;
   GameBase * control = dynamic_cast<GameBase*>(conn->getControlObject());
   if (!control) return;

   // Get control camera info
   MatrixF cam;
   Point3F camPos;
   VectorF camDir;
   conn->getControlCameraTransform(0,&cam);
   cam.getColumn(3, &camPos);
   cam.getColumn(1, &camDir);

   F32 camFovCos;
   conn->getControlCameraFov(&camFovCos);
   camFovCos = mCos(mDegToRad(camFovCos) / 2);

   // Visible distance info & name fading
   F32 visDistance = gClientSceneGraph->getVisibleDistance();
   F32 visDistanceSqr = visDistance * visDistance;
   F32 fadeDistance = visDistance * mDistanceFade;

   // Collision info. We're going to be running LOS tests and we
   // don't want to collide with the control object.
   static U32 losMask = TerrainObjectType | ShapeBaseObjectType | StaticObjectType;
   control->disableCollision();

   // All ghosted objects are added to the server connection group,
   // so we can find all the shape base objects by iterating through
   // our current connection.
   for (SimSetIterator itr(conn); *itr; ++itr) {
      ShapeBase* shape = dynamic_cast< ShapeBase* >(*itr);
      if ( shape ) {
         if (shape != control && shape->getShapeName()) 
         {

            // Target pos to test, if it's a player run the LOS to his eye
            // point, otherwise we'll grab the generic box center.
            Point3F shapePos;
            if (shape->getTypeMask() & PlayerObjectType) 
            {
               MatrixF eye;

               // Use the render eye transform, otherwise we'll see jittering
               shape->getRenderEyeTransform(&eye);
               eye.getColumn(3, &shapePos);
            } 
            else 
            {
                // Use the render transform instead of the box center
                // otherwise it'll jitter.
               MatrixF srtMat = shape->getRenderTransform();
               srtMat.getColumn(3, &shapePos);
            }
            VectorF shapeDir = shapePos - camPos;

            // Test to see if it's in range
            F32 shapeDist = shapeDir.lenSquared();
            if (shapeDist == 0 || shapeDist > visDistanceSqr)
               continue;
            shapeDist = mSqrt(shapeDist);

            // Test to see if it's within our viewcone, this test doesn't
            // actually match the viewport very well, should consider
            // projection and box test.
            shapeDir.normalize();
            F32 dot = mDot(shapeDir, camDir);
            if (dot < camFovCos)
               continue;

            // Test to see if it's behind something, and we want to
            // ignore anything it's mounted on when we run the LOS.
            RayInfo info;
            shape->disableCollision();
            SceneObject *mount = shape->getObjectMount();
            if (mount)
               mount->disableCollision();
            bool los = !gClientContainer.castRay(camPos, shapePos,losMask, &info);
            shape->enableCollision();
            if (mount)
               mount->enableCollision();
            if (!los)
               continue;

            // Project the shape pos into screen space and calculate
            // the distance opacity used to fade the labels into the
            // distance.
            Point3F projPnt;
            shapePos.z += mVerticalOffset;
            if (!parent->project(shapePos, &projPnt))
               continue;
            F32 opacity = (shapeDist < fadeDistance)? 1.0:
               1.0 - (shapeDist - fadeDistance) / (visDistance - fadeDistance);

            // Render the shape's name
            drawName(Point2I((S32)projPnt.x, (S32)projPnt.y),shape->getShapeName(),opacity);
         }
      }
   }

   // Restore control object collision
   control->enableCollision();

   // Border last
   if (mShowFrame)
      GFX->getDrawUtil()->drawRect(updateRect, mFrameColor.toColorI());
}
*/

void OhmShapeNameHud::onRender(Point2I, const RectI& updateRect)
{

   // Background fill first
   if (mShowFill)
      GFX->getDrawUtil()->drawRectFill(updateRect, mFillColor.toColorI());

   // Must be in a TS Control
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());
   if (!parent) return;

   // Must have a connection and control object
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return;

   ShapeBase* lControl = dynamic_cast<ShapeBase*>(conn->getControlObject()); // conn->getControlObject();
   //AIEntity* lControl = dynamic_cast<AIEntity*>(conn->getControlObject()); // conn->getControlObject();
   if (!lControl)
      return;


   // Get control camera info
   MatrixF cam;
   Point3F camPos;
   VectorF camDir;


   conn->getControlCameraTransform(0, &cam);
   cam.getColumn(3, &camPos);
   cam.getColumn(1, &camDir);

   F32 camFov;
   conn->getControlCameraFov(&camFov);
   camFov = mDegToRad(camFov) / 2;

   // Visible distance info & name fading
   //F32 visDistance = gClientSceneGraph->getVisibleDistance();
   F32 fadeDistance = mVisDistance * mDistanceFade;

   // Collision info. We're going to be running LOS tests and we
   // don't want to collide with the control object.
   //XXTH ORIG:   static U32 losMask = TerrainObjectType | ShapeBaseObjectType | StaticObjectType;
   //XXTH replaced with mLosMask static U32 losMask = TerrainObjectType | InteriorLikeObjectType  | StaticShapeObjectType | StaticObjectType;


   lControl->disableCollision();


   //selection helper 
   // AIEntity* firstPossibleObject = NULL;
   // AIEntity* foundObject = NULL;
   ShapeBase* firstPossibleObject = NULL;
   ShapeBase* foundObject = NULL;


   bool foundNewObject = false;
   bool foundOldObject = false;

   // All ghosted objects are added to the server connection group,
   // so we can find all the shape base objects by iterating through
   // our current connection.
   for (SimSetIterator itr(conn); *itr; ++itr)
   {

      // AIEntity* shape = dynamic_cast<AIEntity*>(*itr);
      ShapeBase* shape = dynamic_cast<ShapeBase*>(*itr);
      
      if (shape)
      {
         if (mOnlyPlayerObjects && !(shape->getTypeMask() & PlayerObjectType))
            continue;

         S32 playertype = shape->getPlayerType();


         if (/* 1.98 moved inside(shape != control || mShowMySelf) && */
            true /* 1.99 allways for damagenumbers !!! (shape->getShapeName() || (mSeekNewSelectedObject && (shape->getType() & PlayerObjectType )))*/)
         {

            // Target pos to test, if it's a player run the LOS to his eye
            // point, otherwise we'll grab the generic box .
            Point3F shapePos = Point3F(0.f, 0.f, 0.f);
            Point3F staticshapePos = Point3F(0.f, 0.f, 0.f); //1,98 new



            //reset shapepos 
            shape->setClientHudPosition(Point2I(-1, -1));


            //XXTH mUseStaticDisplay for non animated shapehud text on players!

            if ((shape->getTypeMask() & PlayerObjectType) && !mUseStaticDisplay)
            {
               ShapeBaseData* lData = static_cast<ShapeBaseData*>(shape->getDataBlock());
               bool lhaveEye = false;

               if (lData)
                  lhaveEye = (lData->eyeNode != -1);

               if (lhaveEye)
               {
                  MatrixF eye;
                  // Use the render eye transform, otherwise we'll see jittering
                  shape->getRenderEyeTransform(&eye);
                  eye.getColumn(3, &shapePos);
               }



               //1.98 new for shapehudposition
               MatrixF srtMat = shape->getRenderTransform();
               srtMat.getColumn(3, &staticshapePos);
               Box3F   bla = shape->getRenderWorldBox();
               
               staticshapePos.z = bla.maxExtents.z - mVerticalOffset;
               //we may have a eye somewhere in the head ... mhhh
               if (!lhaveEye || staticshapePos.z > shapePos.z)
               {
                  shapePos = staticshapePos;
               }


            }
            else
            {
               // Use the render transform instead of the box center
               // otherwise it'll jitter.

               MatrixF srtMat = shape->getRenderTransform();
               srtMat.getColumn(3, &shapePos);

               if (playertype == 3) { //XXTH adjust hudname on Harvest like before
                  shapePos.z += 1.5;
               }
               else {
                  Box3F   bla = shape->getRenderWorldBox();
                  shapePos.z = bla.maxExtents.z - mVerticalOffset;
               }
               staticshapePos = shapePos;
            }


            VectorF shapeDir = shapePos - camPos;

            // Test to see if it's in range
            F32 shapeDist = shapeDir.lenSquared();
            shapeDist = mSqrt(shapeDist);

            if (shapeDist == 0 || shapeDist > mVisDistance)
               continue;


            // Test to see if it's within our viewcone, this test doesn't
            // actually match the viewport very well, should consider
            // projection and box test.
            shapeDir.normalize();
            F32 dot = mDot(shapeDir, camDir);
            if (dot < camFov)
               continue;

            // Test to see if it's behind something, and we want to
            // ignore anything it's mounted on when we run the LOS.
            RayInfo info;
            shape->disableCollision();
            ShapeBase* mount = dynamic_cast<ShapeBase*>(shape->getObjectMount());

            if (mount)
               mount->disableCollision();

            //XXTH THIS IS WHERE IT GO INTO ENTLESS LOOP ?!?!?!?

            bool los = !gClientContainer.castRay(camPos, shapePos, mLosMask, &info);
            shape->enableCollision();
            if (mount)
               mount->enableCollision();

            if (!los)
               continue;

            // Project the shape pos into screen space and calculate
            // the distance opacity used to fade the labels into the
            // distance.
            Point3F projPnt;
            projPnt.zero();
            shapePos.z += mVerticalOffset;
            if (!parent->project(shapePos, &projPnt))
               continue;


            //new 1.98 ... 
            shape->setClientHudPosition(Point2I((S32)projPnt.x, (S32)projPnt.y));
            staticshapePos.z += mVerticalOffset;
            if (parent->project(staticshapePos, &projPnt)) {
               shape->setClientStaticHudPosition(Point2I((S32)projPnt.x, (S32)projPnt.y));
            }

            //XXTH 2.20 moved here 
            if (shape->getCloakedState()) //XXTH no name when cloaked
               continue;


            //XXTH 1.98 exit if it is control here!
            if (shape == lControl && !mShowMySelf)
               continue;

            F32 opacity = (shapeDist < fadeDistance) ? 1.0 :
               1.0 - (shapeDist - fadeDistance) / (mVisDistance - fadeDistance);

            bool isSelected = false;
            if (mShowSelectedObject && (shape == mSelectedObject || (mount && mount == mSelectedObject))) {
               isSelected = true;
            }


            //**** Object Selection ****
            if (mSeekNewSelectedObject  //when search is triggert
               && !foundNewObject  //only if not found
               && mSeekPlayerType[playertype] //check type is allowed to seek
               && !mount    //do not check a mount
               && shape != lControl->getObjectMount() //do not check the controls player mount
               && !shape->getDamageState() != ShapeBase::Enabled //2.01 
               )
            {
               if (!mSelectedObject) {
                  foundObject = shape;
                  foundNewObject = true;
               }
               else {
                  //set first possible if the selected is the last 
                  if (!mBackwardSeekObject && !firstPossibleObject && shape != mSelectedObject) {
                     firstPossibleObject = shape;
                  }
                  else  if (mBackwardSeekObject && shape != mSelectedObject)
                  {
                     //Seek every possible on backward search
                     firstPossibleObject = shape;
                  }



                  //we found the old in last run, set the new one
                  if (foundOldObject) {
                     foundObject = shape;
                     foundNewObject = true;
                  }
                  else if (shape == mSelectedObject) {
                     foundOldObject = true;
                     if (mBackwardSeekObject)
                     {
                        // If we have a firstPossibleObject on search 
                        // it should be the previous sice it skipped the selected object
                        // else the firstPossibleObject is filled up until loops ends 
                        // which is the last so this should be the previous before current selected
                        if (firstPossibleObject)
                        {
                           foundObject = firstPossibleObject;
                           foundNewObject = true;
                        }
                        else {
                           foundOldObject = false; //fake we did not find it so it loop to end
                        }
                     }

                  }
               }

            } //end of seek object for selection



               // Render the shape's name
            if (shape->getShapeName()) {
               if (mCustomDrawDamageName) {
                  //V2.20
                  Con::executef(this,  "onDrawDamageName",
                     Con::getIntArg(shape->getClientHudPosition().x), Con::getIntArg(shape->getClientHudPosition().y)
                     , shape->getIdString()
                     /*
                     , shape->getShapeName()
                     , shape->getGuildName()
                     , Con::getIntArg(shape->getPlayerType())
                     , Con::getIntArg(shape->getPlayerRank())
                     , Con::getIntArg(shape->getFaction())
                     , Con::getIntArg(shape->getEntity())
                     , Con::getFloatArg(shape->getDamageValue())
                     */
                     , Con::getFloatArg(opacity)
                     , Con::getIntArg(isSelected)

                  );


               }
               else {
                  //2.24 hack in faction 
                  /*
                     if (%playerType!=0 && %shape.getFaction() > 0 && getClientPlayer().getFaction() >0)
                     {
                       if (getClientPlayer().getFaction() == %shape.getFaction())
                       {
                         %color = "0 255 0" SPC %opa; //green
                         %barcolor = "0 180 0" SPC %opa;
                       } else {
                         %color = "255 0 0" SPC %opa; //red
                         %barcolor = "180 0 0" SPC %opa;
                       }
                     }
                  */

                  /* FIXME !!!! 
                  if (playertype == 1 && shape->getFaction() > 0 && control->getFaction() > 0)
                  {
                     if (control->getFaction() == shape->getFaction())
                     {
                        playertype = 0;
                     }
                     else {
                        playertype = 1;
                     }
                  }
                  */

                  drawDamageName(shape->getClientHudPosition(), shape->getShapeName(), shape->getGuildName(),
                     shape->getPlayerRank(), shape->getDamageValue(),
                     shape->getDamageLevel(), shape->getMaxDamage(), opacity, playertype, isSelected);
               }

            }



            /*
                     if(mShowDamage) {
                        drawDamageName(Point2I((S32)projPnt.x, (S32)projPnt.y),shape->getShapeName(),shape->getGuildName(), shape->getPlayerRank(), shape->getDamageValue(), shape->getDamageLevel(), shape->getMaxDamage(), opacity, playertype);
                       } else {
                           drawName(Point2I((S32)projPnt.x, (S32)projPnt.y),shape->getShapeName(),opacity, playertype);
                      }
            */
         } //if shapename
      }
   }

   // Restore control object collision
   lControl->enableCollision();

   if (mSeekNewSelectedObject) {
      mSeekNewSelectedObject = false;
      if (!foundNewObject && firstPossibleObject) {
         foundObject = firstPossibleObject;
      }
      S32 objId = 0;
      if (foundObject)
         objId = foundObject->getId();

    Con::executef(this,  "onNewSelectObject", Con::getIntArg(objId));

   }


   // Border last
   if (mShowFrame)
      GFX->getDrawUtil()->drawRect(updateRect, mFrameColor.toColorI());
}



//----------------------------------------------------------------------------
/// Render object names.
///
/// Helper function for OhmShapeNameHud::onRender
///
/// @param   offset  Screen coordinates to render name label. (Text is centered
///                  horizontally about this location, with bottom of text at
///                  specified y position.)
/// @param   name    String name to display.
/// @param   opacity Opacity of name (a fraction).
///
/*
void OhmShapeNameHud::drawName(Point2I offset, const char *name, F32 opacity)
{
   F32 width = mProfile->mFont->getStrWidth((const UTF8 *)name) + mLabelPadding.x * 2;
   F32 height = mProfile->mFont->getHeight() + mLabelPadding.y * 2;
   Point2I extent = Point2I(width, height);

   // Center the name
   offset.x -= width / 2;
   offset.y -= height / 2;

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   // Background fill first
   if (mShowLabelFill)
      drawUtil->drawRectFill(RectI(offset, extent), mLabelFillColor.toColorI());

   // Deal with opacity and draw.
   mTextColor.alpha = opacity;
   drawUtil->setBitmapModulation(mTextColor.toColorI());
   drawUtil->drawText(mProfile->mFont, offset + mLabelPadding, name);
   drawUtil->clearBitmapModulation();

   // Border last
   if (mShowLabelFrame)
      drawUtil->drawRect(RectI(offset, extent), mLabelFrameColor.toColorI());
}
*/
//XXTH
void OhmShapeNameHud::drawDamageName(Point2I offset, const char* name, const char* guild, U32 playerrank, F32 damagepercent, F32 damagelevel, F32 maxdamage, F32 opacity, S32 playerType, bool showSelection)
{
   if (!mDrawDamageName) //V2.20
      return;

   char buf[256];
   char buf2[256];
   bool drawDamage = mShowDamage;
   LinearColorF lDamageColor;

   /**
   2020-04-18 lol did not add factions here bsss need a client update again too bad
   */


   // Deal with opacity and draw.
   mTextColor.alpha = opacity;
   switch (playerType) {
   case 0: //player is green
      mTextColor.red = 0.f;
      mTextColor.green = 1.f;
      mTextColor.blue = 0.f;
      break;
   case 1: //bot is red
      mTextColor.red = 1.f;
      mTextColor.green = 0.f;
      mTextColor.blue = 0.f;
      break;
   case 2: //npc is yellow
      mTextColor.red = 1.f;
      mTextColor.green = 1.f;
      mTextColor.blue = 0.f;
      drawDamage = false;
      break;
   case 3: //harvest is magenta
      mTextColor.red = 0.f;
      mTextColor.green = 1.f;
      mTextColor.blue = 1.f;
      break;
   case 4: //farm is blue ==> https://auteria.com/forum/viewtopic.php?f=2&t=2218
      mTextColor.red = 0.25f; //0.f;
      mTextColor.green = 0.25f; //0.f;
      mTextColor.blue = 1.f;
      break;
   case 5: //misc is white or better light gray
      mTextColor.red = 0.8f;
      mTextColor.green = 0.8f;
      mTextColor.blue = 0.8f;
      drawDamage = false;
      break;
   }

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   if (drawDamage) {

      if (playerType == 0)
      {
         lDamageColor = mDamageFriendlyFillColor;
      }
      else {
         lDamageColor = mDamageFillColor;
      }


      offset.y -= mDamageRectSize.y * 2 + 4;

      lDamageColor.alpha = mDamageFrameColor.alpha = opacity;

      // Damage should be 0->1 (0 being no damage,or healthy), but
      // we'll just make sure here as we flip it.
      damagepercent = mClampF(1 - damagepercent, 0, 1);

      // Center the bar
      RectI rect(offset, mDamageRectSize);
      rect.point.x -= mDamageRectSize.x / 2;


      


      // Draw the border
      drawUtil->drawRect(rect, mDamageFrameColor.toColorI());

      // Draw the damage % fill
      rect.point += Point2I(1, 1);
      rect.extent -= Point2I(1, 1);
      rect.extent.x = (S32)(rect.extent.x * damagepercent);
      if (rect.extent.x == 1)
         rect.extent.x = 2;
      if (rect.extent.x > 0)
         drawUtil->drawRectFill(rect, lDamageColor.toColorI());
   }

   if (showSelection) {
      dSprintf(buf, sizeof(buf), "[ %s ]", name);
   }
   else {
      dSprintf(buf, sizeof(buf), "%s", name);
   }
   Con::stripColorChars(buf);
   U32 namewidth = mProfile->mFont->getStrWidth((const UTF8*)buf);

   //XXTH 1.95 new code >>>>>>>>>>>>>>>>>>>><<

   U32 guildwidth = 0;
   U32 fontheight = mProfile->mFont->getHeight();

   if (guild) {
      dSprintf(buf2, sizeof(buf2), " %s", guild);
      guildwidth = mProfile->mFont->getStrWidth((const UTF8*)buf2);
      offset.y -= fontheight * 2;
   }
   else {
      offset.y -= fontheight;
   }

   U32 oldOffsetX = offset.x;
   offset.x -= (namewidth) / 2;
   // Deal with opacity and draw.
   mTextColor.alpha = opacity;
   drawUtil->setBitmapModulation(mTextColor.toColorI());
   drawUtil->drawText(mProfile->mFont, offset + mLabelPadding, name);
   drawUtil->clearBitmapModulation();


   //draw guild 
   if (guild) {
      LinearColorF mGuildColor;
      mGuildColor.alpha = opacity;

      //Client must have Simobjects: RedGuilds and BlueGuilds
     /*   const char *fieldName = StringTable->insert( argv[2] );
        return object->getDataField( fieldName, NULL ); */
      SimObject* ColorGuilds;
      bool GuildColorSet = false;
      if (Sim::findObject("ClientGuildColors", ColorGuilds)) {
         const char* fieldName = StringTable->insert(guild);
         const char* strColor = ColorGuilds->getDataField(fieldName, NULL);

         if (dStrcmp(strColor, "") != 0) {
            GuildColorSet = true;
            dSscanf(strColor, "%f %f %f", &mGuildColor.red, &mGuildColor.green, &mGuildColor.blue);
         }

      }


      if (!GuildColorSet) {

         mGuildColor.red = 0.65f;
         mGuildColor.green = 0.65f;
         mGuildColor.blue = 0.65f;
      }

      offset.y += fontheight;
      offset.x = oldOffsetX - guildwidth / 2;

      
      
      

      //dglSetBitmapModulation(mGuildColor);
      drawUtil->setBitmapModulation(mGuildColor.toColorI());
      //dglDrawText(mProfile->mFont, offset, buf2);
      drawUtil->drawText(mProfile->mFont, offset + mLabelPadding, buf2);
      //dglClearBitmapModulation();
      drawUtil->clearBitmapModulation();

   }




   /* XXTH 1.95 rewrite for guild under name  old code:
      U32 guildwidth;
      if (guild) {
        dSprintf(buf2, sizeof(buf2), " %s", guild);
        guildwidth= mProfile->mFont->getStrWidth((const UTF8 *)buf);
        offset.x -= (namewidth + guildwidth)/ 2;
      } else {
       offset.x -= (namewidth)/ 2;
      }

      offset.y -= mProfile->mFont->getHeight();

      dglSetBitmapModulation(mTextColor);
      dglDrawText(mProfile->mFont, offset, buf);
      dglClearBitmapModulation();


      //draw guild
      if (guild) {
        ColorF mGuildColor;
        mGuildColor.alpha = opacity;
        mGuildColor.red=1;
        mGuildColor.green=1;
        mGuildColor.blue=1;

        offset.x += namewidth;

        dglSetBitmapModulation(mGuildColor);
        dglDrawText(mProfile->mFont, offset, buf2);
        dglClearBitmapModulation();
      }
   */

}


//--------------------------------------------------------------------------
// target selection XXTH
//-------------------------------------------------------------------------
/* 
DefineEngineStringlyVariadicMethod(OhmShapeNameHud, setSelectedObject, void, 3, 3, "id of object, call me when selection has changed!")
{
   if (!Sim::findObject(argv[2], object->mSelectedObject)) {
      object->mSelectedObject = 0;
   }
}
*/

DefineEngineMethod(OhmShapeNameHud, setSelectedObject, bool, (ShapeBase* lObject), , "id of object, call me when selection has changed!")
{
   if (lObject) {
      object->mSelectedObject = lObject;
   }
   object->mSelectedObject = NULL;
   return false;
}


/*
DefineEngineStringlyVariadicMethod(OhmShapeNameHud, seekNewSelectedObject, void, 2, 2, "")
{
   object->mBackwardSeekObject = false;
   object->mSeekNewSelectedObject = true;
}
*/
DefineEngineMethod(OhmShapeNameHud, seekNewSelectedObject, void, (), , "seek for a object - normaly used when pressing TAB")
{
   object->mBackwardSeekObject = false;
   object->mSeekNewSelectedObject = true;
}


/*
DefineEngineStringlyVariadicMethod(OhmShapeNameHud, seekNewSelectedObjectBackward, void, 2, 2, "")
{
   object->mBackwardSeekObject = true;
   object->mSeekNewSelectedObject = true;

}
*/

DefineEngineMethod(OhmShapeNameHud, seekNewSelectedObjectBackward, void, (), , "seek for a object reverse - normaly used when pressing shift TAB")
{
   object->mBackwardSeekObject = true;
   object->mSeekNewSelectedObject = true;
}


//--------------------------------------------------------------------------
// input to playgui 
//--------------------------------------------------------------------------
bool OhmShapeNameHud::onInputEvent(const InputEventInfo& event)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      return parent->onInputEvent(event);

   return false;
}

//--------------------------------------------------------------------------
// object selection additions XXTH
//--------------------------------------------------------------------------
void OhmShapeNameHud::onMouseDown(const GuiEvent& evt)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onMouseDown(evt);
}

void OhmShapeNameHud::onMouseDragged(const GuiEvent& event)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onMouseDragged(event);
}
void OhmShapeNameHud::onMouseUp(const GuiEvent& event)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onMouseUp(event);
}
void OhmShapeNameHud::onMouseMove(const GuiEvent& evt)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onMouseMove(evt);
}
void OhmShapeNameHud::onRightMouseDown(const GuiEvent& event)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onRightMouseDown(event);
}
void OhmShapeNameHud::onRightMouseUp(const GuiEvent& event)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onRightMouseUp(event);
}
void OhmShapeNameHud::onRightMouseDragged(const GuiEvent& event)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onRightMouseDragged(event);
}


void OhmShapeNameHud::onMiddleMouseDown(const GuiEvent& event)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onMiddleMouseDown(event);
}
void OhmShapeNameHud::onMiddleMouseUp(const GuiEvent& event)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onMiddleMouseUp(event);
}
void OhmShapeNameHud::onMiddleMouseDragged(const GuiEvent& event)
{
   // Let's let the parent execute its event handling (if any)
   GuiTSCtrl* parent = dynamic_cast<GuiTSCtrl*>(getParent());

   if (parent)
      parent->onMiddleMouseDragged(event);
}

//--------------------------------------------------------------------------
// config the LosMask 
//--------------------------------------------------------------------------
//XXTH mLosMask
DefineEngineMethod(OhmShapeNameHud, setLosMask, void, (U32 lLosMask), , "Set the losmask for the shapenamehud. Use function BIT() and look at objecttypes.h.")
{
   object->setLosMask(lLosMask);
}


