//-----------------------------------------------------------------------------
// scripts loaded on server startup 
//-----------------------------------------------------------------------------

// Load up all scripts.  This function is called when
// a server is constructed.

exec("./clients.cs");
exec("./camera.cs");

//for pick spawnpoint: 
exec("core/ohmtal/server/tools.cs");

//FIXME exec("./player.cs");


