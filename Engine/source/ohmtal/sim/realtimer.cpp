#if 0 //FIXME MAKE IT BETTER TO COMPILE !!!
//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// realtimer by T.Huehn 2009 (XXTH) (c) huehn-software 2009
// to bad it dont have timezone :(

//-----------------------------------------------------------------------------
/* Docu:
   
   1.) Source changes:
       1.1) Engine: gameProcess.cc
	        On bottom before
			   PROFILE_END();
			   return ret;
		    Add:
			   gRealTimer->advanceTime(timeDelta);

		    On top after
				#include "console/consoleTypes.h"
			Add:
				#include "game/realtimer.h"

	   2.2) Script: [YOURMOD]/game.cs 
			Not the optional GameTimemanagement which is set by simply add 
			dayhours = [int HOURS] to your mission info. Default is 3 change
			it if you like.

			On bottom of function startGame()
			Add:
				realTimerStartMissionNotify();
				//optional you need to set the hours in MissionInfo!
				if (isObject(MissionInfo) && MissionInfo.dayHours !$= "0") {
					// init serverside gamehour management
					if (MissionInfo.dayHours $= "")
						setGameHours(3);
					else
						setGameHours(MissionInfo.dayHours);
				}
	        
			On bottom of function onMissionEnded()
			 Add: 
				   realTimerEndMissionNotify();


  2.) ConsoleFunctions:
	2.1) realTimerStartMissionNotify
	    Must be called when mission starts.
    2.2) realTimerEndMissionNotify
	    Must be called when mission ends.
    2.3) setGameHours(int Hour);
	    Setup the Gamehours and enable usage of it.
		Should be called after realTimerStartMissionNotify.

	2.4) bool RealTimerSchedule(char* timedefinition, refobject|0, command, <arg1...argN>)
	    like schedule but first parameter must be a blank separated string with the following params:
			
			"int Min int Hour int DayOfMonth int Month int DayOfWeek bool OnlyOnce"
			Note: 0 is sunday ! ... was on my windows machine and on my unix machine

		Example Every Day at 15:30 localtime:
			RealTimerSchedule(
			    "30 15 -1 -1 -1 false" //timedefintion
				,0                     //no object
				,"timeForTea"          //function to call
								       //and more params can be added like on usual schedule
				
			);
	2.5) RealTimerPrintEvents
	     listPending Events
    2.6) RealTimerDeleteEvent int ID
	     need the id from listEvents
		 !!!WARNING!!!
		 Dont wait to long the id can get invalid!

    2.7) realTimerActive
	    is it active ?! this may also means a mission is loaded!
			
  3.) Events:
	3.1) function OnNewServerGameHour(%hour)

 -----------------------------------------------------------------------------
 
 2010-05-21: Bugfix without an object event didn't get deleteed

*/
//-----------------------------------------------------------------------------

#include "realtimer.h"
#include "console/engineAPI.h"
#include "platform/platform.h"
#include "platform/types.h"
#include "math/mMathFn.h"

RealTimer* gRealTimer = new RealTimer();

//-----------------------------------------------------------------------------
// RealTimeEvent
//-----------------------------------------------------------------------------
// RealTimeEvent::RealTimeEvent(S32 argc, const char **argv, bool lOnObject, SimObject *lObject, bool lDeleteAfterExecuted
RealTimeEvent::RealTimeEvent(S32 argc, ConsoleValueRef *argv, bool lOnObject, SimObject *lObject, bool lDeleteAfterExecuted
, S32 lMin, S32 lHour, S32 lDayOfMonth, S32 lMonth, S32 lDayOfWeek )
{

	mMarkedForDeletion = false;

	mMin		=	lMin;
	mHour		=	lHour;
	mDayOfMonth =   lDayOfMonth;
	mMonth		=	lMonth;
	mDayOfWeek  =   lDayOfWeek;

	mDeleteAfterExecuted = lDeleteAfterExecuted ;
	if (lOnObject) {
		if (lObject) {
			mOnObject = true;
			mObject   = lObject;
		} else {
			//no object ... bah ! 
			mObject   = NULL;
			mMarkedForDeletion = true;
		}
	} else {
		mOnObject = false;
		mObject   = NULL;
	}

   mArgc = argc;
   U32 totalSize = 0;
   S32 i;
   for(i = 0; i < argc; i++)
      totalSize += dStrlen(argv[i]) + 1;
   totalSize += sizeof(char *) * argc;

   mArgv = (char **) dMalloc(totalSize);
   char *argBase = (char *) &mArgv[argc];
   dsize_t tokLen = 0;

   for(i = 0; i < argc; i++)
   {
      mArgv[i] = argBase;
	  tokLen = dStrlen(argv[i]) + 1;
      dStrcpy(mArgv[i], argv[i], tokLen);
      argBase += tokLen;
   }
}

//-----------------------------------------------------------------------------
RealTimeEvent::~RealTimeEvent()
{


}
//-----------------------------------------------------------------------------
bool RealTimeEvent::validate(Platform::LocalTime lT)
{

	if (   !mMarkedForDeletion
		&& ( mMonth == -1      || lT.month+1 == mMonth)  //jan = 0!!!
		&& ( mDayOfMonth == -1 || lT.monthday == mDayOfMonth ) 
		&& ( mDayOfWeek  == -1 || lT.weekday  == mDayOfWeek  ) 
		&& ( mHour       == -1 || lT.hour     == mHour       ) 
		&& ( mMin        == -1 || lT.min      == mMin        ) 
		) {
		return true;
	}
    return false;
}
//-----------------------------------------------------------------------------
void RealTimeEvent::Print(U32 lId)
{
    //"int Min int Hour int DayOfMonth int Month int DayOfWeek bool OnlyOnce"

	char lArgBuff[256];
	dSprintf(lArgBuff,sizeof(lArgBuff),"%s","");

	for (S32 i = 0 ; i < mArgc; i++)
	{
		dSprintf(lArgBuff,sizeof(lArgBuff),"%s %s",lArgBuff,mArgv[i]);
	}


	if (mOnObject) {
		if (!mObject)
		Con::errorf("ID: %d, Def: %d %d %d %d %d %d , INVALID Object!, Call: %s"
		    ,lId
			,mMin,mHour,mDayOfMonth,mMonth,mDayOfWeek,mDeleteAfterExecuted
		    ,lArgBuff);

		else
		Con::printf("ID: %d, Def: %d %d %d %d %d %d , Object: %d, Call: %s"
		    ,lId
			,mMin,mHour,mDayOfMonth,mMonth,mDayOfWeek,mDeleteAfterExecuted
			,mObject->getIdString()
		    ,lArgBuff);
		  

	} else {
	

		Con::printf("ID: %d, Def: %d %d %d %d %d %d , Call: %s"
			,lId
			,mMin,mHour,mDayOfMonth,mMonth,mDayOfWeek,mDeleteAfterExecuted
		    ,lArgBuff);
	}

}
//-----------------------------------------------------------------------------
void RealTimeEvent::process()
{
 	if(mOnObject) {
		if (mObject) {
			Con::execute(mObject, mArgc, const_cast<const char**>( mArgv ));
			mMarkedForDeletion = mDeleteAfterExecuted;
		} else {
			mMarkedForDeletion = true;
		}
	} else {
      Con::execute(mArgc, const_cast<const char**>( mArgv ));
	  //1.97.2 Bug ?! where is the  mMarkedForDeletion ?!
	  mMarkedForDeletion = mDeleteAfterExecuted;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
RealTimer::RealTimer()
{
  mActive      = false;
  mUseGameTime = false;
  mSkipDelta = 10000;
  mRealTimeSynced = false;
  mGameHour = 666;
  mLastGameHourEventFired = 555;

}
RealTimer::~RealTimer()
{
  mEvents.clear();
}


//-----------------------------------------------------------------------------
// AdvanceTime
// Schedules goes here. The counter is synced against the localtime and
// check events every realtime minute.
// Gameevents are only used on a new hour so it should not a timeing problem.
// 
//
//
// !! warning !! in void DemoGame::processTimeEvent(TimeEvent *event) the
// timedelta is capped to 1024, this can happen when a stoppoint was set
// I ignore this here for now - maybe i should sync it every hour ?! 
// nah i sync it if its out of sync ;) Problem gamehourevents may called 
// multiple times.
// I may should remove the timedelta cap.
// Since I dont know if i get in trouble removing the cap I added 
// mLastGameHourEventFired to prevent mutiple same gamehour events.
//-----------------------------------------------------------------------------
void RealTimer::advanceTime(SimTime timeDelta) 
{ 
  if (!mActive)
		return;

  mSkipDelta -= timeDelta;
  if ( mUseGameTime ) 
	mOverAllDelta += timeDelta;

  if (mSkipDelta > 0 )
		return;
  // bad: timeDelta == 1024  => processTimeEvent made a bad decision ? 
  Platform::LocalTime lt;
  Platform::getLocalTime(lt);
  if (!mRealTimeSynced ) 
  {
	 mSkipDelta = (60 - lt.sec) * 1000;
   	 mRealTimeSynced = true;
	 if (mUseGameTime) {
		mGameHour = 666; //reset to a bogus hour :P
		mTimeOfGameDay = ((( lt.hour % mGameDayHours ) * 60  + lt.min ) * 60 + lt.sec ) * 1000;
	 }
	 return;
  } else {
	  if (lt.sec != 0) { //sync again!
	    mRealTimeSynced = false;
		mSkipDelta += 1000;
	  } else {
		mSkipDelta += 60000;
	  }
  }

  if ( mUseGameTime ) {
	  U32 prevTime = mTimeOfGameDay;
	  mTimeOfGameDay = (mTimeOfGameDay + mOverAllDelta) % mGameDayLenMS ;
	  U32 curGameHour = mFloor((F32)mTimeOfGameDay / (F32)mGameHourLenMS);
	  if (mGameHour != curGameHour && curGameHour != mLastGameHourEventFired) {
			mGameHour = curGameHour;
			mLastGameHourEventFired = mGameHour;
			// Con::executef(2,"OnNewServerGameHour",Con::getIntArg(mGameHour));
			Con::executef("OnNewServerGameHour",mGameHour);
	  }
	  mOverAllDelta = 0;
  }

#ifdef TORQUE_DEBUG
 Con::printf("RealTimer - check! hour: %d, Min: %d, Sec: %d", lt.hour, lt.min,lt.sec);
#endif


 if (mEvents.size()>0) 
 {
	U32 iter = 0;
	while (iter < mEvents.size())
	{
		if (mEvents[iter].deleteMe()) {
			mEvents.erase(iter);
		} else {
			if (mEvents[iter].validate(lt))
				mEvents[iter].process();
			iter++;
		}

	} //while

 } //if size
		

}
//-----------------------------------------------------------------------------
void RealTimer::printEvents()
{


	Platform::LocalTime lt;
	Platform::getLocalTime(lt);
	Con::printf("---------------------------------------");
	Con::printf("CURRENT REALTIMER EVENTS: %d", mEvents.size());
	Con::printf("CURRENT TIME: hour: %d, Min: %d, Sec: %d", lt.hour, lt.min, lt.sec);
	Con::printf("Realtime synced: %d, using gametime:%d", mRealTimeSynced, mUseGameTime);
	if (!mActive)
	{
		Con::warnf("WARNING - REALTIME IS NOT ACTIVE !!!!!");
	}
	Con::printf("---------------------------------------");
	Con::printf("int Min int Hour int DayOfMonth int Month int DayOfWeek (0=sun) bool OnlyOnce");

	U32 iter = 0;
	while (iter < mEvents.size())
	{
		mEvents[iter].Print(iter);
		iter++;

	} //while
	Con::printf("---------------------------------------");

}
//-----------------------------------------------------------------------------
void RealTimer::removeEvent(U32 iter)
{
	if (mEvents.size() > iter) 
		mEvents.erase(iter);

}
//-----------------------------------------------------------------------------
void RealTimer::addEvent(const RealTimeEvent &lEvent)
{
	mEvents.push_back(lEvent);
}
void RealTimer::clearEvents() { 
	mEvents.clear(); 
}
//-----------------------------------------------------------------------------
void RealTimer::setGameHours(U32 lVal) {
	if (lVal == 0) {
		 mUseGameTime    = false; 
		 return;
	}
	 
	 mGameDayHours = lVal; 
	 mGameHourLenMS =  mGameDayHours * 150000;//mFloor(((F32)mGameDayHours * 3600000 / 24.f) ); 
	 mGameDayLenMS  =  mGameHourLenMS * 24;
	 Platform::LocalTime lt;
     Platform::getLocalTime(lt);

	 mGameHour = 666; //reset to a bogus hour :P
	 mLastGameHourEventFired = 555;
	 mTimeOfGameDay = ((( lt.hour % mGameDayHours ) * 60  + lt.min ) * 60 + lt.sec ) * 1000;
	 mOverAllDelta  = 0;
	 mUseGameTime    = true; 
}


// ConsoleFunction (setGameHours,void,2,2,"Set the game hours for RealTimer and enable gametime usage.")
DefineEngineFunction( setGameHours, void, (U32 lHours), ,"Set the game hours for RealTimer and enable gametime usage.")
{
	gRealTimer->setGameHours(lHours);
}
//-----------------------------------------------------------------------------
// ConsoleFunction (getGameHour,S32,1,1,"get the game hours from RealTimer - if invalid it returns -1")
DefineEngineFunction( getGameHour, S32, (), ,"get the game hours from RealTimer - if invalid it returns -1")
{
	S32 hour = gRealTimer->getGameHour();
	if (hour == 666)
		return -1;
	else 
		return hour;
	
}
//-----------------------------------------------------------------------------
void RealTimer::setActive (bool lVal)
{
	mActive = lVal;
	if (!mActive) { //rest stuff for next run
		mUseGameTime = false;
		mSkipDelta = 10000;
		mRealTimeSynced = false;
        clearEvents();
		mLastGameHourEventFired = 555;
	}

}

/*
  DefineEngineFunction(calculateLevel, S32, (U32 level), , "calculateLevel - Return needed XP for given level!")

 */

// ConsoleFunction(realTimerStartMissionNotify, void, 1, 1, "realTimerStartMissionNotify()")
 DefineEngineFunction(realTimerStartMissionNotify, void, (), , "realTimerStartMissionNotify()")
{
	gRealTimer->setActive(true);
}

// ConsoleFunction(realTimerEndMissionNotify, void, 1, 1, "serverTimerEndMissionNotify()")
DefineEngineFunction(realTimerEndMissionNotify, void, (), , "serverTimerEndMissionNotify()")
{
	gRealTimer->setActive(false);
}

// ConsoleFunction(realTimerActive, bool, 1, 1, "is it active - also usefull to find out if game is running")
DefineEngineFunction(realTimerActive, bool, (), , "is it active - also usefull to find out if game is running")
{
	return gRealTimer->isActive();
}
//-----------------------------------------------------------------------------
static const char *getUnit(const char *string, U32 index, const char *set)
{
   U32 sz;
   while(index--)
   {
      if(!*string)
         return "";
      sz = dStrcspn(string, set);
      if (string[sz] == 0)
         return "";
      string += (sz + 1);
   }
   sz = dStrcspn(string, set);
   if (sz == 0)
      return "";
   char *ret = Con::getReturnBuffer(sz+1);
   dStrncpy(ret, string, sz);
   ret[sz] = '\0';
   return ret;
}
//-----------------------------------------------------------------------------
// ConsoleFunction(RealTimerPrintEvents, void, 1,1,"Print current Events")
DefineEngineFunction (RealTimerPrintEvents, void, (), ,"Print current Events")
{
	gRealTimer->printEvents();
}
//-----------------------------------------------------------------------------
// ConsoleFunction(RealTimerDeleteEvent, void, 2,2,"ID")
DefineEngineFunction(RealTimerDeleteEvent, void, (S32 lId), ,"ID")
{
	gRealTimer->removeEvent(lId);
}
//-----------------------------------------------------------------------------
//V2.22 changed from bool to Id (S32)

// ConsoleFunction
DefineEngineStringlyVariadicFunction(RealTimerSchedule, S32, 4, 0, "RealTimeSchedule(char* timedefinition, refobject|0, command, <arg1...argN>)"
				"timedefinitions blank separated string with: \"Min Hour DayOfMonth Month DayOfWeek OnlyOnce\" (-1 means ignored, 0 is sunday), return id of realtimeschedule")
{
   S32 lMin  = dAtoi(getUnit(argv[1], 0, " "));
   S32 lHour = dAtoi(getUnit(argv[1], 1, " "));
   S32 lDoM  = dAtoi(getUnit(argv[1], 2, " "));
   S32 lMon  = dAtoi(getUnit(argv[1], 3, " "));
   S32 lDoW  = dAtoi(getUnit(argv[1], 4, " "));
   bool llDeleteAfterExecuted = dAtob(getUnit(argv[1], 5, " "));

   bool lOnObject = false;
   SimObject *lObject = Sim::findObject(argv[2]);
   if(!lObject)
   {
      if(argv[2][0] != '0')
         return false;

      lObject = Sim::getRootGroup();
	  lOnObject = false;
   } else {
     lOnObject = true;
   }

   const RealTimeEvent evt = RealTimeEvent(argc - 3, argv + 3, lOnObject, lObject, llDeleteAfterExecuted
                                           ,lMin, lHour,lDoM, lMon, lDoW);


   gRealTimer->addEvent(evt);
   return gRealTimer->getEventCount() - 1; //should be the Id
}
#endif //FIXME MAKE IT BETTER TO COMPILE !!!
