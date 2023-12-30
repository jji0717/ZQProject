// DialogCreator.h: interface for the DialogCreator class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIALOGCREATOR_H__C0580F2B_377B_4196_A10F_DB08CE192830__INCLUDED_)
#define AFX_DIALOGCREATOR_H__C0580F2B_377B_4196_A10F_DB08CE192830__INCLUDED_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataPostHouseService.h"
#include "SystemUtils.h"

#define NO_LICENSE_SESSIONS_MAX (500)

#ifndef _NO_NAMESPACE
namespace ZQ {
	namespace StreamSmith {
#endif // #ifndef _NO_NAMESPACE

class RtspDialogCreator: public ZQ::DataPostHouse::IDataDialogFactory, protected ZQ::common::NativeThread
{
public:
	RtspDialogCreator();
	virtual ~RtspDialogCreator();
	
	virtual	void onClose(ZQ::DataPostHouse::IDataDialogFactory::CommunicatorS& comms);
	virtual ZQ::DataPostHouse::IDataDialogPtr onCreateDataDialog(ZQ::DataPostHouse::IDataCommunicatorPtr communicator);
	virtual void onReleaseDataDialog(ZQ::DataPostHouse::IDataDialogPtr dialog, ZQ::DataPostHouse::IDataCommunicatorPtr communicator);

public:
	/// 得到总共的连接数
	uint32 getMainConnCount();

	/// 最大连接数
	uint32 getMaxConnection();

	void setMaxConnection(uint32 maxConns);
	//license 限制时间
	int64 getExpirationTime(){return _connExpireTime;}
	void setExpirationTime(int64& time){_connExpireTime = time;}

	void setIdleTimeout(int32 idleTimeout);
	void setIdleScanInterval(int32 nInterval);

	bool startIdleMonitor();
	void stopIdleMonitor();

protected:		
	typedef std::map<ZQ::DataPostHouse::IDataCommunicatorPtr, ZQ::DataPostHouse::IDataDialogPtr> DialogMap;

	void checkIdleConnection();
	int run(void);

protected:
	int32	_idleScanInterval;
	int32	_idleTimeout;
	SYS::SingleObject		_waitEvent;
	bool	_bMonitorStarted;
	
	DialogMap		_dlgs;
	
protected:
	volatile long		_connCount;
	ZQ::common::Mutex	_lock;
	uint32 _maxConnection;
	int64	_connExpireTime;

};

typedef ZQ::DataPostHouse::ObjectHandle<RtspDialogCreator> RtspDialogCreatorPtr;

#ifndef _NO_NAMESPACE
	} // namespace StreamSmith {
} // namespace ZQ {
#endif // #ifndef _NO_NAMESPACE


#endif // !defined(AFX_DIALOGCREATOR_H__C0580F2B_377B_4196_A10F_DB08CE192830__INCLUDED_)
