#if !defined (TICPMSGPROCESSREQUEST_H)
#define TICPMSGPROCESSREQUEST_H

#include "SMSService.h"
#include "NativeThreadPool.h"
#include "SMSMsg.h"
#include "WriteSocketThread.h"
#include "DBThread.h"
#include "TicpProc.h"

#define OPERATION_SUCCESSFUL 1

class SMSService;
class TicpProc;

class TICPMsgProcessRequest : public ZQ::common::ThreadRequest
{
public:
	TICPMsgProcessRequest(ZQ::common::NativeThreadPool& Pool, SMSService* pSrv, LPMessage pMsg);
	~TICPMsgProcessRequest();

protected:
	int run();
	void final(int retcode, bool bCancelled);

private:
	// 根据 TICP 的返回值 ，设置给 TM 的短信内容
	void SetMesssageContent(int ret);

	// 处理要发送给 TICP 的短信内容
	bool ProcSendContent();

	// 结束该 Request
	void EndRequest();
	
	// put content into WriteSocketThread
	void PutContentIntoWriteThrd();

	// 根据 Response 和 ErrorResponse 来处理短信
	void LastProcessMessage();

	// 插入数据库
	void InsertDB();
	
	// 更新数据库
	void UpdateDB();

	// 获取要发送给TICP的号码
	void GetRealTelephoneNumber(char* realTelephoneNumber);

private:
	SMSMsg* m_pSMSMsg;
	SMSService* m_pService;

	// 与 TICP 交互时的 sequence id
	DWORD m_sequenceID;

	// TICP 的 IP Address 和 port
	char m_ip[20];
	long m_port;

	// 与 TICP 交互时最大等待时间
	DWORD m_timeout;

	// TICP 的返回值
	int m_ret;

	// 接收到的短信内容中的开头标志
	char m_ACFlag[10];//stream control
	char m_CTFlag[10];//chat control
	char m_RGFlag[10];//register control
	int  m_mode;

	// this request has been dealed times
	int  m_processTimes;
	DWORD m_MaxRedoTimes;

	// this content which send to TICP
	char m_SendContent[100];

	// 点播结果回复 TM 的标志
	DWORD m_Response;

	// 点播错误结果回复 TM 的标志
	DWORD m_ErrorResponse;

	TicpProc* m_ticpProc;

	// 昵称标志
	char m_NCFlag[10];
};

#endif