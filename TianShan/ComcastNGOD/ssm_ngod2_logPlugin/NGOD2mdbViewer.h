#ifndef __MDBVIEWER_H__
#define __MDBVIEWER_H__

#include <ZQ_common_conf.h>
#include <strHelper.h>
#include <TimeUtil.h>
#include <SystemUtils.h>
#include <MsgSenderInterface.h>
//#include <ConfigHelper.h>
#include "NativeThreadPool.h"
#include "Pointer.h"
#include <FileLog.h>
#include "ODBCHelper.h"
#include <algorithm>

#define EVENT_State             6
#define EVENT_PropertiesUpdated 8
#define EVENT_Error             9
#define EVENT_BasicInfo         10

#define NGOD2LOG_SESS			"#Sess.sess"
#define NGOD2LOG_METHOD			"#Sess.Method"
#define NGOD2LOG_STATE			"#Sess.State"
#define NGOD2LOG_BANDWIDTH		"#Sess.BandWidth"
#define NGOD2LOG_STREAMER		"#Sess.Streamer"
#define NGOD2LOG_ERRORMODULE	"#Sess.ErrorModule"
#define NGOD2LOG_LASTERROR		"#Sess.LastError"
#define NGOD2LOG_DESTINATION	"#Sess.Destination"
#define NGOD2LOG_PORT			"#Sess.Port"
#define NGOD2LOG_SOP			"#Sess.SOP"
#define NGOD2LOG_CLIENTSESSION	"#Sess.ClientSessId"
#define NGOD2LOG_ONDEMANDSESSID	"#Sess.OnDemandSessId"
#define NGOD2LOG_PAID			"#Sess.PAID"

#define NGOD2LOG_TABLE			"LiveSession"
#define COL_RTSPSESSION			"RTSPSession"
#define COL_CLIENTSESSION		"ClientSession"
#define COL_LASTOPERATION		"LastOperation"
#define COL_STREAMER			"Streamer"
#define COL_BANDWIDTH			"BandWidth"
#define COL_ONDEMANDSESSIONID	"OnDemandSessionId"
#define COL_INSERVICEAT			"InServiceAt"
#define COL_OUTOFSERVICEAT		"OutOfServiceAt"
#define COL_LASTERROR			"LastError"
#define COL_SOP					"SOP"
#define COL_DESTPORT			"Destination_Port"
#define COL_SETUP				"SETUP"
#define COL_TEARDOWN			"TEARDOWN"
#define COL_TIMESTAMP			"[TimeStamp]"
#define COL_TIMESTAMP_MYSQL		"TimeStamp"
#define COL_PAID				"PAID"
#define COL_CREATEDAT           "Created"

#define DEFAULTDATABASESCHEME "create table "NGOD2LOG_TABLE"("COL_RTSPSESSION" char(16) primary key NOT NULL \
								, "COL_CLIENTSESSION" char(32) \
								, "COL_LASTOPERATION" char(32) \
								, "COL_STREAMER" char(50) \
								, "COL_BANDWIDTH" char(20) \
								, "COL_ONDEMANDSESSIONID" char(50) \
								, "COL_INSERVICEAT" char(30) \
								, "COL_OUTOFSERVICEAT" char(30) \
								, "COL_LASTERROR" char(255) \
								, "COL_SOP" char(128) \
								, "COL_DESTPORT" char(128) \
								, "COL_SETUP" char(128) \
								, "COL_TEARDOWN" char(128) \
								, "COL_PAID" char(128) \
								, "COL_TIMESTAMP" char(30), " \
								COL_CREATEDAT         " char(30) " \
								")"

/*
#define MYSQLDATABASESCHEME "create table "NGOD2LOG_TABLE"("COL_RTSPSESSION" char(16) primary key NOT NULL \
								, "COL_CLIENTSESSION" char(32) \
								, "COL_LASTOPERATION" char(32) \
								, "COL_STREAMER" char(50) \
								, "COL_BANDWIDTH" char(20) \
								, "COL_ONDEMANDSESSIONID" char(50) \
								, "COL_INSERVICEAT" char(30) \
								, "COL_OUTOFSERVICEAT" char(30) \
								, "COL_LASTERROR" char(255) \
								, "COL_SOP" char(128) \
								, "COL_DESTPORT" char(128) \
								, "COL_SETUP" char(128) \
								, "COL_TEARDOWN" char(128) \
								, "COL_PAID" char(128) \
								, "COL_TIMESTAMP_MYSQL" char(30)"
								, "COL_CREATEDAT      " char(30))"

*/
#define MYSQLDATABASESCHEME "CREATE TABLE " NGOD2LOG_TABLE "(" \
								COL_RTSPSESSION       " BIGINT(12) PRIMARY KEY NOT NULL, " \
								COL_CLIENTSESSION     " VARCHAR(40), " \
								COL_LASTOPERATION     " CHAR(32), " \
								COL_STREAMER          " CHAR(50), " \
								COL_BANDWIDTH         " INT(16), " \
								COL_ONDEMANDSESSIONID " CHAR(50), " \
								COL_INSERVICEAT       " DATETIME, " \
								COL_OUTOFSERVICEAT    " DATETIME, " \
								COL_LASTERROR         " VARCHAR(1000), " \
								COL_SOP               " CHAR(128), " \
								COL_DESTPORT          " VARCHAR(128), " \
								COL_SETUP             " VARCHAR(128), " \
								COL_TEARDOWN          " VARCHAR(128), " \
								COL_PAID              " CHAR(128), " \
								COL_TIMESTAMP_MYSQL   " CHAR(30), " \
								COL_CREATEDAT         " DATETIME, " \
								"INDEX (" COL_TIMESTAMP_MYSQL ") )"

#define ERRORSTRLEN 1024

class DBClient : public ZQ::common::SharedObject
{
public:
	typedef ZQ::common::Pointer<DBClient> Ptr;
	DBClient() { _dbconn.init(); }
	virtual ~DBClient()
	{
		_dbconn.disconnect();
	}

	ODBCHelper::MDBViewer _dbconn;
};

class OSTRMDBViewer : public ZQ::common::NativeThread
{
	friend class SessionHistPoster;
	typedef const ::std::map< ::std::string, ::std::string>	mdbStringMap;
	typedef ::std::list< ::std::string> strList;
public:
	OSTRMDBViewer();
	~OSTRMDBViewer();

	::std::string getPropByKey(mdbStringMap &propMap, const char* key);

	inline void SetLogger(::ZQ::common::FileLog &fileLog)
	{
		_fileLog = &fileLog;
	}

	//please input timeOut with second
	inline void SetTimeOut(uint32 timeOut)
	{
		_timeOut = timeOut;
	}

	bool InitDatabase(const std::string& dsn, const std::string& user, const std::string& auth);
	bool InitDatabase(const std::string& dbPath, const ::std::string &templatePath);
	void Uninit();

	int	run(void);

	bool ProcessLog(const MSGSTRUCT &msg);
	
private:
	bool GetMethodStatus(const MSGSTRUCT &msg);
	bool GetUpdatedProperties(const MSGSTRUCT &msg);
	bool GetLastError(const MSGSTRUCT &msg);
	bool GetBasicInfo(const MSGSTRUCT &msg);

	void AddToSessIdList(::std::string &sessId);

	//check if there is a record with this session id
//	bool checkRowStatus(::std::string &strSess, const ::std::string &tableName);

    // wrapper of MDBViewer::query
    // with error message dump automatically
    bool sendQuery(const std::string& sql, ODBCHelper::DBTableData& result);
    // query with table name and column names
    bool sendQuery(const std::string& tbl, const ODBCHelper::StringVector& cols, ODBCHelper::DBTableData& result, const std::string& filter);

    // execute a sql command that no result is expected
    // with error message dump automatically
    bool sendCommand(const std::string& sql, ODBCHelper::DBTableData* result = NULL);

	DBClient::Ptr       _pDBClient;
	ZQ::common::Mutex	_lock;

	strList _sessIdList;

	::ZQ::common::FileLog *_fileLog;
	void WritePropertyLog(const MSGSTRUCT &msg);
	bool bRun;
	::ZQ::common::Cond _cond;
	::ZQ::common::Mutex _mutex;
	uint32 _timeOut;

	::std::string m_strMaxTimeStamp;

	::std::string _dbPath;
	::std::string _dsn;
	::std::string _user;
	::std::string _auth;
	bool CompactDataSource();
#ifndef ZQ_OS_MSWIN
	bool CopyFile(const char* src, const char* dst, bool bFailIfExist);
#endif
	char errStr[ERRORSTRLEN];

	ZQ::common::NativeThreadPool _thrdpool;

	typedef std::map<int, DBClient::Ptr> DBClientMap; // map of threadId to MDBViewer
	DBClientMap     _dbClients; // borrows _lock at inserting/deleting conflict
};

#define PLUGINLOG if (_fileLog) (*_fileLog)
#endif __MDBVIEWER_H__
