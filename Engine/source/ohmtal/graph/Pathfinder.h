#ifndef Pathfinder_H
#define Pathfinder_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   Pathfinder.h
//
//  Desc:   class enabling users to create simple environments consisting
//          of different terrain types and then to use various search algorithms
//          to find paths through them
//
//  Author: Mat Buckland  (fup@ai-junkie.com)
//
// Modification: changed to OGE Object for testing
//------------------------------------------------------------------------
#include <vector>
#include <fstream>
#include <string>
#include <list>


//-------------------------------------------
#ifndef _TYPES_H_
#include "platform/types.h"
#endif
#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif
#ifndef _MRECT_H_
#include "math/mRect.h"
#endif
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _TOM2D_H_
#include "ohmtal/2d/tom2DCtrl.h"
#endif



#ifndef S2DVECTOR_H
#include "2D/Vector2D.h"
#endif
#ifndef SPARSEGRAPH_H
#include "SparseGraph.h"
#endif
#ifndef GRAPHALGORITHMS_H
#include "GraphAlgorithms.h"
#endif

#ifndef GRAPTH_UTILS_H
#include "misc/utils.h"
#endif

#ifndef GRAPH_EDGE_TYPES_H
#include "GraphEdgeTypes.h"
#endif

#ifndef GRAPH_NODE_TYPES_H
#include "GraphNodeTypes.h"
#endif

#ifndef _GFXDEVICE_H_
#include "gfx/gfxDevice.h"
#endif

#ifndef _GFX_GFXDRAWER_H_
#include "gfx/gfxDrawUtil.h"
#endif




class Pathfinder : public SimObject
{
    typedef SimObject Parent;

public:
    DECLARE_CONOBJECT(Pathfinder);


/*
  enum brush_type
  {
    normal   = 0,
    obstacle = 1,
    water    = 2,
    mud      = 3,
    source   = 4,
    target   = 5
  };
*/      
  enum algorithm_type
  {
    non,
    search_astar,
    search_bfs,
    search_dfs,
    search_dijkstra
  };

protected:
    tom2DCtrl* mScreen;

private:
  
  //the terrain type of each cell
  std::vector<int>              m_TerrainType;

  //this vector will store any path returned from a graph search
  std::list<int>                m_Path;

  //create a typedef for the graph type
  typedef SparseGraph<NavGraphNode<void*>, GraphEdge> NavGraph;

  NavGraph*                     m_pGraph;
  
  //this vector of edges is used to store any subtree returned from 
  //any of the graph algorithms (such as an SPT)
  std::vector<const GraphEdge*> m_SubTree;

  //the total cost of the path from target to source
  double                         m_dCostToTarget;

  //the currently selected algorithm
  algorithm_type                m_CurrentAlgorithm;

  //the current terrain brush
//  brush_type                    m_CurrentTerrainBrush;

  //the dimensions of the cells
  double                        m_dCellWidth;
  double                        m_dCellHeight;

  //number of cells vertically and horizontally
  int                           m_iCellsX,
                                m_iCellsY;

  //local record of the client area ==> use mArea.extent!!!
//  int                           m_icxClient,
                                //m_icyClient;

  //the indices of the source and target cells
  int                           m_iSourceCell,
                                m_iTargetCell;

  //flags to indicate if the start and finish points have been added
  bool                          m_bStart,
                                m_bFinish;

  //should the graph (nodes and GraphEdges) be rendered?
  bool                          m_bShowGraph;

  //should the tile outlines be rendered
  bool                          m_bShowTiles;

  //holds the time taken for the most currently used algorithm to
  //complete
  //FIXME double                        m_dTimeTaken;
  

  RectI mArea;
  F32  mSquareSizeX, mSquareSizeY;

std::vector<std::vector<F32> >  m_PathCosts;



  //helper function for PaintTerrain (see below)
//  void  UpdateGraphFromBrush(int brush, int CellIndex);
  //void  UpdateGraphFromBrush(int brush, int CellIndex);
 
 std::string GetNameOfCurrentSearchAlgorithm()const;

public:

  Pathfinder():m_bStart(false),
                m_bFinish(false),
                m_bShowGraph(false),
                m_bShowTiles(true),
                m_dCellWidth(0),
                m_dCellHeight(0),
                m_iCellsX(0),
                m_iCellsY(0),
                //FIXME m_dTimeTaken(0.0),
               // m_CurrentTerrainBrush(normal),
                m_iSourceCell(0),
                m_iTargetCell(0),
                //m_icxClient(0),
                //m_icyClient(0),
                m_dCostToTarget(0.0),
                m_pGraph(NULL),
                mScreen(NULL),
                mArea(0,0,0,0),
                mSquareSizeX(0.f),
                mSquareSizeY(0.f)


  {}

  ~Pathfinder(){
      delete m_pGraph;  
      m_PathCosts.clear();  
  }

  void setArea(RectI lArea);
  void CreateGraph(int CellsX, int CellsY);
//  void CreateGraph(RectI lArea, F32 lSquareSize = 8);


  void Render(bool renderCurrentPath = false, bool renderInfoText = false);

  //this will paint whatever cell the cursor is currently over in the 
  //currently selected terrain brush
 // void PaintTerrain(POINTS p);
  SimObject* createPath(Point2F start, Point2F end, const bool smoothPath = true);
  S32 getNodeIndex(F32 x, F32 y);

  //the algorithms
  void CreatePathDFS();
  void CreatePathBFS();
  void CreatePathDijkstra();
  void CreatePathAStar();
  

  //if m_bShowGraph is true the graph will be rendered
  void ToggleShowGraph(){m_bShowGraph = !m_bShowGraph;}
  void SwitchGraphOn(){m_bShowGraph = true;}
  void SwitchGraphOff(){m_bShowGraph = false;}
  bool isShowGraphOn()const{return m_bShowGraph;}

  void ToggleShowTiles(){m_bShowTiles = !m_bShowTiles;}
  void SwitchTilesOn(){m_bShowTiles = true;}
  void SwitchTilesOff(){m_bShowTiles = false;}
  bool isShowTilesOn()const{return m_bShowTiles;}

//  void ChangeBrush(const brush_type NewBrush){m_CurrentTerrainBrush = NewBrush;}

  void ChangeSource(const int cell){m_iSourceCell = cell;}
  void ChangeTarget(const int cell){m_iTargetCell = cell;}

  //converts a POINTS to an index into the graph. Returns false if p
  //is invalid
  bool PointToIndex(Point2F p, int& NodeIndex);

  //returns the terrain cost of the brush type
  //double GetTerrainCost(brush_type brush);

  void Save( char* FileName);
  void Load( char* FileName);


  void UpdateGraphWeight(int weight, int CellIndex);

  void setScreen(tom2DCtrl* lScreen) { mScreen = lScreen; }

  void setAlgorithm(algorithm_type lAlgo) { m_CurrentAlgorithm = lAlgo; }
  
  //this calls the appropriate algorithm
  bool  UpdateAlgorithm();

  //F32 getSquareSize() { return mSquareSize; }
  //F32 getHalfSquareSize() { return mSquareSize / 2; }
 
  double getNodeToNodeCosts(Point2F from, Point2F to);


  void mapReady();

  S32 getNodeCount() {
      return  m_pGraph->NumNodes();
  }

  RectI getArea() {
      return mArea;
  }


};


//=============================================================================================================================
//=============================================================================================================================
//=============================================================================================================================
template <class graph_type>
void Pathfinder_DrawGraph(tom2DCtrl* screen, Point2I offset,  const graph_type& graph, ColorI color, bool DrawNodeIDs = false)
{


    //just return if the graph has no nodes
    if (graph.NumNodes() == 0) return;


    


    //draw the nodes 
    class graph_type::ConstNodeIterator NodeItr(graph);

    // draw as batch
    Vector<RectI> lLines;
    lLines.empty();

    for (const  typename graph_type::NodeType* pN = NodeItr.begin();
        !NodeItr.end();
        pN = NodeItr.next())
    {
        //gdi->Circle(pN->Pos(), 2);
//FIXME but do we need this ?         screen->drawPrimPoint(offset+Point2I(pN->Pos().x, pN->Pos().y), 2, color);

        if (DrawNodeIDs)
        {
//FIXME            gdi->TextColor(200, 200, 200);
//FIXME            gdi->TextAtPos((int)pN->Pos().x + 5, (int)pN->Pos().y - 5, ttos(pN->Index()));
        }

        class graph_type::ConstEdgeIterator EdgeItr(graph, pN->Index());
        for (const typename graph_type::EdgeType* pE = EdgeItr.begin();
            !EdgeItr.end();
            pE = EdgeItr.next())
        {

           lLines.push_back(RectI(
              offset + Point2I(pN->Pos().x, pN->Pos().y),
              offset + Point2I(graph.GetNode(pE->To()).Pos().x, graph.GetNode(pE->To()).Pos().y)
           ));
        }



    } //each pointer

        //draw the lines
    GFXVertexBufferHandle<GFXVertexPCT> verts(GFX, lLines.size() * 2, GFXBufferTypeVolatile);
    verts.lock();

    S32 j = 0;
    for (S32 i = 0; i < lLines.size(); i++)
    {
       verts[j].point.set(lLines[i].point.x, lLines[i].point.y, 0.f);
       verts[j + 1].point.set(lLines[i].extent.x, lLines[i].extent.y, 0.f);
       verts[j].color = color;
       verts[j + 1].color = color;
       j += 2;
    }

    verts.unlock();

    GFX->setVertexBuffer(verts);
    GFX->setStateBlock(GFX->getDrawUtil()->get2DStateBlockRef());
    GFX->setupGenericShaders();
    GFX->drawPrimitive(GFXLineList, 0, lLines.size());

}


#endif
