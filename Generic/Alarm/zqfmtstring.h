
#ifndef _ZQ_FMTSTRING_H_
#define _ZQ_FMTSTRING_H_

#include "alarmcommon.h"
	
#define MAX_CONVERTER_LEN	256
#define MAX_FORMATER_LEN	256
	
TG_BEGIN
	
struct FormatConverter
{
	char	converter[MAX_CONVERTER_LEN];
	char	formater[MAX_FORMATER_LEN];
};
	
//format string by special form
char* FormatString(const char* source_str, const FormatConverter* converters,
				size_t converter_count, char* dest_str, size_t dest_str_buf_size);

TG_END

#endif//_ZQ_FMTSTRING_H_
