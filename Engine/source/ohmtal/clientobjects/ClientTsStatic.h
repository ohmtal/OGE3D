//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------

#ifndef _CLIENTTSSTATIC_H_
#define _CLIENTTSSTATIC_H_


#ifndef _TSSTATIC_H_
#include "T3D/tsStatic.h"
#endif //  _TSSTATIC_H_

class ClientTsStatic : public TSStatic
{
private:
      typedef TSStatic  Parent;
protected:
   
   bool onAdd();
public:
   void setSkinName(const char* name);
   void prepRenderImage(SceneRenderState* state);

   DECLARE_CONOBJECT(ClientTsStatic);

};


#endif //_CLIENTTSSTATIC_H_
