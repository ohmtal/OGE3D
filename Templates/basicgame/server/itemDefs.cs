//-----------------------------------------------------------------------------
// EasternEggItem
//-----------------------------------------------------------------------------
datablock ItemData( EasternEggItem )
{
    category = "Items";
    shapeFile = "~/art/shapes/drops/osterei.dts";
    skinname ="ostereier ostereierblau ostereiergelb ostereiergruen ostereierpink ostereierrot";
    
    sound = "AmmoPickupSound";  //if you dont want sound make this empty
    
    //FIXME classname = CustomItem;
    
    PopTime = 0; //120 sec 0=dont pop!
    
    //basic options:
     mass = 1;
     friction = 1;
     elasticity = 0.3;
     emap = false;
     rotate = "0";
    
};
