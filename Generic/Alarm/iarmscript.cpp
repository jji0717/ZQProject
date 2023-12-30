
#include "iarmscript.h"
#include <windows.h>


bool FindInString(const std::string& str, const std::string& strSet, std::string& strFront, std::string& strEnd)
{
	int nFind = str.find(strSet);
	if (std::string::npos == nFind)
		return false;
	
	strFront = str.substr(0, nFind);
	strEnd = str.substr(nFind+strSet.size(), std::string::npos);
	return true;
}

bool FindSub(const std::string& str, std::string& strFront, std::string& strEnd)
{
	int nTemp = 1;
	for (int i = 0; i < str.size(); ++i)
	{
		if (str[i] == '(')
		{
			++nTemp;
		}
		else if(str[i] == ')')
		{
			--nTemp;
			if (0 == nTemp)
			{
				strFront = str.substr(0, i);
				strEnd = str.substr(i+1, std::string::npos);
				return true;
			}
		}
	}
	return false;
}

bool GetFunc(const std::string& str, const std::string& strSet, std::string& strFront, std::string& strFunc, std::string& strEnd)
{
	std::string strTemp;
	if (!FindInString(str, strSet, strFront, strTemp))
	{
		return false;
	}
	
	if (!FindSub(strTemp, strFunc, strEnd))
	{
		return false;
	}
	return true;
}


std::string Dec2Hex(int nDec, int nLen)
{
	char* strLine = new char[nLen+1];
	memset(strLine, 0, nLen+1);
	
	char strFmt[20] = {0};
	sprintf(strFmt, "%%0%dd", nLen);
	
	sprintf(strLine, strFmt, nDec);
	
	std::string strRtn(strLine);
	delete[] strLine;
	
	return strRtn;
}

std::string MsaTime(const std::string& str)
{
	int nMonth=0, nDay=0, nHour=0, nMin=0, nSec=0, nMSec=0;
	
	sscanf(str.c_str(), "%2d/%2d %2d:%2d:%2d:%3d", &nMonth, &nDay, &nHour, &nMin, &nSec, &nMSec);
	
	std::string strRtn;

	SYSTEMTIME systime;
	GetSystemTime(&systime);
	
	char strTemp[256] = {0};
	sprintf(strTemp, "%d%02d%02d %02d%02d%02d.%03d", systime.wYear, nMonth, nDay, nHour, nMin, nSec, nMSec);
	//
	return strTemp;
}


std::string ApplyFunctions(const std::string& str)
{
	std::string strFront;
	std::string strFunc;
	std::string strEnd;
	if (str.empty())
	{
		return str;
	}
	else if (GetFunc(str, FUNC_MSATIME, strFront, strFunc, strEnd))
	{
		return strFront+MsaTime(ApplyFunctions(strFunc))+ApplyFunctions(strEnd);
	}
	else if (GetFunc(str, FUNC_DEC2HEX, strFront, strFunc, strEnd))
	{
		int nNumber = 0;
		int nLen = 0;
		const char* strLine = ApplyFunctions(strFunc).c_str();
		sscanf(strLine, "%d,%d", &nNumber, &nLen);
		return strFront+Dec2Hex(nNumber, nLen)+ApplyFunctions(strEnd);
	}
	return str;
}

