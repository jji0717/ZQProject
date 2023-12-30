
#include "HttpSession.h"
#include "HttpDialog.h"
#include "ConnectionFactory.h"
#include <DataCommunicatorUnite.h>

DialogFactory::DialogFactory(void)
{
	mSessFac = new HttpSessionFactory();
}

DialogFactory::~DialogFactory(void)
{
}

void DialogFactory::onClose( CommunicatorS& comms )
{	
}

ZQ::DataPostHouse::IDataDialogPtr DialogFactory::onCreateDataDialog( ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	HttpDialogPtr dialog =  new HttpDialog(mSessFac);
	SimpleHttpSessionPtr sess = dialog->getSession();
	sess->onCreated(communicator);
	return dialog;
}

void DialogFactory::onReleaseDataDialog( ZQ::DataPostHouse::IDataDialogPtr dialog , ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	
}

size_t DialogFactory::getCommunicatorCount( ) const
{
	ZQ::common::MutexGuard gd(mCommLocker);
	return mComms.size();	
}

HttpSessionFactoryPtr DialogFactory::getSessionFactory( )
{
	return mSessFac;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
ClientService::ClientService(ZQ::common::Log& logger)
:mLogger(logger),
mDakEnv(NULL),
mDak(NULL)
{
}

ClientService::~ClientService()
{

}

bool ClientService::startService(  size_t threadCount )
{
	stopService();
	mDakEnv = new ZQ::DataPostHouse::DataPostHouseEnv();
	mDakEnv->mLogger = &mLogger;
	mDakEnv->mReadBufferSize = 64 * 1024;

	mDialogFac = new DialogFactory();
	mDakEnv->dataFactory = mDialogFac;

	mDak = new ZQ::DataPostHouse::DataPostDak( *mDakEnv, mDialogFac );
	return mDak->startDak( (int32)threadCount );
}

void ClientService::stopService( )
{
	if( mDak )
	{
		mDak->stopDak();
		delete mDak;
		mDak = NULL;
	}
	if( mDakEnv )
	{
		delete mDakEnv;
		mDakEnv = NULL;
	}
	mDialogFac = NULL;
}

ZQ::DataPostHouse::ASocketPtr ClientService::connect( const std::string& remoteIp , const std::string& remotePort , 
													 const std::string& localIp , const std::string& localPort , 
													 ZQ::DataPostHouse::SharedObjectPtr userData )
{
	
	ZQ::DataPostHouse::AClientSocketTcpPtr client = new ZQ::DataPostHouse::AClientSocketTcp( *mDak , *mDakEnv , mDialogFac ,userData);

	if( client->connectTo(remoteIp,remotePort,(uint32)-1,localIp,localPort) )
	{
		ZQ::DataPostHouse::TcpCommunicatorSettings setting(client);
		setting.setReadBufSize( mDakEnv->getReadBufferSize() );
		if( client->addToDak())
			return client;
		else
			return NULL;
	}
	else
	{
		return NULL;
	}
}
