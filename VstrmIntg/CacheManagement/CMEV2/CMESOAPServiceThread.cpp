#include "Log.h"
#include "CMESOAPServiceThread.h"


#define  SOAP_ACCEPT_TIMEOUT			600		// seconds
#define  SOAP_NO_CLIENT_CONN_TIMES	    6
#define  CME_SOAP_SERVER_TIMEOUT    (24*60*60) // timeout after 24hrs of inactivity


void logSoapFault(struct soap* soap, ZQ::common::Log& log)
{
    if (soap_check_state(soap))
        log(ZQ::common::Log::L_ERROR, "soap struct state not initialized");
    else if (soap->error)
    {
        const char *c, *v = NULL, *s, **d;
        d = soap_faultcode(soap);
        if (!*d)
            soap_set_fault(soap);
        c = *d;
        if (soap->version == 2)
            v = *soap_faultsubcode(soap);
        s = *soap_faultstring(soap);
        d = soap_faultdetail(soap);
        log(ZQ::common::Log::L_ERROR, "%s%d fault: %s [%s] \"%s\" Detail: %s", soap->version ? "SOAP 1." : "Error ", 
			soap->version ? (int)soap->version : soap->error, c, v ? v : "no subcode", s ? s : "[no reason]", d && *d ? *d : "[no detail]");
    }
}

void undoSoap(void * soap)
{
   soap_destroy((struct soap*)soap); // dealloc C++ data
   soap_end((struct soap*)soap); // dealloc data and clean up
   soap_done((struct soap*)soap); // detach soap struct
   free(soap); 
}  // undo()

namespace CacheManagement {

CMESOAPServiceThread::CMESOAPServiceThread(std::string& bindip, int32 bindport)
{
	_cmeIP = bindip;
	_cmePort = bindport;

	char endp[256];
	snprintf(endp, 250, "http://%s:%d/services/CMEService?wsdl", bindip.c_str(), bindport);

	_endpoint = endp;

	_bThreadRunning = true;
	_firtRun = true;
}

CMESOAPServiceThread::~CMESOAPServiceThread( )
{
}

bool CMESOAPServiceThread::soapbind()
{
    int msocket; // master sockets

    soap_init2(&_cmeSoap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);
	_cmeSoap.connect_timeout = CME_SOAP_SERVER_TIMEOUT;
	_cmeSoap.bind_flags     |= SO_REUSEADDR;	// don't use this in unsecured environments 
	_cmeSoap.namespaces      = namespaces;
	
#ifdef ZQ_OS_LINUX
	_cmeSoap.socket_flags = MSG_NOSIGNAL;
#endif

	// socket bind
	msocket = (int) soap_bind(&_cmeSoap, _cmeIP.c_str(), _cmePort, 1000);
	if (msocket < 0)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(CMESOAPServiceThread, "CME SOAP Server soap_bind failed, bindip=%s, bindport=%d"), 
			_cmeIP.c_str(), _cmePort);
		logSoapFault(&_cmeSoap, glog);
		return false;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(CMESOAPServiceThread, "CME SOAP server soap_bind successfully, socket=%d, bindip=%s, bindport=%d"), 
		msocket, _cmeIP.c_str(), _cmePort);
	return true;
}

bool CMESOAPServiceThread::start( )
{
	// if bind failure, fail the start 
	if( !soapbind() )
	{
		return false;
	}

	return NativeThread::start();
}

void CMESOAPServiceThread::stop( )
{
	// notify thread to exit 
	_bThreadRunning = false;
	//signal the wait event
	_waitEvent.signal();
	waitHandle(1000);			// wait short time, since soap_accept() is blocking, always have to wait for. 
}

bool CMESOAPServiceThread::init(void)
{
	_bThreadRunning = true;

	return true;
}

int	CMESOAPServiceThread::run(void)
{
	int retryInterval = 10;  // seconds

	while (_bThreadRunning)
	{
	    int ssocket; // master and slave sockets
		int soapRet = SOAP_OK;

		// soapbind() was called in start() to make sure the socket can be bind to specified IP and port
		// No invoke soapbind() for the first time. If problem happened on the connection, the SOAP will be re-initialized and re-bind
		if(!_firtRun && !soapbind())
		{
			// timeout in 10 seconds
			_waitEvent.wait(retryInterval*1000);	//ms
			
			continue;
		}
		else 
		{
			// set flag
			_firtRun = false;

			// set socket server parameters
		    _cmeSoap.accept_timeout  = SOAP_ACCEPT_TIMEOUT; // So we can trace EGH failures
			_cmeSoap.recv_timeout    = 1;					// old CME use 1 sec
			
			int loop = 0;  // for logging less 
			while (_bThreadRunning)	// loop for accept client connection
			{
				// ready for accet client connection
				ssocket = (int) soap_accept(&_cmeSoap);
				
				if(!_bThreadRunning)
				{
					break; // exit here if thread is exiting
				}

				if ((ssocket < 0) && !_cmeSoap.errnum)  // Timeout?
				{
					loop++;
					if(SOAP_NO_CLIENT_CONN_TIMES == loop)
					{
						glog(ZQ::common::Log::L_WARNING, CLOGFMT(CMESOAPServiceThread, "SOAP accept timeout %d seconds after accumilitve %d times, either no session activity or client is down"), 
							_cmeSoap.accept_timeout, loop);
					}

					// continue to accept client connection after timeout
					continue;
				}
				
				// socket with a errnum != 0
				if (ssocket < 0)
				{
					glog(ZQ::common::Log::L_INFO, CLOGFMT(CMESOAPServiceThread, "soap_accept() retur socket=%d, soap errnum=%d"), 
						ssocket, _cmeSoap.errnum);
					
					// log gsoap error details
					logSoapFault(&_cmeSoap, glog);
					
					// go to soap re-initialize and socket re-bind logic
					break;
				}

//				glog(ZQ::common::Log::L_DEBUG, CLOGFMT(CMESOAPServiceThread, "accepted connection from IP=%d.%d.%d.%d socket=%d"), 
//					(_cmeSoap.ip >> 24)&0xFF, (_cmeSoap.ip >> 16)&0xFF, (_cmeSoap.ip >> 8)&0xFF, _cmeSoap.ip&0xFF, ssocket);

				// duplicated soap object for the connection 
				struct soap* soapConn = soap_copy(&_cmeSoap);
				
				// call the SOAP API which implemented in CMESOAPServiceImpl.cpp
				soapRet = CMESOAP::soap_serve(soapConn);

				if(soapRet > 0) // ignore timeouts
				{
					glog(ZQ::common::Log::L_WARNING, CLOGFMT(CMESOAPServiceThread, "soap_serve() error, returend %d"), soapRet);

					// logging gsoap error details 
					logSoapFault(&_cmeSoap, glog);

					undoSoap(soapConn);

					// go to soap re-initialize and socket re-bind logic
					break;  
				}
				// release the soap connection object
				undoSoap(soapConn);

			}  // while(true) of accepting socket client connection
	    }  // else of soap_bind() succeeded

		// We get here on a SOAP error. Attempt to fix by
		// resetting SOAP as best we can
		// There is a loop for accepting client connection above. 

		soap_destroy(&_cmeSoap); // clean up class instances
		soap_end(&_cmeSoap); // clean up everything and close socket
	    soap_done(&_cmeSoap); // close master socket and detach environment
	}  // while (_bThreadRunning)  // FOREVER loop

	return 0;
}

void CMESOAPServiceThread::final(void)
{
	//do nothing
}

} // end of namespace