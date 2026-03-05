//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
/*

      [X] scan for Terrain height
            - check empty 
            - set each cell height
      [X] fix rendering - at the moment its only a copy of renderObjectExample
      [X] need client server transfer of flags
      [X] manually Area and SquareSize setup instead of using MissionArea
      [X] add material(s) which are not traversel
        New Material properties:
         bool mBlockBuildings; =>  BlockBuildings
         bool mUnwalkable;     =>  Unwalkable



*/
//-----------------------------------------------------------------------------
#include "scene/sceneRenderState.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "T3D/missionArea.h"
#include "math/mathIO.h"
#include "materials/sceneData.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/gfxDrawUtil.h"

#include "basicgrid.h"
#include "terraingrid.h"
#include "ohmtal/ai/aiMath.h"
#include <T3D/convexShape.h>


IMPLEMENT_CO_NETOBJECT_V1(TerrainGrid);


//-----------------------------------------------------------------------------
TerrainGrid::TerrainGrid()
{
   mNetFlags.set(Ghostable | ScopeAlways);
   mTypeMask |= StaticObjectType | StaticShapeObjectType;

   mArea = RectI(0, 0, 0, 0);
   mSquareSize = 0.f;
   mFaction = 0;

   mGrid = new BasicGrid;
   mGrid->setSquareSize(8);
   mGrid->registerObject(); //2023-12-12 Rock 'n Roll!! 

   mTerrain = NULL;
   mVisible = false;

   mGuessedPlaneCount = 0;

   
   mRenderAlpha = 96;
   //green
   mRenderBaseColor = ColorI(0, 255, 0);


   mRectFillDesc.setZReadWrite(true, false);
   mRectFillDesc.setBlend(true, GFXBlendSrcAlpha, GFXBlendInvSrcAlpha);
   //2023-12-12 look from bottom 
   mRectFillDesc.setCullMode(GFXCullNone); 

}
//-----------------------------------------------------------------------------
TerrainGrid::~TerrainGrid()
{
    mGrid->unregisterObject();
   SAFE_DELETE(mGrid);
}

void TerrainGrid::initPersistFields()
{
   addField("area", TypeRectI, Offset(mArea, TerrainGrid)
      , "Custom Area if not set at create MissionArea is used.");

   addField("squareSize", TypeF32, Offset(mSquareSize, TerrainGrid)
      , "Custom squareSize if not set at create Terrain square size is used.");

   addField("RenderAlpha", TypeS32, Offset(mRenderAlpha, TerrainGrid)
      , "set the transparency of the render grid");

   addField("RenderBaseColor", TypeColorI, Offset(mRenderBaseColor, TerrainGrid)
      , "set the base color (default green) of the render grid - alpha is ignored");


   addField("faction", TypeS32, Offset(mFaction, TerrainGrid)
      , "Set a faction in range 0..255. Can't be changed after creation.");

   Parent::initPersistFields();
}

//-----------------------------------------------------------------------------
bool TerrainGrid::onAdd()
{
   if (getIsEnvCacheClientObject())
   {
      mNetFlags.set(IsGhost);
   }

   if (!Parent::onAdd())
      return false;

   if (isClientObject())
   {
      Con::executef(this, "onClientTerrainGrid", getId());
      if (getIsEnvCacheClientObject())
      {
         if (!init())
            return false;
      }
   }
   else {
      if (!init())
         return false;
   }


   return true;
}
//-----------------------------------------------------------------------------
void TerrainGrid::onRemove()
{
 
   removeFromScene();
   Parent::onRemove();
}

//-----------------------------------------------------------------------------
void TerrainGrid::updateWorldBox()
{
   mObjBox.minExtents.x = mGrid->getArea().point.x;
   mObjBox.minExtents.y = mGrid->getArea().point.y;
   mObjBox.minExtents.z = -1000.f;

   mObjBox.maxExtents.x = mGrid->getArea().point.x + mGrid->getArea().extent.x;
   mObjBox.maxExtents.y = mGrid->getArea().point.y + mGrid->getArea().extent.y;
   mObjBox.maxExtents.z = 1000.f;

   resetWorldBox();
   setRenderTransform(mObjToWorld);

   if (isServerObject())
      setMaskBits(PositionMask);


}
//-----------------------------------------------------------------------------
U32 TerrainGrid::packUpdate(NetConnection* conn, U32 mask, BitStream* stream)
{
   // Allow the Parent to get a crack at writing its info
   U32 retMask = Parent::packUpdate(conn, mask, stream);
   // Write our transform information
   if (stream->writeFlag(mask & PositionMask))
   {
      mathWrite(*stream, mArea);
      stream->write(mSquareSize);
      stream->writeInt(mFaction, 8); //8 bits MAX => 255


      mathWrite(*stream, mObjToWorld);
   }
   if (stream->writeFlag(mask & GridMask) && mGrid->getDirty() && mGrid->getInitDone())
   {
      
      S32 dirtyCount = 0;
      bool dirtyParts = false;
      S32 dirtySend = 0;
      //first send the dirty count!
      for (S32 i = 0; i < mGrid->getNodeCount(); i++)
      {
         if (mGrid->getNodeById(i)->isDirty())
            dirtyCount++;
      }
      if (dirtyCount > 20)
      {
         dirtyCount = 20;
         dirtyParts = true;
      }
         
      stream->write(dirtyCount);

      //next send Flags
      for (S32 i = 0; i < mGrid->getNodeCount(); i++)
      {
         if (mGrid->getNodeById(i)->isDirty()) {
            stream->write(i);
            stream->write(mGrid->getNodeById(i)->getFlags());
            mGrid->getNodeById(i)->setClean();
            dirtySend++;
            if (dirtySend >= dirtyCount)
               break;
         }
      }
      if (!dirtyParts)
         mGrid->setClean();
      else
         setMaskBits(GridMask);
   }


   return retMask;
}
//-----------------------------------------------------------------------------
void TerrainGrid::unpackUpdate(NetConnection* conn, BitStream* stream)
{
   // Let the Parent read any info it sent
   Parent::unpackUpdate(conn, stream);

   if (stream->readFlag())
   {
      //Area
      mathRead(*stream, &mArea);
      stream->read(&mSquareSize);
      mFaction = stream->readInt(8); //8 bits MAX => 0..255

      //Transform
      MatrixF mat;
      mathRead(*stream, &mat);
      setTransform(mat);
      setRenderTransform(mat);

      
   }
   //receive node flags! 
   if (stream->readFlag() && mGrid->getInitDone())
   {
      S32 dirtyCount = 0;
      S32 nodeId = 0;
      U32 flags = 0;
      stream->read(&dirtyCount);
      for (S32 i = 0; i < dirtyCount; i++)
      {
         stream->read(&nodeId);
         stream->read(&flags);
         mGrid->getNodeById(nodeId)->setFlags(flags);
         mGrid->getNodeById(nodeId)->setClean();//keep the client clean

      }
      mGrid->setClean(); //keep the client clean
   }

}
//-----------------------------------------------------------------------------
bool TerrainGrid::init()
{
   if (mGrid->getInitDone())
      return true;

   RectI lArea;
   if (mArea.isValidRect())
   {
      lArea = mArea;
   }
   else {
      MissionArea* missionAreaPtr = MissionArea::getServerObject();
      if (!missionAreaPtr)
      {
         // return false;
         lArea = MissionArea::smMissionArea;
         Con::warnf("MissionArea does not exists!!!!! ");
      }
      else {
         lArea = missionAreaPtr->getArea();
      }
   }

   if (!lArea.isValidRect())
      return false;


   Point2I lAreaCenter = lArea.point + lArea.extent / 2;
   Material* lMaterial = NULL;

   mTerrain = AIMath::getTerrainBlockATpos(Point3F((F32)lAreaCenter.x, (F32)lAreaCenter.y,0.f), isServerObject());
   if (!mTerrain)
   {
     // Con::errorf("TerrainGrid::init - NO TERRAIN FOUND .... isClient=%d", isClientObject());
      return false;  
   }

   if (mSquareSize == 0.f)
      mSquareSize = mTerrain->getSquareSize();

    mGrid->init(lArea, mSquareSize);

    BasicGridNode* lNode;
    U32 mask = StaticShapeObjectType;
    Point3F lSquare3F = Point3F(mGrid->getSquareSize() , mGrid->getSquareSize() , 10.f);

    
    

    F32 lz = 0.f;
    VectorF lNormal = VectorF(0.f, 0.f, 0.f);
    for (U32 i = 0; i < this->mGrid->getNodeCount(); ++i)
    {

       

       lNode = this->mGrid->getNodeById(i);
       // BasicGrid Pos is the top left position not the center !!!
    

       lz = AIMath::getTerrainHeight(lNode->getCenterPos(), mTerrain, isServerObject());
       lNode->setZ(lz);

       lNormal = AIMath::getTerrainNormal(lNode->getCenterPos(), mTerrain, isServerObject());
       lNode->setNormal(lNormal);
       
       
       lMaterial = AIMath::getTerrainMaterialAtpos(lNode->getCenterPos(), isServerObject());
       if (lMaterial)
       {
          if (lMaterial->mBlockBuildings)
             lNode->addFlag(1);
          if (lMaterial->mUnwalkable)
          {
             lNode->addFlag(0);
             lNode->setWeight(BASICGRID_UNWALKABLE);
          }
       }

       if (!lNode->isFlagOn(0) && !AIMath::boxEmpty(lNode->getCenterPos(), lSquare3F, mask, isClientObject()))
       {
          lNode->addFlag(0); //cant walk
          lNode->addFlag(1); //cant build
          lNode->setWeight(BASICGRID_UNWALKABLE);
       }



       lNode->setClean(); //no updates here!! 
    } //loop nodes
    // set the shit clean it's done on server AND client
    mGrid->setClean();
    if (isClientObject())
    {
       mGuessedPlaneCount = (S32)(cRenderDist * cRenderDist / mGrid->getSquareSize() / 2.f);
#ifdef TORQUE_DEBUG
       mGrid->PrintInfo();
#endif
    }

    updateWorldBox();

    addToScene();

    return true;
}

//-----------------------------------------------------------------------------
// Object Rendering
//-----------------------------------------------------------------------------
void TerrainGrid::prepRenderImage(SceneRenderState* state)
{


   if (!state->isDiffusePass() || !mVisible || mGuessedPlaneCount < 1)
   {
      return;
   }
   ObjectRenderInst* ri = state->getRenderPass()->allocInst<ObjectRenderInst>();

   ri->renderDelegate.bind(this, &TerrainGrid::renderSimple);
   ri->type = RenderPassManager::RIT_Editor; //FIXME editor ?! 
   ri->translucentSort = true;
   ri->defaultKey = 1;
   state->getRenderPass()->addInst(ri);


}

//--------------------------------------------------------------------------------------------------------------------------------

void TerrainGrid::renderSimple(ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* mi)
{

   

   GFXDrawUtil* drawer = GFX->getDrawUtil();


   F32 lMaxDist = getMin((F32)state->getFarPlane(), cRenderDist);
   // Fetch Camera Position.
   Point3F CameraPosition = state->getCameraPosition();
   // Get the object space y vector

   Point3F CamVector = state->getVectorEye(); //thats ok

/*
#ifdef TORQUE_DEBUG
   ColorI myColorICenter = ColorI(0, 0, 255, 255);
#endif
*/
   ColorI myColorI;
   BasicGridNode* curNode;


   //F32 lHalfPlane = (mGrid->getSquareSize() - 2.f) / 2.f;

   F32 lHalfPlane = mGrid->getSquareSize() / 4.f;

   const S32 vertCount = 6;

//FIXME prefill the buffer after create and update on modify not every drawcall!!!
// search for flareState->vertBuffer for an example
// 
#pragma message("TerrainGrid::renderSimple :: prefill the buffer after create and update on modify not every drawcall!!!")


   GFXVertexBufferHandle<GFXVertexPCT> lVerts(GFX, mGuessedPlaneCount * vertCount, GFXBufferTypeStatic);
   lVerts.lock();
   U32 lPlaneCount = 0;
   // render the nodes
   for (U32 i = 0; i < this->mGrid->getNodeCount(); ++i)
   {


      curNode = this->mGrid->getNodeById(i);
      //XXTH only visible :
      if (!AIMath::isVisible(curNode->getCenterPos(), CameraPosition, CamVector, lMaxDist, this->mGrid->getSquareSize()))
         continue;

      

      //* fps from 400 to 2 !! 
      if (curNode->getPathWeight() == BASICGRID_UNWALKABLE || curNode->isFlagOn(0))  //unwalkable make it red
         myColorI = ColorI(255, 0, 0, mRenderAlpha);
      else if (curNode->isFlagOn(1))  //cant build here! 
         myColorI = ColorI(0, 0, 255, mRenderAlpha);
      else if (curNode->getPathWeight() == 0)  //best walkable make it green
      {
         myColorI = mRenderBaseColor;
         myColorI.alpha = mRenderAlpha;
            //ColorI(0, 255, 0, mRenderAlpha);
      }
      else if (curNode->getPathWeight() < 128)  //medium walkable
         myColorI = ColorI(255, 255, 128, mRenderAlpha);
      else // movable but not as good
         myColorI = ColorI(192, 255, 192, mRenderAlpha);


      //THIS IS FROM  drawSolidPlane
      // void GFXDrawUtil::drawSolidPlane( const GFXStateBlockDesc &desc, const Point3F &pos, const Point2F &size, const ColorI &color )

      //               v0------v2
      //               |     / |
      //               |   /   |
      //               | /     |
      //               v1-----


      // BasicGrid Pos is the top left position not the center !!!
      // so i need to add lHalfPlane 

      Point3F pos = curNode->getCenterPos() + Point3F(0.f, 0.f, 1.f);

      lVerts[lPlaneCount * vertCount].point = pos + Point3F(-lHalfPlane, -lHalfPlane, 0.f);
      lVerts[lPlaneCount * vertCount + 1].point = pos + Point3F(-lHalfPlane, lHalfPlane, 0.f);
      lVerts[lPlaneCount * vertCount + 2].point = pos + Point3F(lHalfPlane, -lHalfPlane, 0.f);


      //                ------v4
      //               |     / |
      //               |   /   |
      //               | /     |
      //               v5-----v3

      lVerts[lPlaneCount * vertCount + 3].point = pos + Point3F(lHalfPlane, lHalfPlane, 0.f);
      lVerts[lPlaneCount * vertCount + 4].point = pos + Point3F(lHalfPlane, -lHalfPlane, 0.f);
      lVerts[lPlaneCount * vertCount + 5].point = pos + Point3F(-lHalfPlane, lHalfPlane, 0.f);

      for (S32 i = 0; i < vertCount; i++)
      {
         lVerts[lPlaneCount * vertCount + i].color = myColorI;
         //lol added normal to BasicGridNode but cant use it this way  ..lVerts[lPlaneCount * vertCount + i].
      }



      //emergExit
      if (lPlaneCount > mGuessedPlaneCount - 2)
      {
         Con::errorf("TerrainGrid::renderSimple guessedPlaneCount is too low!!!");
         break;
      }

      lPlaneCount++;

/*
#ifdef TORQUE_DEBUG

      drawer->drawLine(curNode->getPos(), curNode->getPos() + Point3F(0.f, 0.f, 5.f), myColorICenter);
      drawer->drawLine(curNode->getCenterPos(), curNode->getCenterPos() + Point3F(0.f, 0.f, 5.f), myColorI);
#endif // TORQUE_DEBUG
*/


   } //loop nodes

   lVerts.unlock();
   GFX->setStateBlockByDesc(mRectFillDesc);
   GFX->setVertexBuffer(lVerts);
   GFX->setupGenericShaders();
   GFX->drawPrimitive(GFXTriangleList, 0, lPlaneCount * 2 ); //vertCount  == 6 double triangle looks better :P
   

}

bool TerrainGrid::addFlag(F32 x, F32 y, U8 lFlagId)
{
   BasicGridNode* lNode = getGrid()->findNode(x, y);

   if (lNode)
   {
      lNode->addFlag(lFlagId);
      setMaskBits(GridMask);
      return true;
   }
   return false;
}

//--------------------------------------------------------------------------------------------------------------------------------
// Script 
//--------------------------------------------------------------------------------------------------------------------------------
DefineEngineMethod(TerrainGrid, clientInit, bool, (), , "Init the Terrain Grid on Client side.")
{
   if (object->isClientObject())
   {
      return object->init();
      
   }
      
   Con::errorf("TerrainGrid clientInit only allowed on client side!!");
   return false;
}

DefineEngineMethod(TerrainGrid, setVisible, void, (bool lVisible), , "Init the Terrain Grid on Client side.")
{
   if (object->isClientObject())
   {
      object->setVisible(lVisible);
      return;
   }

   Con::errorf("TerrainGrid setVisible only allowed on client side!!");
}

DefineEngineMethod(TerrainGrid, info, void, (bool listNodes),(false), "grid info")
{
   object->getGrid()->PrintInfo(listNodes);
   
}

//--------------------------------------------------------------------------------------------------------------------------------
DefineEngineMethod(TerrainGrid, setFlags, bool, (F32 x, F32 y, U32 flags), , "x,y; set flags ")
{
   BasicGridNode* lNode = object->getGrid()->findNode(x, y);

   if (lNode)
   {
      lNode->setFlags(flags);
      return true;
   }

   return false;
}

DefineEngineMethod(TerrainGrid, flagOn, bool, (F32 x, F32 y, U8 flagId), , "x,y; U8 flagId ")
{

   return object->addFlag(x,y, flagId);
}

/*
Test:
$TerrainGrid.flagOnPoint($client::lastMouseUp,0);
*/

DefineEngineMethod(TerrainGrid, flagOnPoint, bool, (Point3F point, U8 flagId), , "Point3F point, U8 flagId ")
{
   return object->addFlag(point.x, point.y, flagId);
}


DefineEngineMethod(TerrainGrid, isflagOn, bool, (F32 x, F32 y, U8 flagId), , "x,y; U8 flagId ")
{
   BasicGridNode* lNode = object->getGrid()->findNode(x, y);

   if (lNode)
   {
      lNode->addFlag(flagId);
      return true;
   }

   return false;
}

DefineEngineMethod(TerrainGrid, getGrid, S32, (), , "Return the GridObject ID")
{
   return object->getGrid()->getId();
}
