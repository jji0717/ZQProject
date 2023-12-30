#include "Reporter.h"

#include "NativeThreadPool.h"

static char* SOAP_STATUS_TXT[] = { "SoapFail", "AuthPass", "AuthFail" };

class SOAPRequest : public ZQ::common::ThreadRequest
{
public:
	SOAPRequest(ZQ::common::NativeThreadPool& Pool, DWORD myTicket, int nReqNumber);
	virtual ~SOAPRequest();

public:	
	typedef enum { SOAP_FAIL=0, AUTH_PASS, AUTH_FAIL } SOAPSTATUS ;

	SOAPSTATUS getSetupStatus() { return m_setupStatus; };
	SOAPSTATUS getTeardownStatus() { return m_teardownStatus; };
	
	DWORD getSetupTimeConsumption() { return m_dwSetupTime; };
	DWORD getTeardownTimeConsumption() { return m_dwTeardownTime; };

protected:
	virtual bool init(void);
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

private:
	int   m_nNumber;
	DWORD m_dwMyTicketId;

	DWORD m_dwSetupTime;
	DWORD m_dwTeardownTime;

	SOAPSTATUS  m_setupStatus;
	SOAPSTATUS  m_teardownStatus;
};