#include "NGOD2mdbViewer.h"
#include "PluginConfig.h"
#include <sstream>

#ifndef ZQ_OS_MSWIN
#include <sys/types.h>
#include <sys/stat.h>
#endif

//Copy File for linux
#ifndef ZQ_OS_MSWIN
bool OSTRMDBViewer::CopyFile(const char* src, const char* dst, bool bFailIfExist)
{
	struct stat filestat;
	if (bFailIfExist)
	{
		if (stat(dst, &filestat) == 0) //file exist, return false
		{
			return false;
		}
	}

	//copy file
	char buffer[1024];
	FILE *in,*out;
	int len;
	if((in=fopen(src,"r")) == NULL)
	{
		PLUGINLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CopyFile, "CopyFile fail, Can not open source file"));
		return false;
	}
	if((out=fopen(dst,"w")) == NULL)
	{  
		PLUGINLOG(ZQ::common::Log::L_ERROR, CLOGFMT(CopyFile, "CopyFile fail, Can not open/create destination file"));
		return false;
	}

	while((len=fread(buffer, sizeof(char), sizeof(buffer), in)) > 0)
	{  
		fwrite(buffer, sizeof(char), len, out);
		memset(buffer, 0, 1024);
	}

	fclose(out);
	fclose(in);

	return true;
}
#endif
// escape the ' to '' in a string
static std::string sqlEscape(const std::string& s) {
    std::ostringstream buf;
    for(size_t i = 0; i < s.size(); ++i) {
        if(s[i] == '\'') {
            buf << '\'';
        }
        buf << s[i];
    }
    return buf.str();
}

// -----------------------------
// class SessionHistPoster
// -----------------------------
#ifndef MAPSET
#define MAPSET(_MAPTYPE, _MAP, _KEY, _VAL) if (_MAP.end() ==_MAP.find(_KEY)) _MAP.insert(_MAPTYPE::value_type(_KEY, _VAL)); else _MAP[_KEY] = _VAL
#endif // MAPSET
class SessionHistPoster : protected ZQ::common::ThreadRequest
{
public:
	typedef std::map<std::string, std::string> FieldMap;
	typedef std::map<std::string, SessionHistPoster*> PosterMap;
    typedef enum
    {
        PE_OK, PE_ERROR, PE_DUPLICATE
    } PostErr;

	SessionHistPoster(OSTRMDBViewer& viewer, ZQ::common::Log& logger,
					const std::string& rtspSessId, const FieldMap& fields, const std::string& DBType)
		: _viewer(viewer), _log(logger), ThreadRequest(viewer._thrdpool),
		  _rtspSessId(rtspSessId), _fields(fields), _dbtype(DBType)
	{
	}

	PostErr post()
	{
		if (_rtspSessId.empty() || _fields.empty())
			return PE_ERROR;

		ZQ::common::MutexGuard g(_posterLock);
		PosterMap::iterator it = _postersAwait.find(_rtspSessId);
		if (_postersAwait.end() == it || NULL == it->second)
		{
			// this is a new post of a given _rtspSessId with no previous pending posters, queue it into the threadpool
			MAPSET(PosterMap, _postersAwait, _rtspSessId, this);
            return ThreadRequest::start() ? PE_OK:PE_ERROR;
		}

		// must have a pending poster of the same _rtspSessId if reach here, merge the fields into the existing one and
		// skip queue this to the threadpool but take the preivous instead
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionHistPoster, "p[%p] found a pending poster[%p] for the same session[%s], merging"), this, it->second, _rtspSessId.c_str());
		for (FieldMap::iterator itF = _fields.begin(); itF != _fields.end(); itF++)
		{
			if (itF->first.empty())
				continue;
			MAPSET(FieldMap, it->second->_fields, itF->first, itF->second);
		}

		return PE_DUPLICATE;
	}

protected:
	int run(void)
	{
		{
			// remove self from the _postersAwait
			ZQ::common::MutexGuard g(_posterLock);
			_postersAwait.erase(_rtspSessId);
		}

		// compose the SQL statement
		std::string insertFields = COL_RTSPSESSION;
		std::string insertValues = std::string("'") + _rtspSessId + "'";
		std::string updateExprsn, sqlstmt;

		for (FieldMap::iterator it = _fields.begin(); it != _fields.end(); it++)
		{
			if (it->first.empty() || 0 == it->first.compare(COL_RTSPSESSION))
				continue;

            std::string val = sqlEscape(it->second);
			insertFields += ", " + it->first;
			insertValues += ", '" + val + "'";
			updateExprsn += std::string(", ") + it->first + "='" + val + "'";
		}

		sqlstmt += std::string("(") + insertFields + ") VALUES (" + insertValues + ") ";
		if (updateExprsn.length() >2)
			updateExprsn = updateExprsn.substr(2);

		if (0 == _dbtype.compare("mysql5"))
		{
			sqlstmt = std::string("INSERT INTO " NGOD2LOG_TABLE " (") + insertFields + ") VALUES (" + insertValues + ") ";
//            if (std::string::npos != updateExprsn.find('='))
			sqlstmt += std::string("ON DUPLICATE KEY UPDATE ") + updateExprsn;
		}
		else // if (0 == _dbtype.compare("access") || 0 == _dbtype.compare("mysql"))
		{
            sqlstmt = std::string("SELECT count(*) FROM " NGOD2LOG_TABLE " WHERE " COL_RTSPSESSION "='") + _rtspSessId + "'";
            ODBCHelper::DBTableData result;
            bool bNeedUpdate = false;
            if (_viewer.sendCommand(sqlstmt, &result)) {
                if(!result.empty() && !result.begin()->empty() && (*result.begin())[0] == "1") {
                   bNeedUpdate = true;
                }
            }
            if (bNeedUpdate) {
                sqlstmt = std::string("UPDATE " NGOD2LOG_TABLE " SET ") + updateExprsn +" WHERE " COL_RTSPSESSION "='" + _rtspSessId + "'";
            } else {
                sqlstmt = std::string("INSERT INTO " NGOD2LOG_TABLE " (") + insertFields + ") VALUES (" + insertValues + ") ";
            }
		}
	
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionHistPoster, "[%p] SessionHistPoster() executing: %s"), this, sqlstmt.c_str());
		return _viewer.sendCommand(sqlstmt);
	}
	
	void final(int retcode =0, bool bCancelled =false)
	{ delete this; }

protected:

	OSTRMDBViewer&                _viewer;
	ZQ::common::Log&              _log;

	std::string _rtspSessId, _dbtype;
	FieldMap    _fields;

	static PosterMap              _postersAwait;
	static ZQ::common::Mutex      _posterLock;

};

SessionHistPoster::PosterMap SessionHistPoster::_postersAwait;
ZQ::common::Mutex            SessionHistPoster::_posterLock;

// -----------------------------
// utility funcs
// -----------------------------
static char* convertToSQLDatetime(int64 time, char* buf, int maxLen)
{
	if (NULL == buf)
		return "";

#ifdef ZQ_OS_MSWIN
	SYSTEMTIME systime;
	FILETIME filetime;
	time *= 10000; // //convert msec to 100-nsec
	memcpy(&filetime, &time, sizeof(filetime));

	if (::FileTimeToSystemTime(&filetime, &systime) ==FALSE)
		return "";

	static TIME_ZONE_INFORMATION zoneinfo;
	static bool bGotZoneInfo = false;

	if (!bGotZoneInfo)
	{
		if (GetTimeZoneInformation(&zoneinfo) != TIME_ZONE_ID_INVALID)
			bGotZoneInfo =true;
	}

	if (bGotZoneInfo)
	{
		SYSTEMTIME locst;
		if (0 != SystemTimeToTzSpecificLocalTime(&zoneinfo, &systime, &locst))
			memcpy(&systime, &locst, sizeof(systime));
	}

	if (systime.wMilliseconds != 0)
		snprintf(buf, maxLen, "%4d-%02d-%02dT%02d:%02d:%02d.%03d", 
			systime.wYear, systime.wMonth, systime.wDay,
			systime.wHour, systime.wMinute, systime.wSecond, 
			systime.wMilliseconds);
	else
		snprintf(buf, maxLen, "%4d-%02d-%02dT%02d:%02d:%02d", 
			systime.wYear, systime.wMonth, systime.wDay,
			systime.wHour, systime.wMinute, systime.wSecond);
#else
	int64 msec = time%1000;
	int64 sec  = time/1000;
	time_t tt;

	memcpy(&tt, &sec, sizeof(tt));
	struct tm *ptm = localtime(&tt);

	if (msec != 0)
		snprintf(buf, maxLen, "%4d-%02d-%02dT%02d:%02d:%02d.%03d", 
		ptm->tm_year, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, msec);
	else
		snprintf(buf, maxLen, "%4d-%02d-%02dT%02d:%02d:%02d", 
		ptm->tm_year, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
#endif
    return buf;
}

static char* convertToSQLDatetime(const char* ISO8601, char* buf, int maxLen)
{
	int64 time = ZQ::common::TimeUtil::ISO8601ToTime(ISO8601);
	return convertToSQLDatetime(time, buf, maxLen);
}


class SqlBuilder
{
public:
    typedef std::pair<std::string, std::string> Field;

    template <typename FieldsT> 
    std::string buildINSERT(const std::string& tblName, const FieldsT& fields)
    {
        if(tblName.empty() || fields.empty())
            return "";

        std::ostringstream stmt;
        stmt << "INSERT INTO " << tblName << " (";
        for(typename FieldsT::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
            if(it != fields.begin())
                stmt << ",";
            stmt << it->first;
        }
        stmt << ") VALUES (";
        for(typename FieldsT::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
            if(it != fields.begin())
                stmt << ",";
            stmt << "'" << sqlEscape(it->second) << "'";
        }
        stmt << ")";
        return stmt.str();
    }

    template <typename FieldsT>
    std::string buildUPDATE(const std::string& tblName, const FieldsT& fields, const std::string& whereStmt)
    {
        if(tblName.empty() || fields.empty() || whereStmt.empty())
            return "";

        std::ostringstream stmt;
        stmt << "UPDATE " << tblName << " SET ";
        for(typename FieldsT::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
            if(it != fields.begin())
                stmt << ",";
            stmt << it->first << "='" << sqlEscape(it->second) << "'";
        }

        stmt << " " << whereStmt;
        return stmt.str();
    }

    std::string buildWHERE(const Field& matchField)
    {
        std::ostringstream stmt;
        stmt << "WHERE " << matchField.first << "='" << sqlEscape(matchField.second) << "'";
        return stmt.str();
    }

    template <typename FieldsT>
    std::string buildWHERE(const FieldsT& matchFields)
    {
        if(matchFields.empty())
            return "";

        std::ostringstream stmt;
        stmt << "WHERE ";
        for(typename FieldsT::const_iterator it = matchFields.begin(); it != matchFields.end(); ++it)
        {
            if(it != matchFields.begin())
                stmt << " AND ";
            stmt << it->first << "='" << sqlEscape(it->second) << "'";
        }
        return stmt.str();
    }

};

class DBTrace : public ODBCHelper::ITrace
{
public:
    DBTrace(ZQ::common::Log* pLog, ZQ::common::Log::loglevel_t lvl)
        :pLog_(pLog), lvl_(lvl)
    {
    }
    virtual void writeln(const std::string& msg)
    {
        if(pLog_)
        {
            (*pLog_)(lvl_, CLOGFMT(DB, "%s"), msg.c_str());
        }
    }
private:
    ZQ::common::Log* pLog_;
    ZQ::common::Log::loglevel_t lvl_;
};

extern ::ZQ::common::Config::Loader< NGOD2PlugInCfg > pConfig;
const ::std::string TableName = NGOD2LOG_TABLE;
//const ::std::string PIDPAIDTable = NGOD2LOG_PIDPAIDTABLE;
const ::std::string AllResult = "*";

::std::string OSTRMDBViewer::getPropByKey(mdbStringMap &propMap, const char* key)
{
	mdbStringMap::const_iterator iter = propMap.find(key);
	if (iter != propMap.end())
		return (*iter).second;
	else
		return "";
}

OSTRMDBViewer::OSTRMDBViewer()
: _thrdpool(5)
{
	_fileLog = NULL;
	bRun = false;
	_pDBClient = new DBClient();
//	_mdbv.init();
	//m_ThrdHandle = CreateEvent(NULL, TRUE, TRUE, NULL);
	//ResetEvent(m_ThrdHandle);
}

OSTRMDBViewer::~OSTRMDBViewer()
{
	PLUGINLOG(::ZQ::common::Log::L_NOTICE,CLOGFMT(OSTRMDBViewer,"OSTRMDBViewer destruct"));
	_fileLog = NULL;
}

#define _mdbv (_pDBClient->_dbconn)

bool OSTRMDBViewer::InitDatabase(const std::string& dsn, const std::string& user, const std::string& auth)
{
	_dsn = dsn;
	_user = user;
	_auth = auth;
	bool b = _mdbv.connect(dsn, user, auth);
	if (b)
	{
		::std::string sql;
		ODBCHelper::DBTableData result;
		sql = std::string("select count(*) from ") + NGOD2LOG_TABLE;
		b = sendQuery(sql, result);

		//already has a table in database
		if (b)
			return true;

		//create table in database
		ODBCHelper::StringVector scheme;
		if (pConfig._dbPath.type.compare("access") == 0)
			scheme.push_back(DEFAULTDATABASESCHEME);
		else
			scheme.push_back(MYSQLDATABASESCHEME);
		//scheme.push_back(DEFAULTPID_PAIDSCHEME);
		return _mdbv.createScheme(errStr, ERRORSTRLEN, scheme);
	}
	else
    {        
		#if  ICE_INT_VERSION / 100 >= 306
			_mdbv.dumpLastError(&DBTrace(_fileLog, ZQ::common::Log::L_ERROR));
		#else
			DBTrace DB(_fileLog, ZQ::common::Log::L_ERROR);
        	_mdbv.dumpLastError(&DB);
		#endif
		return false;
    }
}

bool OSTRMDBViewer::InitDatabase(const std::string& dbPath, const ::std::string &templatePath)
{
	_dbPath = dbPath;
	bool b = _mdbv.connect(dbPath);
	if (b)
		return b;
	else
	{
		#if  ICE_INT_VERSION / 100 >= 306
			_mdbv.dumpLastError(&DBTrace(_fileLog, ZQ::common::Log::L_INFO));
		#else
			DBTrace DB(_fileLog, ZQ::common::Log::L_INFO);
        	_mdbv.dumpLastError(&DB);
		#endif
        PLUGINLOG(ZQ::common::Log::L_INFO, CLOGFMT(MDBViewer, "Copy db template from [%s] to [%s]"), templatePath.c_str(), dbPath.c_str());
		//try to copy template to the specify data folder
		b = CopyFile(templatePath.c_str(), dbPath.c_str(), true);
#ifdef ZQ_OS_MSWIN
		SetFileAttributes(dbPath.c_str(), FILE_ATTRIBUTE_NORMAL);
#endif
		if (b)
		{
			b = _mdbv.connect(dbPath);
			if (b)
			{
                PLUGINLOG(ZQ::common::Log::L_INFO, CLOGFMT(MDBViewer, "template is ready, create db scheme"));
				ODBCHelper::StringVector scheme;
				scheme.push_back(DEFAULTDATABASESCHEME);
				return _mdbv.createScheme(errStr, ERRORSTRLEN, scheme);
			}
			else
            {
			#if  ICE_INT_VERSION / 100 >= 306
			 	_mdbv.dumpLastError(&DBTrace(_fileLog, ZQ::common::Log::L_ERROR));
			#else
				DBTrace DB(_fileLog, ZQ::common::Log::L_ERROR);
                _mdbv.dumpLastError(&DB);
			#endif
				return false;
            }
		}
		else
        {
            PLUGINLOG(ZQ::common::Log::L_ERROR, CLOGFMT(MDBViewer, "Can't copy db template from [%s] to [%s]"), templatePath.c_str(), dbPath.c_str());
			return b;
        }
	}
}

void OSTRMDBViewer::Uninit()
{
	::ZQ::common::MutexGuard mg(_lock);
	if (bRun)
	{
		bRun = false;
		//terminate();
		while (1)
		{
			PLUGINLOG(::ZQ::common::Log::L_NOTICE,CLOGFMT(OSTRMDBViewer,"OSTRMDBViewer thread normally exit"));		
			//DWORD ret = WaitForSingleObject(m_ThrdHandle, 1000);
			bool ret = _cond.wait(_mutex, 1000);
			if (!ret)
				break;
		}
	}
	_mdbv.disconnect();

	// clean up all the DB clients
	_pDBClient = NULL;
	_dbClients.clear();
}

#define STRTIMELEN 40

int OSTRMDBViewer::run(void)
{
	bRun = true;
	int64 stampLastClean = 0;
	int64 stampLastCompactMDB = 0;
	
	while (bRun)
	{
		int64 stampNow = ZQ::common::TimeUtil::now();
		int  scanIntv = _timeOut *1000 /20; // 1/20 of timeout, covert sec to msec
		
		if (scanIntv < 60*1000) // adjust to no more frequent than every minute
			scanIntv = 60*1000; 
		if (scanIntv > 60*60*1000) // adjust to no slower than every hour
			scanIntv = 60*60*1000; 

		if ((stampNow - stampLastClean) <scanIntv) // limits the scan frequence
		{
			::SYS::sleep(200);
			continue;
		}

		char strExpiration[STRTIMELEN];
		convertToSQLDatetime(stampNow - _timeOut*1000, strExpiration, sizeof(strExpiration)-2);

		::ZQ::common::MutexGuard mg(_lock);
		if (!bRun)
			break;

		//compact access mdb file every 5 min
		if (pConfig._dbPath.type.compare("access") == 0 && (stampNow - stampLastCompactMDB) > std::min<int64>(10*60*1000, scanIntv*10))
		{
			stampLastCompactMDB = stampNow;
			_mdbv.disconnect();

			bool b =false;
			if (_dbPath.empty())
			{
				b = _mdbv.compact(_dsn, _user, _auth);
				_mdbv.connect(_dsn, _user, _auth);
			}
			else
			{
				b = _mdbv.compact(_dbPath);
				_mdbv.connect(_dbPath);
			}

			PLUGINLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRMDBViewer,"OSTRMDBViewer compact data source %s"), (b?"succeeded":"failed"));
		}

		//delete expired rows in database
		::std::string sql;
		if (pConfig._dbPath.type.compare("access") == 0)
			sql = ::std::string("DELETE FROM ") + TableName + " WHERE " + COL_TIMESTAMP + "<'" + strExpiration + "';";
		else
			sql = ::std::string("DELETE FROM ") + TableName + " WHERE " + COL_TIMESTAMP_MYSQL + "<'" + strExpiration + "';";

		sendCommand(sql);
		stampLastClean = stampNow;

		PLUGINLOG(::ZQ::common::Log::L_NOTICE,CLOGFMT(OSTRMDBViewer,"OSTRMDBViewer delete row record which time<'%s'"), strExpiration);
	}

	PLUGINLOG(::ZQ::common::Log::L_INFO,CLOGFMT(OSTRMDBViewer,"OSTRMDBViewer thread over"));
	_cond.signal();
	return 1;
}

//dispatch log event to specified process method
bool OSTRMDBViewer::ProcessLog(const MSGSTRUCT &msg)
{
	bool b = false;

	if (msg.id == EVENT_State)
	{
		b = GetMethodStatus(msg);
	}
	else if (msg.id == EVENT_PropertiesUpdated)
	{
		b = GetUpdatedProperties(msg);
	}
	else if (msg.id == EVENT_Error)
	{
		b = GetLastError(msg);
	}
	else if (msg.id == EVENT_BasicInfo)
	{
		b = GetBasicInfo(msg);
	}
    else
        b = true;
	return b;
}

bool OSTRMDBViewer::GetMethodStatus(const MSGSTRUCT &msg)
{
//	::ZQ::common::MutexGuard mg(_lock);

	OSTRMDBViewer::mdbStringMap &property = msg.property;
	WritePropertyLog(msg);

    std::string strSess = getPropByKey(property, NGOD2LOG_SESS);
	if (strSess.empty())
    {
        PLUGINLOG(::ZQ::common::Log::L_ERROR,CLOGFMT(OSTRMDBViewer,"No session id in the method state message"));
		return false;
    }

	SessionHistPoster::FieldMap fields;
	std::string strMethod = getPropByKey(property, NGOD2LOG_METHOD);
	MAPSET(SessionHistPoster::FieldMap, fields, COL_LASTOPERATION, strMethod);
	char buf[40];
	std::string sqlTime = convertToSQLDatetime(msg.timestamp.c_str(), buf, sizeof(buf)-2);
	std::string rtspResp = getPropByKey(property, NGOD2LOG_STATE);

	MAPSET(SessionHistPoster::FieldMap, fields,
		(std::string::npos != pConfig._dbPath.type.find("mysql")) ? COL_TIMESTAMP_MYSQL :COL_TIMESTAMP, sqlTime);

	if (strMethod.compare("SETUP") == 0)
	{
		MAPSET(SessionHistPoster::FieldMap, fields, COL_SETUP,          rtspResp);
		MAPSET(SessionHistPoster::FieldMap, fields, COL_INSERVICEAT,    sqlTime);

		if (rtspResp.find("200 OK") != std::string::npos) // clean up the last err for SETUP succeed
			MAPSET(SessionHistPoster::FieldMap, fields, COL_LASTERROR,    "");
	}
	else if (strMethod.compare("TEARDOWN") == 0)
	{
		MAPSET(SessionHistPoster::FieldMap, fields, COL_TEARDOWN,       rtspResp);
		MAPSET(SessionHistPoster::FieldMap, fields, COL_OUTOFSERVICEAT, sqlTime);
	}

	try {
		SessionHistPoster* p = new SessionHistPoster(*this, *this->_fileLog, strSess, fields, pConfig._dbPath.type);
		if (NULL !=p)
		{
            switch (p->post())
            {
            case SessionHistPoster::PE_OK:
                return true;
            case SessionHistPoster::PE_ERROR:
                delete p; return false;
            case SessionHistPoster::PE_DUPLICATE:
                delete p; return true;
            }
        }
	}
	catch (...) {}
	return false;

/*
    // collect the basic info fields
    std::vector<SqlBuilder::Field> fields;
    fields.reserve(8);

    std::string strMethod = getPropByKey(property, NGOD2LOG_METHOD);
    // last operation
    fields.push_back(SqlBuilder::Field(COL_LASTOPERATION, strMethod));
    // time stamp
	if (msg.timestamp > m_strMaxTimeStamp)
		m_strMaxTimeStamp = msg.timestamp;

    if (pConfig._dbPath.type.compare("mysql") == 0) {
        fields.push_back(SqlBuilder::Field(COL_TIMESTAMP_MYSQL, msg.timestamp));
    } else {
        fields.push_back(SqlBuilder::Field(COL_TIMESTAMP, msg.timestamp));
    }

    // state
	if (strMethod.compare("SETUP") == 0) {
        fields.push_back(SqlBuilder::Field(COL_SETUP, getPropByKey(property, NGOD2LOG_STATE)));
        // in service stamp
        fields.push_back(SqlBuilder::Field(COL_INSERVICEAT, msg.timestamp));
    }
    else if (strMethod.compare("TEARDOWN") == 0) {
        fields.push_back(SqlBuilder::Field(COL_TEARDOWN, getPropByKey(property, NGOD2LOG_STATE)));
        // out of service stamp
        fields.push_back(SqlBuilder::Field(COL_OUTOFSERVICEAT, msg.timestamp));
    }
    else { // ignore the other methods's state
    }

	bool b = checkRowStatus(strSess, TableName);
	::std::string sql;
	if (b)
	{
        sql = SqlBuilder().buildUPDATE(TableName, fields, SqlBuilder().buildWHERE(SqlBuilder::Field(COL_RTSPSESSION, strSess)));
	}
	else
	{ // basic info must exist
        PLUGINLOG(::ZQ::common::Log::L_WARNING,CLOGFMT(OSTRMDBViewer,"GetMethodStatus() no basic info"));
        fields.push_back(SqlBuilder::Field(COL_RTSPSESSION, strSess));
        sql = SqlBuilder().buildINSERT(TableName, fields);
	}

	PLUGINLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRMDBViewer,"MethodStatus: sql(%s)"), sql.c_str());
	return sendCommand(sql);
*/
}

bool OSTRMDBViewer::GetUpdatedProperties(const MSGSTRUCT &msg)
{
//	::ZQ::common::MutexGuard mg(_lock);
	OSTRMDBViewer::mdbStringMap &property = msg.property;
	WritePropertyLog(msg);

	std::string strSess = getPropByKey(property, NGOD2LOG_SESS);
	if (strSess.empty())
	{
		PLUGINLOG(::ZQ::common::Log::L_ERROR,CLOGFMT(OSTRMDBViewer,"No session id in the updated properties message"));
		return false;
	}

	SessionHistPoster::FieldMap fields;
	MAPSET(SessionHistPoster::FieldMap, fields, COL_STREAMER,  getPropByKey(property, NGOD2LOG_STREAMER));
	MAPSET(SessionHistPoster::FieldMap, fields, COL_BANDWIDTH, getPropByKey(property, NGOD2LOG_BANDWIDTH));
	MAPSET(SessionHistPoster::FieldMap, fields, COL_PAID,      getPropByKey(property, NGOD2LOG_PAID));

	try {
		SessionHistPoster* p = new SessionHistPoster(*this, *this->_fileLog, strSess, fields, pConfig._dbPath.type);
		if (NULL !=p)
		{
            switch (p->post())
            {
            case SessionHistPoster::PE_OK:
                return true;
            case SessionHistPoster::PE_ERROR:
                delete p; return false;
            case SessionHistPoster::PE_DUPLICATE:
                delete p; return true;
            }
		}
	}
	catch (...) {}
	return false;

/*
	// collect the basic info fields
    std::vector<SqlBuilder::Field> fields;
    fields.reserve(8);

    // streamer
    fields.push_back(SqlBuilder::Field(COL_STREAMER, getPropByKey(property, NGOD2LOG_STREAMER)));
    // bandwidth
    fields.push_back(SqlBuilder::Field(COL_BANDWIDTH, getPropByKey(property, NGOD2LOG_BANDWIDTH)));
    // paid
    fields.push_back(SqlBuilder::Field(COL_PAID, getPropByKey(property, NGOD2LOG_PAID)));

	//process access database to insert/update record
	bool b = checkRowStatus(strSess, TableName);

	::std::string sql;

	if (b) {
        sql =  SqlBuilder().buildUPDATE(TableName, fields, SqlBuilder().buildWHERE(SqlBuilder::Field(COL_RTSPSESSION, strSess)));
    }
	else
	{ // basic info must exist
        PLUGINLOG(::ZQ::common::Log::L_WARNING, CLOGFMT(OSTRMDBViewer,"GetUpdatedProperties() no basic info"));
        fields.push_back(SqlBuilder::Field(COL_RTSPSESSION, strSess));
        sql = SqlBuilder().buildINSERT(TableName, fields);
    }

	PLUGINLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRMDBViewer,"PropertiesUpdated: sql(%s)"), sql.c_str());
	return sendCommand(sql);
*/
}

bool OSTRMDBViewer::GetLastError(const MSGSTRUCT &msg)
{
//	::ZQ::common::MutexGuard mg(_lock);
	OSTRMDBViewer::mdbStringMap &property = msg.property;
	WritePropertyLog(msg);

	std::string strSess = getPropByKey(property, NGOD2LOG_SESS);
	if (strSess.empty())
	{
		PLUGINLOG(::ZQ::common::Log::L_ERROR,CLOGFMT(OSTRMDBViewer,"No session id in the session failed message"));
		return false;
	}

	SessionHistPoster::FieldMap fields;
	std::string lastErr = getPropByKey(property, NGOD2LOG_METHOD)+" ";
	lastErr += getPropByKey(property,  NGOD2LOG_LASTERROR);

	MAPSET(SessionHistPoster::FieldMap, fields, COL_LASTERROR, lastErr);

	try {
		SessionHistPoster* p = new SessionHistPoster(*this, *this->_fileLog, strSess, fields, pConfig._dbPath.type);
		if (NULL !=p)
		{
            switch (p->post())
            {
            case SessionHistPoster::PE_OK:
                return true;
            case SessionHistPoster::PE_ERROR:
                delete p; return false;
            case SessionHistPoster::PE_DUPLICATE:
                delete p; return true;
            }
		}
	}
	catch (...) {}
	return false;

/*
	// collect the basic info fields
    std::vector<SqlBuilder::Field> fields;
    fields.reserve(8);

    // last error
    fields.push_back(SqlBuilder::Field(COL_LASTERROR, getPropByKey(property, NGOD2LOG_LASTERROR)));

	//process access database to insert/update record
	bool b = checkRowStatus(strSess, TableName);

	::std::string sql;

	if (b) {
        sql =  SqlBuilder().buildUPDATE(TableName, fields, SqlBuilder().buildWHERE(SqlBuilder::Field(COL_RTSPSESSION, strSess)));
    }
	else
	{ // basic info must exist
        PLUGINLOG(::ZQ::common::Log::L_WARNING, CLOGFMT(OSTRMDBViewer,"GetLastError() no basic info"));
        fields.push_back(SqlBuilder::Field(COL_RTSPSESSION, strSess));
        sql = SqlBuilder().buildINSERT(TableName, fields);
    }

	PLUGINLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRMDBViewer,"LastError: sql(%s)"), sql .c_str());
	return sendCommand(sql);
*/
}

bool OSTRMDBViewer::GetBasicInfo(const MSGSTRUCT &msg)
{
//	::ZQ::common::MutexGuard mg(_lock);

	OSTRMDBViewer::mdbStringMap &property = msg.property;
	WritePropertyLog(msg);

	std::string strSess = getPropByKey(property, NGOD2LOG_SESS);
	if (strSess.empty())
	{
		PLUGINLOG(::ZQ::common::Log::L_ERROR,CLOGFMT(OSTRMDBViewer, "No session id in the basic info"));
		return false;
	}

	char buf[40];
	std::string sqlTime = convertToSQLDatetime(msg.timestamp.c_str(), buf, sizeof(buf)-2);

	SessionHistPoster::FieldMap fields;
	MAPSET(SessionHistPoster::FieldMap, fields, COL_CLIENTSESSION,     getPropByKey(property,  NGOD2LOG_CLIENTSESSION));
	MAPSET(SessionHistPoster::FieldMap, fields, COL_ONDEMANDSESSIONID, getPropByKey(property,  NGOD2LOG_ONDEMANDSESSID));
	MAPSET(SessionHistPoster::FieldMap, fields, COL_SOP,               getPropByKey(property,  NGOD2LOG_SOP));
	MAPSET(SessionHistPoster::FieldMap, fields, COL_DESTPORT,		   getPropByKey(property,  NGOD2LOG_DESTINATION) + "/" + getPropByKey(property, NGOD2LOG_PORT));
	MAPSET(SessionHistPoster::FieldMap, fields, COL_PAID,              getPropByKey(property,  NGOD2LOG_PAID));
	MAPSET(SessionHistPoster::FieldMap, fields, COL_BANDWIDTH,         getPropByKey(property,  NGOD2LOG_BANDWIDTH));
	MAPSET(SessionHistPoster::FieldMap, fields, COL_CREATEDAT,         sqlTime);

	try {
		SessionHistPoster* p = new SessionHistPoster(*this, *this->_fileLog, strSess, fields, pConfig._dbPath.type);
		if (NULL !=p)
		{
            switch (p->post())
            {
            case SessionHistPoster::PE_OK:
                return true;
            case SessionHistPoster::PE_ERROR:
                delete p; return false;
            case SessionHistPoster::PE_DUPLICATE:
                delete p; return true;
            }
		}
	}
	catch (...) {}
	return false;

/*
	// collect the basic info fields
    std::vector<SqlBuilder::Field> fields;
    fields.reserve(8);

    // client session id
    fields.push_back(SqlBuilder::Field(COL_CLIENTSESSION, getPropByKey(property, NGOD2LOG_CLIENTSESSION)));

    // on-demand session id
    fields.push_back(SqlBuilder::Field(COL_ONDEMANDSESSIONID, getPropByKey(property, NGOD2LOG_ONDEMANDSESSID)));

    // SOP
    fields.push_back(SqlBuilder::Field(COL_SOP, getPropByKey(property, NGOD2LOG_SOP)));

    // Destination/Port
    fields.push_back(SqlBuilder::Field(COL_DESTPORT, getPropByKey(property, NGOD2LOG_DESTINATION) + "/" + getPropByKey(property, NGOD2LOG_PORT)));

    // PAID
    fields.push_back(SqlBuilder::Field(COL_PAID, getPropByKey(property, NGOD2LOG_PAID)));

    // Bandwidth
    fields.push_back(SqlBuilder::Field(COL_BANDWIDTH, getPropByKey(property, NGOD2LOG_BANDWIDTH)));

	//process access database to insert/update record
	bool b = checkRowStatus(strSess, TableName);

	::std::string sql;

	if (b) {
        PLUGINLOG(::ZQ::common::Log::L_WARNING,CLOGFMT(OSTRMDBViewer,"Session(%s) already exist in the db when the basic info arrive"), strSess.c_str());
        sql = SqlBuilder().buildUPDATE(TableName, fields, SqlBuilder().buildWHERE(SqlBuilder::Field(COL_RTSPSESSION, strSess)));
    }
	else {
        fields.push_back(SqlBuilder::Field(COL_RTSPSESSION, strSess));
        sql = SqlBuilder().buildINSERT(TableName, fields);
    }
	PLUGINLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRMDBViewer,"GetBasicInfo: sql(%s)"), sql.c_str());
	return sendCommand(sql);
*/
}

//add session id to list if this session id not in list
void OSTRMDBViewer::AddToSessIdList(::std::string &sessId)
{
	OSTRMDBViewer::strList::iterator iter = find(_sessIdList.begin(), _sessIdList.end(), sessId);
	if (iter == _sessIdList.end())
		_sessIdList.push_back(sessId);
}

/*
bool OSTRMDBViewer::checkRowStatus(::std::string &strSess, const ::std::string &tableName)
{
	ODBCHelper::DBTableData result;
	ODBCHelper::StringVector col;
	col.push_back(AllResult);
	bool b = sendQuery(tableName, col, result, ::std::string(COL_RTSPSESSION) + "='" + sqlEscape(strSess) + "'");

	//query fail and return fail
	if (b == false)
		return false;

	//check if has result
	if (result.empty())//no data in result
		return false;
	else//has data in result
		return true;
}
*/

#define WRITEPROPERTYLOG
void OSTRMDBViewer::WritePropertyLog(const MSGSTRUCT &msg)
{
#ifdef WRITEPROPERTYLOG
	OSTRMDBViewer::mdbStringMap &property = msg.property;

	PLUGINLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRMDBViewer,"get event(%s) id(%d) and parse param at %s"), msg.eventName.c_str(), msg.id, msg.timestamp.c_str());
	for (OSTRMDBViewer::mdbStringMap::const_iterator iter = property.begin(); iter != property.end(); iter++)
		PLUGINLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRMDBViewer,"get Param(%s=%s)"), (*iter).first.c_str(), (*iter).second.c_str());
	PLUGINLOG(::ZQ::common::Log::L_DEBUG,CLOGFMT(OSTRMDBViewer,"leave get event and parse param"));
#endif
}

// wrapper of MDBViewer::query
// with error message dump automatically
bool OSTRMDBViewer::sendQuery(const std::string& sql, ODBCHelper::DBTableData& result)
{
    bool succ = _mdbv.query(sql, result);
    if(!succ)
{
	  #if  ICE_INT_VERSION / 100 >= 306
      	  _mdbv.dumpLastError(&DBTrace(_fileLog, ZQ::common::Log::L_ERROR));
	  #else
		  DBTrace DB(_fileLog, ZQ::common::Log::L_ERROR);
      	  _mdbv.dumpLastError(&DB);
	  #endif
}
    return succ;
}

// query with table name and column names
bool OSTRMDBViewer::sendQuery(const std::string& tbl, const ODBCHelper::StringVector& cols, ODBCHelper::DBTableData& result, const std::string& filter)
{
    bool succ = _mdbv.query(tbl, cols, result, filter);
    if(!succ)
	{
		#if  ICE_INT_VERSION / 100 >= 306
      	    _mdbv.dumpLastError(&DBTrace(_fileLog, ZQ::common::Log::L_ERROR));
		#else
			DBTrace DB(_fileLog, ZQ::common::Log::L_ERROR);
			_mdbv.dumpLastError(&DB);
		 #endif	
	}
    return succ;
}


// execute a sql command that no result is expected
// with error message dump automatically
bool OSTRMDBViewer::sendCommand(const std::string& sql, ODBCHelper::DBTableData* result)
{
	DBClient::Ptr pDBClient = _pDBClient;

#ifdef ZQ_OS_MSWIN
	int thrdId = GetCurrentThreadId();
#else
	int thrdId = getpid();
#endif
//	if (0 == pConfig._dbPath.type.compare("mysql"))
	{
		ZQ::common::MutexGuard g(_lock);
		if (!bRun)
			return false;

		// take the per thread client instead
		pDBClient = NULL;

		DBClientMap::iterator itClient = _dbClients.find(thrdId);
		if (_dbClients.end() != itClient)
			pDBClient = itClient->second;

		if (NULL == pDBClient)
		{
			// open a new DB client for this thread
			pDBClient = new DBClient();
			if (NULL == pDBClient)
				return false;

			// pDBClient->_dbconn.init();
			if (! pDBClient->_dbconn.connect(_dsn, _user, _auth))
				return false;

			// insert this new client into the map
			PLUGINLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(OSTRMDBViewer,"established a new DB connection[%p] for thread[%x] to dsn[%s]"), pDBClient.get(), thrdId, _dsn.c_str());
			MAPSET(DBClientMap, _dbClients, thrdId, pDBClient);
		}
	}

	if (!bRun)
		return false;

    int64 t0 = ZQ::common::now();
    bool execOk = false;
    if(result) {
       execOk = pDBClient->_dbconn.exec(sql, *result);
    } else {
        execOk = pDBClient->_dbconn.exec(sql);
    }
    if (execOk)
	{
		PLUGINLOG(ZQ::common::Log::L_INFO, CLOGFMT(OSTRMDBViewer,"it took %lld msec to execute: %s"), ZQ::common::now() - t0, sql.c_str());
		return true;
    }

	// report the error message
	#if  ICE_INT_VERSION / 100 >= 306
		pDBClient->_dbconn.dumpLastError(&DBTrace(_fileLog, ZQ::common::Log::L_ERROR));
	#else
		DBTrace DB(_fileLog, ZQ::common::Log::L_ERROR);
		pDBClient->_dbconn.dumpLastError(&DB);
	#endif	

	// retry if need
	if (pDBClient->_dbconn.connectionLost())
	{
		PLUGINLOG(ZQ::common::Log::L_WARNING, CLOGFMT(OSTRMDBViewer,"DB connection lost detected, reconnecting..."));
		if(pDBClient->_dbconn.reconnect())
		{
			PLUGINLOG(ZQ::common::Log::L_INFO,CLOGFMT(OSTRMDBViewer,"DB reconnect OK. retry the last command."));
			MAPSET(DBClientMap, _dbClients, thrdId, pDBClient);
            if(result) {
                execOk = pDBClient->_dbconn.exec(sql, *result);
            } else {
                execOk = pDBClient->_dbconn.exec(sql);
            }
			if(execOk)
			{
                PLUGINLOG(ZQ::common::Log::L_INFO,CLOGFMT(OSTRMDBViewer,"Successfully retry the last command:%s. cost %lld msec."), sql.c_str(), ZQ::common::now() - t0);
				return true;
			} 
			// report the error message again
		#if  ICE_INT_VERSION / 100 >= 306
			pDBClient->_dbconn.dumpLastError(&DBTrace(_fileLog, ZQ::common::Log::L_ERROR));
		#else
			DBTrace DB(_fileLog, ZQ::common::Log::L_ERROR);
			pDBClient->_dbconn.dumpLastError(&DB);
		#endif	
			PLUGINLOG(ZQ::common::Log::L_WARNING,CLOGFMT(OSTRMDBViewer,"Failed to retry the last command:%s"), sql.c_str());
		}
		else
		{
			PLUGINLOG(ZQ::common::Log::L_WARNING,CLOGFMT(OSTRMDBViewer,"DB reconnect failed"));
		}
	} 
	else 
	{ // just simple execution fault
	}

	return false;
}

