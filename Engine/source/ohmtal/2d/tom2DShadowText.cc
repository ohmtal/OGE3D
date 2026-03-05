//-----------------------------------------------------------------------------
// Copyright (c) 2023 Ohmtal Game Studio
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

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "core/util/safeDelete.h"
#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"
#include "tom2DCtrl.h"
#include "tom2DRenderObject.h"
#include "tom2DShadowText.h"

IMPLEMENT_CONOBJECT(tom2DShadowText);

void tom2DShadowText::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("ShadowText");
   addField("text", TypeRealString, Offset(mText, tom2DShadowText), "");
   addField("fontFace", TypeRealString, Offset(mFontFace, tom2DShadowText), "Default Arial");
   addField("fontSize", TypeS32, Offset(mFontSize, tom2DShadowText), "Default 12");
   addField("align", TypeS32, Offset(mAlign, tom2DShadowText), "0=left,1=middle,2=right,3=center");
   addField("shadowOffset", TypePoint2I, Offset(mShadowOffset, tom2DShadowText), "");
   addField("textColor", TypeColorI, Offset(mColor, tom2DShadowText), "");
   addField("shadowColor", TypeColorI, Offset(mShadowColor, tom2DShadowText), "");
   endGroup("ShadowText");
}


void tom2DShadowText::onRender(U32 dt, Point3F lOffset)
{
   if (!mScreen || !mFont)
      return;
   Point2I offset = Point2I(getScreen()->getRenderOffset().x + mX, getScreen()->getRenderOffset().x + mY);

   //nothing to do on left
   switch (mAlign) {
   case 1:	//middle
      offset.x -= U32(mFont->getStrWidth(mText.c_str()) / 2);
      break;
   case 2:	//right
      offset.x -= mFont->getStrWidth(mText.c_str());
      break;

   case 3:	//center
      offset.x -= U32(mFont->getStrWidth(mText.c_str()) / 2);
      offset.y -= U32(mFont->getHeight() / 2);
      break;

   }

//   Con::errorf("************** funzt net ****************");

   GFX->getDrawUtil()->setBitmapModulation(mShadowColor);
   GFX->getDrawUtil()->drawText(mFont, offset + mShadowOffset, mText.c_str()); // , getScreen()->getProfile()->mFontColors);

   GFX->getDrawUtil()->setBitmapModulation(mColor);
   GFX->getDrawUtil()->drawText(mFont, offset, mText.c_str()); // , getScreen()->getProfile()->mFontColors);


}

bool tom2DShadowText::onAdd()
{
   if (!Parent::onAdd())
      return false;

   setFont();

   Con::executef(this, "onAdd", getId());
   return true;
}

void tom2DShadowText::onRemove()
{
   Con::executef(this, "onRemove", getId());
   Parent::onRemove();
}

bool tom2DShadowText::setFont()
{
   mFont = GFont::create(mFontFace.c_str(), mFontSize, GuiControlProfile::sFontCacheDirectory);
   if (mFont == NULL) {
      Con::errorf("Failed to load/create profile font (%s/%d)", mFontFace.c_str(), mFontSize);
      return false;
   }
   return true;
}

bool tom2DShadowText::setFont(String lFontFace, S32 lFontSize)
{
   mFontFace = lFontFace;
   mFontSize = lFontSize;
   return setFont();
}

DefineEngineMethod(tom2DShadowText, setFont, bool, (String fontFace, S32 fontSize), (16),
   "Update the Font after it is added. ")
{
   return object-> setFont(fontFace, fontSize );
}
