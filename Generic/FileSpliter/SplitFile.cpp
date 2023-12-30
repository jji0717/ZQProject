#include "getopt.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "afx.h"

#define MAX_FILE_SIZE 100000

int    index;
char   path[100];
char   folder[100];
char   abbr[3];
char   name[100];
DWORD  size;
HANDLE hDestFile;
HANDLE hFile;

void getFileNameAndAbbr(char* path, char* name, char* abbr);
void CreateNewFile(char* data, LONG size);
void ProcBiggerFile();
void ProcSmallerFile();

void main(int argc, char* argv[])
{
	if (argc <2)
	{
		printf("parameters can't be less than 2!\n");
		exit(0);
	}
	int ch;
	while((ch = getopt(argc, argv, "s:b:d:h")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			exit(0);

		case 's':
			strcpy(path, optarg);
//			TRACE("%s\n", path);
			break;

		case 'b':
			size = atol(optarg);
//			TRACE("%d\n", size);
			break;

		case 'd':
			strcpy(folder, optarg);
//			TRACE("%s\n", folder);
			break;

		default:
			break;
		}
	}

	getFileNameAndAbbr(path, name, abbr);
	
	if (size > MAX_FILE_SIZE)
	{
		ProcBiggerFile();
	}
	else
	{
		ProcSmallerFile();
	}	
	exit(0);
}
void getFileNameAndAbbr(char* path, char* name, char* abbr)
{
	char fileName[100];
	memset(abbr, 0x00, 3*sizeof(char));
	memset(name, 0x00, 100*sizeof(char));
	memset(fileName, 0x00, 100*sizeof(char));

	char* p = strstr(path, "\\");
	char* q = NULL;
	while (p)
	{
		p = p + 1;
		q = p;
		p = strstr(p, "\\");
	}
	if (q)
	{
		strncpy(fileName, path, strlen(q));
	}
	else
	{
		strcpy(fileName, path);
	}


	p = q = NULL;
	p = strstr(fileName, ".");
	while (p)
	{
		p = p + 1;
		q = p;
		p = strstr(p, ".");
	}
	strncpy(name, fileName, strlen(fileName)-strlen(q)-1);
	strcpy(abbr, q);
}
void CreateNewFile(char* data, LONG size)
{
	char path[100];
	sprintf(path, "%s\\%s%d.%s", folder, name, index, abbr);
	hFile = CreateFile(path,
		               FILE_ALL_ACCESS, 
					   0,
					   NULL,
					   CREATE_ALWAYS,
					   FILE_ATTRIBUTE_NORMAL, 
					   NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		int err = GetLastError();
		exit(0);
	}
	DWORD tempSize = 0;
	if (!WriteFile(hFile, data, size, &tempSize, NULL))
	{
		printf("write file failed!\n");
		exit(0);
	}
}

void ProcSmallerFile()
{
	hDestFile = CreateFile(path,
							   GENERIC_READ, 
							   0,
							   NULL,
							   OPEN_EXISTING,
							   FILE_ATTRIBUTE_NORMAL, 
							   NULL);
	if (hDestFile == INVALID_HANDLE_VALUE)
	{
		int err = GetLastError();
		exit(0);
	}
	
	index = 0;

	DWORD realSize = 0;
	char buf[MAX_FILE_SIZE];

	do
	{
		memset(buf, 0x00, MAX_FILE_SIZE*sizeof(char));
		if (!ReadFile(hDestFile, buf, size, &realSize, NULL))
		{
			printf("read %s file failed!\n", path);
			exit(0);
		}
		if (realSize > 0)
		{
			if (index == 0)
			{
				if(!CreateDirectory(folder, NULL))
				{
					printf("create %s directory failed!\n", folder);
					exit(0);
				}
			}
			CreateNewFile(buf, realSize);
			index ++;
		}
	}
	while (realSize == size);
}

void ProcBiggerFile()
{
	hDestFile = CreateFile(path, 
						   GENERIC_READ, 
						   0,
						   NULL,
						   OPEN_EXISTING,
						   FILE_ATTRIBUTE_NORMAL, 
						   NULL);
	if (hDestFile == INVALID_HANDLE_VALUE)
	{
		int err = GetLastError();
		exit(0);
	}
	if(!CreateDirectory(folder, NULL))
	{
		printf("create %s directory failed!\n", folder);
		exit(0);
	}
	char buf[MAX_FILE_SIZE];
	memset(buf, 0x00, MAX_FILE_SIZE*sizeof(char));
	DWORD realSize = 0;
	DWORD totalSize;
	index = 0;
	
	do
	{
		char path[100];
		sprintf(path, "%s\\%s%d.%s", folder, name, index, abbr);
		hFile = CreateFile(path,
						   FILE_ALL_ACCESS, 
						   0,
						   NULL,
						   CREATE_ALWAYS,
						   FILE_ATTRIBUTE_NORMAL, 
						   NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			int err = GetLastError();
			exit(0);
		}
		LONG lvSize = size;
		totalSize = 0;
		do
		{
			memset(buf,0x00, MAX_FILE_SIZE*sizeof(char));
			if (lvSize > MAX_FILE_SIZE)
			{
				if (!ReadFile(hDestFile, buf, MAX_FILE_SIZE, &realSize, NULL))
				{
					printf("read %s file failed!\n", path);
					exit(0);
				}
				if (realSize == 0)
				{
					break;
				}
				DWORD tempSize;
				if (!WriteFile(hFile, buf, realSize, &tempSize, NULL))
				{
					printf("write file failed!\n");
					exit(0);
				}
				totalSize = totalSize + realSize;
			}
			else
			{
				if (!ReadFile(hDestFile, buf, lvSize, &realSize, NULL))
				{
					printf("read %s file failed!\n", path);
					exit(0);
				}
				if (realSize == 0)
				{
					break;
				}
				DWORD tempSize;
				if (!WriteFile(hFile, buf, realSize, &tempSize, NULL))
				{
					printf("write file failed!\n");
					exit(0);
				}
				totalSize = totalSize + realSize;
				break;
			}
			lvSize = lvSize - realSize;
		}while(1);
		index ++;
	}while(size == totalSize);
}
