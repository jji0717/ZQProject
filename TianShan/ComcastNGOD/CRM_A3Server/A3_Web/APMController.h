// FileName : APMController.h
// Author   : Zheng Junming
// Date     : 2009-06
// Desc     : handler of asset propagate web page

#ifndef __APMWEB_CONTROLLER_H__
#define __APMWEB_CONTROLLER_H__

#include <Ice/Ice.h>
#include <A3Module.h>

class IHttpRequestCtx;

namespace APMWeb
{

class Controller
{
public:
	Controller(IHttpRequestCtx *pHttpRequestCtx);
	~Controller(void);

public:
	bool listAssets();
	bool listAssetReplica();
	bool deleteReplica();

private:
	bool init();
	void finalize();
	void getMetaData(const char* paramList, TianShanIce::StrValues& metaDataNames, 
		TianShanIce::StrValues& tableHeader);
private:
	const char* _template; // template value
	const char* _endpoint; // endpoint value
	const char* _commonMetaData;
	const char* _replicaMetaData;
private:
	IHttpRequestCtx* _pHttpRequestCtx;
	A3Module::A3FacedePrx _a3FacedeProxy;
	Ice::CommunicatorPtr _ic;  // handle of ice run time 

private:
	static const char* const CS_VAR_MODULE_NAME; // module name 
	static const char* const CS_VAR_TEMPLATE; // template name
	static const char* const CS_VAR_ENDPOINT; // endpoint name
	static const char* const CS_VAR_MAXCOUNT; // maxcount name 
	static const char* const CS_VAR_COMMON_METADATA; // replica common metaData
	static const char* const CS_VAR_REPLIC_METADATA; // replica special metaData

	static const char* const CS_VAR_ASSET_PAID; // current page end PAID
	static const char* const CS_VAR_ASSET_PID;  // current page end PID
	static const char* const CS_VAR_ASSET_PRE_PAID; // previous page end PAID 
	static const char* const CS_VAR_ASSET_PRE_PID; // previous page end PID
	static const char* const CS_VAR_SEARCH_PAID; // search PAID
	static const char* const CS_VAR_SEARCH_PID;  // search PID
	static const char* const CS_VAR_REPLICA_PAID; // replica PAID
	static const char* const CS_VAR_REPLICA_PID; // replica PID
	static const char* const CS_VAR_REPLICA_VOLUME;
	static const char* const CS_VAR_REPLICA_NETID;

};

} // end for APMWeb

#endif // end for __APMWEB_CONTROLLER_H__