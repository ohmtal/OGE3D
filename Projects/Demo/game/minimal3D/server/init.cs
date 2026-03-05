//-----------------------------------------------------------------------------
// Server init
//-----------------------------------------------------------------------------

function initServer()
{
   echo("\n--------- Initializing " @ $appName @ ": Server Scripts ---------");

   // Server::Status is returned in the Game Info Query and represents the
   // current status of the server. This string sould be very short.
   $Server::Status = "Unknown";

   // Specify where the mission files are.
   $Server::MissionFileSpec = $Game::levelPath @ "*.mis";

   // The common module provides the basic server functionality
   initBaseServer();


   // loadup singeltons 
   exec($defaultGame @ "/art/main.cs");

   //  exec server files here >>>>>>>>>>>>
   exec("./game.cs");
   
   //<<<<<<<<<<<<<<<<<<<<<<<< client exec 
}


//-----------------------------------------------------------------------------

function initDedicated()
{
   enableWinConsole(true);
   echo("\n--------- Starting Dedicated Server ---------");

   // Make sure this variable reflects the correct state.
   $Server::Dedicated = true;

   // The server isn't started unless a mission has been specified.
   if ($missionArg !$= "") {
      createServer("MultiPlayer", $missionArg);
   }
   else
      echo("No mission specified (use -mission filename)");
}

