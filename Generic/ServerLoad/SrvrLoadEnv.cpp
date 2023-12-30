#include "./SrvrLoadEnv.h"
#include "./ParseXMLData.h"

#define TMTIME ::localtime // ::gmtime

namespace SrvrLoad
{

#define ExecuteFMT(_C, _X) CLOGFMT(_C, "(Sequence: %08I64d)" _X), sequence

SrvrLoadEnv::SrvrLoadEnv() : _pThreadPool(NULL), _scanInterval(0), _pWatchDog(NULL), _bTestMode(false), _bClearDBOnStart(false)
{
}

SrvrLoadEnv::~SrvrLoadEnv()
{
	doUninit();
}

unsigned __int64 SrvrLoadEnv::now()
{
	FILETIME systemtimeasfiletime;
	unsigned __int64 ltime;

	GetSystemTimeAsFileTime(&systemtimeasfiletime);
	memcpy(&ltime,&systemtimeasfiletime,sizeof(ltime));
	ltime /= 10000;  //convert nsec to msec

	return ltime;
}

void SrvrLoadEnv::doStop()
{
	glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "SrvrLoadEnv::doStop()"));

	if (NULL != _pWatchDog)
		_pWatchDog->stop();
}

static bool connectDB(ConnDBInfo& dbInfo)
{
	bool bConnRst = false;
	TCHAR szBuf[1024];
	try
	{
		CString strConn;
		strConn.Format("DSN=%s;UID=%s;PWD=%s", dbInfo._dsnName.c_str(), dbInfo._userID.c_str(), dbInfo._password.c_str());
		DWORD dwOptions = CDatabase::noOdbcDialog;
		dbInfo._dbConn = new CDatabase();
		dbInfo._dbConn->OpenEx(strConn, dwOptions);
		if (dbInfo._dbConn->IsOpen())
		{
			bConnRst = true;
			dbInfo._errCount =0;
		}
	}
	catch (CDBException* ex)
	{
		ex->GetErrorMessage(szBuf, sizeof(szBuf));
		glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "open database[%s] caught CDBException: %s"), dbInfo._dsnName.c_str(), szBuf);
		ex->Delete();
	}
	catch (CException* ex)
	{
		ex->GetErrorMessage(szBuf, sizeof(szBuf));
		glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "open database[%s] caught CException: %s"), dbInfo._dsnName.c_str(), szBuf);
		ex->Delete();
	}
	catch (const char* exStr)
	{
		glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "open database[%s] caught: %s"), dbInfo._dsnName.c_str(), exStr);
	}
	catch (...)
	{
		glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "open database[%s] caught exception"), dbInfo._dsnName.c_str());
	}

	if (bConnRst)
		glog(DebugLevel, CLOGFMT(SrvrLoadEnv, "database[%s] connected"), dbInfo._dsnName.c_str());
	else if (NULL != dbInfo._dbConn)
	{
		delete dbInfo._dbConn;
		dbInfo._dbConn = NULL;
		dbInfo._errCount =0;
	}

	return bConnRst;
}

HRESULT SrvrLoadEnv::doInit(const std::string& configPath)
{
	glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "doInit(%s)"), configPath.c_str());

	ZQ::common::XMLPreferenceDocumentEx xmlDoc;
	bool bRet = xmlDoc.open(configPath.c_str());
	if (false == bRet)
	{
		glog(EmergLevel, CLOGFMT(SrvrLoadEnv, "xmlDoc.open(%s) failed"), configPath.c_str());
		return S_FALSE;
	}
	
	ZQ::common::XMLPreferenceEx* pRoot = xmlDoc.getRootPreference();
	SmartPreferenceEx smtRoot(pRoot);
	if (NULL == pRoot)
	{
		glog(EmergLevel, CLOGFMT(SrvrLoadEnv, "xmlDoc.getRootPreference() failed"));
		return S_FALSE;
	}
	
	std::map<std::string, std::string> rootProps = pRoot->getProperties();
	std::map<std::string, std::string>::iterator root_itor = rootProps.find("TestMode");
	if (rootProps.end() != root_itor && (0 == stricmp(root_itor->second.c_str(), "1") || 0 == stricmp(root_itor->second.c_str(), "yes")))
	{
		glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "TestMode is On"));
		_bTestMode = true;
	}
	
	root_itor = rootProps.find("ClearDBOnStart");
	if (rootProps.end() != root_itor && (0 == stricmp(root_itor->second.c_str(), "1") || 0 == stricmp(root_itor->second.c_str(), "yes")))
	{
		glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "ClearDBOnStart is On"));
		_bClearDBOnStart = true;
	}

	ZQ::common::XMLPreferenceEx* pWatchList = pRoot->findSubPreference("WatchList");
	SmartPreferenceEx smtWatchList(pWatchList);
	if (NULL == pWatchList)
	{
		glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "configuration <ServerLoad><WatchList></WatchList></ServerLoad> not found"));
		return S_FALSE;
	}

	std::map<std::string, std::string> watchListProps = pWatchList->getProperties();
	std::map<std::string, std::string>::iterator watchList_itor = watchListProps.find("ScanInterval");
	_scanInterval = 60 * 1000; // 1 min
	if (watchListProps.end() != watchList_itor)
		_scanInterval = atoi(watchList_itor->second.c_str()) * 1000;	
	if (_scanInterval < 60 * 1000 && false == _bTestMode)
		_scanInterval = 60 * 1000;
	glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "ScanInterval is %d"), _scanInterval);

	int watchNum = 0;
	ZQ::common::XMLPreferenceEx* pFile = pWatchList->firstChild("File");
	SmartPreferenceEx smtFile(pFile);
	while (NULL != pFile)
	{
		std::map<std::string, std::string> FileProps = pFile->getProperties();
		WatchXMLInfo watchInfo;
		watchInfo._fileName = FileProps["Path"];
//		watchInfo._dftCmGroup = atoi(FileProps["CMGroupID"].c_str());
		memset(&watchInfo._ftLastModify, 0, sizeof(watchInfo._ftLastModify));
		watchInfo._properties = FileProps;
		_watchList.push_back(watchInfo);
		glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "%d. watch %s under ServerLoad"), ++ watchNum, watchInfo._fileName.c_str());

		if (!pWatchList->hasNextChild())
			break;

		ZQ::common::XMLPreferenceEx* pTemp = pFile;
		SmartPreferenceEx smtTemp(pTemp);
		pFile = pWatchList->nextChild();
	}

	std::vector<ConnDBInfo> tmpDbInfos;
	ZQ::common::XMLPreferenceEx* pOteDatabase = pRoot->firstChild("OteDatabase");
	SmartPreferenceEx smtOteDatabase(pOteDatabase);
	while (NULL != pOteDatabase)
	{
		std::map<std::string, std::string> oteMap = pOteDatabase->getProperties();		

		ConnDBInfo dbInfo;
		dbInfo._dsnName = oteMap["DSN"];
		dbInfo._userID = oteMap["User"];
		dbInfo._password = oteMap["Password"];
		dbInfo._instanceStamp = (0 != ::atoi(oteMap["instanceStamp"].c_str()));
		dbInfo._dbConn = NULL;
		tmpDbInfos.push_back(dbInfo);

		if (!pRoot->hasNextChild())
			break;

		ZQ::common::XMLPreferenceEx* pTemp = pOteDatabase;
		SmartPreferenceEx smtTemp(pTemp);
		pOteDatabase = pRoot->nextChild();
	}

	for (uint curDbInfo =0; curDbInfo < tmpDbInfos.size(); curDbInfo ++)
	{
		if (connectDB(tmpDbInfos[curDbInfo]))
			_dbInfos.push_back(tmpDbInfos[curDbInfo]);
	}

	glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "defined %d data source, %d connect-able"), tmpDbInfos.size(), _dbInfos.size());

	int iThreadPoolSize = _watchList.size();
	if (iThreadPoolSize > 5)
		iThreadPoolSize = 5;
	_pThreadPool = new ZQ::common::NativeThreadPool(iThreadPoolSize);
	if (NULL == _pThreadPool)
	{
		glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "Create ThreadPool failed"));
		return S_FALSE;
	}
	
	glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "ThreadPool created, size is %d"), iThreadPoolSize);

	_pWatchDog = new WatchDog(*this);
	if (NULL == _pWatchDog)
	{
		glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "Create WatchDog failed"));
		return S_FALSE;
	}
	glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "WatchDog created"));

	return S_OK;
}

void SrvrLoadEnv::doUninit()
{
	glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "doUninit()"));

	if (NULL != _pWatchDog)
	{
		delete _pWatchDog;
		glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "WatchDog destroyed"));
	}
	_pWatchDog = NULL;

	if (NULL != _pThreadPool)
	{
		delete _pThreadPool;
		glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "ThreadPool destroyed"));
	}
	_pThreadPool = NULL;

	for (uint curDbInfo = 0; curDbInfo < _dbInfos.size(); curDbInfo ++)
	{
		if (NULL != _dbInfos[curDbInfo]._dbConn && _dbInfos[curDbInfo]._dbConn->IsOpen())
		{
		_dbInfos[curDbInfo]._dbConn->Close();
		delete _dbInfos[curDbInfo]._dbConn;
		_dbInfos[curDbInfo]._dbConn = NULL;
		glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "data source(%s) closed"), _dbInfos[curDbInfo]._dsnName.c_str());
		}
	}
}

bool SrvrLoadEnv::needReload(WatchXMLInfo& info)
{
	HANDLE hHandle = NULL;
	WIN32_FIND_DATA findData;
	hHandle = ::FindFirstFile(info._fileName.c_str(), &findData);
	if (INVALID_HANDLE_VALUE == hHandle)
	{
		glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "needReload(), ::FindFirstFile(%s) failed, return false."), info._fileName.c_str());
		return false; // 找不到文件不用reload
	}
	::FindClose(hHandle);

	if (true == _bTestMode) // test mode always return true
	{
		glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "needReload(%s), TestMode is true, always return true."), info._fileName.c_str());
		info._ftLastModify = findData.ftLastWriteTime;
		return true;
	}

	if (info._ftLastModify.dwLowDateTime == findData.ftLastWriteTime.dwLowDateTime && info._ftLastModify.dwHighDateTime == findData.ftLastWriteTime.dwHighDateTime) // 如果最后write时间相同，肯定没变
	{
		glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "needReload(%s), LastWriteTime equal to the last time, return false."), info._fileName.c_str());
		return false;
	}

	glog(InfoLevel, CLOGFMT(SrvrLoadEnv, "needReload(%s), LastWriteTime doesn't match the last time, return true."), info._fileName.c_str());
	info._ftLastModify = findData.ftLastWriteTime;
	return true;
}

bool SrvrLoadEnv::getXMLInfo(std::string xmlName, WatchXMLInfo& xmlInfo) const
{
	for (uint i = 0; i < _watchList.size(); i ++)
	{
		if (xmlName == _watchList[i]._fileName)
		{
			xmlInfo = _watchList[i];
			return true;
		}
	}
	
	glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "getXMLInfo(%s) failed"), xmlName.c_str());
	return false;
}

bool SrvrLoadEnv::setXMLInfo(const WatchXMLInfo& xmlInfo)
{
	for (uint i = 0; i < _watchList.size(); i ++)
	{
		if (xmlInfo._fileName == _watchList[i]._fileName)
		{
			_watchList[i] = xmlInfo;
			return true;
		}
	}
	
	glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "setXMLInfo(%s) failed"), xmlInfo._fileName.c_str());
	return false;
}

int SrvrLoadEnv::start(void)
{
	glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "start()"));

	if (true == _bClearDBOnStart)
	{
		for (uint curDbInfo = 0; curDbInfo < _dbInfos.size(); curDbInfo ++)
		{
			glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "start(), ClearDBOnStart is on, do clear database %s."), _dbInfos[curDbInfo]._dsnName.c_str());
			CString strSql;
			strSql = "DELETE FROM ote_server";
			executeSql(_dbInfos[curDbInfo], strSql, 0);
			strSql = "DELETE FROM ote_instance";
			executeSql(_dbInfos[curDbInfo], strSql, 0);
			strSql = "DELETE FROM ote_nodegroup";
			executeSql(_dbInfos[curDbInfo], strSql, 0);
			strSql = "DELETE FROM ote_cm_app";
			executeSql(_dbInfos[curDbInfo], strSql, 0);
		}
	}

	if (NULL == _pWatchDog)
	{
		glog(ErrorLevel, CLOGFMT(SrvrLoadEnv, "start(), WatchDog is NULL."));
		return 1; // error
	}
	
	for (uint i = 0; i < _watchList.size(); i ++)
	{
		int iDuration = (_scanInterval / _watchList.size()) * i + _scanInterval;
		_pWatchDog->watch(_watchList[i]._fileName, iDuration);
	}
	
	_pWatchDog->start();
	glog(NoticeLevel, CLOGFMT(SrvrLoadEnv, "start(), call WatchDog.start()"));
	
	for (uint j = 0; j < _watchList.size(); j ++)
	{
		WatchXMLInfo xmlinfo;
		if (false == getXMLInfo(_watchList[j]._fileName, xmlinfo))
			continue;

		if (false == needReload(xmlinfo))
			continue;

		if (false == setXMLInfo(xmlinfo))
			continue;

		ParseXMLData* pNewParse = new ParseXMLData(_watchList[j]._fileName, *this, *_pThreadPool, j + 1, _watchList[j]._properties);
		pNewParse->start();
	}

	return 0;
}

bool SrvrLoadEnv::executeSql(ConnDBInfo& dbInfo, const CString& sql, __int64 sequence)
{
	glog(DebugLevel, ExecuteFMT(SrvrLoadEnv, "executeSql() [%s]: %s"), dbInfo._dsnName.c_str(), sql);
	try
	{
		ZQ::common::MutexGuard lk(_lockConn);
		if (dbInfo._errCount > 10 && NULL !=dbInfo._dbConn)
		{
			glog(ErrorLevel, ExecuteFMT(SrvrLoadEnv, "force to reconnect[%s] per %d continuous sql errors"), dbInfo._dsnName.c_str(), dbInfo._errCount);
			delete dbInfo._dbConn;
			dbInfo._dbConn = NULL;
			connectDB(dbInfo);
		}

		if (NULL == dbInfo._dbConn || false == dbInfo._dbConn->IsOpen())
		{
			glog(ErrorLevel, ExecuteFMT(SrvrLoadEnv, "data source %s is not connected now."), dbInfo._dsnName.c_str());
			return false;
		}

		dbInfo._dbConn->ExecuteSQL(sql);
		glog(InfoLevel, ExecuteFMT(SrvrLoadEnv, "executeSql() succeeded [%s]: %s"), dbInfo._dsnName.c_str(), sql);
		dbInfo._errCount =0;
	}
	catch (CDBException* ex)
	{
		dbInfo._errCount++;
		TCHAR szBuf[1024];
		szBuf[sizeof(szBuf) - 1] = '\0';
		ex->GetErrorMessage(szBuf, sizeof(szBuf));
		glog(ErrorLevel, ExecuteFMT(SrvrLoadEnv, "executeSql() [%s]: caught CDBException: %s"), dbInfo._dsnName.c_str(), szBuf);
		ex->Delete();
		return false;
	}
	catch (CException* ex)
	{
		dbInfo._errCount++;
		TCHAR szBuf[1024];
		szBuf[sizeof(szBuf) - 1] = '\0';
		ex->GetErrorMessage(szBuf, sizeof(szBuf));
		glog(ErrorLevel, ExecuteFMT(SrvrLoadEnv, "executeSql() [%s]: caught CException: %s"), dbInfo._dsnName.c_str(), szBuf);
		ex->Delete();
		return false;
	}
	catch (const char* exStr)
	{
		dbInfo._errCount++;
		glog(ErrorLevel, ExecuteFMT(SrvrLoadEnv, "executeSql() [%s]: caught %s"), dbInfo._dsnName.c_str(), exStr);
		return false;
	}
	catch (...)
	{		
		dbInfo._errCount++;
		glog(ErrorLevel, ExecuteFMT(SrvrLoadEnv, "executeSql() [%s]: caught unknown exception"), dbInfo._dsnName.c_str());
		return false;
	}

	return true;
}

void SrvrLoadEnv::updateDB(const db_datas& dbDatas, __int64 sequence)
{
	if (_dbInfos.size() == 0)
	{
		glog(WarningLevel, ExecuteFMT(SrvrLoadEnv, "updateDB(), no data source connected."));
		return;
	}

	for (uint curDbInfo = 0; curDbInfo < _dbInfos.size(); curDbInfo ++)
	{
		glog(DebugLevel, ExecuteFMT(SrvrLoadEnv, "updateDB(%s)"), _dbInfos[curDbInfo]._dsnName.c_str());
		const std::vector<ote_server_rec>& ote_servers = dbDatas.get_ote_servers();
		std::vector<ote_server_rec>::const_iterator server_itor;
		for (server_itor = ote_servers.begin(); server_itor != ote_servers.end(); server_itor ++)
		{
			CString strTmp;
			const ote_server_rec& serverRec = *server_itor;
			CString rmvSql;
			rmvSql.Format("DELETE FROM ote_server WHERE cm_group_id = %d", serverRec.cm_group_id);
			executeSql(_dbInfos[curDbInfo], rmvSql, sequence);

			CString insertFields, insertValues;

			insertFields = "cm_group_id";
			strTmp.Format("%d", serverRec.cm_group_id);
			insertValues += strTmp;

			insertFields += ", version";
			strTmp.Format(", \'%s\'", serverRec.version);
			insertValues += strTmp;

			insertFields += ", date_time";
			strTmp.Format(", \'%s\'", serverRec.data_time);
			insertValues += strTmp;

			if (!serverRec.bServerTypeEmpty)
			{
				insertFields += ", server_type";
				strTmp.Format(", %d", serverRec.server_type);
				insertValues += strTmp;
			}

			if (!serverRec.bIntervalTimeEmpty)
			{
				insertFields += ", interval_time";
				strTmp.Format(", %d", serverRec.interval_time);
				insertValues += strTmp;
			}

			if (!serverRec.bPGLevelEmpty)
			{
				insertFields += ", pg_level";
				strTmp.Format(", %d", serverRec.pg_level);
				insertValues += strTmp;
			}

			CString isertSql = CString("INSERT INTO ote_server (") + insertFields + ") VALUES (" + insertValues + ")";
			executeSql(_dbInfos[curDbInfo], isertSql, sequence);

			if (_dbInfos[curDbInfo]._instanceStamp &&  !serverRec.bIntervalTimeEmpty &&  serverRec.interval_time > 10)
			{
				time_t exp = ::time(NULL) - min(3600, serverRec.interval_time *2);
				struct tm* gmTime = TMTIME(&exp);
				
				CString rmvSql;
				rmvSql.Format("DELETE FROM ote_instance WHERE cm_group_id=%d AND last_updated < \'%04d-%02d-%02d %02d:%02d:%02d\' ", serverRec.cm_group_id,
					1900 + gmTime->tm_year, 1+gmTime->tm_mon, gmTime->tm_mday, gmTime->tm_hour, gmTime->tm_min, gmTime->tm_sec);
				executeSql(_dbInfos[curDbInfo], rmvSql, sequence);
			}
		}

		int preNodeGroup = -1;
		const std::vector<ote_instance_rec>& ote_instances = dbDatas.get_ote_instances();
		std::vector<ote_instance_rec>::const_iterator instance_itor;
		for (instance_itor = ote_instances.begin(); instance_itor != ote_instances.end(); instance_itor ++)
		{
			CString strTmp;
			const ote_instance_rec& instanceRec = *instance_itor;
			CString rmvSql;

/* HanGuan's code, wiped by Andy 8/1/2008
			// 删除所有具有该cm_group_id的instance, 这样可以将之前添加进数据库的具有相同cm_group_id的纪录全部删除
			if (preNodeGroup != instanceRec.cm_group_id)
			{
				rmvSql.Format("delete from ote_instance where cm_group_id=%d", instanceRec.cm_group_id);
				if (true == executeSql(_dbInfos[curDbInfo], rmvSql, sequence))
					preNodeGroup = instanceRec.cm_group_id;
			}
*/

			rmvSql.Format("DELETE FROM ote_instance WHERE cm_group_id=%d AND (ip_address=\'%s\' AND port=\'%d\')", instanceRec.cm_group_id, instanceRec.ip_address, instanceRec.port);
			executeSql(_dbInfos[curDbInfo], rmvSql, sequence);

			CString insertFields, insertValues;
			insertFields = "cm_group_id, host_name, ip_address, session_protocal_type";
			strTmp.Format("%d, \'%s\', \'%s\', \'%s\'", instanceRec.cm_group_id, instanceRec.host_name, instanceRec.ip_address, instanceRec.session_protocal_type);
			insertValues += strTmp;

			if (!instanceRec.bPortEmpty)
			{
				insertFields += ", port";
				strTmp.Format(", %d", instanceRec.port);
				insertValues += strTmp;
			}
			
			if (!instanceRec.bServerLoadEmpty)
			{
				insertFields += ", server_load";
				strTmp.Format(", %d", instanceRec.server_load);
				insertValues += strTmp;
			}

			if (!instanceRec.bLifeTimeEmpty)
			{
				insertFields += ", life_time";
				strTmp.Format(", %d", instanceRec.life_time);
				insertValues += strTmp;
			}

			if (_dbInfos[curDbInfo]._instanceStamp)
			{
				insertFields += ", last_updated";
				time_t now = ::time(NULL);
				struct tm* gmTime = TMTIME(&now);

				strTmp.Format(", \'%04d-%02d-%02d %02d:%02d:%02d\'",
					   1900 + gmTime->tm_year, 1+gmTime->tm_mon, gmTime->tm_mday, gmTime->tm_hour, gmTime->tm_min, gmTime->tm_sec);
				insertValues += strTmp;
			}

			CString InsertSql = "INSERT INTO ote_instance (" + insertFields + ") VALUES (" + insertValues + ")";
			executeSql(_dbInfos[curDbInfo], InsertSql, sequence);
		}
		
		preNodeGroup = -1; // 重置为-1
		const std::vector<ote_group_rec>& ote_groups = dbDatas.get_ote_groups();
		std::vector<ote_group_rec>::const_iterator group_itor;
		for (group_itor = ote_groups.begin(); group_itor != ote_groups.end(); group_itor ++)
		{
			const ote_group_rec& groupRec = *group_itor;
			CString rmvSql;
			if (preNodeGroup != groupRec.cm_group_id)
			{
				rmvSql.Format("DELETE FROM ote_nodegroup WHERE cm_group_id=%d", groupRec.cm_group_id);
				if (true == executeSql(_dbInfos[curDbInfo], rmvSql, sequence))
					preNodeGroup = groupRec.cm_group_id;
			}
			CString isertSql;		
			isertSql.Format("INSERT INTO ote_nodegroup(cm_group_id, nodegroup) "
				"VALUES (%d, %I64d)", groupRec.cm_group_id, groupRec.node_group);
			executeSql(_dbInfos[curDbInfo], isertSql, sequence);
		}
		
		preNodeGroup = -1;
		const std::vector<ote_cm_app_rec>& ote_cm_apps = dbDatas.get_ote_cm_apps();
		std::vector<ote_cm_app_rec>::const_iterator cm_app_itor;
		for (cm_app_itor = ote_cm_apps.begin(); cm_app_itor != ote_cm_apps.end(); cm_app_itor ++)
		{
			const ote_cm_app_rec& cmAppRec = *cm_app_itor;
			CString rmvSql;
			if (preNodeGroup != cmAppRec.cm_group_id)
			{
				rmvSql.Format("DELETE FROM ote_cm_app WHERE cm_group_id=%d", cmAppRec.cm_group_id);
				if (true == executeSql(_dbInfos[curDbInfo], rmvSql, sequence))
					preNodeGroup = cmAppRec.cm_group_id;
			}
			
			CString isertSql;
			isertSql.Format("INSERT INTO ote_cm_app(cm_group_id, app_type) "
				"VALUES (%d, \'%s\')", cmAppRec.cm_group_id, cmAppRec.app_type);
			executeSql(_dbInfos[curDbInfo], isertSql, sequence);
		}
	}
}

SmartPreferenceEx::SmartPreferenceEx(ZQ::common::XMLPreferenceEx*& p) : _p(p)
{
}

SmartPreferenceEx::~SmartPreferenceEx()
{
	if (NULL != _p)
		_p->free();
	_p = NULL;
}

}

