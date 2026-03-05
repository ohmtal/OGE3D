/* 
EMPTY 2
pseudo Instance Test

Docu/WhitePaper: https://idrive.chatwana.net/index.php?mod=forum&feature=showpost&idd=297
Prefab file: basicgame/levels/Instance1.prefab

Basicly this looks not that bad ... hackish hardcoded at the moment ....

*/

function OnMissionStart() 
{

 dError("EMPTY2 OnMissionStart");

 //FIXME 
 // create InstanceManager with param Prefab ... 
 // for housings maybe the place should also be safed as prefab ?!?!   
 $Mission::InstancePrefab="basicgame/levels/Instance1.prefab";
 /*
  ok now we should parse the prefab file and look for the SpawnSphere,
  because we need this for the SpawnPoint
  
  for first Test we hack this in 
  
 */
 $Mission::BaseSpawnPoint = "5.02252 -5.81937 4.00667";   
   
}
function OnMissionCleanup() {
}

//-----------------------------------------------------------------------------
function GameConnection::onClientEnterGame(%this)
{
   
   dError("EMPTY2 GameConnection::onClientEnterGame");   
   
   //CREATE NEW INSTANCE !!!  
   //FIXME GROUPS AND MANAGER FOR THIS 
   
   $Mission::InstanceObjects = new Prefab() { 
      position  = "10000 10000 30";
      filename = $Mission::InstancePrefab;
   };
   
   MissionCleanup.add($Mission::InstanceObjects);
   %playerSpawnPoint = VectorAdd($Mission::InstanceObjects.position, $Mission::BaseSpawnPoint);   
   
   //FIXME on respawn we need to use player.spawnpoint instead of looking for map spawnpoint
   //done in  GameConnection::spawnPlayer :D   
   
    dError("EMPTY2 GameConnection::onClientEnterGame spawnpoint should be:" SPC %playerSpawnPoint);
   
   
   %this.setName("cl_" @ %this.nameBase);   
   
   commandToClient(%this, 'SyncClock', $Sim::Time - $Game::StartTime);
   %cameraSpawnPoint = pickCameraSpawnPoint($Game::DefaultCameraSpawnGroups);
   %this.spawnCamera(%cameraSpawnPoint);
   // Spawn a camera for this client using the found %spawnPoint
   %player = %this.spawnPlayer( %playerSpawnPoint);

   

}