// LogPageOpe.cpp: implementation of the LogPageOpe class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogPageOpe.h"
#include "stdio.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define MAX_LOG_LENGTH		4096
#define CLOG_FILE_HEAD_LENGTH    10

LogPageOpe::LogPageOpe()
{

}

LogPageOpe::~LogPageOpe()
{

}

void LogPageOpe::Analyse(int argc, char** argv)
{
	if(argc == 1)
		printf("Please input your command!");

	else if(argc == 2)
	{
		if(stricmp(argv[1],"-h") == 0)
			GetHelp();
		else if(stricmp(argv[1],"-e") == 0)
			exit(0);
		else
			Error();
	}
	else if(argc == 5)
	{
		DWORD count = 0;
		if(stricmp(argv[1],"-c") == 0 && stricmp(argv[3],"-f") == 0)
		{
			count = GetPageCount(atoi(argv[2]),argv[4]);
			printf("%d",count);
		}
		else if(stricmp(argv[1],"-f") == 0 && stricmp(argv[3],"-c") == 0)
		{
			count = GetPageCount(atoi(argv[4]),argv[2]);
			printf("%d",count);
		}
		else 
			Error();
	}
	
	else if(argc == 7)
	{
		INT64 size = 0;
		DWORD page = 0;
		std::string strFileName = "";
		std::string strFileType = "";

		for(int i=1; i<argc; i=i+2)
		{
			if(stricmp(argv[i],"-s") == 0)
				size = atoi(argv[i+1]);
			else if(stricmp(argv[i],"-c") == 0)
				size = atoi(argv[i+1]);
			else if(stricmp(argv[i],"-p") == 0)
				page = atoi(argv[i+1]);
			else if(stricmp(argv[i],"-f") == 0)
				strFileName = argv[i+1];
			else if(stricmp(argv[i],"-t") == 0)
				strFileType = argv[i+1];
		}
		if(size !=0 && page !=0 && strFileName != "")
			GetPointPage(size,page,strFileName.c_str());
		else if(size !=0 && strFileType !="" && strFileName != "")
		{
			DWORD count = GetPageCount(size,strFileName.c_str());
			printf("%d",count);
		}			
		else
			Error();
	}
	else if(argc == 9)
	{
		INT64 size = 0;
		DWORD page = 0;
		std::string strFileName = "";
		std::string strFileType = "";

		for(int i=1; i<argc; i=i+2)
		{
			if(stricmp(argv[i],"-s") == 0)
				size = atoi(argv[i+1]);
			else if(stricmp(argv[i],"-p") == 0)
				page = atoi(argv[i+1]);
			else if(stricmp(argv[i],"-f") == 0)
				strFileName = argv[i+1];
			else if(stricmp(argv[i],"-t") == 0)
				strFileType = argv[i+1];
		}
		if(size !=0 && page !=0 && strFileName != "" && strFileType != "")
			GetPointPage(size,page,strFileName.c_str(),strFileType.c_str());
		else
			Error();
	}
	else
	{
		Error();
	}
}

void LogPageOpe::GetHelp()
{
	printf("-c<size> -f<filename>	filename文件分成size(B)大小的页面数\n"
		"-s<size> -p<n> -f<filename> -t<filelog/sclog>	文件类型为filelog或sclog(省略为filelog)的filename文件分成size(B)大小的页面，第n页的内容\n");
}

DWORD LogPageOpe::GetPageCount(INT64 nSize, const char *pFileName)
{
	if(nSize <= 0)
		return 0;
	HANDLE hFile = CreateFileA(pFileName, GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE| FILE_SHARE_DELETE,	NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwCount = SetFilePointer(hFile,0,NULL,FILE_END);
		CloseHandle(hFile);
		if(dwCount%nSize == 0)
			dwCount = dwCount/nSize;
		else
			dwCount = dwCount/nSize + 1;
		return dwCount;
	}
	return 0;
}

void LogPageOpe::GetPointPage(INT64 nSize, DWORD nPage, const char *pFileName, const char* pType)
{
	if(nSize <= 0)
		return ;
	if(stricmp(pType,"sclog") != 0 && stricmp(pType,"filelog") != 0)
	{
		printf("Unknown log file type: %s. (only support type \"filelog\" and \"sclog\")\n",pType);
		return;
	}

	if(stricmp(pType,"sclog") == 0)
	{
		GetSCLogPage(nSize,nPage,pFileName);  //sclog 
		return;
	}

	HANDLE hFile = CreateFileA(pFileName, GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE| FILE_SHARE_DELETE,	NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("Open the file \"%s\" failed\n",pFileName);
		return;
	}
	
	else
	{
		DWORD dwFSize = SetFilePointer(hFile,0,NULL,FILE_END);
		INT64 dwBegRead = (nPage-1)*nSize;
		if(dwBegRead >= dwFSize)
		{
			CloseHandle(hFile);
			return;
		}
		
		char logbuf[MAX_LOG_LENGTH] = {0};
		memset(logbuf,0,sizeof(logbuf));
		char* pBeg = NULL;
		char* pEnd = NULL;
		bool bReadCon = true;
		
		INT64 dwRead = nSize;
		DWORD nbyte =0;
		bool bFind = true;
		std::string  strText ="";
		SetFilePointer(hFile,dwBegRead,NULL,FILE_BEGIN);
		while (::ReadFile(hFile, logbuf, MAX_LOG_LENGTH-1, &nbyte, NULL) && nbyte>0)
		{
			pBeg  = logbuf;
			BOOL bHasLine = FALSE;
			logbuf[nbyte] = '\0';
			if(bFind && dwBegRead != 0)
			{
				while (*pBeg != '\0')
				{	
					if(*pBeg == '\r' && *(pBeg+1) != '\n' || *pBeg == '\n' && *(pBeg+1) != '\n')
					{
						pBeg++;						
						break;
					}
					pBeg++;
				}
				bFind = false;
			}
			bFind = false;

			 if(dwRead > nbyte)
				 dwRead -= nbyte;
			 else
			 {
				pEnd = pBeg+dwRead;
				while(*pEnd)
				{
					if(*pEnd == '\r' && *(pEnd+1) != '\n' || *pEnd == '\n' && *(pEnd+1) != '\n')
					{
						*(pEnd+1) = '\0';
						bReadCon = false;
						break;
					}
					pEnd++;
				}
				dwRead = 0;
			 }
			 strText = pBeg;
			 int index=0;
			 while((index=strText.find("\r",index)) != -1)
				 strText.replace(index,1,"");
			 printf("%s",strText.c_str());

			 if(!bReadCon)
				 break;
			 memset(logbuf,0,sizeof(logbuf));
		}
		CloseHandle(hFile);
	}

}

void LogPageOpe::GetSCLogPage(INT64 nSize,DWORD nPage, const char* pFileName)
{
	if(nSize <= 0)
		return ;

	HANDLE hFile = CreateFileA(pFileName, GENERIC_READ,
							FILE_SHARE_READ | FILE_SHARE_WRITE| FILE_SHARE_DELETE,	NULL,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if(hFile == INVALID_HANDLE_VALUE)
	{
		printf("Open the file \"%s\" failed\n",pFileName);
		return;
	}
	INT64 nPosSize = GetLogPos(hFile);
	INT64 nFileSize = SetFilePointer(hFile,0,NULL,FILE_END);
	INT64 nBegRed = nSize*(nPage-1);
	if(nBegRed >= nFileSize)
	{
		CloseHandle(hFile);
		return;
	}
		
	nBegRed += nPosSize;
	if(nBegRed >= nFileSize)
		nBegRed -= nFileSize;
	nBegRed = nBegRed>CLOG_FILE_HEAD_LENGTH+2 ? nBegRed : CLOG_FILE_HEAD_LENGTH+2;
	char logbuf[MAX_LOG_LENGTH] = {0};
	memset(logbuf,0,sizeof(logbuf));
	char* pBeg = NULL;
	char* pEnd = NULL;
	bool bReadCon = true;

	INT64 dwRead = nSize;
	DWORD nbyte = 0;
	bool bFind = true;
	std::string strText = "";
	do{
		SetFilePointer(hFile,nBegRed,NULL,FILE_BEGIN);
		while (::ReadFile(hFile, logbuf, MAX_LOG_LENGTH-1, &nbyte, NULL) && nbyte>0)
		{
			pBeg  = logbuf;
			BOOL bHasLine = FALSE;
			logbuf[nbyte] = '\0';
			if(bFind && nBegRed != CLOG_FILE_HEAD_LENGTH+2 && nBegRed != nPosSize)
			{
				while (*pBeg != '\0')
				{	
					if(*pBeg == '\r' && *(pBeg+1) != '\n' || *pBeg == '\n' && *(pBeg+1) != '\n')
					{
						pBeg++;						
						break;
					}
					pBeg++;
				}
				bFind = false;
			}
			bFind = false;
			
			if(nBegRed < nPosSize && nBegRed + nbyte + CLOG_FILE_HEAD_LENGTH+2>= nPosSize)
				dwRead = nPosSize - nBegRed - CLOG_FILE_HEAD_LENGTH+2;
			else
				NULL;
			 if(dwRead > nbyte)
				 dwRead -= nbyte;
			 else
			 {
				pEnd = pBeg+dwRead;
				
				while(*pEnd)
				{	
					if(*(pEnd-1) == '\n')
					{
						*pEnd = '\0';
						break;
					}
					if(*pEnd == '\r' && *(pEnd+1) != '\n' || *pEnd == '\n' && *(pEnd+1) != '\n')
					{
						*(pEnd+1) = '\0';
						bReadCon = false;
						break;
					}
					pEnd++;
				}
				dwRead = 0;
			 }
			 strText = pBeg;
			 int index =0;
			 while((index=strText.find("\r",index)) != -1)
				 strText.replace(index,1,"");
			 printf("%s",strText.c_str());

			 if(!bReadCon)
				 break;
			 memset(logbuf,0,sizeof(logbuf));
			 nBegRed += nbyte;
		}
		if(nPosSize == nFileSize)
			break;
		nBegRed = CLOG_FILE_HEAD_LENGTH+2;
		
	}while(bReadCon);

	CloseHandle(hFile);
}

void LogPageOpe::Error()
{
	printf("Bad parameters!\n");
}

INT64 LogPageOpe::GetLogPos(HANDLE hFile)
{
	if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
		return 0;
	
	char buf[CLOG_FILE_HEAD_LENGTH+1] = "";
	DWORD nbyte =0;
	
	::SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	
	INT64 pos = 0;
	if (::ReadFile(hFile,&buf,CLOG_FILE_HEAD_LENGTH,&nbyte,NULL) &&  nbyte==CLOG_FILE_HEAD_LENGTH)
		pos = ::atol(buf);
	else pos = CLOG_FILE_HEAD_LENGTH;
	
	return pos;
}