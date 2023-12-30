#ifndef SA_CLIENTSESSION_H_
#define SA_CLIENTSESSION_H_
/*
class ClientSession : public ZQ:;common::SharedObject
{
protected:
	ClientSession(ZQ::common::Log& log);
	typedef ZQ::common::Pointer < ClientSession > Ptr; 

	~ClientSession(void);
public:
protected:
	std::string _sessionId;
	uint32		_streamhandle;
public:
	std::string getSessionId() const;
	uint32		getStreamHandle() const;
private:
	bool formatSetupMessage(char* buf, int len);
	bool formatReleaseMessage(char* buf, int len);

	bool formatPlayMessage(char* buf, int len);
	bool formatPauseMessage(char* buf, int len);
	bool formatstatusMessage(char* buf, int len);
	bool formatResumeMessage(char* buf, int len);
protected:
	
};

class SessionMgr
{
protected:
	std::map<std::string, ClientSession::Ptr> _sessionID2Session;
	std::map<uint32, ClientSession::Ptr>	 _streamHander2Session;
	std::vector<ZQ::common::TCPClient*>

		ZQ::common::Mutex _lockSessionId;
	ZQ::common::Mutex _lockStreamHandler;
public:

	ClientSession::Ptr findClientSessionById(const std::string& sessionId);
	ClientSession::Ptr findClientSessionByStreamHandle(uint32 streamHandle);

	static  int getSessionCount();
	static  ClientSession::Ptr createSession(const std::string& sessionId="");
	static  int _sessionCount;
};

*/
#endif