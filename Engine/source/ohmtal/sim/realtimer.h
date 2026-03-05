#if 0 //FIXME MAKE IT BETTER TO COMPILE !!!

#ifndef _REALTIMER_H_
#define _REALTIMER_H_
//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// realtimer by T.Huehn 2009 (XXTH) (c) huehn-software 2009
// used to trigger realtime events and trigger gametime events
// ported to OGE3D 2025-12-12
//-----------------------------------------------------------------------------
#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif

#ifndef _TVECTOR_H_
#include "core/util/tVector.h"
#endif

//-----------------------------------------------------------------------------
// ok it should work like freebsd's cronjob so something like that:
/*
#              field          allowed values
#              -----          --------------
#              minute         0-59
#              hour           0-23
#              day of month   1-31
#              month          1-12 (or names, see below)
#              day of week    0-7 (0 or 7 is Sun)

allowed asterix:
 *      => dont care about
 NOT NEEDED: * / 5  => every 5  (blanks added because of problems with escaping ;))
 NOT NEEDED: 1-10   => between 1 and 10

 so every entry is a S32 with  -1 means same as "*" in cronjob

-----
* look at ConsoleMethod(SimObject,schedule, S32, 4, 0, "object.schedule(time, command, <arg1...argN>);")
  how to add the args.
* SimConsoleEvent is also good example

*/
class RealTimeEvent {
protected:
   S32 mArgc;
   char **mArgv;
   bool mOnObject;
   SimObject* mObject;

	S32   mMin
		, mHour
		, mDayOfMonth
		, mMonth
		, mDayOfWeek;

	bool mDeleteAfterExecuted;
	bool mMarkedForDeletion;

public:

	//constructor, destructor
	// ConsoleValueRef
	// RealTimeEvent(S32 argc, const char **argv, bool lOnObject, SimObject *lObject, bool lDeleteAfterExecuted
	// 	          , S32 lMin, S32 lHour, S32 lDayOfMonth, S32 lMonth, S32 lDayOfWeek );
	RealTimeEvent(S32 argc, ConsoleValueRef *argv, bool lOnObject, SimObject *lObject, bool lDeleteAfterExecuted
	 	          , S32 lMin, S32 lHour, S32 lDayOfMonth, S32 lMonth, S32 lDayOfWeek );
	~RealTimeEvent();

	bool deleteMe() const { return mMarkedForDeletion; }
	bool validate(Platform::LocalTime lCurTime);	
    void process();
	void Print(U32 lId);
};
//-----------------------------------------------------------------------------

class RealTimer {

protected:
	Vector<RealTimeEvent>	mEvents;


private:
	bool mActive;     // is it active ? 

	bool mRealTimeSynced; // did we do a initial sync ? 
	S32 mSkipDelta;  //how many ms to skip until next run 
	U32 mOverAllDelta; // keep track on overall delta

	bool mUseGameTime; 
	U32 mTimeOfGameDay;    // current day time in ms
	S32 mGameDayHours;     // a gameday must at least one real hour long!
	U32 mGameHour;         // we only care about hours!
	U32 mLastGameHourEventFired; // last event to keep track on fire it twice 
	U32 mGameHourLenMS;    // how many ms is a game hour long 
	U32 mGameDayLenMS;    // how many ms is a game day long 

	
public:
	RealTimer();
	~RealTimer();
	void advanceTime(SimTime timeDelta);
	void setActive (bool lVal);
	bool isActive() const { return mActive; }
	S32 getGameHour() const { return mGameHour; }
	void setGameHours(U32 lVal);
	void addEvent(const RealTimeEvent &lEvent);
	void removeEvent(U32 iter);
	void printEvents();
	void clearEvents();
	S32 getEventCount() { return mEvents.size(); }

};

extern RealTimer* gRealTimer;
#endif // _REALTIMER_H_
#endif //FIXME MAKE IT BETTER TO COMPILE !!!
