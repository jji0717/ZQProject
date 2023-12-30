
#ifndef __MSGSENDER_ICESENDER_H__
#define __MSGSENDER_ICESENDER_H__

#include "BaseSender.h"
#include <NativeThreadPool.h>

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <TsEvents.h>

namespace IceHelper{
    class PublisherCache;
}


class IceSender : public BaseSender
{
	friend class PublishCmd;
public:
	IceSender(int poolSize = MSGSENDER_POOLSIZE);
	virtual ~IceSender();

	virtual bool init(void);
	virtual bool GetParFromFile(const char* pFileName);
	virtual void AddMessage(const MSGSTRUCT& msgStruct, const MessageIdentity& mid, void* ctx);
	virtual void Close();

protected:
	bool ConnectICEStorm();
	bool send(const MSGSTRUCT&);

	bool checkConnect();
	
private:
	ZQ::common::NativeThreadPool	_thPool;

	Ice::CommunicatorPtr			_ic;
    IceHelper::PublisherCache*		_pPubCache;

	bool							_bICECon;
	ZQ::common::Mutex				_lock;

	//configuration item
	std::string						_strManagerCfg;
	int								_nTimeOut;

	bool							_bQuit;
	SysLog*						_sysLog;
};

#endif //__MSGSENDER_ICESENDER_H__
