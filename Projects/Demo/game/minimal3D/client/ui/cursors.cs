// -----------------------------------------------------------------------------
// Cursors


//grrr safeDelete(DefaultCursor);
//new GuiCursor( DefaultCursor ) {hotSpot = "1 1"; bitmapName = "~/art/gui/CUR_3darrow";};

//new GuiCursor( DefaultCursor ) {
   DefaultCursor.hotSpot = "1 1";
   DefaultCursor.renderOffset = "0 0";
   DefaultCursor.bitmapName = "./CUR_3darrow";
//   DefaultCursor.bitmapName = "~/art/gui/CUR_arrow2";
//};

new GuiCursor( AltDefaultCursor) {
   hotSpot = "1 1";
   renderOffset = "0 0";
   bitmapName = "./CUR_3darrow";
};


new GuiCursor( HandCursor) {
   hotSpot = "1 1";
   renderOffset = "0 0";
   bitmapName = "./CUR_hand";
};

new GuiCursor( UseCursor) {
   hotSpot = "1 1";
   renderOffset = "0 0";
   bitmapName = "./CUR_grab";
};

new GuiCursor( MoveCursor ) {
   hotSpot = "1 1";
   renderOffset = "0 0";
   bitmapName = "./CUR_Blank";
};	
