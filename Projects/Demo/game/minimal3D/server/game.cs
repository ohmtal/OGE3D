//-----------------------------------------------------------------------------
// Server game.cs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

function onServerCreated()
{
   // Server::GameType is sent to the master server.
   // This variable should uniquely identify your game and/or mod.
   $Server::GameType = $appName;

   // Server::MissionType sent to the master server.  Clients can
   // filter servers based on mission type.
   $Server::MissionType = "MyMission";

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
     dWarn("********************* FAILED TO LOAD MISSIONSCRIPT" SPC %missionScript);
     exec("./missions/all.cs");
  } else {
     dWarn("*******L O A D E D ************** MISSIONSCRIPT" SPC %missionScript);
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
   
   if (isFunction("OnMissionEnd")) {
      OnMissionEnd();
   }
   
}


