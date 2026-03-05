//-----------------------------------------------------------------------------
// Copyright (c) Jarred Schnelle, updated by Stefan "Beffy" Moises
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
// written by Jarred Schnelle, updated by Stefan "Beffy" Moises
//-----------------------------------------------------------------------------

#ifndef _GUIDRAGDROPBUTTONCTRL_H_
#define _GUIDRAGDROPBUTTONCTRL_H_

#ifndef _GUICONTROL_H_
#include "gui/core/guiControl.h"
#endif
#ifndef _GUIMOUSEEVENTCTRL_H_
#include "gui/utility/guiMouseEventCtrl.h"
#endif



class GuiDragDropButtonCtrl : public GuiMouseEventCtrl
{
private:
	typedef GuiMouseEventCtrl Parent;
    bool mCanMove;          //Can the button your mouse clicks on move at all?
	bool mSnapsBack;        //Does the dragged icon snap back to where it was?
	bool mIsButtonMoving;   //Is the icon(source or copy) moving?
	bool mIsReceiver;       //Is this slot a receiver, as in, can it be written over? 
	bool mCopySource;       //Does this button create copies of itself to be placed, and remain where it is?
	bool mResetSourceOnDrop; //Do we want to clear out the source icon after we've droped it?
	bool mPerformActionOnUp;    //Do we want to perform an action on mouse up? Might not if it's for a power/skill menu
	bool mSwitchImages;     //Do we want it to switch the images between dropper / dropee ?
	SimGroup* mCurrentGroup;
	SimGroup* mTopLevelGroup;
	bool mChangedGroup;
	Point2I mMouseDownPosition;
	Point2I mMouseDownPositionOffset;
	Point2I mChangedGroupPosition;
	RectI mOrigBounds;
    
	//Used for displaying border around specific types of drag/drop icons
	ColorI mDropBorderSearchColor;
	ColorI mDropBorderColor; //Set by siblings
	bool mDropBorder;
	const char* mDropBorderType;
	const char* mDropBorderSearch;
	bool mDropBorderDisplayed; //Set by siblings

    bool mIsDragged; //XXTH onaction only if not dragged!

	//XXTH Cooldown -->
	enum { NUM_COOLDOWN_FRAMES = 36 };

	S32      mTimeOffset;
	S32	     mCoolDown;
	bool     mRunning;
	bool     mReverseAnimation;
	//XXTH <-- Cooldown 
	
protected:
   StringTableEntry mBitmapName;
   StringTableEntry mHoverBitmapName;
   StringTableEntry mDownBitmapName;
   GFXTexHandle mTextureHandle;
   GFXTexHandle mNormalTextureHandle;
   GFXTexHandle mHoverTextureHandle;
   GFXTexHandle mDownTextureHandle;
   Point2I startPoint;
   bool mWrap;

	//XXTH Cooldown -->
   static StringTableEntry sSpellCooldownBitmaps;
   GFXTexHandle     cooldown_txrs[NUM_COOLDOWN_FRAMES];
	//XXTH <-- Cooldown

public:
   //creation methods
   DECLARE_CONOBJECT(GuiDragDropButtonCtrl);
   GuiDragDropButtonCtrl();
   static void initPersistFields();
   static void consoleInit();

   //Parental methods
   bool onWake();
   void onSleep();

   void setBitmapName(const char *name);
   void setBitmapHandle(const GFXTexHandle &handle);

   void setHoverBitmapName(const char *name);
   void setHoverBitmapHandle(const GFXTexHandle &handle);
 
   void setDownBitmapName(const char *name);
   void setDownBitmapHandle(const GFXTexHandle &handle);

   //This is the actual graphic that is shown on the screen
   void setTextureHandle(const GFXTexHandle &handle);
      
   //Getter functions for assignment on MouseUp drop
   const GFXTexHandle getNormalHandle() {return mNormalTextureHandle;}
   const GFXTexHandle getHoverHandle() {return mHoverTextureHandle;}
   const GFXTexHandle getDownHandle() {return mDownTextureHandle;}
   const char *getNormalName() {return mBitmapName;}
   const char *getHoverName() {return mHoverBitmapName;}
   const char *getDownName() {return mDownBitmapName;}


   GuiDragDropButtonCtrl* checkDropPosition(Point2I MouseUpPosition);

   void PlaceTargetImages(GuiDragDropButtonCtrl* DDBCtrl); //XXTH replacement of code inside checkDropPosition


   S32 getWidth() const       { return(mTextureHandle.getWidth()); }
   S32 getHeight() const      { return(mTextureHandle.getHeight()); }

   void onRender(Point2I offset, const RectI &updateRect);
   void setValue(S32 x, S32 y);

   void resetContents();
   void onMouseEnter(const GuiEvent &);
   void onMouseLeave(const GuiEvent &);
   void onMouseDown(const GuiEvent &);
   void onMouseUp(const GuiEvent &);
   void onMouseDragged(const GuiEvent &);
   void onRightMouseDown(const GuiEvent &);

   //Method to draw the dropable borders on drag
   void drawDropBorder(const char*, ColorI);
   void undoDrawDropBorder(const char*);
   void setDropBorderDisplayed(bool);
   void setDropBorderColor(ColorI);

   //Console setter functions
   void setCanMove(bool temp);
   void setIsReceiver(bool temp);
   void setSnapsBack(bool temp);
   void setDropBorder(bool temp);
   void setDropBorderSearchColor(int r, int g, int b);
   void setDropBorderSearchColor(int r, int g, int b, int a);
   void setDropBorderType(const char* temp);
   void setDropBorderSearch(const char* temp);
   
   //Public Getter Methods for Console Commands
   const bool getCanMove() {return mCanMove;}
   const bool getSnapsBack() {return mSnapsBack;}
   const bool getIsReceiver() {return mIsReceiver;}
   const bool getCopySource() {return mCopySource;}
   const bool getDropBorder() {return mDropBorder;}
   const char* getDropBorderType() {return mDropBorderType;}
   const char* getDropBorderSearch() {return mDropBorderSearch;}
   const ColorI getDropBorderSearchColor() {return mDropBorderSearchColor;}

   //Set our own onAction function, so that we may pass variables to the console
   //Doing this so we dont adversly affect the other GuiControls :)
   void onAction();
  

	//XXTH Cooldown -->
   void setReverseTime(S32 newTime);
   void toggleCoolDown(GuiDragDropButtonCtrl *from);

   S32 getCoolDown() { return mCoolDown; }
   S32 getTimeOffset() {return mTimeOffset; }
   bool getRunning() { return  mRunning; }
   bool getReverseAnimation() { return mReverseAnimation; }

   void setCoolDown(S32 val) { mCoolDown=val ; }
   void setTimeOffset(S32 val) {mTimeOffset=val; }
   void setRunning(bool val) { mRunning=val; }
   void setReverseAnimation(bool val) { mReverseAnimation=val; }

   S32  getReverseTime();
   F32  getPercent();
   virtual bool      onAdd();	

	//XXTH <-- Cooldown
};

//XXTH damn FIXME ?!?!?! !!!
extern U32 DragDropArray[GuiControl::mDragDropArraySize];

#endif
