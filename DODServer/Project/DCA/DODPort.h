// DODPort.h: interface for the CDODPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DODPORT_H__4503AB4A_0D3C_46CE_AC73_BC6FB24A1788__INCLUDED_)
#define AFX_DODPORT_H__4503AB4A_0D3C_46CE_AC73_BC6FB24A1788__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "DODChannel.h"

class CDODPort  
{
public:
	CDODPort();
	virtual ~CDODPort();
	
	// channel number;
	int m_ChannelNumber;

	// channel gather
	CDODChannel **m_channel;

	//Create thread for command message
	int CreateChannelProcess();

	//create struct for called kit
	ZQSBFIPPORTINF **m_castPort;
	int m_castcount;

	//Its value is portID,
	int m_nID;

	//zqbroadcast property
	int m_nPmtPID;
	int m_nTotalRate;

	int m_nSessionID;
	int m_nGroupID;

	//create channel path for datawathchsource monitor thread
	CString m_currentDIR;

	//NodeName ,Its value come from parse configruation 
	CString m_sPortName;
};

#endif // !defined(AFX_DODPORT_H__4503AB4A_0D3C_46CE_AC73_BC6FB24A1788__INCLUDED_)
