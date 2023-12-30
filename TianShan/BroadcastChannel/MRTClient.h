#ifndef __MRTCLIENT_4487177__DWEW_H__
#define __MRTCLIENT_4487177__DWEW_H__

#include "MRTDef.h"
#include "Log.h"
#include "soapsoapMRTProxy.h"
#include "Pointer.h"
#include "Locks.h"

namespace ZQMRT
{ 
	class MRTClient: public ZQ::common::SharedObject
	{
	public:
		MRTClient(ZQ::common::Log& log);
		typedef ZQ::common::Pointer<MRTClient>Ptr;
		~MRTClient(void);
	public:
		bool setup(const MRTEndpointInfo& mrtEndpintInfo, const std::string& AssetName, const std::string& destIp, int destPort, int bitrate, const std::string& srmSessionId,
			std::string& mrtSessinId, std::string& errorMessage);
		bool getStatus(const MRTEndpointInfo& mrtEndpintInfo, const std::string& mrtSessinId, const std::string& srmSessionId,std::string& errorMessage);
		bool teardown(const MRTEndpointInfo& mrtEndpintInfo, const std::string& mrtSessinId, const std::string& srmSessionId,std::string& errorMessage);
	private:
		void logSoapErrorMsg(const soapMRT& soapMRTClient, std::string& errorMsg);	
	protected:
		ZQ::common::Log& _log; 
	};
}
#endif//end define __MRTCLIENT_4487177__DWEW_H__
