// TSTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma warning(disable:4786)
#include "TSClient.h"
#include "Log.h"
#include "FileLog.h"
#include <windows.h>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <fstream>

using namespace std;



using namespace ZQTianShan::Layout;


class TSLayoutCtx : public ZQTianShan::Layout::ILayoutCtx
{
public:
    /// set the column names of a grid view. it is required before addRow() is called
    ///@param[in] colnames a list of column names in NULL-terminated strings
    ///@parma[in] colcount the count of columns
    ///@return true if succeed. the layout module may reject the invocation if it is after addRow() calls
    virtual bool setColumns(const char* colnames[], const int colcount);
    
    /// add a row to the grid view
    ///@param[in] colvalues the column values, must in the order of the column definition in previous setColumns()
    ///@parma[in] ref       the URI that this row refers to. The layout module may use it to build up the hyper reference. NULL if non-appliable
    ///@return true if succeed. the layout module may reject the invocation if no setColumns() was called or the column count lead access illegal memory range
    virtual bool addRow(const char* colvalues[], const char* ref=NULL);
    
    
    virtual void set(const char*key, const lvalue& val);
    
    /// get a context attribute
    ///@param[in] key the key name of the attribute in NULL-terminated string
    ///@return value the value of the attribute in NULL-terminated string
    virtual bool get(const char* key, lvalue& val);
    
    
    /// list all context attribute names
    ///@param[in] buf buffer allocated in datasource module to receive the attribute names
    ///@param[in] bufSize the size of buf
    ///@param[out] attrnames pointer to the names of attributes, NULL if reaches the end
    ///@return count of the names of the attributes
    virtual int list(char* buf, const int bufsize, char*** pattrnames);	
    
    virtual	void clear(const char* key);
    
    std::string GridToHtmlTable();
    void reset();
private:
    
    std::map<std::string, lvalue>	_snmpVars;
    //////////////////////////////////////////////////////////////////////////
    typedef std::vector<std::string>		ROWDATA;
    typedef struct _TableRow
    {
        ROWDATA			_row;
        std::string		_ref;
    }TableRow;
    typedef	std::vector<TableRow>			TABLEDATA;
    ROWDATA						m_ColumnName;
    TABLEDATA					m_tableData;
};


bool TSLayoutCtx::setColumns(const char* colnames[], const int colcount)
{
    //clear the column name first
    m_ColumnName.clear();
    for(int i=0;i<colcount;i++)
    {
        m_ColumnName.push_back(colnames[i]);
    }
    return true;
}
/// add a row to the grid view
///@param[in] colvalues the column values, must in the order of the column definition in previous setColumns()
///@parma[in] ref       the URI that this row refers to. The layout module may use it to build up the hyper reference. NULL if non-appliable
///@return true if succeed. the layout module may reject the invocation if no setColumns() was called or the column count lead access illegal memory range
bool TSLayoutCtx::addRow(const char* colvalues[], const char* ref)
{
    TableRow	row;
    if( ref && strlen(ref)>0 )
    {
        row._ref	=	ref;
    }
    int	iColCount = (int) m_ColumnName.size();
    for( int i=0 ; i<iColCount ; i++ )
    {
        row._row.push_back(colvalues[i]);
    }
    m_tableData.push_back(row);
    return true;
}

void TSLayoutCtx::set(const char*key, const lvalue& val)
{
    _snmpVars[key] = val;
}

/// get a context attribute
///@param[in] key the key name of the attribute in NULL-terminated string
///@return value the value of the attribute in NULL-terminated string
bool TSLayoutCtx::get(const char* key, lvalue& val)
{
    std::map<std::string, lvalue>::iterator it;
    it = _snmpVars.find(key);
    if (it==_snmpVars.end())
        return false;
    
    val = it->second;
    
    return true;
}


/// list all context attribute names
///@param[in] buf buffer allocated in datasource module to receive the attribute names
///@param[in] bufSize the size of buf
///@param[out] attrnames pointer to the names of attributes, NULL if reaches the end
///@return count of the names of the attributes
int TSLayoutCtx::list(char* buf, const int bufsize, char*** pattrnames)
{
    //we think the buf is enough	
    char** pAttr = (char**)buf;
    int nVarCount = _snmpVars.size();
    char* pPtr = buf + sizeof(char*) * nVarCount;
    
    int nRetCount = 0;
    std::map<std::string, lvalue>::iterator it=_snmpVars.begin();
    while(it!=_snmpVars.end())
    {
        int nStrLen = it->first.length();
        
        if (pPtr - buf + nStrLen + 1 > bufsize)
        {
            //error, buffer size is not enough, need print out error log
            break;
        }
        
        pAttr[nRetCount] = pPtr;
        pPtr += sprintf(pPtr, "%s", it->first.c_str()) + 1;	// also add a 0 terminate
        nRetCount++;
        
        it++;
    }
    *pattrnames = pAttr;
    
    return nRetCount;
}


void TSLayoutCtx::clear(const char* key)
{
    
}
std::string TSLayoutCtx::GridToHtmlTable()
{
    std::ostringstream buf;
    buf << "<table border=1>";

    buf << "<tr>";
    for(size_t iCol = 0; iCol < m_ColumnName.size(); ++iCol)
    {
        buf << "<td>";
        buf << m_ColumnName[iCol];
        buf << "</td>";
    }
    buf << "</tr>";

    for(size_t iRow = 0; iRow < m_tableData.size(); ++iRow)
    {
        buf << "<tr>";
        for(size_t iCol = 0; iCol < m_tableData[iRow]._row.size(); ++iCol)
        {
            buf << "<td>";
            buf << (m_tableData[iRow]._row)[iCol];
            buf << "</td>";
        }
        buf << "</tr>";
    }

    buf << "</table>";
    return buf.str();
}
void TSLayoutCtx::reset()
{
    _snmpVars.clear();
    m_ColumnName.clear();
    m_tableData.clear();
}
static void testFillGrid();
int main(int argc, char* argv[])
{
    testFillGrid();
    return 0;

    if(argc != 6)
    {
        printf("\nusage:  TSTest host(ipaddr) community svcoid(str) testcount interval(ms)\n");
        return -1;
    }
    const char *hostaddr = argv[1];
    const char *community = argv[2];
    const char *svcoid = argv[3];
    size_t nTimes = atoi(argv[4]);
    size_t interval = atoi(argv[5]);
    
    {
        printf("host address    = %s\n", hostaddr);
        printf("community       = %s\n", community);
        printf("start oid     = %s\n", svcoid);
        printf("------------------------------------------------------------\n");
    }
    //////////////////////////////////////////////////////////////////////////
    std::string monitorlogfilename = "d:\\SnmpTest\\log\\monitor\\";
    monitorlogfilename += svcoid;
    monitorlogfilename += ".log";
    ZQ::common::Log *monitor = NULL;
    try{
        monitor = new ZQ::common::FileLog(monitorlogfilename.c_str(), ZQ::common::Log::L_INFO);
    }
    catch(...)
    {
        
    }
    //////////////////////////////////////////////////////////////////////////
    
    std::string logfilename = "d:\\SnmpTest\\log\\TSClient\\";
    logfilename += svcoid;
    logfilename += ".log";
    
    for(size_t i = 0; i < nTimes; ++i)
    {
        DWORD start_time = GetTickCount();
        int nRet = TCInitialize(logfilename.c_str());	
        if (nRet)
        {
            break;
        }
        
        TSLayoutCtx  tctx;
        
        nRet = TCPopulateSnmpVariables(&tctx, svcoid, false, hostaddr, community);
        if (nRet)
        {
            //ok
            //print variables
            //list names
            char pVarNameBuf[SNMPATTR_VARNAMES_MAXLEN] = {0};
            char **pVarName = NULL;
            int len = tctx.list(pVarNameBuf, SNMPATTR_VARNAMES_MAXLEN, &pVarName);
            for(int i = 0; i < len; ++i)
            {
                TSLayoutCtx::lvalue varValue;
                tctx.get(pVarName[i], varValue);
                
                printf("variable %d:\n", i);
                printf("name     = %s\n", pVarName[i]);
                printf("value    = %s\n", varValue.value);
                printf("oid      = %s\n", varValue.oid);
                printf("readonly = %s\n", varValue.readonly ? "True" : "False");
                printf("\n");
                
                varValue.modify = SNMPATTR_VARVALUE_CHANGED;
                tctx.set(pVarName[i], varValue);
            }
            //nRet = TCPopulateSnmpVariables(&tctx, svcoid, true, hostaddr, community);
        }
        else
        {
            //error
        }
        
        TCUninitialize();
        
        DWORD end_time = GetTickCount();
        //log
        if(monitor)
            (*monitor)(ZQ::common::Log::L_INFO, CLOGFMT("TSTest", "[test index = %u], [cost = %dms]"), i, end_time - start_time);
        Sleep(interval);
    }
    
    
    if(monitor)
        delete monitor;
    monitor = NULL;
    return 0;
}

static void testFillGrid()
{
    int nRet = TCInitialize("C:\\test\\log\\TSTest.log");
    if(nRet)
    {
        return;
    }

    TSLayoutCtx tctx;
    ILayoutCtx::lvalue val = {0};
    strcpy(val.value, "ChOD:ChannelPublisher");
    tctx.set("GridName", val);
    strcpy(val.value, "tcp -h 192.168.81.125 -p 33388");
    tctx.set("EndPoint", val);
    int nRows = TCFillGrid(&tctx);
    //dump
    std::ofstream ofile("c:\\test\\tmp\\grid.html");
    ofile << tctx.GridToHtmlTable();

    tctx.reset();
    strcpy(val.value, "ChOD:ChannelPublishPoint");
    tctx.set("GridName", val);
    strcpy(val.value, "tcp -h 192.168.81.125 -p 33388");
    tctx.set("EndPoint", val);
    strcpy(val.value, "36");
    tctx.set("ChannelName", val);

    nRows = TCFillGrid(&tctx);
    ofile << tctx.GridToHtmlTable();
    TCUninitialize();
}
/*
{
do{	
int nRet = TCInitialize("c:\\tsclient.log");	
if (nRet)
{
break;
}

		TSLayoutCtx  tctx;
        
          nRet = TCPopulateSnmpVariables(&tctx, svcoid, false, "127.0.0.1");
          if (nRet)
          {
          //ok
          //print variables
          //list names
          char pVarNameBuf[SNMPATTR_VARNAMES_MAXLEN] = {0};
          char **pVarName = NULL;
          int len = tctx.list(pVarNameBuf, SNMPATTR_VARNAMES_MAXLEN, &pVarName);
          for(int i = 0; i < len; ++i)
          {
          TSLayoutCtx::lvalue varValue;
          
            printf("variable %d:\n", i);
            printf("name     = %s\n", pVarName[i]);
            tctx.get((const char*)pVarName[i], varValue);
            printf("value    = %s\n", varValue.value);
            printf("oid      = %s\n", varValue.oid);
            printf("readonly = %s\n", varValue.readonly ? "True" : "False");
            
              varValue.modify = SNMPATTR_VARVALUE_CHANGED;
              tctx.set(pVarName[i], varValue);
              }
              nRet = TCPopulateSnmpVariables(&tctx, svcoid, true, "127.0.0.1");
              }
              else
              {
              //error
              }
              
                TCUninitialize();
                }
                while(0);
                
                  return 0;
                  }
*/