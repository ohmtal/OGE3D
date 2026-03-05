//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
// This is a the implementation class to make a own protocol for 
// submitting data to a server or saving data.
//
// Maximum package size is 256 (0..255) !
// combine multiple packages if you need more!
//
// The keys must be on the end in an unused section 
// Last byte is always the package type!
//-----------------------------------------------------------------------------

#ifndef _HUDEL_H_
#define _HUDEL_H_


#ifndef _TORQUE_TYPES_H_
#include "platform/types.h"
#endif

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#ifndef _STRINGFUNCTIONS_H_
#include "core/strings/stringFunctions.h"

#endif

class Hudel
{
protected:
	unsigned char mBuffer[256];
	bool mBitSet[256];

	U8 mSK,mMK,mAK; //,mCS;

	U8 mPckType;

	//for bampf:
	char mFullName[64];

	//S=startkey, M=multikey, A=addkey, C=KeyChecksum, D = first data
	U8 mPacketSize, mPosS, mPosM, mPosA, mPosC, mPosD; 

private:

	void EnHundel(unsigned char* InStr, S32 inLen,S32 StartKey, S32 MultKey, S32 AddKey);
	void DeHundel(unsigned char* InStr,S32 inLen, S32 StartKey, S32 MultKey, S32 AddKey);

	//bampf stuff ! 
	void ebampf(char lbampf[16]);
	bool dbampf(const char lin[16]);
	//unused! bool bampfer();  //old method!

public:
	Hudel();
	~Hudel();

	static char getSubByte(char* lStr,S32 lPos);

	bool setPckType(U8 lType);
    bool initPck(U8 lPacketType);
	S32  ReserveContentPosition(U8 lLen, S16 lStartRange = -1 , S16 lEndRange = -1);
	bool set(U8 lPos, unsigned char lData); //Byte
	bool set(U8 lPos, unsigned char* lData, U8 lLen); //String
	bool set(U8 lPos, U32 lNumber, U8 lLen); //Number

    

	void getFinal(char* OutBuf, bool base64 = true);
    
	//init pck from a package to read it
     bool initPck(char* lData, bool lIsBase64 = true, U8 nonBase64len = 0);
     U8 get(U8 lPos);
     void get(unsigned char* InStr,U8 lPos, U8 lLen);
	 U32 get(U8 lPos, U8 lLen); //Number

	//for bampf:
	char* getFullName() { return mFullName; };
	void setFullName(char *lFullName) { dSprintf(mFullName,64,"%s",lFullName); }

	//simple
	void SimpleEnHudle(char * input, char * result, char myWord[4]);
	void SimpleDeHudle(char * input,char * result,  char myWord[4]);

}; //class Hudel

#endif
