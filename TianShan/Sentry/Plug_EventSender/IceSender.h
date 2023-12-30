// IceSender.h: interface for the IceSender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IceSender_H__4A828828_1D06_49F5_B507_8DD8D6CE8CE7__INCLUDED_)
#define AFX_IceSender_H__4A828828_1D06_49F5_B507_8DD8D6CE8CE7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#pragma warning(disable:4503)

#include <NativeThread.h>
#include "BaseSender.h"
#include <deque>
#include <string>
#include <Locks.h>

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceStorm/IceStorm.h>
#include <TsEvents.h>

namespace IceHelper{
    class PublisherCache;
}

class IceSender : public ZQ::common::NativeThread,public BaseSender
{
public:	
	IceSender();
	virtual ~IceSender();

	virtual bool init(void);
	virtual int run(void);
	
	virtual bool GetParFromFile(const char* pFileName);
	virtual void AddMessage(const MSGSTRUCT& msgStruct);
	virtual void Close();
	
protected:
	bool ReadEventFromFile();
	bool SaveEventToFile(std::deque<MSGSTRUCT>& deq,bool bSaveAll = false);	
	void SetCfgName(const char* pFileName);
	bool ConnectICEStorm();	

    bool send(const MSGSTRUCT&);

private:
	ZQ::common::Mutex	_lock;	

	bool						_hExit;
	ZQ::common::Semaphore		_hMsgSem;
	std::deque<MSGSTRUCT>		_msgQue;
	std::string					_strCfgName;
	size_t						_dwPos;
	bool						_bICECon;

	Ice::CommunicatorPtr		_ic;
	//TianShanIce::Events::GenericEventSinkPrx _eventsinkPrx;
    IceHelper::PublisherCache* _pPubCache;

	//configuration item
	std::string					_strManagerCfg;
	int							_nTimeOut;
	int							_nDequeSize;   //deque size if large this size save some record to file

	std::string					_strSaveName;	//failed send event save path
	FILE*                      _hFile;
};
#endif // !defined(AFX_IceSender_H__4A828828_1D06_49F5_B507_8DD8D6CE8CE7__INCLUDED_)
