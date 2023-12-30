
#ifndef __ZQBROADCASTGUIDS_JZA_H
#define __ZQBROADCASTGUIDS_JZA_H

#include <atlconv.h>
#include <unknwn.h> 

// #define NEED_EVENTLOG 1

#define DODERROR_GENERIC_SUCCESSFUL	 	 (0)	//Successful
#define DODERROR_GENERIC_OUTOFMEMORY	 (1)	//Out of memory
//DOD Broadcast ERROR CODE
#define DOD_BROADCAST_ERROR 0x11300
#define DODERROR_FILE_DATA_PART_LOST		(DOD_BROADCAST_ERROR+ 1) //file Data is not enough to readto check 
#define DODERROR_RESET_PIDRATE				(DOD_BROADCAST_ERROR+ 2) //PID rate is too big
#define DODERROR_BUFFERSENDERROR			(DOD_BROADCAST_ERROR+ 3) //data tcp_send error,socket may not block 
#define DODERROR_SETCHANNELCOUNT			(DOD_BROADCAST_ERROR+ 4) //dwChannelCount is bigger than MAXPINNUMBER
#define DODERROR_CONFIGCHANNELERROR			(DOD_BROADCAST_ERROR+ 5) //ConfigChannel error:cDescriptor is error type
#define DODERROR_FILEPATHOVERLAP			(DOD_BROADCAST_ERROR+ 6) //SetFilePath error:filepath is set twice
#define DODERROR_CREATECURFILE				(DOD_BROADCAST_ERROR+ 7) //CreateCurrentFile error
#define DODERROR_CREATEBAKFILE				(DOD_BROADCAST_ERROR+ 8) //CreatebackFile error
#define DODERROR_PMTPID_VALUE				(DOD_BROADCAST_ERROR+ 9) //PID is small or PID is bigger than 0x1fff
#define DODERROR_PMTPID_TWICE				(DOD_BROADCAST_ERROR+10) //PmtPID is set twice
#define DODERROR_SENDTYPE					(DOD_BROADCAST_ERROR+11) //IPconfig parameter sendtype is error
#define DODERROR_BUFFERINDEX				(DOD_BROADCAST_ERROR+12) //GetBufferModeBuf,index is error from broadcastpin class
#define DODERROR_BUFFERISNULL				(DOD_BROADCAST_ERROR+13) //CBroadcastPin's Receive,buffer is null when continue_data is comes
#define DODERROR_BUFFERLENGTOOLEN			(DOD_BROADCAST_ERROR+14) //CBroadcastPin's Receive,buffer length is too leng.
#define DODERROR_INPUTPID_VALUE				(DOD_BROADCAST_ERROR+15) //PID is small or PID is bigger than 0x1fff
#define DODERROR_STRUCT_DATA_PART_LOST		(DOD_BROADCAST_ERROR+16) //Data is not enough to check  FILESTRUCT
#define DODERROR_SOCKETCREATSUCCESS			(DOD_BROADCAST_ERROR+17) //Connect to server success by tcpip model
#define DODERROR_MULITCASTSOCKETJOINERROR	(DOD_BROADCAST_ERROR+18) //m_pMulitcastSocket join error - ,m_cDestIp,m_wDestPort

// {86FB8364-2FBB-4018-94C9-959F3C60DCF6}
static const GUID CLSID_ZQBroadcast = 
{ 0x86fb8364, 0x2fbb, 0x4018, { 0x94, 0xc9, 0x95, 0x9f, 0x3c, 0x60, 0xdc, 0xf6 } };
// {3081C724-B525-43b1-B2DE-518BF458CB45}

static const GUID IID_IRateControl = 
{ 0x3081c724, 0xb525, 0x43b1, { 0xb2, 0xde, 0x51, 0x8b, 0xf4, 0x58, 0xcb, 0x45 } };

// {9033770B-D118-453b-8EFD-F7F0A30320B8}
static const GUID IID_IIPSetting = 
{ 0x9033770b, 0xd118, 0x453b, { 0x8e, 0xfd, 0xf7, 0xf0, 0xa3, 0x3, 0x20, 0xb8 } };

static const GUID CLSID_SendConfigurePage = 
{ 0x20667a8c, 0xe24e, 0x4ee7, { 0x8e, 0x15, 0x1d, 0xb8, 0x11, 0x4e, 0xac, 0xbb } };
 

typedef struct SampleBlock{
	DWORD		dwProtocol; 	// 0x01: the wrapping rule used currently.
	DWORD		dwVersion; 	// 0x01: the version number of wrapping rule current.
	DWORD		dwPID;		// flag the element stream in which the content of sample is located.
	char		chTag[4];	// the affect of field is the same as the dwPID, e.g. “pic\0”, “hie\0”.
	BYTE        nControlFlag;  //add by simin : 0 – start, 1 – continue, 2 - end
	DWORD		dwCount;	// the count of object
	DWORD		dwLength;	// the length of content of the sample
	//char		*pchContent;	// content
} SampBlock;
//TO Andrew

typedef struct ZQSBF_ChannelInfo_
{

	WORD wPID;			// Channel PID.
	DWORD wRate;			// Kilo bits 
	WORD wAccessType;	// MEMORYTYPE=0 :FILETYPE=1
	BOOL bEnable;		// Channel anble or disable.
	// ------------------------------------------------------ Modified by zhenan_ji at 2005年3月17日 9:55:51
	char cDescriptor[4];
	//for
	WORD wRepeatTime;	// for subchinna
	BOOL bBeDetect;		// 	Automotion
	WORD wBeDetectInterval; //manual time .
	WORD wBeEncrypted;   
	WORD wChannelType;
	WORD nStreamType;
	char 		szPath[MAX_PATH];		//Catalog path 
	int wDataExchangeType;		// 	0, Share Folder; 1, message; 2, Local Folder
	int wStreamCount; //if wChannelType==1 .
	int wSendWithDestination; //create index table or not by destionation

} ZQSBFCHANNELINFO, *PZQSBFCHANNELINFO;

#define SENDTCPIPTYPE		0
#define SENDUDPTYPE			1
#define SENDMULTICASTTYPE	2
#define SENDBROADCASTTYPE	3

#define SENDTTYPETOTAL		4	

typedef struct ZQSBF_IP_PORT_Info_
{
	WORD wSendType;		//TCPIP, UDP, MULTICAST, BROADCAST
	char cSourceIp[16];
	WORD wSourcePort;
	char cDestIp[16];
	WORD wDestPort; 
}ZQSBFIPPORTINFO, *PZQSBFIPPORTINFO;

interface IRateControl : public IUnknown
{
public:
	STDMETHOD(SetChannelCount)( DWORD dwChannelCount )PURE; 

	//The first setChannelCount was called, then call configchannel,because config channelinfo by nPinIndex;
	STDMETHOD(ConfigChannel)( int nPinIndex, ZQSBFCHANNELINFO* pChannelInfo)PURE; 
	STDMETHOD(Enable)( WORD wPID, BOOL bAnable)PURE;
	STDMETHOD(SetTotalRate)(DWORD wTotalRate)PURE; 

// ------------------------------------------------------ Modified by zhenan_ji at 2005年7月14日 14:43:30
	//now ,the function was not used. replace with filepath=gettemppath();
	STDMETHOD(SetFilePath)(char *cfilepath)PURE; 

	//for create pat,pmt;
	STDMETHOD(SetPmtPID)( WORD wPID)PURE; 

	//get channelinfo by channel's PMT_PID
	STDMETHOD(GetChannelInfo)( WORD wPID, ZQSBFCHANNELINFO* pChannelInfo)PURE;
	STDMETHOD(GetTotalRate)(DWORD* pwTotalRate)PURE; 
	STDMETHOD(GetFilePath)(char *cfilepath)PURE; 
	STDMETHOD(GetPmtPID)( WORD* pwPID)PURE; 
};

interface IIPSetting : public IUnknown
{
	//set IP config
	STDMETHOD(SetIPPortConfig)( ZQSBFIPPORTINFO* pInfoList[], int nCount)PURE; 

	//get IP config
	STDMETHOD(GetIPPortConfig)( int nIndex, ZQSBFIPPORTINFO* pInfo)PURE;
};

typedef struct BUFFER_INFO
{
	WORD nPid;
	WORD nBufLen;
	WORD nOffset;
	char* pBuf;
}BufferInfo;

#endif