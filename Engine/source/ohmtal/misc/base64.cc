#include "base64.h"
#include "console/console.h"
#include "console/engineAPI.h"

DefineEngineStringlyVariadicFunction(base64Encode,const char *,2,2,"(string)")
{
	char *returnBuffer = Con::getReturnBuffer( 256 );
	static char inputbuf[256];
	//NO! dSprintf(inputbuf,sizeof(inputbuf),argv[1]);
   dStrcpy(inputbuf, argv[1], sizeof(inputbuf));
   

    Base64::encode((unsigned char*)inputbuf,(unsigned char*)returnBuffer, dStrlen(inputbuf));

	return returnBuffer;
}

DefineEngineStringlyVariadicFunction(base64Decode,const char *,2,2,"(string)")
{
	char *returnBuffer = Con::getReturnBuffer( 256 );
	static char inputbuf[256];
	//NO! dSprintf(inputbuf,sizeof(inputbuf),argv[1]);
   dStrcpy(inputbuf, argv[1], sizeof(inputbuf));

    if (Base64::decode((unsigned char*)inputbuf,(unsigned char*)returnBuffer, dStrlen(inputbuf))==0)
	{
		return "";
	}

	return returnBuffer;
}

