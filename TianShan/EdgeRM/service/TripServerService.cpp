#include "TripServerService.h"


////////////////////////////////////////////////////////////////////
//dialog factory
void DialogFactory::onClose( IDataDialogFactory::CommunicatorS& comms )
{
	//do nothing
}

IDataDialogPtr DialogFactory::onCreateDataDialog( ZQ::DataPostHouse::IDataCommunicatorPtr communicator)
{//������������棬������Ҫ�������Լ�ʵ�ֵ�data dialog�� instance���ظ�DataPostHouse,
	//����һ����ÿ����ص�socket���������ݷ����Ժ�dialog��instance����غ����ͻ�
	//������

	// ��������������أ���������ֻʵ����TripServerDialog (�����������TripServerDialog.h����)
	// ��������ֱ�ӷ���һ��TripServerDialog��ʵ������
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
{//��ʼ��DataPostDak��
	//���DataPostDak������������socket�¼��ġ�
	//һ��һ��socket������ĳ���¼���DataPostDak�ͻ��ҵ�
	//��Ӧ��DataDialog������
	if(!mDak){delete mDak;mDak=0;}
	
	mDialogFactory = new DialogFactory(mProcThPool,mLog);
	
    //����һ��data dialog factory,�Ժ����е�dialog�Ĵ����������factory����

	mDakEnv.mLogger = &mLog;
	mDakEnv.dataFactory = mDialogFactory; //ָ�� dialogfactory, ������dataposthouse�����ʧ������dialogFactoryҪ��������
	//���ֻ���趨һ��logger,���ǵ�ǰֻ����console logger
	//���ֻ��һ��ϸ�ڵ����⣬��ʱ�����

	mDak = new ZQ::DataPostHouse::DataPostDak(mDakEnv,mDialogFactory);
	return mDak->startDak(mRecvPoolSize);
}

bool TripSocketServer::addListener( const std::string& ip, const std::string& port )// ����һ���߳�ȥ����socket����...
{
	//������������棬��ʵֻ����һ������
	//Ϊ���ǵ�server����һ��Listen socket,���Ұ�listen socketע�ᵽdataposthouse���� 
	ZQ::DataPostHouse::AServerSocketTcpPtr p = new ZQ::DataPostHouse::AServerSocketTcp(*mDak,mDakEnv);

    // ************************************************
//     p->SetTripSocketServer(mNativeThreadPoolPtr);
    // ************************************************
	if(!p->startServer(ip,port))
		return false;
	//������ɹ���ӵ�listener��¼���������������˳���ʱ�����ô���
	mListeners.push_back(p);
	return true;
}


void TripSocketServer::stop()
{
	if(!mDak)	return;
	std::vector<ZQ::DataPostHouse::AServerSocketTcpPtr>::iterator itListener = mListeners.begin();
	for( ; itListener != mListeners.end(); itListener++)
	{//ͣ�����е�Listener
		(*itListener)->stop();
	}
	mDak->stopDak();//���ڰ�DataPostDakҲͣ������ô��û���κζ�������socket�¼���
}