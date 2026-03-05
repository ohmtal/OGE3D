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
// Also an effect for state mStateOn
// Added VerticalAlignment
//
//-----------------------------------------------------------------------------
#ifndef _GUI_ENHANCED_BUTTON_CTRL_H
#define _GUI_ENHANCED_BUTTON_CTRL_H

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif
#ifndef _GUIMOUSEEVENTCTRL_H_
#include "gui/utility/guiMouseEventCtrl.h"
#endif
#ifndef _GUIBUTTONCTRL_H_
#include "gui/buttons/guiButtonCtrl.h"
#endif
#ifndef _GFXTEXTUREMANAGER_H_
#include "gfx/gfxTextureManager.h"
#endif

//-----------------------------------------------------------------------------
class GuiEnhancedButtonCtrl : public GuiButtonCtrl
{
public:
   enum ButtonState{
      NORMAL,
      HILIGHT,
      DEPRESSED,
      INACTIVE,
	  ON
   }; 
   enum VerticalAlignmentType
   {
      TopJustify,
      BottomJustify,
      MiddleJustify
   };

   VerticalAlignmentType mVerticalAlignment;                       ///< Horizontal text alignment


private:

   typedef GuiButtonCtrl Parent;
   ColorI mStateOnColor;

   void sendMouseEvent(const char * name, const GuiEvent &);

protected:
   // StringTableEntry mBitmapName;
   String mBitmapName;
   GFXTexHandle mTexture;
   bool mShowAcceleratorKey;

   
   void renderButton(GFXTexHandle &texture, ButtonState state,Point2I &offset, const RectI& updateRect);

public:
   DECLARE_CONOBJECT(GuiEnhancedButtonCtrl);


   GuiEnhancedButtonCtrl();

   static void initPersistFields();

   //Parent methods
   bool onWake();
   void onSleep();
   void inspectPostApply();
   void updateExtent();

   void onMouseDragged(const GuiEvent & event);


   void setStateOn( bool bStateOn );
   void setBitmap(const char *name);

   void onRender(Point2I offset, const RectI &updateRect);

};

//-----------------------------------------------------------------------------
typedef GuiEnhancedButtonCtrl::VerticalAlignmentType verticalAlign;
DefineEnumType(verticalAlign);

#endif //_GUI_ENHANCED_BUTTON_CTRL_H
