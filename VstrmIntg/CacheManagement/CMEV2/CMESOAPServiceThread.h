#ifndef _CMESOAPSERVICETHEAD_H
#define _CMESOAPSERVICETHEAD_H

#include <NativeThread.h>
#include "CMESOAPCMEServiceSoapBindingProxy.h"
#include "SystemUtils.h"

namespace CacheManagement {

class CMESOAPServiceThread : public ZQ::common::NativeThread
{
public:

	CMESOAPServiceThread(std::string& bindip, int32 bindport);

	virtual ~CMESOAPServiceThread( );

public:
	
	bool		start( );	

	void		stop( );

protected:

	bool		init(void);
	int			run(void);
	void		final(void);

private:
	struct soap	_cmeSoap;

	std::string _endpoint;
	std::string _cmeIP;
	int32 _cmePort;
	
	bool					_bThreadRunning;
	SYS::SingleObject		_waitEvent;
	// to bind the ip and port if bind failure, it's configuration
	bool _firtRun;
	bool soapbind();
};

}
#endif
