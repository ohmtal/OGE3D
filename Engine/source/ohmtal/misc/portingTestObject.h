/* =================================================================================================
 portingTestObject
 1.) Testing DefineEngineMethod
   $foo = new portingTestObject();
   $foo.testparams(1,2);
     : 1 2 4711 ==> so the default values are used from last ?!
   $foo.testparams2(1,2,3,4);

================================================================================================= */
#ifndef _portingTestObject_H_
#define _portingTestObject_H_

#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif

#ifndef _MPOINT3_H_
#include "math/mPoint3.h"
#endif


class portingTestObject : public SimObject
{
private:
   typedef SimObject Parent;
protected:
   U32 mU;
   F32 mF;
   S32 mS;
   bool mB;

public:
   DECLARE_CONOBJECT(portingTestObject);
   portingTestObject() {
      mU = 0;
      mF = 0.f;
      mS = 0;
      mB = false;
   }
   bool onAdd();
   void onRemove();

};



#endif
