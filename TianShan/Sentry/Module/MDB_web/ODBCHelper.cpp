#include "ODBCHelper.h"
#include <algorithm>
#include <sstream>
#include <time.h>

namespace ODBCHelper
{

    SQLSMALLINT Handle::type()
    {
        return _type;
    }
    SQLHANDLE Handle::value()
    {
        return _value;
    }

    Handle::Handle():_type(-1), _value(SQL_NULL_HANDLE){}
    Handle::~Handle()
    {
        clear();
    }


    bool Handle::create(SQLSMALLINT HandleType, SQLHANDLE InputHandle)
    {
        clear();
        SQLRETURN sqlRet = ::SQLAllocHandle(HandleType, InputHandle, &_value);

        if(SQL_SUCCESS == sqlRet || SQL_SUCCESS_WITH_INFO == sqlRet)
        {
            _type = HandleType;
            return true;
        }
        else
        {
            clear();
            return false;
        }
    }

    void Handle::clear()
    {
        if(_type != -1)
        {
            ::SQLFreeHandle(_type, _value);
        }
        _type = -1;
        _value = SQL_NULL_HANDLE;
    }


#define SQL_BUF_SIZE    512
    static std::string int2str(int i)
    {
        char buf[22];
        return itoa(i, buf, 10);
    }

    MDBViewer::MDBViewer(): _connected(false), _bConnectionLost(false)
    {
    }
    MDBViewer::~MDBViewer()
    {
        uninit();
    }
    // connect the db with configured database source
    bool MDBViewer::connect(const std::string& dsn, const std::string& user, const std::string& auth)
    {
        // save the db info for reconnecting
        if(&_dbInfo.dsn != &dsn) {
            _dbInfo.dsn = dsn;
            _dbInfo.user = user;
            _dbInfo.auth = auth;
        } // else: int the reconnect, no need to save these value

        // disconnect the db if need
        disconnect();
        SQLRETURN sqlRet = SQLConnect(_conn.value(), (SQLCHAR*)dsn.c_str(), SQL_NTS, (SQLCHAR*)user.c_str(), SQL_NTS, (SQLCHAR*)auth.c_str(), SQL_NTS);

        if(checkReturn(sqlRet, _conn.type(), _conn.value(), std::string("Connect the db with DNS=") + dsn))
        {
            _connected = true;
            return true;
        }
        else
        {
            _connected = false;
            return false;
        }
    }
    // connect the db with a temporary DSN
    bool MDBViewer::connect(const std::string& dbPath)
    {
        if(dbPath.empty())
        {
            _err.clear();
            traceGeneric("Need a valid db path.");
            return false;
        }

        disconnect();

        // generate a temporary name
        std::string tempDSN;
        if(_tempDSN.empty())
        {
            // temp DSN scheme: TS_<thread id>_<time stamp>
            tempDSN.reserve(32);
            tempDSN = "TS_";
#ifdef ZQ_OS_MSWIN
			tempDSN += int2str(::GetCurrentThreadId());
#else
			tempDSN += int2str(getpid());
#endif
            tempDSN += "_";
            tempDSN += int2str(::time(NULL));
        }
        else
        {
            tempDSN = _tempDSN;
        }

        // config the DSN
        const char* driver = "Microsoft Access Driver (*.MDB)";
        std::string attr = "DSN=";
        attr = attr + tempDSN + ";DBQ=" + dbPath;
        BOOL bSetOdbc = SQLConfigDataSource(0, ODBC_ADD_DSN, driver, attr.c_str());
        if (true == bSetOdbc)
        { // save the DSN name
            _tempDSN = tempDSN;
            return connect(_tempDSN, "", "");
        }
        else
        {
            _err.clear();
            std::ostringstream buf;
            buf << "Failed to ADD DSN with DSN ["
                << tempDSN
                << "], Driver ["
                << driver
                << "], Attributes ["
                << attr
                << "].";
            traceGeneric(buf.str());
            return false;
        }
    }

    // disconnect the db
    void MDBViewer::disconnect()
    {
        if(_connected)
        {
            checkReturn(SQLDisconnect(_conn.value()), _conn.type(), _conn.value(), "SQLDisconnect");
        }
        _connected = false;
        _bConnectionLost = false;
    }
    bool MDBViewer::connectionLost() {
        return _bConnectionLost;
    }
    bool MDBViewer::reconnect() {
        return connect(_dbInfo.dsn, _dbInfo.user, _dbInfo.auth);
    }
    // execute a sql command that no result is expected
    bool MDBViewer::exec(const std::string& sql)
    {
        DBTableData result;
        return exec(sql, result, false);
    }
    // execute a sql command and the result is optional
    bool MDBViewer::exec(const std::string& sql, DBTableData& result, bool expectResult)
    {
        result.clear();
        if(!_connected)
        {
            _err.clear();
			_bConnectionLost = true;
            traceGeneric("Not a valid connection.");
            return false;
        }

        Handle stmt;
        if(!stmt.create(SQL_HANDLE_STMT, _conn.value()))
        {
            _err.clear();
            traceGeneric("Failed to create SQLHANDLE of type SQL_HANDLE_STMT.");
            return false;
        }

        SQLRETURN sqlRet = ::SQLExecDirect(stmt.value(), (SQLCHAR*) sql.c_str(), SQL_NTS);
        if(checkReturn(sqlRet, stmt.type(), stmt.value(), std::string("SQLExecDirect ") + sql))
        {
            if(expectResult)
            {
                return getQueryResult(stmt.value(), result);
            }
            else
                return true;
        }
        else
        {
            return false;
        }
    }
    // query through a sql command
    bool MDBViewer::query(const std::string& sql, DBTableData& result)
    {
        return exec(sql, result, true);
    }
    // query with table name and column names
    bool MDBViewer::query(const std::string& tbl, const StringVector& cols, DBTableData& result, const std::string& filter)
    {
        if(tbl.empty())
        {
            _err.clear();
            traceGeneric("Need an explicit table name.");
            return false;
        }

        // prepare the sql command
        std::string sqlCmd = "SELECT ";
        if(!cols.empty())
        {
            for(size_t i = 0; i < cols.size(); ++i)
            {
                if(i != 0)
                    sqlCmd += ',';
                sqlCmd += cols[i];
            }
        }
        else
        {
            sqlCmd += '*';
        }

        sqlCmd += " FROM ";
        sqlCmd += tbl;
        if(!filter.empty())
        {
            sqlCmd += " WHERE ";
            sqlCmd += filter;
        }

        return query(sqlCmd, result);
    }
    // enumerate the table names in the db
    bool MDBViewer::enumTables(StringVector& tbls)
    {
        tbls.clear();

        if(!_connected)
        {
            _err.clear();
            traceGeneric("Not a valid connection.");
            return false;
        }

        Handle stmt;
        if(!stmt.create(SQL_HANDLE_STMT, _conn.value()))
        {
            _err.clear();
            traceGeneric("Failed to create SQLHANDLE of type SQL_HANDLE_STMT.");
            return false;
        }

        SQLRETURN sqlRet = SQLTables(stmt.value(), NULL, 0, NULL, 0, NULL, 0, (SQLCHAR*)"TABLE", SQL_NTS);

        if(!checkReturn(sqlRet, stmt.type(), stmt.value(), "SQLTables"))
        {
            return false;
        }

        DBTableData res;
        if(getQueryResult(stmt.value(), res))
        {
#define COL_IDX_TBLNAME 3 // 1-based

            for(DBTableData::iterator it = res.begin(); it != res.end(); ++it)
            {
                if(it->size() >= COL_IDX_TBLNAME)
                {
                    tbls.push_back(it->at(COL_IDX_TBLNAME - 1));
                }
            }
            return true;
        }
        else
        {
            return false;
        }

    }
    // enumerate the column names in the table
    bool MDBViewer::enumColumns(const std::string& tbl, ColumnsInfo& cols)
    {
        cols.clear();
        if(tbl.empty())
        {
            _err.clear();
            traceGeneric("Need an explicit table name.");
            return false;
        }

        if(!_connected)
        {
            _err.clear();
            traceGeneric("Not a valid connection.");
            return false;
        }

        Handle stmt;
        if(!stmt.create(SQL_HANDLE_STMT, _conn.value()))
        {
            _err.clear();
            traceGeneric("Failed to create SQLHANDLE of type SQL_HANDLE_STMT.");
            return false;
        }

        SQLRETURN sqlRet = SQLColumns(stmt.value(), NULL, 0, NULL, 0, (SQLCHAR*)tbl.c_str(), SQL_NTS, NULL, 0);
        if(!checkReturn(sqlRet, stmt.type(), stmt.value(), std::string("SQLColumns with TBL=") + tbl))
        {
            return false;
        }

        DBTableData res;
        if(getQueryResult(stmt.value(), res))
        {
#define COL_IDX_COLNAME     4 // 1-based
#define COL_IDX_DATATYPE    5 // 1-based
            for(DBTableData::iterator it = res.begin(); it != res.end(); ++it)
            {
                if(it->size() >= COL_IDX_DATATYPE)
                {
                    ColumnInfo colInfo;
                    colInfo.name = it->at(COL_IDX_COLNAME - 1);
                    colInfo.type = atoi(it->at(COL_IDX_DATATYPE - 1).c_str());
                    cols.push_back(colInfo);
                }
            }
            return true;
        }
        else
        {
            return false;
        }

    }

    // trace the db error
    void MDBViewer::dumpLastError(ITrace* traceObj)
    {
        for(size_t i = 0; i < _err.size(); ++i)
        {
            traceObj->writeln(_err[i]);
        }
        _err.clear();
    };

	bool MDBViewer::compact(const ::std::string& dbPath)
	{
		::std::string strSQLConfig;
		char config[512];
		int len = 0;
		strcpy(config + len, "COMPACT_DB=\"");
		len += strlen("COMPACT_DB=\"");

		strcpy(config + len, dbPath.c_str());
		len += dbPath.length();

		strcpy(config + len, "\" \"");
		len += strlen("\" \"");

		memcpy(config + len, "\" General\0\0", 11);
		return SQLConfigDataSource(NULL, ODBC_CONFIG_DSN, (LPSTR)"Microsoft Access Driver (*.mdb)", config);

		//strSQLConfig = ::std::string("COMPACT_DB=\"") + dbPath + "\" \"" + dbPath + "\" General\0\0";
		//return SQLConfigDataSource(NULL, ODBC_CONFIG_DSN, (LPSTR)"Microsoft Access Driver (*.mdb)", strSQLConfig.c_str());
	}

	bool MDBViewer::compact(const std::string& dsn, const std::string& user, const std::string& auth)
	{
		return true;
	}

	bool MDBViewer::createScheme(char* errBuff, unsigned int buffSize, StringVector &scheme)
	{
		errBuff[buffSize - 1] = '\0';
		SQLHANDLE hStmt = SQL_NULL_HANDLE;
		SQLRETURN sqlRet = ::SQLAllocHandle(SQL_HANDLE_STMT, _conn.value(), &hStmt);
		if (sqlRet != SQL_SUCCESS && sqlRet != SQL_SUCCESS_WITH_INFO)
		{
			snprintf(errBuff, buffSize - 1, "SQLAllocHandle SQL_HANDLE_STMT failed");
			setError(sqlRet, SQL_HANDLE_DBC, _conn.value(), errBuff, buffSize);
			return false;
		}

		// do: create database scheme
		for (int i = 0, count = scheme.size(); i < count; i ++)
		{
			sqlRet = ::SQLExecDirect(hStmt, (SQLCHAR*) scheme[i].c_str(), SQL_NTS);
			if (sqlRet != SQL_SUCCESS && sqlRet != SQL_SUCCESS_WITH_INFO)
			{
				snprintf(errBuff, buffSize - 1, "SQLExecDirect %s failed", scheme[i].c_str());
				setError(sqlRet, SQL_HANDLE_STMT, hStmt, errBuff, buffSize);
				::SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
				return false;
			}
		}

		::SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		return true;
	}

    // get the query result
    bool MDBViewer::getQueryResult(SQLHANDLE hStmt, DBTableData &result)
    {
        result.clear();

        SQLSMALLINT nCols = -1;
        char buf[SQL_BUF_SIZE];
        SQLLEN len;
        SQLRETURN sqlRet = SQLNumResultCols(hStmt, &nCols);
        if(!checkReturn(sqlRet, SQL_HANDLE_STMT, hStmt, "SQLNumResultCols"))
        {
            return false;
        }

        while(true)
        {
            sqlRet = SQLFetch(hStmt);
            if(SQL_NO_DATA == sqlRet)
            { // success
                break;
            }

            if(!checkReturn(sqlRet, SQL_HANDLE_STMT, hStmt, "SQLFetch")) // not the SQL_NO_DATA
            {
                return false;
            }

            DBRowData row;
            row.reserve(nCols);

            for(SQLSMALLINT i = 1; i <= nCols; ++i)
            {
                sqlRet = SQLGetData(hStmt, i, SQL_C_CHAR, buf, SQL_BUF_SIZE, &len);
                if(!checkReturn(sqlRet, SQL_HANDLE_STMT, hStmt, std::string("SQLGetData with COL=") + int2str(i)))
                {
                    return false;
                }
                if(SQL_NULL_DATA == len)
                {
                    row.push_back("");
                }
                else
                {
                    row.push_back(buf);
                }
            }
            result.push_back(row);
        }
        return true;
    }

    bool MDBViewer::init()
    {
        uninit();
        if(_env.create(SQL_HANDLE_ENV, SQL_NULL_HANDLE))
        {
            SQLRETURN sqlRet =SQLSetEnvAttr(_env.value(), SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
            if(!checkReturn(sqlRet, _env.type(), _env.value(), "enable the SQL_OV_ODBC3 mode"))
            {
                _env.clear();
                return false;
            }

            if(_conn.create(SQL_HANDLE_DBC, _env.value()))
            {

                sqlRet = SQLSetConnectAttr(_conn.value(), SQL_LOGIN_TIMEOUT, (SQLPOINTER) 5, 0);
                if(checkReturn(sqlRet, _conn.type(), _conn.value(), "SQLSetConnectAttr with LOGIN_TIMEOUT=5"))
                {
                    return true;
                }
                else
                {
                    _conn.clear();
                    _env.clear();
                    return false;
                }
            }
            else
            {
                _err.clear();
                traceGeneric("Failed to create SQLHANDLE of type SQL_HANDLE_DBC.");
                traceDBError(_env.type(), _env.value());

                _env.clear();
                return false;
            }
        }
        else
        {
            _err.clear();
            traceGeneric("Failed to create SQLHANDLE of type SQL_HANDLE_ENV.");
            return false;
        }
    }

    void MDBViewer::uninit()
    {
        disconnect();
        if(!_tempDSN.empty())
        {
            const char* driver = "Microsoft Access Driver (*.MDB)";
            std::string attr = "DSN=";
            attr += _tempDSN;
            SQLConfigDataSource(0, ODBC_REMOVE_DSN, driver, attr.c_str());
            _tempDSN.clear();
        }
        _conn.clear();
        _env.clear();
    }

    void MDBViewer::traceGeneric(const std::string &msg)
    {
        _err.push_back(msg);
    }

    void MDBViewer::traceDBError(SQLSMALLINT HandleType, SQLHANDLE handle)
    {
        SQLCHAR sqlState[SQL_BUF_SIZE], messageText[SQL_BUF_SIZE];
        SQLINTEGER nativeError = 0;
        SQLSMALLINT textLengh;
        SQLSMALLINT i = 1;
        SQLRETURN sqlRet;
        while((sqlRet = SQLGetDiagRec(HandleType, handle, i, sqlState, &nativeError
            , messageText, sizeof(messageText) / sizeof(SQLCHAR), &textLengh)) != SQL_NO_DATA)
        {
            if(!_bConnectionLost) {
                _bConnectionLost = strcmp((const char*)sqlState, "08S01") == 0 || strcmp((const char*)sqlState, "") == 0 || strcmp((const char*)sqlState, "01000") == 0;
            }
            std::ostringstream buf;
            buf << sqlState
                << " | "
                << nativeError
                << " | "
                << messageText;
            traceGeneric(buf.str());
            ++i;
        }
    }

    bool MDBViewer::checkReturn(SQLRETURN sqlRet, SQLSMALLINT HandleType, SQLHANDLE handle, const std::string& action)
    {
        switch(sqlRet)
        {
        case SQL_SUCCESS:
            return true;

        case SQL_SUCCESS_WITH_INFO:
            _err.clear();
            // the messages are provided here, but not inform anybody.
            traceGeneric(std::string("Success with info during [") + action + "]");
            traceDBError(HandleType, handle);
            return true;

        case SQL_ERROR:
            _err.clear();
            traceGeneric(std::string("Fail to [") + action + "]");
            traceDBError(HandleType, handle);
            return false;

        default:
            _err.clear();
            traceGeneric(std::string("Failed to [") + action + "]");
            return false;
        }
    }

	void MDBViewer::setError(SQLRETURN errRtn, SQLSMALLINT errHdlType, SQLHANDLE errHdl, char* errBuff, unsigned int buffSize)
	{
		if (errRtn != SQL_ERROR && errRtn != SQL_SUCCESS_WITH_INFO)
			return;
		SQLCHAR sqlState[MAX_PATH], messageTest[MAX_PATH];
		memset(sqlState, 0, sizeof(sqlState));
		memset(messageTest, 0, sizeof(messageTest));
		SQLINTEGER nativeError;
		SQLSMALLINT textLenght;
		SQLGetDiagRec(errHdlType, errHdl, 1, sqlState, &nativeError, messageTest, sizeof(messageTest) / sizeof(SQLCHAR), &textLenght);
		errBuff[buffSize - 1] = '\0';
		snprintf(errBuff, buffSize - 1, "%s: %s", (char*) sqlState, (char*) messageTest);
	}
}
