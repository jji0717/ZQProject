#ifndef __TIANSHAN_ODBC_HELPER_H__
#define __TIANSHAN_ODBC_HELPER_H__

#ifndef ZQ_OS_MSWIN
#define SQL_WCHART_CONVERT
#endif

#include <ZQ_common_conf.h>
#include <odbcinst.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <list>

#ifdef ZQ_OS_MSWIN
#include <Windows.h>
#endif

namespace ODBCHelper
{
    typedef std::vector<std::string> StringVector;
    typedef StringVector DBRowData;
    typedef std::list<DBRowData> DBTableData;

    // column's meta-info
    struct ColumnInfo
    {
        std::string name; // column name
        SQLSMALLINT type; // data type
    };
    typedef std::vector<ColumnInfo> ColumnsInfo;

    class Handle
    {
    public:
        SQLSMALLINT type();
        SQLHANDLE value();

        Handle();
        ~Handle();

        bool create(SQLSMALLINT HandleType, SQLHANDLE InputHandle);
        void clear();
        
    private:
        SQLSMALLINT _type;
        SQLHANDLE _value;
    };

    class ITrace
    {
    public:
        virtual void writeln(const std::string&) = 0;
    };

    class MDBViewer
    {
    public:
        MDBViewer();
        ~MDBViewer();
        
        // initialize the db environment
        bool init();

        // connect the db with configured database source
        bool connect(const std::string& dsn, const std::string& user, const std::string& auth);
        // connect the db with a temporary DSN
        bool connect(const std::string& dbPath);
        // disconnect the db
        void disconnect();
        // check if the connection is lost
        bool connectionLost();
        // reconnect the db
        bool reconnect();
        // execute a sql command that no result is expected
        bool exec(const std::string& sql);
        // execute a sql command that the result is optional
        bool exec(const std::string& sql, DBTableData& result, bool expectResult = true);
        // query through a sql command
        bool query(const std::string& sql, DBTableData& result);
        // query with table name and column names
        bool query(const std::string& tbl, const StringVector& cols, DBTableData& result, const std::string& filter="");
        // enumerate the table names in the db
        bool enumTables(StringVector& tbls);


        // enumerate the column names in the table
        bool enumColumns(const std::string& tbl, ColumnsInfo& cols);

        // trace the db error
        void dumpLastError(ITrace* traceObj);

		//add by lxm for compact access
		bool compact(const ::std::string& dbPath);
		bool compact(const std::string& dsn, const std::string& user, const std::string& auth);
		bool createScheme(char* errBuff, unsigned int buffSize, StringVector &scheme); 

    private:
        bool getQueryResult(SQLHANDLE hStmt, DBTableData &result);

        void uninit();

        void traceGeneric(const std::string &msg);
        void traceDBError(SQLSMALLINT HandleType, SQLHANDLE handle);

        bool checkReturn(SQLRETURN sqlRet, SQLSMALLINT HandleType, SQLHANDLE handle, const std::string& action);

		//add by lxm for create database scheme
		void setError(SQLRETURN errRtn, SQLSMALLINT errHdlType, SQLHANDLE errHdl, char* errBuff, unsigned int buffSize);
    private:
        StringVector _err;
        Handle _env;
        Handle _conn;
        bool _connected;
        bool _bConnectionLost; // need reconnect
        struct DBInfo {
            std::string dsn;
            std::string user;
            std::string auth;
        } _dbInfo;
        std::string _tempDSN;
    };
}
#endif