// IniInfo.h: interface for the IniInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIINFO_H__2B20CCF5_8076_4C9C_8193_E8C11C8759C3__INCLUDED_)
#define AFX_INIINFO_H__2B20CCF5_8076_4C9C_8193_E8C11C8759C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>


//This class is used to read/write ini information into a file
class IniInfo  
{
public:
	IniInfo();
	IniInfo(char* lpFilePath);
	virtual ~IniInfo();
public:
	//Set file full path
	void				SetFile(std::string	strFilePath);
	
	//Set current section
	void				SetSection(std::string	strSection);
public:
	//get ini data
	bool				GetValue(const std::string	strKey,int&				intValue);
	bool				GetValue(const std::string	strKey,long&			longValue);
	bool				GetValue(const std::string	strKey,bool&			boolValue);	
	bool				GetValue(const std::string	strKey,short&			shortValue);
	bool				GetValue(const std::string	strKey,float&			floatValue);
	bool				GetValue(const std::string	strKey,double&			doubleValue);
	bool				GetValue(const std::string	strKey,std::string&		stringValue);
	bool				GetValue(const std::string	strKey,unsigned char*			byteValue,	int buflens);

//	bool				SetValue(const std::string	strKey,int&				intValue);
//	bool				SetValue(const std::string	strKey,long&			longValue);
//	bool				SetValue(const std::string	strKey,bool&			boolValue);	
//	bool				SetValue(const std::string	strKey,short&			shortValue);
//	bool				SetValue(const std::string	strKey,float&			floatValue);
//	bool				SetValue(const std::string	strKey,double&			doubleValue);
//	bool				SetValue(const std::string	strKey,std::string&		stringValue);
//	bool				SetValue(const std::string	strKey,byte*			byteValue,	int buflens);
private:
	bool				GetIniData(const std::string strkey);


private:
	std::string			m_strCurrentSection;
	std::string			m_strCurrentFile;
	char				m_szCurrentValue[1024];
};

#endif // !defined(AFX_INIINFO_H__2B20CCF5_8076_4C9C_8193_E8C11C8759C3__INCLUDED_)
