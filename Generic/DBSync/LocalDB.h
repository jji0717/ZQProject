/*****************************************************************************
File Name:		LocalDB.h
Author:			Interactive ZQ 
Security:		<NVOD over iTV, Kunming>, Confidential
Description:	Definition of Class LocalDB, which is used for operation on 
                local oracle database via ODBC
Function Inventory: 
				LocalDB
				~LocalDB
				OpenLocalDB
				ExecuteSQL
				CloseLocalDB
				TraceDBError
Modification Log:
When		Version		Who			What
-----------------------------------------------------------------------------
11/21/2002	    0		W.Z.	Original Program (Created)
01/30/2003		1		W.Z.	Modify to utilize transaction mechanism
*****************************************************************************/


//////////////////////////////////////////////////////////////////////
// Declaratoin of include files and definition of symbols, data types
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "locks.h"

using std::vector;

typedef struct tagDBField{			
	BYTE* pValue;
	SQLINTEGER lSize;
	SQLINTEGER lType;	
}DBFIELD;

typedef vector<DBFIELD> DBRECORD;
typedef vector<DBRECORD> DBRESULTSETS;

class LocalDB  
{
	protected:
		SQLHENV	m_hEnv;	//	handle of ODBC environment
		SQLHDBC	m_hDbc;	//	handle of ODBC connection
		ZQ::common::Mutex m_crtlMutex;	//	structure used by a critical section
		bool m_bIsConnected;
		SQLSMALLINT m_transactionFlag;

		BOOL TraceDBError(SQLSMALLINT sqlHandleType, SQLHANDLE sqlHandle,SQLTCHAR* sqlStateResult,DWORD length);	//	Internal method to trace SQL execution error

	public:
		LocalDB();		//	Construction function of class LocalDB
		virtual ~LocalDB();		//	Destruction function of class LocalDB

		typedef enum { INPUT, OUTPUT, INOUTPUT } PARAMTYPE;

		DWORD OpenDB(SQLSMALLINT);	//	External method to open an ODBC connection
		void CloseDB();	//	External method to close an ODBC connection
		DWORD ExecuteSQL(SQLTCHAR *sqlStatement, DWORD deadlockRetryCount = 0);	//	External method to execute a SQL statement via ODBC connection
		DWORD SelectSQL(SQLTCHAR *SQLStr, SQLSMALLINT, SQLPOINTER, SQLINTEGER);	//	External method to retrieve a variable via ODBC connection
		DWORD SelectAllSQL(SQLTCHAR*,DBRECORD SelectCols, DBRESULTSETS* pResultSets);
		void FreeResultSets(DBRESULTSETS* ResultSets);
		DWORD CallStoreProcedure(SQLTCHAR *, 
			                     PARAMTYPE, SQLSMALLINT, SQLSMALLINT, SQLUINTEGER, SQLSMALLINT, SQLPOINTER, SQLINTEGER, SQLINTEGER *,
								 PARAMTYPE, SQLSMALLINT, SQLSMALLINT, SQLUINTEGER, SQLSMALLINT, SQLPOINTER, SQLINTEGER, SQLINTEGER *,		
								 PARAMTYPE, SQLSMALLINT, SQLSMALLINT, SQLUINTEGER, SQLSMALLINT, SQLPOINTER, SQLINTEGER, SQLINTEGER *,
								 DWORD deadlockRetryCount =0, BOOL checkTranCount = FALSE);		
								//	External method to execute a stored procedure via ODBC connection
		DWORD EndTransaction(SQLSMALLINT);	//	External method to commit or rollback a transaction
		DWORD IsConnected(void);// check the connection to LAM is ok
		void RetryConnection(void);
		bool VerifyConnection(void);

		bool m_bIsRetrying;

	private:
		SQLSMALLINT GetParamType(PARAMTYPE type);

		int TraceTransactionCount();
};

