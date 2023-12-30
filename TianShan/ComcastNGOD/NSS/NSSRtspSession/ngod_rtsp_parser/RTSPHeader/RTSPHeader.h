#ifndef _RTSPHEADER_H_
#define _RTSPHEADER_H_

#include <WinSock2.h>
#include "RTSPR2Header.h"

namespace RTSPMessageHeader
{
	static const char *pEndLine			= "\r\n";
	static const char *pTransport		= "Tansport";
	static const char *pUnicast			= "unicast";
	static const char *pClientId		= "client_id";
	static const char *pQamDest			= "qam_destination";
	static const char *pDestination		= "destination";
	static const char *pClientPort		= "client_port";
	static const char *pSource			= "source";
	static const char *pBandwidth		= "bandwidth";
	static const char *pSopName			= "sop_name";
	static const char *pSopGroup		= "sop_group";
	static const char *pQamName			= "qam_name";
	static const char *pQamGroup		= "qam_group";
	static const char *pEdgeInputGroup	= "edge_input_group";
	static const char *pAnnex			= "annex";
	static const char *pChannelWidth	= "channel_width";
	static const char *pModulation		= "modulation";
	static const char *pInterleaver		= "interleaver";
	static const char *pEncryptorGroup	= "encryptor_group";
	static const char *pEncrptoryName	= "encryptor_name";
	static const char *pCseq			= "CSeq";
	static const char *pRequire			= "Require";
	static const char *pOnDemandSessId	= "OnDemandSessionId";
	static const char *pSessionGroup	= "SessionGroup";
	static const char *pSession			= "Session";
	static const char *pUserAgent		= "User-Agent";
}

typedef enum
{
	UnknowError				= 0,
	RTSPOK					= 200,
	BadRequest				= 400,
	Forbidden				= 403,
	NotFound				= 404,
	MethodNotAllowed		= 405,
	NotAcceptable			= 406,
	RequestTimeOut			= 408,
	Gone					= 410,
	RequestTooLarge			= 413,
	UnsupportedMedia		= 415,
	InvalidParameter		= 451,
	NotEnoughBandwidth		= 453,
	SessionNotFound			= 454,
	InvalidRange			= 457,
	OperationNotAllowed		= 459,
	UnsupportedTransport	= 461,
	DestUnreachable			= 462,
	GatewayTimeout			= 504,
	VersionNotSupported		= 505,
	SetupAssetNotFound		= 771,
	SOPNotAvailable			= 772,
	UnknownSOPGroup			= 773,
	UnknownSOPNames			= 774,
	InsufficientBandwidth	= 775
}RTSPSessionState;

typedef enum
{
	PREPARE		= 0,
	SETUP		= 1,
	PLAY		= 2,
	TEARDOWN	= 3,
	PAUSE		= 4,
	ANNOUNCE	= 5,
	GETPARAMETER= 6,
	PING		= 7,
	SETPARAMETER= 8
}RTSPClientState;

typedef struct SessionEvent
{
	SessionEvent()
	{
		InitializeCriticalSection(&_CS);
	}
	~SessionEvent()
	{
		for (map<uint16, HANDLE>::iterator iter = _pHandle.begin(); iter != _pHandle.end(); iter++)
			m_CloseEvent((*iter).first);
		DeleteCriticalSection(&_CS);
	}
	bool m_Init(uint16 CSeq)
	{
		EnterCriticalSection(&_CS);
		if (_pHandle.find(CSeq) != _pHandle.end())
		{
			LeaveCriticalSection(&_CS);
			return false;
		}
		_pHandle[CSeq] = CreateEvent(NULL, true, false, NULL);
		LeaveCriticalSection(&_CS);
		return true;
	}
	bool m_SetEvent(uint16 CSeq)
	{
		EnterCriticalSection(&_CS);
		bool b  = false;
		if (_pHandle.find(CSeq) == _pHandle.end())
		{
			LeaveCriticalSection(&_CS);
			return false;
		}
		b = SetEvent(_pHandle[CSeq]);
		LeaveCriticalSection(&_CS);
		return b;
	}
	bool m_ResetEvent(uint16 CSeq)
	{
		EnterCriticalSection(&_CS);
		bool b  = false;
		if (_pHandle.find(CSeq) == _pHandle.end())
		{
			LeaveCriticalSection(&_CS);
			return false;
		}
		b = ResetEvent(_pHandle[CSeq]);
		LeaveCriticalSection(&_CS);
		return b;
	}
	bool m_CloseEvent(uint16 CSeq)
	{
		EnterCriticalSection(&_CS);
		bool b  = false;
		map<uint16, HANDLE>::iterator iter = _pHandle.find(CSeq);
		if (iter == _pHandle.end())
		{
			LeaveCriticalSection(&_CS);
			return false;
		}
		b = CloseHandle(_pHandle[CSeq]);
		_pHandle.erase(iter);
		LeaveCriticalSection(&_CS);
		return b;
	}

	CRITICAL_SECTION	_CS;
	map<uint16, HANDLE>	_pHandle;
}SessionEvent;

typedef struct RTSPClientSession
{
	//base information
	string				strStreamName;		//ice stream object to string
	string				strServerPath;		//server IP(address)
	SOCKET				*RTSPSocket;		//rtsp interactive socket
	float				fScale;				//stream play scale
	uint16				uServerPort;		//server RTSP listen port
	uint16				uClientCSeq;		//client RTSP CSeq
	uint16				uServerCSeq;		//server RTSP CSeq
	uint16				uLocalPort;			//local port to initial this session
	string				strLocalIP;			//local ip address(*.*.*.*)
	string				strCurrentTimePoint;//MOD current stream time point
	string				strUserAgentID;		//MOD client user-agent
	//HANDLE			*m_pEventHandle;	//handle point to event to wake up
	SessionEvent		m_pEventHandle;		//handle point to event to wake up
	RTSPSessionState	iRTSPSessionState;	//RTSP Session Status
	RTSPClientState		iRTSPClientState;	//RTSP client status
	CRITICAL_SECTION	*m_pCS;				//pointer to group socket critical section
	CRITICAL_SECTION	*m_pCS_ClientSeq;	//pointer to group client sequence critical section
	CRITICAL_SECTION	*m_pCS_ServerSeq;	//pointer to group server sequence critical section

	//R2 Header
	RTSPR2Header		m_RTSPR2Header;		//Header content of R2 interaction

	//temporary use
	string				strAssetId;
}RTSPClientSession;

#endif