//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// CoreNetConnection
// T.Huehn 2009 (XXTH)
// 
// The real minimal netconnection - not compatible with gameconnection.
// The attempt is to have a miniminimal connection for commandtoserver/client
// this could be the base to overwrite gameconnection to for different net 
// handling. 
//-----------------------------------------------------------------------------

#ifndef _CORENETCONNECTION_H_
#define _CORENETCONNECTION_H_

#ifndef _NETCONNECTION_H_
#include "sim/netConnection.h"
#endif

class CoreNetConnection : public NetConnection
{
private:
   typedef NetConnection Parent;

   U32 mNextUpdate;
   bool mClientAFK;
   bool mDebug; //console spam 
public:
	DECLARE_CONOBJECT(CoreNetConnection);

	CoreNetConnection();

    


	bool canRemoteCreate();
   /// @name Event Handling
   /// @{

   virtual void onTimedOut();
   virtual void onConnectTimedOut();
   virtual void onDisconnect(const char *reason);
   virtual void onConnectionRejected(const char *reason);
   virtual void onConnectionEstablished(bool isInitiator);
   virtual void handleStartupError(const char *errorString);

   virtual bool readConnectRequest(BitStream *stream, const char **errorString);

   void onRemove();

   virtual void checkPacketSend(bool force); //need it virtual in netconnection also!
   virtual void handlePacket(BitStream *stream); //need it virtual in netconnection also!

   void setDebug(bool newState) { mDebug=newState; }
   bool getDebug() { return mDebug; }

   void setClientAFK(bool newState);
   bool getClientAFK() { return mClientAFK; }
protected:
   virtual void connectionError (const char *errorString);

}; //class  CoreNetConnection

#endif
