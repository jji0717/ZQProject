
#include "zqfmtstring.h"
#include "zqerror.h"
#include <assert.h>
#include <string>

TG_BEGIN

char* FormatString(const char* source_str, const FormatConverter* converters,
	size_t converter_count, char* dest_str, size_t dest_str_buf_size)
{
	DEFENSE_ASSERT(source_str&&converters&&dest_str, NULL);

	size_t dest_position = 0;

	size_t soure_str_len = strlen(source_str);
	for (size_t i = 0; i < soure_str_len; ++i)
	{
		for (size_t j = 0; j < converter_count; ++j)
		{
			size_t convert_len = strlen(converters[j].converter);
			DEFENSE_ASSERT((convert_len<MAX_CONVERTER_LEN), NULL);
			if (0 == strncmp(source_str+i, converters[j].converter, convert_len))
			{
				strncpy(dest_str+dest_position, converters[j].formater,
					dest_str_buf_size-dest_position-1);
				
				i				+= convert_len-1;
				dest_position	+= strlen(converters[j].formater);
				break;
			}

			if (j == converter_count-1)
				dest_str[dest_position++] = source_str[i];
		}
	}
	
	dest_str[dest_position] = '\0';
	return dest_str;
}

TG_END
