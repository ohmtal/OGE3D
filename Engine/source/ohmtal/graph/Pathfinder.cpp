#include "Pathfinder.h"
#include "HandyGraphFunctions.h"
#include "AStarHeuristicPolicies.h"
#include "misc/Stream_Utility_Functions.h"
//REMOVED!! #include "time/PrecisionTimer.h"


#include "gfx/gfxDevice.h"
#include "gfx/gfxDrawUtil.h"

#include "ohmtal/2d/tom2DCtrl.h"
//#include "constants.h"



#include <iostream>
using namespace std;


IMPLEMENT_CONOBJECT(Pathfinder);

//----------------------- CreateGraph ------------------------------------
//
//------------------------------------------------------------------------
void Pathfinder::setArea(RectI lArea)
{
    mArea = lArea;
}

void Pathfinder::CreateGraph(int CellsX,int CellsY)
//void Pathfinder::CreateGraph(RectI lArea, F32 lSquareSize)

{

    if (mArea.len_x() == 0 || mArea.len_y() == 0)
    {
        Con::errorf("Bad Pathfinder Area! Cant create graph!");
        return;
    }


  //initialize the terrain vector with normal terrain

  m_iCellsX = CellsX;
  m_iCellsY = CellsY;

  mSquareSizeX  = mCeil(mArea.len_x() / m_iCellsX);
  mSquareSizeY = mCeil(mArea.len_y() / m_iCellsY);

  //FIXME redundant variables!!! 
  m_dCellWidth  = mSquareSizeX;
  m_dCellHeight = mSquareSizeY;

  m_TerrainType.assign(m_iCellsX * m_iCellsY, 0);


  //delete any old graph
  delete m_pGraph;

  //create the graph
  m_pGraph = new NavGraph(false);//not a digraph


  //void GraphHelper_CreateGrid(graph_type & graph, int cySize, int cxSize, int NumCellsY, int NumCellsX)
  GraphHelper_CreateGrid(*m_pGraph, mArea.extent.x, mArea.extent.y, m_iCellsX, m_iCellsY);

  //initialize source and target indexes to mid top and bottom of grid 
  PointToIndex(VectorToPOINTS(Vector2D(mArea.extent.x /2, m_dCellHeight*2)), m_iTargetCell);
  PointToIndex(VectorToPOINTS(Vector2D(mArea.extent.x /2, mArea.extent.x -m_dCellHeight*2)), m_iSourceCell);

  m_Path.clear();
  m_SubTree.clear();

  m_CurrentAlgorithm = search_astar; //XXTH non;
  //FIXME   m_dTimeTaken = 0;
}

//--------------------- PointToIndex -------------------------------------
//
//  converts a POINTS into an index into the graph
//------------------------------------------------------------------------
bool Pathfinder::PointToIndex(Point2F p, int& NodeIndex)
{
  //convert p to an index into the graph
  int x = (int)((double)(p.x)/m_dCellWidth);  
  int y = (int)((double)(p.y)/m_dCellHeight); 
  
  //make sure the values are legal
  if ( (x>m_iCellsX) || (y>m_iCellsY) )
  {
    NodeIndex = -1;

    return false;
  }

  NodeIndex = y*m_iCellsX+x;

  return true;
}

//----------------- GetTerrainCost ---------------------------------------
//
//  returns the cost of the terrain represented by the current brush type
//------------------------------------------------------------------------
/*
double Pathfinder::GetTerrainCost(const brush_type brush)
{
  const double cost_normal = 1.0;
  const double cost_water  = 2.0;
  const double cost_mud    = 1.5;

  switch (brush)
  {
    case normal: return cost_normal;
    case water:  return cost_water;
    case mud:    return cost_mud;
    default:     return GraphUtils::MaxDouble;
  };
}
*/
  
//----------------------- PaintTerrain -----------------------------------
//
//  this either changes the terrain at position p to whatever the current
//  terrain brush is set to, or it adjusts the source/target cell
//------------------------------------------------------------------------
/*
void Pathfinder::PaintTerrain(POINTS p)
{


 FIXME 
  //convert p to an index into the graph
  int x = (int)((double)(p.x)/m_dCellWidth);
  int y = (int)((double)(p.y)/m_dCellHeight);

  //make sure the values are legal
  if ( (x>m_iCellsX) || (y>(m_iCellsY-1)) ) return;

  //reset path and tree records
  m_SubTree.clear();
  m_Path.clear();

  //if the current terrain brush is set to either source or target we
  //should change the appropriate node
  if ( (m_CurrentTerrainBrush == source) || (m_CurrentTerrainBrush == target) )
  {
    switch (m_CurrentTerrainBrush)
    {
    case source:

      m_iSourceCell = y*m_iCellsX+x; break;

    case target:

      m_iTargetCell = y*m_iCellsX+x; break;
      
    }//end switch
  }

  //otherwise, change the terrain at the current mouse position
  else
  {
    UpdateGraphFromBrush(m_CurrentTerrainBrush, y*m_iCellsX+x);
  }

  //update any currently selected algorithm
  UpdateAlgorithm();
  
}
*/
//--------------------------- UpdateGraphFromBrush ----------------------------
//
//  given a brush and a node index, this method updates the graph appropriately
//  (by removing/adding nodes or changing the costs of the node's edges)
//-----------------------------------------------------------------------------
/*
void Pathfinder::UpdateGraphFromBrush(int brush, int CellIndex)
{
  //set the terrain type in the terrain index
  m_TerrainType[CellIndex] = brush;

  //if current brush is an obstacle then this node must be removed
  //from the graph
  if (brush == 1)
  {
    m_pGraph->RemoveNode(CellIndex);
  }

  else
  {
    //make the node active again if it is currently inactive
    if (!m_pGraph->isNodePresent(CellIndex))
    {
      int y = CellIndex / m_iCellsY;
      int x = CellIndex - (y*m_iCellsY);

      m_pGraph->AddNode(NavGraph::NodeType(CellIndex, Vector2D(x*m_dCellWidth + m_dCellWidth/2.0,
                                                               y*m_dCellHeight+m_dCellHeight/2.0)));

      GraphHelper_AddAllNeighboursToGridNode(*m_pGraph, y, x, m_iCellsX, m_iCellsY);
    }

    //set the edge costs in the graph
    WeightNavGraphNodeEdges(*m_pGraph, CellIndex, GetTerrainCost((brush_type)brush));                            
  }
}
*/
void Pathfinder::UpdateGraphWeight(int weight, int CellIndex)
{
    //set the terrain type in the terrain index
    m_TerrainType[CellIndex] = weight;

    //if current brush is an obstacle then this node must be removed
    //from the graph

    //0 is bad for cost calculation so fake a 1!
    if (weight == 0) {
        weight = 1;
    } 
    else if (weight == 255)
    {
        m_pGraph->RemoveNode(CellIndex);
        return;
    }
    


    {
        //make the node active again if it is currently inactive
        if (!m_pGraph->isNodePresent(CellIndex))
        {
            int y = CellIndex / m_iCellsY;
            int x = CellIndex - (y * m_iCellsY);

            m_pGraph->AddNode(NavGraph::NodeType(CellIndex, Vector2D(x * m_dCellWidth + m_dCellWidth / 2.0,
                y * m_dCellHeight + m_dCellHeight / 2.0)));

            GraphHelper_AddAllNeighboursToGridNode(*m_pGraph, y, x, m_iCellsX, m_iCellsY);
        }

        //set the edge costs in the graph
        WeightNavGraphNodeEdges(*m_pGraph, CellIndex, weight);
    }
}

//--------------------------- UpdateAlgorithm ---------------------------------
bool Pathfinder::UpdateAlgorithm()
{
  //update any current algorithm
  switch(m_CurrentAlgorithm)
  {
  case non:

    break;

  case search_dfs:

    CreatePathDFS(); break;

  case search_bfs:
    
    CreatePathBFS(); break;

  case search_dijkstra:

    CreatePathDijkstra(); break;

  case search_astar:
    
    CreatePathAStar(); break;

  default: break;
  }

  return m_Path.size() > 0;
}

//------------------------- CreatePathDFS --------------------------------
//
//  uses DFS to find a path between the start and target cells.
//  Stores the path as a series of node indexes in m_Path.
//------------------------------------------------------------------------
void Pathfinder::CreatePathDFS()
{
  //set current algorithm
  m_CurrentAlgorithm = search_dfs;

  //clear any existing path
  m_Path.clear();
  m_SubTree.clear();

  //create and start a timer
  //FIXME PrecisionTimer timer; timer.Start();
  


  //do the search
  Graph_SearchDFS<NavGraph> DFS(*m_pGraph, m_iSourceCell, m_iTargetCell);

  //record the time taken  
  //FIXME m_dTimeTaken = timer.TimeElapsed();

  //now grab the path (if one has been found)
  if (DFS.Found())
  {
    m_Path = DFS.GetPathToTarget();
  }

  m_SubTree = DFS.GetSearchTree();

  m_dCostToTarget = 0.0;
}


//------------------------- CreatePathBFS --------------------------------
//
//  uses BFS to find a path between the start and target cells.
//  Stores the path as a series of node indexes in m_Path.
//------------------------------------------------------------------------
void Pathfinder::CreatePathBFS()
{
  //set current algorithm
  m_CurrentAlgorithm = search_bfs;

  //clear any existing path
  m_Path.clear();
  m_SubTree.clear();

  //create and start a timer
  //FIXME PrecisionTimer timer; timer.Start();

  //do the search
  Graph_SearchBFS<NavGraph> BFS(*m_pGraph, m_iSourceCell, m_iTargetCell);

    //record the time taken  
  //FIXME m_dTimeTaken = timer.TimeElapsed();

  //now grab the path (if one has been found)
  if (BFS.Found())
  {
    m_Path = BFS.GetPathToTarget();
  }

  m_SubTree = BFS.GetSearchTree();

  m_dCostToTarget = 0.0;
}

//-------------------------- CreatePathDijkstra --------------------------
//
//  creates a path from m_iSourceCell to m_iTargetCell using Dijkstra's algorithm
//------------------------------------------------------------------------
void Pathfinder::CreatePathDijkstra()
{
  //set current algorithm
  m_CurrentAlgorithm = search_dijkstra;

  //create and start a timer
  //FIXME PrecisionTimer timer; timer.Start();
    
  Graph_SearchDijkstra<NavGraph> djk(*m_pGraph, m_iSourceCell, m_iTargetCell);

  //record the time taken  
  //FIXME m_dTimeTaken = timer.TimeElapsed();

  m_Path = djk.GetPathToTarget();

  m_SubTree = djk.GetSPT();

  m_dCostToTarget = djk.GetCostToTarget();
}

//--------------------------- CreatePathAStar ---------------------------
//------------------------------------------------------------------------
void Pathfinder::CreatePathAStar()
{
  //set current algorithm
  m_CurrentAlgorithm = search_astar;
      
  //create and start a timer
  //FIXME PrecisionTimer timer; timer.Start();
  
  //create a couple of typedefs so the code will sit comfortably on the page   
  typedef Graph_SearchAStar<NavGraph, Heuristic_Euclid> AStarSearch;

  //create an instance of the A* search using the Euclidean heuristic
  AStarSearch AStar(*m_pGraph, m_iSourceCell, m_iTargetCell);
  

  //record the time taken  
  //FIXME m_dTimeTaken = timer.TimeElapsed();

  m_Path = AStar.GetPathToTarget();

  m_SubTree = AStar.GetSPT();

  m_dCostToTarget = AStar.GetCostToTarget();

}

//---------------------------Load n save methods ------------------------------
//-----------------------------------------------------------------------------

void Pathfinder::Save( char* FileName)
{
  ofstream save(FileName);
  assert (save && "Pathfinder::Save< bad file >");

  //save the size of the grid
  save << m_iCellsX << endl;
  save << m_iCellsY << endl;

  //save the terrain
  for (unsigned int t=0; t<m_TerrainType.size(); ++t)
  {
      save << m_TerrainType[t] << endl;
  }

//FIXME save load point2Point stuff => m_PathCosts
      

}

//-------------------------------- Load ---------------------------------------
//-----------------------------------------------------------------------------
void Pathfinder::Load( char* FileName)
{
  ifstream load(FileName);
  assert (load && "Pathfinder::Save< bad file >");

  //load the size of the grid
  load >> m_iCellsX;
  load >> m_iCellsY;

  //create a graph of the correct size
  CreateGraph(m_iCellsY, m_iCellsX);

  int terrain;

  //save the terrain
  for (int t=0; t<m_iCellsX*m_iCellsY; ++t)
  {
    load >> terrain;
    
      m_TerrainType[t] = terrain;
      UpdateGraphWeight(terrain, t);
      
  }

//FIXME save load point2Point stuff => m_PathCosts
}

//------------------------ GetNameOfCurrentSearchAlgorithm --------------------
//-----------------------------------------------------------------------------
std::string Pathfinder::GetNameOfCurrentSearchAlgorithm()const
{
  switch(m_CurrentAlgorithm)
  {
  case non: return "";
  case search_astar: return "A Star";
  case search_bfs: return "Breadth First";
  case search_dfs: return "Depth First";
  case search_dijkstra: return "Dijkstras";
  default: return "UNKNOWN!";
  }
}

//---------------------------- Render ------------------------------------
//
// 
//------------------------------------------------------------------------


void Pathfinder::Render(bool renderCurrentPath, bool renderInfoText)
{

    if (!mScreen)
    {
        return;
    }


    ColorI lColor = ColorI(255,255,255,255); 


    // draw as batch
    GFXVertexBufferHandle<GFXVertexPCT> verts(GFX, m_pGraph->NumNodes() * 6, GFXBufferTypeVolatile);
    verts.lock();
    S32 idx = 0;

    S32 lTrans = 64;
    //render all the cells
    for (int nd = 0; nd < m_pGraph->NumNodes(); ++nd)
    {

        switch (m_TerrainType[nd])
        {
        case 0:
            lColor = ColorI(64, 64, 64, lTrans);
            break;

        case 192:
            lColor = ColorI(0, 0, 255, lTrans);
            break;

        case 255:
            lColor = ColorI(255, 255, 255, lTrans);
            break;

        default:
            lColor = ColorI(255, 0, 0, lTrans);
            break;

        }//end switch


        int left   = mArea.point.x + (int)(m_pGraph->GetNode(nd).Pos().x - m_dCellWidth / 2.0);
        int top    = mArea.point.y + (int)(m_pGraph->GetNode(nd).Pos().y - m_dCellHeight / 2.0);
        int right  = mArea.point.x + (int)(1 + m_pGraph->GetNode(nd).Pos().x + m_dCellWidth / 2.0);
        int bottom = mArea.point.y + (int)(1 + m_pGraph->GetNode(nd).Pos().y + m_dCellHeight / 2.0);
         //               v0---------v1
         //               | 
         //               | 
         //               | 
         //               v2


        //dirst
        verts[idx].point.set(left, top, 0.0f);
        verts[idx].color = lColor;
        verts[idx + 1].point.set(right,top, 0.0f);
        verts[idx + 1].color = lColor;
        verts[idx + 2].point.set(left,bottom, 0.0f);
        verts[idx + 2].color = lColor;
        //second

         //                          v4
         //                           |
         //                           |
         //                           |
         //               v3---------v5

        verts[idx + 3].point.set(left, bottom, 0.0f);
        verts[idx + 3].color = lColor;
        verts[idx + 4].point.set(right, top, 0.0f);
        verts[idx + 4].color = lColor;
        verts[idx + 5].point.set(right, bottom, 0.0f);
        verts[idx + 5].color = lColor;


        idx += 6;
        //(const Point2I & upperL, const Point2I & lowerR, const ColorI & color, bool doFill)
        //mScreen->drawPrimRect(mArea.point + Point2I(left, top), mArea.point + Point2I(right, bottom), lColor, true);

    } //for (int nd = 0; nd < m_pGraph->NumNodes(); ++nd)

    verts.unlock();

    GFX->setStateBlock(GFX->getDrawUtil()->get2DStateBlockRef());
    GFX->setVertexBuffer(verts);
    GFX->setupGenericShaders();
    GFX->drawPrimitive(GFXTriangleList, 0, m_pGraph->NumNodes() * 2);



//------------------------
//GraphHelper_DrawUsingGDI<NavGraph>(*m_pGraph, Cgdi::light_grey, false);  //false = don't draw node IDs
  //just return if the graph has no nodes
    lColor = ColorI(128, 128, 128, 255);
    Pathfinder_DrawGraph(mScreen, mArea.point , *m_pGraph, lColor, false);
//------------------------
/* FIXME
  //draw any tree retrieved from the algorithms
  gdi->RedPen();

  for (unsigned int e=0; e<m_SubTree.size(); ++e)
  {
    if (m_SubTree[e])
    {
      Vector2D from = m_pGraph->GetNode(m_SubTree[e]->From()).Pos();
      Vector2D to   = m_pGraph->GetNode(m_SubTree[e]->To()).Pos();

      gdi->Line(from, to);
    }
  }
*/
//------------------------
//draw the path (if any)

    if (renderCurrentPath && m_Path.size() > 0)
    {
        lColor = ColorI(0, 0, 255, 255);

        std::list<int>::iterator it = m_Path.begin();
        std::list<int>::iterator nxt = it; ++nxt;
        for (it; nxt != m_Path.end(); ++it, ++nxt)
        {
            Point2I lStart = mArea.point + Point2I((S32)m_pGraph->GetNode(*it).Pos().x, (S32)m_pGraph->GetNode(*it).Pos().y);
            Point2I lEnd   = mArea.point + Point2I((S32)m_pGraph->GetNode(*nxt).Pos().x, (S32)m_pGraph->GetNode(*nxt).Pos().y);
            mScreen->drawPrimLine(lStart, lEnd, lColor);
        }
    }

    if (renderInfoText)
    {
       GuiControlProfile* lProfile = (GuiControlProfile*)Sim::findObject("GuiConsole2WProfile");


       std::string s = "ID:" + std::to_string(getId()) + " Algorithm:" + GetNameOfCurrentSearchAlgorithm() +
          " Area:" + std::to_string(getArea().point.x) + " " + std::to_string(getArea().point.y) +
          " " + std::to_string(getArea().extent.x) + " " + std::to_string(getArea().extent.y) +
          " Nodes:" + std::to_string(getNodeCount());
       mScreen->writeText(s.c_str(), lProfile, mArea.point.x, mArea.point.y - 30);

       //display the total path cost if appropriate
       if (m_CurrentAlgorithm == search_astar || m_CurrentAlgorithm == search_dijkstra)
       {
          //gdi->TextAtPos(m_icxClient - 110, m_icyClient + 3, "Cost is " + ttos(m_dCostToTarget));
          string s = "Cost is " + ttos(m_dCostToTarget);
          mScreen->writeText(s.c_str(), lProfile, mArea.point.x, mArea.point.y - 20);
       }
    }


}


double Pathfinder::getNodeToNodeCosts(Point2F from, Point2F to)
{
   
    int startNodeIdx = getNodeIndex(from.x, from.y);
    int endNodeIdx = getNodeIndex(to.x, to.y);

    //FIXME validate values and check costs are filled! 

    return m_PathCosts[startNodeIdx][endNodeIdx];
}

//=============================================================================================
ConsoleMethod(Pathfinder, init, bool, 4, 4, "param: area: x y w h, F32 SquareSize")
{
    F32 lSquareSize;
    RectI area;

    dSscanf(argv[2], "%d %d %d %d", &area.point.x, &area.point.y, &area.extent.x, &area.extent.y);
    lSquareSize = dAtof(argv[3]);

    if (!area.isValidRect())
        return false;

    object->setArea(area);
    S32 lCellCount = mCeil(area.len_x() / lSquareSize); //only need one len!
    object->CreateGraph(lCellCount, lCellCount);

    return true;
}

ConsoleMethod(Pathfinder, setScreen, bool, 3, 3, "screenObj")
{

    tom2DCtrl* screen = (tom2DCtrl*)Sim::findObject(dAtoi(argv[2]));
    if (screen)
    {
        object->setScreen(screen);
        return true;
    }
    


    return true;
}



ConsoleMethod(Pathfinder, setWeightByNodeId, bool, 4, 4, "nodeId, U8 weight")
{
//   object->UpdateGraphWeight(dAtoi(argv[2]), dAtoi(argv[3]));
    object->UpdateGraphWeight(dAtoi(argv[3]), dAtoi(argv[2]));
    return true;
}

void Pathfinder::mapReady()
{

    //calculate the cost lookup table
    Con::printf("CreateAllPairsCostsTable ... please wait.");


    //clear if set 
    m_PathCosts.clear();


    //record the time taken  
    m_PathCosts = CreateAllPairsCostsTable(*m_pGraph);

    Con::printf("CreateAllPairsCostsTable ... done.");

}

ConsoleMethod(Pathfinder, mapReady, void, 2, 2, "")
{
    object->mapReady();

}

ConsoleMethod(Pathfinder, render, void, 2, 2, "")
{
    object->Render();
    
}
//=============================================================================================
//=============================================================================================

S32 Pathfinder::getNodeIndex(F32 x, F32 y)
{
    S32 xNode;
    S32 yNode;

    //position out of area 
    if (!mArea.pointInRect(Point2I((U32)x, (U32)y)))
        return -1;

    xNode = mFloor(((x - mArea.point.x) / mSquareSizeX));
    yNode = mFloor(((y - mArea.point.y) / mSquareSizeY));


    if (xNode < 0 || xNode > m_iCellsX || yNode < 0 || yNode > m_iCellsX)
        return -1;

    S32 node = xNode + yNode * m_iCellsX;
    if (node >= 0 && node < m_pGraph->NumNodes())
        return node;

    return -1;
}

SimObject* Pathfinder::createPath(Point2F start, Point2F end, bool smoothPath )
{

    m_iSourceCell = getNodeIndex(start.x, start.y);
    m_iTargetCell = getNodeIndex(end.x, end.y);

    if ( m_CurrentAlgorithm == non)
      m_CurrentAlgorithm = search_astar;

    //m_CurrentAlgorithm = search_dijkstra;
    if (UpdateAlgorithm())
    {

        SimObject* pathObject = new SimObject();

        S32 i = 0;
        std::list<int>::iterator it = m_Path.begin();
        for (it; it != m_Path.end(); ++it)
        {
            char nbuf[64]; dMemset(nbuf, 0, 64);
            dSprintf(nbuf, 20, "node%d", i);
            const char* fieldName = StringTable->insert(nbuf);

            Point2I lPoint = mArea.point + Point2I((S32)m_pGraph->GetNode(*it).Pos().x, (S32)m_pGraph->GetNode(*it).Pos().y );

            dSprintf(nbuf, 64, "%d %d",
                lPoint.x,
                lPoint.y
            );

            pathObject->setDataField(fieldName, NULL, nbuf);
            i++;
        }

        return pathObject;
    }
    return NULL;


}


ConsoleMethod(Pathfinder, findPath, S32, 4, 5, "findPath (Point2F start, Point2F goal, bool smoothPath = true) - Create a path between the two points and Return the ID of path")
{

    if ((dStrlen(argv[2]) != 0) && (dStrlen(argv[3]) != 0))
    {
        Point2F start;
        Point2F goal;
        bool smooth = false;
        dSscanf(argv[2], "%f %f", &start.x, &start.y);
        dSscanf(argv[3], "%f %f", &goal.x, &goal.y);
        if (argc > 5)
            smooth = dAtob(argv[4]);


        SimObject* result = object->createPath(start, goal, smooth);
        if (result)
        {
            result->registerObject();
            return result->getId();
        }
        else
            return 0;

    }
    else {
        return 0;
    }
}


// SceneObj.pathFinder.setAlgorithm(1);

ConsoleMethod(Pathfinder, setAlgorithm, void, 3, 3, "non,search_astar,search_dijkstra,search_bfs,search_dfs")
{

    S32 lValue;
    Pathfinder::algorithm_type Algo;
    
    switch (dAtoi(argv[2]))
    {
    case 1: Algo = Pathfinder::search_astar; break;
    case 2: Algo = Pathfinder::search_dijkstra; break;
    case 3: Algo = Pathfinder::search_bfs; break;
    case 4: Algo = Pathfinder::search_dfs; break;
    default: Algo = Pathfinder::non; break;


    }
    object->setAlgorithm(Algo);
    object->UpdateAlgorithm();
    
}


ConsoleMethod(Pathfinder, getNodeToNodeCosts, S32, 4, 4, "(param Point2F from,  Point2F to;	return F32 distance)"
    "return nodecosts of to points to calculated closed path it -1 then it failed")
{

    const Point2F lFrom;
    const Point2F lTo;
    dSscanf(argv[2], "%g %g", &lFrom.x, &lFrom.y);
    dSscanf(argv[3], "%g %g", &lTo.x, &lTo.y);
    S32 result = object->getNodeToNodeCosts(lFrom, lTo);

    return result;
}




