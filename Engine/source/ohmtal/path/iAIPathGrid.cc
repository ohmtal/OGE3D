/*------------------------------------------------------------------ -
The MIT License(MIT)

Copyright(c) 2006 Gavin Bunney and Tom Romano
Copyright(c) 2009/23 huehn-software / Ohmtal Game Studio

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
https://github.com/dawogfather/immersiveAI/
-------------------------------------------------------------------*/

//XXTH GRID NEED PARTIONING for speed up!!!!!!

#include "terrain/terrData.h"


#include "scene/sceneManager.h"
#include "platform/profiler.h"

#include "scene/sceneRenderState.h"
#include "gfx/gfxDrawUtil.h"
#include "renderInstance/renderPassManager.h"
#include "gfx/primBuilder.h"

#include "iAIPathNode.h"
#include "iAIPathGrid.h"
#include "iAIPathMap.h"
#include "iAIPathGlobal.h"
#include "iAIPathFind.h"

#include "ohmtal/misc/terrainTweaker.h"
#include "ohmtal/ai/aiMath.h"
#include <T3D/gameBase/gameProcess.h>


IMPLEMENT_CONOBJECT(iAIPathGrid);
//--------------------------------------------------------------------------------------------------------------------------------
iAIPathGrid::iAIPathGrid()
{
   //T3D >>> 
   mTypeMask |= StaticObjectType | StaticShapeObjectType;
   //<<<<

	this->setPosition(Point3F(0,0,0));
	// this->mTypeMask |= iAIPathGridObjectType;

	this->mCompiled = false;
	this->mDensity = 0.0f;
	this->mGridBox = Box3F(0,0,0, 0,0,0);
	this->mShow = false;
	this->mNodesCountX = 0;
	this->mNodesCountY = 0;
}
//--------------------------------------------------------------------------------------------------------------------------------
iAIPathGrid::~iAIPathGrid()
{
	while (!this->mNodes.empty())
	{
		this->mNodes.erase((U32)0);
	}
	this->mNodes.clear();
}
//--------------------------------------------------------------------------------------------------------------------------------
bool iAIPathGrid::onAdd()
{


	// call Parent, ensure worked
	if (!Parent::onAdd())
	   return false;

	// create object box
	this->updateWorldBox();

	// add to scene
    //is done in addObjectToScene gClientContainer.addObject(this);

   mNetFlags.set(IsGhost);
   gClientSceneGraph->addObjectToScene(this);


	
	return true;
}
//--------------------------------------------------------------------------------------------------------------------------------
void iAIPathGrid::onRemove()
{
	// remove from scene
	removeFromScene();
	Parent::onRemove();
}
//--------------------------------------------------------------------------------------------------------------------------------
void iAIPathGrid::prepRenderImage(SceneRenderState* state)
{
   if (!this->mShow)
      return;

   ObjectRenderInst* ri = state->getRenderPass()->allocInst<ObjectRenderInst>();
   
   ri->renderDelegate.bind(this, &iAIPathGrid::renderSimple);
   ri->type = RenderPassManager::RIT_Editor;
   ri->translucentSort = true;
   ri->defaultKey = 1;
   state->getRenderPass()->addInst(ri);

}
//--------------------------------------------------------------------------------------------------------------------------------
void iAIPathGrid::renderSimple(ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* mi)
{

   if (state->isReflectPass() || !this->mShow)
      return;



   GFXDrawUtil* drawer = GFX->getDrawUtil();

   /*
   GFXStateBlockDesc desc;
   desc.setZReadWrite(true, false);
   desc.setBlend(true);
   desc.setCullMode(GFXCullNone);
   if (isSelected())
   {
      drawer->drawCube(desc, getWorldBox(), ColorI(136, 255, 228, 5));
      desc.setFillModeWireframe();
      drawer->drawCube(desc, getWorldBox(), ColorI::BLACK);
   }
   */


   F32 lMaxDist = getMin((F32)state->getFarPlane(), 100.f);
   // Fetch Camera Position.
   Point3F CameraPosition = state->getCameraPosition();
   // Get the object space y vector

   /*
   Point3F CamVector;
   MatrixF mv = state->getDiffuseCameraTransform(); //no idea if this this right lol
   mv.inverse();
   mv.getColumn(1, &CamVector);
   */
   Point3F CamVector = state->getVectorEye(); //thats ok


   ColorI myColorI;
   // render the nodes
   for (U32 i = 0; i < this->mNodes.size(); ++i)
   {

      //XXTH only visible :
      if (!AIMath::isVisible(this->mNodes[i]->mPosition, CameraPosition, CamVector, lMaxDist, this->mGridSize))
         continue;

      
//* fps from 400 to 2 !! 
      if (this->mNodes[i]->mMoveModifier == 255)  //unmovable make it red
         myColorI = ColorI(255, 0, 0, 255);
      else if (this->mNodes[i]->mMoveModifier == 0)  //best movable make it green
         myColorI = ColorI(0, 255, 0, 255);
      else if (this->mNodes[i]->mMoveModifier < 128)  //good movable make it ?
         myColorI = ColorI(255, 255, 128, 255);
      else // movable but not as good
         myColorI = ColorI(192, 255, 192, 255);

      drawer->drawLine(
         this->mNodes[i]->mPosition + IAIPATHGLOBAL_GRID_RENDER_CLEARANCE
         , this->mNodes[i]->mPosition + IAIPATHGLOBAL_GRID_RENDER_CLEARANCE + IAIPATHGLOBAL_GRID_RENDER_NODE_HEIGHT
         , myColorI
      );

      for (U32 j = 0; j < this->mNodes[i]->mNeighbours.size(); ++j)
      {
         if (this->mNodes[i]->mNeighbours[j])
         {
            drawer->drawLine(
               this->mNodes[i]->mPosition + IAIPATHGLOBAL_GRID_RENDER_CLEARANCE
               , this->mNodes[i]->mNeighbours[j]->mPosition + IAIPATHGLOBAL_GRID_RENDER_CLEARANCE
               , ColorI(IAIPATHGLOBAL_GRID_RENDER_COLOUR)
               );

         }
      }
   }

   // render the gridbox
   /*
   drawer->drawCube()
   glColor4ub(IAIPATHGLOBAL_GRID_RENDER_BOX_COLOUR);
   dglWireCube(Point3F(this->mObjBox.len_x() / 2, this->mObjBox.len_y() / 2, this->mObjBox.len_z() / 2), this->getBoxCenter());
   */


   /*
	F32 lMaxDist = getMin((F32)state->getFarPlane(),100.f );
	// Fetch Camera Position.
	Point3F CameraPosition  = state->getCameraPosition();
    // Get the object space y vector
    Point3F CamVector;
    MatrixF mv = state->mModelview;
    mv.inverse();
    mv.getColumn(1, &CamVector);



	// save matrix to restore canonical state
	glPushMatrix();
	
	// going to render some lines!
	glBegin(GL_LINES);
	
	// render the nodes
	for (U32 i = 0; i < this->mNodes.size(); ++i)
	{

		//XXTH only visible :
		if (!AIMath::isVisible( this->mNodes[i]->mPosition, CameraPosition, CamVector, lMaxDist, this->mGridSize)  )
			 continue;

		//glColor4ub(IAIPATHGLOBAL_GRID_RENDER_NODE_COLOUR);
        //XXTH render stick in this->mPathNodes[j]->mMoveModifier (weight) color
		// glColor4ub(this->mNodes[i]->mMoveModifier, this->mNodes[i]->mMoveModifier, this->mNodes[i]->mMoveModifier, 255);
		
		//make colors nicer :P
		
		if (this->mNodes[i]->mMoveModifier == 255)  //unmovable make it red
				glColor4ub(255 , 0, 0, 255);
		else if (this->mNodes[i]->mMoveModifier == 0)  //best movable make it green
				glColor4ub(0 , 255 , 0, 255);
		else if (this->mNodes[i]->mMoveModifier < 128)  //good movable make it ?
				glColor4ub(255, 255 , 128, 255);
		else // movable but not as good
				glColor4ub(192, 255 , 192, 255);

		glVertex3fv(this->mNodes[i]->mPosition + IAIPATHGLOBAL_GRID_RENDER_CLEARANCE);
		glVertex3fv(this->mNodes[i]->mPosition + IAIPATHGLOBAL_GRID_RENDER_CLEARANCE + IAIPATHGLOBAL_GRID_RENDER_NODE_HEIGHT);

		// render neighbour links
		glColor4ub(IAIPATHGLOBAL_GRID_RENDER_COLOUR);
		for (U32 j = 0; j < this->mNodes[i]->mNeighbours.size(); ++j)
		{
			if (this->mNodes[i]->mNeighbours[j])
			{
				glVertex3fv(this->mNodes[i]->mPosition + IAIPATHGLOBAL_GRID_RENDER_CLEARANCE);
				glVertex3fv(this->mNodes[i]->mNeighbours[j]->mPosition + IAIPATHGLOBAL_GRID_RENDER_CLEARANCE);
			}
		}
	}

	// render the gridbox
	glColor4ub(IAIPATHGLOBAL_GRID_RENDER_BOX_COLOUR);
	dglWireCube(Point3F(this->mObjBox.len_x()/2, this->mObjBox.len_y()/2, this->mObjBox.len_z()/2), this->getBoxCenter());

	// end of line drawing
	glEnd();

	// restore canonical rendering state
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	// restore canonical maxtrix state
	glPopMatrix();

	// ensure canonical state is restored
	AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");

*/
}
//--------------------------------------------------------------------------------------------------------------------------------
void iAIPathGrid::toggleDisplay()
{
	this->mShow = !this->mShow;
}
//--------------------------------------------------------------------------------------------------------------------------------
void iAIPathGrid::clearGrid()
{
	this->mNodes.clear();
	this->mCompiled = false;
	this->mDensity = 0.0f;
	this->mGridBox = Box3F(0,0,0, 0,0,0);
	this->mNodesCountX = 0;
	this->mNodesCountY = 0;
}
//--------------------------------------------------------------------------------------------------------------------------------
bool iAIPathGrid::createCustomGrid(const Point3F worldStart, const Point3F worldEnd,  Vector<Box3F> &avoidList, const F32 density, U32 typeMask)
{
	// clear all the nodes
	this->clearGrid();

	// set the grid start & end bounds
	this->mGridBox = Box3F(worldStart, worldEnd);
	
	// check the gridbox is actually valid
	if (!this->mGridBox.isValidBox())
	{
		Con::errorf("Immersive AI :: Seek :: Grid build failed - grid box not valid!");
		return false;
	}

	// the density is nodes per gridsize; default gridSize to 10.0f if none found
	this->mGridSize = Con::getFloatVariable("$Server::gridSize", 10.0f);
	

	this->mDensity = density / this->mGridSize;

	// density step needs to be the squareroot, as operates in both X & Y
	F32 densityStep = 1 / mSqrt(this->mDensity);

	//XXTH U16 bitmapX, bitmapY;
	U8 lNodeWeight = 0; 
    
	// calculate the count of nodes in x & y
	this->mNodesCountX = this->mGridBox.len_x() * mSqrt(this->mDensity);
	this->mNodesCountY = this->mGridBox.len_y() * mSqrt(this->mDensity);


	// create all nodes
	RayInfo lRay;
	Point3F lCenter;
	Point3F lExtent = Point3F(1.f,1.f,0.3f);
	bool lBadNode;
   bool lBadSlope;
	for (U16 iterX = 0; iterX < this->mNodesCountX; ++iterX)
	{
		for (U16 iterY = 0; iterY < this->mNodesCountY; ++iterY)
		{
			Point3F nodePos = this->mGridBox.minExtents;
			nodePos.x += densityStep * iterX;
			nodePos.y += densityStep * iterY;
			nodePos.z = worldEnd.z;

			lBadNode = true;
			if (gServerContainer.castRay(Point3F(nodePos.x,nodePos.y,worldEnd.z), Point3F(nodePos.x,nodePos.y,worldStart.z), typeMask, &lRay)) {
  		      
		       nodePos.z = lRay.point.z + 0.5f;//XXTH prior 1.97 was: 0.5f;
			   lCenter = Point3F(nodePos.x,nodePos.y,nodePos.z+lExtent.z);

             //2023-03-01 check slope!
             lBadSlope = mSqrt(lRay.normal.y * lRay.normal.y) > 0.6f;
			   if (!lBadSlope && AIMath::boxEmpty(lCenter, lExtent, IAIPATHGLOBAL_COLLISION_MASK) )
			   {
				   // create the node
				   lBadNode = false;
				   lNodeWeight = 0;
			   } else {
                   lNodeWeight = 1; 
				   //lBadNode = true;
			   }
			}
            iAIPathNode *newNode = new iAIPathNode(nodePos, this, iterX, iterY, lNodeWeight);
			newNode->mBadNode = lBadNode;
		    this->mNodes.push_back(newNode);
		
		}
	}

    this->linkNeighbours(avoidList, true); //2023-04-03 why? =>  false);

	// set as compiled if any nodes in the grid
	this->mCompiled = (this->mNodes.size() > 0);

	// update the world box so renders properly
	this->updateWorldBox();

	return this->mCompiled;
}
//--------------------------------------------------------------------------------------------------------------------------------
// XXTH 2023-03-10 custom grid where to add nodes and neighbours manually 
//--------------------------------------------------------------------------------------------------------------------------------
bool iAIPathGrid::initCustomGrid(S32 lGridSize)
{
   // clear all the nodes
   this->mGridSize = lGridSize;
   this->clearGrid();
   return true;
}

bool iAIPathGrid::AddNodeCustomGrid(Point3F position)
{
   //check if we have a node there !! 
   iAIPathNode* node1 = this->getClosestNode(position);
   if (node1 && AIMath::Distance3D(node1->getPosition(), position) < this->mGridSize * 2)
   {

#ifdef TORQUE_DEBUG
      Con::warnf("iAIPathGrid::AddNodeCustomGrid failed to add node .. thats BAD check GridSize!!");
#endif
      return false;
   }

   S32 newIDx = this->mNodes.size();
   iAIPathNode* newNode = new iAIPathNode(position, this, newIDx, 0, 0);

   if (newNode)
   {
      this->mNodes.push_back(newNode);
      return true;
   }
      

   return false;
}
bool iAIPathGrid::LinkNodesInCustomGrid(Point3F pos1, Point3F pos2)
{
   iAIPathNode* node1 = this->getClosestNode(pos1);
   iAIPathNode* node2 = this->getClosestNode(pos2);
   if (node1 && node2)
   {
      if (
         node1->addNeighbourNoCheck(node2)
         &&
         node2->addNeighbourNoCheck(node1)
         )
      {
#ifdef TORQUE_DEBUG
         Con::printf("iAIPathGrid::LinkNodesInCustomGrid linked nodes (%f,%f,%f) => (%f,%f,%f)",
            node1->mPosition.x, node1->mPosition.y, node1->mPosition.z,
            node2->mPosition.x, node2->mPosition.y, node2->mPosition.z);
#endif
         return true;
      }
      else {
#ifdef TORQUE_DEBUG
         Con::errorf("iAIPathGrid::LinkNodesInCustomGrid linked nodes FAILED! (%f,%f,%f) => (%f,%f,%f)",
            node1->mPosition.x, node1->mPosition.y, node1->mPosition.z,
            node2->mPosition.x, node2->mPosition.y, node2->mPosition.z);
#endif

      }
   }
   else {
#ifdef TORQUE_DEBUG
      Con::errorf("iAIPathGrid::LinkNodesInCustomGrid linked nodes FAILED!");
      if (node1)
         Con::errorf("iAIPathGrid::LinkNodesInCustomGrid but node1 found : (%f,%f,%f)",
            node1->mPosition.x, node1->mPosition.y, node1->mPosition.z);
      if (node2)
         Con::errorf("iAIPathGrid::LinkNodesInCustomGrid but node2 found : (%f,%f,%f)",
            node2->mPosition.x, node2->mPosition.y, node2->mPosition.z);
#endif
   }

   return false;
}
bool iAIPathGrid::compileCustomGrid()
{
   this->mCompiled = (this->mNodes.size() > 0);
   // set the grid start & end bounds
   //extend box
   Point3F lExt = Point3F(20.f, 20.f, 5.f);

   //find out the gridBox
   Point3F lWorldStart;
   Point3F lWorldEnd;
   Point3F lPos;
   lWorldStart = Point3F(10000.f, 10000.f, 10000.f);
   lWorldEnd = Point3F(-10000.f, -10000.f, -10000.f);

   for (S32 i = 0; i < this->mNodes.size(); i++)
   {
      if (this->mNodes[i]->mBadNode)
         continue;

      lPos = this->mNodes[i]->mPosition;
      if (lPos.x < lWorldStart.x)
         lWorldStart.x = lPos.x;
      if (lPos.x > lWorldEnd.x)
         lWorldEnd.x = lPos.x;
      if (lPos.y < lWorldStart.y)
         lWorldStart.y = lPos.y;
      if (lPos.y > lWorldEnd.y)
         lWorldEnd.y = lPos.y;
      if (lPos.z < lWorldStart.z)
         lWorldStart.z = lPos.z;
      if (lPos.z > lWorldEnd.z)
         lWorldEnd.z = lPos.z;
   }


   lWorldStart -= lExt;
   lWorldEnd += lExt;
   this->mGridBox = Box3F(lWorldStart, lWorldEnd);

   // update the world box so renders properly

    // set position as halfway point
   this->setPosition(lWorldStart + ((lWorldEnd - lWorldStart) / 2));

   // create a box to encompass the entire path
   this->mObjBox.minExtents.set(-(this->getPosition() - lWorldStart));
   this->mObjBox.maxExtents.set(lWorldEnd - this->getPosition());

   // must reset world box & transform when changing object box
   this->resetWorldBox();
   this->setRenderTransform(mObjToWorld);

   return this->mCompiled;

}
//--------------------------------------------------------------------------------------------------------------------------------
bool iAIPathGrid::createCustomGridFromMarkers(SimPath::Path *pathObject)
{

   
	if (!pathObject)
		return false;

	// clear all the nodes
	this->clearGrid();


	Point3F lWorldStart;
	Point3F lWorldEnd;
	Point3F lPos;
   bool lInitWorldPos = true; //2.31
	this->mGridSize = 50.f; //hardcoded DO NOT CHANGE!
	F32 lDist = 0;
	F32 lMaxDist = 0;

	this->mNodesCountX=this->mNodesCountY=0;
   
	for (S32 i = 0; i < pathObject->size(); i++)
		{
			Marker* pMarker = static_cast<Marker*>((*pathObject)[i]);
			if (pMarker != NULL) 
			{


				//find out the gridBox
				lPos = pMarker->getPosition();

            if (lInitWorldPos) //2.31
            {
               lWorldStart = lPos;
               lWorldEnd = lPos;
               lInitWorldPos = false;
            }

				if (lPos.x < lWorldStart.x)
					lWorldStart.x = lPos.x;
				if (lPos.x > lWorldEnd.x)
					lWorldEnd.x = lPos.x;
				if (lPos.y < lWorldStart.y)
					lWorldStart.y = lPos.y;
				if (lPos.y > lWorldEnd.y)
					lWorldEnd.y = lPos.y;
				if (lPos.z < lWorldStart.z)
					lWorldStart.z = lPos.z;
				if (lPos.z > lWorldEnd.z)
					lWorldEnd.z = lPos.z;

				iAIPathNode *newNode = new iAIPathNode(lPos, this, i, 0, 0);
				//temp usage of fitness , will be set to 0 later again!
				newNode->mFitness = (F32)pMarker->mMSToNext;
				this->mNodes.push_back(newNode);
				//this->mNodesCountX++;
			}
	}

	U32 lNodeCount = this->mNodes.size();
	// set as compiled if any nodes in the grid
	this->mCompiled = (lNodeCount > 0);


	F32 lDefDist = (F32) this->mGridSize*2;

	if (this->mCompiled)
	{
       for (U32 i=0; i<lNodeCount; i++)
		   for (U32 j=0; j<lNodeCount; j++)
			   if (i!=j)
			   {
#ifdef TORQUE_DEBUG
					iAIPathNode* node1 = this->mNodes[i];
					iAIPathNode* node2 = this->mNodes[j];

					Con::printf("Marker Node (%f,%f,%f) => (%f,%f,%f)",
						node1->mPosition.x,node1->mPosition.y,node1->mPosition.z,
						node2->mPosition.x,node2->mPosition.y,node2->mPosition.z);
#endif

                   lDist =  AIMath::Distance2D(this->mNodes[i]->mPosition,this->mNodes[j]->mPosition);
				   lMaxDist = getMin(lDefDist,getMin( this->mNodes[i]->mFitness,  this->mNodes[j]->mFitness));

				   if (lDist <= lMaxDist
					   && this->mNodes[i]->addNeighbour(this->mNodes[j],false))
				   {
#ifdef TORQUE_DEBUG
					   Con::printf("Marker Node linked: linked nodes (%f,%f,%f) => (%f,%f,%f)",
							this->mNodes[i]->mPosition.x,this->mNodes[i]->mPosition.y,this->mNodes[i]->mPosition.z,
							this->mNodes[j]->mPosition.x,this->mNodes[j]->mPosition.y,this->mNodes[j]->mPosition.z);
#endif
				   }
			   }

				



	}

	//reset fitness again:
	 for (U32 i=0; i<lNodeCount; i++)
		this->mNodes[i]->mFitness=0.f;



	// set the grid start & end bounds
	//extend box
	Point3F lExt = Point3F(20.f,20.f,5.f);
	lWorldStart-=lExt;
	lWorldEnd+=lExt;
	this->mGridBox = Box3F(lWorldStart, lWorldEnd);

	// update the world box so renders properly

    // set position as halfway point
	this->setPosition(lWorldStart + ((lWorldEnd - lWorldStart) / 2));
		
		// create a box to encompass the entire path
		this->mObjBox.minExtents.set(-(this->getPosition() - lWorldStart));
		this->mObjBox.maxExtents.set(lWorldEnd - this->getPosition());

		// must reset world box & transform when changing object box
		this->resetWorldBox();
		this->setRenderTransform(mObjToWorld);




	return this->mCompiled;
}

//--------------------------------------------------------------------------------------------------------------------------------
bool iAIPathGrid::createTerrainGrid(const Point3F worldStart, const Point3F worldEnd, GFXTexHandle mMapHandle, Vector<Box3F> &avoidList, const F32 density)
{
	// grab the terrain & ensure valid

   Point3F myPoint = worldEnd + worldStart;

   TerrainBlock* terrain = AIMath::getTerrainBlockATpos(myPoint, true);// OhmtalTerrainTools::getTerrainUnderWorldPoint(myPoint);// dynamic_cast<TerrainBlock*>(Sim::findObject("Terrain"));
	if (!terrain)
		return false;

	// clear all the nodes
	this->clearGrid();

	// set the grid start & end bounds
	this->mGridBox = Box3F(worldStart, worldEnd);
	
	// check the gridbox is actually valid
	if (!this->mGridBox.isValidBox())
	{
		Con::errorf("Immersive AI :: Seek :: Grid build failed - grid box not valid!");
		return false;
	}

	// the density is nodes per gridsize; default gridSize to 10.0f if none found
	this->mGridSize = Con::getFloatVariable("$Server::gridSize", 10.0f);
	

	this->mDensity = density / this->mGridSize;

	// density step needs to be the squareroot, as operates in both X & Y
	F32 densityStep = 1 / mSqrt(this->mDensity);

	U16 bitmapX, bitmapY;
	U8 mNodeWeight = 0; 
    
	GBitmap *bmp = mMapHandle.getBitmap();

	// calculate the count of nodes in x & y
	this->mNodesCountX = this->mGridBox.len_x() * mSqrt(this->mDensity);
	this->mNodesCountY = this->mGridBox.len_y() * mSqrt(this->mDensity);

    U32 terrainhalfsize = terrain->getSquareSize() * 128;
	F32 halfSquare = terrain->getSquareSize() / 2;


	// create all nodes
	for (U16 iterX = 0; iterX < this->mNodesCountX; ++iterX)
	{
		for (U16 iterY = 0; iterY < this->mNodesCountY; ++iterY)
		{
			Point3F nodePos = this->mGridBox.minExtents;
			nodePos.x += densityStep * iterX;
			nodePos.y += densityStep * iterY;

				if (bmp) {
					//XXTH get bitmap position on a 512x512 bitmap
					bitmapX = mFloor((nodePos.x + terrainhalfsize) / halfSquare);
					bitmapY = mFloor((nodePos.y - terrainhalfsize) / (halfSquare * -1));
				   
					ColorI rColor;
					if (bmp->getColor(bitmapX, bitmapY,rColor)) {
							mNodeWeight = (rColor.red  + rColor.green + rColor.blue) / 3;
					}
				}

			// transform point to terrain transform
			terrain->getWorldTransform().mulP(nodePos);
			nodePos.convolveInverse(terrain->getScale());
			F32 height;
			if (terrain->getHeight(Point2F(nodePos.x, nodePos.y), &height))
			{
				nodePos.z = height;
				nodePos.convolve(terrain->getScale());
				terrain->getTransform().mulP(nodePos);
			}

			// create the node
            iAIPathNode *newNode = new iAIPathNode(nodePos, this, iterX, iterY, mNodeWeight);
		    this->mNodes.push_back(newNode);
		
		}
	}

	this->linkNeighbours(avoidList);

	// set as compiled if any nodes in the grid
	this->mCompiled = (this->mNodes.size() > 0);

	// update the world box so renders properly
	this->updateWorldBox();

	return this->mCompiled;
}
//--------------------------------------------------------------------------------------------------------------------------------
void iAIPathGrid::linkNeighbours(Vector<Box3F> &avoidList ,bool clearanceCheck ) 
{
	// join all the node neighbours
	for (U32 iter = 0; iter < this->mNodes.size(); ++iter)
	{
		U16 currentX = this->mNodes[iter]->mIdX;
		U16 currentY = this->mNodes[iter]->mIdY;
		U8 badNodes = 0;

		/* NO we keep the unwalkable nodes for altering movemodifier!
		if (this->mNodes[iter]->mMoveModifier==IAIPATHGLOBAL_MOVE_MODIFIER_UNTRAVERSAL) {
			 this->mNodes[iter]->mBadNode=true;
			 continue;
		}
		*/
		if (this->mNodes[iter]->mBadNode)
			continue;

		// north
		if (currentY < (this->mNodesCountY - 1))
			if (!this->mNodes[iter]->addNeighbour(this->mNodes[iter+1],clearanceCheck)) badNodes++;

		// south
		if (currentY > 0)
			if (!this->mNodes[iter]->addNeighbour(this->mNodes[iter-1],clearanceCheck)) badNodes++;

		// east
		if (currentX < (this->mNodesCountX-1))
			if (!this->mNodes[iter]->addNeighbour(this->mNodes[iter + this->mNodesCountY],clearanceCheck)) badNodes++;

		// west
		if (currentX > 0)
			if (!this->mNodes[iter]->addNeighbour(this->mNodes[iter - this->mNodesCountY],clearanceCheck)) badNodes++;

		// north-east
		if (currentX < (this->mNodesCountX - 1) && (currentY < (this->mNodesCountY - 1)))
			if (!this->mNodes[iter]->addNeighbour(this->mNodes[ iter + this->mNodesCountY + 1],clearanceCheck)) badNodes++;

		// south-east
		if (currentX < (this->mNodesCountX - 1) && (currentY > 0))
			if (!this->mNodes[iter]->addNeighbour(this->mNodes[ iter+this->mNodesCountY-1]),clearanceCheck) badNodes++;

		// south-west
		if ((currentX > 0) && (currentY > 0))
			if (!this->mNodes[iter]->addNeighbour(this->mNodes[ iter - this->mNodesCountY - 1 ],clearanceCheck)) badNodes++;

		// north-west
		if ((currentX > 0) && (currentY < (this->mNodesCountY - 1)))
			if (!this->mNodes[iter]->addNeighbour(this->mNodes[ iter - this->mNodesCountY + 1 ],clearanceCheck)) badNodes++;

		//XXTH remove existing neighbours on bad nodes
/* XXTH 1.97 mhhh 
		if (badNodes>3) {
			//sucks  this->mNodes[iter]->mMoveModifier=IAIPATHGLOBAL_MOVE_MODIFIER_UNTRAVERSAL;
			this->mNodes[iter]->mBadNode = true;
		}
*/
	}

	// cull invalid and alone nodes
	for (U32 gridIter = 0; gridIter < this->mNodes.size(); ++gridIter)
	{
		// remove the node if in an invalid position or has no neighbours or in avoid list
		if ((!this->mNodes[gridIter]->isClear(clearanceCheck)) || (this->mNodes[gridIter]->mNeighbours.size() == 0) ||
			this->isInAvoidList(this->mNodes[gridIter], avoidList)
			|| this->mNodes[gridIter]->mBadNode //XXTH
			)
		{
			// remove it from its neighbours first
			for (U32 j = 0; j < this->mNodes[gridIter]->mNeighbours.size(); ++j)
			{
				this->mNodes[gridIter]->mNeighbours[j]->removeNeighbour(this->mNodes[gridIter]);
			}
			
			// remove from grid list
			this->mNodes.erase(gridIter);
		}
	}

}
//--------------------------------------------------------------------------------------------------------------------------------
bool iAIPathGrid::isInAvoidList(const iAIPathNode *node, const Vector<Box3F> &avoidList)
{
	// iterate over all boxes in the avoid list
	for (U32 z = 0; z < avoidList.size(); ++z)
	{
		Box3F nodeBox = Box3F(node->mPosition - (IAIPATHGLOBAL_NODE_CLEARANCE/2), node->mPosition + (IAIPATHGLOBAL_NODE_CLEARANCE/2));

		// see if the avoid list box overlaps the nodes box
		if (avoidList[z].isOverlapped(nodeBox))
			return true;
	}

	// didn't overlap on any boxes
	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------
void iAIPathGrid::updateWorldBox()
{
	if (this->mCompiled && this->mNodesCountY > 0) //XXTH this->mNodesCountY>0 hack for marker worldbox
	{
		Point3F min = Point3F(this->mNodes.front()->mPosition);
		Point3F max = Point3F(this->mNodes.front()->mPosition);

       


		// iterate over all nodes and find the min & max
		for (U32 i = 0; i < this->mNodes.size(); ++i)
		{
			if (this->mNodes[i]->mPosition.x < min.x)
				min.x = this->mNodes[i]->mPosition.x;
			if (this->mNodes[i]->mPosition.y < min.y)
				min.y = this->mNodes[i]->mPosition.y;
			if (this->mNodes[i]->mPosition.z < min.z)
				min.z = this->mNodes[i]->mPosition.z;


			if (this->mNodes[i]->mPosition.x > max.x)
				max.x = this->mNodes[i]->mPosition.x;
			if (this->mNodes[i]->mPosition.y > max.y)
				max.y = this->mNodes[i]->mPosition.y;
			if (this->mNodes[i]->mPosition.z > max.z)
				max.z = this->mNodes[i]->mPosition.z;
		}

		//xxth small x,y extent by gridsize
		Point3F lExt = Point3F(this->mGridSize,this->mGridSize,0) / 2; 
		min-=lExt;
		max+=lExt;

		// set position as halfway point
		this->setPosition(min + ((max - min) / 2));
		
		// create a box to encompass the entire path
		this->mObjBox.minExtents.set(-(this->getPosition() - min));
		this->mObjBox.maxExtents.set(max - this->getPosition());

		// must reset world box & transform when changing object box
		this->resetWorldBox();
		this->setRenderTransform(mObjToWorld);
	}
}
//--------------------------------------------------------------------------------------------------------------------------------
iAIPathNode* iAIPathGrid::getClosestNode(const Point3F position)
{
	PROFILE_START(iAIPathGrid_getClosestNode);

	F32 closestVec = 1000.0f;
	F32 currentVec = 1000.0f;
	Point3F curNodePos;
	iAIPathNode* closestNode = 0;

	//XXTH 
	iAIPathFind* lPathFinder = iAIPathFind::getInstance();

	// iterate over all nodes
	for (U32 i = 0; i < this->mNodes.size(); ++i)
	{
		//XXTH is bad node ? 
		if (this->mNodes[i]->mBadNode)
			continue;

		// calculate the vector length
		curNodePos = this->mNodes[i]->mPosition;
		
		currentVec = (position - curNodePos).len();

		// check if its closest one found so far
		if (currentVec < closestVec)
		{

			//1.97.3 MUST ALSO CHECK TERRAIN :(( added TerrainObjectType
			bool lValid = lPathFinder->smoothPathConnectionValid(position,curNodePos,(TerrainObjectType  | StaticShapeObjectType | TerrainLikeObjectType | InteriorLikeObjectType ));
            //XXTH Check visibility
//mhhh && lPathFinder->smoothPathConnectionValid(position,curNodePos,(InteriorObjectType | StaticShapeObjectType |  StaticTSObjectType ))
 		    if (lValid || (currentVec >  this->mGridSize * 2 && lValid)  ) 
			//if (currentVec >  this->mGridSize * 2 || lPathFinder->smoothPathConnectionValid(position,curNodePos,(InteriorObjectType | StaticShapeObjectType |  StaticTSObjectType )))
			{
				// save as closest so far
				closestVec = currentVec;
				closestNode = this->mNodes[i];
			}
		}
	}
    PROFILE_END();
	return closestNode;
}
//--------------------------------------------------------------------------------------------------------------------------------
U32 iAIPathGrid::updateMoveModifierBox(Box3F Box, U8 newModifier) {

	U32 result = 0;

	//enhance box
	Box.minExtents -= IAIPATHGLOBAL_NODE_CLEARANCE;
	Box.maxExtents += IAIPATHGLOBAL_NODE_CLEARANCE;

	// iterate over all nodes
	for (U32 i = 0; i < this->mNodes.size(); ++i)
	{
		// check if its closest one found so far
		if (
			    this->mNodes[i]->mPosition.x > Box.minExtents.x
			 && this->mNodes[i]->mPosition.x < Box.maxExtents.x
			 && this->mNodes[i]->mPosition.y > Box.minExtents.y
			 && this->mNodes[i]->mPosition.y < Box.maxExtents.y
			)
		{
			// set new modifier
			this->mNodes[i]->mMoveModifier = newModifier;
			result++;
		}
	}
	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------
U32 iAIPathGrid::restoreMoveModifierBox(Box3F Box) {

	U32 result = 0;

	//enhance box
	Box.minExtents -= IAIPATHGLOBAL_NODE_CLEARANCE;
	Box.maxExtents += IAIPATHGLOBAL_NODE_CLEARANCE;

	// iterate over all nodes
	for (U32 i = 0; i < this->mNodes.size(); ++i)
	{
		// check if its closest one found so far
		if (
			    this->mNodes[i]->mPosition.x > Box.minExtents.x
			 && this->mNodes[i]->mPosition.x < Box.maxExtents.x
			 && this->mNodes[i]->mPosition.y > Box.minExtents.y
			 && this->mNodes[i]->mPosition.y < Box.maxExtents.y
			)
		{
			// set new modifier
			this->mNodes[i]->mMoveModifier = this->mNodes[i]->mSaveMoveModifier;
			result++;
		}
	}
	return result;
}
//--------------------------------------------------------------------------------------------------------------------------------
//XXTH Link 2 grids:
bool iAIPathGrid::linkGrid(Point3F lPos, iAIPathGrid * lNeighbourGrid)
{
	if (!lNeighbourGrid)
	{
		Con::errorf("INVALID GRID IN iAIPathGrid::linkGrid");
		return false;
	}

	iAIPathNode* node1 = this->getClosestNode(lPos);
	iAIPathNode* node2 = lNeighbourGrid->getClosestNode(lPos);
	if (node1 && node2)
	{
	   if (
		   node1->addNeighbour(node2)
		   &&
		   node2->addNeighbour(node1)
		   )
	   {
		   Con::printf("iAIPathGrid::linkGrid linked nodes (%f,%f,%f) => (%f,%f,%f)",
			   node1->mPosition.x,node1->mPosition.y,node1->mPosition.z,
			   node2->mPosition.x,node2->mPosition.y,node2->mPosition.z);
		   return true;
	   } else {
		   Con::errorf("iAIPathGrid::linkGrid linked nodes FAILED! (%f,%f,%f) => (%f,%f,%f)",
			   node1->mPosition.x,node1->mPosition.y,node1->mPosition.z,
			   node2->mPosition.x,node2->mPosition.y,node2->mPosition.z);

	   }
	} else {
		Con::errorf("iAIPathGrid::linkGrid linked nodes FAILED!");
		if (node1)
			Con::errorf("iAIPathGrid::linkGrid but node1 found : (%f,%f,%f)",
			   node1->mPosition.x,node1->mPosition.y,node1->mPosition.z);
		if (node2)
			Con::errorf("iAIPathGrid::linkGrid but node2 found : (%f,%f,%f)",

			   node2->mPosition.x,node2->mPosition.y,node2->mPosition.z);

	}

	return false;
}
//--------------------------------------------------------------------------------------------------------------------------------
//XXTH Link 2 Nodes must be exact positions!! :
// 2.28 no clearance check! 
bool iAIPathGrid::linkNodes(Point3F lPos1, Point3F lPos2)
{

   iAIPathNode* node1 = this->getClosestNode(lPos1);
   iAIPathNode* node2 = this->getClosestNode(lPos2);
   if (node1 && node2)
   {
      if (
         node1->addNeighbourNoCheck(node2)
         &&
         node2->addNeighbourNoCheck(node1)
         )
      {
         Con::printf("iAIPathGrid::linkNodes linked nodes (%f,%f,%f) => (%f,%f,%f)",
            node1->mPosition.x, node1->mPosition.y, node1->mPosition.z,
            node2->mPosition.x, node2->mPosition.y, node2->mPosition.z);
         return true;
      }
      else {
         Con::errorf("iAIPathGrid::linkNodes linked nodes FAILED! (%f,%f,%f) => (%f,%f,%f)",
            node1->mPosition.x, node1->mPosition.y, node1->mPosition.z,
            node2->mPosition.x, node2->mPosition.y, node2->mPosition.z);

      }
   }
   else {
      Con::errorf("iAIPathGrid::linkGrid linkNodes nodes FAILED!");
      if (node1)
         Con::errorf("iAIPathGrid::linkNodes but node1 found : (%f,%f,%f)",
            node1->mPosition.x, node1->mPosition.y, node1->mPosition.z);
      if (node2)
         Con::errorf("iAIPathGrid::linkNodes but node2 found : (%f,%f,%f)",

            node2->mPosition.x, node2->mPosition.y, node2->mPosition.z);

   }

   return false;
}
//--------------------
DefineEngineMethod(iAIPathGrid, linkNodes, bool, (Point3F lPos1, Point3F lPos2), , "Link nodes manually without clearance check")
{
   return object->linkNodes(lPos1, lPos2);

}
//--------------------------------------------------------------------------------------------------------------------------------
//2.28 bool unLinkNodes(Point3F lPos1, Point3F lPos2);
//--------------------------------------------------------------------------------------------------------------------------------
bool iAIPathGrid::unLinkNodes(Point3F lPos1, Point3F lPos2)
{

   iAIPathNode* node1 = this->getClosestNode(lPos1);
   iAIPathNode* node2 = this->getClosestNode(lPos2);
   if (node1 && node2)
   {
      if (
         node1->removeNeighbour(node2)
         &&
         node2->removeNeighbour(node1)
         )
      {
         Con::printf("iAIPathGrid::unLinkNodes (%f,%f,%f) => (%f,%f,%f)",
            node1->mPosition.x, node1->mPosition.y, node1->mPosition.z,
            node2->mPosition.x, node2->mPosition.y, node2->mPosition.z);
         return true;
      }
      else {
         Con::errorf("iAIPathGrid::unLinkNodes FAILED! (%f,%f,%f) => (%f,%f,%f)",
            node1->mPosition.x, node1->mPosition.y, node1->mPosition.z,
            node2->mPosition.x, node2->mPosition.y, node2->mPosition.z);

      }
   }
   else {
      Con::errorf("iAIPathGrid::unLinkNodes nodes FAILED!");
      if (node1)
         Con::errorf("iAIPathGrid::unLinkNodes but node1 found : (%f,%f,%f)",
            node1->mPosition.x, node1->mPosition.y, node1->mPosition.z);
      if (node2)
         Con::errorf("iAIPathGrid::unLinkNodes but node2 found : (%f,%f,%f)",

            node2->mPosition.x, node2->mPosition.y, node2->mPosition.z);

   }

   return false;
}
//--------------------------------------------------------------------------------------------------------------------------------
//2.30 remove all neightbours from node

bool iAIPathGrid::unlinkNode(Point3F lPos1)
{
   iAIPathNode* node1 = this->getClosestNode(lPos1);
   iAIPathNode* node2 = NULL;
   if (!node1)
      return false;
   while (node1->mNeighbours.size() > 0)
   {
      node2 = node1->mNeighbours[0];
      // removed check 
      node1->removeNeighbour(node2);
      node2->removeNeighbour(node1);
   }
   return true;
}

//----
//ConsoleMethod(iAIPathGrid, unlinkNode, bool, 3, 3, "bool iAIPathGrid.unlinkNode>(Point3F pos1) unlink all nodes from node near given point")
DefineEngineMethod(iAIPathGrid, unlinkNode, bool, (Point3F lPos1), , "unlink all nodes from node near given point")
{
   return object->unlinkNode(lPos1);
}

//--------------------
DefineEngineMethod(iAIPathGrid, unlinkNodes, bool, (Point3F lPos1, Point3F lPos2), , "unlink nodes manually")
{
   return object->unLinkNodes(lPos1, lPos2);
}

//----
//--------------------
DefineEngineMethod(iAIPathGrid, linkGrid, bool, (Point3F lPos, iAIPathGrid* lNeighbourGrid), , "link grid manually")
{
   if (lNeighbourGrid)
      return object->linkGrid(lPos, lNeighbourGrid);
   else
      Con::errorf("Invaild neighbourGrid!!");

   return false;
}

/*
DefineEngineStringlyVariadicMethod( iAIPathGrid, linkGrid, bool, 4, 4, "bool iAIPathGrid.linkGrid(Point3F pos, iAIPathGrid * neighbourGrid)")
{
  Point3F lPos;
  dSscanf(argv[2], "%f %f %f", &lPos.x, &lPos.y, &lPos.z);

  iAIPathGrid* lNeighbourGrid;
  if( !Sim::findObject( argv[3], lNeighbourGrid ) ) {
	  Con::errorf("Invaild neighbourGrid!!");
  }
  return object->linkGrid(lPos, lNeighbourGrid);

}
*/

//--------------------------------------------------------------------------------------------------------------------------------
//XXTH closest on grid:
DefineEngineMethod(iAIPathGrid, closestNode, const char*, (Point3F lPos), , "Get the position of the closest node at  position. Or empty string if not found")
{
   // get the closest node
   iAIPathNode* closestNode = object->getClosestNode(lPos);
   // ensure found a closest index
   if (closestNode)
   {
      char* returnBuffer = Con::getReturnBuffer(256);
      dSprintf(returnBuffer, 256, "%f %f %f", closestNode->mPosition.x, closestNode->mPosition.y, closestNode->mPosition.z);

      return returnBuffer;
   }
   Con::errorf("Immersive AI :: Seek :: PathMap - no node found near %f %f %f", lPos.x, lPos.y, lPos.z);
   return "";

}
/*
DefineEngineStringlyVariadicMethod( iAIPathGrid, closestNode, const char *, 3, 3,
			  "Point3F iAIPathGrid.closestNode(Point3F pos) - Get the closest node to the supplied position.")
{
	// ensure pos passed
	if (dStrlen(argv[2]) != 0) {

		// pass the args into a Point3F
		Point3F position;
		dSscanf(argv[2], "%f %f %f", &position.x, &position.y, &position.z);

		// get the closest node
		iAIPathNode* closestNode = object->getClosestNode(position);

		// ensure found a closest index
		if (closestNode)
		{
			char *returnBuffer = Con::getReturnBuffer(256);
			dSprintf(returnBuffer, 256, "%f %f %f", closestNode->mPosition.x, closestNode->mPosition.y, closestNode->mPosition.z);

			return returnBuffer;
		} else
		{
			Con::errorf("Immersive AI :: Seek :: PathMap - no node found near %f %f %f", position.x, position.y, position.z);
			return "";
		}
	} else
	{
		Con::errorf("Immersive AI :: Seek :: PathMap - no Point3F parsed to ClosestNode!");
		return "";
	}
}
*/
//--------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------
//called in iAIPathMap!! DefineEngineMethod(iAIPathGrid, initCustomGrid, bool, (), ,

DefineEngineMethod(iAIPathGrid, AddNodeCustomGrid, bool, (Point3F position), ,
   "add a node to a custom grid")
{
   return object->AddNodeCustomGrid(position);
}
   
DefineEngineMethod(iAIPathGrid, LinkNodesCustomGrid, bool, (Point3F pos1, Point3F pos2), ,
   "add a node to a custom grid")
{
   return object->LinkNodesInCustomGrid(pos1, pos2);
}


//called in iAIPathMap!! DefineEngineMethod(iAIPathGrid, compileCustomGrid, bool, (), ,
