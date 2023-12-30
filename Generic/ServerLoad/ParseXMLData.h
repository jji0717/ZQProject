#ifndef __ParseTianShan_H__
#define __ParseTianShan_H__

#pragma warning(disable: 4503)
#include <string>

#include <NativeThreadPool.h>
#include <XMLPreferenceEx.h>
#include "./db_datas.h"

namespace SrvrLoad
{
class ConnectionWrap;
class SrvrLoadEnv;
class ParseXMLData : public ZQ::common::ThreadRequest
{
public: 
	ParseXMLData(const std::string& filename, SrvrLoadEnv& env, ZQ::common::NativeThreadPool& pool, __int64 sequence, std::map<std::string, std::string>& properties);
	virtual ~ParseXMLData();

public: 
	const db_datas& get_result() const;

protected: 
	virtual bool init(void);
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected: 
/*
	void parseTianShan();
	void ote_servers_ts();
	void ote_instances_ts();
	void ote_groups_ts();
	void ote_cm_apps_ts();
// */

	void parseAxiom();
	void ote_servers_axm();
	void ote_instances_axm();
	void ote_groups_axm();
	void ote_cm_apps_axm();

protected:
	void parseTianShan2();
	void parseTianShanCmGroup(const std::string& sVersion, const std::string&sDataTime, const std::string&sInterval, ZQ::common::XMLPreferenceEx* pCmGroup);
	void parseTianShanNodeGroup(ote_server_rec& cmGroupInfo, ZQ::common::XMLPreferenceEx* pNodeGroup);
	void parseTianShanInstance(ote_server_rec& cmGroupInfo, ZQ::common::XMLPreferenceEx* pInstance);
	void parseTianShanApp(ote_server_rec& cmGroupInfo, ZQ::common::XMLPreferenceEx* pAppType);
	
private: 
	db_datas _dbDatas;
	SrvrLoadEnv& _env;
	std::string _fileName;
	ZQ::common::XMLPreferenceDocumentEx _xmlDoc;
	ZQ::common::XMLPreferenceEx* _pRoot;
	std::map<std::string, std::string> _properties;
	__int64 _sequence;

};

}

#endif // #define __ParseTianShan_H__

