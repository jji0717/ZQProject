#pragma once
#include <winsock2.h>
#include <string>

#pragma comment(lib, "wsock32.lib")

typedef const struct sockaddr* LPCSOCKADDR;

//////class BlockingSocketException//////
class BlockingSocketException
{
public:
   // Constructor
   BlockingSocketException(char* pchMessage);
   ~BlockingSocketException();

   virtual bool GetErrorMessage(char* lpstrError, int nMaxError);
   virtual std::string GetErrorMessage();
   
private:
   int     m_nError;
   std::string m_strMessage;
};

//////class BlockingSocket/////////
class BlockingSocket
{
public:
	BlockingSocket(void);
	~BlockingSocket(void);

	
	void Cleanup();
	void Create(int nType = SOCK_STREAM);
	void Close();

	BlockingSocket* CreateInstance()const;
	void Bind(LPCSOCKADDR psa) const;
	void Listen() const;
	void Connect(LPCSOCKADDR psa) const;
	bool Accept(BlockingSocket& s, LPSOCKADDR psa) const;
	int  Send(const char* pch, int nSize, int nSecs) const;
	int  Write(const char* pch, int nSize, int nSecs) const;
	int  Receive(char* pch, int nSize, int nSecs) const;
	int  SendDatagram(const char* pch, int nSize, LPCSOCKADDR psa, int nSecs) const;
	int  ReceiveDatagram(char* pch, int nSize, LPSOCKADDR psa, int nSecs) const;
	void GetPeerAddr(LPSOCKADDR psa) const;
	void GetSockAddr(LPSOCKADDR psa) const;
	bool CheckReadability() const;

	sockaddr_in   GetHostByName(const char* pchName, int nPort = 0);
	const char* GetHostByAddr(LPCSOCKADDR psa);
	operator SOCKET() const { return _sock; }

private:
   SOCKET _sock;
};
