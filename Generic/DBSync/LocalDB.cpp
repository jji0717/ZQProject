/*****************************************************************************
File Name:		LocalDB.cpp
Author:			Interactive ZQ 
Security:		<NVOD over iTV, Kunming>, Confidential
Description:	Implementation of Class LocalDB, which is used for operation on 
                local oracle database via ODBC
Function Inventory: 
				LocalDB
				~LocalDB
				OpenDB
				CloseDB
				ExecuteSQL
				SelectSQL
				CallStoreProcedure
Modification Log:
When		Version		Who			What
-----------------------------------------------------------------------------
11/21/2002	    0		W.Z.	Original Program (Created)
01/30/2003		1		W.Z.	Modify to utilize transaction mechanism
11/23/2006      2       KenQ    Adding param type for CallStoreProcedure() 
                                to specify each parameter type: IN/OUT/INOUT
*****************************************************************************/


//////////////////////////////////////////////////////////////////////
// Declaratoin of include files and definition of symbols
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sclog.h"
#include "LocalDB.h"
#include "DBSyncServ.h"
#include "DSInterface.h"
#include <strsafe.h>
extern DWORD g_SQLDeadlockRetryCount;
#define MAXNAME 256
#define ODBC_SCAN_TIMEOUT	3000


using ZQ::common::Log;
extern CDSInterface g_ds;
//////////////////////////////////////////////////////////////////////
// Declaration of Global Variables and Functions
//////////////////////////////////////////////////////////////////////

extern TCHAR g_IZQDSNName[MAXNAME];		//	Global string of data source string of ODBC connection with Local DB
extern TCHAR g_IZQUserName[MAXNAME];	//	Global string of user name to open ODBC connection with Local DB
extern TCHAR g_IZQPassword[MAXNAME];	//	Global string of password to open ODBC connection with Local DB
extern CFile g_file;	//	Global handle of trace file

extern ZQ::common::Log* gpDbSyncLog;
extern DBSyncServ server;

extern DWORD g_TraceTransactionCount;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//SQLHENV			LocalDB::m_hEnv;
//SQLHDBC			LocalDB::m_hDbc;

/*****************************************************************************
Function Name:	LocalDB
Arguments:		None
Returns:		None
Description:	Construction of LocalDB objects.
*****************************************************************************/
LocalDB::LocalDB()
{
	m_hEnv = SQL_NULL_HENV;
	m_hDbc = SQL_NULL_HDBC;
	m_bIsConnected = false;
	m_bIsRetrying = false;
}

/*****************************************************************************
Function Name:	~LocalDB
Arguments:		None
Returns:		None
Description:	Deonstruction of LocalDB objects.
*****************************************************************************/
LocalDB::~LocalDB()
{

}

//////////////////////////////////////////////////////////////////////
// Implementation of Methods
//////////////////////////////////////////////////////////////////////

/*****************************************************************************
Function Name:	OpenDB
Arguments:		Flag to indicate if opened with capable of transaction (SQLSMALLINT)
Returns:		(DWORD) (0:success 1:fail)
Description:	This function open an ODBC connection with Local DB.
*****************************************************************************/
DWORD LocalDB::OpenDB(SQLSMALLINT transactionFlag)
{
	m_transactionFlag = transactionFlag;

	RETCODE	 retCode = SQL_ERROR;	//	return code
	DWORD threadid = GetCurrentThreadId();

	// free allocated handel first
	if(m_hDbc != SQL_NULL_HDBC)
	{
		SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
		m_hDbc = SQL_NULL_HDBC;
	}
	if(m_hEnv != SQL_NULL_HENV)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
		m_hEnv = SQL_NULL_HENV;
	}
	//	Set environment parameters needed to open an ODBC connection
	retCode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &m_hEnv);
	if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO) && (retCode != SQL_NO_DATA))	//	error occured
	{
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:OpenDB Fail!"),threadid);
		(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQLAllocHandle failed"));
	}
    retCode = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);

	//	Set connection parameters needed to open an ODBC connection
	retCode = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);
	//	Set commit type of the connection according to transactionFlag
	retCode = SQLSetConnectAttr(m_hDbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)transactionFlag, SQL_IS_UINTEGER);

	// set cursor lib option
	retCode = SQLSetConnectAttr(m_hDbc, SQL_ATTR_ODBC_CURSORS, (SQLPOINTER)SQL_CUR_USE_IF_NEEDED, SQL_IS_UINTEGER);
	
	//	Open an ODBC connection
	retCode = SQLConnect(m_hDbc, (SQLWCHAR*)g_IZQDSNName, (SWORD)wcslen(g_IZQDSNName),
						 (SQLWCHAR*)g_IZQUserName, (SWORD)wcslen(g_IZQUserName),
						 (SQLWCHAR*)g_IZQPassword, (SWORD)wcslen(g_IZQPassword));
	if ( (retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO))	//	error occured
	{
		//	Trace the error
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Open LocalDB Fail!"),threadid);
		m_bIsConnected = false;
		retCode = SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
		retCode = SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);

		m_hEnv = SQL_NULL_HENV;
		m_hDbc = SQL_NULL_HDBC;

		return 1;	//	Exit with error
	}
	m_bIsConnected = true;
	(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:Open LocalDB Success: %ld"),threadid, m_hDbc);
	
	
	SQLINTEGER retFlag;
	SQLINTEGER flagLength;
	retCode = SQLGetConnectAttr(m_hDbc, SQL_ATTR_AUTOCOMMIT, &retFlag, SQL_IS_UINTEGER, &flagLength);

	(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:Open LocalDB Auto-commit Mode: %d"),threadid, retFlag);
	
	return 0;	//	success
}

/*****************************************************************************
Function Name:	CloseDB
Arguments:		None
Returns:		None
Description:	This function close an ODBC connection with Local DB.
*****************************************************************************/
void LocalDB::CloseDB()
{
	//	Delete the critical section for SQL statement executing
    SQLHDBC tempHandle = m_hDbc;
	DWORD rowCount =0 ;

	if(m_bIsConnected == false)
		return;	
	
	//	Close the ODBC connection
	try{
	
		SQLRETURN retCode = SQLDisconnect(m_hDbc);
		rowCount ++;
		//	Release handle resource
		retCode = SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
		m_hDbc = SQL_NULL_HDBC;

		rowCount ++;
		retCode = SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
		m_hEnv = SQL_NULL_HENV;

		if(retCode == SQL_SUCCESS)
			(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Close DB connection %d,successfully"),GetCurrentThreadId(), tempHandle);
		else
			(*gpDbSyncLog)(Log::L_DEBUG,_T("%d:Close DB connection %d,Failed"),GetCurrentThreadId(),tempHandle);
		m_bIsConnected = false;	
	}catch(...)
	{		
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exception catched when close DB connection %d.%d"),GetCurrentThreadId(),tempHandle,rowCount);
	}
	return;
}

/*****************************************************************************
Function Name:	ExecuteSQL
Arguments:		SQL Statement (SQLTCHAR *)
Returns:		(DWORD) (0:success !0:fail)
Description:	This function execute an SQL statement via ODBC connection with Local DB.
*****************************************************************************/
DWORD LocalDB::ExecuteSQL(SQLTCHAR *sqlStatement,DWORD deadlockRetryCount)
{
	DWORD dwErr;
	RETCODE	 retCode = SQL_ERROR;	//	return code
	SQLHSTMT hStmt = SQL_NULL_HSTMT;	//	handle of SQL statement

	//	Begin an critical section for SQL statement executing
	ZQ::common::MutexGuard tmpGd(m_crtlMutex);

	try{

		//	Execute an SQL statement via the ODBC connection
		retCode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt);
		DWORD threadid = GetCurrentThreadId();
		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO) && (retCode != SQL_NO_DATA))	//	error occured
		{
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s Fail!"),threadid, sqlStatement);
			(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQLAllocHandle failed"));
		}
		
		retCode = SQLSetStmtAttr(hStmt, SQL_ATTR_NOSCAN, (SQLPOINTER)SQL_NOSCAN_ON, SQL_IS_UINTEGER);
		
		retCode = SQLExecDirect(hStmt, sqlStatement, SQL_NTS);

		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO) && (retCode != SQL_NO_DATA))	//	error occured
		{
			//	Trace the error
		//	TRACE("%S Fail!", sqlStatement);
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s Fail!"),threadid, sqlStatement);
			
			switch (retCode)
			{
			case SQL_NEED_DATA:
				(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQL_NEED_DATA"));
				
				break;
			case SQL_STILL_EXECUTING:
				(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQL_STILL_EXECUTING"));
				
				break;
			case SQL_INVALID_HANDLE:
				 dwErr = GetLastError();
				(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQL_INVALID_HANDLE"));
				
				break;
			case SQL_ERROR:
				(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQL_ERROR"));
				
				break;
			default:
				(*gpDbSyncLog)(Log::L_INFO, _T("Reason: Unknown"));
				
				break;
			}
			SQLTCHAR sqlStateResult[MAX_PATH]=_T("");
			bool bFlag = TraceDBError(SQL_HANDLE_STMT, hStmt,sqlStateResult,MAX_PATH);
			if(bFlag)
			{
				//	---------- modified by KenQ 2006-08-03-------------
				// TraceDBError() is a block function, it returns ONLY after the connection is ready.
				// So call ExecuteSQL() again here to re-execute the failed sql operation 
				return ExecuteSQL(sqlStatement, deadlockRetryCount);
			}
			else

				if((wcscmp(sqlStateResult, _T("40001"))==0))
				{
				 //	---------- modified by HuangLi 2007-07-02-------------
			     // if the DataBase Locked, re-execute the failed sql operation
					if(deadlockRetryCount < g_SQLDeadlockRetryCount )
					{	
						Sleep(1);
						(*gpDbSyncLog)(Log::L_INFO, _T("Database Dead-lock happen, re-execute the failed SQL with %d/%d"), 
										deadlockRetryCount+1, g_SQLDeadlockRetryCount);
						return ExecuteSQL(sqlStatement, ++deadlockRetryCount);
					}
				}
		}
		else if(retCode == SQL_SUCCESS)
		{
			(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:%s, Success."), threadid,sqlStatement);	//	added for tracing
		}
		else if(retCode == SQL_NO_DATA)
		{
			(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:%s, Affect no records."), threadid,sqlStatement);	//	added for tracing
		}
		//	Release handle resource
		retCode = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	}catch(...)
	{		
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exception caught when ExecuteSQL"),GetCurrentThreadId());
		retCode = SQL_ERROR;
	}

	if(g_TraceTransactionCount != 0 && m_transactionFlag == SQL_AUTOCOMMIT_ON)
	{
		int tranCount = TraceTransactionCount();
		if(tranCount > 0)
		{
			(*gpDbSyncLog)(Log::L_ERROR, _T("@@TRANCOUNT = %d, after executing sql: %s"), tranCount, sqlStatement);
		}
	}

	return retCode;
}
DWORD LocalDB::SelectAllSQL(SQLTCHAR* sqlStatement,DBRECORD SelectCols,DBRESULTSETS* pResultSets)
{
	RETCODE	 retCode = SQL_ERROR;	//	return code
	SQLINTEGER retLength = 0;	//	length of return value
	SQLHSTMT hStmt = SQL_NULL_HSTMT;	//	handle of SQL statement

	//	Begin an critical section for SQL statement executing
	DWORD threadid = GetCurrentThreadId();
	ZQ::common::MutexGuard tmpGd(m_crtlMutex);

	try
	{
	
		//	Execute an SELECT SQL statement via the ODBC connection
		retCode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt);
		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO) && (retCode != SQL_NO_DATA))	//	error occured
		{
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s Fail!"),threadid, sqlStatement);
			(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQLAllocHandle failed"));
		}
		retCode = SQLExecDirect(hStmt, sqlStatement, SQL_NTS);
		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)&&(retCode != SQL_NO_DATA))	//	error occured
		{
			//	Trace the error

			//TRACE(_T("%s Fail!"), sqlStatement);
			(*gpDbSyncLog)(Log::L_INFO,_T("%s Fail!"), sqlStatement);
			SQLTCHAR sqlStateResult[MAX_PATH] = _T("");
			if(TraceDBError(SQL_HANDLE_STMT, hStmt,sqlStateResult,MAX_PATH))
			{
				//	---------- modified by KenQ 2006-08-03-------------
				// TraceDBError() is a block function, it returns ONLY after the connection is ready.
				// So call SelectAllSQL() again here to re-execute the failed sql operation 
				return SelectAllSQL(sqlStatement, SelectCols, pResultSets);
			}

		}
		else	//	successfully executed
		{
			//	Retrieve the value to variable
			while(1)
			{		
				DBRECORD record;
				for(DWORD i = 0; i < SelectCols.size();i++)
				{
					DBFIELD field = SelectCols.at(i);
					switch(field.lType)
					{
						case SQL_C_TCHAR:
							field.pValue = new BYTE[field.lSize*2];
							memset(field.pValue,0,field.lSize*2);
							retCode = SQLBindCol(hStmt, i+1, SQL_C_TCHAR, (SQLPOINTER)field.pValue, field.lSize*2, &field.lSize);
							break;
						case SQL_C_ULONG:
							field.pValue = new BYTE[4];
							memset(field.pValue,0,4);
							retCode = SQLBindCol(hStmt, i+1, SQL_C_ULONG, (SQLPOINTER)field.pValue, 4, &field.lSize);
							break;
						default:
							break;
					}
					record.push_back(field);
				}			
				retCode = SQLFetch(hStmt);

				if((retCode == SQL_SUCCESS_WITH_INFO))
				{
					SQLTCHAR sqlStateResult[MAX_PATH] = _T("");
					if(TraceDBError(SQL_HANDLE_STMT, hStmt,sqlStateResult,MAX_PATH))
					{
						// TraceDBError() is a block function, it returns ONLY after the connection is ready.
						// So call SelectAllSQL() again here to re-execute the failed sql operation 
						return SelectAllSQL(sqlStatement, SelectCols, pResultSets);
					}
				}
				if((retCode == SQL_SUCCESS)||(retCode == SQL_SUCCESS_WITH_INFO))
				{
					pResultSets->push_back(record);
				}			
				else 
				{
					if(retCode != SQL_NO_DATA)
					{
						//TRACE(_T("%s Fail!"), sqlStatement);							
						SQLTCHAR sqlStateResult[MAX_PATH] = _T("");
						if(TraceDBError(SQL_HANDLE_STMT, hStmt,sqlStateResult,MAX_PATH))
						{
							// TraceDBError() is a block function, it returns ONLY after the connection is ready.
							// So call SelectAllSQL() again here to re-execute the failed sql operation 
							return SelectAllSQL(sqlStatement, SelectCols, pResultSets);
						}
					}
					try{
					
						for (DWORD i = 0; i < record.size(); i ++)
						{
							DBFIELD field = record.at(i);
							delete[] field.pValue;
						}	
					}catch(...)
					{
						(*gpDbSyncLog)(Log::L_INFO,_T("%d:exception catch when release unused memory for ResultSet."),threadid);
					}
					break;
				}
			}

			SQLCloseCursor(hStmt);
		}
		//	Release handle resource
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

		if(pResultSets->size() == 0)
		{
			(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:%s: not found"),threadid, sqlStatement);	//	added for tracing	
			retCode = SQL_NO_DATA;
		}
		else
		{
			(*gpDbSyncLog)(Log::L_DEBUG,  _T("%d:%s: found"),threadid, sqlStatement);	//	added for tracing
			retCode = SQL_SUCCESS;
		}
	} catch (...) {
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exception caught when SelectAllSQL."),GetCurrentThreadId());
		retCode = SQL_ERROR;
	}

	if(g_TraceTransactionCount != 0 && m_transactionFlag == SQL_AUTOCOMMIT_ON)
	{
		int tranCount = TraceTransactionCount();
		if(tranCount > 0)
		{
			(*gpDbSyncLog)(Log::L_ERROR, _T("@@TRANCOUNT = %d, after executing sql: %s"), tranCount, sqlStatement);
		}
	}

	return retCode;
}
void LocalDB::FreeResultSets(DBRESULTSETS* ResultSets)
{
	try{
	
		for(DWORD i = 0; i < ResultSets->size() ; i++)
		{
			DBRECORD record = ResultSets->at(i);
			for(DWORD j =0; j < record.size(); j++)
			{
				DBFIELD field = record.at(j);
				delete[] field.pValue;
			}
			record.clear();
		}	
	}catch(...)
	{
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exception caught when free the ResultSet."),GetCurrentThreadId());
	}
	ResultSets->clear();
}
/*****************************************************************************
Function Name:	SelectSQL
Arguments:		SQL Statement (SQLTCHAR *), 
				Type of the variable to return (SQLSMALLINT), 
				Pointer to the variable to return (SQLPOINTER), 
				Buffer size of the variable to return (SQLINTEGER)
Returns:		(DWORD) (0:success !0:fail)
Description:	This function retrieve an variable (its type could be either
				SQL_C_ULONG or SQL_C_TCHAR) via ODBC connection 
				with Local DB.
*****************************************************************************/
DWORD LocalDB::SelectSQL(SQLTCHAR *sqlStatement, SQLSMALLINT sqlType, SQLPOINTER ptrValue, SQLINTEGER bufferSize)
{
	RETCODE	 retCode = SQL_ERROR;	//	return code
	SQLINTEGER retLength = 0;	//	length of return value
	SQLHSTMT hStmt = SQL_NULL_HSTMT;	//	handle of SQL statement

	//	Begin an critical section for SQL statement executing

	ZQ::common::MutexGuard tmpGd(m_crtlMutex);
	DWORD threadid = GetCurrentThreadId();

	try{

		//	Execute an SELECT SQL statement via the ODBC connection
		retCode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt);
		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO) && (retCode != SQL_NO_DATA))	//	error occured
		{
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s Fail!"),threadid, sqlStatement);
			(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQLAllocHandle failed"));
		}
		retCode = SQLExecDirect(hStmt, sqlStatement, SQL_NTS);
		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)&&(retCode != SQL_NO_DATA))	//	error occured
		{
			//	Trace the error

			//TRACE(_T("%s Fail!"), sqlStatement);
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s Fail!"),threadid, sqlStatement);
			SQLTCHAR sqlStateResult[MAX_PATH] = _T("");
			if(TraceDBError(SQL_HANDLE_STMT, hStmt,sqlStateResult,MAX_PATH))
			{
				// TraceDBError() is a block function, it returns ONLY after the connection is ready.
				// So call SelectSQL() again here to re-execute the failed sql operation 
				return SelectSQL(sqlStatement, sqlType, ptrValue, bufferSize);
			}
		}
		else	//	successfully executed
		{
		
			//	Retrieve the value to variable
			retCode = SQLBindCol(hStmt, 1, sqlType, ptrValue, bufferSize, &retLength);
			retCode = SQLFetch(hStmt);
			if(retCode == 1)
			{
				SQLTCHAR sqlStateResult[MAX_PATH] = _T("");
				if(TraceDBError(SQL_HANDLE_STMT, hStmt,sqlStateResult,MAX_PATH))
				{
					// TraceDBError() is a block function, it returns ONLY after the connection is ready.
					// So call SelectSQL() again here to re-execute the failed sql operation 
					return SelectSQL(sqlStatement, sqlType, ptrValue, bufferSize);
				}
			}
			SQLCloseCursor(hStmt);
		}
		//	Release handle resource
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

		if(retCode == SQL_NO_DATA)
		{
			(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:%s: not found"),threadid, sqlStatement);	//	added for tracing
			

		}
		else
		{
			(*gpDbSyncLog)(Log::L_DEBUG,  _T("%d:%s: found"),threadid, sqlStatement);//	added for tracing		

		}
	}catch (...) {
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exception caught when SelectSQL."),GetCurrentThreadId());
		retCode = SQL_ERROR;
	}

	if(g_TraceTransactionCount != 0 && m_transactionFlag == SQL_AUTOCOMMIT_ON)
	{
		int tranCount = TraceTransactionCount();
		if(tranCount > 0)
		{
			(*gpDbSyncLog)(Log::L_ERROR, _T("@@TRANCOUNT = %d, after executing sql: %s"), tranCount, sqlStatement);
		}
	}

	return retCode;
}

/*****************************************************************************
Function Name:	TraceDBError
Arguments:		Type of Handle (SQLSMALLINT), Handle (SQLHANDLE)
Returns:		Number of error message (SQLSMALLINT)
Description:	This function trace the error messages occured while executing
				an SQL statement via ODBC connection with Local DB.
				(Type of handle to trace could be SQL_HANDLE_ENV, SQL_HANDLE_DBC,
				SQL_HANDLE_STMT or SQL_HANDLE_DESC)
*****************************************************************************/
BOOL LocalDB::TraceDBError(SQLSMALLINT sqlHandleType, SQLHANDLE sqlHandle,SQLTCHAR *sqlStateResult,DWORD length)
{
	SQLTCHAR		sqlState[6];	//	string to indicate execution state 
	SQLTCHAR		sqlMsg[SQL_MAX_MESSAGE_LENGTH];	//	string to indicate error message 
	SQLINTEGER	sqlNativeError=0;	//	NO. of error 
	SQLSMALLINT	msgLen=0;	//	length of error message
	SQLSMALLINT	msgNum=0;	//	number of error messages
	DWORD threadid = GetCurrentThreadId();
	msgNum = 1;
	while (SQLGetDiagRec(sqlHandleType, sqlHandle, msgNum, sqlState, &sqlNativeError, 
		   sqlMsg, sizeof(sqlMsg), &msgLen) != SQL_NO_DATA) //	still has error message
	{
		//	Log the error message
		(*gpDbSyncLog)(Log::L_DEBUG,  _T("%d:SqlState: %s, NativeError: %d, Msg: %s, MsgLen: %d"),
			 threadid,sqlState, sqlNativeError, sqlMsg, msgLen);
		if((wcscmp(sqlState, _T("08S01"))==0)|| (wcscmp(sqlState,_T("")) ==0) ||(wcscmp(sqlState, _T("01000"))==0))
		{
			m_bIsConnected = false;
			(*gpDbSyncLog)(Log::L_ERROR, _T("%d:Connection to LAM is break.Block here.Try to reconnect it."),GetCurrentThreadId());

			if(OpenDB(m_transactionFlag) == SQL_SUCCESS)
			{
				return TRUE;
			}
			Sleep(ODBC_SCAN_TIMEOUT);
			continue;
		}

		StringCbCopy(sqlStateResult,length * 2,sqlState);

		return FALSE;
	}
	return FALSE;
}

SQLSMALLINT LocalDB::GetParamType(PARAMTYPE type)
{
	switch(type)
	{
	case INPUT:
		return SQL_PARAM_INPUT;
	case OUTPUT:
		return SQL_PARAM_OUTPUT;
	case INOUTPUT:
		return SQL_PARAM_INPUT_OUTPUT;
	default:
		return SQL_PARAM_INPUT;
	}
	return SQL_PARAM_INPUT;
}


/*****************************************************************************
Function Name:	`Procedure
Arguments:		SQL Statement (SQLTCHAR *), 
				Value Type of Input Parameter (SQLSMALLINT), Type of Input Parameter (SQLSMALLINT),
				Column Size of Input Parameter (SQLUINTEGER), Decimal Digits of Input Parameter (SQLSMALLINT),
				Pointer to Input Parameter Value (SQLPOINTER), Buffer Size of Input Parameter (SQLINTEGER),
				Length Indication of Input Parameter (SQLINTEGER),
				Value Type of Output Parameter (SQLSMALLINT), Type of Output Parameter (SQLSMALLINT),
				Column Size of Output Parameter (SQLUINTEGER), Decimal Digits of Output Parameter (SQLSMALLINT),
				Pointer to Output Parameter Value (SQLPOINTER), Buffer Size of Output Parameter (SQLINTEGER),
				Length Indication of Output Parameter (SQLINTEGER)
				Value Type of Output Parameter 2 (SQLSMALLINT), Type of Output Parameter 2 (SQLSMALLINT),
				Column Size of Output Parameter 2 (SQLUINTEGER), Decimal Digits of Output Parameter 2 (SQLSMALLINT),
				Pointer to Output Parameter Value 2 (SQLPOINTER), Buffer Size of Output Parameter 2 (SQLINTEGER),
				Length Indication of Output Parameter 2 (SQLINTEGER)
Returns:		(DWORD) (0:success !0:fail)
Description:	This function executes a store procedure via ODBC connection with Local DB.
*****************************************************************************/
DWORD LocalDB::CallStoreProcedure(SQLTCHAR *sqlStatement, 
								  PARAMTYPE paramType0, SQLSMALLINT inValueType, SQLSMALLINT inParaType, SQLUINTEGER inColSize, SQLSMALLINT inDecDigits,
								  SQLPOINTER inParaValuePtr, SQLINTEGER inBufferLength, SQLINTEGER *inIndPtr,
								  PARAMTYPE paramType1, SQLSMALLINT outValueType, SQLSMALLINT outParaType, SQLUINTEGER outColSize, SQLSMALLINT outDecDigits,
								  SQLPOINTER outParaValuePtr, SQLINTEGER outBufferLength, SQLINTEGER *outIndPtr,
								  PARAMTYPE paramType2, SQLSMALLINT outValueType2, SQLSMALLINT outParaType2, SQLUINTEGER outColSize2, SQLSMALLINT outDecDigits2,
								  SQLPOINTER outParaValuePtr2, SQLINTEGER outBufferLength2, SQLINTEGER *outIndPtr2,
								  DWORD deadlockRetryCount ,BOOL checkTranCount)
{
	RETCODE	 retCode = SQL_ERROR;	//	return code
	SQLHSTMT hStmt = SQL_NULL_HSTMT;	//	handle of SQL statement
	DWORD threadid = GetCurrentThreadId();
	//	Begin an critical section for SQL statement executing
	ZQ::common::MutexGuard tmpGd(m_crtlMutex);

	(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:%s"),threadid, sqlStatement);	//	added for tracing
	
	try
	{
		//	Bind parameters for the store procedure
		retCode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt);
		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO) && (retCode != SQL_NO_DATA))	//	error occured
		{
			(*gpDbSyncLog)(Log::L_INFO,_T("%d:%s Fail!"),threadid, sqlStatement);
			(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQLAllocHandle failed"));
		}

		// set the timeout to be 5 minutes
		SQLSetStmtAttr(hStmt, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)300, SQL_IS_UINTEGER);

		if(inParaValuePtr != NULL)
		{
			retCode = SQLBindParameter(hStmt, 1, GetParamType(paramType0), 
									   inValueType, inParaType, inColSize, inDecDigits,
									   inParaValuePtr, inBufferLength, inIndPtr);
		}

		if(inParaValuePtr != NULL && outParaValuePtr != NULL)
		{
			retCode = SQLBindParameter(hStmt, 2, GetParamType(paramType1), 
									   outValueType, outParaType, outColSize, outDecDigits,
									   outParaValuePtr, outBufferLength, outIndPtr);
		}

		if(inParaValuePtr != NULL && outParaValuePtr != NULL && outParaValuePtr2 != NULL)
		{
			retCode = SQLBindParameter(hStmt, 3, GetParamType(paramType2), 
									   outValueType2, outParaType2, outColSize2, outDecDigits2,
									   outParaValuePtr2, outBufferLength2, outIndPtr2);
		}

		//	Call a store procedure via the ODBC connection
		retCode = SQLExecDirect(hStmt, sqlStatement, SQL_NTS);
		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)&&(retCode != SQL_NO_DATA))	//	error occured
		{

			(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:%s Fail!"), threadid,sqlStatement);
	

			SQLTCHAR sqlStateResult[MAX_PATH] = _T("");
			bool bFlag = TraceDBError(SQL_HANDLE_STMT, hStmt,sqlStateResult,MAX_PATH);
			if(bFlag)
			{
				// TraceDBError() is a block function, it returns ONLY after the connection is ready.
				// So call CallStoreProcedure() again here to re-execute the failed sql operation 
				return CallStoreProcedure(sqlStatement, 
									  paramType0, inValueType, inParaType, inColSize, inDecDigits,
									  inParaValuePtr, inBufferLength, inIndPtr,
									  paramType1, outValueType, outParaType, outColSize, outDecDigits,
									  outParaValuePtr, outBufferLength, outIndPtr,
									  paramType2, outValueType2, outParaType2, outColSize2, outDecDigits2,
									  outParaValuePtr2, outBufferLength2, outIndPtr2, deadlockRetryCount, checkTranCount);

			}
			else

				if((wcscmp(sqlStateResult, _T("40001")) == 0))
				{
				 //	---------- modified by HuangLi 2007-07-10-------------
			     // if the DataBase Locked, re-execute the failed sql operation
					if(deadlockRetryCount < g_SQLDeadlockRetryCount )
					{	
						Sleep(1);
						(*gpDbSyncLog)(Log::L_INFO, _T("Database Dead-lock happen, re-execute the failed SQL with %d/%d"), 
										deadlockRetryCount+1, g_SQLDeadlockRetryCount);
						return CallStoreProcedure(sqlStatement, 
									  paramType0, inValueType, inParaType, inColSize, inDecDigits,
									  inParaValuePtr, inBufferLength, inIndPtr,
									  paramType1, outValueType, outParaType, outColSize, outDecDigits,
									  outParaValuePtr, outBufferLength, outIndPtr,
									  paramType2, outValueType2, outParaType2, outColSize2, outDecDigits2,
									  outParaValuePtr2, outBufferLength2, outIndPtr2, ++deadlockRetryCount, checkTranCount);
					}
				}
		}

		while ( ( retCode = SQLMoreResults(hStmt) ) != SQL_NO_DATA ) 
		{
			// can not Sleep() here;
		}

		//	Release handle resource
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

		(*gpDbSyncLog)(Log::L_DEBUG,  _T("%d:done"),threadid);	//	added for tracing
	
	} catch (...) {
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exception caught when CallStoreProcedure."),GetCurrentThreadId());
		retCode = SQL_ERROR;
	}	

	if( (g_TraceTransactionCount != 0 || checkTranCount) && m_transactionFlag == SQL_AUTOCOMMIT_ON)
	{
		int tranCount = TraceTransactionCount();
		if(tranCount > 0)
		{
			(*gpDbSyncLog)(Log::L_ERROR, _T("@@TRANCOUNT = %d, after executing sql: %s. Will re-create the connection"), tranCount, sqlStatement);

			(*gpDbSyncLog)(Log::L_NOTICE, _T("Close Database connection..."));
			CloseDB();

			(*gpDbSyncLog)(Log::L_NOTICE, _T("Open Database connection..."));
			while( OpenDB(m_transactionFlag) != SQL_SUCCESS)
			{
				(*gpDbSyncLog)(Log::L_ERROR, _T("Create Database connection failed, re-create 3 seconds later"));
				Sleep(ODBC_SCAN_TIMEOUT);
			}
			(*gpDbSyncLog)(Log::L_NOTICE, _T("Database connection re-created successfully"));
		}
	}
	
	return retCode;
}


/*****************************************************************************
Function Name:	EndTransaction
Arguments:		Flag to indicate whether to Commit or to Rollback (SQLSMALLINT)
Returns:		(DWORD) (0:success 1:fail)
Description:	This function commits or rollbacks a transaction in Local DB.
*****************************************************************************/
DWORD LocalDB::EndTransaction(SQLSMALLINT endFlag)
{
	RETCODE	 retCode = SQL_ERROR;	//	return code
	DWORD threadid = GetCurrentThreadId();
	//	End transaction with endFlag which might be COMMIT or ROLLBACK
	if(endFlag == SQL_ROLLBACK)
		int i =0;
	try{
	
	retCode = SQLEndTran(SQL_HANDLE_DBC, m_hDbc, endFlag);

	if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)&&(retCode!= SQL_NO_DATA))	//	error occured
	{
		//	Trace the eror
 		(*gpDbSyncLog)(Log::L_INFO, _T("%d:End Transaction Fail!"),threadid);
		SQLTCHAR sqlStateResult[MAX_PATH] = _T("");
		if(TraceDBError(SQL_HANDLE_DBC, m_hDbc,sqlStateResult,MAX_PATH))
		{
			//	---------- modified by KenQ 2006-08-03-------------
			// TraceDBError() is a block function, it returns ONLY after the connection is ready.
			// So call EndTransaction() again here to re-execute the failed sql operation 
			return EndTransaction(endFlag);
		}

		return 1;	//	Exit with error
	}
	}
	catch(...)
	{
		endFlag = SQL_ROLLBACK;
		(*gpDbSyncLog)(Log::L_INFO, _T("%d:Cath exception Commit Transaction %ld Successly."),threadid, m_hDbc);
	}
	if (endFlag == SQL_COMMIT)	//	to end with Commit
		(*gpDbSyncLog)(Log::L_DEBUG, _T("%d:Commit Transaction %ld Successly."),threadid, m_hDbc);
	else	//	to end with Rollback
		(*gpDbSyncLog)(Log::L_INFO, _T("%d:Rollback Transaction %ld Successly."),threadid, m_hDbc);
	

	if(g_TraceTransactionCount != 0 && m_transactionFlag == SQL_AUTOCOMMIT_OFF)
	{
		int tranCount = TraceTransactionCount();
		if(tranCount > 0)
		{
			(*gpDbSyncLog)(Log::L_ERROR, _T("@@TRANCOUNT = %d, after executing GetNewLocalUID()"), tranCount);
		}
	}

	return 0;	//	success
}

/*****************************************************************************
Function Name:	IsConnected
Arguments:		
Returns:		(DWORD) (0:success 1:fail)
Description:	This function check the whether connection to DB is ok.
*****************************************************************************/
DWORD LocalDB::IsConnected(void)
{
	if(!m_bIsConnected)
		return 1;
	return 0;
}
void LocalDB::RetryConnection(void)
{
	// do nothing if conection is ok.

	if(m_bIsConnected||m_bIsRetrying)
		return;

	m_bIsRetrying = true;
	while (!m_bIsConnected)
	{
//		SQLGetInfo(m_hDbc)
		(*gpDbSyncLog)(Log::L_DEBUG,_T("Trying to reconnect to local database..."));	

		if(OpenDB(m_transactionFlag) == SQL_SUCCESS)						
		{		
		//	---------- modified by KenQ 2006-08-03-------------
		//  Changed original logic - restart DBSync when local db connection lost
		//  to be without restarting, coz now there is Queue to keep the callback
		//  received during the connection broken
			
			// break;

			m_bIsRetrying = false;
			(*gpDbSyncLog)(Log::L_DEBUG,_T("Reconnect to LAM now succeed"));	

			return;
		}
		Sleep(ODBC_SCAN_TIMEOUT);
	}	
	// relaunch the service
	//server.exitProcess();
	CloseDB();
//	server.exitProcess();

	//////////////////////////////////////////////////////////////////////////
	// added by Bernie.Zhao  2005.11.4
	// If the connection to LAM is broken, restart all
//	RTNDEF rtn = 0;
//	try
//	{
//		rtn = g_ds.UnInitDS();
//	} catch(...)
//	{
//		(*gpDbSyncLog)(Log::L_INFO,_T("Local DB encounters an expection when unit the service."));	
//	}
//
//	for(;;)
//	{
//		Sleep(ITV_RETRY_TIMEOUT);
//		try
//		{
//			rtn = g_ds.InitDS();
//		} catch(...)
//		{
//			(*gpDbSyncLog)(Log::L_INFO,_T("Local DB encounters an expection when init the service."));	
//		}
//
//		if(rtn == ITV_SUCCESS)
//			break;
//	}

	m_bIsRetrying = false;
	(*gpDbSyncLog)(Log::L_NOTICE,_T("Connection to LAM is lost. Restart the service"));	
	HANDLE handle = OpenEvent(EVENT_ALL_ACCESS,false,_T("DBSYNCAPP_Stop"));
	SetEvent(handle);
}

bool LocalDB::VerifyConnection(void)
{
	RETCODE nRetCode = SQL_ERROR;
	SWORD nResult;

	SWORD nAPIConformance;
	nRetCode = ::SQLGetInfo(m_hDbc, SQL_ODBC_API_CONFORMANCE,
		&nAPIConformance, sizeof(nAPIConformance), &nResult);
	if (nRetCode == SQL_ERROR)
		return false;

	if (nAPIConformance < SQL_OAC_LEVEL1)
		return false;

	SWORD nSQLConformance;
	nRetCode = ::SQLGetInfo(m_hDbc, SQL_ODBC_SQL_CONFORMANCE,
		&nSQLConformance, sizeof(nSQLConformance), &nResult);
	if (nRetCode == SQL_ERROR)
		return false;

	if (nSQLConformance < SQL_OSC_MINIMUM)
		return false;
	return true;
}

int LocalDB::TraceTransactionCount()
{
	SQLTCHAR sqlStatement[1024];
	StringCbPrintf(sqlStatement, 1024*sizeof(SQLTCHAR), _T("SELECT @@TRANCOUNT"));
	
	SQLSMALLINT sqlType = SQL_C_ULONG;
	DWORD retValue = 0;
	SQLPOINTER ptrValue = &retValue;
	SQLINTEGER bufferSize = 0;

	RETCODE	 retCode = SQL_ERROR;	//	return code
	SQLINTEGER retLength = 0;	//	length of return value
	SQLHSTMT hStmt = SQL_NULL_HSTMT;	//	handle of SQL statement

	try{
		//	Execute an SELECT SQL statement via the ODBC connection
		retCode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt);
		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO) && (retCode != SQL_NO_DATA))	//	error occured
		{
			(*gpDbSyncLog)(Log::L_INFO, _T("Reason: SQLAllocHandle failed"));
		}
		retCode = SQLExecDirect(hStmt, sqlStatement, SQL_NTS);
		if ((retCode != SQL_SUCCESS) && (retCode != SQL_SUCCESS_WITH_INFO)&&(retCode != SQL_NO_DATA))	//	error occured
		{
			(*gpDbSyncLog)(Log::L_INFO,_T("%s Fail!"), sqlStatement);
		}
		else	//	successfully executed
		{
		
			//	Retrieve the value to variable
			retCode = SQLBindCol(hStmt, 1, sqlType, ptrValue, bufferSize, &retLength);
			retCode = SQLFetch(hStmt);
			if(retCode == 1)
			{
				(*gpDbSyncLog)(Log::L_INFO,_T("%s Fail!"), sqlStatement);	
			}
			SQLCloseCursor(hStmt);
		}
		//	Release handle resource
		SQLFreeHandle(SQL_HANDLE_STMT, hStmt);

		if(retCode == SQL_NO_DATA)
		{
			(*gpDbSyncLog)(Log::L_DEBUG, _T("%s: not found"), sqlStatement);	//	added for tracing
		}
		else
		{
			//(*gpDbSyncLog)(Log::L_DEBUG,  _T("%d:%s: found"),threadid, sqlStatement);//	added for tracing		

		}
	}catch (...) {
		(*gpDbSyncLog)(Log::L_INFO,_T("%d:Exception caught when SelectSQL."),GetCurrentThreadId());
		retCode = SQL_ERROR;
	}

	return retValue;
}