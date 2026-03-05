/**
 * Ostern Egg Spawn Manager
 * @since 2019-04-01 / game version 2.1
 *
 exec("basicgame/server/ostern.cs"); 
 initOstern();
 finitOstern();
 
 echo("OSTEREI CHECK:" SPC OstereierGroup.getCount() SPC "/" SPC OstereierGroup.SpawnCount); 
   
   
 2021-03-02 OGE3D Test
   * WARNING %this is reserved !!! does not   

 */

exec("./tools.cs");


function lazyOstern()
{
 initOstern("0 0 10" , 50, 30);
}

function spawnPosTest()
{
   //echo(getTerrainLevel(pl_visitor.getPosition(),100,0,false, false));
   
   echo(getTerrainLevel("0 0 10",100,0,false, false));
}

 //------------------------------------------------------------------------------
 function initOstern(%pos,%radius, %count)
 {
   if (isObject(OstereierGroup))
      OstereierGroup.delete();
      
   new SimGroup(OstereierGroup);
      
//FIXME    OstereierGroup.loot     = $itemloot[99] @ ";OsternRoosterSpawnItem 10 1 1;GreatHellPigSpawnItem 20 1 1"; //Osterrooster
  
   OstereierGroup.pos = "0 0 0";
   if ( %pos !$= "") 
      OstereierGroup.pos = %pos;

   OstereierGroup.radius = (getword(MissionArea.area,3) / 2) - 100;
   if ( %radius !$= "") 
      OstereierGroup.radius = %radius;

   
   if (%count !$= "")
      OstereierGroup.SpawnCount = %count;
   else
      OstereierGroup.SpawnCount = Tools::round(OstereierGroup.radius / 3.5);

      
   schedule(5000,0,"Ostereier_check");   
   
      
 }
//------------------------------------------------------------------------------
function finitOstern()
{

   if (!isObject(OstereierGroup))
         return false;
   
   while (OstereierGroup.getCount() > 0)
   {
         OstereierGroup.getObject(0).delete();
   }
   OstereierGroup.delete();
   return 1;

}
//------------------------------------------------------------------------------
function Ostereier_check()
{
  echo("**** OSTEREI CHECK:" SPC OstereierGroup.getCount() SPC "/" SPC OstereierGroup.SpawnCount); 
  %groupObj = OstereierGroup;
  while(%groupObj.getCount() < %groupObj.SpawnCount)
  {
      Ostereier_spawnEi();
      // 2020-04-11 changed from 10 to 30000 for testing 2021 back to 10 :P
      schedule(5000,0,"Ostereier_check");
      return;
  }
  schedule( 600000,0,"Ostereier_check"); //all 10 minutes!
}
//------------------------------------------------------------------------------
function Ostereier_spawnEi()
{
  %groupObj = OstereierGroup;
  %dings =  OstereierGroup;
  
  
  
  //FIXME %stuff = Tools::GetLOOTFROMString(%groupObj.loot);
  %scores = "10 30 50 100";
  %stuff = getword(%scores, getRandom(3));
  %money = 0;
  %id =  CreateNewBag(getTerrainLevel(%groupObj.pos,%groupObj.radius,0,false, false), EasternEggItem,%stuff, %money);
  %id.setShapeName("DemoEgg" SPC %id.getId());
  echo("Span OsterEi :) pos=" SPC %id.getPosition() SPC "shapename=" SPC %id.getShapeName); 
  %id.setscale("1.2 1.2 1.2");
  

  %groupObj.add(%id);
      
  return %id;
}
