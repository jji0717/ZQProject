

#ifndef _FTP_TRANSFER_SESSION_
#define _FTP_TRANSFER_SESSION_

#include "IPushTrigger.h"
#include "Locks.h"
#include <map>
#include "FtpsXfer.h"

extern uint32 _dwSocketReadTimeoutSecs;	

#ifndef MPEG2_TRANSPORT_PACKET_SIZE
#define MPEG2_TRANSPORT_PACKET_SIZE		0x0BC			// 188 byte packets
#endif
#define DEF_BUFFERBLOCK_SIZE	(1000 * MPEG2_TRANSPORT_PACKET_SIZE)


namespace ZQTianShan{
	namespace ContentProvision{
		class IPushSource;
	}
}

using namespace ZQTianShan::ContentProvision;

class FtpsPushXfer:public FtpsXfer, public IPushSource
{
public:
	FtpsPushXfer(FtpConnection& ftps, FtpSite& site, FtpSock* pasv_sock, FtpSock *port_sock, NativeThreadPool& Pool, context_t* pContext=NULL);
	virtual ~FtpsPushXfer();
	
	virtual bool recvFile();

	virtual unsigned int read(void* pBuf, unsigned int nReadBytes);
	virtual void close(bool succ, const char* szErr = 0);

protected:

	std::string		_strLogHint;
	int64			_dwStart;
	int64			_ulFileSize;
	std::string		_strContentKey;
};

#endif
