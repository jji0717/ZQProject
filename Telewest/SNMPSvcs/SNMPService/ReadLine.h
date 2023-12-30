
#ifndef _D_LIB_READLINE_H_
#define _D_LIB_READLINE_H_

#include <windows.h>

#define SKIP_HEAD_CHARS 10

//read a line from log file
int ReadLogLine(HANDLE hFile, char* strLine, size_t zLen, bool bStart = false);

int GetLogEnd(HANDLE hFile);

bool GoLogBegin(HANDLE hFile);

bool GoFirstLogLine(HANDLE hFile);

#endif//_D_LIB_READLINE_H_
