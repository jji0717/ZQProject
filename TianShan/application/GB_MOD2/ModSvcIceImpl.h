#ifndef __ZQMODApplication_ModSvcIceImpl_H__
#define __ZQMODApplication_ModSvcIceImpl_H__

#define _WINSOCK2API_
#include "ModSvcIce.h"
#include "ote.h"
#include "Surf_Tianshan.h"
#include <IceUtil/IceUtil.h>
#include "urlstr.h"
#include "ZQ_common_conf.h"
#include "ModCfgLoader.h"
#include "GB_IMODHelperObj.h"
namespace ZQMODApplication
{

#if  ICE_INT_VERSION / 100 >= 306
    class OTEForStateCB : public IceUtil::Shared
    {   
    public:
        OTEForStateCB(const std::string& cltSession);
    private:
        void handleException(const Ice::Exception&){}
    public:
        void sessionTeardown(const Ice::AsyncResultPtr&);   
    protected:
        std::string     clientSessionId;
    };    
    typedef IceUtil::Handle<OTEForStateCB> OTEForStateCBPtr;    
#else
const char* oteGetErrorDesc(int nErrorCode);

class OTEForTeardownCB : public com::izq::ote::tianshan::AMI_MoDIceInterface_sessionTeardown
{
public:
	OTEForTeardownCB(const std::string& cltSession);
    virtual void ice_response(const ::com::izq::ote::tianshan::SessionResultData& rd);
    virtual void ice_exception(const ::Ice::Exception& ex);

private:
	std::string		clientSessionId;
};

typedef ::IceUtil::Handle<OTEForTeardownCB> OTEForTeardownCBPtr;
#endif

class ModEnv;
// typedef IceUtil::Mutex ServantMutex;
typedef IceUtil::RecMutex ServantMutex;
//////////////////////////////////////////////////////////////////////////
// implementation of Mod purchase
//////////////////////////////////////////////////////////////////////////
class ModPurchaseImpl : public ZQTianShan::Application::MOD::ModPurchase, public IceUtil::AbstractMutexI<ServantMutex>
{
public: 
	ModPurchaseImpl(ModEnv& env);
	virtual ~ModPurchaseImpl();

public: 
	::TianShanIce::SRM::SessionPrx getSession(const ::Ice::Current& = ::Ice::Current()) const;
	void provision(const ::Ice::Current& = ::Ice::Current());
	void render(const ::TianShanIce::Streamer::StreamPrx& streamPrx, 
				const ::TianShanIce::SRM::SessionPrx& weiwooSessPrx, 
				const ::Ice::Current& = ::Ice::Current());
    void detach(const ::std::string& sessId, 
				const ::TianShanIce::Properties& prop, 
				const ::Ice::Current& = ::Ice::Current());
    void bookmark(const ::std::string& title,
				const ::TianShanIce::SRM::SessionPrx& weiwooSessPrx, 
				const ::Ice::Current& = ::Ice::Current());
    ::Ice::Int getParameters(const ::TianShanIce::StrValues&, const ::TianShanIce::ValueMap&, ::TianShanIce::ValueMap&, const ::Ice::Current& = ::Ice::Current()) const;
	::std::string getId(const ::Ice::Current& = ::Ice::Current()) const;
    ::Ice::Identity getIdent(const ::Ice::Current& = ::Ice::Current()) const;
    ::TianShanIce::Streamer::StreamPrx getStream(const ::Ice::Current& = ::Ice::Current()) const;
    ::std::string getClientSessionId(const ::Ice::Current& = ::Ice::Current()) const;
    ::std::string getServerSessionId(const ::Ice::Current& = ::Ice::Current()) const;
	bool isInService(const ::Ice::Current& = ::Ice::Current()) const;
	void sendStatusNotice(const ::Ice::Current& = ::Ice::Current());

	TianShanIce::ValueMap getPrivataData(const ::Ice::Current& = ::Ice::Current()) const;
	void setPrivateData(const ::std::string& key, const ::TianShanIce::Variant& variant,const ::Ice::Current& = ::Ice::Current());
	void setPrivateData2(const ::TianShanIce::ValueMap& privateData,const ::Ice::Current& = ::Ice::Current());

    bool getAEFromLAM(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData);
	bool getAEForTianShanSurf(ZQ::common::Config::Holder<Urlpattern>& apppathinfo,ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData);
	bool AuthorOnOTE(ZQ::common::Config::Holder<Urlpattern>& apppathinfo, ::TianShanIce::ValueMap& privData);
    bool getAesstInfo(ZQ::common::URLStr& urlObj, ::TianShanIce::ValueMap& privData);

	TianShanIce::Application::PlaylistInfo getPlaylistInfo(const ::Ice::Current& c) const;
protected:
	void TestElementMod(ZQTianShan::Application::MOD::AEReturnData& aeRetData, ::TianShanIce::ValueMap& privData);
private: 
	ModEnv& _env;
	std::vector<TianShanIce::Streamer::PlaylistItemSetupInfo> _playListItemInfos;
	Ice::Long _stampClean;
};

typedef IceInternal::Handle<ModPurchaseImpl> PurchaseIPtr;

} // namespace ZQMODApplication

#endif // #define __ZQMODApplication_ModSvcIceImpl_H__

