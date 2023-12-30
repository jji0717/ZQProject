
/*
**	FILENAME			DeviceAgent.cpp
**
**	PURPOSE				This file include a class: CDeviceAgent;
**						the class CDeviceAgent is used as a primary control class in the service, when service
**						start, the class will be created and all work will start along with the class activation.
**						
**
**	CREATION DATE		01-25-2005
**	LAST MODIFICATION	01-25-2005
**
**	AUTHOR				Leon.li (Interactive ZQ)
**
**

Revision History:

Rev			   Date        Who			Description
----		---------    ------		----------------------------------
V1,8,7,5	2006.08.11	zhenan_ji 	change port number limited about portinfo struct to list; 
V1,8,8,0	2006.09.07	zhenan_ji 	1 :Fixed bug: DSA service down while it receives multiple clients connection.	
									  (major point: at CListenManager::Notify() in ClistenManager.cpp . 
									  Owner can not delete itself.)
*/

#ifndef CDEVICECONTROLLER_H
#define CDEVICECONTROLLER_H


#include "ListenManager.h"
//#include "SingleConnect.h"
#include "Markup.h"
//#include <vstrmuser.h>
#include "clog.h"

#include <algorithm>
#include <string>
#include <vector>

#include "IceService.h"

using namespace std;

/*
primary class to control program running.
*/

class CDeviceAgent
{
private:
	struIP		m_struIP;
public:
	CListenManager * m_pConnectManager;
public:
	CDeviceAgent();	//construtor
	~CDeviceAgent();	//destrutor

	// Initialize configuration.
	void Initialize();

	// Uninitialize, destroy object.
	void UnInitialize();
	// check file exists.
	bool FileExists( LPCTSTR lpszFilename );

protected:
	IceService		m_iceService;
};

extern CDeviceAgent *g_pDeviceAgent;

#endif
