//-----------------------------------------------------------------------------
// Torque Game Engine 
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
        $Game::Company = "Ohmtal Game Studio";
        $Game::Caption = "Minimal3D";
        $Game::Icon    = "OhmtalGSIcon48.png"; 

        $Game::VersionString = "Minimal3D";
        $Game::VersionNumber = 1; 

        //where are my levels ? !! with trailing slash !!
        //used in createAndConnectToLocalServer( "SinglePlayer",  "empty.mis" );
        $Game::levelPath  = $defaultGame @ "/levels/";

        
        //if false console is disabled on release build
        $ALPHAVERSION = true;



//-----------------------------------------------------------------------------
// Load up common script base
   loadDir("core");
//-----------------------------------------------------------------------------
// Package overrides to initialize the mod.
package game {

function displayHelp() {
   Parent::displayHelp();
   error(
      "Options:\n"@
      "  -dedicated             Start as dedicated server\n"@
      "  -connect <address>     For non-dedicated: Connect to a game at <address>\n" @
      "  -mission <filename>    For dedicated: Load the mission\n"
   );
}

function parseArgs()
{
   // Call the parent
   Parent::parseArgs();

echo("~~~~~~~~~~~~~~~~~~~~~~ parseArgs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");    
   
   // Arguments, which override everything else.
   for (%i = 1; %i < $Game::argc ; %i++)
   {
      %arg = $Game::argv[%i];
      %nextArg = $Game::argv[%i+1];
      %hasNextArg = $Game::argc - %i > 1;
   
      switch$ (%arg)
      {
         //--------------------
         case "-dedicated":
            $Server::Dedicated = true;
            enableWinConsole(true);
            $argUsed[%i]++;

         //--------------------
         case "-mission":
            $argUsed[%i]++;
            if (%hasNextArg) {
               $missionArg = %nextArg;
               $argUsed[%i+1]++;
               %i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -mission <filename>");

         //--------------------
        case "-port":
            $argUsed[%i]++;
            if (%hasNextArg) {
               //lol will be overwritten somewhere ?! $Pref::Server::Port = %nextArg;
               $Server::Port = %nextArg;
               $argUsed[%i+1]++;
               %i++;
            }
            else
               error("Error: Port Line argument. Usage: -port <port>");
         //--------------------
         case "-connect":
            $argUsed[%i]++;
            if (%hasNextArg) {
               $JoinGameAddress = %nextArg;
               $argUsed[%i+1]++;
               %i++;
            }
            else
               error("Error: Missing Command Line argument. Usage: -connect <ip_address>");
      }
   }
}

function onStart()
{
   // The core does initialization which requires some of
   // the preferences to loaded... so do that first.  
   exec( "./client/defaults.cs" );
   exec( "./server/defaults.cs" );
             
   Parent::onStart();
   echo("\n--------- Initializing Directory: " SPC $defaultGame SPC "! ---------");

   if (isObject(Canvas) && $Game::Caption !$= "")
       Canvas.setWindowTitle($Game::Caption);


   // Load the scripts that start it all...
   exec("./client/init.cs");
   exec("./server/init.cs");

           
   $Pref::Net::LagThreshold       = "400";  
   $pref::Net::PacketRateToServer = "32"; //32  
   $pref::Net::PacketRateToClient = "32"; //32 
   $pref::Net::PacketSize         = "508";  
           

   
   // Init the physics plugin.
   physicsInit();
      
   // Start up the audio system.
   sfxStartup();

   // Server gets loaded for all sessions, since clients
   // can host in-game servers.
   initServer();

   // Start up in either client, or dedicated server mode
   if ($Server::Dedicated)
      initDedicated();
   else
      initClient();
}

function onExit()
{
   // Ensure that we are disconnected and/or the server is destroyed.
   // This prevents crashes due to the SceneGraph being deleted before
   // the objects it contains.
   if ($Server::Dedicated)
      destroyServer();
   else
      disconnect();
   
   // Destroy the physics plugin.
   physicsDestroy();
      
   echo("Exporting client prefs");
   export("$pref::*", $Client::prefsFile, False);

   echo("Exporting server prefs");
   export("$Pref::Server::*", $Server::prefsFile, False);

   Parent::onExit();
}

}; // package fps

// Activate the game package.
activatePackage(game);
