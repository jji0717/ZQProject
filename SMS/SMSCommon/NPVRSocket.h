#if !defined (NPRVSOCKET_H)
#define NPRVSOCKET_H

#include <afx.h>
#include <WINSOCK2.H>
#include <Mswsock.h>

#define  ERROR_READ		(-1 * FD_READ)
#define  ERROR_CONNECT  (-1 * FD_CONNECT)
#define  ERROR_CLOSE	(-1 * FD_CLOSE)
#define  OPER_SUCCESS	0

class NPVRSocket
{
public:
	NPVRSocket();
	~NPVRSocket();

	bool InitialSocket();

	// client function
	bool Connect(int port, char* ip);
	bool SendC(char* buf, int bufLen);
	bool RecvC(char* buf, int bufLen);
	bool WSASelectC(WSAEVENT& hWSAEvent, long lNetworkEvents);
	bool SelectC(DWORD timeout);
	long GetNetworkEventC(WSAEVENT& hWSAEvent);
	bool CloseSocketC();

	// server function
	bool Listen(int port);
	bool Accept();
	bool SendS(char* buf, int bufLen);
	bool RecvS(char* buf, int bufLen);
	bool WSASelectS(WSAEVENT& hWSAEvent, long lNetworkEvents);
	bool CloseSocketS();

private:
	SOCKET m_socket;
	SOCKADDR_IN m_addr;
	SOCKET m_hSocket;
};


#endif