echo("server/clients.cs FIXME MINIMAL!!"); 

//-----------------------------------------------------------------------------
// GameConnection Methods
// These methods are extensions to the GameConnection class. Extending
// GameConnection makes it easier to deal with some of this functionality,
// but these could also be implemented as stand-alone functions.
//-----------------------------------------------------------------------------
/*
exec("minimal3d/server/clients.cs");

*/
//-----------------------------------------------------------------------------
$Game::DefaultPlayerDataBlock = DefaultPlayerData;
//-----------------------------------------------------------------------------


   function GameConnection::onConnect(%this, %name)
   {
      // Send down the connection error info, the client is responsible for
      // displaying this message if a connection error occurs.
      messageClient(%this, 'MsgConnectionError',"",$Pref::Server::ConnectionError);

      // Send mission information to the client
      sendLoadInfoToClient(%this);

      // Simulated client lag for testing...
      // %this.setSimulatedNetParams(0.1, 30);

      // Get the client's unique id:
      // %authInfo = %this.getAuthInfo();
      // %this.guid = getField(%authInfo, 3);
      %this.guid = 0;
      addToServerGuidList(%this.guid);

      dError("GameConnection::onConnect " SPC %this.getId() SPC %name);
      %this.nameBase = %name;

      // Set admin status
      if (%this.getAddress() $= "local")
      {
         %this.isAdmin = true;
         %this.isSuperAdmin = true;
      }
      else
      {
         %this.isAdmin = false;
         %this.isSuperAdmin = false;
      }

      //
      echo("CADD: "@ %this @" "@ %this.getAddress());

      // If the mission is running, go ahead download it to the client
      if ($missionRunning)
      {
         %this.loadMission();
      }
      else if ($Server::LoadFailMsg !$= "")
      {
         messageClient(%this, 'MsgLoadFailed', $Server::LoadFailMsg);
      }
      $Server::PlayerCount++;
   }
//-----------------------------------------------------------------------------

function GameConnection::spawnPlayer(%this, %spawnPoint)
{
   
   error("FIXME MINIMAL!!!");

   if (isObject(%this.player))
   {
      error("Attempting to create a player for a client that already has one!");
      return %this.player;
   }

   
  // we use also the ai class here and set isAiControlled
    %player = new aiEntity("pl_" @ %this.nameBase) {
//   %player = new LiteUnit("pl_" @ %this.nameBase) {
//   %player = new Player("pl_" @ %this.nameBase) {
       dataBlock = $Game::DefaultPlayerDataBlock; 
       client = %this;
//       class = Player;
       isAiControlled = false;
    };
    %player.setShapeName(%this.nameBase);
    %player.setSkinName("red");
    
    if (%player.getClassName() $= "AIEntity") {
        %player.setStatic(false);
        %player.setspeedmodifier(25);
        %player.isHumanControlled=true;
    }
    
    
      
    %this.setfirstperson(0);
    %this.player = %player;
    %player.client = %this;

    if (%this.SpawnPoint !$="") {
       %spawnPoint= %this.SpawnPoint;
    } else  if (%spawnPoint $= "") {
      %spawnPoint = pickSpawnPoint(%player,%this);
    }
    
    %player.setTransform(%spawnPoint);
    %player.setSpawnPoint(%spawnPoint); //FOR UNDER TERRAIN CHECK
    %this.SpawnPoint = %spawnPoint; //setup for instances and so on !! 

    if (!isObject(%player))
    {
      %this.spawnCamera(%spawnPoint);
      return;
   }
   if (isObject(%this.camera))
   {
      if (%player.getClassname() $= "Player"  || %player.getClassname() $= "AIEntity")
         %this.camera.setTransform(%player.getEyeTransform());
      else
         %this.camera.setTransform(%player.getTransform());
   }
   MissionCleanup.add(%player);

   
   %this.setControlObject(%player);
   
   
   //give some buffs:
   %player.critRand        = 5;    // 15;    //lower is better min:0 = everything crit! 
   %player.critMultiFrom   = 8;    //2.5;   // normal damage is multiplied by min x when crit
   %player.critMultiTo     = 14;   //4.0;   // normal damage is multiplied by max x when crit
   %player.normMultiFrom   = 2.5; //0.75; // normal damage is multiplied by min x
   %player.normMultiTo     = 3.0; // 1.25; // normal damage is multiplied by max x 
   %player.armorPercent    = 75; //75% less damage!
   
   
   
   return %player;


}

//-----------------------------------------------------------------------------
function GameConnection::onClientEnterGame(%this)
{
   
   //we did unload every thing come back later ... 
   /* not working!
   if (!isObject(MissionCleanup))
   {
      loadMission( $Server::MissionFile );   
      %this.schedule(1000,onClientEnterGame);
      return;
   }
   */
   
    %this.setName("cl_" @ %this.nameBase);   
   
   
   commandToClient(%this, 'SyncClock', $Sim::Time - $Game::StartTime);
//   %cameraSpawnPoint = pickCameraSpawnPoint($Game::DefaultCameraSpawnGroups);

  if (isObject( BaseMarker1))
  {
      %cameraSpawnPointObject = BaseMarker1;
  } else {
     //fixme sanity 
      %cameraSpawnPointObject = PlayerDropPoints.getObject(0);
  }

    
   %cameraSpawnPoint = %cameraSpawnPointObject.getPosition();
   %tmpZ = getWord(%cameraSpawnPoint,2) + 50;
    
   
   %cameraSpawnPoint = setWord(%cameraSpawnPoint, 2, %tmpZ);

   %this.spawnCamera(%cameraSpawnPointObject);
   %this.camera.setPosition(%cameraSpawnPoint);
   
   // Spawn a camera for this client using the found %spawnPoint
   // not for wtp %player = %this.spawnPlayer( );

   //setup camera mode RTS like 
   //%this.camera.controlMode = "Overhead";
   %this.camera.controlMode = "RTS";
   %this.camera.setRotation("0.8 0 0");
   
   %player = %this.spawnPlayer( );
   
   // Inform the client of all the other clients
   //FIXME
}

function GameConnection::onClientLeaveGame(%this)
{
//fixme save game 
  if (isObject(%this.player)) {
      dEcho("onClientLeaveGame Player object delayed delete!!" SPC %this.player.getId());
      %this.player.schedule(1000, "delete"); 
  } else {
      dError("onClientLeaveGame Player object is not available for delayed delete!!");
  }

  //if server is empty we unload everything...
  // lol that sucks ..
  // btw playercount is 1 here when the last leave!!
  /*
   if( $Server::PlayerCount == 1 && $Server::Dedicated)
      schedule(0, 0, "endMission");
   */

}



//-----------------------------------------------------------------------------

function GameConnection::onLeaveMissionArea(%this)
{
   // The control objects invoke this method when they
   // move out of the mission area.

   messageClient(%this, 'MsgClientJoin', '\c2Now leaving the mission area!');
}

function GameConnection::onEnterMissionArea(%this)
{
   // The control objects invoke this method when they
   // move back into the mission area.

   messageClient(%this, 'MsgClientJoin', '\c2Now entering the mission area.');
}

//-----------------------------------------------------------------------------

function GameConnection::onDeath(%this, %sourceObject, %sourceClient, %damageType, %damLoc)
{
   game.onDeath(%this, %sourceObject, %sourceClient, %damageType, %damLoc);
}

