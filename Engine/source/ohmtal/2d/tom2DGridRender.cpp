#include "tom2DGridRender.h"
/**
  tom2DGridRender.cpp

  @since 2024-01-04
  @author XXTH

  License at: ohmtal/misc/ohmtalMIT.h

  Example:
    SceneObj.GridRender = new tom2DGridRender();
    SceneObj.GridRender.setGrid(SceneObj.Grid);
    GameScreen.addRenderObject(SceneObj.GridRender);

  TODO:
   variable colors for weights

*/
#include "console/console.h"
#include "console/consoleTypes.h"

#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"

#include "tom2DCtrl.h"
#include "tom2DRenderObject.h"
#include "ohmtal/grid/basicgrid.h"

IMPLEMENT_CONOBJECT(tom2DGridRender);

//-----------------------------------------------------------------------------
tom2DGridRender::tom2DGridRender()
{
   mGrid = NULL;
   mTriangleCount = 0;

   for (S32 i = 0; i < 256; i++)
      mColorTable[i] = ColorI(128, 128, 128, 255);
}
//-----------------------------------------------------------------------------
tom2DGridRender::~tom2DGridRender()
{
   if (mScreen) {
      // mScreen->removeSprite(this);
      mScreen->removeRenderObject(this);
      mScreen = NULL;
   }
}
//-----------------------------------------------------------------------------
void tom2DGridRender::setGrid(BasicGrid* lGrid)
{
   mGrid = lGrid;
   updateVerts();
}
//-----------------------------------------------------------------------------

void tom2DGridRender::updateVerts()
{
   S32 nodeCount = mGrid->getNodeCount();
   

   if (!mGrid || nodeCount < 4)
   {
      return;
   }

   S32 vertCount = nodeCount * 6;
   mTriangleCount = nodeCount * 2;
   mVertBuff.set(GFX, vertCount, GFXBufferTypeDynamic);
   GFXVertexPC* vert = mVertBuff.lock();
   //GFXVertexPCT* vert = mVertBuff.lock();

   ColorI lColor = ColorI(255, 255, 255, 255);
   S32 lTrans = 200;

   S32 lSize     = mGrid->getSquareSize();

   
   


   for (S32 i = 0; i < nodeCount; i++)
   {

      BasicGridNode* lNode = mGrid->getNodeById(i);

      lColor = getWeightColor(lNode->getWeight());

      F32 left   = F32(lNode->getPos().x);
      F32 top    = F32(lNode->getPos().y);
      F32 right  = F32(lNode->getPos().x + lSize);
      F32 bottom = F32(lNode->getPos().y + lSize);

      //               v0---------v1
      //               | 
      //               | 
      //               | 
      //               v2

      //v0 
      vert->point.set(left, top, 0.f);
      vert->color = lColor;
      vert++;

      //v1
      vert->point.set(right, top, 0.f);
      vert->color = lColor;
      vert++;

      //v2
      vert->point.set(left, bottom, 0.f);
      vert->color = lColor;
      vert++;

      //                          v4
      //                           |
      //                           |
      //                           |
      //               v3---------v5

      //v3
      vert->point.set(left, bottom, 0.f);
      vert->color = lColor;
      vert++;

      //v4
      vert->point.set(right, top, 0.f);
      vert->color = lColor;
      vert++;

      //v5
      vert->point.set(right, bottom, 0.f);
      vert->color = lColor;
      vert++;

   }

   mVertBuff.unlock();

}
//-----------------------------------------------------------------------------
void tom2DGridRender::onRender(U32 dt, Point3F lOffset)
{
   if (mVertBuff.isNull() || mTriangleCount < 2)
      return;

   GFX->setStateBlock(GFX->getDrawUtil()->get2DStateBlockRef());
   GFX->setVertexBuffer(mVertBuff);
   GFX->setupGenericShaders();
   GFX->drawPrimitive(GFXTriangleList, 0, mTriangleCount);

}

void tom2DGridRender::setWeightColor(U8 index, ColorI lColor)
{
      
   mColorTable[index] = lColor;
}


//-----------------------------------------------------------------------------
DefineEngineMethod(tom2DGridRender, setGrid, void, (BasicGrid* lGrid), , "Set the Grid")
{
   if (lGrid)
      object->setGrid(lGrid);
   else
      Con::printf("tom2DGridRender::setGrid :: Object  doesn't exist");

}
//-----------------------------------------------------------------------------
DefineEngineMethod(tom2DGridRender, update, void, (), , "update the grid")
{
   object->updateVerts();

}

//-----------------------------------------------------------------------------
DefineEngineMethod(tom2DGridRender, setWeightColor, void, (U8 index,ColorI color), , "set a color for a weight")
{
   object->setWeightColor(index,color);

}

