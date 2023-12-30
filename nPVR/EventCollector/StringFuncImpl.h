
#ifndef _STRING_FUNC_IMPL_ 
#define _STRING_FUNC_IMPL_

typedef bool (*STRING_PROC)(const char*, char*, int);

struct SFUNC_DATA
{
	const char* sFunctionName;
	STRING_PROC procAddr;
};

extern const SFUNC_DATA	_string_func_data[];
extern int _string_func_count;


bool SF_MSA_TIME(const char* inputs, char* outputs, int size);
bool SF_DEC2HEX8(const char* inputs, char* outputs, int size);
bool SF_UTC_TIME(const char* inputs, char* outputs, int size);

#endif