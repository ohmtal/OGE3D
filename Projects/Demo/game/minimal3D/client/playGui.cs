//-----------------------------------------------------------------------------
// Copyright (c) 2023 Ohmtal Game Studio
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

/*
exec("minimal3D/client/playgui.cs");
*/

function PlayGui::onWake(%this)
{

   // Turn off any shell sounds...
   // sfxStop( ... );

   $enableDirectInput = "1";
   activateDirectInput();

   /* FIXME CHAT !! 
   // Message hud dialog
   if ( isObject( MainChatHud ) )
   {
      Canvas.pushDialog( MainChatHud );
      chatHud.attach(HudMessageVector);
   }      
   */
   
    playgui.noCursor = 0;
    Canvas.checkCursor();
   
   
   // setup shapenamehud
   myShapenameHud.setLosMask(0); // BIT( 2 ) is for terrain 
   
   // just update the action map here
   moveMap.push();

   // hack city - these controls are floating around and need to be clamped
   if ( isFunction( "refreshCenterTextCtrl" ) )
      schedule(0, 0, "refreshCenterTextCtrl");
   if ( isFunction( "refreshBottomTextCtrl" ) )
      schedule(0, 0, "refreshBottomTextCtrl");
}

function PlayGui::onSleep(%this)
{

   if ( isObject( MainChatHud ) )
      Canvas.popDialog( MainChatHud );
   
   // pop the keymaps
   moveMap.pop();
}

function PlayGui::clearHud( %this )
{
   Canvas.popDialog( MainChatHud );

   while ( %this.getCount() > 0 )
      %this.getObject( 0 ).delete();
}



//-----------------------------------------------------------------------------

function refreshBottomTextCtrl()
{
   BottomPrintText.position = "0 0";
}

function refreshCenterTextCtrl()
{
   CenterPrintText.position = "0 0";
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
function PlayGui::onMousePanDisplay(%this, %enable, %flags)
{
   if(%flags & 2)
      moveleft(%enable);
   if(%flags & 4)
      moveright(%enable);
   if(%flags & 8)
      moveforward(%enable);
   if(%flags & 16)
      movebackward(%enable);
}
//------------------------------------------------------------------------------
// PlayGUI select
//------------------------------------------------------------------------------
function PlayGui::onRightClickObject(%this,%obj)
{
    //FIXME AFX selectron and notify server!!! 
}


function PlayGui::onDragListStart(%this)
{
    //FIXME clear selection SimSet  
    dEcho("PlayGui::onDragListStart >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
}

function PlayGui::onDragListObject(%this,%obj)
{
    //FIXME Fill the SimSet with the selected objects! and set selectrons ?!
    //remove this debug shit again !! >>> 
      %ghostid = ServerConnection.getGhostID(%obj.getid());
      %serverObj = cl_visitor.resolveObjectFromGhostIndex(%ghostid);
      dEcho("<><><>PlayGui::onDragListObject :" SPC %obj SPC "SERVER:" SPC %serverObj);
    //<<<remove 
       

}

function PlayGui::onDragListEnd(%this)
{
   //FIXME notify server
   dEcho("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<PlayGui::onDragListEnd");
}

