

#ifndef _INFOCOL_STRING_FUNCTION_
#define _INFOCOL_STRING_FUNCTION_


void StrFuncInit();

//////////////////////////////////////////////////////////////////////////
///@param szString  IN	source string
///@param szReturn  OUT  output string, applyed string function string
///@param nSize     in  size of szReturn buffer
///@return  true when string function call exist and applyed, false not exist string func
bool StrFuncDispatch(const char* szString, char*szReturn, int nSize);

void StrFuncClose();


#endif