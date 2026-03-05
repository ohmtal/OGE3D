/*
  BASE64
  http://base64.sourceforge.net/b64.c
  merged and new encode/decode functions by t.huehn (XXTH)
*/


#ifndef BASE64_CDECODE_H
#define BASE64_CDECODE_H

#ifndef _MMATHFN_H_
#include "math/mMathFn.h"
#endif

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif


#ifndef _HTUTILS_H_
#include "htutils.h"
#endif


#ifdef TORQUE_DEBUG 
#ifndef _SIMBASE_H_
#include "console/simBase.h"
#endif
#endif

namespace Base64
{
/*
** Translation Table as described in RFC1113
*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author)
*/
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/
inline void encodeblock( unsigned char in[3], unsigned char out[4], S32 len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}


inline void encode(const unsigned char* InStr,unsigned char* OutStr, S32 len)
{

   F32 lLen =  len;

   S32 blocks =  (lLen + 2.f ) / 3.f; //<< THIS!! 

   S32 outLen = blocks * 4;
   
   S32 bytesleft = len;
   S32 bytestodo = 0;
   unsigned char in[3], out[4];
   U32 lNeedle = 0;
   for (S32 i=0; i<blocks; i++)
   {
	  bytestodo = 0;
	  for (S32 j=0; j<3; j++)
	  {
		if (bytesleft>0)
		{
			in[j]=InStr[i*3+j];
			bytesleft--;bytestodo++;
		}
	  }
	  Base64::encodeblock( in, out, bytestodo );
      for (U8 k=0; k<4; k++)
	  {
          OutStr[lNeedle]=out[k];
		  lNeedle++;
	  }
   }

   OutStr[outLen] = 0;

}



/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
inline void decodeblock( unsigned char in[4], unsigned char out[3] )
{   
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

inline S32 decode(const unsigned char* InStr,unsigned char* OutStr, S32 len)
{
   S32 blocks = (S32)(len / 4.f);
   if ((F32) blocks != HT::mCeilf(len / 4.f) )
   {
	 return 0; //invalid base 64 string! must be dividable by 4!
   }
   S32 outLen = blocks * 3;
   unsigned char in[4], out[3], v;
   S32 bytestodo = 0;
   S32 lNeedle = 0;
   for (S32 i=0; i<blocks; i++)
   {
      S32 bytestodo = 0;
	  for (S32 j=0; j<4; j++) 
	  {

		// in[j]=InStr[i*4+j];

		  v = (unsigned char) InStr[i*4+j];
		  v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
          if( v ) 
			  v = (unsigned char) ((v == '$') ? 0 : v - 61);

		  if (v) {
			in[ j ] = (unsigned char) (v - 1);
			bytestodo++;
		  }
	  }
	  Base64::decodeblock( in, out);
      for (U8 k=0; k<bytestodo-1; k++)
	  {
          OutStr[lNeedle]=out[k];
		  lNeedle++;
	  }
   }
   for (U32 k=lNeedle; k<=outLen; k++)
		OutStr[k] = 0;

  return lNeedle;
}

} //namespace
#endif /* BASE64_CENCODE_H */

