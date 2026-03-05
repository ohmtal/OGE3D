/*
exec("basicbasicgame/client/bob.cs");

exec("basicbasicgame/client/bob.cs");ClientBob::listClientObjects();
*/

function ClientBob::listClientObjects()
{
   
   initContainerALLSearch(  true ); //HAHA hacked in
   
   while ((%obj = containerSearchNext(true)) != 0) { 
      %className = %obj.getClassName();
      echo(%obj.getId() SPC %obj.getName() SPC %className); 
   }
   /*
   for(%i=0; %i<40; %i++) {
   
      %obj = containerSearchNext(true);
      if (!isObject(%obj))
      {
         dEcho("NIX" SPC %i SPC %obj);
         continue;
      }
      %className = %obj.getClassName();
      dEcho(%i SPC %obj.getId() SPC %obj.getName() SPC %className); 
   }
   */
   
   ContainerClearSearch(true);

}
function ClientBob::findEnvironmentObjectType()
{
   %typeMask = ( $TypeMasks::EnvironmentObjectType );
   %typeMask = ( $TypeMasks::EnvironmentObjectType );
   initContainerTypeSearch( %typeMask, true );
   

   while ((%obj = containerSearchNext(true)) != 0) { 
      %className = %obj.getClassName();
      echo(%obj.getId() SPC %obj.getName() SPC %className); 
   }

/*
   for(%i=0; %i<40; %i++) {
   
      %obj = containerSearchNext(true);
      if (!isObject(%obj))
      {
         dEcho("NIX" SPC %i SPC %obj);
         continue;
      }
      %className = %obj.getClassName();
      dEcho(%i SPC %obj.getId() SPC %obj.getName() SPC %className); 
   }
*/ 

   ContainerClearSearch(true);  
}


function ClientBob::findSun()
{
   %typeMask = ( $TypeMasks::EnvironmentObjectType );
   initContainerTypeSearch( %typeMask, true );
   while ((%obj = containerSearchNext(true)) != 0) { 
      %className = %obj.getClassName();
      dEcho(%obj.getId() SPC %className); 
      if (%className $= "ScatterSky"  || %className $= "Sun") {
         dEcho("FOUND:" SPC %obj.getId());
         ContainerClearSearch(true);
         return %obj;
      }
   }

   //not found :(
   dEcho("NO sun found ");
   ContainerClearSearch(true);
   return 0;
}


function ClientBob::setSunElevation(%elevation)
{
   
   %sun = ClientBob::findSun();
   if ( !%sun) 
      return false;
      
   %sun.elevation=%elevation;
   return true;   
}

function ClientBob::makeNight()
{
   %sun = ClientBob::findSun();
   if ( !%sun) 
      return false;
      
   %sun.elevation=270;
   %sun.castShadows = false;
   return true;
}

function ClientBob::makeDay()
{
   %sun = ClientBob::findSun();
   if ( !%sun) 
      return false;
      
   %sun.elevation=45;
   %sun.castShadows = true;
   return true;
}


