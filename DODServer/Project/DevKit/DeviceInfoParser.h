
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

// the function to be callbacked by device manager to report status to playback server.
int ReportStatus(LONG lDeviceID, LONG lTransportID, LONG lPortID, LONG lStatus);

// the function to be callbacked by device manager to report error to playback server.
int ReportError(LONG lDeviceID, LONG lTransportID, LONG lPortID, DWORD dwError, LPCTSTR strMsg);


// ISCParser is a pure virtual class.
// User must inherit it and define their own Parse method.
class ISCParser
{
public:
	virtual VOID Parse( FredPtr block ) = 0;
};

class CDeviceInfoParser: public ISCParser
{
public:
	int m_nCommandID;
	
public:
	// heart beat thread function.
	int GetCommandID();

	void ReportHeartBeat();
public:
	CDeviceInfoParser()
	{
		m_nCommandID=1;
		m_CommandList=NULL;
	}

	// Parse message from Playback Server.
	VOID Parse(FredPtr block );
	
	// Get current datetime.
	CString GetCurrDateTime();
	CPtrList *m_CommandList;
};

#endif