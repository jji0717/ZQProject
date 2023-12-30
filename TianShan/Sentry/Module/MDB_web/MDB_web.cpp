// MDB_web.cpp : Defines the entry point for the DLL application.
//

#include <log.h>
#include <httpdInterface.h>
#include <strHelper.h>
#include "ODBCHelper.h"
#include <boost/algorithm/string.hpp>
#include <sstream>

BOOL APIENTRY DllMain( HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

class GLogTrace : public ODBCHelper::ITrace
{
public:
    explicit GLogTrace(ZQ::common::Log::loglevel_t lvl)
        :_lvl(lvl)
    {   
    }
    virtual void writeln(const std::string& msg)
    {
        glog(_lvl, CLOGFMT(MDB_web,"%s"), msg.c_str());
    }
private:
    ZQ::common::Log::loglevel_t _lvl;
};

template<class CollT>
std::string join(const CollT& coll, const std::string& delimiter)
{
    std::string retval;
    for(CollT::const_iterator it = coll.begin(); it != coll.end(); ++it)
    {
        if(it != coll.begin())
            retval += delimiter;
        retval += (*it);
    }
    return retval;
}

#define IsValidCStr(s) (s && '\0' != *s)
struct DBSchema
{
    const char* webEntry; // entry thru HTTP
    const char* table; // table name in db
    const char* pKey; // primary key of the table
    const char* fields; // the database fields. format: field[:displayname][other fields seperated by ',']
    char* sortable; // the fields that can be sorted
    const char* searchable; // the fields that can be searched
    typedef std::vector< std::pair<std::string, std::string> > Fields;
};

// fields parsing helper
static DBSchema::Fields dbFields(const DBSchema* sch);
static const char* displayName(const DBSchema::Fields& fields, const char* fieldName);

////////////////////////
// parameters:
//      db path
//      rows/page, nthpage
//      searchfor, searchvalue, sortby
static bool dbTablePage(IHttpRequestCtx *pHttpRequestCtx, const DBSchema *pSchema)
{
    if(NULL == pHttpRequestCtx)
        return false;

    // step 1: check the parameters
    const char* dbPath = pHttpRequestCtx->GetRequestVar("path");
    if(NULL == dbPath || '\0' == (*dbPath))
    {
        pHttpRequestCtx->Response().SetLastError("Require the mdb file path.");
        return false;
    }
    const char* tblName = pSchema->table;

    ODBCHelper::MDBViewer mdbv;
    if(!mdbv.init())
    {
        mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
        pHttpRequestCtx->Response().SetLastError("Failed to setup the db environment.");
        return false;
    }
    if(!mdbv.connect(dbPath))
    {
        mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
        pHttpRequestCtx->Response().SetLastError((std::string("Failed to connect db [") + dbPath + "].").c_str());
        return false;
    }

    ODBCHelper::ColumnsInfo colAttrs;
    { // enumerate the columns attributes
        if(!mdbv.enumColumns(tblName, colAttrs))
        {
            mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
            pHttpRequestCtx->Response().SetLastError((std::string("Failed to enumerate the column attributes of table [") + tblName + "].").c_str());
            return false; 
        }
    }
#pragma message(__MSGLOC__"TODO: check the parameters")
    // sortby : default not sort
    const char* sortby = pHttpRequestCtx->GetRequestVar("sortby");
    if(NULL == sortby)
        sortby = "";

    bool nosorting = ('\0' == *sortby);
    bool reverseShowing = nosorting;

    // rows per page : default 20
    const char* count = pHttpRequestCtx->GetRequestVar("count");
    int nCount = count ? atoi(count) : 0;
    if(nCount < 20)
        nCount = 20;
    if(nCount > 200)
        nCount = 200;

    const char* nth = pHttpRequestCtx->GetRequestVar("nth");
    int nthPage = nth ? atoi(nth) : 0;
    if(nthPage < 1)
        nthPage = 1;

    // search function
    const char* searchfor = pHttpRequestCtx->GetRequestVar("searchfor"); // the field search for
    const char* searchval = pHttpRequestCtx->GetRequestVar("searchval"); // the target value
    const char* comp = "="; // the compare method

    DBSchema::Fields fields = dbFields(pSchema);

    std::string searchFilter;
    if(IsValidCStr(searchfor) && IsValidCStr(searchval) && displayName(fields, searchfor))
    {
        for(size_t i = 0; i < colAttrs.size(); ++i)
            if(colAttrs[i].name == searchfor)
            {
#define Is_CHAR_Type(t) (t == SQL_CHAR || t == SQL_VARCHAR)
                if(Is_CHAR_Type(colAttrs[i].type))
                    searchFilter = std::string(" WHERE ") + searchfor + comp + "'" + searchval + "'";
                else
                    searchFilter = std::string(" WHERE ") + searchfor + comp + searchval;
                break;
            }
    }

    // step 2: query the db
    ODBCHelper::DBTableData data;
    std::ostringstream sqlCmdBuf;
    // get total rows
    sqlCmdBuf << "SELECT count(" << pSchema->pKey << ") FROM " << pSchema->table << searchFilter;
    if(!mdbv.query(sqlCmdBuf.str(), data))
    {
        mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
        pHttpRequestCtx->Response().SetLastError((std::string("Failed to query data from db [") + dbPath + "].").c_str());
        return false; 
    }
    int total = 0; // the total record count
    if(data.size() == 1 && data.begin()->size() == 1)
    {
        total = atoi(data.begin()->begin()->c_str());
    }

    // paging

    int nTotalPage = 0;
    int nCurPage = 0;
    int nCurPageFrom = 0;
    int nCurPageCount = 0;
    if(total != 0)
    {
        int nLastPageCount = total % nCount;
        nTotalPage = total / nCount + 1;

        if(nLastPageCount == 0)
        {
            nTotalPage -= 1;
            nLastPageCount = nCount;
        }
        if(nTotalPage == 0)
        { // no data
        }
        else if(nTotalPage == 1)
        { // no paging
            nCurPage = 1;
            nCurPageCount = nLastPageCount;
        }
        else // at least 2 pages
        {
            if(nthPage >= nTotalPage)
            { // last page
                nCurPage = nTotalPage;
                nCurPageCount = nLastPageCount;
            }
            else
            {
                nCurPage = nthPage;
                nCurPageCount = nCount;
            }

            if(!reverseShowing)
            { // 
                nCurPageFrom = (nCurPage - 1) * nCount;
            }
            else // reverse showing
            {
                nCurPageFrom = total - ((nCurPage - 1) * nCount + nCurPageCount);
            }
        } // end else
    }
    // else the default

    std::string ordering;
    if(!nosorting)
    {
        ordering += " ORDER BY ";
        ordering += sortby;
    }

    // query
    data.clear();
    sqlCmdBuf.str("");

    if(nTotalPage != 0)
    { // query the data
        std::string queryFields;
        for(DBSchema::Fields::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
            if(it != fields.begin())
                queryFields += ",";
            queryFields += it->first;
        }

        sqlCmdBuf
            << "SELECT TOP " << nCurPageCount << " " << queryFields << " FROM " << pSchema->table;
        if(0 == nCurPageFrom)
        {
            sqlCmdBuf << searchFilter;
        }
        else
        {
            if(searchFilter.empty())
            {
                sqlCmdBuf << " WHERE ";
            }
            else
            {
                sqlCmdBuf << searchFilter << " AND ";
            }
            sqlCmdBuf
                << pSchema->pKey << " NOT IN "
                << "(SELECT TOP " << nCurPageFrom << " " << pSchema->pKey  << " FROM " << pSchema->table << searchFilter << ordering << ")";
        }
        sqlCmdBuf << ordering;

        std::string sqlCmd = sqlCmdBuf.str();
        glog(ZQ::common::Log::L_DEBUG, CLOGFMT(MDB_web, "Excute SQL command: %s"), sqlCmd.c_str());
        if(!mdbv.query(sqlCmd, data))
        {
            mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
            pHttpRequestCtx->Response().SetLastError((std::string("Failed to query data from db [") + dbPath + "].").c_str());
            return false; 
        }
    }
    else // nTotalPage == 0
    { // no data
    }

    // step 3: send back the result
    IHttpResponse &out = pHttpRequestCtx->Response();
    // the query bar
    out << "<form name='queryform' method='post' action='" << pHttpRequestCtx->GetRootURL() << pSchema->webEntry << "'>"
        << "Search for:<br>\n"
        << "<select name='searchfor'>\n";
    {
        ZQ::common::stringHelper::STRINGVECTOR searchables;
        ZQ::common::stringHelper::SplitString(pSchema->searchable, searchables, ",", ", ");
        for(size_t i = 0; i < searchables.size(); ++i)
        {
            out << "<option value='" << searchables[i] << "' >" << displayName(fields, searchables[i].c_str()) << "</option>\n";
        }
    }
    out << "</select><span style='margin-left:10'></span>"
        << "<input type='text' name='searchval'><input type='submit' value='Search'>\n";

    out << "<input type='hidden' name='#template' value='" << pHttpRequestCtx->GetRequestVar("#template") << "'>\n";
    out << "<input type='hidden' name='path' value='" << dbPath << "'>\n";
    out << "<input type='hidden' name='count' value='" << nCount << "'>\n";
    out << "</form>";
    out << "<hr>\n";

    // the page info

    // navigation pannel
    out << "<form name='navform' method='post' action='" << pHttpRequestCtx->GetRootURL() << pSchema->webEntry << "'>";
    out << "<input type='hidden' name='#template' value='" << pHttpRequestCtx->GetRequestVar("#template") << "'>\n";
    out << "<input type='hidden' name='path' value='" << dbPath << "'>\n";
    out << "<input type='hidden' name='count' value='" << nCount << "'>\n";
    out << "<input id='nav-sortby' type='hidden' name='sortby' value='" << sortby << "'>\n";
    out << "<input type='hidden' name='searchfor' value='" << searchfor << "'>\n";
    out << "<input type='hidden' name='searchval' value='" << searchval << "'>\n";
    out << "<input id='nav-nth' type='hidden' name='nth' value='" << nthPage << "'>\n";
    out << "</form>";

    out << "<script type='text/javascript'>\n";
    // resort in the currrent search set
    out << "  function resortby(key){\n"
        << "    document.getElementById('nav-sortby').value = key;\n"
        << "    document.getElementById('nav-nth').value = '1';\n"
        << "    document.getElementById('navform').submit();\n"
        << "  }\n";
    out << "</script>\n";

    out << "<span>";
    // previous page
    if(nCurPage > 1)
        out << "<a href='' onclick=\"document.getElementById('nav-nth').value = '"
            << (nCurPage - 1) << "';"
            << "document.getElementById('navform').submit(); return false\">previous</a>";
    else // this is the first page
        out << "previous";
    out << "<span style='padding:0px 10px;text-align:center'>[" << nCurPage << "/" << nTotalPage << "]</span>";
    // next page
    if(nCurPage != nTotalPage)
        out << "<a href='' onclick=\"document.getElementById('nav-nth').value = '"
            << (nCurPage + 1) << "';"
            << "document.getElementById('navform').submit();return false\">next</a>";
    else // this is the last page
        out << "next";
    out << "</span>\n";

    // data
    out << "<table class='listTable'>\n";
    {
        out << "<tr>";
        ZQ::common::stringHelper::STRINGVECTOR sortables;
        ZQ::common::stringHelper::SplitString(pSchema->sortable, sortables, ",", ", ");
        for(DBSchema::Fields::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
            out << "<th>";
            bool bSortByThis = it->first == sortby;
            bool isThisSortable = std::find(sortables.begin(), sortables.end(), it->first) != sortables.end();
            if(isThisSortable)
                out << (bSortByThis ? "&gt;" : "")
                    << "<a title='sort by " << it->second << "' href='' onclick=\"resortby('" << it->first << "'); return false\">" << it->second << "<small>&#9660;</small></a>"
                    << (bSortByThis ? "&lt;" : "");
            else
                out << it->second;
            out << "</th>\n";
        }
        out << "</tr>\n";
    }
    // print the data
    if(reverseShowing)
        std::reverse(data.begin(), data.end());

    for(ODBCHelper::DBTableData::const_iterator it_row = data.begin(); it_row != data.end(); ++it_row)
    {
        ODBCHelper::DBRowData::const_iterator it_item = it_row->begin();
        out << "<tr>";
        for(; it_item != it_row->end(); ++it_item)
        {
            out << "<td>" << boost::trim_copy(*it_item) << "</td>";
        }

        out << "</tr>\n";
    }
    out << "</table>\n";
    return true;
}

//For DSN (mysql only)
static bool dbTablePageDSN(IHttpRequestCtx *pHttpRequestCtx, const DBSchema *pSchema)
{
    if(NULL == pHttpRequestCtx)
        return false;

    // step 1: check the parameters
    const char* dbPath = pHttpRequestCtx->GetRequestVar("dsn");
    if(NULL == dbPath || '\0' == (*dbPath))
    {
        pHttpRequestCtx->Response().SetLastError("Require the dsn.");
        return false;
    }
    const char* tblName = pSchema->table;

    ODBCHelper::MDBViewer mdbv;
    if(!mdbv.init())
    {
        mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
        pHttpRequestCtx->Response().SetLastError("Failed to setup the db environment.");
        return false;
    }
    ::std::string strUser;
    ::std::string strPasswd;
    if(!mdbv.connect(dbPath, strUser, strPasswd))
    {
        mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
        pHttpRequestCtx->Response().SetLastError((std::string("Failed to connect db [") + dbPath + "].").c_str());
        return false;
    }

    ODBCHelper::ColumnsInfo colAttrs;
    { // enumerate the columns attributes
        if(!mdbv.enumColumns(tblName, colAttrs))
        {
            mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
            pHttpRequestCtx->Response().SetLastError((std::string("Failed to enumerate the column attributes of table [") + tblName + "].").c_str());
            return false; 
        }
    }
#pragma message(__MSGLOC__"TODO: check the parameters")
    // sortby : default not sort
    const char* sortby = pHttpRequestCtx->GetRequestVar("sortby");
    if(NULL == sortby)
        sortby = "";

    bool nosorting = ('\0' == *sortby);
    bool reverseShowing = nosorting;

    // rows per page : default 20
    const char* count = pHttpRequestCtx->GetRequestVar("count");
    int nCount = count ? atoi(count) : 0;
    if(nCount < 20)
        nCount = 20;
    if(nCount > 200)
        nCount = 200;

    const char* nth = pHttpRequestCtx->GetRequestVar("nth");
    int nthPage = nth ? atoi(nth) : 0;
    if(nthPage < 1)
        nthPage = 1;

    // search function
    const char* searchfor = pHttpRequestCtx->GetRequestVar("searchfor"); // the field search for
    const char* searchval = pHttpRequestCtx->GetRequestVar("searchval"); // the target value
    const char* comp = "="; // the compare method

    DBSchema::Fields fields = dbFields(pSchema);
    std::string searchFilter;
    if(IsValidCStr(searchfor) && IsValidCStr(searchval) && displayName(fields, searchfor))
    {
        searchFilter = std::string(" WHERE ") + searchfor + comp + "'" + searchval + "'";
    }

    // step 2: query the db
    ODBCHelper::DBTableData data;
    std::ostringstream sqlCmdBuf;
    // get total rows
    sqlCmdBuf << "SELECT count(" << pSchema->pKey << ") FROM " << pSchema->table << searchFilter;
    if(!mdbv.query(sqlCmdBuf.str(), data))
    {
        mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
        pHttpRequestCtx->Response().SetLastError((std::string("Failed to query data from db [") + dbPath + "].").c_str());
        return false; 
    }
    int total = 0; // the total record count
    if(data.size() == 1 && data.begin()->size() == 1)
    {
        total = atoi(data.begin()->begin()->c_str());
    }

    // paging

    int nTotalPage = 0;
    int nCurPage = 0;
    int nCurPageFrom = 0;
    int nCurPageCount = 0;
    if(total != 0)
    {
        int nLastPageCount = total % nCount;
        nTotalPage = total / nCount + 1;

        if(nLastPageCount == 0)
        {
            nTotalPage -= 1;
            nLastPageCount = nCount;
        }
        if(nTotalPage == 0)
        { // no data
        }
        else if(nTotalPage == 1)
        { // no paging
            nCurPage = 1;
            nCurPageCount = nLastPageCount;
        }
        else // at least 2 pages
        {
            if(nthPage >= nTotalPage)
            { // last page
                nCurPage = nTotalPage;
                nCurPageCount = nLastPageCount;
            }
            else
            {
                nCurPage = nthPage;
                nCurPageCount = nCount;
            }

            if(!reverseShowing)
            { // 
                nCurPageFrom = (nCurPage - 1) * nCount;
            }
            else // reverse showing
            {
                nCurPageFrom = total - ((nCurPage - 1) * nCount + nCurPageCount);
            }
        } // end else
    }
    // else the default

    std::string ordering;
    if(!nosorting)
    {
        ordering += " ORDER BY ";
        ordering += sortby;
    }
    // query
    data.clear();
    sqlCmdBuf.str("");

    if(nTotalPage != 0)
    { // query the data
        std::string queryFields;
        for(DBSchema::Fields::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
            if(it != fields.begin())
                queryFields += ",";
            queryFields += it->first;
        }

        sqlCmdBuf
            << "SELECT " << queryFields << " FROM"
            << " " << pSchema->table << searchFilter << ordering
            << " limit " << nCurPageFrom << "," << nCurPageCount;

        std::string sqlCmd = sqlCmdBuf.str();
        glog(ZQ::common::Log::L_DEBUG, CLOGFMT(MDB_web, "Excute SQL command: %s"), sqlCmd.c_str());
        if(!mdbv.query(sqlCmd, data))
        {
            mdbv.dumpLastError(&GLogTrace(ZQ::common::Log::L_ERROR));
            pHttpRequestCtx->Response().SetLastError((std::string("Failed to query data from db [") + dbPath + "].").c_str());
            return false; 
        }
    }
    else // nTotalPage == 0
    { // no data
    }

    // step 3: send back the result
    IHttpResponse &out = pHttpRequestCtx->Response();
    // the query bar
    out << "<form name='queryform' method='post' action='" << pHttpRequestCtx->GetRootURL() << pSchema->webEntry << "'>"
        << "Search for:<br>\n"
        << "<select name='searchfor'>\n";
    {
        ZQ::common::stringHelper::STRINGVECTOR searchables;
        ZQ::common::stringHelper::SplitString(pSchema->searchable, searchables, ",", ", ");
        for(size_t i = 0; i < searchables.size(); ++i)
        {
            out << "<option value='" << searchables[i] << "' >" << displayName(fields, searchables[i].c_str()) << "</option>\n";
        }
    }
    out << "</select><span style='margin-left:10'></span>"
        << "<input type='text' name='searchval'><input type='submit' value='Search'>\n";

    out << "<input type='hidden' name='#template' value='" << pHttpRequestCtx->GetRequestVar("#template") << "'>\n";
    out << "<input type='hidden' name='dsn' value='" << dbPath << "'>\n";
    out << "<input type='hidden' name='count' value='" << nCount << "'>\n";
    out << "</form>";
    out << "<hr>\n";

    // the page info

    // navigation pannel
    out << "<form name='navform' method='post' action='" << pHttpRequestCtx->GetRootURL() << pSchema->webEntry << "'>";
    out << "<input type='hidden' name='#template' value='" << pHttpRequestCtx->GetRequestVar("#template") << "'>\n";
    out << "<input type='hidden' name='dsn' value='" << dbPath << "'>\n";
    out << "<input type='hidden' name='count' value='" << nCount << "'>\n";
    out << "<input id='nav-sortby' type='hidden' name='sortby' value='" << sortby << "'>\n";
    out << "<input type='hidden' name='searchfor' value='" << searchfor << "'>\n";
    out << "<input type='hidden' name='searchval' value='" << searchval << "'>\n";
    out << "<input id='nav-nth' type='hidden' name='nth' value='" << nthPage << "'>\n";
    out << "</form>";

    out << "<script type='text/javascript'>\n";
    // resort in the currrent search set
    out << "  function resortby(key){\n"
        << "    document.getElementById('nav-sortby').value = key;\n"
        << "    document.getElementById('nav-nth').value = '1';\n"
        << "    document.getElementById('navform').submit();\n"
        << "  }\n";
    out << "</script>\n";

    out << "<span>";
    // previous page
    if(nCurPage > 1)
        out << "<a href='' onclick=\"document.getElementById('nav-nth').value = '"
            << (nCurPage - 1) << "';"
            << "document.getElementById('navform').submit(); return false\">previous</a>";
    else // this is the first page
        out << "previous";
    out << "<span style='padding:0px 10px;text-align:center'>[" << nCurPage << "/" << nTotalPage << "]</span>";
    // next page
    if(nCurPage != nTotalPage)
        out << "<a href='' onclick=\"document.getElementById('nav-nth').value = '"
            << (nCurPage + 1) << "';"
            << "document.getElementById('navform').submit();return false\">next</a>";
    else // this is the last page
        out << "next";
    out << "</span>\n";

    // data
    out << "<table class='listTable'>\n";
    {
        out << "<tr>";
        ZQ::common::stringHelper::STRINGVECTOR sortables;
        ZQ::common::stringHelper::SplitString(pSchema->sortable, sortables, ",", ", ");
        for(DBSchema::Fields::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
            out << "<th>";
            bool bSortByThis = it->first == sortby;
            bool isThisSortable = std::find(sortables.begin(), sortables.end(), it->first) != sortables.end();
            if(isThisSortable)
                out << (bSortByThis ? "&gt;" : "")
                    << "<a title='sort by " << it->second << "' href='' onclick=\"resortby('" << it->first << "'); return false\">" << it->second << "<small>&#9660;</small></a>"
                    << (bSortByThis ? "&lt;" : "");
            else
                out << it->second;
            out << "</th>\n";
        }
        out << "</tr>\n";
    }
    // print the data
    if(reverseShowing)
        std::reverse(data.begin(), data.end());

    for(ODBCHelper::DBTableData::const_iterator it_row = data.begin(); it_row != data.end(); ++it_row)
    {
        ODBCHelper::DBRowData::const_iterator it_item = it_row->begin();
        out << "<tr>";
        for(; it_item != it_row->end(); ++it_item)
        {
            out << "<td>" << boost::trim_copy(*it_item) << "</td>";
        }

        out << "</tr>\n";
    }
    out << "</table>\n";
    return true;
}

extern "C"
{
	__declspec(dllexport) bool LibInit(ZQ::common::Log* pLog)
	{
		if(NULL == pLog)
			return false;

		ZQ::common::setGlogger(pLog);

		return true;
	}

	__declspec(dllexport) void LibUninit( )
	{
		ZQ::common::setGlogger();
	}

#pragma message(__MSGLOC__"Should we use the DSN directly?")
    __declspec(dllexport) bool SessionPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        static DBSchema sch = {
            "SessionPage.mdb.tswl",
            "Sessions",
            "Session",
            "Session,Site,Path,Storage,Streamer,ServiceGroup,Bandwidth,Stream",
            "Session,Stream",
            "Session,Site,Path,Storage,Streamer,Stream"
        };
        return dbTablePage(pHttpRequestCtx, &sch);
    }

    __declspec(dllexport) bool NgodSessionPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        static DBSchema sch = {
            "NgodSessionPage.mdb.tswl",
            "LiveSession",
            "RTSPSession",
            "RTSPSession:RTSPSessionId,OnDemandSessionId,ClientSession:ClientSessionId,PAID,SOP,Streamer,Bandwidth,Destination_Port:Destination/Port,SETUP,InServiceAt:RequestTime,OutOfServiceAt:CompletionTime,TEARDOWN,LastError",
            "RTSPSession,OnDemandSessionId,ClientSession",
            "RTSPSession,OnDemandSessionId,ClientSession,PAID,Streamer"
        };
        return dbTablePage(pHttpRequestCtx, &sch);
    }

    __declspec(dllexport) bool NgodSessionPageDSN(IHttpRequestCtx *pHttpRequestCtx)
    {
        static DBSchema sch = {
            "NgodSessionPageDSN.mdb.tswl",
            "LiveSession",
            "RTSPSession",
            "RTSPSession:RTSPSessionId,OnDemandSessionId,ClientSession:ClientSessionId,PAID,SOP,Streamer,Bandwidth,Destination_Port:Destination/Port,SETUP,InServiceAt:RequestTime,OutOfServiceAt:CompletionTime,TEARDOWN,LastError",
            "RTSPSession,OnDemandSessionId,ClientSession",
            "RTSPSession,OnDemandSessionId,ClientSession,PAID,Streamer"
        };
        return dbTablePageDSN(pHttpRequestCtx, &sch);
    }
}

/// db fields parsing helper
static DBSchema::Fields dbFields(const DBSchema* sch)
{
    using namespace ZQ::common::stringHelper;
    DBSchema::Fields result;
    if(sch)
    {
        STRINGVECTOR fields;
        ZQ::common::stringHelper::SplitString(sch->fields, fields, ",");

        for(STRINGVECTOR::const_iterator it = fields.begin(); it != fields.end(); ++it)
        {
            std::string fieldName, displayName;
            std::string::size_type colonPos = it->find(':');
            if(colonPos != std::string::npos)
            {
                fieldName = it->substr(0, colonPos);
                displayName=  it->substr(colonPos + 1);
            }
            else
            {
                fieldName = (*it);
            }

            if(displayName.empty())
            {
                displayName = fieldName;
            }

            result.push_back(std::make_pair(fieldName, displayName));
        } // end for
    } // end if
    return result;
}

static const char* displayName(const DBSchema::Fields& fields, const char* fieldName)
{
    if(fieldName)
    {
        DBSchema::Fields::const_iterator it;
        for(it = fields.begin(); it != fields.end(); ++it)
            if(it->first == fieldName)
                return it->second.c_str();
    }
    return NULL;
}
