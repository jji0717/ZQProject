#ifndef _DANIEL_REGISTRYEX_H_
#define _DANIEL_REGISTRYEX_H_

#ifdef UNICODE
#undef _UNICODE
#undef UNICODE
#define USE_UNICODE
#endif

#include <windows.h>
#include <assert.h>

//registry 
class RegistryEx_T
{
public:
	//the data type in registry
	enum Type
	{
		TP_NONE					= REG_NONE,
		TP_BINARY				= REG_BINARY,
		TP_DWORD_LITTLE_ENDIAN	= REG_DWORD_LITTLE_ENDIAN,
		TP_DWORD_BIG_ENDIAN		= REG_DWORD_BIG_ENDIAN,
		TP_EXPAND_SZ			= REG_EXPAND_SZ,
		TP_LINK					= REG_LINK,
		TP_MULTI_SZ				= REG_MULTI_SZ,
		TP_RESOURCE_LIST		= REG_RESOURCE_LIST,
		TP_SZ					= REG_SZ
	};

	//the root path name in registry
	enum Root
	{
		RT_HKEY_CLASSES_ROOT		= HKEY_CLASSES_ROOT,
		RT_HKEY_CURRENT_CONFIG		= HKEY_CURRENT_CONFIG,
		RT_HKEY_CURRENT_USER		= HKEY_CURRENT_USER,
		RT_HKEY_LOCAL_MACHINE		= HKEY_LOCAL_MACHINE,
		RT_HKEY_USERS				= HKEY_USERS,
		RT_HKEY_PERFORMANCE_DATA	= HKEY_PERFORMANCE_DATA,
		RT_HKEY_DYN_DATA			= HKEY_DYN_DATA
	};
private:
	//current key
	HKEY	m_hKey;
public:
	//constructor and distructor
	RegistryEx_T(const char* strPath, Root root = RT_HKEY_LOCAL_MACHINE);

	~RegistryEx_T();

	//return true if get error
	bool IsError(void);

	//read data and data type from registry
	bool Load(const char* strNode, RegistryEx_T::Type& tp,BYTE* pData, size_t zLen);

	//read data from registry
	bool LoadNoType(const char* strNode, BYTE* pData, size_t zLen);

	//write data into registry
	bool Save(const char* strNode, RegistryEx_T::Type tp, const BYTE* pData, size_t zLen);

	//read a string from registry
	//the function name can not be LoadString 
	bool LoadStr(const char* strNode, const char* strData, size_t zLen);

	//read a dword number from registry
	bool LoadDword(const char* strNode, DWORD& dwData);

	//write a string to registry
	bool SaveString(const char* strNode, const char* strData);

	//write a dword number to reigstry
	bool SaveDword(const char* strNode, DWORD dwData);

	//delete a sub node in registry
	bool Delete(const char* strNode);

	//[bad function name]
	//query sub keys in current key
	bool EnumSubkey(size_t szPos, char* strSubKey, size_t szLen);

	//[bad function name]
	//query values in current key
	bool EnumValue(size_t szPos, char* strKey, size_t szKeyLen,
						char* strValue, size_t szValueLen);
};

#ifdef USE_UNICODE
#undef USE_UNICODE
#define _UNICODE
#define UNICODE
#endif

#endif//_DANIEL_REGISTRYEX_H_