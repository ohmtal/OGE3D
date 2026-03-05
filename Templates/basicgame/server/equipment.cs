//   exec("basicgame/server/equipment.cs");


function test() { 
   cl_Visitor.shieldOff();
   cl_Visitor.schedule(0,"shieldOn"); 
   echo("SHIELD ON ?!?");
}

function GameConnection::shieldOn(%this)
{
   
   %isAvatar = false; 
   %image = BeetleShieldImage; //== %item.image
   
//    if (%isAvatar) // (!%this.player.isAnimal)
//    {
    
      %mp = "Bip01 L Forearm"; //%isAvatar ? "mount1" : "mount3";
      %scale = %item.image.meshScale !$= "" ? %item.image.meshScale : 1;
      %this.player.mountEquipment(%image,%mp, %scale);
//    }
    %this.player.shielded = 1;

   
}

function GameConnection::shieldOff(%this)
{
   %isAvatar = false; 

//   if (%isAvatar) 
//   {
      %mp = "Bip01 L Forearm"; //%isAvatar ? "mount1" : "mount3"; 
      %this.player.unmountEquipment(%mp);
      
//   }  
   %this.player.shielded = 0;

}
//--------------------------------------------------------------------------
// Shield01Image
//--------------------------------------------------------------------------
datablock ShapeBaseData(Shield01Image)
{
   // Basic Item properties
   shapeFile = "~/art/shapes/shield/shield.dts";
   item = Shield01Item;
   emap = true;

//     offset = "0 0 0";
//     rotation = "0 0 1 -72";
   offset = "-0.15 0 0";
   rotation = "0 0.9 0.9 20";
   
};
//--------------------------------------------------------------------------
// BeetleShieldImage
//--------------------------------------------------------------------------
datablock ShapeBaseData(BeetleShieldImage:Shield01Image)
{
   // Basic Item properties
   shapeFile = "~/art/shapes/shield/beetleshield.dts";
   item = BeetleShieldItem;
};
