
#ifndef _ZQ_STREAMSMITH_HELPER_CLASS_HEADER_FILE_H__
#define _ZQ_STREAMSMITH_HELPER_CLASS_HEADER_FILE_H__

#include <windows.h>
#include <string>
class HelperClass
{
public:
	HelperClass(void);
	~HelperClass(void);
public:
	static std::string dumpBinary( const std::string& strBin );
	static bool	GetRegistryValue( const char* path ,  const char* key , DWORD& value);
	static bool GetRegistryValue( const char* path ,  const char* key , std::string& value);	
private:
	static bool	getRegistryData( const char* path ,  const char* key , BYTE* value ,DWORD size , DWORD type);

};

#endif//_ZQ_STREAMSMITH_HELPER_CLASS_HEADER_FILE_H__
