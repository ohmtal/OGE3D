//-----------------------------------------------------------------------------
// Copyright (c) 2009 huehn-software / Ohmtal Game Studio
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
//=================================================================================================
// tom2DTexture
//=================================================================================================

#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "gfx/gfxDevice.h"
#include "gui/core/guiTypes.h"
#include "tom2DTexture.h"



IMPLEMENT_CONOBJECT(tom2DTexture);
//------------------------------------------------------------------------------
tom2DTexture::tom2DTexture(void)
{
   mBitmapName = StringTable->insert("");
   mTextureHandle = NULL;

   mRows = 1;
   mCols = 1;

   mColorModulation = ColorI(255, 255, 255, 255);

}
//------------------------------------------------------------------------------
tom2DTexture::~tom2DTexture()
{
   /*
      if (mTextureHandle)
         mTextureHandle.~TextureHandle();
   */
   //should free the texture! by TextureHandle::unlock()
   mTextureHandle = NULL;

}

void tom2DTexture::initPersistFields()
{
   Parent::initPersistFields();
   addGroup("Modulation");
   addField("color", TypeColorI, Offset(mColorModulation, tom2DTexture));
   endGroup("Modulation");

}

//------------------------------------------------------------------------------
void tom2DTexture::setParts(U32 cols, U32 rows) {
   if (cols < 1 || rows < 1) return;
   mCols = cols;
   mRows = rows;

}
//------------------------------------------------------------------------------
bool tom2DTexture::setBitmapName(void* obj, const char* data)
{
   static_cast<tom2DTexture*>(obj)->setBitmap(data);
   return false;
}

//------------------------------------------------------------------------------
void tom2DTexture::setBitmap(const char* name, bool clampTexture, bool setupTransparent)
{
   mBitmapName = StringTable->insert(name);
   if (*mBitmapName) {
      /* FIXME 	   if (setupTransparent)
      {


         GBitmap *bmp = this->loadBmpTrans(mBitmapName);
          mTextureHandle = TextureHandle(mBitmapName, bmp , clampTexture);
      }
      else
         mTextureHandle = TextureHandle(mBitmapName, BitmapTexture , clampTexture);
      */
      //works but bad profile mTextureHandle = GFXTexHandle(mBitmapName, &GFXDefaultGUIProfile, avar("%s() - mTextureHandle (line %d)", __FUNCTION__, __LINE__));
      mTextureHandle = GFXTexHandle(mBitmapName, &GFXStaticTextureSRGBProfile, avar("%s() - mTextureHandle (line %d)", __FUNCTION__, __LINE__));


   }
   else
      mTextureHandle = NULL;
}
//------------------------------------------------------------------------------

void tom2DTexture::addToBitmapsheet(const char* name)
{

   mBitmapName = StringTable->insert(name);
   GBitmap* bmp = this->loadBmpTrans(mBitmapName);
   if (bmp)
      mBitmapSheets.push_back(bmp);
   else
      Con::errorf("addToBitmapsheet :: Failed to load bitmap %s", name);

}
//------------------------------------------------------------------------------
void tom2DTexture::generateBitmapsheet()
{
   S32 count = mBitmapSheets.size();
   if (count < 2) //kleiner 2 macht es keinen sinn ;)
      return;
   bool initBig = true;
   GBitmap* bigBmp;
   U32 w, h, i;
   Vector<GBitmap*>::iterator pBmp = mBitmapSheets.begin();
   i = 0;
   for (pBmp = mBitmapSheets.begin(); pBmp != mBitmapSheets.end(); pBmp++)
   {
      if (initBig)
      {
         w = (*pBmp)->getWidth();
         h = (*pBmp)->getHeight();
         bigBmp = new GBitmap(w * count, h, false, GFXFormatR8G8B8A8);
         initBig = false;
      }

      // void copyRect(const GBitmap *src, const RectI &srcRect, const Point2I &dstPoint);
      bigBmp->copyRect((*pBmp), RectI(0, 0, w, h), Point2I(i * w, 0));

      i++;
   }

   for (i = 0; i < count; i++)
   {
      if (mBitmapSheets[i])
         delete mBitmapSheets[i];
   }
   mBitmapSheets.clear();


   if (!bigBmp)
      return;


   //FIXME mTextureHandle = TextureHandle(NULL, bigBmp , false);
   mTextureHandle = GFXTexHandle(mBitmapName, &GFXDefaultGUIProfile, avar("%s() - mTextureHandle (line %d)", __FUNCTION__, __LINE__));

   this->setParts(count, 1);
}
//------------------------------------------------------------------------------
/*
 * GBitmap * tom2DTexture::loadBmpTrans(const char *name)
 * use the top left pixel color and set it transparent ... hopefully
 */
GBitmap* tom2DTexture::loadBmpTrans(const char* name)
{

   GBitmap* bmp = GBitmap::load(name);
   if (!bmp) {
      Con::errorf("Cound not load image: %s", name);
      return NULL;
   }
   if (bmp->getFormat() != GFXFormatR8G8B8A8)
      return bmp;

   S32 sW = bmp->getWidth();
   S32 sH = bmp->getHeight();

   S32 w = 0;
   S32 h = 0;
   F32 p = 1;
   do {
      w = (S32)mPow(2, p);
      p++;

   } while (w < sW);
   p = 1;
   do {
      h = (S32)mPow(2, p);
      p++;

   } while (h < sH);


   U8* rgbBits = bmp->getWritableBits();
   U8 transR = rgbBits[0];
   U8 transG = rgbBits[1];
   U8 transB = rgbBits[2];

   if (w != sW || h != sH)
   {
      GBitmap* bmp3 = new GBitmap(w, h, false, GFXFormatR8G8B8A8);

      U8* tmpBits = bmp3->getWritableBits();
      for (S32 wi = 0; wi < w; wi++)
      {
         for (S32 hi = 0; hi < h; hi++)
         {
            tmpBits[wi * 3 + hi * 3 * w + 0] = transR;
            tmpBits[wi * 3 + hi * 3 * w + 1] = transG;
            tmpBits[wi * 3 + hi * 3 * w + 2] = transB;
         }
      }
      bmp3->copyRect(bmp, RectI(0, 0, sW, sH), Point2I(0, 0));

      delete bmp;
      bmp = bmp3;
      rgbBits = tmpBits;		//		rgbBits = bmp->getWritableBits();
   }


   GBitmap* bmp2 = new GBitmap(w, h, false, GFXFormatR8G8B8A8);
   U8* bmpBits = bmp2->getWritableBits();


   for (S32 wi = 0; wi < w; wi++)
   {
      for (S32 hi = 0; hi < h; hi++)
      {
         bmpBits[wi * 4 + hi * 4 * w + 0] = rgbBits[wi * 3 + hi * 3 * w + 0];
         bmpBits[wi * 4 + hi * 4 * w + 1] = rgbBits[wi * 3 + hi * 3 * w + 1];
         bmpBits[wi * 4 + hi * 4 * w + 2] = rgbBits[wi * 3 + hi * 3 * w + 2];
         if (
            rgbBits[wi * 3 + hi * 3 * w + 0] == transR
            && rgbBits[wi * 3 + hi * 3 * w + 1] == transG
            && rgbBits[wi * 3 + hi * 3 * w + 2] == transB
            )
         {
            bmpBits[wi * 4 + hi * 4 * w + 3] = 0;
         }
         else {
            bmpBits[wi * 4 + hi * 4 * w + 3] = 255;
         }
      }
   }
   delete bmp;
   bmp = bmp2;

   return bmp;

}


//------------------------------------------------------------------------------
DefineEngineMethod(tom2DTexture, setBitmap, bool, (const char* filename, bool clamptexture, bool DUMMYsetuptransparent),(false,false) ,
      "(string filename, bool clamptexture)"
   "Set the bitmap displayed in the control. Note that it is limited in size, to 256x256.")
{
   object->setBitmap(filename, clamptexture, false); 

   if (!object->getTextureHandle()) {
      return false;
   }
      
   return true;
}
//------------------------------------------------------------------------------

DefineEngineMethod(tom2DTexture, addToBitmapsheet, void, (const char* filename), , "(string filename)"
   "Add a bitmap to bitmap sheet ... dont forget to call generateBitmapsheet")
{
   object->addToBitmapsheet(filename);
}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DTexture, generateBitmapsheet, void, (), , "()"
   "Build a big (long) bitmap out of bitmap sheet ... and clear bitmap sheet")
{
   object->generateBitmapsheet();
}

//------------------------------------------------------------------------------
DefineEngineMethod(tom2DTexture, setParts, void, (S32 cols, S32 rows), , "(Uint cols, Uint rows)"
   "Break a bitmap in parts to use multitexture.")
{
   object->setParts(cols,rows);
}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DTexture, getWidth, S32, (), , "()"
   "get the width of the texture, if broke in parts it get the width of the part")
{
   return object->getWidth();
}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DTexture, getheight, S32, (), , "()"
   "get the height of the texture, if broke in parts it get the height of the part")
{
   return object->getHeight();
}
//------------------------------------------------------------------------------
DefineEngineMethod(tom2DTexture, getBitmapName, const char * , (), , "()"
   "get the name of the current bitmap")
{
   return object->getBitmapName();
}
