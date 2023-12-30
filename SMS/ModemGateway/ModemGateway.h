#if !defined(ModemGateway_H)
#define ModemGateway_H

#include "afx.h"

#include "BaseSchangeServiceApplication.h"

#include <map>
using namespace std;

#include "SMSComPortReadThread.h"
#include "SMSComPortWriteThread.h"
#include "SMSProcessRawMsgThread.h"

#include "SMSXmlProc.h"

#define SMSSRV_DEFAULT_COM        1
#define SMSSRV_DEFAULT_TIMES      5
#define SMSSRV_DEFAULT_PORT       61232
#define SMSSRV_DEFAULT_TIMEOUT    10000
#define SMSSRV_DEFAULT_PLAY_FLAG  _T("点播")
#define SMSSRV_DEFAULT_CHAT_FLAG  _T("聊天")
#define SMSSRV_DEFAULT_REG_FLAG   _T("注册")
#define SMSSRV_DEFAULT_OVER_TIME  2
#define SMSSRV_DEFAULT_RESPONSE   1
#define SMSSRV_DEFAULT_ERROR_RESPONSE 1
#define SMSSRV_DEFAULT_ECHO       0

typedef map<wstring, unsigned short> PROFIXMAP;

class ModemGateway : public ZQ::common::BaseSchangeServiceApplication
{
public:
	ModemGateway();
	~ModemGateway();

	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	HRESULT	OnStart(void);
	HRESULT OnStop(void);


	// @param[out] playFlag
	// @param[out] ipAddr
	// @param[out] port
	// @param[out] timeout
	// @param[out] times
	// @param[out] overtime
	//void getConfig(char* playFlag, char* ipAddr, int* port, DWORD* timeout, int* times, int* overtime, DWORD* response, DWORD* errorResponse);

	// @param[out] overtime
	//void getConfig(int* overtime, char* DBPath, int* echo);

//	char*	 getPlayFlag() {return m_PlayFlag;};
//	char*	 getChatFlag() {return m_ChatFlag;};
//	char*	 getRegFlag()  {return m_RegFlag;};

	char*    getPlayFlag() {return m_info._ACFlag; };
	char*    getChatFlag() {return m_info._CTFlag; };
	char*    getRegFlag()  {return m_info._RGFlag; };
	char*    getNCFlag()   {return m_info._NCFlag; };
	
	wchar_t* getIP()	   {return m_wszIP; };
	wchar_t* getDBPath()   {return m_dbPath; };
	DWORD    getComPort()  {return m_com; };
	DWORD    getIpPort()   {return m_port; };
	DWORD    getTimes()	   {return m_times; };
	DWORD    getTimeout()  {return m_timeout; };
	DWORD    getOvertime() {return m_overtime; };

//	DWORD    getResponse() {return m_response;};
//	DWORD    getErrRsp()   {return m_errorResponse;};

	DWORD    getResponse() {return m_info._response; };
	DWORD    getErrRsp()   {return m_info._errorResponse; };

	DWORD	 getEcho()	   {return m_echo;};
	
	SMSComPortReadThread*	getComRead()   {return m_comReadThd;};
	SMSComPortWriteThread*	getComWrite()  {return m_comWriteThd;};
	SMSProcessRawMsgThread* getRawThread() {return m_processRawMsgThd;};

	bool GetReturnCode(int actionCode, char* returnText);
	
private:
	// get initialization from regedit
	void getInitialize();

	void getDefaultPath(wchar_t* dbPath, wchar_t* configFile);

	bool readConfigXml();
	
private:
	wchar_t m_wszIP[20];
	wchar_t m_dbPath[BUFSIZ];
	//wchar_t m_ConfileFile[BUFSIZ];

//	char m_PlayFlag[20];
//	char m_ChatFlag[20];
//	char m_RegFlag[20];
	
	DWORD m_com;
	DWORD m_port;
	DWORD m_times;
	DWORD m_timeout;
	DWORD m_overtime;
//	DWORD m_response;
//	DWORD m_errorResponse;
	DWORD m_echo;

	PROFIXMAP m_profixOpMap;

	SMSXmlProc  m_pXmlProc;

	CMapStringToString*		m_ReturnCodeMap;

	SMSComPortReadThread*   m_comReadThd;      //subService
	SMSComPortWriteThread*  m_comWriteThd;     //subService
	SMSProcessRawMsgThread* m_processRawMsgThd;//subService

	// SMS 配置的初始化信息
	SMSConfigInfo m_info;
};

#endif // !defined(ModemGateway_H)