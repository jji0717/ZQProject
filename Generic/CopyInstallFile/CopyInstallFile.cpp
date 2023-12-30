#include "windows.h"
#include "stdio.h"
#ifndef MAXPATH
#define MAXPATH	260	
#endif

#pragma  comment(lib,"Version.lib")

void EmitErrorMsg (HRESULT hr);
HRESULT GetFileVersion (char *filename, VS_FIXEDFILEINFO *vsf);
HRESULT GetFileDate (char *filename, FILETIME *pft);
HRESULT LastError();
int WhichIsNewer (char *fname1, char *fname2);
void ShowUsage(void);

void ShowUsage(void)
{
	printf("\t\tThis tool is to install file to destine path.\n");
	printf("\t\tAnd it will check the version of file and give out report.\n");
	printf("\t\tUsage:");	
	printf("\t\t\tCopyInstallFile -h or -?\t\t:Show this screen.\n");
	printf("\t\t\tCopyInstallFile [FileName] [InstallFileName]\n");
	printf("\n\n");	
}
void printtime (FILETIME *t) {

   FILETIME lft;
   FILETIME *ft = &lft;
   FileTimeToLocalFileTime(t,ft);
   printf("%08x %08x",ft->dwHighDateTime,ft->dwLowDateTime); {
      SYSTEMTIME stCreate;
      BOOL bret = FileTimeToSystemTime(ft,&stCreate);
      printf("    %02d/%02d/%d  %02d:%02d:%02d\n",
      stCreate.wMonth, stCreate.wDay, stCreate.wYear,
      stCreate.wHour, stCreate.wMinute, stCreate.wSecond);
   }
}



int WhichIsNewer (char *fname1, char *fname2) {
   // 1 if argv[1] is newer
   // 2 if argv[2] is newer
   // 3 if they are the same version
   // 0 if there is an error

   int ndxNewerFile;
   HRESULT ret;
   VS_FIXEDFILEINFO vsf1,vsf2;

   if ( SUCCEEDED((ret=GetFileVersion(fname1,&vsf1))) && SUCCEEDED((ret=GetFileVersion(fname2,&vsf2)))) {
      // both files have a file version resource
      // compare by file version resource
      if (vsf1.dwFileVersionMS > vsf2.dwFileVersionMS) {
         ndxNewerFile = 1;
      }
      else 
         if (vsf1.dwFileVersionMS < vsf2.dwFileVersionMS) {
            ndxNewerFile = 2;
         }
         else {   // if (vsf1.dwFileVersionMS == vsf2.dwFileVersionMS)
            if (vsf1.dwFileVersionLS > vsf2.dwFileVersionLS) {
               ndxNewerFile = 1;
            }
            else if (vsf1.dwFileVersionLS < vsf2.dwFileVersionLS) {
               ndxNewerFile = 2;
            }
            else {   // if (vsf1.dwFileVersionLS == vsf2.dwFileVersionLS)
               ndxNewerFile = 3;
            }
         }
   }

   else {
      // compare by date
      FILETIME ft1,ft2;
      if (SUCCEEDED((ret=GetFileDate(fname1,&ft1))) && SUCCEEDED((ret=GetFileDate(fname2,&ft2))))
      {
         LONG x = CompareFileTime(&ft1,&ft2);
         if (x == -1) 
            ndxNewerFile = 2;
         else 
            if (x == 0) 
               ndxNewerFile = 3;
            else 
               if (x == 1) ndxNewerFile = 1;
               else {
                  EmitErrorMsg(E_FAIL);
                  return 0;
               }
      }
      else {
         EmitErrorMsg(ret);
         return 0;
      }
   }
   return ndxNewerFile;
}

HRESULT GetFileDate (char *filename, FILETIME *pft) {
   // we are interested only in the create time
   // this is the equiv of "modified time" in the 
   // Windows Explorer properties dialog
   FILETIME ct,lat;
   HANDLE hFile = CreateFile(filename,GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0);
   if (hFile == INVALID_HANDLE_VALUE) 
      return LastError();
   BOOL bret = GetFileTime(hFile,&ct,&lat,pft);
   if (bret == 0) 
      return LastError();
   return S_OK;
}

// This function gets the file version info structure
HRESULT GetFileVersion (char *filename, VS_FIXEDFILEINFO *pvsf) {
   DWORD dwHandle;
   DWORD cchver = GetFileVersionInfoSize(filename,&dwHandle);
   if (cchver == 0) 
      return LastError();
   char* pver = new char[cchver];
   BOOL bret = GetFileVersionInfo(filename,dwHandle,cchver,pver);
   if (!bret) 
      return LastError();
   UINT uLen;
   void *pbuf;
   bret = VerQueryValue(pver,"\\",&pbuf,&uLen);
   if (!bret) 
      return LastError();
   memcpy(pvsf,pbuf,sizeof(VS_FIXEDFILEINFO));
   delete[] pver;
   return S_OK;
}

HRESULT LastError () {
   HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
   if (SUCCEEDED(hr)) 
      return E_FAIL;
   return hr;
}

// This little function emits an error message based on WIN32 error messages
void EmitErrorMsg (HRESULT hr) {
   char szMsg[1024];
   FormatMessage( 
      FORMAT_MESSAGE_FROM_SYSTEM, 
      NULL,
      hr,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      szMsg,
      1024,
      NULL 
      );
        printf("%s\n",szMsg);
}
int main(int argc,char* argv[])
{
	if (argc != 3)
	{
		if((strcmp(argv[1],"-h")==0)||(strcmp(argv[1],"-?")==0)||(strcmp(argv[1],"-h")==0))
		{
			ShowUsage();
		}
		else
			printf("Invalid number of parameter. -h or -? to show usage.\n");
		return 1;
	}
	//	get parameter
	char sNewFileName[MAXPATH];
	char sInstallFileName[MAXPATH];
	strcpy(sNewFileName,argv[1]);
	strcpy(sInstallFileName,argv[2]);
	// parse the FileName
	char sFileName[MAXPATH];
	int nLen = strlen(sNewFileName);
	int nPos = nLen;
	while(nPos)
	{
		if((sNewFileName[nPos-1] == '\\')||(sNewFileName[nPos-1] == '/'))
		{
			strcpy(sFileName,sNewFileName+nPos);
			break;
		}
		nPos --;
	}
	if(nPos == 0)
	{
		strcpy(sNewFileName,sFileName);
	}
	// check if the file exist in destine 
	WIN32_FIND_DATA FindFileData;
	bool bFileFind = false;
	HANDLE hFile = ::FindFirstFile(sInstallFileName,&FindFileData);
	if(hFile != INVALID_HANDLE_VALUE)
		bFileFind = true;
	::FindClose(hFile);
	// get version information of original file and package file

	if(bFileFind)//find file
	{		
		int nRtn = WhichIsNewer(sNewFileName,sInstallFileName);
		switch(nRtn)
		{
		case	0:			
			break;
		case	1:
			::SetFileAttributes(sInstallFileName,FILE_ATTRIBUTE_NORMAL);
			if(::CopyFile(sNewFileName,sInstallFileName,FALSE) != FALSE)
			{
				printf("Updated %-20s:%s\n",sFileName,"Copy File Success.");
			}else
			{
				printf("Skipped %-20s:%s\n",sFileName,"Failed.");
			}
			break;
		case	2:			
			printf("Skipped %-20s:%s\n",sFileName,"Newer version is already installed.");
			break;
		case	3:
			printf("Skipped %-20s:%s\n",sFileName,"This version is already installed.");
			break;
		default	:
			break;
		}
	}
	else
	{			
		if(::CopyFile(sNewFileName,sInstallFileName,FALSE) != FALSE)
		{
			printf("Installed %-20s:%s\n",sFileName,"Success.");
		}else
		{
			printf("Skipped %-20s:%s\n",sFileName,"Failed.");
		}		

	}
	return 0;
}









































