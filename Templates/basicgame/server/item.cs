//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// These scripts make use of dynamic attribute values on Item datablocks,
// these are as follows:
//
//    maxInventory      Max inventory per object (100 bullets per box, etc.)
//    pickupName        Name to display when client pickups item
//
// Item objects can have:
//
//    count             The # of inventory items in the object.  This
//                      defaults to maxInventory if not set.

// Respawntime is the amount of time it takes for a static "auto-respawn"
// object, such as an ammo box or weapon, to re-appear after it's been
// picked up.  Any item marked as "static" is automaticlly respawned.
$Item::RespawnTime = 30 * 1000;

// Poptime represents how long dynamic items (those that are thrown or
// dropped) will last in the world before being deleted.
$Item::PopTime = 30 * 1000;

//-----------------------------------------------------------------------------
// ItemData base class methods used by all items
//-----------------------------------------------------------------------------

//XXTH demogame test
exec("./itemDefs.cs");
exec("./ostern.cs");


/*
 exec("basicgame/server/item.cs");
 */

/*
pl_visitor.Throw(pl_visitor.getMountedObject(0));
*/



function servercmdNotifyServerDropItems(%client)
{
   dEcho("servercmdNotifyServerDropItems enter");
   %player = %client.player;
          while (%player.getMountedObjectCount() >0) 
          {
             %egg = %player.getMountedObject(0);
             %egg.setShapeName(%egg.savShapeName);
             %egg.unMount();
             %egg.setTransForm(getTerrainLevel(%player.getPosition(),2));
          }
   
}

function servercmdNotifyServerItemAction(%client,%clientid, %ghostid)
{
   %targetobject=%client.resolveObjectFromGhostIndex(%ghostid);
   
   if (isObject(%targetobject) && %targetobject.getClassName() $= "Item" 
       && %targetobject.getDatablock().getname()  $= "EasternEggItem")
   {
      
      if ( %targetobject.ismounted() ) {
         error("NOT ALLOWED!");
         //FIXME MESSAGE CLIENT
         return;
      }
            
      
      %count = %client.player.getMountedObjectCount();
      %maxObjects = 2;
      
      if (%count >= %maxObjects ) {
         error("FIXME message.... can't carry any more");
         //FIXME MESSAGE CLIENT
         return;
      } else {
         
         //USEFULL METHODS (sceneObject): getMountedObject(node), getMountedObjectCount         
         // unmountObject(objid)
         
         %slot = %count;
         %client.player.mountobject(%targetobject,%slot);
         %targetobject.savShapeName = %targetobject.getShapeName();
         %targetobject.setShapename("");
         
      }
      

   }
}

function CreateNewBag(%position, %itemname, %stuff, %money, %owner, %monster, %harvest)
{
      %drop = new Item() {
         position = %position;
         dataBlock = %itemname;
         rotate = %itemname.rotate;
         static = "1";
       };
       
          //Random skin 
       if (%drop.dataBlock.skinname !$="") {
          %TmpTexId = getRandom(0,getWordCount(%drop.dataBlock.skinname) -1);
          %skinname  = getWord(%drop.dataBlock.skinname,%TmpTexId);
          %drop.setSkinName(%skinname);
       }

       %drop.stuff= %stuff;
      /*        
       %drop.setPlayerType(5);
       if (!isObject(%owner) || !isObject(%owner.client)) %owner = 0; 
       %drop.owner = %owner;
       if (%owner != 0)
       {
         %nick = %drop.owner.getShapename();
         if (%nick !$= "") 
         {
             %drop.setShapeName(%nick @ "'s bag");
             %drop.setGuildName(%drop.owner.getGuildName());
         }
       }
       */
       %drop.money = %money;
       %drop.time  = $sim::Time;
       
       
       %drop.schedulePop();
       
       dEcho("ADD TO ITEM TO MISSIONCLEANUP:" SPC %drop.getId() SPC " count:" SPC MissionCleanup.getCount());
       MissionCleanup.add(%drop);

       return %drop;
}


//-----------------------------------------------------------------------------
function Item::schedulePop(%this)
{
   // This method deletes the object after a default duration. Dynamic
   // items such as thrown or drop weapons are usually popped to avoid
   // world clutter.
   
   //1.99 remove old timers
   if (isEventPending(%this.shedFade)) cancel(%this.shedFade);
   if (isEventPending(%this.shedDelete)) cancel(%this.shedDelete);
   
   %popTime = $Item::PopTime; //set default 2.10
   if (%this.getDataBlock().PopTime !$="") {
      %popTime = %this.getDataBlock().PopTime; 
   }
   if ( %popTime == 0) //dont pop / easten eggs :P
   {
      //spam error("ITEM:: NO POPTIME FOR " SPC %this.getId());
      return ;
   } else {
       // error("POPTIME" SPC %poptime SPC  "FOR " SPC %this.getId());
   }
   %this.shedFade   = %this.schedule(%popTime - 1000, "startFade", 1000, 0, true);
   %this.shedDelete = %this.schedule(%popTime, "delete");
}
