//-----------------------------------------------------------------------------
// Client init
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
function initClient()
{
   echo("\n--------- Initializing " @ $appName @ ": Client Scripts ---------");

   // Make sure this variable reflects the correct state.
   $Server::Dedicated = false;

   // Game information used to query the master server
   $Client::GameTypeQuery = $appName;
   $Client::MissionTypeQuery = "Any";

   // These should be game specific GuiProfiles.  Custom profiles are saved out
   // from the Gui Editor.  Either of these may override any that already exist.
   exec($defaultGame @ "/art/gui/defaultGameProfiles.cs");
   exec($defaultGame @ "/art/gui/customProfiles.cs"); 
   
   // The common module provides basic client functionality
   initBaseClient();

   // Use our prefs to configure our Canvas/Window
   configureCanvas();

   // XXTH loadup singeltons 
   exec($defaultGame @ "/art/main.cs");
   
   //  exec client files here >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
   //scripts
   exec("./missionDownload.cs");
   exec("./loadingGui.cs");
   exec("./serverConnection.cs");
   exec("./playGui.cs");
   exec("./gameHud2D.cs");
   
   // ui 
   exec("./ui/StartupGui.gui");
   exec("./ui/startMissionGui.gui");
   exec("./ui/mainMenuGui.gui");
   exec("./ui/cursors.cs");
   exec("./ui/loadingGui.gui");
   exec("./ui/playGui.gui");
   // from core 
   
   exec("core/ohmtal/client/default.bind.cs");
   exec("core/ohmtal/client/optionsDlg.cs");
   
   
   //<<<<<<<<<<<<<<<<<<<<<<<< client exec <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 
   if (isFile($Client::configFile))
      exec($Client::configFile);

   loadMaterials();

   // Really shouldn't be starting the networking unless we are
   // going to connect to a remote server, or host a multi-player
   // game.
   setNetPort(0);

   // Copy saved script prefs into C++ code.
   setDefaultFov( $pref::Player::defaultFov );
   setZoomSpeed( $pref::Player::zoomSpeed );


   Canvas.setCursor("DefaultCursor");
   loadMainMenu();
//moved to core    Canvas.showWindow();
   
   
}


//-----------------------------------------------------------------------------

function loadMainMenu()
{
   // Startup the client with the Main menu...
   if (isObject( MainMenuGui ))
      Canvas.setContent( MainMenuGui );
   
   Canvas.setCursor("DefaultCursor");

   // first check if we have a level file to load
   if ($levelToLoad !$= "")
   {
      %levelFile = $defaultGame @ "/levels/";
      %ext = getSubStr($levelToLoad, strlen($levelToLoad) - 3, 3);
      if(%ext !$= "mis")
         %levelFile = %levelFile @ $levelToLoad @ ".mis";
      else
         %levelFile = %levelFile @ $levelToLoad;

      // Clear out the $levelToLoad so we don't attempt to load the level again
      // later on.
      $levelToLoad = "";
      
      // let's make sure the file exists
      %file = findFirstFile(%levelFile);

      if(%file !$= "")
         createAndConnectToLocalServer( "SinglePlayer", %file );
   }
}

function loadLoadingGui(%displayText)
{
   Canvas.setContent("LoadingGui");
   LoadingProgress.setValue(1);

   if (%displayText !$= "")
   {
      LoadingProgressTxt.setValue(%displayText);
   }
   else
   {
      LoadingProgressTxt.setValue("WAITING FOR SERVER");
   }

   Canvas.repaint();
}
