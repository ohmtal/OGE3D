exec("core/ohmtal/shared/smtp.cs");

/*
exec("basicgame/server/scripts/tools.cs");
*/
//==============================================================================
//  Syslog 
//  %priority : DEBUG WARNING ALERT NOTICE
//==============================================================================
function Syslog(%priority,%function,%msg) {


   if (%priority $= "ALERT") {
     error("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ !!!!!!!!!!!!!!! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
     error("SYSLOG:" @ %priority @ ":" @ %function @ ":" @ %msg);
     error("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ !!!!!!!!!!!!!!! ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
     sendmail($pref::smtp::from,$pref::smtp::to,"AUTERIA Alert in function" SPC %function, %msg);
     
   } else {
     echo("SYSLOG:" @ %priority @ ":" @ %function @ ":" @ %msg);
   }

}
//==============================================================================
// pickSpawnPoint()  
// enhanced version from: http://www.garagegames.com/index.php?sec=mg&mod=resource&page=view&qid=2347
// 2.10 added player for distance check to get the closest point!
//      client is used for overwrite on gornheim from GameConnection::spawnPlayer
//==============================================================================
function pickSpawnPoint(%player, %client)   
{
// error ("ENTER pickSpawnPoint!! "); 

	%groupName = "MissionGroup/PlayerDropPoints";
	%group = nameToID(%groupName);

	if (%group != -1)
	{
		%count = %group.getCount();
		if (%count != 0)
		{
         if (isObject(%player))
         {
            %curDist = 99999;
            %curPos  = %player.getPosition();
            %curRad  = 10;
            for (%i=0; %i < %count; %i++)
            {
               %spawn = %group.getObject(%index);
               %dist = shapebase::getDistanceFrom(%player, %spawn);
               if (%dist < %curDist)
               {
                  %curDist = %dist;
                  %curPos  = %spawn.position;
                  %curRad  = %spawn.radius;
               }
            }
            //we should have the closest spawnpoint :P
            return getTerrainLevel(%curPos, %curRad , 0.5, false, true);
            
         } else { //get a random spawnpoint?!
            %index = getRandom(%count-1);
            %spawn = %group.getObject(%index);
   
            return getTerrainLevel(%spawn.position,%spawn.radius, 0.5, false, true);
         }
		}
		else
			error("No spawn points found in " @ %groupName);
	}
	else
		error("Missing spawn points group " @ %groupName);

	// Could be no spawn points, in which case we'll stick the
	// player at the center of the world.
	return "0 0 300 1 0 0 0";
}
//==============================================================================
//  getTerrainLevel for spawnpoint selection
// if checkinside terrainheight will be ignored!
//==============================================================================
function getTerrainLevel(%pos,%rad, %addZ, %allcheck, %checkinside)
{
    
       %x = getWord(%pos,0);
       %y = getWord(%pos,1);
       %z = getWord(%pos,2);
       
//FIXME OGE3D have no isPointInsideSafe so far
%checkInside = false;       
       
       if (%checkInside ) {
           %isInside = isPointInsideSafe(%x SPC %y SPC %z);
           if (%isInside < 0) {
              error("getTerrainLevel :: RETURN THE GIVEN POSITION! BECAUSE isPointInside failed and all other actions are to dangerous!");
              return (%pos);
           }   
       }

       %retries = 0;
	while(%retries < 500)
	{
        if (%allcheck == true ) {
		     %mask = ($TypeMasks::PlayerObjectType | $TypeMasks::VehicleObjectType 
                   | $TypeMasks::MoveableObjectType | $TypeMasks::StaticShapeObjectType 
                   | $TypeMasks::ForceFieldObjectType //OGE3D|  $TypeMasks::InteriorObjectType 
                   //quark °° | $TypeMasks::TerrainLikeObjectType | $TypeMasks::InteriorLikeObjectType
                   | $TypeMasks::ItemObjectType | $TypeMasks::StaticTSObjectType);
        } else { 
		     %mask = ($TypeMasks::PlayerObjectType | $TypeMasks::VehicleObjectType 
                   | $TypeMasks::MoveableObjectType | $TypeMasks::StaticShapeObjectType 
                   | $TypeMasks::StaticTSObjectType);
        }
        
        %x = getWord(%pos, 0);
        %y = getWord(%pos, 1); 
        if (%rad > 0 ) {
           %x += mFloor(getRandom(%rad * 2) - %rad);
           %y += mFloor(getRandom(%rad * 2) - %rad);
        }
                
        %mayOk = true;
        if (%checkinside && %isInside)  {
            %mayOk = isPointInsideSafe(%x SPC %y SPC %z) == 1;
            %z = getWord(%pos, 2);
        } else if (getTerrainUnderWorldPoint(%x SPC %y SPC %z)) {
            %z = getTerrainHeight(  %x SPC %y );
       } else {
          %rayStart = %x SPC %y SPC (%z + 100);
          %rayEnd   = %x SPC %y SPC (%z - 100);
          
          %searchResult = containerRayCast(%rayStart, %rayEnd, 
                        $TypeMasks::TerrainLikeObjectType 
                        | $TypeMasks::InteriorLikeObjectType
                        | $TypeMasks::TerrainObjectType 
                        );
          if (%searchResult) {
             %z = getWord(%searchResult,3);
        
          }
       }
		 %z += %addZ;
		
		 %position = %x SPC %y SPC %z;

      //echo("check ContainerBoxEmpty(" SPC %mask SPC %position SPC 0.9 SPC 0.9 SPC 2 SPC ")) MAYOK=" SPC %mayOk );

		 if (%mayOk   && ContainerBoxEmpty(%mask,%position,0.9,0.9,2)) //pre 2.10 && ContainerBoxEmpty(%mask,%position,0.7))
		 {
//error("getTerrainLevel Return Position: " SPC  %position SPC "from" SPC %pos SPC "tries:" SPC %retries);                
			return %position;
       } else {
		   %retries++;
         switch (%retries) {//increase the rad a little
                case   1: %rad+=2;
                case 100: %rad+=2;
                case 200: %rad+=5;
                case 300: %rad+=10;
                case 400: %rad+=13;
         } // switch  
      }
	}
   error("getTerrainLevel :: 500 retries and no usable position !!!! RETURN THE GIVEN POSITION! All other actions are to dangerous!");
	return %pos;
}  

//==============================================================================
//  chooseReSpawnposition for respawnpoint selection ( login at last position )
//  return pos if position not usable. but must be used since its not checked!
//==============================================================================
function chooseReSpawnposition(%pos)
{
	%x = getWord(%pos, 0);
	%y = getWord(%pos, 1);
        %z = getWord(%pos, 2);
        %rad=0; 
        //1.94.1 %zrad=5;
        
        %isinside = isPointInsideSafe(%pos);
        
        //1.95:
        if (%isinside < 0 ) { //damn it failed to get hitface i guess
           error("chooseReSpawnposition :: RETURN THE GIVEN POSITION! BECAUSE isPointInside failed and all other actions are to dangerous!");
           return (%pos);
        }
        
        
        //1.94.1 need to raise z because of interior !
        // !!!! 1.95 remove this again when using ispointinsidesafe !!!!
        /*
        if (!%isInSide) { 
           %z += 0.5;
        }
        */
        
        %retries = 0;
	while(%retries < 500)
	{
       if (!%isInSide) { //dont check terrainheight inside!
          %tz=getTerrainHeight(%x SPC %y);
          //1.94.1 changed !! if (%tz+1 > %z) %z=%tz+2;
          if (%tz > %z) %z=%tz;
       }                   
       %position = %x @ " " @ %y @ " " @ %z;
       //1.97 not check for players! $TypeMasks::PlayerObjectType  - I need this for new stuck command
       // should not be a problem since harvest stuff use getterrainlevel
       %mask = ( $TypeMasks::VehicleObjectType | 
                $TypeMasks::MoveableObjectType | $TypeMasks::StaticShapeObjectType | $TypeMasks::StaticTSObjectType);

       // empty and inside if was inside!
       %newIsInside = isPointInsideSafe(%x SPC %y SPC %z);
                
                
		if (( (%isinside == 0 && %newIsInside == 0 ) || (%isinside == 1 && %newIsInside == 1) ) 
           && ContainerBoxEmpty(%mask,%position,0.7)) 
		{
// error("Return Position: " SPC  %position SPC "tries:" SPC %retries);                
			return %position;
		} else {
         %x = getWord(%pos, 0) + mFloor(getRandom(%rad * 2) - %rad);
		   %y = getWord(%pos, 1) + mFloor(getRandom(%rad * 2) - %rad);
         //1.94.1 ! changed to if %z = getWord(%pos, 2) + mFloor(getRandom(%zrad));
         %retries++;
         switch (%retries) {//increase the rad a little
            case   1: %rad+=2;
            case 100: %rad+=2;
            case 200: %rad+=5;
            case 300: %rad+=10;
            case 400: %rad+=13;
         }  
       }
	}
        
  error("chooseReSpawnposition :: 500 retries and no usable position !!!! RETURN THE GIVEN POSITION! All other actions are to dangerous!");
	return %pos;
}  



//------------------------------------------------------------------------------

function removeBlanks(%string) {
   return strreplace(%string,"%20"," ");
} 

function AddBlanks(%string) {
  return strreplace(%string," ","%20");
}

//------------------------------------------------------------------------------
// calculateLevel

// NOW c++ console FUNCTION MUCH FASTER !!!

// returns the score needed for next level.
// STABLE UNTIL RANK 2005! USE _OLD IF YOU NEED BIGGER VALUES!
// OLD IS FASTER UNTIL LEVEL 13

function calculateLevel_script(%level) {


//  if (%level < 13 || %level > 2004) 
//        return(calculateLevel_OLD(%level));
        
  %multi = mfloor((%level-1)/3)+2;
  %rmulti = 0;  
  if (%level>3) {
    for (%j=2 ; %j<%multi; %j++) {
                %rmulti += %j*3;
    }
  }
  %rmulti++;
  if (%level>0) {
         %finmodulo = ((%level+2) % 3)+1;
         %rmulti += %finmodulo*%multi;
  }
  %result = %rmulti * 200; 
  return (%result);
  
}
//-----------
// ::: OLD :::
function calculateLevel_OLD(%level) {
  %needed=0;
  %add = 200;
  for (%i = 0; %i<=%level; %i++) {
    if (%i % 3 == 1 ) %add += 200;
    %needed+=%add;
  }
//    echo ("XP for level " @ %level @ " = " @ %needed );
  return %needed;
  
}

//------------------------------------------------------------------------------
// bounce an object to col with impulse like 10 - its like magnetism
function Tools::bounceTo(%obj,  %col , %impulse){
   %dif = vectorSub(%col.getWorldBoxCenter(),getWords(%obj.getWorldBoxCenter(),0,2));
   %nDif = vectorNormalize(%dif);
   %sDif = vectorScale(%nDif, %impulse);
   %vel = getwords(%sDif, 0, 1) SPC getWord(%sDif, 2) + 3.75;
   %obj.setVelocity(%vel); 
}

// bounce an object from col with impulse like 10
function Tools::bounceFrom(%obj,  %col , %impulse){
   Tools::BounceTo(%obj,%col,%impulse * -1);
}
//------------------------------------------------------------------------------
function Tools::pushPlayerBack(%victim,  %attacker)
{
  // the push back is relative to the attacker
  // a straight push back would be along the attackers
  // Y axis....

  // right now we always push the victim at his center
  // we could explore what happnes if we push at the
  // point of contact instead (might turn or do something intersting)

  // get the usual direction to push...we could get the Y axis of
  // the attacker with getTransform() then grabbing the rotation part
  // and passing that to VectorOrthoBasis() and then using column 1
  // whichi would be words 3,4,5 (couting from 0)...but that's overkill
  // for something that can be approximated pretty good by a line drawn
  // from attacker to victim...so let's use that instead
  %vpos = %victim.getWorldBoxCenter();
  %pushDirection = VectorSub(%vpos,%attacker.getWorldBoxCenter());
  %pushDirection = VectorNormalize(%pushDirection);

  
  %impulse = getRandom(2,6);//was  2,15;

  // ok apply impulse to victim's center
  %mass = %victim.getDataBlock().mass;
  %pushVec = VectorScale(%pushDirection,%impulse * %mass);

// error("Applying, to player " @ %victim @ " of mass " @ %mass @ ", an impulseVec: " @ %pushVec);

  %victim.applyImpulse(%vpos, %pushVec);
}

//------------------------------------------------------------------------------
function Tools::ClearDynFields(%obj, %ignorefield) {
       %cnt=%obj.getDynamicFieldCount();
       %lst="";
       
       for (%i=0;%i<%cnt;%i++) {
        %fld= getField(%obj.getDynamicField(%i),0);
        if (%fld !$= %ignorefield) {
           if (%lst !$="") {
             %lst=%lst SPC getField(%fld,0);
           } else {
              %lst=getField(%fld,0);
           }
        }   
       } //for
       for (%i=0;%i<getwordcount(%lst);%i++) {
         %cmd=%obj.getid() @ "." @ getword(%lst,%i)@"=\"\";";
         eval(%cmd);
       }
 
}

//------------------------------------------------------------------------------
function Tools::Round(%num){
 if(%num - mFloor(%num) < 0.5)
  return mFloor(%num);
 else
  return mCeil(%num);
}



//------------------------------------------------------------------------------
// Loot stuff
//------------------------------------------------------------------------------
/*
   Tools::GetLOOTFROMString
   @param string %str
   @return string %stuff 

   Like monster drop but ignore Quests since we have no player object
   %str like : "SoilItem 10 1 3;WoodItem 10 4 10;MagicPowderItem 10 1 5;TergratiaTeleItem 100 1 1;SugarItem 3 1 4"
   %stuff like "SoilItem 3;WoodItem 4"
 */
function Tools::GetLOOTFROMString(%str)
{
  %tmpstr=%str;
  while( "" !$= %tmpstr )  {
     %tmpstr = nextToken( %tmpstr , "line" , ";" );
	  %line=trim(%line);
	  %name = getword( %line,0);
	  %chance = getword( %line,1);
     %randchance = %chance*1-1; 
     %min = getword( %line,2);
     %max = getword( %line,3);
     %qty = getRandom(%max-%min) + %min;
     if (%qty > 0 && %randchance >=0) {
        %droprand=getRandom(%randchance);
         if (%droprand == 0 ) {
             if (%stuff !$= "") {
                %stuff = %stuff @ ";";
             }
             %stuff = %stuff @ %name SPC %qty;  
         }
     }
  } //while
  return %stuff;
}
//------------------------------------------------------------------------------
/*
   Special Loot only Pick One Item for RandomQuestLoot
   ALL BLANK SEPARATED AND MUST MOD 3 NO EXTRA BLANKS!! 
   %str like : "SoilItem 1 3 WoodItem 4 10 MagicPowderItem 1 5 TergratiaTeleItem 1 1 SugarItem 1 4"
   %stuff like "SoilItem 3"
   
   randomreward:SoilItem 1 3 WoodItem 4 10 MagicPowderItem 1 5 TergratiaTeleItem 1 1 SugarItem 1 4
   
   TEST: 
    echo(Tools::GetOneRandomLoot("SoilItem 1 3 WoodItem 4 10 MagicPowderItem 1 5 TergratiaTeleItem 1 1 SugarItem 1 4"));
*/
function Tools::GetOneRandomLoot(%str)
{
  %count   = getWordCount(%str) / 3 ;
  %itemIdx = getRandom(0,%count-1) * 3;
  %item = getWord(%str,%itemIdx);
  %min  = getWord(%str,%itemIdx + 1);
  %max  = getWord(%str,%itemIdx + 2);
  %qty = getRandom(%max-%min) + %min;
  %stuff = %item SPC %qty;  

  return %stuff;
}







