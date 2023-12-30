// ImportDB.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include <fstream>
#include <string>
#include <vector>
#include <comdef.h>




/*

  ImportDB  <-p>  <-i> <-f | -n | -s> [-d]
	 -p <files path to import into db>
	 -i <import table info file>, contains the source file name, table name, table item count, delimited by ",",a line for one file
	 -f <Access mdb filename for import>
	 -n <system ODBC DSN>, specific the DSN in the system ODBC DSN Manager
	 -s <DSN string>, directly specific the DSN string for the DB connection
	 -d <delimited character>, default is "~"
	 example:
	 ImportDB -p c:\result -i table.txt -f 2008-08-12.mdb  
	 ImportDB -p c:\result -i table.txt -n TestDSN
	 ImportDB -p c:\result -i table.txt -s "Driver={SQL Server};Description=sqldemo;SERVER=127.0.0.1;UID=LoginID;PWD=Password;DATABASE=Database_Name"
	 
	   the import table info file format:
	   
		 ssm_req.txt,ssm_req,7
		 vmca_req.txt,vmca_req,9
		 rtsp_req.txt,rtsp_req,7
		 ssm_cstrm.txt,ssm_cstrm,5
		 ssm_lam.txt,ssm_lam,9
		 ss_crdb.txt,ss_crdb,3
		 rtsp_recv.txt,rtsp_recv,4

  */


#pragma warning(push)

#pragma warning(disable : 4146)

#import <msado15.dll> rename("EOF","adoEOF") 
using namespace ADODB;

#pragma warning(pop)

using namespace std;

struct ImportDBInfo{
	char szFile[30];
	char szTable[30];
	int	 nColumon;
	
	ImportDBInfo()
	{
		memset(szFile,0,sizeof(szFile));
		memset(szTable,0,sizeof(szTable));
		nColumon = 0;
	}
};

bool getTableFile(const std::string& strFile, std::vector<struct ImportDBInfo>& dbInfoV);//get the info list

void usage()
{
	printf("ImportDB  <-p>  <-i> <-f | -n | -s> [-d]\n\n");
	printf("-p <files path to import into db>\n");
	printf("-i <import table info file>, contains the source file name, table name, table item count, delimited by ',',a line for one file\n");
	printf("-f <Access mdb filename for import>\n");
	printf("-n <system ODBC DSN>, specific the DSN in the system ODBC DSN Manager\n");
	printf("-s <DSN string>, directly specific the DSN string for the DB connection\n");
	printf("-d <delimited character>, default is character '~'\n\n");
	printf("example:\n");
	printf("-p c:\\result -i table.txt -f report.mdb\n");
	printf("-p c:\\result -i table.txt -n testdb\n");
	printf("-p c:\\result -i table.txt -s \"Driver={SQL Server};Description=sqldemo;SERVER=127.0.0.1;UID=LoginID;PWD=Password;DATABASE=Database_Name\"\n");

}

int main(int argc, char* argv[])
{
	if (argc<7)
	{
		printf("Not input right parameter\n\n");
		usage();
		return 0;
	}
	std::string strCom;
	std::string strPath;
	std::string strTableFile;
	std::string strDSN;
	char chIn = '~';//delimited character
	for(int nar = 1; nar < argc-1; nar = nar + 2)
	{
		strCom = argv[nar];
		if(stricmp(strCom.c_str(),"-p") == 0)//file path
			strPath = argv[nar+1];
		else if(stricmp(strCom.c_str(),"-i") == 0)//import table info file
			strTableFile = argv[nar+1];
		else if(stricmp(strCom.c_str(),"-f") == 0)//Access mdb filename for import
		{
			strDSN = "DRIVER={Microsoft Access Driver (*.mdb)};DBQ=";
			strDSN += argv[nar+1];
		}
		else if(stricmp(strCom.c_str(),"-n") == 0)//system ODBC DSN
		{
			strDSN = argv[nar+1]; 
		}
		else if(stricmp(strCom.c_str(),"-s") == 0)//DSN string
			strDSN = argv[nar+1];
		else if(stricmp(strCom.c_str(),"-d") == 0)//delimited character
			chIn = argv[nar+1][0];
		else
		{
			printf("Unknown the argument '%s'\n",strCom.c_str());
			return 0;
		}
	}

	std::vector<struct ImportDBInfo> dbInfoV;
	if(!getTableFile(strTableFile, dbInfoV))
	{
		printf("getTableFile() false\n");
		return 0;
	}
	
	HANDLE hHandle = GetCurrentThread();	
	SetThreadPriority(hHandle, THREAD_PRIORITY_BELOW_NORMAL);
	
	CoInitialize(NULL);
	
	//do ImportDB
	std::vector<struct ImportDBInfo>::iterator it;
	int nImportDBedFile=0;
	for(it = dbInfoV.begin();it < dbInfoV.end(); it++)
	{
		//
		//connect to the db
		//	
		_ConnectionPtr	pADODB;
		
		try
		{
			if(pADODB.CreateInstance(__uuidof(Connection)))
			{
				printf("Failed to create ado connection object\n");
				CoUninitialize();
				return 0;
			}
			
			pADODB->Open(strDSN.c_str(), L"", L"", -1);		
		}
		catch(_com_error e)
		{
			printf("Failed to open database with DSN[%s] with error[%s]\n", strDSN.c_str(), e.ErrorMessage());	
			CoUninitialize();
			return 0;
		}

		//
		// open file
		//		
		char szFileName[512];
		sprintf(szFileName, "%s\\%s", strPath.c_str(),it->szFile);
		ifstream imfile;
		imfile.open(szFileName, ios_base::in);
		if(!imfile.is_open())
		{
			printf("Failed to open file %s\n", szFileName);
			continue;
		}
		printf("Open file[%s] successful\n", szFileName);

		//
		// open table
		//
		_RecordsetPtr	pRecordSet;
		
		try
		{
			pRecordSet.CreateInstance(__uuidof(Recordset));	
			pRecordSet->putref_ActiveConnection(pADODB);
			pRecordSet->Open((LPCTSTR)it->szTable, _variant_t((IDispatch *) pADODB, true),adOpenStatic,adLockPessimistic,adCmdTable);
			printf("Success to open table[%s]\n", it->szTable);
		}
		catch(_com_error e)
		{
			printf("Failed to open table[%s] with error[%s]\n", it->szTable, e.ErrorMessage());	
			continue;
		}
		printf("Importing file %s...\n",it->szFile);
		
		//
		// read file & insert into
		//
		int nImportDBCount=0;
		for(int nLineNo=1;!imfile.eof();nLineNo++)
		{
			char szLine[1024];		
			int nRead=sizeof(szLine) - 1;
			imfile.getline(szLine, nRead);
			if (!szLine[0])
			{
				continue;
			}

			const char* szItems[16];
			int nItemCount=0;
			{
				memset(szItems, 0, sizeof(szItems));
				char* pPtr = szLine;				
				szItems[nItemCount++]=pPtr;

				do				
				{
					if (*pPtr==chIn)
					{
						*pPtr='\0';
						pPtr++;
						szItems[nItemCount++]=pPtr;

						if (nItemCount>=15)
						{
							break;
							printf("Overflowed\n");
						}
					}
					else
					{
						pPtr++;
					}					
				}while(*pPtr&&pPtr-szLine<1024);
			}

			if (!szItems[0][0])
			{
				printf("Line[%d] in file[%s] not correct\n", nLineNo, szFileName);
				continue;
			}

			if (nItemCount != it->nColumon)
			{
				printf("Warning: line[%d] columon count is %d not %d\n", nLineNo, nItemCount, it->nColumon);
			}

			//
			// insert into table
			//
			try
			{
				pRecordSet->AddNew();				
				for(int k=0;k<nItemCount;k++)
				{
					_bstr_t tmp(szItems[k]);
					pRecordSet->Fields->Item[(long)k]->Value = tmp; 
				}
				pRecordSet->Update(vtMissing, vtMissing);
				nImportDBCount++;
			}
			catch(_com_error e)
			{
				pRecordSet->CancelUpdate();
				printf("Failed to add line[%d] to table[%s], error[%s]\n", nLineNo, it->szTable, e.ErrorMessage());
			}

		};

		imfile.close();

		try
		{
			pRecordSet->Close();			
		}
		catch(_com_error e)
		{
			printf("Exception found when close recordset for table[%s], %s\n", it->szTable, e.ErrorMessage());			
		}
		

		printf("Success to ImportDB file[%s] to table[%s], record count[%d]\n", szFileName, it->szTable, nImportDBCount);
		nImportDBedFile++;

		try
		{
			pADODB->Close();
		}
		catch(_com_error e)
		{
			printf("Exception found when close connection, %s\n", e.ErrorMessage());	
		}	
	}
	printf("ImportDBed [%d] files to DB with DSN[%s]\n", nImportDBedFile, strDSN.c_str());

	

	CoUninitialize();

	return 0;
}

bool getTableFile(const std::string& strFile, std::vector<struct ImportDBInfo>& dbInfoV)
{
	ifstream imfile;
	imfile.open(strFile.c_str(), ios_base::in);
	if(!imfile.is_open())
	{
		printf("Failed to open file %s\n", strFile.c_str());
		return false;
	}
	char szLine[1024] = {0};		

	while(!imfile.eof())
	{
		memset(szLine,0,sizeof(szLine));
		imfile.getline(szLine, sizeof(szLine)-1);
		if (!szLine[0])
		{
			continue;
		}
		char* pB = szLine;
		char* pI = strchr(szLine,',');
		if(pI == NULL)
			continue;

		*pI = '\0';
		struct ImportDBInfo info;
		memcpy(info.szFile,pB,strlen(pB));
		pB = pI+1;
		pI = strchr(pB,',');
		if(pI == NULL)
			continue;

		*pI = '\0';
		memcpy(info.szTable, pB,strlen(pB));
		pB = pI+1;
		info.nColumon = atoi(pB);

		dbInfoV.push_back(info);
	}
	imfile.close();

	return true;
}
