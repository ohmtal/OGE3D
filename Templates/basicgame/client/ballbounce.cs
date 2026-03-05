/*
     exec("basicgame/client/ballbounce.cs");
     
     tom2Dgui.maxBalls = 2000;
*/


//------------------------------------------------------------------------------
function tom2Dgui::MoreBalls(%this) {
     if (tom2DGui.ballCount >= tom2Dgui.maxBalls) 
        return;
        
        
     $newX = getRandom(50, tom2Dgui.w -100);
     $newY = getRandom(50, tom2Dgui.h -100);
     %ball = new tom2dSprite() {
        class  = "MyBall2D";
        size   = "64 64";
        Layer = 10;
        x     = $newX;
        y     = $newY;
        Visible = true;
        
        DirX   = 1;
        DirY   = 1;
      }; 
     %ball.Speed  = 10 * getRandom(8,16); 
     %ball.setTexture(tom2Dgui.img_ball);
     tom2DGui.addRenderObject(%ball);
     tom2DGui.ballCount++;
      
     tom2DGui.schedule(1000,moreBalls);
  
  
  
}

function MyText::onRender(%this,%dt, %bla) {
  %this.screen.writeText(1,10,"Balls:" SPC %this.screen.ballCount,0,NumericHealthProfile);
  %this.screen.writeText(%this.screen.w / 2 ,10,"FPS:" SPC $fps::real,1,NumericHealthProfile);
  %this.screen.writeText(%this.screen.w,10,"DT:" SPC %dt,2,NumericHealthProfile);
  

  
}

function MyBall2D::onUpdate(%this,%fDt) 
{
   
   if (!tom2Dgui.isAwake())
      return;

  if (%this.x + 8 >= tom2Dgui.w && %this.dirX == 1) {
     %this.dirX =  -1;
     %this.speed++;
  } else if (%this.x <= 8 && %this.dirX == -1) {
     %this.dirX =  1;
     %this.speed++;
  }
  
  if (%this.Y + 8 >= tom2Dgui.h && %this.dirY == 1) {
     %this.dirY =  -1;
     %this.speed++;
  } else if (%this.y <= 8 && %this.dirY == -1) {
     %this.dirY =  1;
     %this.speed++;
  }

  if (%this.speed > 150) {
    //echo("* ball blows off");
    tom2DGui.ballCount--;
    %this.schedule(0,delete);
    tom2DGui.schedule(1000,moreBalls);
  }
 
}

function tom2DPlayGui::onWake(%this)
{
   
}

function tom2DPlayGui::onSleep(%this)
{
   
}

//------------------------------------------------------------------------------
/*
function CreateBallBounce() {
   */
   if (!isObject(tom2DPlayGui)) {

new GuiControl(tom2DPlayGui) {
   canSaveDynamicFields = "0";
   Profile = "GuiModelessDialogProfile";
   HorizSizing = "right";
   VertSizing = "bottom";
   position = "0 0";
   Extent = "1024 786";
   BaseExtent = "1024 786";
   MinExtent = "8 8";
   canSave = "1";
   Visible = "1";
   hovertime = "1000";

   new GuiWindowCtrl(wndtom2DGui) {
      canSaveDynamicFields = "0";
      Profile = "GuiWindowProfile";
      HorizSizing = "right";
      VertSizing = "bottom";
      position = "0 0";
      Extent = "1024 786";
      MinExtent = "8 2";
      canSave = "1";
      Visible = "1";
      hovertime = "1000";
      text = "Test tom2D Game";
      maxLength = "1024";
      resizeWidth = "1";
      resizeHeight = "1";
      canMove = "1";
      canClose = "1";
      canMinimize = "0";
      canMaximize = "0";
      minSize = "50 50";
      TitleHeight = 12;
      closeCommand = "Canvas.popDialog(tom2DPlayGui);";
      

   new tom2DCtrl(tom2Dgui) {
      canSaveDynamicFields = "0";
      Profile = "GuiDefaultProfile";
      HorizSizing = "width";
      VertSizing = "height";
      position = "3 25";
      Extent = "394 268";
      Extent = "1018 760";
      BaseExtent = "1018 760"; 
      MinExtent = "8 2";
      canSave = "1";
      Visible = "1";
      hovertime = "1000";
      applyFilterToChildren = "1";
      cameraZRot = "0";
      forceFOV = "30";
   };

   };
};



     // FIXME Canvas.pushDialog(tom2DPlayGui);
     tom2Dgui.w = getWord(tom2Dgui.BaseExtent,0);
     tom2Dgui.h = getWord(tom2Dgui.BaseExtent,1);
     
  
     tom2Dgui.maxBalls = 50;
     tom2Dgui.img_ball = new tom2Dtexture();
     tom2Dgui.img_ball.setBitmap("basicgame/art/shapes/ball/autball");
     tom2DGui.ballCount = 0;
     tom2DGui.moreBalls();
     
     tom2Dgui.img_back = new tom2Dtexture();
     tom2Dgui.img_back.setBitmap("basicgame/art/gui/OGS_bluesplash");
     tom2Dgui.back = new tom2dSprite() {
        size   = tom2Dgui.w SPC tom2Dgui.h;
        Layer = 98;
        x     = tom2Dgui.w / 2;
        y     = tom2Dgui.h / 2;
        Visible = true;
     };
     tom2Dgui.back.setTexture(tom2Dgui.img_back);
     tom2DGui.addRenderObject(tom2Dgui.back);
     
     
      
     %text = new tom2DScriptRenderObject() {
       class = "MyText";
       screen = tom2Dgui;
       Layer = 5;
       Visible = true;
    };
    tom2DGui.addRenderObject(%text);
     
  } else {
 }  
//}
