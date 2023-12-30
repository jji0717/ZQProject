#include   "stdafx.h"
#include   "ClientDialog.h"
#include   "vrep.h"
#include   "ClientRequestHandle.h"
#include	"ClientSockets.h"

extern char* openMsg;
extern char* updateMsg;
extern char* keepAliveMsg;
extern int  openMsgSize;
extern int updateMsgSize;
extern int keepAliveMsgSize;

void ClientHeartbeat::add(ClientDialog* aClientDialogPtr)
{
	//ZQ::common::MutexGuard guard(mLockSessList);
    mClientDialogPtrs.push_back(aClientDialogPtr);
    if(mClientDialogPtrs.size() == 1)
        mWakeUpEvent.signal();
}

void ClientHeartbeat::remove(ClientDialog* clientDialog)
{
	mClientDialogPtrs.remove(clientDialog);
}

int ClientHeartbeat::run(void)
{
	if(mBHeartStarted)
		return 0;
	mBHeartStarted = true;

    int nextSleep = 1000;
	glog(ZQ::common::Log::L_INFO,CLOGFMT(ClientHeartbeat,"start sent heart beat..."));
	printf("start sent heart beat...\n");
    while(!mQuit)
    {
        /*while(mTestSessions.size() <=0 && !mQuit)*/
        {
            mWakeUpEvent.wait(nextSleep);
        }

        if(mQuit)
            break;

        // scan for those sessions that met timeout, and determine the minimal sleep time
        nextSleep = 1000;
        {
            //ZQ::common::MutexGuard guard(mLockSessList);
			if(0 == mClientDialogPtrs.size())
				stop();

            //printf("This is in SessionManager::run(), ClientDialogVectorSize = %u\n",mClientDialogPtrS.size());
            for (ClientDialogPtrs::iterator it = mClientDialogPtrs.begin(); !mQuit && it != mClientDialogPtrs.end();)
            {
				int64 stampNow = ZQ::common::TimeUtil::now();
                if ((*it)->mDialogTimeOut <=0)
                    continue;
				if( ZQ::common::TimeUtil::now() - (*it)->mTimeWasBorn > (*it)->mHoldTime )
				{
					(*it)->stop();
					it = mClientDialogPtrs.begin();
				}
				else
				{
					int32 workSessionTimeOut = (int32)((*it)->mDialogTimeOut*0.75) + 500;
					int64 timeout = (*it)->mLastUpdate + workSessionTimeOut - stampNow;

					if (timeout <= 0)
					{
						char buf[256] = "";
						memset(buf,0,sizeof(buf));
						ZQ::common::TimeUtil::TimeToUTC(ZQ::common::TimeUtil::now(),buf,sizeof(buf));
						(*it)->OnTimer();
					}
					else if(timeout < nextSleep)
					{
						nextSleep = (int)timeout;
					}
					++it;
				}
                //   ********************************************************************************************

            }
        }
        mWakeUpEvent.wait(nextSleep);
    }
    return 0;
}

void ClientHeartbeat::stop()
{
	glog(ZQ::common::Log::L_INFO,CLOGFMT(ClientHeartbeat,"stop send heart beat"));
#ifdef _DEBUG
	printf("stop send heart beat...\n");
#endif
    mQuit = true;
    mWakeUpEvent.signal();
}

static  ClientHeartbeatPtr  clientHeartbeat = new ClientHeartbeat;

ClientDialog::ClientDialog(ZQ::common::NativeThreadPool & thPool, ZQ::common::Log& aLog)
:mLog(aLog), mThPool(thPool)
{
    //printf("This is in ClientDialog::ClientDialog(),have create ClientDialog! \n");
    //mPrevMsgIndex = 0;
   // mIsInHeartBeatVector = false;
	clientHeartbeat->add(this);
	mLastUpdate = ZQ::common::now();
	mLastMsgRecved = LASTMSG_NULL;
}

ClientDialog::~ClientDialog(void)
{

}

void ClientDialog::startHeartbeat()
{
	clientHeartbeat->start();
}

void ClientDialog::stopHeartbeat()
{
	clientHeartbeat->stop();
}

void ClientDialog::stop()
{
	glog(ZQ::common::Log::L_INFO,CLOGFMT(ClientDialog,"Session[%lld] stop ClientDialog"),mComm->getCommunicatorId());
	clientHeartbeat->remove(this);
	mComm->close();
	
}

void show(uint8* buf,uint16 size)
{
	int i = 0;
	for(;i<size;i++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\nmsg count:%d\n",size);

}


void ClientDialog::onCommunicatorSetup( ZQ::DataPostHouse::IDataCommunicatorPtr communicator )
{
    mComm = communicator;
	//new  send open message
    
    char buf[4096] = {0};
	//show((uint8*)openMsg,openMsgSize);
	size_t byteProcessed = 0;
	if(!openMsg)
	{
		printf("invalid open message\n");
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(ClientDialog,"invalid open message"));
		return;
	}

	//ZQ::ERRP::ERRPMsg::Ptr pMsg =  ZQ::ERRP::ERRPMsg::parseMessage((uint8*)openMsg,openMsgSize,byteProcessed);
	ZQ::Vrep::OpenMessage openMessage;

	/*
	ZQ::ERRP::StringMap metaData;
	pMsg->toMetaData(metaData);
	ZQ::ERRP::StringMap::iterator iter = metaData.find(ERRPMD_OpenHoldTime);
	mHoldTime = atoi(iter->second.c_str())*1000;
	mTimeWasBorn = ZQ::common::TimeUtil::now();
	mDialogTimeOut = atoi(iter->second.c_str())/3 * 1000;
	*/
	openMessage.getHoldTime((ZQ::Vrep::word&)mHoldTime);
	mTimeWasBorn = ZQ::common::TimeUtil::now();
	mDialogTimeOut = mHoldTime/3;

	if(mDialogTimeOut < 5000)
		mDialogTimeOut = 5000;

	if(openMsgSize)
	{
		mComm->write((int8*)openMsg,openMsgSize);
		glog(ZQ::common::Log::L_INFO,CLOGFMT(ClientDialog,"Session[%lld] had sent a open request"),mComm->getCommunicatorId());
	}
	else
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(ClientDialog,"Session[%lld] no open message"),mComm->getCommunicatorId());
		return;
	}

    printf("This is in ClientDialog::onCommunicatorSetup(), have send OPEN_MESSAGE, sockedId:[%u]\n",mComm->getCommunicatorId());
    ///////////////////////////////////////////////////////
}

void ClientDialog::onCommunicatorDestroyed( ZQ::DataPostHouse::IDataCommunicatorPtr communicator ) 
{
	mComm = 0;
}

bool ClientDialog::onRead( const int8* buffer , size_t bufSize )
{
    uint64   aSocketId  = mComm->getCommunicatorId();
    //printf("This is in ClientDialog::onRead(), , SocketId:[%u]1\n",aSocketId);
	// 这个函数在相关的socket上有数据的时候会被调用
	// 其中bufer是数据的其实地址，bufSize其实是dataSize,也就是数据的大小。
	// 当这个函数被调用的时候，代码作者一般情况下会根据具体协议去解析这个数据

	if(!buffer || bufSize <= 0)
		return true;// no data passed in, reject	
    ClientRequestHandle*  aClientRequestHandlePtr = new ClientRequestHandle(*this,mComm, mThPool,(uint8*)buffer, bufSize, mLog,mLastMsgRecved);
    aClientRequestHandlePtr->start();

	return true;
}

void ClientDialog::onWritten( size_t bufSize )
{
	// 这个由于windows和linux对于发送事件的解释是完全不一样的。
	// 而且我们基本不用异步发送，所以这个函数一般是不管的
}

void ClientDialog::onError( )
{
	//通常情况下，这里面不需要有任何实现。当然，可以写一些日志之类的东西
}

void ClientDialog::OnTimer()
{
    //send keepAlive Message
	if(!mComm)
		return;
    ZQ::DataPostHouse::IDataCommunicatorPtr mCommTemp = mComm;
    int64  aSocketId   =  mCommTemp->getCommunicatorId();
	glog(ZQ::common::Log::L_INFO,CLOGFMT(ClientDialog,"Session[%lld]send a heart beat"),mComm->getCommunicatorId());
    mCommTemp->write(keepAliveMsg,keepAliveMsgSize);
	mLastUpdate = ZQ::common::TimeUtil::now();
}
