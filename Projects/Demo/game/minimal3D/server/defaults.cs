//-----------------------------------------------------------------------------
// Server defaults
//-----------------------------------------------------------------------------

// First we execute the core default preferences.
exec( "core/scripts/server/defaults.cs" );

//-----------------------------------------------------------------------------
$Server::prefsFile = "userdata/" @ $defaultGame @ "_serverprefs.cs";

if ( isFile( $Server::prefsFile ) )
    exec( $Server::prefsFile );
