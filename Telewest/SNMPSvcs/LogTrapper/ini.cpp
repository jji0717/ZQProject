
//ini.cpp

#ifdef _UNICODE
#define DEF_UNICODE
#undef _UNICODE
#undef UNICODE
#endif//_UNICODE

#include "ini.h"
#include <assert.h>

void GetStringList(const char* strStringList, size_t zLen, std::vector<std::string>& arrLists)
{
	assert(strStringList);

	std::string strGet;
	const char* strNext = strStringList;
	size_t zTotalLen = 0;
	arrLists.clear();
	for (;;)
	{
		strGet = strNext;
		arrLists.push_back(strGet);

		zTotalLen += (strlen(strNext)+1);
		if (zTotalLen < zLen)
		{
			strNext = strStringList+zTotalLen;
			if (NULL == strNext[0])
			{//at the end
				break;
			}
		}
		else
		{//up to limit string_list length
			break;
		}
	}
}

IniFile::IniFile(const char* strFileName)
{
	if (NULL != strFileName)
	{
		SetFileName(strFileName);
	}
}

void IniFile::SetFileName(const char* strFileName)
{
	assert(strFileName);

	m_strFileName = strFileName;
}

std::string IniFile::GetFileName(void)
{
	return m_strFileName;
}

bool IniFile::EnumSection(std::vector<std::string>& arrLists)
{
	char* strSection = new char[MAX_INI_SECTION];
	memset(strSection, 0, MAX_INI_SECTION);

	DWORD dwGet = GetPrivateProfileSectionNames(strSection, MAX_INI_SECTION, GetFileName().c_str());
	GetStringList(strSection, MAX_INI_SECTION, arrLists);

	delete[] strSection;

	return dwGet > 0;
}

bool IniFile::EnumKey(const char* strSection, std::vector<std::string>& arrLists)
{
	char* strKey = new char[MAX_INI_KEY];
	memset(strKey, 0, MAX_INI_KEY);

	DWORD dwGet = GetPrivateProfileSection(strSection, strKey, MAX_INI_KEY, GetFileName().c_str());
	GetStringList(strKey, MAX_INI_KEY, arrLists);

	delete[] strKey;

	return dwGet > 0;
}

bool IniFile::WriteKey(const char* strSection, const char* strKey, const char* strValue)
{
	return TRUE == WritePrivateProfileString(strSection, strKey, strValue, GetFileName().c_str());
}

std::string IniFile::ReadKey(const char* strSection, const char* strKey)
{
	char strGet[MAX_INI_LINE] = {0};

	GetPrivateProfileString(strSection, strKey, "", strGet, MAX_INI_LINE, GetFileName().c_str());

	//FILE* hFile = fopen("c:\\isa.test", "a");
	//fprintf(hFile, "Sec: %s\n\tKey: %s\n\tValue: %s\n", strSection, strKey, strGet);
	//fclose(hFile);

	return strGet;
}

bool IniFile::DeleteSection(const char* strSection)
{
	return TRUE == WritePrivateProfileString(strSection, NULL, NULL, GetFileName().c_str());
}

bool IniFile::DeleteKey(const char* strSection, const char* strKey)
{
	return TRUE == WritePrivateProfileString(strSection, strKey, NULL, GetFileName().c_str());
}

#ifdef DEF_UNICODE
#define _UNICODE
#define UNICODE
#undef DEF_UNICODE
#endif//DEF_UNICODE


