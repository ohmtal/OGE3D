//----------------------------------------------------------------------------
// Mission Loading & Mission Info
// The mission loading server handshaking is handled by the
// core/scripts/client/missingLoading.cs.  This portion handles the interface
// with the game GUI.
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Loading Phases:
// Phase 1: Download Datablocks
// Phase 2: Download Ghost Objects
// Phase 3: Scene Lighting

//----------------------------------------------------------------------------
// Phase 1
//----------------------------------------------------------------------------

function onMissionDownloadPhase1(%missionName, %musicTrack)
{   

   if ($pref::Video::disablePostFX) 
   {
      PostFXManager.settingsSetEnabled(false);
   
   } else {
      // Load the post effect presets for this mission.
      %path = $defaultGame @ "/levels/" @ fileBase( %missionName ) @ $PostFXManager::fileExtension;
      if ( isScriptFile( %path ) )
         postFXManager::loadPresetHandler( %path ); 
      else
         PostFXManager::settingsApplyDefaultPreset();
   }            
   // Close and clear the message hud (in case it's open)
   if ( isObject( MessageHud ) )
      MessageHud.close();

   // Reset the loading progress controls:
   if ( !isObject( LoadingProgress ) )
      return;
	  
   LoadingProgress.setValue(0);
   LoadingProgressTxt.setValue("LOADING DATABLOCKS");
   Canvas.repaint();
}

function onPhase1Progress(%progress)
{
   if ( !isObject( LoadingProgress ) )
   {
      echo("I. loading " SPC %progress);
      return;
	}  
      
   LoadingProgress.setValue(%progress);
   Canvas.repaint(33);
}

function onPhase1Complete()
{
   if ( !isObject( LoadingProgress ) )
   {
      echo("I. loading completed.");
      return;
	}  
	  
   LoadingProgress.setValue( 1 );
   Canvas.repaint();
}

//----------------------------------------------------------------------------
// Phase 2
//----------------------------------------------------------------------------

function onMissionDownloadPhase2()
{

   if ( !isObject( LoadingProgress ) )
      return;
      
   LoadingProgress.setValue(0);
   LoadingProgressTxt.setValue("LOADING OBJECTS");
   Canvas.repaint();
}

function onPhase2Progress(%progress)
{
   if ( !isObject( LoadingProgress ) )
   {
      echo("II. loading " SPC %progress);
      return;
	}  
        
   LoadingProgress.setValue(%progress);
   Canvas.repaint(33);
}

function onPhase2Complete()
{
   if ( !isObject( LoadingProgress ) )
   {
      echo("II. loading completed");
      return;
	}  
	  
   LoadingProgress.setValue( 1 );
   Canvas.repaint();
}   

function onFileChunkReceived(%fileName, %ofs, %size)
{
   if ( !isObject( LoadingProgress ) )
      return;     

   LoadingProgress.setValue(%ofs / %size);
   LoadingProgressTxt.setValue("Downloading " @ %fileName @ "...");
}

//----------------------------------------------------------------------------
// Phase 3
//----------------------------------------------------------------------------

function onMissionDownloadPhase3()
{
   if ( !isObject( LoadingProgress ) )
      return;
      

   LoadingProgress.setValue(0);
   LoadingProgressTxt.setValue("LIGHTING MISSION");
   Canvas.repaint();
}

function onPhase3Progress(%progress)
{
   if ( !isObject( LoadingProgress ) )
   {
      echo("III. loading " SPC %progress);
      return;
	}  
   LoadingProgress.setValue(%progress);
   Canvas.repaint(33);
}

function onPhase3Complete()
{
   $lightingMission = false;

   if ( !isObject( LoadingProgress ) )
   {
      echo("II. loading completed");
      return;
	}  
	  
	  
   LoadingProgressTxt.setValue("STARTING MISSION");
   LoadingProgress.setValue( 1 );
   Canvas.repaint();
}

//----------------------------------------------------------------------------
// Mission loading done!
//----------------------------------------------------------------------------

function onMissionDownloadComplete()
{
   //2023-10-28 call all to overwrite the default functions! 
   if (isFile( $defaultgame @ "/client/missions/all.cs"))
   {
      exec("./missions/all.cs");
   }
   
   if (exec("./missions/" @ fileBase($Client::MissionFile) @ ".cs") == 0) {
      echo("* Client Mission " SPC fileBase($Client::MissionFile) SPC "loaded");
   }
   if (isFunction("onClientMissionStart")) onClientMissionStart();
   
   
   
}


//------------------------------------------------------------------------------
// Before downloading a mission, the server transmits the mission
// information through these messages.
//------------------------------------------------------------------------------

addMessageCallback( 'MsgLoadInfo', handleLoadInfoMessage );
addMessageCallback( 'MsgLoadDescripition', handleLoadDescriptionMessage );
addMessageCallback( 'MsgLoadInfoDone', handleLoadInfoDoneMessage );
addMessageCallback( 'MsgLoadFailed', handleLoadFailedMessage );

//------------------------------------------------------------------------------

function handleLoadInfoMessage( %msgType, %msgString, %mapName ) 
{
   if (!isObject(LoadingGui))
      return;

   // Make sure the LoadingGUI is displayed
   if (Canvas.getContent() != LoadingGui.getId())
   {
      loadLoadingGui("LOADING MISSION FILE");
   }
   
	// Clear all of the loading info lines:
	for( %line = 0; %line < LoadingGui.qLineCount; %line++ )
		LoadingGui.qLine[%line] = "";
	LoadingGui.qLineCount = 0;
}

//------------------------------------------------------------------------------

function handleLoadDescriptionMessage( %msgType, %msgString, %line )
{

   if (!isObject(LoadingGui))
      return;

	LoadingGui.qLine[LoadingGui.qLineCount] = %line;
	LoadingGui.qLineCount++;

   // Gather up all the previous lines, append the current one
   // and stuff it into the control
	%text = "<spush><font:Arial:16>";
	
	for( %line = 0; %line < LoadingGui.qLineCount - 1; %line++ )
		%text = %text @ LoadingGui.qLine[%line] @ " ";
   %text = %text @ LoadingGui.qLine[%line] @ "<spop>";
}

//------------------------------------------------------------------------------

function handleLoadInfoDoneMessage( %msgType, %msgString )
{
   // This will get called after the last description line is sent.
}

//------------------------------------------------------------------------------

function handleLoadFailedMessage( %msgType, %msgString )
{
   MessageBoxOK( "Mission Load Failed", %msgString NL "Press OK to return to the Main Menu", "disconnect();" );
}
