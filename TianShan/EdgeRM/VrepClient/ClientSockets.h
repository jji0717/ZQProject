#ifndef __CLIENT_DAK_H__
#define __CLIENT_DAK_H__

#include "DataCommunicatorUnite.h"
#include "ClientDialog.h"

//the dataDak will invoke this instance to create dialog defined by users,
//if there are data arrived ,it will invoke onread() in each dialog
class ClientDialogFactory:public ZQ::DataPostHouse::IDataDialogFactory
{
public:
	ClientDialogFactory(ZQ::common::NativeThreadPool& mThPool, ZQ::common::Log& aLog): mLog(aLog),mThPool(mThPool){}
protected:
	virtual	void										onClose( CommunicatorS& comms );

	virtual ZQ::DataPostHouse::IDataDialogPtr			onCreateDataDialog( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;

	virtual void										onReleaseDataDialog( ZQ::DataPostHouse::IDataDialogPtr dialog , 
																				ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) ;
private:

	ZQ::common::NativeThreadPool&  mThPool;
	ZQ::common::Log& mLog;
};
typedef ZQ::DataPostHouse::ObjectHandle<ClientDialogFactory> ClientDialogFactoryPtr;

#define FILE_OPENMSG		"openMsg.txt"
#define FIlE_UPDATEMSG		"updateMsg.txt"
#define FILE_KEEPALIVEMSG	"keepAliveMsg.txt"

//create a dataDak,and add dialog to the dak
class ClientSockets
{
public:
	ClientSockets(ZQ::common::Log& aLog,ZQ::common::NativeThreadPool& mThPool):mLog(aLog),mThPool(mThPool){}
	virtual ~ClientSockets(void);

	bool	start( int recvThPool , int socketsCount ,uint64 interval);

	bool    setSocketPara(std::string remoteIP,int remotePort);

	bool	createMsg(uint16 holdTime);

	void	stop();

	typedef struct clientSocketInfo{
		std::string		_remoteIp;
		int				_remotePort;
	}ClientSocketInfo;

private:
	ZQ::DataPostHouse::DataPostHouseEnv					mDakEnv;
	ZQ::DataPostHouse::DataPostDak*						mDak;
	ClientDialogFactoryPtr								mDialogFactoryPtr;
	ZQ::common::Log&									mLog;
	ZQ::common::NativeThreadPool&						mThPool;
	std::list<ZQ::DataPostHouse::AClientSocketTcpPtr>	mClientSockets;
	ClientSocketInfo									mClientSocketInfo;
};

#endif //__CLIENT_DAK_H__