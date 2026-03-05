//------------------------------------------------------------------------------
// gameHud2d handler
// exec("minimal3D/client/gameHud2D.cs");
//------------------------------------------------------------------------------
//const 
  $textAlignLeft = 0;
  $textAlignMiddle = 1;
  $textAlignRight = 2;
  $textAlignCenter = 3;
//------------------------------------------------------------------------------
function Game2DHud::createStatText(%this)
{

dWarn("********************* Game2DHud::createStatText ********************************");

   %this.statText = new tom2DShadowText() {
      text = "Hello World";
      x = 10;
      y = getWord(%this.getExtent(),1) - 20;
      fontFace = "Courier New";
      fontSize = 16;
      align    = $textAlignLeft;
      shadowOffset = "1 1";
      textColor   = "250 250 250";
      shadowColor = "20 20 20";
   };
   /* hint
      to change ths font use: 
         Game2DHud.statText.setFont("Courier New",20);
   */ 
   
   $bla = %this.statText;
   
   %this.addRenderObject(%this.statText);
}
//------------------------------------------------------------------------------
function Game2DHud::onResize(%this)
{
 %this.statText.y = getWord(%this.getExtent(),1) - 20;
}
//------------------------------------------------------------------------------
function Game2DHud::onUpdate(%this,%dt)
{

   if (!isObject(%this.statText)) 
      %this.createStatText();

 
 %this.statText.text = strformat("fps:%4.0f", $fps::real)
      @ strformat(", poly:%8d", $GFXDeviceStatistics::polyCount)
      @ strformat(", ghosts:%4d", ServerConnection.getGhostsActive())
      ;
      
}


