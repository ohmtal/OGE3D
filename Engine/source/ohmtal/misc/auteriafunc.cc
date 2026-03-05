//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
#include "console/console.h"
#include "console/console.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "math/mPoint3.h"
#include "math/mMatrix.h"
#include "math/mMathFn.h"
#include "T3D/gameBase/gameConnection.h"
#include "core/strings/stringFunctions.h"
#include "platform/types.h"
#include "materials/materialDefinition.h"
#include "platform/platformVolume.h"
#include "scene/sceneObject.h"

#include "ohmtal/ai/aiMath.h"

//------------------------------------------------------------------------------------------------
//added 2024-03-17
/*

want to use it like the old showtool,
but dont get it work and no time to dig deeper.

tested with:
~~
exec("tools/shapeEditor/scripts/shapeEditor.ed.cs");
echo(mountFilePath("../../../../SDK/auteria/sheepdog/data/shapes/animals/"));
ShapeEdSelectWindow.navigate("C:/torque/SDK/auteria/sheepdog/data/shapes/animals", true);
~~
ConsoleFunctionGroupBegin(FileSystemExt, "Add filesystem volume access.");
DefineEngineFunction(mountFilePath, bool, (const char* lFilePath), , "mount a path to the FileSystem")
{
   //Torque::FS::Mount( "game", Platform::FS::createNativeFS( ( const char* ) szPathCopy ) );
   // bool Mount(String root, const Path &path)
   return Torque::FS::Mount("game", lFilePath);
}

ConsoleFunctionGroupEnd(FileSystemExt);
*/
//--------------------------------------- XXTH ---------------------------------------------------------
ConsoleFunctionGroupBegin(DateFunctions, "Some small datafunctions.");

/*
   struct LocalTime
   {
      U8  sec;        // seconds after minute (0-59)
      U8  min;        // Minutes after hour (0-59)
      U8  hour;       // Hours after midnight (0-23)
      U8  month;      // Month (0-11; 0=january)
      U8  monthday;   // Day of the month (1-31)
      U8  weekday;    // Day of the week (0-6) XXTH NO 0=Sunday!
      U16 year;       // current year minus 1900
      U16 yearday;    // Day of year (0-365)
      bool isdst;     // true if daylight savings time is active
   };

*/
DefineEngineStringlyVariadicFunction(getLocalTimeEnhanced, const char*, 1, 2, "getLocalTime([bool enhancedInfo]) - Return datetime of the cmos. YEAR MONTH DAY HOUR MIN SEC WEEKDAY [DAYOFYEAR DST]")
{
   bool lEnhancedInfo = false; //since 1.98
   if (argc == 2)
      lEnhancedInfo = dAtob(argv[1]);
   if (lEnhancedInfo)
   {
      char* returnBuffer = Con::getReturnBuffer(30);
      Platform::LocalTime lt;
      Platform::getLocalTime(lt);
      dSprintf(returnBuffer, 30, "%04d %02d %02d %02d %02d %02d %d %d %d",
         lt.year + 1900,
         lt.month + 1,
         lt.monthday,
         lt.hour,
         lt.min,
         lt.sec,
         lt.weekday,
         lt.yearday,
         lt.isdst
      );
      return returnBuffer;
   }
   else {
      char* returnBuffer = Con::getReturnBuffer(22);
      Platform::LocalTime lt;
      Platform::getLocalTime(lt);
      dSprintf(returnBuffer, 22, "%04d %02d %02d %02d %02d %02d %d",
         lt.year + 1900,
         lt.month + 1,
         lt.monthday,
         lt.hour,
         lt.min,
         lt.sec,
         lt.weekday
      );
      return returnBuffer;
   }


}


/**
  * getIsoDateTime TGE 1.98!
  * get ISO DATE like 2015-05-21 10:10:10
  *
  */

DefineEngineStringlyVariadicFunction(getIsoDateTime, const char*, 1, 2, "getIsoDateTime() - Return the ISO Date of the cmos. Like 2015-05-21 10:10:10")
{
   char* returnBuffer = Con::getReturnBuffer(20);
   Platform::LocalTime lt;
   Platform::getLocalTime(lt);
   dSprintf(returnBuffer, 20, "%04d-%02d-%02d %02d:%02d:%02d",
      lt.year + 1900,
      lt.month + 1,
      lt.monthday,
      lt.hour,
      lt.min,
      lt.sec

   );

   return returnBuffer;
}

/**
  * getWeekDay since TGE 1.98!
  * 0 = as int, 1= as string, 2 as short String!
  *
  */

DefineEngineStringlyVariadicFunction(getWeekDay, const char*, 1, 2, "getWeekDay(int type) - Return the weekday type=> 0 = as int, 1= as string, 2 as short String! ")
{
   S32 lType = 0;
   if (argc == 2)
      lType = dAtoi(argv[1]);

   char* returnBuffer = Con::getReturnBuffer(30);
   Platform::LocalTime lt;
   Platform::getLocalTime(lt);

   if (lType == 2)
   {
      //shortWeekName SUN MON TUE WED THU FRI SAT
      switch (lt.weekday)
      {
      case 0:	dSprintf(returnBuffer, 30, "%s", "SUN"); break;
      case 1:	dSprintf(returnBuffer, 30, "%s", "MON"); break;
      case 2:	dSprintf(returnBuffer, 30, "%s", "TUE"); break; //2.10 fixed was THU *LOL* so much confusing 
      case 3:	dSprintf(returnBuffer, 30, "%s", "WED"); break;
      case 4:	dSprintf(returnBuffer, 30, "%s", "THU"); break;
      case 5:	dSprintf(returnBuffer, 30, "%s", "FRI"); break;
      case 6:	dSprintf(returnBuffer, 30, "%s", "SAT"); break;

      }
      return returnBuffer;
   }

   if (lType == 1)
   {
      //shortWeekName SUN MON TUE WED THU FRI SAT
      switch (lt.weekday)
      {
      case 0:	dSprintf(returnBuffer, 30, "%s", "Sunday"); break;
      case 1:	dSprintf(returnBuffer, 30, "%s", "Monday"); break;
      case 2:	dSprintf(returnBuffer, 30, "%s", "Tuesday"); break;
      case 3:	dSprintf(returnBuffer, 30, "%s", "Wednesday"); break;
      case 4:	dSprintf(returnBuffer, 30, "%s", "Thursday"); break;
      case 5:	dSprintf(returnBuffer, 30, "%s", "Friday"); break;
      case 6:	dSprintf(returnBuffer, 30, "%s", "Saturday"); break;

      }
      return returnBuffer;
   }


   dSprintf(returnBuffer, 30, "%d", lt.weekday);
   return returnBuffer;

}
/**
  * getIsoDate since TGE 1.98!
  * get ISO DATE like 2015-05-21
  *
  */

DefineEngineStringlyVariadicFunction(getIsoDate, const char*, 1, 1, "getIsoDate() - Return the ISO Date of the cmos. Like 2015-05-21")
{
   char* returnBuffer = Con::getReturnBuffer(11);
   Platform::LocalTime lt;
   Platform::getLocalTime(lt);
   dSprintf(returnBuffer, 11, "%04d-%02d-%02d",
      lt.year + 1900,
      lt.month + 1,
      lt.monthday
   );

   return returnBuffer;
}



DefineEngineStringlyVariadicFunction(getUnixTimeStamp, S32, 1, 1, "getUnixTimeStamp() - Return unixtimestamp in S32!")
{
   //char *returnBuffer = Con::getReturnBuffer( 20 );
   return Platform::getTime();
}

//msToTimeStr
#define M_MILISECONDSPERHOUR		(1000*60*60)
#define M_MILISECONDSPERMINUTE		(1000*60)
DefineEngineStringlyVariadicFunction(getMsToTimeStr, const char*, 3, 3, "convert ms to time string, params: MS, bool autoHour")
{
   U32 ms = dAtoi(argv[1]);
   bool autoHour = dAtob(argv[2]);
   char* returnBuffer = Con::getReturnBuffer(64);

   U32 sec, min, hour;
   hour = ms / M_MILISECONDSPERHOUR;
   min = (ms % M_MILISECONDSPERHOUR) / M_MILISECONDSPERMINUTE;
   sec = ((ms % M_MILISECONDSPERHOUR) % M_MILISECONDSPERMINUTE) / 1000;

   if (autoHour && hour == 0) {
      dSprintf(returnBuffer, 64, "%02d:%02d", min, sec);
   }
   else {
      dSprintf(returnBuffer, 64, "%02d:%02d:%02d", hour, min, sec);
   }

   return returnBuffer;
}

//XXTH added for debug stuff 2010-06-17
DefineEngineStringlyVariadicFunction(getVirtualMilliseconds, S32, 1, 1, "getVirtualMilliseconds() - Return VirtualMilliseconds in S32!")
{
   return Platform::getVirtualMilliseconds();
}


// getLocalTimeFormated
DefineEngineStringlyVariadicFunction(getLocalTimeFormated, const char*, 1, 2, "getLocalTimeFormated(iso=false) - Return datetime of the cmos. DAY/MONTH/YEAR HOUR:MIN:SEC")
{
   bool isoFormat = false;
   if (argc > 1)
      isoFormat = dAtob(argv[1]);

   char* returnBuffer = Con::getReturnBuffer(22);
   Platform::LocalTime lt;
   Platform::getLocalTime(lt);

   if (isoFormat)
   {
      dSprintf(returnBuffer, 22, "%04d-%02d-%02d %02d:%02d:%02d",
         lt.year + 1900,
         lt.month + 1,
         lt.monthday,
         lt.hour,
         lt.min,
         lt.sec
      );

   }
   else {
      dSprintf(returnBuffer, 22, "%02d/%02d/%04d %02d:%02d:%02d",
         lt.monthday,
         lt.month + 1,
         lt.year + 1900,
         lt.hour,
         lt.min,
         lt.sec
      );
   }


   return returnBuffer;
}



ConsoleFunctionGroupEnd(DateFunctions);

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

ConsoleFunctionGroupBegin(AuteriaFunctions, "Some small auteria functions.");


//--------------------------------------------------------------------------
DefineEngineFunction(calculateLevel, S32, (U32 level), , "calculateLevel - Return needed XP for given level!")
{

   
   U32 multi = mFloor((level - 1) / 3) + 2;
   U32 rmulti = 0;
   U32 j;
   if (level > 3) {
      for (j = 2; j < multi; j++) {
         rmulti += j * 3;
      }
   }
   rmulti++;
   if (level > 0) {
      U32 finmodulo = ((level + 2) % 3) + 1;
      rmulti += finmodulo * multi;
   }
   S32 result = rmulti * 200;
   return (result);

}
//--------------------------------------------------------------------------
//XXTH in my game i lost LocalClientConnection because i named the gameconnection so i need a consolefunction 
//     to give me the object for editor

DefineEngineFunction(getLocalClientConnection, S32, (), , "getLocalClientConnection - Return local connection object") {

   GameConnection* connection = GameConnection::getLocalClientConnection();
   S32 result = 0;
   if (connection)
      result = connection->getId();
   return(result);

}

//--------------------------------------------------------------------------
DefineEngineFunction(getCatmullrom, F32, (const char* args), , "(t p0 p1 p2 p3) return F32") {
   F32 t, p0, p1, p2, p3, result;

   dSscanf(args, "%g %g %g %g %g", &t, &p0, &p1, &p2, &p3);
   result = mCatmullrom(t, p0, p1, p2, p3);
   return result;
}
//--------------------------------------------------------------------------
//BIT's Bitte ein bit :P
DefineEngineFunction(BIT, S32, (S32 bit), , "get your bit.")
{
   S32 result = BIT(bit);
   return result;

}
//--------------------------------------------------------------------------
//CHR
// Returns a one-character string containing the character specified by ascii .
//chr its like chr in delphi or php
DefineEngineFunction(CHR, const char*, ( U8 value ), ,"string chr  ( int $ascii  ) Returns a one-character string containing the character specified by ascii .")
{


   char buf[2];
   buf[1] = 0;
//   buf[0] = (U8)dAtoi(argv[1]);
   buf[0] = value;
   S32 result = BIT(value);

   char* returnBuffer = Con::getReturnBuffer(2);
   dSprintf(returnBuffer, 2, "%s", buf);
   return returnBuffer;


}
//--------------------------------------------------------------------------
//ORD
// Returns a one-character string containing the character specified by ascii .
//chr its like chr in delphi or php
DefineEngineFunction(ORD, S32, (const char* str), , "int ord  ( string $string  ) Returns the ASCII value of the first character of string .")
{

   char c = str[0]; //dangerous ?!
   S32 result = (U8)c;
   return result;

}
//--------------------------------------------------------------------------
//mmhhh some 0 byte problems ... :(
/*
//PACKNUMBER
ConsoleFunction(packNumber, const char *, 3, 3, "char * ( S32,bool istwoByte ) Returns byte 'string' of number, bool istwoByte=S16, else default S32")
{
   U8 lLen = dAtob(argv[2]) ? 2: 4;
   U32 lNumber = dAtoi(argv[1]);
    char buf[5];
   dMemset(buf, 0, sizeof(buf));
   for (S32 i = 0; i < lLen; i++)
      buf[i] = (lNumber >> (i*8)) & 0xFF;

  char *returnBuffer = Con::getReturnBuffer( 5 );
  dSprintf(returnBuffer ,5, "%s",buf);
  Con::errorf("len:%d", dStrlen(returnBuffer));
  return returnBuffer;

}
//--------------------------------------------------------------------------
//UNPACKNUMBER
ConsoleFunction(unpackNumber, S32, 3, 3, "S32 ( char *,bool istwoByte ) Returns number of bytestring, bool istwoByte=S16, else default S32")
{
   U8 lLen = dAtob(argv[2]) ? 2: 4;
   U32 result = 0;
    char buf[5], buf2[5];
   dMemset(buf2, 0, sizeof(buf2));
   dSprintf(buf,sizeof(buf),"%s",argv[1]);

   //eeeps
   S32 lNeedle = lLen - dStrlen(buf);
    for (S32 i=lNeedle; i<lLen; i++)
      buf2[i] = buf[i];



   for (S32 i = 0; i < lLen; i++)
      result |= (buf2[i] << (i*8));

   return result;

}
*/
//--------------------------------------------------------------------------
//Give back the bits as string from a byte
DefineEngineFunction(numberToBits, const char*, (U32 number, U32 digits), , "numberToBits max 32 bits, Params: %number,%digits; Returns bin string.")
{

   if (digits > 32) {
      Con::errorf("Only 32 digits are allowed!!");
      return "";
   }

   char buf[33];
   dMemset(buf, '0', 33);
   buf[digits] = 0;

   S32 j = 0;
   for (S32 i = digits - 1; i >= 0; i--)
   {
      buf[j] = ((number >> i) & 1) ? '1' : '0';
      j++;
   }

   char* returnBuffer = Con::getReturnBuffer(33);
   dSprintf(returnBuffer, 33, "%s", buf);
   return returnBuffer;

}
//--------------------------------------------------------------------------
// rpg style rank based XP calculation:
DefineEngineFunction(calculateRankXP, S32, (U32 pr, U32 tr, U32 xp), ,
   "calculateRankXP - Return XP from rank calculation. Params: Params: PlayerRank,TargetRank,BaseXP!")
{
   if (pr == tr) return xp; //same rank basexp
   if (pr > tr) {
      if (pr > tr + 9) return 0; //no XP for easy opponents >+10 ranks
      return mCeil(xp * (1 - (pr - tr) * 0.05)); //reducing by 5% each rank
   }
   else { //tr > pr
      if (pr > tr - 5) { //target is max 4 levels higher give more XP
         return mCeil(xp * (1 + (tr - pr) * 0.0375));
      }
      else { //target is skull class!
         return mCeil(xp * (1.16 - (tr - pr - 4) * 0.65 / (tr - 5)));
      }
   }
}



//--------------------------------------------------------------------------
//Example $bla = sprintf("x=%1 y=%2",%x,%y);
DefineEngineStringlyVariadicFunction(sprintf, const char*, 1, 10, "sprintf(string,arg1,arg2...arg9);")
{
   argc;
   char* ret = Con::getReturnBuffer(1024);
   ret[0] = 0; //initialize and terminate the string
   char chr[2];
   const char* ptr = argv[1];
   while (*ptr != 0) //XXTH BSD ?? NULL)
   {
      if (*ptr == '%')
      {
         if (ptr[1] >= '1' && ptr[1] <= '9')
         {
            chr[0] = ptr[1];
            if ((dAtoi(chr) + 1) < argc)
            {
               dStrcat(ret, argv[dAtoi(chr) + 1],1024);
               ptr += 2; // skip the %n
               continue;
            }
         }
      }
      chr[0] = *ptr;
      chr[1] = 0;
      dStrcat(ret, chr,1024);
      ptr++;
   }
   return ret;
}
//--------------- handy terrain tools --------------------------------
DefineEngineFunction(getAbsoluteTerrainHeight, F32, (Point2F position2D, bool onServer), (true), "get the absolute terrain z at the given position")
{
   Point3F lPos = Point3F(position2D.x, position2D.y, 0.f);
   return AIMath::getTerrainHeight(lPos, NULL, onServer);
}

DefineEngineFunction(getTerrainNormal, Point3F, (Point2F position2D, bool onServer),(true), "get the absolute terrainnormal vector at the given position")
{
   Point3F lPos = Point3F(position2D.x, position2D.y, 0.f);
   return AIMath::getTerrainNormal(lPos);
}

DefineEngineFunction(getTerrainPoint, Point3F, (Point2F position2D, F32 zOffset,bool onServer), (0.f, true), "return the modified positon to terrain at the given position, alter Z bei offset (default 0.f)")
{
   Point3F lPos = Point3F(position2D.x, position2D.y, 0.f);
   Point3F lResult = AIMath::getTerrainPoint(lPos, onServer);
   lResult.z += zOffset;
   return lResult;
}

DefineEngineFunction(getTerrainMaterialName, const char *, (Point3F position, bool onServer), , "return the Terrain material on the given 3D point")
{
   
   Material* mat = AIMath::getTerrainMaterialAtpos(position, onServer);

   if (mat) {
      return mat->getName();

   }
   return "";
}



//--------------- some more AIMath --------------------------------
DefineEngineFunction(getDistance2D, F32, (Point3F from, Point3F to), , "Calculate the Distance by to point in 2D (x,y)")
{

   return AIMath::Distance2D(from, to);
}

DefineEngineFunction(getDistance3D, F32, (Point3F from, Point3F to), , "Calculate the Distance by to point")
{

  return AIMath::Distance3D(from, to);
}




DefineEngineFunction(RotateAroundOrigin, VectorF, (VectorF forwardVector, F32 radians), , "Example RotateAroundOrigin(%this.getForwardVector(), mDegToRad(90));")
{
   return AIMath::RotateAroundOrigin(forwardVector, radians);
}

DefineEngineFunction(M_PI, F32, (), , "Want my PI ;)")
{
   return M_PI;
}


DefineEngineFunction(boxempty, bool, (Point3F centerPosition, Point3F extent, bool clientOnly, U32 mask), (Point3F(1.f, 1.f, 1.f), false, 256), "Can i place someting there ? ")
{
   //U32 mask = StaticShapeObjectType ;
   //TGE:   U32 mask = StaticShapeObjectType  | StaticTSObjectType | InteriorObjectType;
   
   return AIMath::boxEmpty(centerPosition, extent, mask, clientOnly);
}



DefineEngineFunction(boxEmptyBox, bool, (Box3F box, bool clientOnly, U32 mask), , "Can i place someting there ? ")
{
   return AIMath::boxEmpty2(box, mask, clientOnly);
}

DefineEngineFunction(boxEmptyBoxIgnore, bool, (Box3F box, bool clientOnly, U32 mask, SceneObject* ignoreObject),, "Can i place someting there ? ")
{
   bool result = false;
   if (ignoreObject != NULL)
      ignoreObject->disableCollision();
   result = AIMath::boxEmpty2(box, mask, clientOnly);
   if (ignoreObject != NULL)
      ignoreObject->enableCollision();

   return result;
}

DefineEngineFunction(disableCollision, void, (SceneObject * obj), , "Disable collision for an object")
{
   obj->disableCollision();
}

DefineEngineFunction(enableCollision, void, (SceneObject* obj), , "Disable collision for an object")
{
   obj->enableCollision();
}


DefineEngineFunction(getPositionCloseToDestination, Point3F, (Point3F start, Point3F destination, F32 radius), , " a point radius meters before destination ")
{

   return AIMath::getPositionCloseToDestination(start, destination, radius);
}


DefineEngineFunction(getPositionCloseToObject, Point3F, (SceneObject* fromObj, SceneObject* toObj, F32 radius), (0.f), " a point radius meters before destination ")
{
   return AIMath::getPositionCloseToObject(fromObj, toObj, radius);
}

DefineEngineFunction(getPositionCloseToObjectOnTerrain, Point3F, (SceneObject* fromObj, SceneObject* toObj, F32 radius, bool onServer), (0.f, true), " a point radius meters before destination ")
{
    
   return AIMath::getTerrainPoint(  AIMath::getPositionCloseToObject(fromObj, toObj, radius), onServer );
}


DefineEngineFunction(getVariPoint, Point3F, (Point3F location, F32 tolerance, bool onServer), (2.f, true), " get a random 2D  point on terrain in the radius = tolerance arround location ")
{
   return AIMath::getTerrainPoint( AIMath::getVariPoint(location, tolerance), onServer);
}

DefineEngineFunction(getVariPoint2D, Point3F, (Point3F location, F32 tolerance), (2.f), " get a random 2D  point in the radius = tolerance arround location ")
{
  return  AIMath::getVariPoint(location, tolerance);
}


DefineEngineFunction(rotateObject, bool, (SceneObject* object, F32 degree), (90.f), "rotate an object on Z axis by degree(default 90) ")
{

   if (!object)
      return false;

   const MatrixF& tmat = object->getTransform();
   AngAxisF aa(tmat);

   Point3F pos;
   F32 oldDegree, newDegree;
   const F32 MaxDegree = 180.f; 
   tmat.getColumn(3, &pos);
   aa.axis = Point3F(0.f, 0.f, 1.f);
   oldDegree = aa.angle / M_RAD;
   newDegree = mFmod((oldDegree + degree), MaxDegree);
   aa.angle = newDegree * M_RAD;

   Con::printf("rotateObject by %.0f old:%.0f new:%.0f  aa.angle:%.2f", degree, oldDegree, newDegree, aa.angle);

   MatrixF mat;
   aa.setMatrix(&mat);
   mat.setColumn(3, pos);
   object->setTransform(mat);

   return true;

}



ConsoleFunctionGroupEnd(AuteriaFunctions);
