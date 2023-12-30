// InitInfo.h: interface for the InitInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INITINFO_H__5881EF01_4F1B_46F6_B1D2_A79F1D227129__INCLUDED_)
#define AFX_INITINFO_H__5881EF01_4F1B_46F6_B1D2_A79F1D227129__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <vector>

class InitInfo  
{
public:
	InitInfo();
	virtual ~InitInfo();

	bool init(const char* szFilename);

	void close(){}

	//set current session name
	void setCurrent(const char* szSessionName);

	bool getValue(const char*key, int& value, bool bLog = true);
	bool getValue(const char*key, std::string& value, bool bLog = true, bool bEmptyFail = false);
	bool getValue(const char*key, char* value, int size, bool bLog = true, bool bEmptyFail = false);

	bool getValue(const char*key, std::string& value, const char* strDefault, bool bLog = true);
	bool getValue(const char*key, char* value, int size, const char* strDefault, bool bLog = true);
	
	//add by Hongquan.Zhang	
	bool getValue(const char*key, unsigned char* value,int size, bool bLog = true, bool bEmptyFail = false);
	bool getValue(const char*key, long& value, bool bLog = true, bool bEmptyFail = false);
	bool getValue(const char*key, double& value, bool bLog = true, bool bEmptyFail = false);
	bool getValue(const char*key, float& value, bool bLog = true, bool bEmptyFail = false);
	bool getValue(const char*key, short& value, bool bLog = true, bool bEmptyFail = false);
	bool getValue(const char*key, bool& value, bool bLog = true, bool bEmptyFail = false);
	
	std::string		GetIniFilePath();
private:
	char	_szFileName[256];
	char	_szCurSession[256];
};

#endif // !defined(AFX_INITINFO_H__5881EF01_4F1B_46F6_B1D2_A79F1D227129__INCLUDED_)
