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

#include "scene/sceneManager.h"

#include "iAIPath.h"
#include "iAIPathMap.h"
#include "iAIPathFind.h"
#include "iAIPathGlobal.h"

IMPLEMENT_CO_NETOBJECT_V1(iAIPath);

iAIPath::iAIPath()
{
//	this->mTypeMask |= iAIPathObjectType;
	this->setPosition(Point3F(0,0,0));

	this->mTraversing = false;
	this->mShow = false;
	this->mRenderSpline = true;
	this->mLastNode = 0;

	// default path colour is orangey
	this->mPathColour = ColorI(157, 93, 31, 255);

	// default node colour is redish
	this->mPathNodeColour = ColorI(157, 31, 60, 255);
}

bool iAIPath::createPath(iAIPathMap* pathMap, Point3F start, Point3F end, const bool smoothPath)
{

	iAIPathNode* startNode = pathMap->getClosestNode(start);
	iAIPathNode* endNode   = pathMap->getClosestNode(end);


	if (!startNode || !endNode) 
			return false;

	// check if start and end nodes in the same position
	if (startNode->mPosition == endNode->mPosition)
	{
		// just push on the end node
		this->mPathNodes.push_back(endNode);
		return true;
	}

	// get an instance of the singleton pathfinder
	iAIPathFind* pathFinder = iAIPathFind::getInstance();
	this->mTraversing = false;
   

	// find the path; if unable to find a path, loop until IAIPATHGLOBAL_PATH_RETRY_COUNT is reached
	U32 retryCount = 0;
	while ((!(pathFinder->generatePath(startNode, endNode, this->mPathNodes, smoothPath))) && (retryCount <= IAIPATHGLOBAL_PATH_RETRY_COUNT))
		++retryCount;

	// check that a path was found
	if (this->mPathNodes.size() > 0)
	{
		// update the world box, so that path will render
		this->updateWorldBox();
		return true;
	} else
	{
		Con::errorf("Immersive AI :: Seek :: Unable to find a valid path from %f, %f, %f to %f, %f, %f", start.x, start.y, start.z, end.x, end.y, end.z);
		return false;
	}
}

Point3F iAIPath::getNextPosition()
{
	// only pop previous node if we are already traversing!
	if (this->mTraversing)
	{
		// check that this isnt the last node
		if (this->mPathNodes.size() > 1)
		{
			// update the last node
			this->mLastNode = this->mPathNodes.front();

			// remove the previous node from the list
			this->mPathNodes.pop_front();
		}
	} else
	{
		this->mTraversing = true;
	}

	// return the node, if any left
	if (this->mPathNodes.size() > 0)
	{
		iAIPathNode* returnNode = this->mPathNodes.front();
		
		// if we are going to return the last node, clear the list
		if (this->mPathNodes.size() == 1)
		{
			this->mPathNodes.clear();
			this->mLastNode = 0;
		}
		mCurMoveModifier = returnNode->mMoveModifier;
		return returnNode->mPosition;
	} else
	{
		return IAIPATHGLOBAL_INVALID_POSITION;
	}
}
// XXTH ---------------------------------------->>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Point3F iAIPath::getNodePosition(S32 idx)
{
   
   if (idx < 0 || idx >= this->mPathNodes.size())
      return IAIPATHGLOBAL_INVALID_POSITION;

   return this->mPathNodes[idx]->getPosition();
}
// <<<<<<<<<<<<<<<<<<<<<<<<< --------------------------------XXTH -----------
bool iAIPath::hasNextNode()
{
	return (this->mPathNodes.size() > 0);
}

U32 iAIPath::nodeCount()
{
	return this->mPathNodes.size();
}

void iAIPath::updateWorldBox()
{
	// only need a both if there is a path ;)
	if (this->mPathNodes.size() > 0)
	{
		Point3F min = Point3F(this->mPathNodes.front()->mPosition);
		Point3F max = Point3F(this->mPathNodes.front()->mPosition);

		// iterate over all nodes and find the min & max
		for (U32 i = 0; i < this->mPathNodes.size(); ++i)
		{
			if (this->mPathNodes[i]->mPosition.x < min.x)
				min.x = this->mPathNodes[i]->mPosition.x;
			if (this->mPathNodes[i]->mPosition.y < min.y)
				min.y = this->mPathNodes[i]->mPosition.y;
			if (this->mPathNodes[i]->mPosition.z < min.z)
				min.z = this->mPathNodes[i]->mPosition.z;


			if (this->mPathNodes[i]->mPosition.x > max.x)
				max.x = this->mPathNodes[i]->mPosition.x;
			if (this->mPathNodes[i]->mPosition.y > max.y)
				max.y = this->mPathNodes[i]->mPosition.y;
			if (this->mPathNodes[i]->mPosition.z > max.z)
				max.z = this->mPathNodes[i]->mPosition.z;
		}

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

bool iAIPath::onAdd()
{
	// call Parent, ensure worked
	if(!Parent::onAdd())
	   return false;

	// create object box
	this->updateWorldBox();

	// add to scene
    //is done in addObjectToScene gClientContainer.addObject(this);
    gClientSceneGraph->addObjectToScene(this);
	
	return true;
}

void iAIPath::onRemove()
{
	// remove from scene
	removeFromScene();
	Parent::onRemove();
}

void iAIPath::prepRenderImage(SceneRenderState* state)
{

/* FIXME T3D
	// render if there is a path to render and want to show it
	if ((this->mShow) && (this->mPathNodes.size() > 0))
	{
		// return if last state
		if (this->isLastState(state, stateKey)) return false;
		
		// set last state
		this->setLastState(state, stateKey);

		// see if object rendered
		if (state->isObjectRendered(this))
		{
			// get a SceneRenderImage to show on	
			SceneRenderImage* image = new SceneRenderImage;
			image->obj = this;

			// insert into scene image
			state->insertRenderImage(image);
		}
	}

	return false;
*/
}

void iAIPath::renderObject(ObjectRenderInst* ri, SceneRenderState* state, BaseMatInstance* mi)
{

/* FIXME T3D
	// save matrix to restore canonical state
	glPushMatrix();
	
	// enable blend
	glEnable(GL_BLEND);

	// always rendering lines
	glBegin(GL_LINES);

	// see if we want a linear or spline path
	if (this->mRenderSpline)
	{
		CameraSpline pathSpline;

		// add the lastNode to the spline
		if (this->mLastNode)
		{
			pathSpline.push_back(new CameraSpline::Knot(
					this->mLastNode->mPosition,
					QuatF(0, 0, 0, 0),
					1.0f,
					CameraSpline::Knot::NORMAL,
					CameraSpline::Knot::SPLINE));
		}

		// iterate over all the nodes: add to spline and draw the stick
		for (U32 j = 0; j < this->mPathNodes.size(); j++)
		{
			if (this->mPathNodes[j])
			{
				// add a new knot for each path node
				pathSpline.push_back(new CameraSpline::Knot(
						this->mPathNodes[j]->mPosition,
						QuatF(0, 0, 0, 0),
						1.0f,
						CameraSpline::Knot::NORMAL,
						CameraSpline::Knot::SPLINE));
			}

			// draw the path node
			// mMoveModifier
//			glColor4ub(this->mPathNodeColour.red, this->mPathNodeColour.green, this->mPathNodeColour.blue, this->mPathNodeColour.alpha);
			//XXTH render stick in this->mPathNodes[j]->mMoveModifier (weight) color
			glColor4ub(this->mPathNodes[j]->mMoveModifier, this->mPathNodes[j]->mMoveModifier, this->mPathNodes[j]->mMoveModifier, this->mPathNodeColour.alpha);
			glVertex3fv(this->mPathNodes[j]->mPosition + IAIPATHGLOBAL_PATH_RENDER_CLEARANCE);
			glVertex3fv(this->mPathNodes[j]->mPosition + IAIPATHGLOBAL_PATH_RENDER_CLEARANCE + IAIPATHGLOBAL_PATH_RENDER_NODE_HEIGHT);
		}

		F32 iter = 0.0f;
		Point3F lastPoint = Point3F(0,0,0);

		// draw the entire spline
		while (iter < (pathSpline.size()-1))
		{
			// get set k to the current knot value
			CameraSpline::Knot k;
			pathSpline.value(iter, &k);

			// advance the spline iter
			iter = pathSpline.advanceDist(iter, 2.0f);

			// get the knot point information
			Point3F newPoint;
			k.mRotation.mulP(Point3F(0,0,0), &newPoint);
			newPoint += k.mPosition;

			// check if there is a last point info stored
			if (lastPoint == Point3F(0,0,0))
			{
				lastPoint = newPoint;
			} else
			{
				glColor4ub(this->mPathColour.red, this->mPathColour.green, this->mPathColour.blue, this->mPathColour.alpha);

				// draw a line between the new point and the last point
				glVertex3fv(newPoint + IAIPATHGLOBAL_PATH_RENDER_CLEARANCE);
				glVertex3fv(lastPoint + IAIPATHGLOBAL_PATH_RENDER_CLEARANCE);

				// set the new last point to the new point
				lastPoint = newPoint;
			}
		}

	} else
	{
		// draw a path between the last node and the current start node
		if (this->mLastNode)
		{
			glColor4ub(this->mPathColour.red, this->mPathColour.green, this->mPathColour.blue, this->mPathColour.alpha);
			glVertex3fv(this->mLastNode->mPosition + IAIPATHGLOBAL_PATH_RENDER_CLEARANCE);
			glVertex3fv(this->mPathNodes[0]->mPosition+ IAIPATHGLOBAL_PATH_RENDER_CLEARANCE);
		}

		for (int j = 1; j < this->mPathNodes.size(); j++)
		{
			if (this->mPathNodes[j])
			{
				// draw the path line
				glColor4ub(this->mPathColour.red, this->mPathColour.green, this->mPathColour.blue, this->mPathColour.alpha);
				glVertex3fv(this->mPathNodes[j-1]->mPosition + IAIPATHGLOBAL_PATH_RENDER_CLEARANCE);
				glVertex3fv(this->mPathNodes[j]->mPosition + IAIPATHGLOBAL_PATH_RENDER_CLEARANCE);

 				// draw the node stick
   			    //XXTH render stick in this->mPathNodes[j]->mMoveModifier (weight) color
			    glColor4ub(this->mPathNodes[j]->mMoveModifier, this->mPathNodes[j]->mMoveModifier, this->mPathNodes[j]->mMoveModifier, this->mPathNodeColour.alpha);
				//glColor4ub(this->mPathNodeColour.red, this->mPathNodeColour.green, this->mPathNodeColour.blue, this->mPathNodeColour.alpha);
				glVertex3fv(this->mPathNodes[j]->mPosition + IAIPATHGLOBAL_PATH_RENDER_CLEARANCE);
				glVertex3fv(this->mPathNodes[j]->mPosition + IAIPATHGLOBAL_PATH_RENDER_CLEARANCE + IAIPATHGLOBAL_PATH_RENDER_NODE_HEIGHT);
			}
		}
	}

	// end of line drawing
	glEnd();

	// disable the blend
	glDisable(GL_BLEND);

	// restore canonical maxtrix state
	glPopMatrix();

	// ensure canonical state is restored
	AssertFatal(dglIsInCanonicalState(), "Error, GL not in canonical state on exit");
   */
}


void iAIPath::initPersistFields()
{
	Parent::initPersistFields();
 
	addGroup("Misc");
		addField("showPath", TypeBool, Offset(mShow, iAIPath), "Display the path on rendering.");
		addField("renderSpline", TypeBool, Offset(mRenderSpline, iAIPath), "Render the path as a spline. If set to false, will render as linear.");
		addField("pathColour", TypeColorI, Offset(mPathColour, iAIPath), "The colour of the rendered path.");
		addField("pathNodeColour", TypeColorI, Offset(mPathNodeColour, iAIPath), "The colour of the nodes on the rendered path.");
	endGroup("Misc");
}

ConsoleMethodGroupBegin(iAIPath, ScriptFunctions, "iAIPath Script Functions");

DefineEngineStringlyVariadicMethod( iAIPath, createPath, bool, 4, 5,
			  "bool iAIPath.createPath(Point3F start, Point3F goal, bool smoothPath = true) - Create a path between the two points.")
{
	// ensure pos passed
	if ((dStrlen(argv[2]) != 0) && (dStrlen(argv[3]) != 0))
	{
		// parse arguments into points and create the path
		Point3F start;
		Point3F goal;
		dSscanf(argv[2], "%f %f %f", &start.x, &start.y, &start.z);
		dSscanf(argv[3], "%f %f %f", &goal.x, &goal.y, &goal.z);

		// get the path map variable
		iAIPathMap* pathMap = 0;
		if (Sim::findObject(dAtoi(Con::getVariable("$iAIPathMap")), pathMap))
		{
			// see of the smoothPath parameter is set
			if (dStrlen(argv[4]) != 0)
				return (object->createPath(pathMap, start, goal, dAtob(argv[4])));
			else
				return (object->createPath(pathMap, start, goal));
		} else
		{
			Con::errorf("Immersive AI :: Seek :: Path - unable to find the iAIPathMap");
			return false;
		}
	} else
	{
		Con::errorf("Immersive AI :: Seek :: Path- not enough nodes passed to CreatePath!");
		return false;
	}
}

DefineEngineStringlyVariadicMethod( iAIPath, nextPosition, const char*, 2, 2,
			  "Point3F iAIPath.nextPosition() - Get the next position on the path. Current position will be deleted!!")
{
	char *returnBuffer = Con::getReturnBuffer(256);

	Point3F nextPosition = object->getNextPosition();
	if (nextPosition != IAIPATHGLOBAL_INVALID_POSITION)
	{
		dSprintf(returnBuffer, 256, "%f %f %f", nextPosition.x, nextPosition.y, nextPosition.z);
	} else
	{
		dSprintf(returnBuffer, 256, "");
	}

	return returnBuffer;
}

DefineEngineMethod(iAIPath, nodePosition, Point3F, (S32 idx), ,
   "get the position of a node with the index idx without removeing the node like nextPosition!!")
{
   return object->getNodePosition(idx);
}



// DefineEngineStringlyVariadicMethod( iAIPath, CurMoveModifier, S32, 2, 2,
DefineEngineMethod(iAIPath, CurMoveModifier,S32, (), ,
			  "U8 mCurMoveModifier - moveModifier from last nextPosition() call. 0=best walkable, 255 = unwalkable")
{
	return object->mCurMoveModifier;
}



// DefineEngineStringlyVariadicMethod( iAIPath, hasNextNode, bool, 2, 2,
DefineEngineMethod(iAIPath, hasNextNode, bool, (), ,
			  "bool iAIPath.hasNextNode() - Returns if the path has another node.")
{
	return (object->hasNextNode());
}

//DefineEngineStringlyVariadicMethod( iAIPath, nodeCount, S32, 2, 2,
DefineEngineMethod(iAIPath, nodeCount, S32, (), ,
			  "U32 iAIPath.nodeCount() - Returns number of nodes left in the path.")
{
	return (object->nodeCount());
}


ConsoleMethodGroupEnd(iAIPath, ScriptFunctions);
