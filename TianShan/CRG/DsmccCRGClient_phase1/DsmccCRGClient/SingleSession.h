#ifndef	SA_SINGLESESSION_H__
#define SA_SINGLESESSION_H__
#include "TCPSocket.h"
#include "UDPSocket.h"
#include "DsmccSocket.h"

enum ResponseCode{
	session_ok    = 200,
	setup_error   = 701,
	play_error    = 702,
	pause_error   = 703,
	status_error  = 704,
	resume_error  = 705,
	release_error = 706,
	connect_error = 422,
	send_error = 400,
	step_error = 406,
	parse_error = 421,
};

class ClientThread;
class SingleSession
{
public:
	SingleSession(void);
	~SingleSession(void);
	bool Init();
	bool UnInit();
	bool Start();

private:
	bool Setup();
	bool TestSetupMessage(uint8* buf, size_t maxLen);
	bool Play();
	bool TestPlayMessage(uint8* buf, int maxLen);
	bool Pause();
	bool Status();
	bool Resume();
	bool Release();

	std::string CreateRandom(int strLen);
	std::string  GetSessionId();
	void CreateSessionId();
	bool HexToString(std::string& outputString ,const unsigned char* buf, unsigned short int len);
	int toStreamHandle(const uint8* pbuf,int len);
protected:
	bool m_bInit;
	uint32 m_nStreamHandle;
	std::string m_strSessionId;
	ClientThread* m_pthread;
	DsmccClientSocket* m_dsmccClient;
	DsmccClientSocket* m_lscpClient;

	int _stepNumber;	
	int _responseCode;

	std::string _errorRequestName;
	std::string _currentStatus;
	int _errResponseCode;
	int _errStepNumber;
	bool m_bffbegin;

public:
	static std::string getErrorMessage(int errorcode);

	void GetErrorInfo(std::string& status,std::string& requestName, int& responseCode, std::string& errorMessage, int& stepNum);
//	ZQ::common::UDPSocket* m_udpClient;
};


#endif