
#pragma  warning(disable: 4786)

#include "StringFuncton.h"
#include <string>
#include <vector>
#include <map>
#include "Log.h"
#include <boost/regex.hpp>
#include "StringFuncImpl.h"


using namespace ZQ::common;
using namespace std;



#define STRING_FUNC_REGULAR_TEXT	"([^#]*)#([\\w]+)\\(([^\\)]*)\\)(.*)"


map<string, const SFUNC_DATA*>	_string_func_map;
boost::regex				_regFuncSyntax;

void StrFuncInit()
{
	_string_func_map.clear();
	for(int i=0;i<_string_func_count;i++)
	{
		_string_func_map[_string_func_data[i].sFunctionName] = &_string_func_data[i];
	}

	_regFuncSyntax.assign(STRING_FUNC_REGULAR_TEXT);
}

void StrFuncClose()
{
	_string_func_map.clear();
}

bool StrFuncDispatch(const char* szString, char*szReturn, int nSize)
{
	typedef boost::match_results<std::string::const_iterator> res_t;
	res_t results;
	std::string value;
	
	if (!boost::regex_match(szString, results, _regFuncSyntax))
		return false;
	
	char  szRet[256];

	char* pPtr = szReturn;
	string strFuncName(results[2].first, results[2].second);	
	map<string, const SFUNC_DATA*>::iterator itm = _string_func_map.find(strFuncName);
	if (itm!=_string_func_map.end())
	{
		// call function
		STRING_PROC pProcAddr = (itm->second)->procAddr;
		string  strParam(results[3].first, results[3].second);
		
		if (!pProcAddr(strParam.c_str(), szRet, sizeof(szRet)))
		{
			glog(Log::L_ERROR, "Function %s execute fail with param %s", strFuncName.c_str(), strParam.c_str());
		}
	}
	else
	{
		glog(Log::L_ERROR, "Function %s not found", strFuncName.c_str());
	}

	const char* p1 = results[1].first;
	const char* p2 = results[1].second;
	
	while(p1<p2)
		*pPtr++=*p1++;
	
	p1 = szRet;
	while(*p1)
		*pPtr++=*p1++;
	
	p1 = results[4].first;
	p2 = results[4].second;	//*p2 should be NULL
	if (p1<p2)
	{
		if (StrFuncDispatch(p1, pPtr, nSize-(pPtr-szReturn)))
		{
			//nothing to do
		}
		else
		{
			while(p1<p2)
				*pPtr++=*p1++;

			*pPtr = '\0';
		}
	}
	else
	{
		*pPtr = '\0';
	}

	return true;
}
