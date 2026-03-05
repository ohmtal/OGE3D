//-----------------------------------------------------------------------------
// Client defaults
//-----------------------------------------------------------------------------

// First we execute the core default preferences.
exec( "core/scripts/client/defaults.cs" );
//-----------------------------------------------------------------------------

$pref::Video::displayDevice = "OpenGL";
$pref::Video::disableVerticalSync = 1;
$pref::Video::mode = "1280 800 false 32 60 4";
$pref::Video::defaultFenceCount = 0;
$pref::Video::screenShotSession = 0;
$pref::Video::screenShotFormat = "PNG";

//-----------------------------------------------------------------------------
$Client::prefsFile = "userdata/" @ $defaultGame @ "_clientprefs.cs";
$Client::configFile = "userdata/" @ $defaultGame @ "_clientconfig.cs";

if ( isFile( $Client::prefsFile ) )
    exec( $Client::prefsFile );
    

    
//overwrite with settings is set argv -toaster OR -flagship     
execOverwriteSettings(); //toaster, ...
