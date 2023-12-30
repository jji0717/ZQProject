// versioncheck.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <iomanip.h>
#include <string>
#include <list>

#pragma warning (disable: 4786)
//#include "stringoperation.h"
#define FILE_NAME_LENGTH 30
#define FILE_VERSION_LENGTH 20
#define FILE_CREATE_TIME_LENGTH 30
#define FILE_SIZE_LENGTH 20
#pragma comment(lib,"version")

//  The function is called to get version of a file specified
//  by filename, if success it return true, and the version is
//  returned in ffi. if not success, it return false, means the
//  file does not have version.
bool GetFileVersion(LPTSTR filename,VS_FIXEDFILEINFO *ffi) 
{
    DWORD retBytes,dwHandle;
	char *verBuff;
	retBytes = ::GetFileVersionInfoSize(filename,&dwHandle);
	if(retBytes <= 0)
		return false;

	verBuff = new char[retBytes];

	if(!::GetFileVersionInfo(filename,dwHandle,retBytes,verBuff))
	{
		delete []verBuff;
		return false;
	}

	UINT uLen;
	LPVOID tempBuff;
	if(!::VerQueryValue(verBuff,"\\",&tempBuff,&uLen))
	{
		delete []verBuff;
		return false;
	}

	memcpy(ffi,tempBuff,sizeof(VS_FIXEDFILEINFO));

	delete []verBuff;

	return true;
}

const char * GetRightStr(const char *str,UINT nLen)
{
	char *retChars = new char[nLen+1];
	int strLen = strlen(str);
	int i,j;
	for(i=strLen-4,j=0;i<strLen;i++,j++)
	{
		retChars[j] = str[i];
	}
	retChars[j] = '\0';
	return retChars;
}

void ReplaceChar(LPTSTR inStr,TCHAR from,TCHAR to)
{
	int nLen = strlen(inStr);
	for(int i=0;i<nLen;i++)
	{
		if(inStr[i] == from)
			inStr[i] = to;
	}
}

void WriteFileHeadInfo(ofstream &os,bool bShowOnScreen)
{
	if(bShowOnScreen)
	{
		cout.setf(ios::left);
		cout<<setw(FILE_NAME_LENGTH)<<"FILE_NAME"<<" ";
		cout<<setw(FILE_VERSION_LENGTH)<<"FILE_VERSION";
		cout<<setw(FILE_CREATE_TIME_LENGTH)<<"FILE_CREATE_TIME";
		cout<<setw(FILE_SIZE_LENGTH)<<"FILE_SIZE"<<endl;
	}
	else
	{
		os<<setw(FILE_NAME_LENGTH)<<"FILE_NAME"<<" ";
		os<<setw(FILE_VERSION_LENGTH)<<"FILE_VERSION";
		os<<setw(FILE_CREATE_TIME_LENGTH)<<"FILE_CREATE_TIME";
		os<<setw(FILE_SIZE_LENGTH)<<"FILE_SIZE"<<endl;
	}
}

bool ReadFileHeadInfo(ifstream &is)
{
	int i;
	bool bRet = true;
	char strArray[4][MAX_PATH];
	char strArrayConst[4][20] = {"FILE_NAME","FILE_VERSION","FILE_CREATE_TIME","FILE_SIZE"};
	memset(strArray,0,4*MAX_PATH);
	for(i=0;i<4;i++)
	{
		is>>strArray[i];
	}
	for(i=0;i<4;i++)
	{
		if(strcmp(strArray[i],strArrayConst[i]) != 0)
		{
			bRet = false;
			break;
		}
	}
	return bRet;
}

//  Check files under the special directory according to
//  a data file, To see if there was any file losed or 
//  file version changed etc. namely, if any change have 
//  happened it will report them.
bool CheckFileVersion(char *ddir)
{
	bool bRet = true;

	//check if lpszPath and ddir exist.
	HANDLE hdl;
	WIN32_FIND_DATA dt;

	hdl = ::FindFirstFile(ddir,&dt);
	if(hdl == INVALID_HANDLE_VALUE)
	{
		cout<<"File "<<ddir<<" does not exist, it must be a existed file."<<endl;
		return false;
	}
	::FindClose(hdl);

	//create a ifstream object and open it.
    ifstream ifile;
	ifile.open(ddir,ios::in);//open in binary mode.
	if(!ifile)
	{
		cout<<"File "<<ddir<<" can't open."<<endl;
		return false;
	}

	char filename[MAX_PATH];
	char version[FILE_VERSION_LENGTH],date[FILE_CREATE_TIME_LENGTH];
	char date1[FILE_CREATE_TIME_LENGTH],date2[FILE_CREATE_TIME_LENGTH];
	unsigned int filesize;

	char chdir[MAX_PATH] = {0};

	while(!ifile.eof())
	{
		memset(filename,0,MAX_PATH);
		ifile>>filename;            //read file name
		if(strcmp(filename,"") == 0)
			continue;

		ReplaceChar(filename,'>',' ');
		if(filename[0] == '.' || filename[1] == ':')//it is a directory
		{
			memset(chdir, 0, sizeof(chdir));
			strcpy(chdir,filename);

			HANDLE hdl;
			WIN32_FIND_DATA dt;
			hdl = ::FindFirstFile(chdir,&dt);
			if(hdl == INVALID_HANDLE_VALUE)
			{
				cout<<chdir<<" is a invalid directory."<<endl;
				return false;
			}
			::FindClose(hdl);
			
			//check file data to see if invalid.
			if(!ReadFileHeadInfo(ifile))
			{
				cout<<ddir<<" file head info is not a valid data."<<endl;
				return false;
			}
			continue;
		}
		
		ifile>>version;             //read version infomation
		ifile>>date1>>date2;        //read time created
		strcpy(date,date1);
		strcat(date," ");
		strcat(date,date2);
		ifile>>filesize;            //read file size.
		TCHAR fullname[MAX_PATH] = {0};
		strcpy(fullname,chdir);
		strcat(fullname,"\\");
		strcat(fullname,filename);

		//searching the file and storing the infomation to a WIN32_FIND_DATA object.
		HANDLE hHandle;
		WIN32_FIND_DATA finddata;
		hHandle = ::FindFirstFile(fullname,&finddata);
		if(hHandle == INVALID_HANDLE_VALUE)
		{
			//if the program goes this way means there is a file losed.
			if(strcmp(filename,"") != 0)
			{
				bRet = false;
			    cout<<fullname<<" is losed."<<endl;
			}
		}
		else
		{
			//check file information.
			SYSTEMTIME systime;
			FileTimeToSystemTime(&finddata.ftCreationTime,&systime);

			//get version info.
			VS_FIXEDFILEINFO pvsf;
			char tVersion[MAX_PATH];
			if(GetFileVersion(fullname,&pvsf))
			{
				sprintf(tVersion,"%d.%d.%d.%d",HIWORD(pvsf.dwFileVersionMS),
					LOWORD(pvsf.dwFileVersionMS),HIWORD(pvsf.dwFileVersionLS),
					LOWORD(pvsf.dwFileVersionLS));
			}
			else
			{
				sprintf(tVersion,"No Version Info");
			}

			//get date when the file created.
			char tDate[30];
			sprintf(tDate,"%04d-%02d-%02d %02d:%02d:%02d",systime.wYear,
				systime.wMonth,systime.wDay,systime.wHour,
				systime.wMinute,systime.wSecond);
			
			//get the file size.
			unsigned int tFileSize;
			tFileSize = finddata.nFileSizeHigh * MAXDWORD + finddata.nFileSizeHigh
				+ finddata.nFileSizeLow;

			if(strcmp(version,tVersion) != 0)
			{
				bRet = false;
				cout<<fullname<<"'s version changed. From "<<version<<" to "<<tVersion<<endl;
			}

			if(strcmp(date,tDate) != 0)
			{
				bRet = false;
				cout<<fullname<<"'s create time changed. From "<<date<<" to "<<tDate<<endl;
			}

			if(filesize != tFileSize)
			{
				bRet = false;
				cout<<fullname<<"'s size changed. From "<<filesize<<" to "<<tFileSize<<endl;
			}

		}
		FindClose(hHandle);
	}
	ifile.close();

	return bRet;
}

//  The function is used to get a path from a filename
//  which has path.
//  e.g. fullname: c:\windows\system32\seachange.dll
//           path: c:\windows\system32\
//  it does not return any value.
void GetPathName(char *pathname,char *lpszPath)
{
	int nLen = strlen(lpszPath);
	int i,j;
	for(i=nLen-1;i>=0;i--)
	{
		if(lpszPath[i] == '\\')
			break;
	}
	for(j=0;j<=i;j++)
	{
		pathname[j] = lpszPath[j];
	}
	pathname[j] = '\0';
}

//  Output information of files under special directory.
//  you can print the information on screen or write to
//  a data file so that you can use it in the future. 
bool OutputFileInfo(const char *lpszPath,char *ddir,int index=1)
{	
	//if show on screen.
	bool bShowOnScreen = false;
	if(strcmp(ddir,"") == 0)
		bShowOnScreen = true;

	WIN32_FIND_DATA filedata; 
	HANDLE hHandle;
	TCHAR sdir[MAX_PATH];
	memset(sdir,0,MAX_PATH);

	char curpath[MAX_PATH] = {0};
	DWORD re = GetCurrentDirectory(sizeof(curpath),curpath);

	char* ppath = NULL;
	if(lpszPath[0] == '.')
	{
		int i = 1;
		if(lpszPath[1] == '.')
		{
			i = 2;
			ppath = strrchr(curpath,'\\');
			*ppath = '\0';
		}
		strcpy(sdir,curpath);
		strcat(sdir,lpszPath+i);
	}
	else if((ppath = strrchr(lpszPath,'\\')) == NULL)
	{
		strcpy(sdir,curpath);
		strcat(sdir,"\\");
		strcat(sdir,lpszPath);
	}
	else
		strcpy(sdir,lpszPath);

	bool ispath=false,ispathwith=false;
	if(strcmp(GetRightStr(lpszPath,4),"\\*.*") != 0)
	{
		if(lpszPath[strlen(lpszPath)-1] == '\\')
		{
			strcat(sdir,"*.*");
		}
		else
		{
			hHandle = ::FindFirstFile(lpszPath,&filedata);
			if(hHandle != INVALID_HANDLE_VALUE)
			{
				if((filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				{ 
					strcat(sdir,"\\*.*");
				}
			}
			::FindClose(hHandle);
		}
	}
	
	hHandle = ::FindFirstFile(sdir,&filedata);
	if(hHandle == INVALID_HANDLE_VALUE)
	{
		cout<<lpszPath<<" is not a file or directory."<<endl;
		return false;
	}

	//define a output file object.
	ofstream ofile;
	if(!bShowOnScreen)
	{
		if(index == 1)
			ofile.open(ddir,ios::out);
		else
			ofile.open(ddir,ios::app);
		if(!ofile.is_open())
		{
			cout<<"File "<<ddir<<" can't be created."<<endl;
			return false;
		}
		
		ofile.setf(ios::left);
	}

	//write directory
	char chdir[MAX_PATH] = {0};
	strcpy(chdir, sdir);
	char* pS = strrchr(chdir,'\\');
	if(pS != NULL)
		*pS = '\0';
	else
		return false;

	if(!bShowOnScreen)
		ofile<<chdir<<endl;
	else
		cout<<chdir<<endl;
	
	//write header
	WriteFileHeadInfo(ofile,bShowOnScreen);

	do
	{
		//get file's short name from long file with path.
		TCHAR shortname[MAX_PATH];
		TCHAR fullname[MAX_PATH];
		TCHAR pathname[MAX_PATH];
		memset(pathname,0,MAX_PATH);
		memset(shortname,0,MAX_PATH);
		memset(fullname,0,MAX_PATH);
		GetPathName(pathname,sdir);
		strcpy(shortname,filedata.cFileName);
	    strcpy(fullname,pathname);
		strcat(fullname,filedata.cFileName);
		/// if the file name has white space char ' ', it will be replaced to '>'.
		ReplaceChar(shortname,' ','>');

		//get version information.
		VS_FIXEDFILEINFO ffi;

		//ignore the files that don't have version info.
		if(!GetFileVersion(fullname,&ffi))
			continue;

		//change filetime to systime.
		SYSTEMTIME systime;
		FileTimeToSystemTime(&filedata.ftCreationTime,&systime);
		char version[FILE_VERSION_LENGTH];
		sprintf(version,"%d.%d.%d.%d",HIWORD(ffi.dwFileVersionMS),
			LOWORD(ffi.dwFileVersionMS),HIWORD(ffi.dwFileVersionLS),
			LOWORD(ffi.dwFileVersionLS));
		char date[FILE_CREATE_TIME_LENGTH];
		sprintf(date,"%04d-%02d-%02d %02d:%02d:%02d",systime.wYear,
			systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond);
		unsigned int filesize;
		filesize = (filedata.nFileSizeHigh * (MAXDWORD + 1)) + filedata.nFileSizeLow;
		if(!bShowOnScreen)
		{
		    ofile<<setw(FILE_NAME_LENGTH)<<shortname<<" ";
			ofile<<setw(FILE_VERSION_LENGTH)<<version;
		    ofile<<setw(FILE_CREATE_TIME_LENGTH)<<date;
		    ofile<<filesize<<endl;
		}
		else
		{
			cout.setf(ios::left);
			cout<<setw(FILE_NAME_LENGTH)<<shortname<<" ";
			cout<<setw(FILE_VERSION_LENGTH)<<version;
			cout<<setw(FILE_CREATE_TIME_LENGTH)<<date;
			cout<<filesize<<endl;
		}

	}while(::FindNextFile(hHandle,&filedata));

	//¹Ø±ÕÎÄ¼þ
	FindClose(hHandle);
	ofile<<endl;
	ofile.close();

	return true;
}

void ShowHelp()
{
	cout<<"------------------------------------------------------------------------------"<<endl;
	cout<<"How to run applition: vercheck"<<endl;
	cout<<"------------------------------------------------------------------------------"<<endl;
	cout<<"a)      vercheck [-help]"<<endl;
	cout<<"b)      vercheck <fileName fileName2 ... | directory directory2 ...> [-out file]"<<endl;
	cout<<"c)      vercheck <-check file>"<<endl<<endl;
	cout<<"fileName         get version info of the specified files"<<endl;
	cout<<"directory        get version info under the specified directorys"<<endl;
	cout<<"-help            show help info"<<endl;
	cout<<"-out file        write version info to the file,if not set this parameter will put the version info to screen"<<endl;
	cout<<"-check file      check the version info from the file content"<<endl<<endl;
	cout<<"e.g.    vercheck c:\\windows c:\\TianShan\\bin"<<endl;
	cout<<"e.g.    vercheck c:\\work\\build.exe c:\\windows -out d:\\vercheck.txt"<<endl;
	cout<<"e.g.    vercheck -check d:\\vercheck.txt"<<endl;
}

int main(int argc, char* argv[])
{
	if(argc<2 || stricmp(argv[1],"-help") == 0)
	{
		ShowHelp();
		return 0;
	}

	bool bcheck = false;
	char filename[MAX_PATH] = {0};
	std::list<std::string> pathName;
	for(int i = 1; i < argc; i++)
	{
		if(stricmp(argv[i],"-help") == 0)
		{
			ShowHelp();
			return 0;
		}
		else if(stricmp(argv[i], "-check") == 0)
		{
			bcheck = true;
			if(++i >= argc)
			{
				ShowHelp();
				return 0;
			}

			strcpy(filename,argv[i]);
			break;
		}
		else if(stricmp(argv[i],"-out") == 0)
		{
			if(++i >= argc)
			{
				ShowHelp();
				return 0;
			}
			strcpy(filename,argv[i]);
			break;
		}
		else
		{
			pathName.push_back(argv[i]);
		}
	}
	if(bcheck)
	{
		if(CheckFileVersion(filename))
		{
		    cout<<"File Check Over,There is no change."<<endl;
		    cout<<"Type (versioncheck -help) then you can get help."<<endl;
			    return 1;
		}
		else
			return 0;
	}

	if(pathName.size() == 0)
	{
		ShowHelp();
		return 0;
	}
	
	std::list<std::string>::iterator it;
	int nindex = 1;
	for(it=pathName.begin(); it != pathName.end(); it++)
	{
		if(OutputFileInfo((*it).c_str(), filename, nindex))
		{
			nindex++;
			if(strlen(filename) != 0)
				cout<<"Save "<<(*it).c_str()<<" file version Infomation Success."<<endl;
		}
		
		cout<<endl;
	}

	return 1;
}

