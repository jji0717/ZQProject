#ifndef SA_DSMCCTHREAD_H_
#define  SA_DSMCCTHREAD_H_
#include "NativeThread.h"
#include "SingleSession.h"

class ClientThread:public ZQ::common::NativeThread
{
public:

	ClientThread(DsmccClientSocket* socket):m_tcpsocket(socket)
	{
	}
	virtual int  run()
	{
		while(1)
		{
			char recvBuf[1024];
			int bytesToRead = sizeof(recvBuf);
 			int bytesRead =m_tcpsocket->RecvBinary(recvBuf,bytesToRead);
//			printf("received len = %d\n",bytesRead);
// 			for(int i = 0; i < bytesRead; i++)
//  				printf("%0x\t ", recvBuf[i]);
// 			printf("\n \n");
 			if (bytesRead > 0)
 			{
 				m_tcpsocket->OnRecvData(recvBuf,bytesRead);
 			}
			if (m_tcpsocket->getSetupStatus()&&m_tcpsocket->getReleaseStatus())
				return 0;
		}
		return 0;
	}
private:
		DsmccClientSocket * m_tcpsocket;
};
#endif