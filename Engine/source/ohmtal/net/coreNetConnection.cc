//-----------------------------------------------------------------------------
//  License at: ohmtal/misc/ohmtalMIT.h
//-----------------------------------------------------------------------------
// CoreNetConnection
// T.Huehn 2009 (XXTH)
// 
// ---
// 
// 2012 NOTE:
//		Remote Connection only works on two different clients... dont ask me why
//		Cost me 3 hours to find this out.  
// 
// ===> 2023 Solution: you need to set setNetPort(0); on client!!!!!  -- took me some time again ;)
// ---
// The real minimal netconnection - not compatible with NetConnection.
// The attempt is to have a minimal connection for commandtoserver/client
// this could be the base to overwrite NetConnection to for different net 
// handling. 
//
// Need a modification in netConnection.h to overwrite checkPacketSend:
//
//  virtual void checkPacketSend(bool force);
//  virtual void handlePacket(BitStream *stream);

// added afk to raise the delay do not know if this would also work on a server
// because there is too much data for the client while the server need nothing
// from a AFK client. 

// perfect would be, if I find out if there is something in the event or ghost
// queue and force the send and let it sleep for a lets say 10 sec heardbeat
// if there is nothing.

//-----------------------------------------------------------------------------
#include "coreNetConnection.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"
#include "console/simBase.h"
#include "core/stream/bitStream.h"

IMPLEMENT_CONOBJECT(CoreNetConnection);
//-----------------------------------------------------------------------------
// REQUIRED STUFF:
//-----------------------------------------------------------------------------
CoreNetConnection::CoreNetConnection() 
{
	mNextUpdate = 0; 
	mClientAFK  = false;
    mDebug      = false;
}
//-----------------------------------------------------------------------------
bool CoreNetConnection::canRemoteCreate()
{
   return true;
}
//-----------------------------------------------------------------------------
void CoreNetConnection::onConnectionEstablished(bool isInitiator)
{
   if(isInitiator)
   {
      setGhostFrom(false);
      setGhostTo(true);
      setSendingEvents(true);
      setTranslatesStrings(true);
      setIsConnectionToServer();
      mServerConnection = this;
      Con::printf("Connection established %d", getId());
      Con::executef(this,  "onConnectionAccepted");
   }
   else
   {
      setGhostFrom(true);
      setGhostTo(false);
      setSendingEvents(true);
      setTranslatesStrings(true);
      Sim::getClientGroup()->addObject(this);

      Con::executef(this,  "onConnect");

   }
}
//-----------------------------------------------------------------------------
// OPTIONAL STUFF:
//-----------------------------------------------------------------------------
void CoreNetConnection::onTimedOut()
{
   if(isConnectionToServer())
   {
      Con::printf("Connection to server timed out");
      Con::executef(this,  "onConnectionTimedOut");
   }
   else
   {
      Con::printf("Client %d timed out.", getId());
//      setDisconnectReason("TimedOut");
   }

}
//-----------------------------------------------------------------------------
void CoreNetConnection::onConnectTimedOut()
{
   Con::executef(this,  "onConnectRequestTimedOut");
}
//-----------------------------------------------------------------------------
void CoreNetConnection::onDisconnect(const char *reason)
{
   if(isConnectionToServer())
   {
      Con::printf("Connection with server lost.");
      Con::executef(this,  "onConnectionDropped", reason);
   }
   else
   {
      Con::printf("Client %d disconnected.", getId());
//      setDisconnectReason(reason);
   }
}
//-----------------------------------------------------------------------------
void CoreNetConnection::onConnectionRejected(const char *reason)
{
   Con::executef(this,  "onConnectRequestRejected", reason);
}
//-----------------------------------------------------------------------------
void CoreNetConnection::handleStartupError(const char *errorString)
{
   Con::executef(this,  "onConnectRequestRejected", errorString);
}
//-----------------------------------------------------------------------------
void CoreNetConnection::connectionError(const char *errorString)
{
   if(isConnectionToServer())
   {
      Con::printf("Connection error: %s.", errorString);
      Con::executef(this,  "onConnectionError", errorString);
   }
   else
   {
      Con::printf("Client %d packet error: %s.", getId(), errorString);
//      setDisconnectReason("Packet Error.");
   }
   deleteObject();
}
//-----------------------------------------------------------------------------
bool CoreNetConnection::readConnectRequest(BitStream *stream, const char **errorString)
{
   if(!Parent::readConnectRequest(stream, errorString))
      return false;
//some nifty checks here .....

//for now i only tell the script about the ip address
   char buffer[64];
   Net::addressToString(getNetAddress(), buffer);

   const char *ret = Con::executef(this,  "onConnectRequest",buffer);
   if(ret[0])
   {
      *errorString = ret;
      return false;
   }

   return true;

}
//-----------------------------------------------------------------------------
void CoreNetConnection::onRemove()
{
   if(isNetworkConnection())
   {
	   sendDisconnectPacket(""); //XXTH fixme we need a reason 
   }
   if(!isConnectionToServer())
      Con::executef(this,  "onDrop", ""); //XXTH also 2nd param would be a reason 

//   if (mControlObject)
//      mControlObject->setControllingClient(0);
   Parent::onRemove();
}
//-----------------------------------------------------------------------------
// TEST reduce useless traffic:
// there is a constant stream of ~35 bits whats that ? 

// buildSendPacketHeader => 33bits with empty packettype!
// changed flags: 2 bits!
// PacketNotify note => 3 or 4 (maybe more depents on information in note) bits
//       Not is the data ! here we must look ! 

// ok it does not work because of mCurRate and NetRate struct are private
// so its more an test
//
// so, now it works if size is not changed and I set it to 200, also need to send fake
// flags for changed rate/size? or is it ok ... if it is i do not know what this sync is 
// in original checkPackerSend
//
// added AFK ... when player is afk we dont need so much spam ! 
//-----------------------------------------------------------------------------
void CoreNetConnection::checkPacketSend(bool force)
{
//	Parent::checkPacketSend(force);
//	return;

   U32 curTime = Platform::getVirtualMilliseconds();

   if (!force && curTime < mNextUpdate)
	   return;

//FIXME !!!! fake the paketsite
	S32 paketSize = 200; //fatal error ? - yes when is set per console 



  if(windowFull())
      return;

   BitStream *stream = BitStream::getPacketStream(paketSize);
   buildSendPacketHeader(stream);

   //mmhhh mLastUpdateTime = curTime;

   PacketNotify *note = allocNotify();
   if(!mNotifyQueueHead)
      mNotifyQueueHead = note;
   else
      mNotifyQueueTail->nextPacket = note;
   mNotifyQueueTail = note;
   note->nextPacket = NULL;
   note->sendTime = curTime;

   //reuse of flags 
   if (isConnectionToServer()) //we must check it for local connection else it does not work ... we would not need it on local but to make it clean
		stream->writeFlag(mClientAFK);
   else
		stream->writeFlag(false);

   stream->writeFlag(false);

   //unused U32 start = stream->getCurPos();
   writePacket(stream, note);
/* debug heavy console spam!*/
#ifdef TORQUE_DEBUG
   if (getDebug())
   {
		if (isConnectionToServer()) 
			Con::printf("CLIENT => SERVER Stream bits:%d, AFK:%d", stream->getCurPos(), mClientAFK);
		else
			Con::printf("SERVER => CLIENT (id:%d) Stream bits:%d, AFK:%d", getId(), stream->getCurPos(), mClientAFK);
	}
#endif


   sendPacket(stream);

   //AFK !!
   if (mClientAFK )
   {
	   if (isConnectionToServer()) //less !
			mNextUpdate =  curTime + 10000; //HARDCODED ... 10 sek!  client->server 
	   else
		   	mNextUpdate =  curTime + 5000; //HARDCODED ... 5 sek! should be less when some action go on
   } else {
	   if (isConnectionToServer()) //less !
			mNextUpdate =  curTime + 125; //FIXME HARDCODED ... 1/8 sek!
	   else
		   mNextUpdate =  curTime + 100; //FIXME HARDCODED ... 1/10 sek!
   }
}
//-----------------------------------------------------------------------------
// need virtual void handlePacket(BitStream *stream);
void CoreNetConnection::handlePacket(BitStream *bstream)
{

   mErrorBuffer = String();

//we use this flag for afk 
   if (isConnectionToServer()) //we must check it for local connection else it does not work ... we would not need it on local but to make it clean
		bstream->readFlag();
   else
		mClientAFK = bstream->readFlag();

//cool another flag to use :P
   if(bstream->readFlag())
   {
   }

   readPacket(bstream);


   if (mErrorBuffer.isNotEmpty())
      connectionError(mErrorBuffer);

}
//-----------------------------------------------------------------------------
void CoreNetConnection::setClientAFK(bool newState)
{
	if (!isConnectionToServer()) {
		Con::errorf("Not on server!");
		return;
	}
	if (getClientAFK() == newState)
			return; 

	mClientAFK = newState;

	if (!getClientAFK()) { //must be true before
		checkPacketSend(true);
	}

}
//-----------------------------------------------------------------------------
// Scriptbinding 
//-----------------------------------------------------------------------------

DefineEngineMethod(CoreNetConnection, setDebug, void, (bool value), , "debug spam on/off")
{
   object->setDebug(value);
}

DefineEngineMethod(CoreNetConnection, setAFK, void, (bool value), , "AFK")
{
   object->setClientAFK(value);
}

