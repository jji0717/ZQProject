
#include <stdio.h>
#include "HelperClass.h"

HelperClass::HelperClass(void)
{
}

HelperClass::~HelperClass(void)
{
}
bool inline validString(const char* key )
{
	return static_cast<bool>( key && key[0]!= 0);
}
bool	HelperClass::getRegistryData( const char* path ,  const char* key , BYTE* data ,DWORD dataSize , DWORD type)
{
	if(!validString( path ) || !validString( key ) )
	{
		return false;
	}
	//open the registry first
	HKEY hResultKey = NULL;
	if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,path,0,KEY_QUERY_VALUE,&hResultKey))
	{
		return false;
	}
	DWORD dwType=0;	
	if(ERROR_SUCCESS != RegQueryValueEx(hResultKey,key,0,&dwType,data,&dataSize))
	{
		RegCloseKey(hResultKey);
		return false;
	}
	if(dwType != type )
	{
		RegCloseKey(hResultKey);
		return false;
	}
	return true;
}
bool HelperClass::GetRegistryValue( const char* path ,  const char* key , std::string& value)
{
	BYTE data[32];
	memset( data , 0 , sizeof(data) );
	if( !getRegistryData(path,key,data,sizeof(data),REG_SZ) )
	{
		return false;
	}
	value = (char*)data;
	return true;
}
bool HelperClass::GetRegistryValue( const char* path ,  const char* key , DWORD& value)
{
	BYTE data[32];
	if( !getRegistryData(path,key,data,sizeof(data),REG_DWORD) )
	{
		return false;
	}
	DWORD dwOutput;
	DWORD dwTemp;
	dwTemp=data[3];
	dwOutput=dwTemp<<24;
	dwTemp=data[2];
	dwTemp=dwTemp<<16;
	dwOutput|=dwTemp;
	dwTemp=data[1];
	dwTemp=dwTemp<<8;
	dwOutput|=dwTemp;
	dwOutput|=data[0];	

	value = dwOutput;

	return true;
}

std::string HelperClass::dumpBinary( const std::string& strBin )
{
	char szBuf[1024]={0};
	if( strBin.empty() )
	{
		return "";
	}
	int iBinSize = strBin.length();
	int iPos = 0;
	int iSize = sizeof(szBuf) - 1;
	
	char* pBuf = szBuf;	

	const unsigned char* pBin = reinterpret_cast<const unsigned char*> ( strBin.c_str() );

	for( int i = 0 ; i < iBinSize ; i++ )
	{
		iPos = _snprintf( pBuf,iSize ,"%02X", *pBin );
		iSize -= iPos;
		pBuf += iPos;
		pBin ++;
	}
	
	return std::string(szBuf);
}