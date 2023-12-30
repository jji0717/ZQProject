// DialogCreator.cpp: implementation of the DialogCreator class.
//
//////////////////////////////////////////////////////////////////////

#include "DialogCreator.h"

#include "global.h"

#include "RtspDialog.h"

#include "DialogType.h"

#ifdef _SUPPORT_LSC_PROTOCOL_
	#include "LscDialogImpl.h"
#endif

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

//////////////////////////////////////////////////////////////////////////

using namespace ZQ::DataPostHouse;

RtspDialogCreator::RtspDialogCreator()
{
	_connCount = 0;
	
	_maxConnection = NO_LICENSE_SESSIONS_MAX;
	_connExpireTime = ZQ::common::TimeUtil::now();

	_bMonitorStarted = false;
	_idleTimeout = 3000000;
	_idleScanInterval = 60000;
	

}

RtspDialogCreator::~RtspDialogCreator()
{
	stopIdleMonitor();
}

IDataDialogPtr RtspDialogCreator::onCreateDataDialog(IDataCommunicatorPtr communicator)
{	
#ifdef _RTSP_PROXY
	if (ZQ::common::TimeUtil::now() > _connExpireTime && _maxConnection > NO_LICENSE_SESSIONS_MAX)
	{
		char timeBuffer[64];
		memset(timeBuffer, '\0', sizeof(timeBuffer));
		ZQ::common::TimeUtil::TimeToUTC(_connExpireTime, timeBuffer, sizeof(timeBuffer) - 1, true);
		_maxConnection = NO_LICENSE_SESSIONS_MAX;
		glog(Log::L_WARNING, CLOGFMT(RtspDialogCreator, "onCreateDataDialog() license expiration[%s] exceeded, reduced to maxConnection[%d]."),timeBuffer, _maxConnection);
	}
#endif
	{
		ZQ::common::MutexGuard guard(_lock);
		if ( _dlgs.size() != 0 && _dlgs.size() -1 >= _maxConnection )
		{
			glog(Log::L_WARNING, "RtspDialogCreator::onCreateDataDialog() : Exceed the max connections[%d] count", _maxConnection);
			return NULL;
		}
	}
	IDataDialogPtr dlg = NULL;
	DialogTypePtr dialogTypePtr = DialogTypePtr::dynamicCast(communicator->getUserData());
	int appType = dialogTypePtr->getType();
	if (appType == APP_TYPE_RTSP) 	
	{
		dlg = new RtspDialog(*_GlobalObject::getSessionMgr());		
	}
	else if ( appType == APP_TYPE_LSCP ) 
	{
#ifdef _SUPPORT_LSC_PROTOCOL_
		dlg = new LscDialog( *_GlobalObject::getSessionMgr() );
#else
		dlg =	NULL;
#endif
	}
	else
	{
		glog(Log::L_ERROR,"RtspDialogCreator::onCreateDataDialog() : Invalid APP type[%d] passed in",appType);
		return NULL;
	}
	if (dlg == NULL) 
	{
		glog(ZQ::common::Log::L_ERROR,CLOGFMT(RtspDialogCreator,"Create Dialog failed"));
		return dlg;
	}
	
	ZQ::common::MutexGuard guard(_lock);
	if( _dlgs.find(communicator) != _dlgs.end() ) {
		glog(Log::L_ERROR,CLOGFMT(RtspDialogCreator,"Logic error, trying to bind one communicator on two dialogs"));
		return 0;
	}
	_dlgs.insert(std::make_pair<>(communicator,dlg));
	
	//DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) 
	{		
//		InterlockedIncrement(&_sessionCount);
		_connCount++;
		glog(Log::L_DEBUG, "RtspDialogCreator::onCreateDataDialogue(): "
			"created dialog count %d", _connCount);
	}
	return dlg;
}

void RtspDialogCreator::onReleaseDataDialog(IDataDialogPtr dialog, IDataCommunicatorPtr communicator)
{	
	ZQ::common::MutexGuard guard(_lock);
	DialogMap::iterator itor = _dlgs.find(communicator);
	
	if ( itor == _dlgs.end() )
	{
		return;
	}
	_dlgs.erase(itor);
	//DEBUG_DETAIL(DEBUG_DETAIL_LEVEL3) 
	{
//			InterlockedDecrement(&_sessionCount);
		_connCount--;
		glog(Log::L_DEBUG, "RtspDialogCreator::onReleaseDataDialogue(): "
			"release dailogue count %d", _connCount);
	}

}

void RtspDialogCreator::onClose(IDataDialogFactory::CommunicatorS& comms)
{
	glog(Log::L_DEBUG, "RtspDialogCreator::onClose() : Dialog numbers is [%d] when dialog creator close", comms.size());
	IDataDialogFactory::CommunicatorS::iterator it = comms.begin();
	for(; it != comms.end(); it++ )
	{
		(*it)->close();
	}
	ZQ::common::MutexGuard guard(_lock);
	_dlgs.clear();
}

uint32 RtspDialogCreator::getMainConnCount()
{
	ZQ::common::MutexGuard gd(mCommLocker);
	uint32 nTotalCount = _dlgs.size();
	return nTotalCount;
}

uint32 RtspDialogCreator::getMaxConnection()
{
	return _maxConnection;
}

void RtspDialogCreator::setMaxConnection(uint32 maxConns)
{
	_maxConnection = maxConns;
}

void RtspDialogCreator::checkIdleConnection()
{
	std::vector<IDataCommunicatorPtr> vIdleConns;
	std::vector<IDataCommunicatorPtr>::iterator it;
	IDataCommunicatorPtr mainConn;

	{
		IDataDialogFactory::CommunicatorS::iterator ita;
		ZQ::common::MutexGuard gd(mCommLocker);

		for(ita = mComms.begin(); ita != mComms.end(); ita ++)
		{
			mainConn = *ita;

			if (mainConn->getIdleTime() >= _idleTimeout)
			{
				vIdleConns.push_back(mainConn);
			}
		}
	}

	for(it=vIdleConns.begin();it!=vIdleConns.end();it++)
	{
		mainConn = *it;
		/*		IDialogue* dlg = mainConn->getDialogue();
		if (dlg) 
		{
		dlg->onIdlenessTimeout(mainConn);
		}
		*/
		//
		
		std::string strRemoteIp, strRemotePort;
		mainConn->getRemoteAddress(strRemoteIp, strRemotePort);
		int nIdleTime = mainConn->getIdleTime();
		mainConn->close();		
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(IdleTimeout, "Client[%s:%s] connection idle for %dms, idle timeout and close"), 
			strRemoteIp.c_str(), strRemotePort.c_str(), nIdleTime);
	}
}

int RtspDialogCreator::run( void )
{
	glog(ZQ::common::Log::L_INFO, CLOGFMT(IdleTimeout, "check idle time thread enter, idletimeout[%d]ms, idlescaninterval[%d]ms"),
		_idleTimeout, _idleScanInterval);

	while(1)
	{
		SYS::SingleObject::STATE st = _waitEvent.wait(_idleScanInterval);

		if (st != SYS::SingleObject::TIMEDOUT)
			break;	//exit

		checkIdleConnection();
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(IdleTimeout, "check idle time thread leave"));
	return 0;
}

void RtspDialogCreator::setIdleTimeout( int32 idleTimeout )
{
	_idleTimeout = idleTimeout;
}

void RtspDialogCreator::setIdleScanInterval( int32 nInterval )
{
	_idleScanInterval = nInterval;
}

bool RtspDialogCreator::startIdleMonitor()
{
	if (_bMonitorStarted)
		return true;

	start();
	_bMonitorStarted = true;
	return true;
}

void RtspDialogCreator::stopIdleMonitor()
{
	if (!_bMonitorStarted)
		return;

	_waitEvent.signal();
	waitHandle(5000);

	IDataDialogFactory::close();

	_bMonitorStarted = false;
}

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE
