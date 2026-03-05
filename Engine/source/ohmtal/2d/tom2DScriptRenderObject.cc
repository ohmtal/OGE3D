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
// tom2DScriptRenderObject
// Handle with care this is sloooooooooooooooooow but fast for prototyping!! 
//------------------------------------------------------------------------------

#include "tom2DCtrl.h"
#include "tom2DRenderObject.h"

#include "tom2DScriptRenderObject.h"


IMPLEMENT_CONOBJECT(tom2DScriptRenderObject);


void tom2DScriptRenderObject::onRender(U32 dt, Point3F lOffset)
{
   if (getScreen())
   {
      //Con::executef( this, 3, "onRender", getScreen()->getIdString(), Con::getIntArg(dt));
      Con::executef(this, "onRender", dt);
   }
}

void tom2DScriptRenderObject::onUpdate(F32 fDt)
{
   if (getScreen())
   {
      //Con::executef( this, 2, "onUpdate", Con::getFloatArg(fDt));
      Con::executef(this, "onUpdate", Con::getFloatArg(fDt));
   }
}

bool tom2DScriptRenderObject::onAdd()
{
   if (!Parent::onAdd())
      return false;


   //linkNamespaces();
   // Call onAdd in script!
   //Con::executef(this, 2, "onAdd", Con::getIntArg(getId()));
   Con::executef(this, "onAdd", getId());
   return true;
}

void tom2DScriptRenderObject::onRemove()
{
   // Call onRemove in script!
   Con::executef(this, "onRemove", getId());

   //unlinkNamespaces();

   if (getScreen())
      getScreen()->removeRenderObject(this);

   Parent::onRemove();
}
