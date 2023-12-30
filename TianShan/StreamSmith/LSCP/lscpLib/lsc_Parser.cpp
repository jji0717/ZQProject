#include "lsc_parser.h"
#include <stdio.h>
#ifdef ZQ_OS_MSWIN
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace lsc
{
lscMessage::lscMessage ()
{	
	_opCode = (lsc :: OperationCode)0;
	memset(&_message,0, sizeof(_message));
	_bCompleteHeader = false ;
	_bCompleteBody	 = false;	
	_ownByteSize	 = 0;
}
lscMessage::~lscMessage ()
{
}
void lscMessage::setStreamHandle( uint32_t strmHandle )
{
	_message.jump.header.streamHandle =strmHandle;
}

void lscMessage::setTransactionId( uint8_t transId )
{
	_message.jump.header.transactionId = transId;
}

void lscMessage::setStatusCode( uint8_t statusCode )
{
	_message.jump.header.statusCode = statusCode;
}

void lscMessage::setVersion( uint8_t version )
{
	_message.jump.header.version = version;
}

void lscMessage::setOpCode( uint8_t opCode )
{
	_message.jump.header.opCode =opCode;
	_opCode = (lsc :: OperationCode)opCode;
}
void lscMessage::CopyFrom(const lscMessage& msg)
{
	memcpy ( &_message , &msg._message , sizeof(msg) );
	_opCode = msg._opCode;	
}

const lsc::OperationCode	lscMessage::GetLscMessageOpCode( ) const
{
	return _opCode;
}
unsigned int lscMessage::getMsgSize ()
{
	return _message.jump.header.CalculateMessageLength ();
}

const LSCMESSAGE& lscMessage::GetLscMessageContent( ) const
{
	return _message;
}
void lscMessage::hton()
{	
	switch (_opCode)
	{
	case LSC_PAUSE:     
		{
			_message.pause.hton ();
		}
		break;
	case LSC_RESUME:
		{
			_message.resume.hton ();
		}
		break;
	case LSC_STATUS:
		{
			_message.status.header.hton ();
		}
		break;
	case LSC_RESET:
		{
			_message.resume.hton ();
		}
		break;
	case LSC_JUMP:		
	case LSC_PLAY:
		{
			_message.play.hton ();
		}
		break;
	case LSC_DONE:
	case LSC_PAUSE_REPLY:
	case LSC_RESUME_REPLY:
	case LSC_STATUS_REPLY:
	case LSC_RESET_REPLY:
	case LSC_JUMP_REPLY:
	case LSC_PLAY_REPLY:
		{
			_message.response.hton ();
		}
		break;
	default:
		{//Invalid message op code
			return ;
		}
		break;
	}
}
bool lscMessage::Parse ()
{
	if ( !_bCompleteBody || !_bCompleteHeader ) 
	{//NOt complete message
		return false;
	}
    switch (_opCode)
	{
	case LSC_PAUSE:     
		{
			_message.pause.ntoh ();
		}
		break;
	case LSC_RESUME:
		{
			_message.resume.ntoh ();
		}
		break;
	case LSC_STATUS:
		{
			_message.status.header.ntoh ();
		}
		break;
	case LSC_RESET:
		{
			_message.resume.ntoh ();
		}
		break;
	case LSC_JUMP:		
	case LSC_PLAY:
		{
			_message.play.ntoh ();
		}
		break;
	case LSC_DONE:
	case LSC_PAUSE_REPLY:
	case LSC_RESUME_REPLY:
	case LSC_STATUS_REPLY:
	case LSC_RESET_REPLY:
	case LSC_JUMP_REPLY:
	case LSC_PLAY_REPLY:
		{
			_message.response.ntoh ();
		}
		break;
	default:
		{//Invalid message op code
			return false;
		}
		break;
	}
	return true;
}

#define STANDARDHEADERSIZE (int)sizeof(lsc::StandardHeader_t)

lscMessage* ParseMessage( void*& buf , int& size , lscMessage*& lastMsg )
{
	lscMessage* msg = lastMsg ? lastMsg : new lscMessage();

	int	byteUsed = 0;

	while ( size > 0 )
	{
		if ( ! msg->_bCompleteHeader ) 
		{//parse header
			int byteCanCopy = STANDARDHEADERSIZE >= size ? size : STANDARDHEADERSIZE;
			
			memcpy ( (unsigned char*)(&msg->_message.jump)+msg->_ownByteSize , buf , byteCanCopy);
			
			size -= byteCanCopy;
			msg->_ownByteSize += byteCanCopy;
			byteUsed += byteCanCopy;
			buf = (char*)buf + byteCanCopy;
			
			if ( byteCanCopy >=  STANDARDHEADERSIZE ) 
			{
				msg->_opCode = (lsc::OperationCode)(msg->_message.jump.header.opCode);
				msg->_bCompleteHeader = true;
			}
			else
			{//message is not complete yet,wait for new data
				msg->_bCompleteHeader = false;
				lastMsg = msg;
				return NULL;
			}			
		}
		if ( !msg->_bCompleteBody ) 
		{
			int needByte = msg->_message.jump.header.CalculateMessageLength () -  STANDARDHEADERSIZE;
			if (needByte < 0) // ==0 is acceptable
			{//Invalid LSC Message
				lastMsg = NULL;
				//throw out a exception
				char	szException[1024];
				szException[1023] = '\0';

				//Convert the header from network byte order to host byte order
				msg->_message.jump.header.ntoh ();				
				//delete the msg 				
				int iBufSize = sizeof(szException)-1;
				int iPos =  0;
				iPos =  snprintf(szException+iPos,iBufSize,"Invalid Message standard header:");
				iBufSize -= iPos;
				for ( size_t i =0 ;i< sizeof(lsc::StandardHeader_t) ; i ++ ) 
				{
					if (iBufSize>0) 
					{
						int iTempSize  =  snprintf(szException+iPos,iBufSize,"%02x ",((unsigned char*)&msg->_message.jump.header)[i]);
						iBufSize -= iTempSize;
						iPos += iTempSize;
					}
				}
				delete msg;
				throw errorMessageException(szException);
				return NULL;
			}
			int byteCanCopy = needByte >= size ? size : needByte;
			memcpy( (unsigned char*)(&msg->_message.jump)+msg->_ownByteSize ,buf , byteCanCopy );
			size -=byteCanCopy;
			msg->_ownByteSize += byteCanCopy;
			buf = (char*)buf + byteCanCopy;
			if ( byteCanCopy >= needByte ) 
			{
				msg->_bCompleteBody = true;				
				lastMsg = NULL;				
				return msg;
			}
			else
			{
				msg->_bCompleteBody = false;
				lastMsg = msg;
				return NULL;
			}
		}
		else
		{
			lastMsg = NULL;
			return msg;
		}
	}
	return NULL;	
}
uint32_t  lscMessage::toMetaData(StringMap& metadata)
{
   return toLscHeader(metadata);
}
bool    lscMessage::readMetaData(const StringMap& metadata)
{
   return readLscHeader(metadata);		 
}
uint32_t  lscMessage::toLscHeader(StringMap& metadata)
{
	_message.jump.header.ntoh();

	char strTemp[33];
	memset(strTemp, 0, sizeof(strTemp));
	itoa(_message.jump.header.streamHandle, strTemp, 10);
	sprintf(strTemp,"%u",_message.jump.header.streamHandle);
    metadata[CRMetaData_LscStreamHandle] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	itoa(_message.jump.header.transactionId, strTemp, 10);
	metadata[CRMetaData_LscTransactionId] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	itoa(_message.jump.header.statusCode, strTemp, 10);
	metadata[CRMetaData_LscStatusCode] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	itoa(_message.jump.header.version, strTemp, 10);
	metadata[CRMetaData_LscVersion] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	itoa(_message.jump.header.opCode, strTemp, 10);
	metadata[CRMetaData_LscOpCode] = strTemp;
   
	_message.jump.header.hton();
	return (uint32_t)metadata.size();
}
bool  lscMessage::readLscHeader(const StringMap& metadata)
{
	StringMap::const_iterator itorMd;

	itorMd =  metadata.find(CRMetaData_LscStreamHandle);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	uint32_t streamHandle = 0;
	sscanf(itorMd->second.c_str(), "%u", &streamHandle);
	setStreamHandle(streamHandle);

	itorMd =  metadata.find(CRMetaData_LscTransactionId);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	setTransactionId((uint8_t)atoi(itorMd->second.c_str()));

	itorMd =  metadata.find(CRMetaData_LscStatusCode);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	setStatusCode((uint8_t)atoi(itorMd->second.c_str()));

	itorMd =  metadata.find(CRMetaData_LscVersion);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	setVersion((uint8_t)atoi(itorMd->second.c_str()));

	itorMd =  metadata.find(CRMetaData_LscOpCode);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	setOpCode((uint8_t)atoi(itorMd->second.c_str()));
	_message.jump.header.hton();
	return true;
}

uint32_t lscResponse::toMetaData(StringMap& metadata)
{
	toLscHeader(metadata);
	char strTemp[33];

	_message.response.data.ntoh();
	memset(strTemp, 0, sizeof(strTemp));
	sprintf(strTemp, "%u", _message.response.data.currentNpt);
	metadata[CRMetaData_ResponseCurrentNpt] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	sprintf(strTemp, "%d", _message.response.data.numerator);
	metadata[CRMetaData_ResponseNumerator] = strTemp; 

	memset(strTemp, 0, sizeof(strTemp));
	itoa(_message.response.data.denominator, strTemp, 10);
	metadata[CRMetaData_ResponseDenominator] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	itoa(_message.response.data.mode, strTemp, 10);
	metadata[CRMetaData_ResponseMode] = strTemp;      

	_message.response.data.hton();
   return (uint32_t)metadata.size();
}
bool  lscResponse::readMetaData(const StringMap& metadata)
{
   if(!readLscHeader(metadata))
	   return false;

   char strTemp[33] = "";
   StringMap::const_iterator itorMd;
   itorMd =  metadata.find(CRMetaData_ResponseCurrentNpt);
   if(itorMd == metadata.end() || itorMd->second.empty())
   {
	   return false;
   }
   uint32_t currentNPT = 0;
   memset(strTemp, 0, sizeof(strTemp));
   sscanf(itorMd->second.c_str(), "%u", &currentNPT);
   setCurrentNpt(currentNPT);

   int16_t numerator;
   itorMd =  metadata.find(CRMetaData_ResponseNumerator);
   if(itorMd == metadata.end() || itorMd->second.empty())
   {
	   return false;
   }
   sscanf(itorMd->second.c_str(), "%d", &numerator);

   itorMd =  metadata.find(CRMetaData_ResponseDenominator);
   if(itorMd == metadata.end() || itorMd->second.empty())
   {
	   return false;
   }
   uint16_t denominator = (uint16_t)atoi(itorMd->second.c_str());
   setScale(numerator, denominator);

   itorMd =  metadata.find(CRMetaData_ResponseMode);
   if(itorMd == metadata.end() || itorMd->second.empty())
   {
	   return false;
   }
   setMode((uint8_t)atoi(itorMd->second.c_str()));

   _message.response.data.hton();
   return true;
}
uint32_t lscPlay::toMetaData(StringMap& metadata)
{
	toLscHeader(metadata);

	_message.play.data.ntoh();
	char strTemp[33];

	memset(strTemp, 0, sizeof(strTemp));	
	sprintf(strTemp,"%u",_message.play.data.startNpt);
	metadata[CRMetaData_PlayStartNpt] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));	
	sprintf(strTemp,"%u",_message.play.data.stopNpt);
	metadata[CRMetaData_PlayStopNpt] = strTemp; 

	memset(strTemp, 0, sizeof(strTemp));
	sprintf(strTemp, "%d",_message.play.data.numerator);
	metadata[CRMetaData_PlayNumerator] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	//itoa(_message.play.data.denominator, strTemp, 10);
	sprintf(strTemp,"%u",_message.play.data.denominator);
	metadata[CRMetaData_PlayDenominator] = strTemp; 
	_message.play.data.hton();
	return (uint32_t)metadata.size();
}
bool  lscPlay::readMetaData(const StringMap& metadata)
{
	if(!readLscHeader(metadata))
		return false;

	StringMap::const_iterator itorMd;
	itorMd =  metadata.find(CRMetaData_PlayStartNpt);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	uint32_t startNPT = 0;
	sscanf(itorMd->second.c_str(), "%u", &startNPT);
	setStartNpt(startNPT);

	itorMd =  metadata.find(CRMetaData_PlayStopNpt);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	uint32_t stopNPT = 0;
	sscanf(itorMd->second.c_str(), "%u", &stopNPT);
	setStopNpt(stopNPT);

	int16_t numerator;
	itorMd =  metadata.find(CRMetaData_PlayNumerator);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	sscanf(itorMd->second.c_str(), "%d", &numerator);

	itorMd =  metadata.find(CRMetaData_PlayDenominator);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	uint16_t denominator = (uint16_t)atoi(itorMd->second.c_str());
	setScale(numerator, denominator);

	_message.play.data.hton();
	return true;
}
//copy lscplay to lscresume
uint32_t lscResume::toMetaData(StringMap& metadata)
{
	toLscHeader(metadata);

	_message.resume.ntoh();

	char strTemp[33];

	memset(strTemp, 0, sizeof(strTemp));	
	sprintf(strTemp,"%u",_message.resume.startNpt);
	metadata[CRMetaData_PlayStartNpt] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	sprintf(strTemp,"%d",_message.resume.numerator);
	metadata[CRMetaData_PlayNumerator] = strTemp;

	memset(strTemp, 0, sizeof(strTemp));
	sprintf(strTemp,"%u",_message.resume.denominator);
	metadata[CRMetaData_PlayDenominator] = strTemp;    
	_message.resume.hton();
	return (uint32_t)metadata.size();
}
bool  lscResume::readMetaData(const StringMap& metadata)
{
	if(!readLscHeader(metadata))
		return false;

	StringMap::const_iterator itorMd;
	itorMd =  metadata.find(CRMetaData_PlayStartNpt);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	uint32_t startNPT = 0;
	sscanf(itorMd->second.c_str(), "%u", &startNPT);
	setStartNpt(startNPT);

	int16_t numerator ;
	itorMd =  metadata.find(CRMetaData_PlayNumerator);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	sscanf(itorMd->second.c_str(), "%d", &numerator);

	itorMd =  metadata.find(CRMetaData_PlayDenominator);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	uint16_t denominator = (uint16_t)atoi(itorMd->second.c_str());
	setScale(numerator, denominator);

	_message.resume.hton();
	return true;
}
uint32_t lscPause::toMetaData(StringMap& metadata)
{
	toLscHeader(metadata);

    _message.pause.ntoh();
	char strTemp[33];

	memset(strTemp, 0, sizeof(strTemp));
	sprintf(strTemp,"%u",_message.pause.stopNpt);
//	itoa(_message.pause.stopNpt, strTemp, 10);
	metadata[CRMetaData_PauseStopNpt] = strTemp;

	_message.pause.hton();
	return (uint32_t)metadata.size();
}
bool  lscPause::readMetaData(const StringMap& metadata)
{
	if(!readLscHeader(metadata))
		return false;
   
	StringMap::const_iterator itorMd;
	itorMd =  metadata.find(CRMetaData_PauseStopNpt);
	if(itorMd == metadata.end() || itorMd->second.empty())
	{
		return false;
	}
	uint32_t startNPT = 0;
	sscanf(itorMd->second.c_str(), "%u", &startNPT);
	setStopNpt(startNPT);

	_message.pause.hton();
	return true;
}
}
