// IniInfo.cpp: implementation of the IniInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "Log.h"
#include "IniInfo.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
using namespace ZQ::common;

IniInfo::IniInfo()
{

}

IniInfo::~IniInfo()
{

}
void IniInfo::SetFile(std::string strFilePath)
{
	m_strCurrentFile=strFilePath;
	glog(Log::L_DEBUG,"Set current ini file=%s",strFilePath.c_str());
}
void IniInfo::SetSection(std::string strSection)
{
	m_strCurrentSection=strSection;
	glog(Log::L_DEBUG,"set current section =%s",strSection.c_str());
}
bool IniInfo::GetIniData(const std::string strkey)
{
	ZeroMemory(m_szCurrentValue,1024);
	GetPrivateProfileStringA(m_strCurrentSection.c_str(),strkey.c_str(),"",m_szCurrentValue,1023,m_strCurrentFile.c_str());
	glog(Log::L_DEBUG,"Get ini data with key=%s  And return value=%s",
							strkey.c_str(),m_szCurrentValue);
	return true;
}
bool IniInfo::GetValue(const std::string strKey,byte* byteValue, int buflens)
{
	if(buflens>1023||buflens<=0)
	{
		glog(Log::L_DEBUG,"IniInfo::GetValue()## invalid buflens,it must between 0 and 1023");
		return false;
	}
	GetIniData(strKey);
	memcpy(byteValue,m_szCurrentValue,buflens);
	return true;
}
bool IniInfo::GetValue(const std::string strKey,std::string& stringValue)
{
	GetIniData(strKey);
	stringValue=m_szCurrentValue;
	return true;
}
bool IniInfo::GetValue(const std::string strKey,double& doubleValue)
{
	GetIniData(strKey);
	doubleValue=atof(m_szCurrentValue);
	return true;
}
bool IniInfo::GetValue(const std::string strKey,float& floatValue)
{
	GetIniData(strKey);
	floatValue=(float)atof(m_szCurrentValue);
	return true;
}
bool IniInfo::GetValue(const std::string	strKey,short&			shortValue)
{
	GetIniData(strKey);
	shortValue=(short)atoi(m_szCurrentValue);
	return true;
}
bool IniInfo::GetValue(const std::string strKey,bool& boolValue)
{
	GetIniData(strKey);
	boolValue=(atoi(m_szCurrentValue)==1?true:false);
	return true;
}
bool IniInfo::GetValue(const std::string	strKey,long&			longValue)
{
	GetIniData(strKey);
	longValue=atol(m_szCurrentValue);
	return true;
}
bool IniInfo::GetValue(const std::string	strKey,int&				intValue)
{
	GetIniData(strKey);
	intValue=atoi(m_szCurrentValue);
	return true;
}

