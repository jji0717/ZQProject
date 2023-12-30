// ZQSnmpSubagent.h: interface for the ZQSnmpSubagent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZQSNMPSUBAGENT_H__FE6C3B12_B80D_4E79_95D1_8C1CBAC09E30__INCLUDED_)
#define AFX_ZQSNMPSUBAGENT_H__FE6C3B12_B80D_4E79_95D1_8C1CBAC09E30__INCLUDED_

#include "ServiceMIB.h"
#include <Locks.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ZQSnmpSubagent  
{
public:
	ZQSnmpSubagent(UINT serviceID, UINT svcProcess, UINT svcProcessInstanceId);
	~ZQSnmpSubagent();

    bool ManageVariable(const char *name, void *address, DWORD type, DWORD access, UINT instId);
    bool Run();

	timeout_t setTimeout(timeout_t timeOut){ return m_selectTimeout = timeOut;}

private:
    ZQSNMP_STATUS processMessage(const void *pRequestMsg, int len, std::string *pResponseMsg);
    ZQSNMP_STATUS processRequest(BYTE pduType, SnmpVarBind *pVb);

    ZQSNMP_STATUS processGetRequest(SnmpVarBind *pVb);
    ZQSNMP_STATUS processGetNextRequest(SnmpVarBind *pVb);
    ZQSNMP_STATUS processSetRequest(SnmpVarBind *pVb);

    bool init();
    void unInit();
    static DWORD WINAPI ThreadProc(LPVOID lpParameter);
private:
    bool   m_bRunning;
    HANDLE m_hPipe;

    HANDLE  m_thread;
	UINT    m_serviceID;
	UINT    m_svcProcess;
	UINT    m_serviceInstanceId;
	uint32  m_snmpUdpBasePort;
	timeout_t m_selectTimeout;

    ServiceMIB m_svcMib;
    ZQ::common::Mutex m_lockMib;
};

#endif // !defined(AFX_ZQSNMPSUBAGENT_H__FE6C3B12_B80D_4E79_95D1_8C1CBAC09E30__INCLUDED_)
