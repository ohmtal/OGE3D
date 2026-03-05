#ifndef _OHMTALGLOBALS_H_
#define _OHMTALGLOBALS_H_

class OhmtalGlobals {
   private:
      static bool mDevelMode;
public:
   static void init();
   static inline bool getDevelMode() { return mDevelMode;  }
   static inline void setDevelMode(bool lDevelMode) { mDevelMode = lDevelMode; }


}; //class
#endif

