#include "TripServerService.h"


////////////////////////////////////////////////////////////////////
//dialog factory
void DialogFactory::onClose( IDataDialogFactory::CommunicatorS& comms )
{
	//do nothing
}

IDataDialogPtr DialogFactory::onCreateDataDialog( ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{//在这个函数里面，我们需要把我们自己实现的data dialog的 instance返回给DataPostHouse,
	//这样一来，每当相关的socket上面有数据发生以后。dialog的instance的相关函数就会
	//被调用

	// 在这个例子里面呢，由于我们只实现了TripServerDialog (这个的声明在TripServerDialog.h里面)
	// 所以我们直接返回一个TripServerDialog的实例即可
	return new TripServerDialog(mProcThPool, mLog);
}

void DialogFactory::onReleaseDataDialog( ZQ::DataPostHouse::IDataDialogPtr dialog, 
										ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
	//do nothing because we do not have any references to dialog or communicator here
}

//////////////////////////////////////////////////////////
// EchoService
TripSocketServer::TripSocketServer(ZQ::common::Log& aLog,int recvPoolSize,int procPoolSize)
:mDak(0),mLog(aLog),mRecvPoolSize(recvPoolSize),mProcThPool(*(new ZQ::common::NativeThreadPool(procPoolSize)))
{

}

TripSocketServer::~TripSocketServer(void)
{
	if(mDak)
	{
		delete mDak;
		mDak = 0;
	}
	if(&mProcThPool)
		delete &mProcThPool;
}

bool TripSocketServer::start()
{//初始化DataPostDak，
	//这个DataPostDak就是用来监听socket事件的。
	//一旦一个socket上有了某个事件，DataPostDak就会找到
	//相应的DataDialog来处理
	if(!mDak){delete mDak;mDak=0;}
	
	mDialogFactory = new DialogFactory(mProcThPool,mLog);
	
    //创建一个data dialog factory,以后所有的dialog的创建都由这个factory来做

	mDakEnv.mLogger = &mLog;
	mDakEnv.dataFactory = mDialogFactory; //指定 dialogfactory, 这里是dataposthouse的设计失误。所以dialogFactory要传入两遍
	//这个只是设定一个logger,我们当前只是用console logger
	//这个只是一个细节的问题，暂时不必深究

	mDak = new ZQ::DataPostHouse::DataPostDak(mDakEnv,mDialogFactory);
	return mDak->startDak(mRecvPoolSize);
}

bool TripSocketServer::addListener( const std::string& ip, const std::string& port )// 产生一个线程去监听socket链接...
{
	//在这个函数里面，其实只做了一件事情
	//为我们的server创建一个Listen socket,并且把listen socket注册到dataposthouse里面 
	ZQ::DataPostHouse::AServerSocketTcpPtr p = new ZQ::DataPostHouse::AServerSocketTcp(*mDak,mDakEnv);

    // ************************************************
//     p->SetTripSocketServer(mNativeThreadPoolPtr);
    // ************************************************
	if(!p->startServer(ip,port))
		return false;
	//把这个成功添加的listener记录下来，将来程序退出的时候还有用处的
	mListeners.push_back(p);
	return true;
}


void TripSocketServer::stop()
{
	if(!mDak)	return;
	std::vector<ZQ::DataPostHouse::AServerSocketTcpPtr>::iterator itListener = mListeners.begin();
	for( ; itListener != mListeners.end(); itListener++)
	{//停掉所有的Listener
		(*itListener)->stop();
	}
	mDak->stopDak();//现在把DataPostDak也停掉，那么就没有任何东西监听socket事件了
}