
#include "registryex.h"


RegistryEx_T::RegistryEx_T(const char* strPath, RegistryEx_T::Root root)
		:m_hKey(NULL)
	{
		HKEY hRoot = (HKEY)root;
		if (ERROR_SUCCESS != RegOpenKeyEx(hRoot, strPath, 0, KEY_READ|KEY_WRITE, &m_hKey))
		{
			DWORD dwTemp;
			RegCreateKeyEx(hRoot, strPath, 0, "", 0, KEY_READ|KEY_WRITE, NULL, &m_hKey, &dwTemp);
		}
	}

	RegistryEx_T::~RegistryEx_T()
	{
		if (!IsError())
		{
			RegCloseKey(m_hKey);
		}
	}

	bool RegistryEx_T::IsError(void)
	{
		return NULL == m_hKey;
	}

	bool RegistryEx_T::Load(const char* strNode, RegistryEx_T::Type& tp,BYTE* pData, size_t zLen)
	{
		assert(strNode&&pData&&m_hKey);

		return ERROR_SUCCESS == RegQueryValueEx(m_hKey, strNode, NULL, (DWORD*)&tp, pData, (DWORD*)&zLen);
	}

	bool RegistryEx_T::LoadNoType(const char* strNode, BYTE* pData, size_t zLen)
	{
		assert(strNode&&pData&&m_hKey);

		return ERROR_SUCCESS == RegQueryValueEx(m_hKey, strNode, NULL, NULL, pData, (DWORD*)&zLen);
	}

	bool RegistryEx_T::Save(const char* strNode, RegistryEx_T::Type tp, const BYTE* pData, size_t zLen)
	{
		assert(strNode&&pData&&m_hKey);

		return ERROR_SUCCESS == RegSetValueEx(m_hKey, strNode, 0, (DWORD)tp, pData, zLen);
	}

	bool RegistryEx_T::LoadStr(const char* strNode, const char* strData, size_t zLen)
	{
		return LoadNoType(strNode, (BYTE*)strData, zLen);
	}

	bool RegistryEx_T::LoadDword(const char* strNode, DWORD& dwData)
	{
		return LoadNoType(strNode, (BYTE*)&dwData, sizeof(DWORD));
	}

	bool RegistryEx_T::SaveString(const char* strNode, const char* strData)
	{
		return Save(strNode, TP_SZ, (const BYTE*)strData, strlen(strData));
	}

	bool RegistryEx_T::SaveDword(const char* strNode, DWORD dwData)
	{
		return Save(strNode, TP_DWORD_LITTLE_ENDIAN, (const BYTE*)&dwData, sizeof(DWORD));
	}

	bool RegistryEx_T::Delete(const char* strNode)
	{
		assert(strNode&&m_hKey);

		return ERROR_SUCCESS == RegDeleteValue(m_hKey, strNode);
	}

	bool RegistryEx_T::EnumSubkey(size_t szPos, char* strSubKey, size_t szLen)
	{
		assert(strSubKey);
		memset(strSubKey, 0,szLen*sizeof(char));
		return ERROR_SUCCESS == RegEnumKeyEx(m_hKey, szPos, strSubKey, (DWORD*)&szLen, 0, 0, 0, 0);
	}

	bool RegistryEx_T::EnumValue(size_t szPos, char* strKey, size_t szKeyLen,
						char* strValue, size_t szValueLen)
	{
		assert(strKey && strValue);
		memset(strKey, 0, szKeyLen*sizeof(char));
		memset(strValue, 0,szValueLen*sizeof(char));
		return ERROR_SUCCESS == RegEnumValue(m_hKey, szPos, strKey, (DWORD*)&szKeyLen, 0, 0, (BYTE*)strValue, (DWORD*)&szValueLen);
	}