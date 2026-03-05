//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
//
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

function onServerCreated()
{
   // Server::GameType is sent to the master server.
   // This variable should uniquely identify your game and/or mod.
   $Server::GameType = $appName;

   // Server::MissionType sent to the master server.  Clients can
   // filter servers based on mission type.
   $Server::MissionType = "Deathmatch";

   // GameStartTime is the sim time the game started. Used to calculated
   // game elapsed time.
   $Game::StartTime = 0;

   // Create the server physics world.
   physicsInitWorld( "server" );

   // Load up any objects or datablocks saved to the editor managed scripts
   %datablockFiles = new ArrayObject();
   %datablockFiles.add( $defaultGame @ "/art/ribbons/ribbonExec.cs" );   
   %datablockFiles.add( $defaultGame @ "/art/particles/managedParticleData.cs" );
   %datablockFiles.add( $defaultGame @ "/art/particles/managedParticleEmitterData.cs" );
   %datablockFiles.add( $defaultGame @ "/art/decals/managedDecalData.cs" );
   %datablockFiles.add( $defaultGame @ "/art/datablocks/managedDatablocks.cs" );
   %datablockFiles.add( $defaultGame @ "/art/forest/managedItemData.cs" );
   %datablockFiles.add( $defaultGame @ "/art/datablocks/datablockExec.cs" );   
   loadDatablockFiles( %datablockFiles, true );

   // Run the other gameplay scripts in this folder
   exec("./scriptExec.cs");

/*$Server::MissionFile NOT SET YET ?!?!?!?

*/  
   // Keep track of when the game started
   $Game::StartTime = $Sim::Time;
   
}
//-----------------------------------------------------------------------------
function onServerDestroyed()
{
   // This function is called as part of a server shutdown.

   physicsDestroyWorld( "server" );

   if (isFunction("OnMissionCleanup")) {
      OnMissionCleanup();
   }
}

//-----------------------------------------------------------------------------
function startGame()
{
   
  %missionScript = "./missions/" @ fileBase($Server::MissionFile) @ ".cs";
  if (exec(%missionScript) == 0) {
     dError("********************* FAILED TO LOAD MISSIONSCRIPT" SPC %missionScript);
     exec("./missions/all.cs");
  } else {
     dError("*******L O A D E D ************** MISSIONSCRIPT" SPC %missionScript);
  }
  
   // This is where the game play should start
   // The default onMissionLoaded function starts the game.

   if (isFunction("OnMissionStart")) {
      OnMissionStart();
   }
   
   
}

function endGame()
{
   // This is where the game play should end
   // The default onMissionEnded function shuts down the game.
   
   if (isFunction("OnMissionStart")) {
      OnMissionEnd();
   }
   
}

//-----------------------------------------------------------------------------
// GameConnection Methods
// These methods are extensions to the GameConnection class. Extending
// GameConnection makes it easier to deal with some of this functionality,
// but these could also be implemented as stand-alone functions.
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
   
   if (isObject(%this.player))
   {
      error("Attempting to create a player for a client that already has one!");
   }


  %player = new AIEntity("pl_" @ %this.nameBase) {
//   %player = new Player("pl_" @ %this.nameBase) {
       dataBlock = $Game::DefaultPlayerDataBlock;
       client = %this;
    };
    %player.setShapeName(%this.nameBase);
    %player.setStatic(false);
    %player.setspeedmodifier(25);
    %player.isHumanControlled=true;
    
    
      
    %this.setfirstperson(0);
    %this.player = %player;
    %player.client = %this;

    if (%this.SpawnPoint $="") {
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
   
  if ($GLOBALS::useAFXCamera) {
    %this.camera.setCameraSubject(%player);
    %this.camera.setThirdPersonMode();
    //OLD STYLE: %this.camera.setThirdPersonOffset("0 -3 3");
    %this.camera.setThirdPersonOffset("0 0 3"); //("0 -3 3");
    %this.camera.setThirdPersonDistance(3); // this is default 3rd-person distance    
    %this.camera.setThirdPersonSnap();
    %this.setCameraObject(%this.camera);
    
    
    
   }
   
   
   return %player;


}
//-----------------------------------------------------------------------------
function GameConnection::onClientEnterGame(%this)
{
   
    %this.setName("cl_" @ %this.nameBase);   
   
   
   commandToClient(%this, 'SyncClock', $Sim::Time - $Game::StartTime);
   %cameraSpawnPoint = pickCameraSpawnPoint($Game::DefaultCameraSpawnGroups);
   %this.spawnCamera(%cameraSpawnPoint);
   // Spawn a camera for this client using the found %spawnPoint
   %player = %this.spawnPlayer( %playerSpawnPoint);

   /* wrong place here ?!?!?
  if ($GLOBALS::useAFXCamera) {
    %this.camera.setCameraSubject(%player);
    %this.camera.setThirdPersonMode();
    %this.camera.setThirdPersonOffset("0 -3 3");
    %this.camera.setThirdPersonSnap();
    %this.setCameraObject(%this.camera);
   }
  */
      

   // Inform the client of all the other clients
   //FIXME
}

function GameConnection::onClientLeaveGame(%this)
{
//fixme save game 
  %this.player.schedule(1000, "delete");

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

