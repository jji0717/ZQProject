#ifndef __c2_client_http_dialog_header_file_h__
#define __c2_client_http_dialog_header_file_h__

#include <ZQ_common_conf.h>
#include <DataPostHouse/DataCommunicatorUnite.h>
#include "HttpProtocol.h"
#include "HttpSession.h"
#include "HttpSession.h"

class HttpDialog : public ZQ::DataPostHouse::IDataDialog
{
public:
	HttpDialog( HttpSessionFactoryPtr fac );
	virtual ~HttpDialog(void);
	
	virtual		void		onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;

	virtual		void		onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;

	virtual		bool		onRead( const int8* buffer , size_t bufSize ) ;

	virtual		void		onWritten( size_t bufSize ) ;

	virtual		void		onError( ) ;

	SimpleHttpSessionPtr			getSession();

protected:

	void					parseHttpMessage( const char*& data , size_t& size );
	void					findStartline( const char*& data , size_t& size );
	void					findHeaders( const char*& data , size_t& size );

protected:

	

private:
	SimpleHttpSessionPtr			mSession;
	HttpSessionFactoryPtr			mSessionFactory;
	HttpMessagePtr					mReservedMsg;
	std::string						mReservedData;//because the reserved data must be headers, so we can use string to save it
	ZQ::DataPostHouse::IDataCommunicatorPtr	mComm;	
};
typedef ZQ::DataPostHouse::ObjectHandle<HttpDialog> HttpDialogPtr;

#endif //__c2_client_http_dialog_header_file_h__
