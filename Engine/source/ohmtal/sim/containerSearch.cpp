//-----------------------------------------------------------------------------
// class containerSearch public SimSet
// replacement for InitContainerRadiusSearch which is not safe! 
// @since 2025-11-22
/*
---------------------------------------------
  defaults: 
    position = Point3F(0.0,0.0,0.0)
    radius   = 10.0
    typemask = $TypeMasks::PlayerObjectType
    useClientContainer = false
---------------------------------------------

Testing:
$cs=new ContainerSearch() { position=pl_visitor.getPosition(); radius=40;  }; 
echo($cs.search()); $cs.listObjects();
for($i=0; $i<$cs.getCount();$i++) echo($cs.getObject($i).getShapeName());
$cs.delete();
*/
//-----------------------------------------------------------------------------
#include "console/sim.h"
#include "console/consoleInternal.h"
#include "scene/sceneContainer.h"
#include "scene/sceneObject.h"
#include "containerSearch.h"
//-----------------------------------------------------------------------------
IMPLEMENT_CONOBJECT(containerSearch);
//-----------------------------------------------------------------------------
void containerSearch::initPersistFields()
{

//    addField("useDatablockCalls", TypeBool, Offset(mUseDatablockCalls, LiteUnit),
//        "@brief Use server callbacks on datablock instead on object \n");

    Parent::initPersistFields();
    addGroup("Search");
    addField("position", TypePoint3F, Offset(mPosition, containerSearch));
    addField("radius", TypeF32, Offset(mRadius, containerSearch));
    addField("typemask", TypeS32, Offset(mMask, containerSearch));
    addField("useClientContainer", TypeBool, Offset(mUseClientContainer, containerSearch));
    addField("TimeStamp", TypeS32, Offset(mTimeStamp, containerSearch));
    
    endGroup("Search");

}
//-----------------------------------------------------------------------------
S32 containerSearch::search()
{
    SimpleQueryList	lQueryList;

    //clear dunno why this function does not exits !  >>
    this->lock();
    while (this->size() > 0)
        this->removeObject(*(this->begin()));
    this->unlock();
    //<< clear


    Box3F queryBox(mPosition,mPosition);
#ifdef TORQUE_SHADERGEN
    queryBox.minExtents -= Point3F(mRadius, mRadius, mRadius);
    queryBox.maxExtents += Point3F(mRadius, mRadius, mRadius);
#else
    queryBox.min -= Point3F(mRadius, mRadius, mRadius);
    queryBox.max += Point3F(mRadius, mRadius, mRadius);
#endif
    if (mUseClientContainer)
        gClientContainer.findObjects(queryBox, mMask, SimpleQueryList::insertionCallback, &lQueryList);
    else
        gServerContainer.findObjects(queryBox, mMask, SimpleQueryList::insertionCallback, &lQueryList);
    
    
    mTimeStamp = Platform::getTime();
    for (S32 a = 0; a < lQueryList.mList.size(); ++a) 
    {
        // this->addObject(static_cast<SimObject*>(&(*lQueryList.mList[a])));
       // SimObject tmpTarget = static_cast<SimObject*>(&(*lQueryList.mList[a]));
       
       this->addObject(dynamic_cast<SimObject*>(&(*lQueryList.mList[a])));
    }

    return this->size();
}
//-----------------------------------------------------------------------------

DefineEngineMethod(containerSearch, search, S32, (), ,
   "search with the given parameter and add the objects to itself")

{
   return object->search();
}


/*
ConsoleMethod(containerSearch, search, S32, 2, 2, "search with the given parameter and add the objects to itself")
{
   return object->search();
}
*/

