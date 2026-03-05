//exec("basicgame/client/items.cs");

function itemAction(%obj)
{
   %ghost = ServerConnection.getGhostID(%obj.getid());
   //error("Client right Clicked item" @ " ID=" @ %obj.getid() @ "GHOST=" @ %ghost);
   cmdToServer('NotifyServerItemAction', %obj.getid(), %ghost);
}



// called from pressing "E"
function itemActionEvent( %val )
{
   if ( %val ) 
   {
      %obj = getClientCloseObject();
      if (%obj > 0 && isObject(%obj))
      {
         echo ("itemAction on" SPC %obj);
         itemAction(%obj);
      }
   }
}


// called from pressing "Q"
function dropItemsEvent( %val )
{

   if ( %val ) 
   {
      cmdToServer('NotifyServerDropItems');
   }
}
