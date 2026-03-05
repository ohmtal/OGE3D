//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// aiGroup by t.huehn (XXTH) (c) huehn-software 2009
//-----------------------------------------------------------------------------
#ifndef _AIGROUP_H_
#define _AIGROUP_H_


#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif
#ifndef _AIENTITY_H_
#include "aiEntity.h"
#endif



class aiGroup : public SimObject
{
private:
   typedef SimObject Parent;

protected:
   AIEntity*	mLeader;
   Vector<AIEntity*> mMembers;

   bool mAutoAssignLeader;
   S32  mMaxMembers;

public:
   //creation methods
   DECLARE_CONOBJECT(aiGroup);

// bool addMember(aiEntity* lVal);
// setLeader(aiEntity* lVal);
// rmvMember(aiEntity* lVal);
// bool isMember(aiEntity* lVal);
// bool isLeader(aiEntity* lVal);
// ShapeBase* getLeader();
// S32 getMemberCount();
// void SendMemberEvent(   S32 mArgc;   char **mArgv; ); //console based event

// setFormation
// pathMarsh
// pathFlee
// attackTargets



};


#endif //_AIGROUP_H_
