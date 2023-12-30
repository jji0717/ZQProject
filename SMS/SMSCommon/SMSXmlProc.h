#if !defined(AFX_SMSXMLPROC_H)
#define AFX_SMSXMLPROC_H

#include "afx.h"
#include "xmlpreference.h"

#define RESPONSE				  1
#define MAX_LENGTH				  500
#define SMSSRV_DEFAULT_PORT       61232
#define SMSSRV_DEFAULT_IP_ADR     "127.0.0.1"

using namespace ZQ::common;

class SMSXmlProc;

struct SMSConfigInfo
{	
	char			_configFilePath[MAX_PATH + 1];
	
	// TICP µÄ IP ºÍ PORT
	CStringList		_TicpIplist;
	CStringList		_TicpPortlist;

	CStringList		_TMFlagList; 
	CStringList		_TicpFlagList; 
	
	CStringList		_ActionCodeList;
	CStringList		_ReturnTextList;
	
	CStringList		_SPList;
	CStringList		_SPNumberList;
	
	char			_ACFlag[10];
	char			_CTFlag[10];
	char			_RGFlag[10];
	char			_NCFlag[10];

	char			_TMIp[20];
	DWORD			_TMPort;
	
	DWORD			_response;
	DWORD			_errorResponse;
	DWORD			_replyHistory;
	
	int				_success[MAX_LENGTH];

public:
	SMSConfigInfo();
};

class SMSXmlProc
{
public:
	SMSXmlProc();
	
	virtual ~SMSXmlProc();

	// Com Initialize
	void CoInit()
	{
		if (!init)
			init = new ZQ::common::ComInitializer;
	}
	
	void CoUnInit()
	{
		if (init)
		{
			delete init;
			init = NULL;
		}
	}

	// process Xml
	// @param[in]  sXml //Xml file content
	// @param[out] sequenceId
	// @param[out] actionCode
	// @param[out] parameter1
	// @param[out] parameter2
	void XmlProc(char* sXml, 
				 char* sequenceId, 
				 char* actionCode, 
				 char* parameter1, 
				 char* parameter2 = NULL);

	bool XmlGetConfig(SMSConfigInfo* info);
	
protected:
	ZQ::common::ComInitializer* init;
};
#endif //!defined(AFX_SMSXMLPROC_H)