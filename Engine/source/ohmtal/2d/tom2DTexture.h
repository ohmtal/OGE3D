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
#ifndef _TOM2DTEXTURE_H_
#define _TOM2DTEXTURE_H_

#ifndef _GFXTEXTUREMANAGER_H_
#include "gfx/gfxTextureManager.h"
#endif

#ifndef _GFXTEXTUREHANDLE_H_
#include "gfx/gfxTextureHandle.h"
#endif
//=================================================================================================
// tom2DTexture
//=================================================================================================
class tom2DTexture : public SimObject
{
private:
   typedef SimObject Parent;

protected:
   Vector<GBitmap*>mBitmapSheets;

   static bool setBitmapName(void* obj, const char* data);
   //?? static const char* getBitmapName(void* obj, const char* data);
   GBitmap* loadBmpTrans(const char* name);

   StringTableEntry mBitmapName;
   //TextureHandle mTextureHandle;
   GFXTexHandle mTextureHandle;

   U32 mRows;
   U32 mCols;

   ColorI mColorModulation;


public:
   //creation methods
   DECLARE_CONOBJECT(tom2DTexture);
   tom2DTexture();
   ~tom2DTexture();

   static void initPersistFields();


   void setParts(U32 cols, U32 rows);
   U32 getCols() { return (mCols); }
   U32 getRows() { return (mRows); }

   void setBitmap(const char* name, bool clampTexture = false, bool setupTransparent = false);
   S32 getWidth() const { return(mTextureHandle.getWidth() / mCols); }
   S32 getHeight() const { return(mTextureHandle.getHeight() / mRows); }

   bool setColor(ColorI lColorModulation) { mColorModulation = lColorModulation; }
   ColorI getColor() { return mColorModulation; }


   void addToBitmapsheet(const char* name);
   void generateBitmapsheet();


   //TextureHandle getTextureHandle() {return mTextureHandle; }
   GFXTexHandle getTextureHandle() { return mTextureHandle; }

   StringTableEntry getBitmapName() { return  mBitmapName; }
};


#endif
