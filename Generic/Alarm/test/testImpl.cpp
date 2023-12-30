
#include "../implinclude.h"
#include <stdio.h>
	
bool Action_Proc(char* const* key, char* const* data, size_t count)
{
	printf("Action_Proc\n");
	
	printf(">do proc:\n");
	for (size_t i = 0; i < count; ++i)
	{
		printf("%s : %s\n", key[i], data[i]);
	}
	return true;
}

bool Init_Proc()
{
	printf("Init_Proc\n");
	return true;
}

void UnInit_Proc()
{
	printf("UnInit_Proc\n");
}
