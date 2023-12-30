
/*
**	FILENAME			DeviceInfoParser.h
**
**	PURPOSE				The file is a header file, used along with DeviceInfoParser.cpp.
**						a pure-virtual class CDeviceInfoParser is declared in the file, and this class is used to provide a 
**						interface for parsing packages that are got from socket connecting with AM.
**						
**						
**
**	CREATION DATE		19-07-2004
**	LAST MODIFICATION	21-07-2004
**
**	AUTHOR				Leon.li (Interactive ZQ)
**
**
*/


#ifndef DeviceInfoParserH
#define DeviceInfoParserH

#include "scqueue.h"
#include "Markup.h"
#include "scsocket.h"
#include "DODServerController.h"
//#include "msxmldom.h"
//#include "sccommonreceiver.h"
//#include "E:\Leira\Working\DVB Broadcast Application Suite\MediaPumper\MediaPumpController\DeviceManager.h"
//#include "DeviceManager.h"

#define MAXDEVICE		10
#define MAXTRANSPORT	10
#define MAXPORT			20

#define MAKEDEVICEID(x,y,z)		((x<<16)+(y<<8)+z)  // the value that is made of three value will be passed in user define message and unique data of per list item.
#define MAKEPORTID(x, y)		((x<<8)+y)			// the value that is made of two value will be received/sent from/to playback-server/devicemanager-dll.

#define GETDEVICEID(x)		((x>>16)&0xFFFF)
#define GETTRANSPORTID(x)	((x>>8)&0xFF)
#define GETPORTID(x)		(x&0xFF)

extern struIP		m_struIP;

// the function to be callbacked by device manager to write log file.
void WriteLog(char *fmt, ...);

// the function to be callbacked by device manager to report status to playback server.
int ReportStatus(LONG lDeviceID, LONG lTransportID, LONG lPortID, LONG lStatus);

// the function to be callbacked by device manager to report error to playback server.
int ReportError(LONG lDeviceID, LONG lTransportID, LONG lPortID, DWORD dwError, LPCTSTR strMsg);


// ISCParser is a pure virtual class.
// User must inherit it and define their own Parse method.
class ISCParser
{
public:
	virtual VOID Parse( CSCTCPSocket * pSocket, CSCMemoryBlockPtr block ) = 0;
};

extern CRITICAL_SECTION	m_csSendProtect;		// protect senddata
//CDeviceInfoParser inherit from ISCParser for parsing package in current application DeviceInfo Server.

class CDeviceInfoParser: public ISCParser
{
public:
	static CDeviceInfoParser * s_pDeviceInfoParser;

	// the handle of the parent window ( dialog ).
	HWND	  m_hWnd;

	//CMarkup m_XmlDOM, m_SendXmlDOM;

	// the id of this controller, have be discarded.
	UINT32	  m_iControllerID;

	// the value will be used to check the exists of local configuration file 
	// for reporting to playback server when the connection to playback is created successful.
	WORD	  m_iHaveConfig;		

	// when the connection to playback server has successful, check this m_iHaveConfig, false to send handshake, 
	// true to check the m_bHaveReportList for 
	//
	bool	  m_bHaveReportList;

	// the object of device manager.
	CDODDeviceController m_DeviceManager;

public:
	static DWORD		m_iHeartBeatInterval;	// second
	static DWORD		m_iHeartBeatReferVal;	// after each heart beat, this value is reset.
	CWinThread		   *m_HeartBeatThread;		// use a thread deal heart beat mechanism.
	static bool			m_bHeartBeatThreadAlive;// control heartbeat thread.
	static bool			m_bHeartBeatThreadExit;	// check whether heartbeat over.	

	// called in heart beat thread for proving alive itself.
	void ReportHeartBeat();
public:
	CDeviceInfoParser( HWND hWnd )
	{
		s_pDeviceInfoParser = this;
		m_hWnd = hWnd;
	}

	// Parse message from Playback Server.
	VOID Parse( CSCTCPSocket * pSocket, CSCMemoryBlockPtr block );

	// test
	void Test( LPCTSTR sFileName );

	void ParseCommand(CMarkup m_XmlDOM );

	// Get current time, return CString.
	CString GetCurrTime();

	// Get current datetime.
	CString GetCurrDateTime();

	// Add Message Header
	void AddMessageHeader( CMarkup & tmpXmlDOM,  DWORD flag, DWORD no, WORD code );

	// put message to a message queue
	void SendMessage( CMarkup & tmpXmlDOM, int iPackageNumber=-1 );
	void SendData( CMarkup & tmpXmlDOM, int iPackageNumber=-1 );

	// end heartbeat thread.
	void EndHeartBeatThread();

	// Report ports information from calling GetPort() of PortController.
	void ReportPortInfo( DWORD dwPortID, DWORD dwSessionID, DWORD dwCommandID );

	void ReportReturn( DWORD dwPortID, DWORD dwFlag, DWORD dwSessionID, DWORD dwMessageCode, DWORD dwRtn );
};

#endif