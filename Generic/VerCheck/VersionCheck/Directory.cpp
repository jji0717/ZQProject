//////////////////////////////////////////////////////////////////////////
// Module: Directory.cpp
// Author: Han Guan
//////////////////////////////////////////////////////////////////////////

#include "Directory.h"
#include "File.h"
#include <iostream>
#include <iomanip>
using  namespace std;


//特殊用途
std::string GetPathString(const ::std::string& fullName)
{
	char pathName[MAX_PATH];
	memset(pathName,0,MAX_PATH);
	int strLen = fullName.size();
	int cur;
	for(cur = strLen-1;cur >= 0; cur--)
	{
		if(fullName[cur] == '\\' || fullName[cur] == '/')
			break;
	}
	if(cur < 0)
	{
		return pathName;
	}
	else
	{
		int tempCur=0;
		while(tempCur<cur)
		{
			pathName[tempCur] = fullName[tempCur];
			tempCur++;
		}
	}
	return pathName;
}
// 获得conString中右边第一个vChar后面的字符串，若没有vChar则返回空字符串。
std::string GetStringRightAtChar(const ::std::string& contString,char vChar)
{
	char strTemp[MAX_PATH];
	memset(strTemp,0,MAX_PATH);
	int strLen = contString.size();
	int cur;
	for(cur = strLen-1;cur >= 0;cur--)
	{
		if(contString[cur] == vChar)
			break;
	}
	if(cur < 0)
	{
		return strTemp;
	}
	else
	{
		cur++;
		int i=0;
		while(cur<strLen)
		{
			strTemp[i] = contString[cur];
			i++;
			cur++;
		}
		strTemp[i] = '\0';
		return strTemp;
	}
}

// 获得contString中在第一个vChar左边的字符串, 若没有vChar则返回空字符串.
std::string GetStringLeftAtChar(const ::std::string& contString,char vChar)
{
	char strTemp[MAX_PATH];
	memset(strTemp,0,MAX_PATH);
	int strLen = contString.size();
	int cur;
	for(cur = 0;cur < strLen;cur++)
	{
		if(contString[cur] == vChar)
			break;
	}
	if(cur >= strLen)
	{
		return strTemp;
	}
	else
	{
		int tempCur=0;
		while(tempCur<cur)
		{
			strTemp[tempCur] = contString[tempCur];
			tempCur++;
		}
		strTemp[tempCur] = '\0';
		return strTemp;
	}
}

std::string ReplaceChar(std::string& str,char from,char to)
{
	int nLen = str.size();
	for(int i=0; i<nLen; i++)
	{
		if(str[i] == from)
			str[i] = to;
	}
	return str;
}

Directory::Directory()
{
}

Directory::Directory(std::string directory,std::string query) : bSubDirectoryScaned(false),m_query(query),m_directory(directory)
{
}

Directory::Directory(std::string userString) : bSubDirectoryScaned(false)
{
	preParse(userString,m_directory,m_query);
}

Directory::~Directory()
{
	free();
}

void Directory::preParse(std::string name,std::string& directory,std::string& query)
{
	if(!hasWildCard(name))
	{
		if(isDirectoryExist(name))
		{
			if(name[name.size()-1] == '\\')
			{
				name[name.size()-1] = '\0';
				directory = name;
			}
			else
			{
				directory = name;
			}
			query = "\\*.*";
		}
		else
		{
			directory = GetPathString(name);
			query = "\\" + GetStringRightAtChar(name,'\\');
		}
	}
	else
	{
		if(isValid(name))
		{
			directory = GetPathString(name);
			query = "\\" + GetStringRightAtChar(name,'\\');
		}
		else
		{
			throw exception("Invalid input string");
		}
	}
}

void Directory::parse(bool bSub)
{
	m_vectorDirectory.clear();
	m_vectorFile.clear();
	WIN32_FIND_DATA findData;
	HANDLE hHandle;
	char query[MAX_PATH];
	sprintf(query,"%s%s",m_directory.c_str(),m_query.c_str());
	hHandle = ::FindFirstFile (query,&findData);
	if (hHandle != INVALID_HANDLE_VALUE)
	{
		do {
			if ( (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( strcmp(findData.cFileName,".") != 0 && strcmp(findData.cFileName,"..") != 0 )
				{
					// DO: 标识子目录已经扫描过了
					bSubDirectoryScaned = true;
					char directory[MAX_PATH];
					sprintf(directory,"%s\\%s",m_directory.c_str(),findData.cFileName);
					Directory* newDirectory = new Directory(directory,m_query);
					if (bSub)
					{
						newDirectory->parse(true);
					}
					m_vectorDirectory.push_back(newDirectory);
				}
			}
			else
			{
				File* newFile = new File(this,&findData);
				m_vectorFile.push_back(newFile);
			}
		} while(::FindNextFile(hHandle,&findData));
	}

	// DO: 如果还没有扫描子目录，查找所有子目录
	if(!bSubDirectoryScaned)
	{
		sprintf(query,"%s\\*.*",m_directory.c_str());
		hHandle = ::FindFirstFile (query,&findData);
		if (hHandle == INVALID_HANDLE_VALUE)
			return;
		do {
			if ( (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
			{
				if ( strcmp(findData.cFileName,".") != 0 && strcmp(findData.cFileName,"..") != 0 )
				{
					char directory[MAX_PATH];
					sprintf(directory,"%s\\%s",m_directory.c_str(),findData.cFileName);
					Directory* newDirectory = new Directory(directory,m_query);
					if (bSub)
					{
						newDirectory->parse(true);
					}
					m_vectorDirectory.push_back(newDirectory);
				}
			}
		} while(::FindNextFile(hHandle,&findData));
	}
}

void Directory::free()
{
	// 释放所有文件占用的内存
	vector_f_itor f_itor;
	for(f_itor=m_vectorFile.begin(); f_itor!=m_vectorFile.end(); f_itor++)
	{
		if(*f_itor != NULL)
		{
			delete (*f_itor);
			*f_itor = NULL;
		}
	}
	m_vectorFile.clear();

	vector_d_itor d_itor;
	for(d_itor=m_vectorDirectory.begin(); d_itor!=m_vectorDirectory.end(); d_itor++)
	{
		if(*d_itor != NULL)
		{
			(*d_itor)->free();
			*d_itor = NULL;
		}
	}
	m_vectorDirectory.clear();
}

void Directory::setDirectory(std::string name)
{
	free();
	m_directory = name;
}

void Directory::print(bool bSub)
{
	cout<<getFullName()<<endl;
	vector_f_itor f_itor;
	for(f_itor=m_vectorFile.begin(); f_itor!=m_vectorFile.end(); f_itor++)
	{
		if(*f_itor != NULL)
			cout<<setw(30)<<(*f_itor)->getName()<<" "<<setw(10)<<(*f_itor)->getSize()<<" "<<setw(20)<<(*f_itor)->getCreateTime()<<" "<<setw(16)<<(*f_itor)->getVersion()<<endl;
	}
	vector_d_itor d_itor;
	for(d_itor=m_vectorDirectory.begin(); d_itor!=m_vectorDirectory.end(); d_itor++)
	{
		if(*d_itor != NULL)
		{
			if(bSub)
				(*d_itor)->print(true);
			else
				cout<<(*d_itor)->getFullName()<<endl;
		}
	}
}

void Directory::output(std::string name,bool bSub)
{
	ofstream of;
	of.open(name.c_str(),ios::out);
	if(of.is_open())
	{
		output(of,bSub);
		of.close();
	}
}

void Directory::output(ostream of,bool bSub)
{
	of<<setw(3)<<"D "<<ReplaceChar(getFullName(),' ','*')<<endl;
	vector_f_itor f_itor;
	for(f_itor=m_vectorFile.begin(); f_itor!=m_vectorFile.end(); f_itor++)
	{
		if(*f_itor != NULL)
		{
			std::string sVersion = (*f_itor)->getVersion();
			if(sVersion.size())
			{
				of<<setw(3)<<"FV ";
			}
			else
			{
				of<<setw(3)<<"F ";
			}
			of<<setw(30)<<ReplaceChar((*f_itor)->getName(),' ','*')<<" "<<setw(10)<<(*f_itor)->getSize()<<" "<<setw(20)<<(*f_itor)->getCreateTime()<<" "<<setw(16)<<sVersion<<endl;
		}
	}
	vector_d_itor d_itor;
	for(d_itor=m_vectorDirectory.begin(); d_itor!=m_vectorDirectory.end(); d_itor++)
	{
		if(*d_itor != NULL)
		{
			if(bSub)
				(*d_itor)->output(of,true);
			else
				of<<setw(3)<<"D "<<ReplaceChar((*d_itor)->getFullName(),' ','*')<<endl;
		}
	}
}

void Directory::check(std::string dname,std::string fname,bool bSub)
{
	Directory curD(dname);
	curD.parse(bSub);
	Directory preD;
	ifstream is;
	is.open(fname.c_str(),ios::in);
	if(!is.is_open())
		return;
	buildDirectory(preD,is);
}

void Directory::buildDirectory(Directory& directory,istream& is)
{
	std::string str;
	is>>str;
	if(str == "D")
	{
		std::string sDirectory;
		is>>sDirectory;
		directory.m_vectorDirectory.push_back(new Directory(sDirectory));
	}
	else if(str == "F")
	{
	}
	else if(str == "FV")
	{
	}
}

std::string Directory::getName()
{
	return(GetStringRightAtChar(getFullName(),'\\'));
}

std::string Directory::getFullName()
{
	return m_directory;
}

Directory* Directory::getDirectory(std::string name)
{
	vector_d_itor d_itor;
	for(d_itor=m_vectorDirectory.begin(); d_itor != m_vectorDirectory.end(); d_itor++)
	{
		if(*d_itor != NULL)
		{
			if(name == (*d_itor)->getName())
				return (*d_itor);
		}
	}
	return NULL;
}

File* Directory::getFile(std::string name)
{
	vector_f_itor f_itor;
	for(f_itor=m_vectorFile.begin(); f_itor!=m_vectorFile.end(); f_itor++)
	{
		if(*f_itor != NULL)
		{
			if(name == (*f_itor)->getName())
				return (*f_itor);
		}
	}
	return NULL;
}

vector_d& Directory::getDirectoryVector()
{
	return m_vectorDirectory;
}

vector_f& Directory::getFileVector()
{
	return m_vectorFile;
}

bool Directory::isDirectoryExist(std::string name)
{
	WIN32_FIND_DATA findData;
	HANDLE hHandle;
	if(name[name.size()-1] == '\\')
	{
		name[name.size()-1] = '\0';
	}	
	hHandle = ::FindFirstFile (name.c_str(),&findData);
	if (hHandle == INVALID_HANDLE_VALUE)
		return false;
	if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
	{
		if(strcmp(findData.cFileName,".") != 0 && strcmp(findData.cFileName,"..") != 0)
		{
			return true;
		}
	}
	return false;
}

bool Directory::isValid(::std::string str)
{
	std::string path;
	path = GetPathString(str);
	if(hasWildCard(path))
		return false;
	return true;
}

bool Directory::hasWildCard(std::string str)
{
	int strlen = str.size();
	for(int i=0;i<strlen;i++)
	{
		if(str[i]=='?' || str[i]=='*')
			return true;
	}
	return false;
}


