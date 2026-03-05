//-----------------------------------------------------------------------------
// LiteShapeBase
// @author     XXTH
// @created    2023-03-13
// i detected shapebase seams to have a memory leak and i need to find out where !!!
// 100 staticshapes eat 24 bytes each sec on dedicated server !!! same player, aiPlayer, aiEntity and aiEntityRTS do .. so i
// guess it must be shapebase 
//-----------------------------------------------------------------------------
#ifndef _LITESHAPEBASE_H_
#define _LITESHAPEBASE_H_

#ifndef _SHAPEBASE_H_
#include "T3D/shapeBase.h"
#endif

class LiteShapeBase : public ShapeBase
{
protected:
   typedef ShapeBase Parent;

   
public:
   DECLARE_CONOBJECT(LiteShapeBase);

/*
   LiteShapeBase() :
      mBla(false);
   {};
   */
   void processTick(const Move* move);

};


#endif //_LITESHAPEBASE_H_
