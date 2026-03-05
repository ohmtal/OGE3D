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

//-----------------------------------------------------------------------------
// Misc server commands avialable to clients
//-----------------------------------------------------------------------------

function serverCmdSuicide(%client)
{
   if (isObject(%client.player))
      %client.player.kill("Suicide");
}

function serverCmdPlayCel(%client,%anim)
{
   if (isObject(%client.player))
      %client.player.playCelAnimation(%anim);
}

function serverCmdTestAnimation(%client, %anim)
{
   if (isObject(%client.player))
      %client.player.playTestAnimation(%anim);
}

function serverCmdPlayDeath(%client)
{
   if (isObject(%client.player))
      %client.player.playDeathAnimation();
}

// ----------------------------------------------------------------------------
// Throw/Toss
// ----------------------------------------------------------------------------

function serverCmdThrow(%client, %data)
{
   %player = %client.player;
   if(!isObject(%player) || %player.getState() $= "Dead" || !$Game::Running)
      return;
   switch$ (%data)
   {
      case "Weapon":
         %item = (%player.getMountedImage($WeaponSlot) == 0) ? "" : %player.getMountedImage($WeaponSlot).item;
         if (%item !$="")
            %player.throw(%item);
      case "Ammo":
         %weapon = (%player.getMountedImage($WeaponSlot) == 0) ? "" : %player.getMountedImage($WeaponSlot);
         if (%weapon !$= "")
         {
            if(%weapon.ammo !$= "")
               %player.throw(%weapon.ammo);
         }
      default:
         if(%player.hasInventory(%data.getName()))
            %player.throw(%data);
   }
}

// ----------------------------------------------------------------------------
// Force game end and cycle
// Probably don't want this in a final game without some checks.  Anyone could
// restart a game.
// ----------------------------------------------------------------------------

function serverCmdFinishGame()
{
   cycleGame();
}

// ----------------------------------------------------------------------------
// Cycle weapons
// ----------------------------------------------------------------------------

function serverCmdCycleWeapon(%client, %direction)
{
   %client.getControlObject().cycleWeapon(%direction);
}

// ----------------------------------------------------------------------------
// Unmount current weapon
// ----------------------------------------------------------------------------

function serverCmdUnmountWeapon(%client)
{
   %client.getControlObject().unmountImage($WeaponSlot);
}

// ----------------------------------------------------------------------------
// Weapon reloading
// ----------------------------------------------------------------------------

function serverCmdReloadWeapon(%client)
{
   %player = %client.getControlObject();
   %image = %player.getMountedImage($WeaponSlot);
   
   // Don't reload if the weapon's full.
   if (%player.getInventory(%image.ammo) == %image.ammo.maxInventory)
      return;
      
   if (%image > 0)
      %image.clearAmmoClip(%player, $WeaponSlot);
}


//-----------------------------------------------------------------------------
// afxCam 
//-----------------------------------------------------------------------------
//SHORT VERSION : 
function serverCmdDollyThirdPersonCam(%client, %toward)
{
//FIXME maxdistance somewhere else defined
  %maxdistance = 10;
   
  %distance = %client.camera.getThirdPersonDistance();
  if (%toward) 
    %distance -= 0.5;
  else
    %distance += 0.5;

  %distance = mclamp(%distance,0.9,%maxdistance);

  %client.camera.setThirdPersonDistance(%distance);    
}


/*
//~~~~~~~~~~~~~~~~~~~~//~~~~~~~~~~~~~~~~~~~~//
// AFX CAMERA DOLLY 
//   usually by MouseWheel or Home/End keys

function dollyCamAwayFromPlayer(%client)
{ 
  %distance = %client.camera.getThirdPersonDistance();
  %distance += 0.5;
  %client.camera.setThirdPersonDistance(%distance);
}

function dollyCamTowardPlayer(%client)
{   
  %distance = %client.camera.getThirdPersonDistance();
  %distance -= 0.5;
  %client.camera.setThirdPersonDistance(%distance);
}

function serverCmdDollyThirdPersonCam(%client, %toward)
{
  if (%toward)
    dollyCamTowardPlayer(%client);
  else
    dollyCamAwayFromPlayer(%client); 
}

 OLD SHIT !! 
function dollyCamAwayFromPlayer(%client)
{
  if (!$GLOBALS::useAFXCamera) {
    return;
  }
  %offset = %client.camera.getThirdPersonOffset();

  %x_pos = firstWord(%offset);
  %y_pos = getWord(%offset,1);
  %z_pos = getWord(%offset,2);

  
  %y_pos -= 0.5;
  
  if (%y_pos < -10) return; //limit 
  

  %client.camera.setThirdPersonOffset(%x_pos @ " " @ %y_pos @ " " @ %z_pos);
}
//-----------------------------------------------------------------------------
function dollyCamTowardPlayer(%client)
{   
  if (!$GLOBALS::useAFXCamera) {
    return;
  }
    %offset = %client.camera.getThirdPersonOffset();

    %x_pos = firstWord(%offset);
    %y_pos = getWord(%offset,1);
    %z_pos = getWord(%offset,2);

    if (%y_pos < -1.0)
    {
      %y_pos += 0.5;
    }
    else
    {
      %y_pos = -0.5;
      //echo("GOTO FIRST PERSON");
      ///$firstPerson = !$firstPerson;
    }

    %client.camera.setThirdPersonOffset(%x_pos @ " " @ %y_pos @ " " @ %z_pos);
}
//-----------------------------------------------------------------------------
function serverCmdDollyThirdPersonCam(%client, %toward)
{
  if (!$GLOBALS::useAFXCamera) {
    return;
  }
  if (%toward)
  {
    dollyCamTowardPlayer(%client);
  }
  else
  {
    dollyCamAwayFromPlayer(%client); 
  }
}
*/

