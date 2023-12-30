
#ifndef __c2client_connection_factory_header_file_h__cherry__
#define __c2client_connection_factory_header_file_h__cherry__

#include <ZQ_common_conf.h>
#include <Log.h>
#include <DataPostHouse/DataCommunicatorUnite.h>

class DialogFactory : public ZQ::DataPostHouse::IDataDialogFactory
{
public:
	DialogFactory( );
	virtual ~DialogFactory(void);

public:

	virtual	void										onClose( CommunicatorS& comms ) ;

	virtual ZQ::DataPostHouse::IDataDialogPtr			onCreateDataDialog( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;

	virtual void										onReleaseDataDialog( ZQ::DataPostHouse::IDataDialogPtr dialog , ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;

	HttpSessionFactoryPtr								getSessionFactory( );
	

	size_t												getCommunicatorCount( ) const;
private:

	HttpSessionFactoryPtr								mSessFac;
};

typedef ZQ::DataPostHouse::ObjectHandle<DialogFactory>	DialogFactoryPtr;

class ClientService
{
public:
	ClientService( ZQ::common::Log& logger);
	virtual ~ClientService();
public:

	bool startService( size_t threadCount = 20 );

	void stopService( );

	ZQ::DataPostHouse::ASocketPtr connect( const std::string& remoteIp , const std::string& remotePort ,
											const std::string& localIp = "" , const std::string& localPort = "" , 
											ZQ::DataPostHouse::SharedObjectPtr userData = NULL );

	DialogFactoryPtr	getDialogfactory()
	{
		return mDialogFac;
	}
private:
	ZQ::common::Log&		mLogger;
	DialogFactoryPtr		mDialogFac;
	ZQ::DataPostHouse::DataPostHouseEnv*		mDakEnv;

	ZQ::DataPostHouse::DataPostDak*			mDak;
};

#endif//__c2client_connection_factory_header_file_h__cherry__
