//-----------------------------------------------------------------------------
// Copyright (c) 2009/2023 huehn-software / Ohmtal Game Studio
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
//
// guiEnhancedButton 
//
// Button with auto coloring highlight, inactive - no depressed
// Also an effect for PersistentActivated  - Example: used in ToM to tell player
// that an building is active for keyboard placement.
//
//
// 2023-12-13 ported to OGE3D
//
//-----------------------------------------------------------------------------
#include "platform/platform.h"
#include "gui/buttons/guiBitmapButtonCtrl.h"
#include "core/util/path.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gui/core/guiCanvas.h"
#include "gui/core/guiDefaultControlRender.h"
#include "gfx/gfxDrawUtil.h"
#include "gfx/gfxTextureManager.h"
#include "guiEnhancedButton.h"


//-------------------------------------
ImplementEnumType(verticalAlign, "vertical Alignments")
{ GuiEnhancedButtonCtrl::TopJustify,    "top", "..." },
{ GuiEnhancedButtonCtrl::MiddleJustify, "middle", "..."},
{ GuiEnhancedButtonCtrl::BottomJustify, "bottom", "..."},
EndImplementEnumType;

//-------------------------------------

IMPLEMENT_CONOBJECT(GuiEnhancedButtonCtrl);
//-------------------------------------
GuiEnhancedButtonCtrl::GuiEnhancedButtonCtrl()
{
   mBitmapName = StringTable->insert("");
   // mBounds.extent.set(140, 30);
   setExtent(140, 30);
   mStateOnColor = ColorI(255,255,0,255);
   mVerticalAlignment  = MiddleJustify;

   mShowAcceleratorKey = true;
}


//-------------------------------------
void GuiEnhancedButtonCtrl::initPersistFields()
{
   Parent::initPersistFields();
   
   addField("bitmap", TypeStringFilename, Offset(mBitmapName, GuiEnhancedButtonCtrl));
   addField("stateoncolor", TypeColorI, Offset(mStateOnColor , GuiEnhancedButtonCtrl));
   addField("verticalJustify", TYPEID< verticalAlign >(), Offset(mVerticalAlignment, GuiEnhancedButtonCtrl));
   addField("ShowAcceleratorKey", TypeBool, Offset(mShowAcceleratorKey, GuiEnhancedButtonCtrl));
   
}
//-------------------------------------
void GuiEnhancedButtonCtrl::setStateOn( bool bStateOn )
{
/*	
   if(!mActive && bStateOn ) //do not set it ON when not active, but OFF is allowed
      return;
*/    
   mStateOn = bStateOn;
   //setUpdate();
}

//-------------------------------------
bool GuiEnhancedButtonCtrl::onWake()
{
   if (! Parent::onWake())
      return false;
   setActive(true);
   setBitmap(mBitmapName);
   return true;
}


//------------------------------------------------------------------------------
void GuiEnhancedButtonCtrl::sendMouseEvent(const char * name, const GuiEvent & event)
{
   char buf[3][32];
   dSprintf(buf[0], 32, "%d", event.modifier);
   dSprintf(buf[1], 32, "%d %d", event.mousePoint.x, event.mousePoint.y);
   dSprintf(buf[2], 32, "%d", event.mouseClickCount);
   //Con::executef(this, 4, name, buf[0], buf[1], buf[2]);
   Con::executef(this, name, buf[0], buf[1], buf[2]);
}

//-------------------------------------
void GuiEnhancedButtonCtrl::onMouseDragged(const GuiEvent & event)
{
   sendMouseEvent("onMouseDragged", event);
}

//-------------------------------------
void GuiEnhancedButtonCtrl::onSleep()
{
   mTexture = NULL;
   Parent::onSleep();
}


//-------------------------------------

// Legacy method.  Can just assign to bitmap field.
DefineEngineMethod(GuiEnhancedButtonCtrl, setBitmap, void, (const char* path), ,
   "Set the bitmap to show on the button.\n"
   "@param path Path to the texture file in any of the supported formats.\n")
{
   object->setBitmap(path);
}

//-------------------------------------
void GuiEnhancedButtonCtrl::inspectPostApply()
{
   // if the extent is set to (0,0) in the gui editor and appy hit, this control will
   // set it's extent to be exactly the size of the normal bitmap (if present)
   Parent::inspectPostApply();
   updateExtent();
}

void GuiEnhancedButtonCtrl::updateExtent()
{
   if ((getWidth() == 0) && (getWidth() == 0) && mTexture)
   {
      setExtent(mTexture->getWidth(), mTexture->getHeight());
   }
}


//-------------------------------------
void GuiEnhancedButtonCtrl::setBitmap(const char *name)
{
   mBitmapName = name;
   if(!isAwake())
      return;

   if (!mBitmapName.isEmpty())
   {
      mTexture = GFXTexHandle(mBitmapName, &GFXStaticTextureSRGBProfile, avar("%s() - mTextureHandle (line %d)", __FUNCTION__, __LINE__));
      if (mTexture.isNull()) {
         Con::warnf("GuiEnhancedButtonCtrl::setBitmap - Unable to load texture: %s", mBitmapName.c_str());
         this->setBitmap(GFXTextureManager::getUnavailableTexturePath());
         return;
      }
      updateExtent();
   }
   else
   {
      mTexture = NULL;
   }
   setUpdate();
}


//-------------------------------------
void GuiEnhancedButtonCtrl::onRender(Point2I offset, const RectI& updateRect)
{
	ButtonState state = NORMAL;
   if (mActive)
   {
      if (mMouseOver) state = HILIGHT;
      if (mDepressed ) state = DEPRESSED;
	  if (mStateOn) state = ON;
   }
   else
      state = INACTIVE;
// XXTH FIXME
   renderButton(mTexture, state, offset, updateRect); 

}

//------------------------------------------------------------------------------

void GuiEnhancedButtonCtrl::renderButton(GFXTexHandle &texture, ButtonState state, Point2I &offset, const RectI& updateRect)
{

   if (texture)
   {

	   //Border!
	   RectI boundsRect(offset, getExtent());
	   if( mProfile->mBorder != 0 && !mHasTheme )
	   {
		  if (mDepressed || mStateOn)
			 renderFilledBorder( boundsRect, mProfile->mBorderColorHL, mProfile->mFillColorHL );
		  else if (!mActive)  //XXTH added!
			 renderFilledBorder( boundsRect, mProfile->mBorderColorNA, mProfile->mFillColorNA );
		  else
			 renderFilledBorder( boundsRect, mProfile->mBorderColor, mProfile->mFillColor );
	   }
	   else if( mHasTheme )
	   {
		  S32 indexMultiplier = 1;
		  if ( mMouseOver ) 
			 indexMultiplier = 3;
		  else if ( mDepressed || mStateOn )
			 indexMultiplier = 2;
		  else if ( !mActive )
			 indexMultiplier = 4;

		  renderSizableBitmapBordersFilled( boundsRect, indexMultiplier, mProfile );
	   }

	   
	   
      RectI rect(offset + Point2I(2,2) , getExtent() - Point2I(4, 4)); //a bit smaller
      ColorI fontColor = mProfile->mFontColor;
	  Point2I textPos = offset;

     GFX->getDrawUtil()->clearBitmapModulation();
	  switch (state)
		{
			case NORMAL:      
                  GFX->getDrawUtil()->setBitmapModulation(ColorI(230, 230, 230, 255));
						break;
			case HILIGHT:     
				        //is white
				        fontColor = mProfile->mFontColorHL;
						break;
			case ON: 
				{
				        //is white
				        RectI lBorderBounds(offset , getExtent()); 
						
                  GFX->getDrawUtil()->drawRect(lBorderBounds, mStateOnColor );
						lBorderBounds.point +=Point2I(1,1);
						lBorderBounds.extent -=Point2I(2,2);
                  GFX->getDrawUtil()->drawRect(lBorderBounds, mStateOnColor);

						fontColor = mProfile->mFontColorHL;
						break;
				}
			case DEPRESSED: 
						rect.point +=  Point2I(1,1);
						fontColor = mProfile->mFontColorSEL;
						textPos += Point2I(1,1);
						break;
			case INACTIVE:
            GFX->getDrawUtil()->setBitmapModulation(ColorI(50, 50, 50, 255));
				//dglSetBitmapModulation(ColorF(0.2f,0.2f,0.2f,1.f));
				fontColor = mProfile->mFontColorNA;
				break;
		}

     GFX->getDrawUtil()->drawBitmapStretch(texture, rect);
      // dglDrawBitmapStretch(texture, rect, GFlip_None);
      GFX->getDrawUtil()->clearBitmapModulation();
	  if (dStrlen(mButtonText) > 0)
	  {
          // Make sure we take the profile's textOffset into account.
		  if (mVerticalAlignment != MiddleJustify)
		  {
            //S32 textHeight = mProfile->mFont->getHeight() / 2; 
			S32 lTextYOffset = (getExtent().y - mProfile->mFont->getHeight()) / 2;
			switch (mVerticalAlignment) 
			{
				case TopJustify:
							 textPos.y -= lTextYOffset;
						break;
				case BottomJustify:
							 textPos.y += lTextYOffset;
						break;

			}
			
		  }
        GFX->getDrawUtil()->setBitmapModulation(fontColor);
        renderJustifiedText(textPos, getExtent(), mButtonText);

	  }

//XXTH ACCEL RENDER TEST 20141209
	  if (mShowAcceleratorKey && getAcceleratorKey() != StringTable->insert(""))
	  {
		  S32 lTextYOffset = (getExtent().y - mProfile->mFont->getHeight()) / 2;
		  renderJustifiedText(offset + Point2I(0,-lTextYOffset), getExtent(), getAcceleratorKey());
	  }
      renderChildControls( offset, updateRect);
   }
   else
      Parent::onRender(offset, updateRect);
}

//------------------------------------------------------------------------------


DefineEngineMethod( GuiEnhancedButtonCtrl, GuiEnhancedButtonCtrl, void, (bool isStateOn), , "(bool isStateOn) ignores groups and set state on" )
{
   object->setStateOn(isStateOn);
}
