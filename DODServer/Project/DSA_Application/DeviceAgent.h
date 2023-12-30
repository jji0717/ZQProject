
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
using namespace std;

/*
primary class to control program running.
*/

class CDeviceAgent
{
private:
	struIP		m_struIP;
public:
	//m_pConnectManager // this class is used to manage communication.
	//CSingleConnect * m_pConnectManager;
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
};

extern CDeviceAgent *g_pDeviceAgent;

#endif
