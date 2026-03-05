//-----------------------------------------------------------------------------
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
/**
 *  2021-03-02 XXTH lot of changed to display name and NOT display damage, also
 *                  added distance. Should be moved in a different class,,,
 *                  but since old behaviour is still default it's ok 
 */
//-----------------------------------------------------------------------------
#include "platform/platform.h"

#include "gui/core/guiControl.h"
#include "gui/controls/guiBitmapCtrl.h"
#include "console/consoleTypes.h"
#include "scene/sceneManager.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"

//XXTH guiTextureCanvas
// #ifdef _res_guiTextureCanvas
#include "gui/core/guiTextureCanvas.h"
// #endif
#include "T3D/tsStatic.h"

//-----------------------------------------------------------------------------
/// Vary basic cross hair hud.
/// Uses the base bitmap control to render a bitmap, and decides whether
/// to draw or not depending on the current control object and it's state.
/// If there is ShapeBase object under the cross hair and it's named,
/// then a small health bar is displayed.
class GuiCrossHairHud : public GuiBitmapCtrl
{
   typedef GuiBitmapCtrl Parent;

   LinearColorF   mDamageFillColor;
   LinearColorF   mDamageFrameColor;
   Point2I  mDamageRectSize;
   Point2I  mDamageOffset;

   //XXTH guiTextureCanvas
// #ifdef _res_guiTextureCanvas
   PlatformTimer* mFrameTime;
// #endif
   //XXTH
   bool mDrawShapename;
   bool mDrawDamage;
   bool mAllow3thperson;
//   bool mIgnoreControlType; //XXTH for afxCamera
   F32 mObjectDistance;
   LinearColorF   mTextColor;
   Point2I  mLabelPadding;
   ShapeBase* mCursorObject;

protected:
   void drawDamage(Point2I offset, F32 damage, F32 opacity);
   void drawName(Point2I offset, const char* name, F32 opacity);

public:
   GuiCrossHairHud();

   //XXTH
   ShapeBase* getCursorObject() { return mCursorObject; }

   void onRender( Point2I, const RectI &);
//XXTH guiTextureCanvas
// #ifdef _res_guiTextureCanvas
   bool testGuiInteraction();
// #endif
   static void initPersistFields();
   DECLARE_CONOBJECT( GuiCrossHairHud );
   DECLARE_CATEGORY( "Gui Game" );
   DECLARE_DESCRIPTION( "Basic cross hair hud. Reacts to state of control object.\n"
      "Also displays health bar for named objects under the cross hair." );
};

/// Valid object types for which the cross hair will render, this
/// should really all be script controlled.
static const U32 ObjectMask = PlayerObjectType | VehicleObjectType;


//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT( GuiCrossHairHud );

ConsoleDocClass( GuiCrossHairHud,
   "@brief Basic cross hair hud. Reacts to state of control object. Also displays health bar for named objects under the cross hair.\n\n"
   "Uses the base bitmap control to render a bitmap, and decides whether to draw or not depending "
   "on the current control object and it's state. If there is ShapeBase object under the cross hair "
   "and it's named, then a small health bar is displayed.\n\n"
     
   "@tsexample\n"
		"\n new GuiCrossHairHud()"
		"{\n"
		"	damageFillColor = \"1.0 0.0 0.0 1.0\"; // Fills with a solid red color\n"
		"	damageFrameColor = \"1.0 1.0 1.0 1.0\"; // Solid white frame color\n"
		"	damageRect = \"15 5\";\n"
		"	damageOffset = \"0 -10\";\n"
		"};\n"
   "@endtsexample\n"
   
   "@ingroup GuiGame\n"
);

GuiCrossHairHud::GuiCrossHairHud()
{
   mDamageFillColor.set( 0.0f, 1.0f, 0.0f, 1.0f );
   mDamageFrameColor.set( 1.0f, 0.6f, 0.0f, 1.0f );
   mDamageRectSize.set(50, 4);
   mDamageOffset.set(0,32);

   //XXTH guiTextureCanvas
// #ifdef _res_guiTextureCanvas
   mFrameTime = PlatformTimer::create();
// #endif

   //XXTH
   mDrawShapename = false;
   mDrawDamage = true;
   mAllow3thperson = false;
//   mIgnoreControlType = false;
   mTextColor.set(0, 1, 0, 1);
   mLabelPadding.set(0, 0);
   mCursorObject = NULL;
   mObjectDistance = 0.f;
}

void GuiCrossHairHud::initPersistFields()
{
   addGroup("Ohmtal");
   addField("ObjectDistance", TypeF32, Offset(mObjectDistance, GuiCrossHairHud), "Distance where Object get tracked. 0=visible Distance");
   addField("Allow3thperson", TypeBool, Offset(mAllow3thperson, GuiCrossHairHud), "render on 3thperson");
//   addField("IgnoreControlType", TypeBool, Offset(mIgnoreControlType, GuiCrossHairHud), "render on ALL controltypes for afxCamera");
   
   addField("ShapenameEnable", TypeBool, Offset(mDrawShapename, GuiCrossHairHud), "Shapename rendered");
   addField("DamageBarEnable", TypeBool, Offset(mDrawDamage, GuiCrossHairHud), "Damage bar rendered");
   addField("textColor", TypeColorF, Offset(mTextColor, GuiCrossHairHud), "Color for the text on this control.");
   addField("labelPadding", TypePoint2I, Offset(mLabelPadding, GuiCrossHairHud), "The padding (in pixels) between the label text and the frame.");
   endGroup("Ohmtal");
   addGroup("Damage");		
   addField( "damageFillColor", TypeColorF, Offset( mDamageFillColor, GuiCrossHairHud ), "As the health bar depletes, this color will represent the health loss amount." );
   addField( "damageFrameColor", TypeColorF, Offset( mDamageFrameColor, GuiCrossHairHud ), "Color for the health bar's frame." );
   addField( "damageRect", TypePoint2I, Offset( mDamageRectSize, GuiCrossHairHud ), "Size for the health bar portion of the control." );
   addField( "damageOffset", TypePoint2I, Offset( mDamageOffset, GuiCrossHairHud ), "Offset for drawing the damage portion of the health control." );
   endGroup("Damage");

   

   Parent::initPersistFields();
}


//-----------------------------------------------------------------------------
//XXTH guiTextureCanvas
// #ifdef _res_guiTextureCanvas
bool GuiCrossHairHud::testGuiInteraction()

{

   if (mFrameTime->getElapsedMs() < 16)
      return false;
   mFrameTime->reset();

   bool interacting = false;
   // Must have a connection and player control object
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return false;
   ShapeBase* control = dynamic_cast<ShapeBase*>(conn->getControlObject());
   if (!control || !(control->getTypeMask() & ObjectMask) || !conn->isFirstPerson())
      return false;
   
   // Get control camera info
   MatrixF cam;
   Point3F camPos;
   conn->getControlCameraTransform(0, &cam);
   cam.getColumn(3, &camPos);

   // Extend the camera vector to create an endpoint for our ray
   Point3F endPos;
   cam.getColumn(1, &endPos);
   endPos *= GuiTextureCanvas::getRayLength();
   endPos += camPos;

   // Collision info. We're going to be running LOS tests and we
   // don't want to collide with the control object.
   static U32 losMask = TerrainObjectType | ShapeBaseObjectType | StaticShapeObjectType;
   control->disableCollision();

   RayInfo info;
   info.generateTexCoord = true;
   if (gClientContainer.castRay(camPos, endPos, losMask, &info)) {
      // Hit something... but we'll only display health for named
      // ShapeBase objects.  Could mask against the object type here
      // and do a static cast if it's a ShapeBaseObjectType, but this
      // isn't a performance situation, so I'll just use dynamic_cast.
      if (TSStatic * obj = dynamic_cast<TSStatic*>(info.object))
         interacting = GuiTextureCanvas::castRay(obj, camPos, endPos, &info);
   }

   // Restore control object collision
   control->enableCollision();
   GuiTextureCanvas::setInteract(interacting, &info);
   return interacting;
}
// #endif
//-----------------------------------------------------------------------------

void GuiCrossHairHud::onRender(Point2I offset, const RectI &updateRect)
{
   // Must have a connection and player control object
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn)
      return;
   GameBase* control = dynamic_cast<GameBase*>(conn->getCameraObject());

   /*
     XXTH afxCamera problem :
     control is afxCamera but OnjectMask ist only Player and Vehicle

   debug: 
   bool l1 = !control;
   bool l2 = (!conn->isFirstPerson() && !mAllow3thperson);
   bool l3 = !(control->getTypeMask() & ObjectMask);


   lol next problem with afxCamera => control ...
      * The player is not ignored since it is not the control 

   */
   

   if (!control || (!conn->isFirstPerson() && !mAllow3thperson) || !(control->getTypeMask() & ObjectMask))
      return;

   //XXTH stupid string compare ... ^^ better do something like ignore controlType
   /*
   if (!(control->getTypeMask() & ObjectMask) && dStrcmp("afxCamera", control->getClassName()) != 0)
      return;

   was not bad but does not work 
      if (!mIgnoreControlType && !(control->getTypeMask() & ObjectMask) )
      return;

   */

 //XXTH guiTextureCanvas
 //if we're interacting with a gui, skip rendering
// #ifdef _res_guiTextureCanvas
   if (testGuiInteraction())
      return;
// #endif


   // Parent render.
   Parent::onRender(offset,updateRect);

   // Get control camera info
   MatrixF cam;
   Point3F camPos;
   conn->getControlCameraTransform(0,&cam);
   cam.getColumn(3, &camPos);

   // Extend the camera vector to create an endpoint for our ray
   Point3F endPos;
   cam.getColumn(1, &endPos);
   if (mObjectDistance > 0.f)
   {
      endPos *= mObjectDistance;
   }
   else {
      endPos *= gClientSceneGraph->getVisibleDistance();
   }
   
   endPos += camPos;

   // Collision info. We're going to be running LOS tests and we
   // don't want to collide with the control object.
//XXTH guiTextureCanvas
// #ifdef _res_guiTextureCanvas
   static U32 losMask = TerrainObjectType | ShapeBaseObjectType | StaticShapeObjectType;
// #else
//    static U32 losMask = TerrainObjectType | ShapeBaseObjectType;
// #endif
   control->disableCollision();

   mCursorObject = NULL;

   RayInfo info;
   if (gClientContainer.castRay(camPos, endPos, losMask, &info)) {
      // Hit something... but we'll only display health for named
      // ShapeBase objects.  Could mask against the object type here
      // and do a static cast if it's a ShapeBaseObjectType, but this
      // isn't a performance situation, so I'll just use dynamic_cast.
      if (ShapeBase* obj = dynamic_cast<ShapeBase*>(info.object))
      {
         if (obj->getShapeName()) {
            offset.x = updateRect.point.x + updateRect.extent.x / 2;
            offset.y = updateRect.point.y + updateRect.extent.y / 2;

            if (mDrawDamage)
               drawDamage(offset + mDamageOffset, obj->getDamageValue(), 1);

            //XXTH lol space is too small
            if (mDrawShapename)
               drawName(offset, obj->getShapeName(), 1);
         }
         mCursorObject = obj;
      }
   }

   // Restore control object collision
   control->enableCollision();
}


//-----------------------------------------------------------------------------
/**
   Display a damage bar ubove the shape.
   This is a support funtion, called by onRender.
*/
void GuiCrossHairHud::drawDamage(Point2I offset, F32 damage, F32 opacity)
{
   mDamageFillColor.alpha = mDamageFrameColor.alpha = opacity;

   // Damage should be 0->1 (0 being no damage,or healthy), but
   // we'll just make sure here as we flip it.
   damage = mClampF(1 - damage, 0, 1);

   // Center the bar
   RectI rect(offset, mDamageRectSize);
   rect.point.x -= mDamageRectSize.x / 2;

   // Draw the border
   GFX->getDrawUtil()->drawRect(rect, mDamageFrameColor.toColorI());

   // Draw the damage % fill
   rect.point += Point2I(1, 1);
   rect.extent -= Point2I(1, 1);
   rect.extent.x = (S32)(rect.extent.x * damage);
   if (rect.extent.x == 1)
      rect.extent.x = 2;
   if (rect.extent.x > 0)
      GFX->getDrawUtil()->drawRectFill(rect, mDamageFillColor.toColorI());
}


//----------------------------------------------------------------------------
/// Render object names.
/// XXTH added to crosshair!!!
/// Helper function for GuiShapeNameHud::onRender
///
/// @param   offset  Screen coordinates to render name label. (Text is centered
///                  horizontally about this location, with bottom of text at
///                  specified y position.)
/// @param   name    String name to display.
/// @param   opacity Opacity of name (a fraction).
void GuiCrossHairHud::drawName(Point2I offset, const char* name, F32 opacity)
{
   F32 width = mProfile->mFont->getStrWidth((const UTF8*)name) + mLabelPadding.x * 2;
   F32 height = mProfile->mFont->getHeight() + mLabelPadding.y * 2;
   Point2I extent = Point2I(width, height);

   // Center the name
   offset.x -= width / 2;
   offset.y -= height / 2;

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   // Background fill first
   /* FIXME 
   if (mShowLabelFill)
      drawUtil->drawRectFill(RectI(offset, extent), mLabelFillColor.toColorI());
   */

   // Deal with opacity and draw.
   mTextColor.alpha = opacity;
   drawUtil->setBitmapModulation(mTextColor.toColorI());
   drawUtil->drawText(mProfile->mFont, offset + mLabelPadding, name);
   drawUtil->clearBitmapModulation();

   // Border last
   /* FIXME 
   if (mShowLabelFrame)
      drawUtil->drawRect(RectI(offset, extent), mLabelFrameColor.toColorI());
      */
}
// ShapeBase* mCursorObject;
DefineEngineMethod(GuiCrossHairHud, getCursorObject, S32, (), , "get the current crosshair object ob client")
{

   ShapeBase* target = object->getCursorObject();
   return (target) ? target->getId() : -1;

   
}
