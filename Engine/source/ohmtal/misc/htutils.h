#ifndef _HTUTILS_H_
#define _HTUTILS_H_

#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif


namespace HT
{
//--------------------------------------------------------------------------
inline void writeU16(U8 *buffer, U16 value, U32 pos)
{
	//eeeps need something better!
	for (U8 i = 0; i < 2; i++)
		buffer[pos + i] = (value >> (i*8)) & 0xFF;
  
}
//--------------------------------------------------------------------------
inline void writeU32(U8 *buffer, U32 value, U32 pos)
{
	for (U8 i = 0; i < 4; i++)
		buffer[pos + i] = (value >> (i*8)) & 0xFF;
  
}
//--------------------------------------------------------------------------
inline U16 readU16(U8 *buffer, U32 pos)
{
     U16 result = 0; 
     for (U8 i = 0; i < 2; i++)
	    result |= (buffer[pos + i] << (i*8));

     return result;
  
}
//--------------------------------------------------------------------------
inline U32 readU32(U8 *buffer, U32 pos)
{
     U32 result = 0; 
     for (U8 i = 0; i < 4; i++)
	    result |= (buffer[pos + i] << (i*8));

     return result;
  
}

//--------------------------------------------------------------------------
inline S32 getHexVal(char c)
{
   if(c >= '0' && c <= '9')
      return c - '0';
   else if(c >= 'A' && c <= 'Z')
      return c - 'A' + 10;
   else if(c >= 'a' && c <= 'z')
      return c - 'a' + 10;
   return -1;
}

//--------------------------------------------------------------------------
inline bool scanforchar(const char *str, U32 &idx, char c)
{
   U32 startidx = idx;
   while(str[idx] != c && str[idx] && str[idx] != ':' && str[idx] != '>' && str[idx] != '\n')
      idx++;
   return str[idx] == c && startidx != idx;
}


inline F32 mCeilf(const F32 val) //XXTH 
{
   return ceilf(val);
}

//XXTH mRound
inline S32 mRound(const F32 val)
{
   if (val - mFloor(val) < 0.5)
      return (S32)mFloor(val);
   else
      return (S32)mCeil(val);
}

/* FIXME 
inline void forwardslash(char* str)
{
   while (*str)
   {
      if (*str == '\\')
         *str = '/';
      str++;
   }
}

// UNTESTED USE WITH CATE
inline String fileBase( const char* fileName)
{
   S32 pathLen = dStrlen(fileName);
   FrameTemp<char> szPathCopy(pathLen + 1);

   dStrcpy(szPathCopy, fileName, pathLen + 1);
   forwardslash(szPathCopy);

   const char* path = dStrrchr(szPathCopy, '/');
   if (!path)
      path = szPathCopy;
   else
      path++;
   dsize_t retLen = dStrlen(path) + 1;
   char* ret = Con::getReturnBuffer(retLen);
   dStrcpy(ret, path, retLen);
   char* ext = dStrrchr(ret, '.');
   if (ext)
      *ext = 0;
   return ret;

}
*/



} //namespace HT
#endif
