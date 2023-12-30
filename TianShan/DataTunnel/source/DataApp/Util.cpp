// Util.cpp: implementation of the Util class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Util.h"
#include <list>
#include <string>
#include <vector>
::Ice::Identity createStreamIdentity(const std::string& space, 
									 const std::string& name)
{
	Ice::Identity ident;
	ident.category = space;
	ident.name = name;
	return ident;
}

::Ice::Identity createMuxItemIdentity(const std::string& space, 
									  const std::string& strmName, 
									  const std::string& itemName)
{
	Ice::Identity ident;
	ident.category = space;
	ident.name = itemName + "\\/" + strmName;
	return ident;
}

::Ice::ObjectPrx createObjectWithEndPoint(const Ice::CommunicatorPtr& ic, 
										  const Ice::Identity& ident, 
										  const std::string& endPoint)
{
	std::string proxy;
	proxy = ic->identityToString(ident);
	proxy += ":";
	proxy += endPoint;

	return ic->stringToProxy(proxy);
}
int  ListFile(const char *argv, std::list<std::string> &File)
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	char DirSpec[MAX_PATH];  // directory specification
	DWORD dwError;
	
	//printf ("Target directory is %s.\n", argv);
	strncpy (DirSpec, argv, strlen(argv)+1);
	strncat (DirSpec, "\\*", 3);
	
	hFind = FindFirstFileA(DirSpec, &FindFileData);

	if (hFind == INVALID_HANDLE_VALUE) 
	{
		glog( ZQ::common::Log::L_ERROR,"ListFile::Invalid \
			file handle. Error is %u",GetLastError());		 
		return -1;
	} 
	else 
	{
		//printf ("First file name is %s\n", FindFileData.cFileName);
		if ( FindFileData.dwFileAttributes == 32 )
		{
			strcpy(DirSpec,  argv);   
            strcat(DirSpec,  "\\");
			strcat(DirSpec, FindFileData.cFileName);
			File.push_back(DirSpec);
		}

		while (FindNextFileA(hFind, &FindFileData) != 0) 
		{
			//printf ("Next file name is %s\n", FindFileData.cFileName);
			if ( FindFileData.dwFileAttributes == 32 )
			{
				strcpy(DirSpec,  argv);   
				strcat(DirSpec,  "\\");
				strcat(DirSpec, FindFileData.cFileName);
				File.push_back(DirSpec);
			}
		}
		
		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES) 
		{
			glog( ZQ::common::Log::L_ERROR,"ListFile::\
				FindNextFile error. Error is %u",  GetLastError());		 
			return -1;
		}
	}
	return 0;
}
bool ListDir(const char *lpSrcFile, std::list<std::string> &Directory) 
{   
	if( FILE_ATTRIBUTE_DIRECTORY == ::GetFileAttributesA(lpSrcFile))     
	{   
		WIN32_FIND_DATAA   FindFileData;   
		char   FileName[_MAX_PATH];   

		strcpy(   FileName   ,   lpSrcFile   );   
		strcat(   FileName   ,   "\\*.*"   );   
		
		HANDLE hFindFile = ::FindFirstFileA( FileName, &FindFileData);   
		if ( INVALID_HANDLE_VALUE == hFindFile )   
		{   
			return FALSE;   
		}   
		//CString strSrcPathName;   
		while( ::FindNextFileA( hFindFile, &FindFileData ) )   
		{   
			if(!( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))//不是目录   
			{   
				// CString   strSrcPathName(   lpSrcFile   );   
				// strSrcPathName+=_T("\\");   
				// strSrcPathName+=   FindFileData.cFileName;   
				//printf("%s\n", FindFileData.cFileName);
                
			}   
			else//目录   
			{   
				//记录下目录
				if ( strcmp(FindFileData.cFileName, ".") !=0 && strcmp(FindFileData.cFileName, "..") != 0 )
				{
					//printf("%s\n", FindFileData.cFileName);
					strcpy(FileName,  lpSrcFile);   
		            strcat(FileName,  "\\");
					strcat(FileName, FindFileData.cFileName);
					Directory.push_back(FileName);
					ListDir(FileName ,  Directory);
				}
			}   
		}   
		
		::FindClose(   hFindFile   );   
		
		if ( ERROR_NO_MORE_FILES != GetLastError() )   
			return   FALSE;   
	}   
	return   1;   
}  
bool DeleteDirectory(std::string DeleteDir,bool IsDel)
{
	std::list<std::string> Directory;
	std::list<std::string>::iterator iterDirectory;
	std::list<std::string> File;
	std::list<std::string>::iterator iterFile;
	
	Directory.push_front(DeleteDir);
	
	ListDir(DeleteDir.c_str(), Directory);
//    DirectoryDel = Directory;

 	for (iterDirectory = Directory.begin(); iterDirectory != Directory.end(); 
	                                                         ++iterDirectory)
	{
	  if(ListFile( (*iterDirectory).c_str(), File ))
		  return false;
	}
	
	for (iterFile = File.begin(); iterFile != File.end(); ++iterFile)
	{
		if (DeleteFileA( (*iterFile).c_str()) == 0)
			return false;
	}

	if(!IsDel)
	{
		Directory.pop_front();
	}

	std::list<std::string>::reverse_iterator iter;
	for (iter = Directory.rbegin(); iter != Directory.rend(); iter++)
	{
		if (RemoveDirectoryA( (*iter).c_str() ) == 0)
			return false;
	}
	return true;
}
bool CopyAllFile(std::string DeleteDir, std::string srcPath)
{
	std::list<std::string> Directory;
	std::list<std::string>::iterator iterDirectory;

	std::list<std::string> File;
	std::list<std::string>::iterator iterFile;

	Directory.push_back(DeleteDir);
	ListDir(DeleteDir.c_str(), Directory);

	std::string tempdest;
	std::string strtemp;
 	for (iterDirectory = Directory.begin(); iterDirectory != Directory.end(); ++iterDirectory)
	{
		
  		if(ListFile( (*iterDirectory).c_str(), File ))
			return false;
		//*iterDirectory = srcPath + (*iterDirectory).substr(2);
		*iterDirectory = (*iterDirectory).replace(0, DeleteDir.length(), srcPath.c_str());
        strtemp = *iterDirectory;
		if ( *iterDirectory != srcPath )
		{
			if (  ( (_access ( (*iterDirectory).c_str(), 0 )) != 0 ) )
			{
				if (!CreateDirectoryA((*iterDirectory).c_str(), NULL))
					{
						return false;
					} 
			}
		}
	}
	std::list<std::string> CopyedFile(File);
	std::list<std::string>::iterator iterCopyedFile;

	for (iterCopyedFile = CopyedFile.begin(); iterCopyedFile != CopyedFile.end(); ++iterCopyedFile )
	{
		tempdest = *iterCopyedFile;
		BOOL bresult = CopyFileA(tempdest.c_str(), (*iterCopyedFile).replace(0, DeleteDir.length(), srcPath.c_str()).c_str(), false);
		if ( bresult == 0 )
		{

			return false;
		}
	}
	return true;
}
