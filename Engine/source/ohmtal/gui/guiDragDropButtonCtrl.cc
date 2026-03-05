//-----------------------------------------------------------------------------
// Copyright (c) Jarred Schnelle, updated by Stefan "Beffy" Moises
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
// written by Jarred Schnelle, updated by Stefan "Beffy" Moises
/* 
  2019-02-17 XXTH Added cooldown functionality from afxCoolDownBitmap 
		only rendered when wrap is off

  2020-05-19 ported to T3D
*/
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "console/consoleTypes.h"
//#include "dgl/dgl.h"
//#include "platform/platformAudio.h"  
#include "gui/core/guiCanvas.h"           
#include "guiDragDropButtonCtrl.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "afx/arcaneFX.h" //XXTH CoolDown
#include "sfx/sfxSystem.h"
#include "sfx/sfxTrack.h"

//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(GuiDragDropButtonCtrl);

StringTableEntry GuiDragDropButtonCtrl::sSpellCooldownBitmaps = ""; // XXTH Cooldown

//-----------------------------------------------------------------------------
GuiDragDropButtonCtrl::GuiDragDropButtonCtrl(void)
{
   mBitmapName = StringTable->insert("");
   mHoverBitmapName = StringTable->insert("");
   mDownBitmapName = StringTable->insert("");
   startPoint.set(0, 0);
   mWrap = false;
   mIsButtonMoving = false;
   mCanMove = true;
   mCopySource = true;
   mIsReceiver = true;
   mSnapsBack = true;
   mResetSourceOnDrop = false;
   mPerformActionOnUp = true;
   mDropBorderSearchColor.set(255,0,0);
   mDropBorderColor.set(0,0,0);
   mDropBorder = true;
   mDropBorderType = StringTable->insert("defaultDrop");
   mDropBorderSearch = StringTable->insert("defaultDrop");
   mDropBorderDisplayed = false;
   mSwitchImages = true;

   mIsDragged = false;

//XXTH Cooldown -->
  if (sSpellCooldownBitmaps == NULL)
    sSpellCooldownBitmaps = ST_NULLSTRING;
  setExtent(30, 30);
  
  
  mTimeOffset = 0;
  mCoolDown   = 0;
  mRunning    = false;
  mReverseAnimation = false;
//XXTH <-- Cooldown 
}


void GuiDragDropButtonCtrl::initPersistFields()
{
   Parent::initPersistFields();
   addField("Bitmap", TypeFilename, Offset(mBitmapName, GuiDragDropButtonCtrl));
   addField("HoverBitmap", TypeFilename, Offset(mHoverBitmapName, GuiDragDropButtonCtrl));
   addField("DownBitmap", TypeFilename, Offset(mDownBitmapName, GuiDragDropButtonCtrl));
   addField("canMove",   TypeBool,     Offset(mCanMove, GuiDragDropButtonCtrl));
   addField("isReceiver", TypeBool, Offset(mIsReceiver, GuiDragDropButtonCtrl));
   addField("snapsBack", TypeBool, Offset(mSnapsBack, GuiDragDropButtonCtrl));
   addField("copySource", TypeBool, Offset(mCopySource, GuiDragDropButtonCtrl));
   addField("switchImages", TypeBool, Offset(mSwitchImages, GuiDragDropButtonCtrl));
   addField("resetSourceOnDrop", TypeBool, Offset(mResetSourceOnDrop, GuiDragDropButtonCtrl));
   addField("dropBorder", TypeBool, Offset(mDropBorder, GuiDragDropButtonCtrl));
   addField("dropBorderSearchColor", TypeColorI, Offset(mDropBorderSearchColor, GuiDragDropButtonCtrl));
   addField("dropBorderType", TypeString, Offset(mDropBorderType, GuiDragDropButtonCtrl));
   addField("dropBorderSearch", TypeString, Offset(mDropBorderSearch, GuiDragDropButtonCtrl));   
   addField("performActionOnUp", TypeBool, Offset(mPerformActionOnUp, GuiDragDropButtonCtrl));

//XXTH Cooldown -->
  addGroup("CoolDown");
    addField("ReverseAnimation",   TypeBool,    Offset(mReverseAnimation, GuiDragDropButtonCtrl));
  endGroup("CoolDown");

  Con::addVariable("pref::GuiDragDropButtonCtrl::spellCooldownBitmaps", TypeString, &sSpellCooldownBitmaps);
//XXTH <-- Cooldown 
}

// ----------------------------------------------------------
// Console functions
// ----------------------------------------------------------

DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setValue, void, 4, 4, "(int a, int b)"
              "Set the value of the dragdrop control.")
{
   object->setValue(dAtoi(argv[2]), dAtoi(argv[3]));
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setBitmap, void, 3, 3, "(string name)"
              "Set the bitmap of the dragdrop control.")
{
   object->setBitmapName(argv[2]);
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setHoverBitmap, void, 3, 3, "(string name)"
              "Set the hover bitmap of the dragdrop control.")
{
   object->setHoverBitmapName(argv[2]);
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setDownBitmap, void, 3, 3, "(string name)"
              "Set the down bitmap of the dragdrop control.")
{
   object->setDownBitmapName(argv[2]);
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setCanMove, void, 3, 3, "(bool canMove)"
              "Set the dragdrop control moveable.")
{
   object->setCanMove(dAtob(argv[2]));
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setIsReceiver, void, 3, 3, "(bool isReceiver)"
              "Set the dragdrop control as receiver.")
{
   object->setIsReceiver(dAtob(argv[2]));
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setSnapsBack, void, 3, 3, "(bool snapsBack)"
              "Set the dragdrop control to snap back.")
{
   object->setSnapsBack(dAtob(argv[2]));
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setDropBorder, void, 3, 3, "(bool dropBorder)"
              "Set the dragdrop control to drop the border.")
{
   object->setDropBorder(dAtob(argv[2]));
}

DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setDropBorderSearchColor, void, 6, 6, "(bool dropBorder)"
              "Set the border search color of the dragdrop control.")
{
   if(argc > 5)
      object->setDropBorderSearchColor(dAtoi(argv[2]), dAtoi(argv[3]), dAtoi(argv[4]), dAtoi(argv[5]));
   else
      object->setDropBorderSearchColor(dAtoi(argv[2]), dAtoi(argv[3]), dAtoi(argv[4]));
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setDropBorderType, void, 3, 3, "(string type)"
              "Set the drop border type of the dragdrop control.")
{
   object->setDropBorderType(argv[2]);
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, setDropBorderSearch, void, 3, 3, "(string search)"
              "Set the drop border search of the dragdrop control.")
{
   object->setDropBorderSearch(argv[2]);
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getBitmap, const char*, 2, 2, ""
              "Get the bitmap of the dragdrop control.")
{
   const char* temp = object->getNormalName();
   return temp;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getHoverBitmap, const char*, 2, 2, ""
              "Get the hover bitmap of the dragdrop control.")
{
   const char* temp = object->getHoverName();
   return temp;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getDownBitmap, const char*, 2, 2, ""
              "Get the down bitmap of the dragdrop control.")
{
   const char* temp = object->getDownName();
   return temp;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getCanMove, const char*, 2, 2, ""
              "Get the move capability of the dragdrop control.")
{
   bool temp = object->getCanMove();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%b", temp);
   return returnBuffer;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getSnapsBack, const char*, 2, 2, ""
              "Get the snap back capability of the dragdrop control.")
{
   bool temp = object->getSnapsBack();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%b", temp);
   return returnBuffer;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getIsReceiver, const char*, 2, 2, ""
              "Get the receiver capability of the dragdrop control.")
{
   bool temp = object->getIsReceiver();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%b", temp);
   return returnBuffer;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getCopySource, const char*, 2, 2, ""
              "Get the copy source capability of the dragdrop control.")
{
   bool temp = object->getCopySource();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%b", temp);
   return returnBuffer;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getDropBorder, const char*, 2, 2, ""
              "Get the drop border capability of the dragdrop control.")
{
   bool temp = object->getDropBorder();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%b", temp);
   return returnBuffer;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getDropBorderType, const char*, 2, 2, ""
              "Get the drop border type of the dragdrop control.")
{
   bool temp = object->getDropBorderType();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%b", temp);
   return returnBuffer;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getDropBorderSearch, const char*, 2, 2, ""
              "Get the drop border search of the dragdrop control.")
{
   bool temp = object->getDropBorderSearch();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%b", temp);
   return returnBuffer;
}
DefineEngineStringlyVariadicMethod( GuiDragDropButtonCtrl, getDropBorderSearchColor, const char*, 2, 2, ""
              "Get the drop border search color of the dragdrop control.")
{
   ColorI temp = object->getDropBorderSearchColor();

   char* returnBuffer = Con::getReturnBuffer(256);
   dSprintf(returnBuffer,256,"%d %d %d %d", temp.red, temp.green, temp.blue, temp.alpha);
   return returnBuffer;
}





//CanMove Member Function - Setter
void GuiDragDropButtonCtrl::setCanMove(bool temp)
{
   mCanMove = temp;
}

//IsReceiver Member Function - Setter
void GuiDragDropButtonCtrl::setIsReceiver(bool temp)
{
   mIsReceiver = temp;
}

//SnapsBack Member Function - Setter
void GuiDragDropButtonCtrl::setSnapsBack(bool temp)
{
   mSnapsBack = temp;
}

//DropBorder Member Function - Setter
void GuiDragDropButtonCtrl::setDropBorder(bool temp)
{
   mDropBorder = temp;
}

//DropBorderSearchColor Member Function - Setter
void GuiDragDropButtonCtrl::setDropBorderSearchColor(int r, int g, int b)
{
   mDropBorderSearchColor.set(r, g, b);
}

void GuiDragDropButtonCtrl::setDropBorderSearchColor(int r, int g, int b, int a)
{
   mDropBorderSearchColor.set(r, g, b, a);
}

//DropBorderType Member Function - Setter
void GuiDragDropButtonCtrl::setDropBorderType(const char* temp)
{
   mDropBorderType = temp;
}


//DropBorderSearch Member Function - Setter
void GuiDragDropButtonCtrl::setDropBorderSearch(const char* temp)
{
   mDropBorderSearch = temp;
}


void GuiDragDropButtonCtrl::consoleInit()
{
}

bool GuiDragDropButtonCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   setActive(true);
   setBitmapName(mBitmapName);
   setHoverBitmapName(mHoverBitmapName);
   setDownBitmapName(mDownBitmapName);
   return true;
}

void GuiDragDropButtonCtrl::onSleep()
{
   mTextureHandle = NULL;
   mHoverTextureHandle = NULL;
   mDownTextureHandle = NULL;
   mNormalTextureHandle = NULL;
   Parent::onSleep();
}

void GuiDragDropButtonCtrl::setBitmapName(const char *name)
{
   mBitmapName = StringTable->insert(name);
   if (*mBitmapName)
      mTextureHandle = GFXTexHandle(mBitmapName, &GFXStaticTextureSRGBProfile, avar("%s() - mTextureHandle (line %d)", __FUNCTION__, __LINE__));
   else
      mTextureHandle = NULL;

   mNormalTextureHandle = mTextureHandle; //Store the normal(default) texture for our up/down/enter/leave events

   //XXTH new bitmap guess the cooldown is over ^^ 
   /*
	if (mRunning)
		mRunning=false;	
   */

   setUpdate();
}

void GuiDragDropButtonCtrl::setHoverBitmapName(const char *name)
{
   mHoverBitmapName = StringTable->insert(name);
   if (*mHoverBitmapName)
      mHoverTextureHandle = GFXTexHandle(mHoverBitmapName, &GFXStaticTextureSRGBProfile, avar("%s() - mTextureHandle (line %d)", __FUNCTION__, __LINE__));
   else
      mHoverTextureHandle = NULL;
   setUpdate();
}   

void GuiDragDropButtonCtrl::setDownBitmapName(const char *name)
{
   mDownBitmapName = StringTable->insert(name);
   if (*mDownBitmapName)
      mDownTextureHandle = GFXTexHandle(mDownBitmapName, &GFXStaticTextureSRGBProfile, avar("%s() - mTextureHandle (line %d)", __FUNCTION__, __LINE__));
   else
      mDownTextureHandle = NULL;
   setUpdate();
} 

void GuiDragDropButtonCtrl::setBitmapHandle(const GFXTexHandle &handle)
{
   mTextureHandle = handle;
   mNormalTextureHandle = mTextureHandle;
}   

void GuiDragDropButtonCtrl::setHoverBitmapHandle(const GFXTexHandle &handle)
{
   mHoverTextureHandle = handle;   
}

void GuiDragDropButtonCtrl::setDownBitmapHandle(const GFXTexHandle &handle)
{
   mDownTextureHandle = handle;   
}


void GuiDragDropButtonCtrl::setTextureHandle(const GFXTexHandle &handle)
{
   mTextureHandle = handle;
}
//---------------------------------------------------------------------------------------------

void GuiDragDropButtonCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   if (mTextureHandle)
   {
      GFX->getDrawUtil()->clearBitmapModulation();
      if(mWrap)
      {
         GFXTextureObject* texture = (GFXTextureObject *) mTextureHandle;
         RectI srcRegion;
         RectI dstRegion;
         
         float xdone = ((float)getBounds().extent.x/(float)texture->getBitmapWidth())+1;
         float ydone = ((float)getBounds().extent.y/(float)texture->getBitmapHeight())+1;

         int xshift = startPoint.x%texture->getBitmapWidth();
         int yshift = startPoint.y%texture->getBitmapHeight();
         for(int y = 0; y < ydone; ++y)
            for(int x = 0; x < xdone; ++x)
            {
               srcRegion.set(0,0,texture->getBitmapWidth(),texture->getBitmapHeight());
               dstRegion.set( ((texture->getBitmapWidth() *x)+offset.x)-xshift,
                  ((texture->getBitmapHeight() * y)+offset.y)-yshift,
                  texture->getBitmapWidth(),
                  texture->getBitmapHeight());
               //dglDrawBitmapStretchSR(texture,dstRegion, srcRegion, false);
               GFX->getDrawUtil()->drawBitmapStretchSR(texture, dstRegion, srcRegion);
            }
      }
      else
      {
         RectI rect(offset, getBounds().extent);

         GFX->getDrawUtil()->drawBitmapStretch(mTextureHandle, rect);

         //dglDrawBitmapStretch(mTextureHandle, rect);
		//XXTH Cooldown --> only here what is mWrap ? 
		F32 percent = getPercent();
		if (percent < 1) 
		{
			// dglDrawBitmapStretch(cooldown_txrs[(int)(36.0f*percent)], rect);
         GFX->getDrawUtil()->drawBitmapStretch(cooldown_txrs[(int)(36.0f * percent)], rect);
		}
		//XXTH <-- Cooldown

      }
   }

   //If the button is being dragged, and we're supposed to display borders around the targets
   if (mIsButtonMoving && mDropBorder)
   {
      //Con::errorf("Trying to draw them borders...");     
      drawDropBorder(mDropBorderSearch, mDropBorderSearchColor);
   }

   //if we're supposed to be displaying a colored border because we're a target, draw it
   //another sibling class sets our border bool and our border color
   //The extra If/Else statement for button moving is because I dont want to draw a border around
   //the button that is actually moving.  You may remove this if you'd like.
   if (!mIsButtonMoving)
   {
      if (mDropBorderDisplayed)
      {
         RectI rect(offset.x, offset.y, getExtent().x, getExtent().y);
         GFX->getDrawUtil()->drawRect(rect, mDropBorderColor);
         //dglDrawRect(rect, mDropBorderColor);
      }
      else if (mProfile->mBorder || !mTextureHandle)
      {
         RectI rect(offset.x, offset.y, getExtent().x, getExtent().y);
         //dglDrawRect(rect, mProfile->mBorderColor);
         GFX->getDrawUtil()->drawRect(rect, mProfile->mBorderColor);
      }
      else {}
   }
   else
   {
      if (mProfile->mBorder || !mTextureHandle)
      {
         RectI rect(offset.x, offset.y, getExtent().x, getExtent().y);
         //dglDrawRect(rect, mProfile->mBorderColor);
         GFX->getDrawUtil()->drawRect(rect, mProfile->mBorderColor);
      }
   }
   renderChildControls(offset, updateRect);

}
//---------------------------------------------------------------------------------------------

void GuiDragDropButtonCtrl::setValue(S32 x, S32 y)
{
   if (mTextureHandle)
   {
      GFXTextureObject* texture = (GFXTextureObject *) mTextureHandle;
      x+=texture->getBitmapWidth()/2;
      y+=texture->getBitmapHeight() /2;
  	}
   while (x < 0)
      x += 256;
   startPoint.x = x % 256;

   while (y < 0)
      y += 256;
   startPoint.y = y % 256;
}


void GuiDragDropButtonCtrl::onMouseEnter(const GuiEvent &event)
{
   Parent::onMouseEnter(event);

   if ( mActive && mProfile->mSoundButtonOver )
   {
      SFX->playOnce(mProfile->mSoundButtonOver);
      
      //AUDIOHANDLE handle = alxCreateSource(mProfile->mSoundButtonOver);
      //alxPlay(handle);
   }

   //Dont change textures if we're dragging the icons around
   if(!mIsButtonMoving && mHoverTextureHandle)  
   {
      //Only change textures if the Hover texture exists
      mTextureHandle = mHoverTextureHandle;    
      setUpdate();    
   }    
   //Dont change textures here either
   else if(!mIsButtonMoving)                    
   {
      //There is no hover, so we make the hover the NormalTexture
      mHoverTextureHandle = mNormalTextureHandle;  
      mTextureHandle = mHoverTextureHandle;
      setUpdate();
   }
   else{}

}

void GuiDragDropButtonCtrl::onMouseLeave(const GuiEvent &event)
{
   Parent::onMouseLeave(event);

   //Make sure we switch to the NormalTexture when we've left & make sure we're not moving
   if(!mIsButtonMoving && mTextureHandle != mNormalTextureHandle)
   {
      mTextureHandle = mNormalTextureHandle;
      setUpdate();
   }

}

void GuiDragDropButtonCtrl::onMouseDown(const GuiEvent & event)
{
   Parent::onMouseDown(event);
   mIsDragged = false;

   mOrigBounds = getBounds();
   mMouseDownPosition = event.mousePoint;
   mMouseDownPositionOffset = event.mousePoint;
//   mMouseDownPositionOffset.x += 10;
//   mMouseDownPositionOffset.y += 15;
   mMouseDownPositionOffset.x -= getExtent().x / 2;
   mMouseDownPositionOffset.y -= getExtent().y / 2;

   if(mDownTextureHandle)
   {
      mTextureHandle = mDownTextureHandle;
      setUpdate();
   }
   else  // There's no down, lets see if there is a hover, if there's no hover we'll use the normal
   {
      if(mHoverTextureHandle)
         mDownTextureHandle = mHoverTextureHandle;
      else
         mDownTextureHandle = mNormalTextureHandle;

      mTextureHandle = mDownTextureHandle;
      setUpdate();
   }

   Point2I localPoint = globalToLocalCoord(event.mousePoint);

   //Let's check to see if we clicked inside the button
   if(localPoint.x < 0 || localPoint.x > getExtent().x || localPoint.y < 0 || localPoint.y > getExtent().y || !mCanMove)
      mIsButtonMoving = false;	

   else
   {
      mIsButtonMoving = true;

      //Ok, now we need to find out if we're good where we're at, or if we need to move the icon up the GUI
      //tree so we can actually move it around the screen.

      //We're interested in what comes two levels below GuiCanvas, because that's the highest level of the tree,
      //where we can still see the GUI elements

      GuiControl* thirdParent = NULL;
      GuiControl* previousParent = NULL;
      GuiControl* currentParent = this->getParent();

      GuiCanvas* root = this->getRoot();

      //Get the position in relation to it's immediate parent
      mChangedGroupPosition = this->getPosition();

      //Now we cycle through the parents, updating our position in relation to them.
      //We will also take this opportunity to find out what lives 2 steps below the GuiCanvas
      //The group that this parent lives in, is the top group which we can move things around visibly.

      while(currentParent && currentParent->getClassName() != root->getClassName())
      {        
         //we have a parent that isnt GuiCanvas, let's find it's position, and add it to our
         //ChangedGroupPosition.  This ChangedGroupPosition will be equal to the original Drag/Drop
         //GUI element's global position.
         mChangedGroupPosition += currentParent->getPosition();

         thirdParent = previousParent;
         previousParent = currentParent;
         //Grab the new parent, and loop
         currentParent = currentParent->getParent();
      }

      //if thirdParent changed, then we've moved up at least 2 layers, and we need to use this layer as our
      //parent.  This means that "currentParent == GuiCanvas", and "previousParent == GameTSCtrl"
      //the previousParent could really be anything, but it's not what we want

      //if the thirdParent didnt change(read, thirdPraent == NULL), then we only moved up one layer,
      //and we need to use the parent stored in previousParent.  This means that currentParent == GuiCanvas

      if(thirdParent)
      {
         //START MOVE GUI LAYERS
         //Store the current group that this button belongs to, so we can switch back when we're done
         mCurrentGroup = this->getGroup();
         //Find the group, One level below GameTSCtrl
         mTopLevelGroup = thirdParent->getGroup();
         //remove the DragDrop button from it's current group
         mCurrentGroup->removeObject(this);
         //and add it to the TopLevelGroup
         mTopLevelGroup->addObject(this);
         mChangedGroup = true;
         //END MOVE GUI LAYERS
         //START CANVAS UPDATE
         resize(mChangedGroupPosition, mOrigBounds.extent);
         //END CANVAS UPDATE
      }
      else
         mChangedGroup = false;

      mouseLock(); //Lock the mouse after we've done all our changes
   }

}

//--------------------------------------------------------------------------------------------------
void GuiDragDropButtonCtrl::onMouseUp(const GuiEvent & event)
{
  if(!mIsButtonMoving) return ; //prevent the crash ?!

   bool executedFunction = false;
   Parent::onMouseUp(event);
   mIsButtonMoving = false;
   mouseUnlock();

   //Shut off the rendering boxes around the targets if it draws them
   if(mDropBorder)
      undoDrawDropBorder(mDropBorderSearch);

   //Should always be within the bounds when mouse goes up, so we're still hovering, unless we snap
   //but we'll handle snap down below
   mTextureHandle = mHoverTextureHandle;

   //Let's check if the button can even move before we see if it can drop
   //This should simplify the button to be simply a ButtonCtrl element if that's all we want
   //No need to check for drop recievers if the thing cant move!
   if(mCanMove)
   {
      //checkDropPosition(whereverLetGoOfMouseButton)		
      GuiDragDropButtonCtrl* tmpDrop =  checkDropPosition(event.mousePoint);
   
      if(tmpDrop)
      {
	     bool allowDrop = true;
         Con::setVariable("$DroppedCtrl", this->getName());
         //Execute the console function for dropped items

         allowDrop = dStrcmp(Con::executef(this,  "onDropped"),"0") !=0;
		 if (allowDrop) {
		    PlaceTargetImages(tmpDrop);
			if(!mCopySource) 
			{
				//Delete the object if we've dropped it. And DONT want to copy the source button.
				//This will MOVE the contents into another button, and delete the current button.
	            this->deleteObject();
				return;
			}
			if(mResetSourceOnDrop && !mSwitchImages)
				resetContents();
		 } 

         //Give the console a way to find out what we actually dropped
         //I use this to save off the images for my command bar, since it has many layers which may
         //have different images in any given slot.  This could be done in the script, but this just
         //saves me the headache of putting $SomeVar = thisThingsName, every time I want to use it.

		 /* orig: 
         Con::setVariable("$DroppedCtrl", this->getName());
         //Execute the console function for dropped items
         Con::executef(this, 1, "onDropped");
		 */
         //We dont want to do both an onDropped() and an onAction() in the console
         executedFunction = true; 
      }
      else
      {
         const char* varCtrl = NULL;
         Con::setVariable("$DroppedCtrl", varCtrl);
         Con::setVariable("$DroppedUponCtrl", varCtrl);
      }

      //If it's supposed to snap back, snap it back
      if(mSnapsBack)
      {
         GuiControl *parent = getParent();
         GuiCanvas *root = getRoot();
         bool update = false;
         if (! root) return;
         if (parent)
            update = true;
         if (update)
         {
            Point2I pos = parent->localToGlobalCoord(getPosition());
            root->addUpdateRegion(pos, getExtent());
            resize(mOrigBounds.point, mOrigBounds.extent);
         }
         mTextureHandle = mNormalTextureHandle;
      }
      else
      {
         GuiControl *parent = getParent();
         GuiCanvas *root = getRoot();
         bool update = false;
         if (! root) return;
         if (parent)
            update = true;
         if (update)
         {
            Point2I pos = parent->localToGlobalCoord(getPosition());
            root->addUpdateRegion(pos, getExtent());
            resize(event.mousePoint, mOrigBounds.extent);
         }
         mTextureHandle = mNormalTextureHandle;
      }



      //Correct the changes we've made to the groups
      //NOTE:: If you dont want the thing to snap back, and it has changed groups in the tree
      //then it stays on the top level of the GuiTree, because you may run into problems with the
      //button not displaying if you dont snap back, and move it outside the extents of the Gui
      //it came from.  This is just a work around, as I personally dont plan to use multi-tree, non-snapping
      //buttons.  In the future, checks for extent versus group could be made, to see if you're even allowed to
      //not snap back.  As in, if the icon lands outside your local extent, ignore the GUIs demands to not snap back
      //and do it anyways.
      if(mChangedGroup && mSnapsBack)
      {

		 mTopLevelGroup->removeObject(this);
         mCurrentGroup->addObject(this);
      }

   } //Close bracket for mCanMove if check
   setUpdate();

   //if we didnt do an onDrop() console function, go ahead and call the onAction() function
   //Also, the GUI creator can specify if he wants the button to perform any actions on mouse up,
   //In creation of a skill/power listing, with these buttons, you may not want the spells to be
   //castable from the menu.  This solves that by taking away the onAction method.
   if(!mIsDragged && !executedFunction && mPerformActionOnUp)
      onAction();
   else
	  Con::executef(this,  "onDisplayUpdate");
}

void GuiDragDropButtonCtrl::onMouseDragged(const GuiEvent &event)
{

   //Dont do anything if we cant move
   if(!mCanMove)
      return;

   GuiControl *parent = getParent();
   GuiCanvas *root = getRoot();
   if (!root)
      return;


   Point2I deltaMousePosition = event.mousePoint - mMouseDownPosition;
   Point2I newPosition = getPosition(); //Defined in case we drag the mouse and dont move the button, oops
   Point2I newExtent = getExtent();

   //getPosition()
   //getExtent()

   bool update = false;
   //if we didnt change groups, use the regular position
   if (mIsButtonMoving && parent && !mChangedGroup)
   {
      newPosition.x = getMax(0, getMin(parent->getExtent().x - getExtent().x, mMouseDownPositionOffset.x + deltaMousePosition.x));
      newPosition.y = getMax(0, getMin(parent->getExtent().y - getExtent().y, mMouseDownPositionOffset.y + deltaMousePosition.y));
      update = true;
   }
   //if we did change groups, we need to use our newly found position
   else if(mIsButtonMoving && parent && mChangedGroup)
   {   
      newPosition.x = getMax(0, getMin(parent->getExtent().x - getExtent().x, mMouseDownPositionOffset.x + deltaMousePosition.x));
      newPosition.y = getMax(0, getMin(parent->getExtent().y - getExtent().y, mMouseDownPositionOffset.y + deltaMousePosition.y));
      update = true;
   }

   if (update)
   {
      Point2I pos = parent->localToGlobalCoord(getPosition());
      root->addUpdateRegion(pos, getExtent());
      resize(newPosition, newExtent);
	  //XXTH set dragged only if Dragged over some pixels, else it's hard to click on it
	  if (mAbs(deltaMousePosition.x)>3 ||  mAbs(deltaMousePosition.y)>3) {
			mIsDragged = true;
	  }
   }
}


void GuiDragDropButtonCtrl::onRightMouseDown(const GuiEvent &event)
{
   Parent::onRightMouseDown(event);
}


//---------------------------------------------------------------------------------------------------------------
void GuiDragDropButtonCtrl::PlaceTargetImages(GuiDragDropButtonCtrl* DDBCtrl) //XXTH replacement of code inside checkDropPosition
{
                  if(mSwitchImages)
                  {
                     //Grab our texture handles for what we're dropping onto incase there's something there
                     const GFXTexHandle tempHandle = DDBCtrl->getNormalHandle();  
                     const char* tempBitmap   = DDBCtrl->getNormalName();
                     const char* tempHover    = DDBCtrl->getHoverName();
                     const char* tempDown     = DDBCtrl->getDownName();

                     DDBCtrl->setTextureHandle(this->getNormalHandle());
                     //No need to set the other Handles, the following 3 lines do that				  
                     DDBCtrl->setBitmapName(this->getNormalName());
                     DDBCtrl->setHoverBitmapName(this->getHoverName());
                     DDBCtrl->setDownBitmapName(this->getDownName());

					 this->setTextureHandle(tempHandle);
                     this->setBitmapName(tempBitmap);
                     this->setHoverBitmapName(tempHover);
                     this->setDownBitmapName(tempDown);

					 //XXTH CoolDown 
					 DDBCtrl->toggleCoolDown(this);


                  }
                  else
                  {
                     //Put our new texture handles in that spot
                     DDBCtrl->setTextureHandle(this->getNormalHandle());
                     //No need to set the other Handles, the following 3 lines do that				  
                     DDBCtrl->setBitmapName(this->getNormalName());
                     DDBCtrl->setHoverBitmapName(this->getHoverName());
                     DDBCtrl->setDownBitmapName(this->getDownName());
					 //XXTH CoolDown 
					 DDBCtrl->setReverseTime(this->getReverseTime());
                  }

				  DDBCtrl->setUpdate();

}
//---------------------------------------------------------------------------------------------------------------
//Expects the Position of the MouseUpEvent from guiDragDropButtonCtrl
GuiDragDropButtonCtrl* GuiDragDropButtonCtrl::checkDropPosition(Point2I MouseUpPosition)
{
   //Check for Active Elements
   SimObject* DDObj = NULL;
   GuiControl* DDCtrl = NULL;
   GuiControl* Parent = NULL;
   Point2I ArrayPosition;  //Coordinates of the object in the array
   Point2I ArrayExtent;    //Extent of the object so we can check to see if the Mouse Up event is within this extent.
   Point2I TempExtent;     //We will subtract the ArrayPosition from the MouseUpPosition, and create this new extent	
   for(U32 i=0; i<mDragDropArraySize; i++)
   {
      //Check the Array for non zero values, which means we have a Drag/Drop ID number	
      if(GuiControl::DragDropArray[i] != 0 && GuiControl::DragDropArray[i] != this->getId())
      {
         //Get the SimObject pointer that belongs to the ID number
         DDObj = Sim::findObject(GuiControl::DragDropArray[i]);
         //Turn it into a GuiControl so we can work with position
         DDCtrl = dynamic_cast<GuiControl *>(DDObj);
         //Find out if this GuiControl is awake on the screen
         if(DDCtrl->isAwake())
         {
            //It's awake, let's find out where it is, in relation to the top level GameGui
            //This makes the assumption that your top level GUI is GameTSCtrl(it should be)
            //This will cycle through the parents, and add the parents relative position to
            //the children's relative positions.

            ArrayPosition = DDCtrl->getPosition();
            Parent = DDCtrl->getParent();  //Find the parent control
            if(Parent)					
            {
               while(Parent && Parent->getClassName() != "GameTSCtrl")
               {
                  ArrayPosition += Parent->getPosition();
                  Parent = Parent->getParent();
               }
            }

            //We will subtract the 2 positions, to form a new rectangle.  If this rectangle is fully contained
            //within the extent of the ArrayObject, then we've clicked inside the ArrayObject
            TempExtent = MouseUpPosition - ArrayPosition;
            //Get the ArrayExtent so we can compare the two rectangles
            ArrayExtent = DDCtrl->getExtent();
            //Check to see if it's within
            if(TempExtent.x >= 0 && TempExtent.x <= ArrayExtent.x && TempExtent.y >= 0 && TempExtent.y <= ArrayExtent.y)
            {
               //We've dropped our button within another DDGui Element, time to
               //assign new values to the receiver, if it is one.

               //Create a pointer to our DDBCtrl that was stored within the Array
               GuiDragDropButtonCtrl* DDBCtrl = dynamic_cast<GuiDragDropButtonCtrl *>(DDObj);

               //Start the assignment process of: 3 Texture Names, 4 Texture Handles, Command & Variable
               //My intentions are to leave a lot up to the GUI designer.  If they want the receiving end to have
               //completly different actions(such as transmit/receive capabilities), they should implement these
               //on creation of the GUI.  This is not the place to alter GUI design.
               if(DDBCtrl->mIsReceiver && this->getDropBorderSearch() == DDBCtrl->getDropBorderType())
               {
/*XXTH
                  if(mSwitchImages)
                  {
                     //Grab our texture handles for what we're dropping onto incase there's something there
                     const GFXTexHandle tempHandle = DDBCtrl->getNormalHandle();  
                     const char* tempBitmap   = DDBCtrl->getNormalName();
                     const char* tempHover    = DDBCtrl->getHoverName();
                     const char* tempDown     = DDBCtrl->getDownName();

                     DDBCtrl->setTextureHandle(this->getNormalHandle());
                     //No need to set the other Handles, the following 3 lines do that				  
                     DDBCtrl->setBitmapName(this->getNormalName());
                     DDBCtrl->setHoverBitmapName(this->getHoverName());
                     DDBCtrl->setDownBitmapName(this->getDownName());

                     this->setTextureHandle(tempHandle);
                     this->setBitmapName(tempBitmap);
                     this->setHoverBitmapName(tempHover);
                     this->setDownBitmapName(tempDown);
                  }
                  else
                  {
                     //Put our new texture handles in that spot
                     DDBCtrl->setTextureHandle(this->getNormalHandle());
                     //No need to set the other Handles, the following 3 lines do that				  
                     DDBCtrl->setBitmapName(this->getNormalName());
                     DDBCtrl->setHoverBitmapName(this->getHoverName());
                     DDBCtrl->setDownBitmapName(this->getDownName());
                  }
*/
                  //Set a variable in the script so the script will know what receieved the new textures.
                  //This is VERY necessary since we will be handling ALL functions via script.
                  //The idea is as follows:
                  //We drag and drop a button(the function doing the dropping will call an onDrop function),
                  //if this button falls upon something ($DroppedUponControl).  In the script we could trigger
                  //$DroppedUponControl.onDroppedUpon()

                  Con::setVariable("$DroppedUponCtrl", DDObj->getName());

                  //Old style function execution if you dont like how I made it all script	(2 lines)
                  //DDBCtrl->setConsoleCommand(this->getConsoleCommand());
                  //DDBCtrl->setConsoleVariable(this->getConsoleVariable());

                  
                  return DDBCtrl; // We've succeeded in placing the button
               }
               //If it's not a receiver then we just move on to the next one, since we may be dropping on something
               //that is right below it.  This gives a tad bit more flexibility to the person making the GUI
            }
         }
      }
   }
   return NULL;  // If we're here, we never placed our button
}

void GuiDragDropButtonCtrl::drawDropBorder(const char* searchString, ColorI searchColor)
{
   SimObject* DDObj = NULL;
   GuiControl* DDCtrl = NULL;
   GuiDragDropButtonCtrl* DDBCtrl = NULL;

   for(U32 i=0; i<mDragDropArraySize; i++)
   {
      if(GuiControl::DragDropArray[i] != 0)  //if we have an ID number
      {
         //Get the SimObject pointer that belongs to the ID number
         DDObj = Sim::findObject(GuiControl::DragDropArray[i]);
         //Turn it into a GuiControl so we can work with position
         DDCtrl = dynamic_cast<GuiControl *>(DDObj);
         //Find out if this GuiControl is awake on the screen
         if(DDCtrl->isAwake())
         {
            DDBCtrl = dynamic_cast<GuiDragDropButtonCtrl *>(DDObj);
            //Find out what type of DragDropButton it is, and see if it's what we're looking for
            //If it's what we want, then let's draw the border around it
            if(searchString == DDBCtrl->getDropBorderType())
            {
               DDBCtrl->setDropBorderDisplayed(true);
               DDBCtrl->setDropBorderColor(searchColor);
            }
         }
      }
   }
}

//Identical to the one above, but it simply sets the Displayed to false
void GuiDragDropButtonCtrl::undoDrawDropBorder(const char* searchString)
{
   SimObject* DDObj = NULL;
   GuiControl* DDCtrl = NULL;
   GuiDragDropButtonCtrl* DDBCtrl = NULL;

   for(U32 i=0; i<mDragDropArraySize; i++)
   {
      if(GuiControl::DragDropArray[i] != 0)  //if we have an ID number
      {
         //Get the SimObject pointer that belongs to the ID number
         DDObj = Sim::findObject(GuiControl::DragDropArray[i]);
         //Turn it into a GuiControl so we can work with position
         DDCtrl = dynamic_cast<GuiControl *>(DDObj);
         //Find out if this GuiControl is awake on the screen
         if(DDCtrl->isAwake())
         {
            DDBCtrl = dynamic_cast<GuiDragDropButtonCtrl *>(DDObj);
            //Find out what type of DragDropButton it is, and see if it's what we're looking for
            //If it's what we want, then let's draw the border around it
            if(searchString == DDBCtrl->getDropBorderType())
            {
               DDBCtrl->setDropBorderDisplayed(false);
            }
         }
      }
   }
}

void GuiDragDropButtonCtrl::setDropBorderDisplayed(bool display)
{
   mDropBorderDisplayed = display;
}

void GuiDragDropButtonCtrl::setDropBorderColor(ColorI color)
{
   mDropBorderColor = color;
}

//Reset the Contents to the original state.
//This means, we're saving whatever bools they have, but we're clearing the images, and the command/var
void GuiDragDropButtonCtrl::resetContents()
{
   setBitmapName("");
   setBitmapHandle(NULL);
   setHoverBitmapName("");
   setHoverBitmapHandle(NULL);
   setDownBitmapName("");
   setDownBitmapHandle(NULL);
   setTextureHandle(NULL);
   setConsoleCommand("");
   setConsoleVariable("");
   setUpdate();
}


//We're not going to handle any function calling in code, let's leave that up to the script
void GuiDragDropButtonCtrl::onAction()
{
   char buf[16];
   dSprintf(buf, sizeof(buf), "%d", getId());
   Con::setVariable("$ThisControl", buf);

   Con::setVariable("$ActionCtrlName", getName());
   Con::executef(this,  "onAction");	   
}

//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
// XXTH Cooldown
bool GuiDragDropButtonCtrl::onAdd()
{
  if (!Parent::onAdd())
    return false;

  if (sSpellCooldownBitmaps != NULL)
  {
    char buffer[256];
    for (int i = 0; i < NUM_COOLDOWN_FRAMES; i++)
    {
      dSprintf(buffer, 256, "%s_%.2d", sSpellCooldownBitmaps, i);
      cooldown_txrs[i] = GFXTexHandle(buffer, &GFXStaticTextureSRGBProfile, avar("%s() - mTextureHandle (line %d)", __FUNCTION__, __LINE__));

         //GFXTexHandle(buffer, BitmapTexture, true);
    }
  }

  return true;
}
//---------------------------------------------------------------------------------------------
void GuiDragDropButtonCtrl::toggleCoolDown(GuiDragDropButtonCtrl *from)
{
	S32 savCoolDown    = from->getCoolDown();
	S32 savTimeOffset  = from->getTimeOffset();
	bool savRunning    = from->getRunning();
	bool savReverse    = from->getReverseAnimation();

	from->setCoolDown(this->getCoolDown());
	from->setTimeOffset(this->getTimeOffset());
	from->setRunning(this->getRunning());
	from->setReverseAnimation(this->getReverseAnimation());

	this->setCoolDown(savCoolDown);
	this->setTimeOffset(savTimeOffset);
	this->setRunning(savRunning);
	this->setReverseAnimation(savReverse);


}
//---------------------------------------------------------------------------------------------
void GuiDragDropButtonCtrl::setReverseTime(S32 time)
{
   // Set the current time in seconds.
   mCoolDown = time;
   mTimeOffset = time + Platform::getVirtualMilliseconds();
   mRunning = true;
}
//---------------------------------------------------------------------------------------------
S32 GuiDragDropButtonCtrl::getReverseTime()
{
   if (!mRunning || (mTimeOffset - Platform::getVirtualMilliseconds() <=0))
		return 0;
   return mTimeOffset - Platform::getVirtualMilliseconds();
}
//---------------------------------------------------------------------------------------------
F32 GuiDragDropButtonCtrl::getPercent()
{  
   F32 snapshot = getReverseTime();
   if (snapshot <= 0.f) {
	   if (mRunning) {
		   mRunning = false;
		   Con::executef(this,  "onDone");
	   }
	   return 1;
   }
   if (!mReverseAnimation) snapshot =  F32(mCoolDown) - snapshot;

   F32 result = snapshot / F32(mCoolDown);
   return result;
}
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~~//
DefineEngineStringlyVariadicMethod(GuiDragDropButtonCtrl,setCoolDown, void, 3, 3,"Counts down from Time in ms)")
{
   object->setReverseTime(dAtof(argv[2]));
}

DefineEngineStringlyVariadicMethod(GuiDragDropButtonCtrl,getBalance, S32, 2, 2,"()Returns balance time in msecs.")
{
   return object->getReverseTime();
}
