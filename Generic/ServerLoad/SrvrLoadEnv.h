#ifndef __SrvrLoadEnv_H__
#define __SrvrLoadEnv_H__

#define _WINSOCK2API_
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "XMLPreferenceEx.h"
#include "Locks.h"

#include <string>
#include <vector>

#undef _WINDOWS_
#include <afxdb.h>

#include "./db_datas.h"
#include "./WatchDog.h"

namespace SrvrLoad
{

#define DebugLevel ZQ::common::Log::L_DEBUG
#define InfoLevel ZQ::common::Log::L_INFO
#define WarningLevel ZQ::common::Log::L_WARNING
#define NoticeLevel ZQ::common::Log::L_NOTICE
#define ErrorLevel ZQ::common::Log::L_ERROR
#define EmergLevel ZQ::common::Log::L_EMERG

struct WatchXMLInfo
{
	std::string _fileName;
//	int _dftCmGroup;
	FILETIME _ftLastModify;
	std::map<std::string, std::string> _properties;
};

struct ConnDBInfo
{
	std::string _dsnName;
	std::string _userID;
	std::string _password;
	CDatabase*  _dbConn;
	int         _errCount;
	bool _instanceStamp;
};

class ParseXMLData;
class SrvrLoadEnv
{
	friend class WatchDog;
	friend class ParseXMLData;
public: 
	SrvrLoadEnv();
	virtual ~SrvrLoadEnv();

public: 
	HRESULT doInit(const std::string& configPath);
	void doUninit();
	
	bool executeSql(ConnDBInfo& dbInfo, const CString& sql, __int64 sequence);
	void updateDB(const db_datas& dbDatas, __int64 sequence);

	bool getXMLInfo(std::string xmlName, WatchXMLInfo& xmlInfo) const;
	bool setXMLInfo(const WatchXMLInfo& xmlInfo);
	
	virtual void doStop();

	int start(void);

	bool needReload(WatchXMLInfo& info);

	unsigned __int64 now();

	uint getWatchListSize() {return _watchList.size();}

private: 
	ZQ::common::NativeThreadPool* _pThreadPool;
	std::vector<WatchXMLInfo> _watchList;
	std::vector<ConnDBInfo> _dbInfos;
	ZQ::common::Mutex _lockConn;
	int _scanInterval;
	WatchDog* _pWatchDog;
	bool _bTestMode;
	bool _bClearDBOnStart;

};

class SmartPreferenceEx
{
public: 
	SmartPreferenceEx(ZQ::common::XMLPreferenceEx*& p);
	virtual ~SmartPreferenceEx();

protected: 
	ZQ::common::XMLPreferenceEx*& _p;
};

}

#endif // #define __SrvrLoadEnv_H__

