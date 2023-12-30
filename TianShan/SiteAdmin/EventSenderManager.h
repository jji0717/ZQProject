#ifndef _SITEADMIN_EVENTSENDER_MANAGER_H__
#define _SITEADMIN_EVENTSENDER_MANAGER_H__

#include "MsgSenderInterface.h"
#include "NativeThread.h"
#include "Locks.h"
#include <vector>
#include "EventChannel.h"
#include <list>
#include <vector>
#include "DynSharedObj.h"

class SiteAdminEnv;


class EventPluginFacet : public ZQ::common::DynSharedFacet
{
	// declare this Facet object as a child of DynSharedFacet
	DECLARE_DSOFACET(EventPluginFacet, DynSharedFacet);

	// declare the API prototypes
	DECLARE_PROC(void, InitModuleEntry, (IMsgSender* pEventDispatcher, const char *type, const char* pText));
	DECLARE_PROC(void, UninitModuleEntry, (IMsgSender* pEventDispatcher));

	// map the external APIs
	DSOFACET_PROC_BEGIN();
		DSOFACET_PROC(InitModuleEntry);
		DSOFACET_PROC(UninitModuleEntry);
	DSOFACET_PROC_END();
};

class EventSenderManager : public IMsgSender , public ZQ::common::NativeThread
{
public:
	EventSenderManager(SiteAdminEnv& env);
	~EventSenderManager();
	
	virtual	bool	regist(const OnNewMessage& pMsg,const char* type ) ;
	virtual	void	unregist( const OnNewMessage& pMsg, const char* type ) ;
	virtual void	ack(const MessageIdentity& mid, void* ctx) {}
public:
	///setup the environment,include load all needed plugin and initialize them	
	bool			SetupEventSenderEnvironment( );
	void			DestroyEventSenderEnvironment( );
	void			PostEvent(MSGSTRUCT* eventMsg );
protected:
	int				run();
	void			startEventChannel();
	void			stopEventChannel();
private:

	typedef std::vector<ZQ::common::DynSharedObj*> PLUGINS;
	PLUGINS			mPlugins;	
	typedef std::vector<OnNewMessage>	VecMessageReceiver;
	typedef std::list<MSGSTRUCT*>		LSTEventMsg;

	VecMessageReceiver								mVecReceiver;
	SiteAdminEnv&									mEnv;
	bool											mbQuit;
	ZQ::common::Semaphore							mSemaphore;
	LSTEventMsg										mLstEvent;
	ZQ::common::Mutex								mLstLocker;
	TianShanIce::Events::EventChannelImpl::Ptr		mEventChannel;
};

const char* FormatLocalTime(char* buf,int size);
const char* SystemTimeToUTC(int64 t,char* buf,int size);


#endif//_SITEADMIN_EVENTSENDER_MANAGER_H__
