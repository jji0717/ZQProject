#if !defined(AFX_MODMRTPROXY_H__69FFD043_BFCA_48F8_8FAC_3B414E227B07__INCLUDED_)
#define AFX_MODMRTPROXY_H__69FFD043_BFCA_48F8_8FAC_3B414E227B07__INCLUDED_

#include "MRTStreamService.h"
namespace ZQMODApplication
{ 
	class ModEnv;
	class MODMRTProxy: public  MRTProxy
	{
	public:
		MODMRTProxy(ModEnv& env, Ice::ObjectAdapterPtr& Adapter,const std::string& dbPath ,const std::string& netId,const std::vector<std::string>& streamers, const StreamNetIDToMRTEndpoints& mrtEndpintInfos, ZQ::common::Log& log,int target = 1000 );
	public:
		~MODMRTProxy(void);
	public:
		virtual bool findMRTSetupInfoByStreamId(const std::string& streamId, std::string& AssetName, std::string& destIp, int& destPort,
												int& bitrate, std::string& srmSessionId, std::string& streamNetId);
		virtual bool findMRTSessionIdByStreamId(const std::string& streamId, std::string& mrtSessionId, std::string& srmSessionId, std::string& streamNetId);
		virtual bool updateMRTSessionIdToStore(const std::string& streamId, const std::string& mrtSessionId, const std::string& streamNetId);
	protected:
		ModEnv& _env;
	};
}
#endif // !defined(AFX_MODMRTPROXY_H__69FFD043_BFCA_48F8_8FAC_3B414E227B07__INCLUDED_)
