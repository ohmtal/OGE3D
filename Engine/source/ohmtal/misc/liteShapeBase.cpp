//-----------------------------------------------------------------------------
// LiteShapeBase
// @author     XXTH
// @created    2023-03-13
// i detected shapebase seams to have a memory leak and i need to find out where !!!
// 100 staticshapes eat 24 bytes each sec on dedicated server !!! same player, aiPlayer, aiEntity and aiEntityRTS do .. so i
// guess it must be shapebase
//
//
// 
// At the moment is makes no sense :P 
// 
//-----------------------------------------------------------------------------
#include "platform/platform.h"
#include "console/consoleInternal.h"
#include "console/engineAPI.h"
#include "console/consoleObject.h"

#include "liteShapeBase.h"


/*
Testing:
new ShapeBaseData(LiteShapeData) { shapeName="wtp/art/shapes/actors/mrbox/player.dts"; };

$mrBox = new LiteShapeBase() {datablock=LiteShapeData; pos = BaseMarker1.getPosition(); }



*/

//---------------- CLASS LiteShapebase ----------------------------------------
IMPLEMENT_CO_NETOBJECT_V1(LiteShapeBase);

//-----------------------------------------------------------------------------
void LiteShapeBase::processTick(const Move* move)
{
   Parent::processTick(move);
}
//-----------------------------------------------------------------------------
