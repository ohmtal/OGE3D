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
//------------------------------------------------------------------------------
//tom2DRenderObject
//------------------------------------------------------------------------------

#include "console/engineAPI.h"
#include "tom2DCtrl.h"
#include "tom2DRenderObject.h"

void tom2DRenderObject::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("tom2DRenderObject");
   addField("x", TypeF32, Offset(mX, tom2DRenderObject), "float X Position");
   addField("y", TypeF32, Offset(mY, tom2DRenderObject), "float Y Position");
   addField("layer", TypeS32, Offset(mLayer, tom2DRenderObject), "float Layer (0..9999)");


//example   addProtectedField("name", TypeName, Offset(mObjectName, SimObject), &setProtectedName, &defaultProtectedGetFn,  "Optional global name of this object.");
   // mhhh addProtectedField("Layer", TypeS32, Offset(mLayer, tom2DRenderObject),&staticSetLayer,&staticGetLayer,"float Layer (0..9999)");
   addField("visible", TypeBool, Offset(mVisible, tom2DRenderObject), "Rendered or not");

   endGroup("tom2DRenderObject");
}


void tom2DRenderObject::onRemove()
{
   //2023-04-10 
   if (mScreen)
      mScreen->removeRenderObject(this);

   Parent::onRemove();
}

void tom2DRenderObject::setLayer(U32 lLayer)
{
   if (mLayer == lLayer) // no need to sort if its equal ! 
      return;
   mLayer = lLayer;
   if (mLayer > HS2D_MAXLAYERS)
      mLayer = HS2D_MAXLAYERS - 1;

   if (mScreen)
      mScreen->sortRenderObjects();
}


