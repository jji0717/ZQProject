#ifndef _SITEADMIN_EVENTSENDER_MANAGER_H__
#define _SITEADMIN_EVENTSENDER_MANAGER_H__

#include "../../Sentry/eventsink/MsgSenderInterface.h"
#include <nativethread.h>
#include <locks.h>
#include <vector>
#include <SiteServiceConfig.h>
#include <eventchannel.h>
#include "SiteAdminSvcEnv.h"

#include <list>

namespace ZQTianShan {
namespace Site {

class EventSenderManager : public IMsgSender , public ZQ::common::NativeThread
{
public:
	EventSenderManager(SiteAdminSvcEnv& env);
	~EventSenderManager();
	
	virtual	bool	regist(const OnNewMessage& pMsg,const char* type ) ;
    virtual	void	unregist( const OnNewMessage& pMsg, const char* type ) ;
    /// acknowledge the sent message
    virtual void ack(const MessageIdentity& mid, void* ctx) {}
public:
	///setup the environment,include load all needed plugin and initialize them	
	bool			SetupEventSenderEnvironment( );
	void			DestroyEventSenderEnvironment( );
	void			PostEvent(const MSGSTRUCT& eventMsg );
protected:
	int				run();
private:
	typedef struct _ModuleInfo 
	{
		HMODULE			_hDll;
		std::string		_strPathDll;
	}ModuleInfo;
	typedef std::vector<ModuleInfo>		vecDll;
	typedef std::vector<OnNewMessage>	VecMessageReceiver;
	typedef std::list<MSGSTRUCT>		LSTEventMsg;

	VecMessageReceiver								_vecReceiver;
	vecDll											_vecDll;
	SiteAdminSvcEnv&								_env;
	bool											_bQuit;
	HANDLE											_hNewEvent;
	LSTEventMsg										_lstEvent;
	ZQ::common::Mutex								_lstLocker;
};

const char* FormatLocalTime(char* buf,int size);
const char* SystemTimeToUTC(__int64 t,char* buf,int size);

class EventTranslator: public ZQ::common::NativeThread
{
public:
    EventTranslator(SiteAdminSvcEnv& env, EventSenderManager& ssMan);
    ~EventTranslator();
    virtual int run();

private:
    SiteAdminSvcEnv& _env;
    EventSenderManager& _ssMan;
    HANDLE _hQuit;
};

}}//namespace ZQTianShan::Site

#endif//_SITEADMIN_EVENTSENDER_MANAGER_H__