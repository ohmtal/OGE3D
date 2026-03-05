#include "console/engineAPI.h"
#include "portingTestObject.h";


IMPLEMENT_CONOBJECT(portingTestObject);

bool portingTestObject::onAdd()
{
   if (!Parent::onAdd())
      return false;


   //linkNamespaces();
   // Call onAdd in script!
   //Con::executef(this, 2, "onAdd", Con::getIntArg(getId()));
   Con::executef(this, "onAdd", getId());
   return true;
}

void portingTestObject::onRemove()
{
   // Call onRemove in script!
   Con::executef(this, "onRemove", getId());

   //unlinkNamespaces();


   Parent::onRemove();
}


DefineEngineMethod(portingTestObject, testparams, void, (U32 uvalue, F32 fvalue, S32 svalue),(4711) , "")
{
      Con::printf("Values: %d %f %d",uvalue, fvalue, svalue );
}


DefineEngineMethod(portingTestObject, testparams2, void, (S32 v1, S32 v2, S32 v3, S32 v4, S32 v5, S32 v6), (4711,815), "")
{
   Con::printf("Values: %d %d %d %d %d %d", v1,v2,v3,v4,v5,v6);
}
