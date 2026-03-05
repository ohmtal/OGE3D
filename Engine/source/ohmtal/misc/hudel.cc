//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//
// This is a the implementation class to make a own protocol for 
// submitting data to a server or saving data.
//
//

#include "hudel.h"
#include "base64.h"


//Torque stuff 
#include "math/mRandom.h"
#include "console/simBase.h"
#include "app/version.h"
#include "core/util/safeDelete.h"
#include "console/engineAPI.h"
//-----------------------------------------------------------------------------
Hudel::Hudel()
{
 

}

Hudel::~Hudel()
{

}
//-----------------------------------------------------------------------------
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// The keys must be on the end in an unused section 
// Last byte is always the package type!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//-----------------------------------------------------------------------------

bool Hudel::setPckType(U8 lType)
{
  mPckType = lType;

 
  //signal where we start to mark the bitset to unused
  //so the keys are the behind the highest byte
  // While Checksum (PosC) and first Payload Position can also in 
  // data range. In Example we use 59 since its not much space behind
  // but highest could be also 61 from mPosM
  //
  //base64 len will be mPacketSite / 3 * 4 , if it not divdeble by 3 base64 fill it up
  //
  // 
  U8 lHighestByte = 0; 

  switch (lType)
  {
	  //NEVER USE 0!!!!

	  case 255:  //ToM Testpaket
			mPacketSize = 70; 
			mPosS = 64;
			mPosM = 61;
			mPosA = 63;
			mPosC = 60;
			mPosD = 59;
			lHighestByte = 59;
		  break;
	  default:
		  return false;

  }

  for (U8 i = lHighestByte; i < mPacketSize; i++)
	  mBitSet[i] = true;

  //This ones are in encoded section:
  mBitSet[mPosC] = true;
  mBitSet[mPosD] = true;

  return true;
}
//-----------------------------------------------------------------------------
// all pos must be different and in range of 1..lSize-1
// I'll NOT CHECK THIS HERE!
bool Hudel::initPck(U8 lPacketType)
{
   dMemset(mBuffer,0,sizeof(mBuffer));
   for (U16 i = 0; i < 256; i++)
		mBitSet[i] = false;


//packet type:
   if (!setPckType(lPacketType))
	   return false;


  //fill buffer with randum junk 
  for (U16 i = 0; i < mPacketSize; i++)
	mBuffer[i]=(char) gRandGen.randI(0,255);


   mSK = gRandGen.randI(1,255);
   mMK = gRandGen.randI(1,255);
   mAK = gRandGen.randI(1,255);
   U8 lCheckSum = (S32)mSK + (S32)mMK + (S32)mAK + 128;

   mBuffer[mPosC] = lCheckSum;

#ifdef TORQUE_DEBUG 
   Con::errorf("KEYS: %d, %d, %d, CHECK: %d",mSK,mMK,mAK,lCheckSum);
#endif

   return true;

}
//-----------------------------------------------------------------------------
// init package from data
bool Hudel::initPck(char* lData, bool lIsBase64, U8 nonBase64len)
{
   dMemset(mBuffer,0,sizeof(mBuffer));
   for (U16 i = 0; i < sizeof(mBitSet); i++)
		mBitSet[i] = false;

   U8 lLen = 0;
   if (lIsBase64)
   {
	    S32 lDatalen = dStrlen(lData);
		if (!Base64::decode((unsigned char*)lData, mBuffer,lDatalen))
			return false;
        S32 lWorkLen = lDatalen / 4 * 3;
		lLen = 0;
		for (S32 i = lWorkLen-1;  i >= 0; i--)
		{
          if (lLen == 0 && mBuffer[i] != 0)
		  {
				lLen = i+1;
				break;
		  }
		}


		
   } else {
		lLen = nonBase64len;
        dMemcpy(mBuffer,lData,lLen);
   }

   U8 lType = mBuffer[lLen - 1];
#ifdef TORQUE_DEBUG 
   Con::errorf("========================================"); 
   
   Con::errorf("DATA LEN : %d, FINAL LEN: %d",dStrlen(lData), lLen);
#endif

   //check for an existing packagetype and if the size match
   if (!setPckType(lType))
	   return false;
   if (lLen != mPacketSize)
	   return false;
 
   //mmhh so far so good
   mSK = mBuffer[mPosS];
   mMK = mBuffer[mPosM];
   mAK = mBuffer[mPosA];
   U8 lCheckSumTest = (S32)mSK + (S32)mMK + (S32)mAK + 128;

   DeHundel(mBuffer,mPacketSize, mSK * 66, mMK, mAK);
   U8 lCheckSum = mBuffer[mPosC];

   if (lCheckSum != lCheckSumTest)
	   return false;



   return true;
}
//-----------------------------------------------------------------------------
// GET BYTE 
U8 Hudel::get(U8 lPos) 
{
	return mBuffer[lPos];
}

//-----------------------------------------------------------------------------
// GET STRING
void Hudel::get(unsigned char* InStr,U8 lPos, U8 lLen)
{
   for (U8 i = 0; i<lLen; i++)
	   InStr[i]= mBuffer[i+lPos];

}

//-----------------------------------------------------------------------------
// GET NUMBER

U32 Hudel::get(U8 lPos, U8 lLen) //Number
{
	
	if (lLen > 4) 
		return false;
	U32 result = 0;

    for (S32 i = 0; i < lLen; i++)
	   result |= (mBuffer[lPos + i] << (i*8));

  
    return result;
}


//-----------------------------------------------------------------------------
// SET BYTE 
bool Hudel::set(U8 lPos, unsigned char lData) 
{
  if (mBitSet[lPos])
  {
	  mBuffer[lPos] = lData;
	  return true;
  } else {
	  return false;
  }
}
//-----------------------------------------------------------------------------
// SET STRING
bool Hudel::set(U8 lPos, unsigned char* lData, U8 lLen) //String WILL be enhudelt
{
   for (U8 i = 0; i<lLen; i++)
	     if (!mBitSet[lPos+lLen])
			 return false;

   unsigned char buf[256];
   dMemset(buf,0,sizeof(buf));
   dMemcpy(buf,lData,lLen);

   for (U8 i = 0; i<lLen; i++)
	   mBuffer[lPos+i]=buf[i];

  return true;

}
//-----------------------------------------------------------------------------
// SET NUMBER
/*
// example to get it back!
//    U32 lBackScore = 0;
//    for (S32 i = 0; i < 4; i++)
//	   lBackScore |= (c_score[i] << (i*8));

*/
bool Hudel::set(U8 lPos, U32 lNumber, U8 lLen) //Number
{
	if (lLen > 4) 
		return false;

	for (S32 i = 0; i < lLen; i++)
		mBuffer[lPos + i] = (lNumber >> (i*8)) & 0xFF;
  
    return true;
}
//-----------------------------------------------------------------------------
// return a content position or -1 if failed!
// it it failed we need to reinit and try again ! 
// the range is optional and set with -1 if not used!
// range must be defined in 0..mPacketSize - 1
S32 Hudel::ReserveContentPosition(U8 lLen, S16 lStartRange, S16 lEndRange )
{
  bool lCheckedPositions[256];
   for (U16 i = 0; i < sizeof(lCheckedPositions); i++)
		lCheckedPositions[i] = mBitSet[i];

   bool lDone = false;
   S32 result = -1;
   S32 lNeedle = 0;

   if (lStartRange < 0 )
	   lStartRange = 0;

   if (lEndRange < 0 )
	   lEndRange = mPacketSize - 1;

   S32 lastStartByte =  lEndRange - lLen + 1;
   
   if (lastStartByte < 0 ) 
	   return -2; //something really wrong here!

   U16 lAvailStartByteCnt = lastStartByte - lStartRange + 1;

   while (!lDone)
   {

     bool lFoundOverlap = false;
     lNeedle = gRandGen.randI(lStartRange,lastStartByte);
	 if (lCheckedPositions[lNeedle]) //we already checked this
		 continue;
	 lCheckedPositions[lNeedle] = true;
	 for (S32 i = lNeedle; i < lNeedle + lLen; i++)
	 {
		 if (mBitSet[i]) {
			 lFoundOverlap = true;
			 break;
		 }
	 }
	 if (!lFoundOverlap) { //found a good position :D
		 lDone = true;
		 result = lNeedle;
	 } else {
        //check if we out of tries:
		 U16 lDoneBytes = 0;
		 for (S32 i = lStartRange; i<= lastStartByte; i++)
			 if (lCheckedPositions[i])
				 lDoneBytes++;

		 if (lDoneBytes >= lAvailStartByteCnt) //we did try out all bytes thats bad! 
			 return -1;

	 }
   } //while !lDone

   //reserve the space of found
   if (result >= 0) 
   {
	 for (S32 i = result; i < result + lLen; i++)
	 {
		 mBitSet[i] = true;
     }
   } //if result

   return result;
} // S32 Hudel::ReserveContentPosition(U16 lLen, lStartRange, lEndRange )

//-----------------------------------------------------------------------------
void Hudel::EnHundel(unsigned char* InStr, S32 inLen,S32 StartKey, S32 MultKey, S32 AddKey)
{
#ifdef TORQUE_DEBUG
	//Con::printf("Hudel::EnHundel inLen=%d, Keys:%d %d %d Str:%s",inLen,StartKey,MultKey,AddKey,InStr);
#endif

	for (S32 i=0; i< inLen; i++)
	{
      InStr[i] = char((U8)InStr[i] ^ (StartKey >> 8));
	  StartKey = ((U8) InStr[i] + StartKey) * MultKey + AddKey;
	}

}
//-----------------------------------------------------------------------------
void Hudel::DeHundel(unsigned char* InStr,S32 inLen, S32 StartKey, S32 MultKey, S32 AddKey)
{
    U8 lsav=0;
	for (S32 i=0; i< inLen; i++)
	{
      lsav = (U8)InStr[i];
      InStr[i] = char((U8)InStr[i] ^ (StartKey >> 8));
	  StartKey = (lsav + StartKey) * MultKey + AddKey;

	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/*
  New bampfer: 
          /starting with 0!
          BYTE  0 appbuf[0]
          BYTE  1 namebuf[0]
          BYTE  2 appbuf[1]
          BYTE  3 addkey
          BYTE  4 -
          BYTE  5 namebuf[1]
          BYTE  6 appbuf[2]
          BYTE  7 namebuf[2]
          BYTE  8 multkey
          BYTE  9 -
          BYTE 10 appbuf[3] 
          BYTE 11 namebuf[3]
          BYTE 12 keys checksum 
          BYTE 13 startkey
          BYTE 14 checksum of bytes 0..12

  eBampf create a new registration key out of:
  * Applicationname <- version.h getVersionString()
  * Fullname <- hudel set/getFullname()

*/
void Hudel::ebampf(char lbampf[16])
{
	char bchars[37]="0123456789abcdefghijklmonpqrstuvwxyz";
	bchars[36] = 0;

	dMemset(lbampf,0,16);
    lbampf[4] = '-';
	lbampf[9] = '-';


    U8 tmpcheck = 0;
	S32 tmplen = 0;

	//**** key bampfs ****
	U8 slulu = gRandGen.randI(2,37); //starkey
	U8 alulu = gRandGen.randI(2,37); //addkey
	U8 mlulu = gRandGen.randI(2,37); //multikey

    lbampf[3]  = bchars [alulu-2];
	lbampf[8]  = bchars [mlulu-2];
	lbampf[13] = bchars [slulu-2];

	tmpcheck = slulu+alulu+mlulu;
	lbampf[12]= bchars [tmpcheck % 36];


    char nbuf[64];dMemset(nbuf,0,64);
    //**** name bampfs ****
	tmplen = dStrlen(getFullName());
	dSprintf(nbuf,64,getFullName());
	EnHundel((unsigned char*)nbuf,tmplen,slulu * 666 ,mlulu,alulu);
    
	lbampf[1]= bchars [(U8)nbuf[0] % 36];
	lbampf[5]= bchars [(U8)nbuf[tmplen-1] % 36];
	lbampf[7]= bchars [(U8)nbuf[(S32)(tmplen / 2)] % 36];

	tmpcheck = 0;
	for (S32 i = 0; i< tmplen; i++)
		tmpcheck += (U8) nbuf[i];
	lbampf[11]= bchars [tmpcheck % 36];

	//**** appname bampfs ****
	tmplen = dStrlen(getVersionString());
	dMemset(nbuf,0,64);
	dSprintf(nbuf,64,"%s",getVersionString());
	EnHundel((unsigned char*)nbuf,tmplen,slulu * 666 ,mlulu,alulu);

	lbampf[0]= bchars [(U8)nbuf[0] % 36];
	lbampf[2]= bchars [(U8)nbuf[tmplen-1] % 36];
	lbampf[6]= bchars [(U8)nbuf[(S32)(tmplen / 2)] % 36];

	tmpcheck = 0;
	for (S32 i = 0; i< tmplen; i++)
		tmpcheck += (U8) nbuf[i];
	lbampf[10]= bchars [tmpcheck % 36];

	// **** final checksum ****
	tmpcheck = 0;
	for (S32 i = 0; i< 14; i++)
		tmpcheck += (U8) lbampf[i];
	lbampf[14]= bchars [tmpcheck % 36];

}
//-----------------------------------------------------------------------------
U8 getbampfpos(char c)
{
	char bchars[37]="0123456789abcdefghijklmonpqrstuvwxyz";

	for (S32 i=0; i<37; i++)
		if (bchars[i] == c)
			return i;

	return 255;

}

bool Hudel::dbampf(const char lin[16])
{
	char bchars[37]="0123456789abcdefghijklmonpqrstuvwxyz";
	char lbampf[16];
	for (S32 i = 0; i<16; i++) {
		lbampf[i]=dTolower(lin[i]);
		if (lbampf[i] != '-' && getbampfpos(lbampf[i]) > 36)
			return false;
	}

	U8 tmpcheck = 0;
	S32 tmplen = 0;

	// **** final checksum ****
	tmpcheck = 0;
	for (S32 i = 0; i< 14; i++)
		tmpcheck += (U8) lbampf[i];
	if (lbampf[14] != bchars [tmpcheck % 36])
		return false;

	//**** key bampfs ****
	U8 slulu = getbampfpos(lbampf[13]) + 2;
	U8 alulu = getbampfpos(lbampf[3]) + 2;
	U8 mlulu = getbampfpos(lbampf[8]) + 2;


	tmpcheck = slulu+alulu+mlulu;
	if (lbampf[12] != bchars [tmpcheck % 36])
		return false;

    char nbuf[64];dMemset(nbuf,0,64);
    //**** name bampfs ****
	tmplen = dStrlen(getFullName());
	dSprintf(nbuf,64,getFullName());
	EnHundel((unsigned char*)nbuf,tmplen,slulu * 666 ,mlulu,alulu);
    
	if (lbampf[1] != bchars [(U8)nbuf[0] % 36])
		return false;

	if (lbampf[5] != bchars [(U8)nbuf[tmplen-1] % 36])
		return false;

	if (lbampf[7] != bchars [(U8)nbuf[(S32)(tmplen / 2)] % 36])
		return false;

	tmpcheck = 0;
	for (S32 i = 0; i< tmplen; i++)
		tmpcheck += (U8) nbuf[i];
	if (lbampf[11] != bchars [tmpcheck % 36])
		return false;

	//**** appname bampfs ****
	tmplen = dStrlen(getVersionString());
	dMemset(nbuf,0,64);
	dSprintf(nbuf,64,"%s",getVersionString());
	EnHundel((unsigned char*)nbuf,tmplen,slulu * 666 ,mlulu,alulu);

	if (lbampf[0] != bchars [(U8)nbuf[0] % 36])
		return false;

	if (lbampf[2] != bchars [(U8)nbuf[tmplen-1] % 36])
		return false;

	if (lbampf[6] != bchars [(U8)nbuf[(S32)(tmplen / 2)] % 36])
		return false;

	tmpcheck = 0;
	for (S32 i = 0; i< tmplen; i++)
		tmpcheck += (U8) nbuf[i];
	if (lbampf[10] != bchars [tmpcheck % 36])
		return false;


    return true;
}
//-----------------------------------------------------------------------------
char Hudel::getSubByte(char* lStr,S32 lPos)
{
   char buf[2];
   dStrncpy(buf,lStr+lPos,2);
   S32 n = dAtoi(buf);
   return (char) n;
}
//-----------------------------------------------------------------------------
void Hudel::getFinal(char* OutBuf, bool base64)
{
   //here we go:
   unsigned char buf[256];
   dMemset(buf,0,sizeof(buf));

   dMemcpy(buf,mBuffer,mPacketSize);

   //startkey is multiplyed by 66!
   EnHundel(buf,mPacketSize,(S32)mSK * 66, mMK, mAK);

   //fill in the packettype
   buf[mPacketSize - 1] = mPckType;

   //fill in the keys
   buf[mPosS] = mSK; 
   buf[mPosM] = mMK; 
   buf[mPosA] = mAK; 

   char* result;
   if (base64) {
		Base64::encode(buf,(unsigned char*)OutBuf, mPacketSize);
   } else {
	   dMemcpy(OutBuf, buf,mPacketSize);
   }
	   
}
//===============================================================================================================================
#ifdef TORQUE_DEBUG
/**
* from https://gist.github.com/ircmaxell/c26ff31a80ac69b1349a
* there are no licence informations.... the site is about sharing code, notes, and snippets. 
*/
static char* php_bin2hex(const unsigned char* old, const size_t oldlen)
{
   static char hexconvtab[] = "0123456789abcdef";
   char* result = (char*)dMalloc(oldlen * 2 + 1);
   size_t i, j;

   for (i = j = 0; i < oldlen; i++) {
      result[j++] = hexconvtab[old[i] >> 4];
      result[j++] = hexconvtab[old[i] & 15];
   }
   result[j] = '\0';
   return result;
}


static char* php_hex2bin(const unsigned char* old, const size_t oldlen)
{
   size_t target_length = oldlen >> 1;
   char* str = (char*)dMalloc(target_length);
   //orig: unsigned char* ret = str;
   char* ret = str;
   size_t i, j;

   for (i = j = 0; i < target_length; i++) {
      unsigned char c = old[j++];
      unsigned char d;

      if (c >= '0' && c <= '9') {
         d = (c - '0') << 4;
      }
      else if (c >= 'a' && c <= 'f') {
         d = (c - 'a' + 10) << 4;
      }
      else if (c >= 'A' && c <= 'F') {
         d = (c - 'A' + 10) << 4;
      }
      else {
         free(str);
         return NULL;
      }
      c = old[j++];
      if (c >= '0' && c <= '9') {
         d |= c - '0';
      }
      else if (c >= 'a' && c <= 'f') {
         d |= c - 'a' + 10;
      }
      else if (c >= 'A' && c <= 'F') {
         d |= c - 'A' + 10;
      }
      else {
         free(str);
         return NULL;
      }
      ret[i] = d;
   }
   ret[i] = '\0';

   return str;
}
#endif
//===============================================================================================================================
void Hudel::SimpleEnHudle(char * input,char * result,  char myWord[4])
{
	mSK = myWord[0];
	mMK = myWord[1];
	mAK = myWord[2];

	S32 lStrLen = dStrlen(input); //fix for "0" chars after hudle
	EnHundel((unsigned char*)input,lStrLen ,mSK, mMK, mAK);

#ifdef TORQUE_DEBUG
   Con::errorf("phpstyle bintohex:%s", php_bin2hex((unsigned char*)input,lStrLen));
#endif // TORQUE_DEBUG

	Base64::encode((unsigned char*)input,(unsigned char*)result, lStrLen );
	   
}
void Hudel::SimpleDeHudle(char * input,char * result,  char myWord[4])
{
	mSK = myWord[0];
	mMK = myWord[1];
	mAK = myWord[2];

	S32 lStrLen = 0; //fix for "0" chars after hudle

	lStrLen = Base64::decode((unsigned char*)input, (unsigned char*)result,dStrlen(input));
	if (!lStrLen)
			return ;

	DeHundel((unsigned char*)result,lStrLen, mSK , mMK, mAK);
	   
}

#ifdef TORQUE_DEBUG
DefineEngineStringlyVariadicFunction(TestSimpleHudle,bool,1,1,"")
{
	Hudel* h = new Hudel;
	char inputbuf[256];
    dSprintf(inputbuf,sizeof(inputbuf),"Hello World");

	char resultbuf[256];
	char myWord[4] = "abc" ;

	h->SimpleEnHudle(inputbuf,resultbuf,myWord);
	h->SimpleDeHudle(resultbuf,inputbuf,myWord);

	SAFE_DELETE(h);
	return true;
}
#endif

DefineEngineStringlyVariadicFunction(HudleLine,const char *,2,3,"(string, *char3Key)")
{
	char *returnBuffer = Con::getReturnBuffer( 256 );
	static char inputbuf[256];

   dStrcpy(inputbuf, argv[1],sizeof(inputbuf));
   static char myWord[4] = "xab";
   if (argc == 3)
      dStrcpy(myWord, argv[2], sizeof(myWord));
   else
      dStrcpy(myWord, "xab", sizeof(myWord));

   //XXTH THIS SUCKS WHEN A % is in the string!! 
  /*
   dSprintf(inputbuf,sizeof(inputbuf),argv[1]);
  static char myWord[4] = "xab" ;
   if (argc == 3)
     dSprintf(myWord,sizeof(myWord),argv[2]);
  else
     dSprintf(myWord,sizeof(myWord),"xab");
   */

	Hudel* h = new Hudel;
	h->SimpleEnHudle(inputbuf,returnBuffer,myWord);

	SAFE_DELETE(h);
	return returnBuffer;
}
DefineEngineStringlyVariadicFunction(DeHudleLine,const char *,2,3,"(string, *char3Key)")
{
	char *returnBuffer = Con::getReturnBuffer( 256 );
	static char inputbuf[256];

   dStrcpy(inputbuf, argv[1], sizeof(inputbuf));
   static char myWord[4] = "xab";
   if (argc == 3)
      dStrcpy(myWord, argv[2], sizeof(myWord));
   else
      dStrcpy(myWord, "xab", sizeof(myWord));

   //XXTH THIS SUCKS WHEN A % is in the string!!
   /* XXTH THIS SUCKS WHEN A % is in the string!!
     dSprintf(inputbuf,sizeof(inputbuf),argv[1]);
   static char myWord[4] = "xab" ;
    if (argc == 3)
      dSprintf(myWord,sizeof(myWord),argv[2]);
   else
      dSprintf(myWord,sizeof(myWord),"xab");
    */

	Hudel* h = new Hudel;
	h->SimpleDeHudle(inputbuf,returnBuffer,myWord);

	SAFE_DELETE(h);
	return returnBuffer;
}

//===========================================================================================


//-----------------------------------------------------------------------------
/* unused old bampf stuff: 
bool Hudel::bampfer()
{

	char * lSerial  = "7242-1091-885523";
	char * lName    = "Kall Npp";

	char * lAppName = dStrdup(getVersionString());
	char buf[256];dMemset(buf,0,255);
	char tmpstr2[13];dMemset(tmpstr2,0,13);
	dSprintf(tmpstr2,sizeof(tmpstr2),"%d%d%d",1969,2705,1997);
	
	buf[0]=lSerial[7];
	buf[1]=lSerial[8];
	S32 lulu = dAtoi(buf);

	dMemset(buf,0,256);
	S32 lenAppName = dStrlen(lAppName);
	S32 lenName    = dStrlen(lName);
	for (S32 i = 0; i < 6 || i >=lenAppName ; i++)
		buf[i]=dTolower(lAppName[i]);

	S32 lneedle = (lenAppName>=6) ? 6 : lenAppName;
	for (S32 i = 0; i < 6 || i >=lenName ; i++)
		buf[i+lneedle]=dTolower(lName[i]);

	S32 tmplen = dStrlen(buf);
	for (S32 i = 0; i < tmplen; i++)
	{
		buf[i] = (U8) lulu ^ (U8) buf[i];
	}
    char digitbuf[4];dMemset(digitbuf,0,4);
	for (S32 i = tmplen-1; i>=0; i--)
	{
		dSprintf(digitbuf,sizeof(digitbuf),"%d",(U8) buf[i]);
		tmpstr2[i] = digitbuf[1];
	}
	dMemset(buf,0,256);
    buf[0]=tmpstr2[0];  
	buf[1]=tmpstr2[1];  
	buf[2]=tmpstr2[2];  
	buf[3]=tmpstr2[3];  
	buf[4]='-';  
	buf[5]=tmpstr2[4];  
	buf[6]=tmpstr2[5];  
	dMemset(digitbuf,0,4);
	dSprintf(digitbuf,sizeof(digitbuf),"%d",lulu);
    buf[7]=digitbuf[0];  
	buf[8]=digitbuf[1];  
	buf[9]='-';  
	buf[10]=tmpstr2[6];  
	buf[11]=tmpstr2[7];  
	buf[12]=tmpstr2[8];  
	buf[13]=tmpstr2[9];  
	buf[14]=tmpstr2[10];  
	buf[15]=tmpstr2[11];  
	buf[16]=0;  
	

	Con::errorf("BAMPFER %s",buf);
  // getVersionString
  dFree(lAppName);

  return dStrcmp(lSerial,buf) == 0;
  
}
// warning result must be cleared with dFree !!!!
// return an 89 byte null terminated base 64 encoded char* 
char* ToMHandler::h_i_packet()
{

#pragma message("FIXME HARDCODED STUFF!")
char * lSerial="3833-1983-990202";
U16 lVersion = getVersionNumber();
U32 lScore = 750000;
U8 lLevel = 1;

    unsigned char c_score[4];
	for (S32 i = 0; i < 4; i++)
		c_score[i] = (lScore >> (i*8)) & 0xFF;

// example to get it back!
//    U32 lBackScore = 0;
//    for (S32 i = 0; i < 4; i++)
//	   lBackScore |= (c_score[i] << (i*8));

	unsigned char c_version[2];
	for (S32 i = 0; i < 2; i++)
		c_version[i] = (lVersion >> (i*8)) & 0xFF;


	unsigned char c_serial[7];
	c_serial[0]=getSubByte(lSerial,0);	
	c_serial[1]=getSubByte(lSerial,2);	
	c_serial[2]=getSubByte(lSerial,5);	
	c_serial[3]=getSubByte(lSerial,7);
	c_serial[4]=getSubByte(lSerial,10);	
	c_serial[5]=getSubByte(lSerial,12);	
	c_serial[6]=getSubByte(lSerial,14);	

  

	char pck[67];
	pck[66] = NULL;
	//fill packet with random chunk
	for (S32 i = 0; i<66; i++)
		pck[i]=(char) gRandGen.randI(1,255);

	U8 serialpos = 0;
	U8 scorepos = 0;
	// PAYLOAD 1: BYTE 60     -> 1 byte payload 2 position  
    serialpos = gRandGen.randI(0,23);
	pck[59] = (char) serialpos;
    // PAYLOAD 2: BYTE 0..31  (start 0..23)   -> 7 byte serial + 1 byte payload II position
	for (S32 i = 0; i<8; i++)
		pck[i+serialpos] = c_serial[i];


	scorepos = gRandGen.randI(32,50);
	pck[serialpos+8] = (char) scorepos;

#ifdef TORQUE_DEBUG
	Con::errorf("XXTH SERIAL POSITION: %d",serialpos); 
	Con::errorf("XXTH SCORE POSITION: %d",scorepos); 
#endif

    // PAYLOAD 3: BYTE 32..59 (start 32..50)  -> 4 byte score + 2 byte version + 1byte level
	for (S32 i = 0; i<4; i++)
		pck[i+scorepos] = c_score[i];
	scorepos+=4;
	for (S32 i = 0; i<2; i++)
		pck[i+scorepos] = c_version[i];
    scorepos+=2;
	pck[scorepos] = (char) lLevel;

	S32 lStartKey = gRandGen.randI(1,255);
	S32 lMultKey = gRandGen.randI(1,255);
	S32 lAddKey  = gRandGen.randI(1,255);

    U8  lChkSum  =  lStartKey+ lMultKey + lAddKey + 128;
	pck[60] = lChkSum;

   // MyEnHundel((unsigned char*)pck, 66, 991,234,1234);
	MyEnHundel((unsigned char*)pck, 66, lStartKey * 66, lMultKey,lAddKey);

	pck[64] = (U8) lStartKey; // Byte 65
	pck[61] = (U8) lMultKey;  // Byte 62
	pck[63] = (U8) lAddKey;   // Byte 64

   char result[89]; //is 88 bytes ? 64/3 ~=22 * 4 => USING 66!
   result[88]=NULL;
   char* dummy = Base64::encode((unsigned char*)pck,66);
   return dummy;
   

}

*/
