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
// tom2DScriptRenderObject
//=================================================================================================
#ifndef _TOM2DSCRIPTRENDEROBJ_H_
#define _TOM2DSCRIPTRENDEROBJ_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif

#ifndef _TOM2DUTILS_H_
#include "tom2DUtils.h"
#endif

#ifndef _TOM2DRENDEROBJ_H_
#include "ohmtal/2d/tom2DRenderObject.h"
#endif


class tom2DScriptRenderObject : public tom2DRenderObject
{
private:
   typedef tom2DRenderObject Parent;
public:
   DECLARE_CONOBJECT(tom2DScriptRenderObject);

   virtual void onRender(U32 dt, Point3F lOffset);
   virtual void onUpdate(F32 fDt);
   bool onAdd();
   void onRemove();

};

#endif
