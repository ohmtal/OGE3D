#include "platform/platform.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/ast.h"
//#include "core/resManager.h"
#include "core/stream/fileStream.h"
#include "console/compiler.h"
//#include "platform/event.h"
//#include "platform/gameInterface.h"
#include "console/engineAPI.h"
#include "md5file.h"

static const char Base16Values[] = "0123456789ABCDEF";
DefineEngineStringlyVariadicFunction(getFileMD5, const char*, 2, 2, "getFileMD5(file)")
{
        char * fileMD5;
		char *returnBuffer = Con::getReturnBuffer( 256 );
        fileMD5 = MD5File( argv[1] ); 
		if (!fileMD5) return "";
        dSprintf( returnBuffer, 256, "%s", fileMD5 );
        dFree(fileMD5);
        return returnBuffer;

}


DefineEngineStringlyVariadicFunction(getMD5, const char*, 2, 2, "getMD5(string)")
{
        char * strMD5;
		char *returnBuffer = Con::getReturnBuffer( 256 );
        strMD5 = MD5String( argv[1] ); 
		if (!strMD5) return "";
        dSprintf( returnBuffer, 256, "%s", strMD5 );
        dFree(strMD5);
        return returnBuffer;
}

