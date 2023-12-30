// md5sum.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "windows.h"

#include "getopt.h"
#include "MD5CheckSumUtil.h"


void usage()
{
	printf("Usage: md5sum -f <file> read from file to generate md5 check sum\n");
	printf("       -i read from std input to generate md5 check sum\n");
	printf("       -h display this help\n");	
}

int main(int argc, char* argv[])
{
	if (argc <2)
	{
		usage();
		return 0;
	}

	char szSourceFile[256];
	bool bReadFromSTD = false;

	int ch;
	while((ch = getopt(argc, argv, "hHiIf:F:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
		case 'H':
			usage();
			return 0;

		case 'f':
		case 'F':
			if (optarg)
			{
				strcpy(szSourceFile, optarg);
			}
			break;

		case 'i':
		case 'I':
			bReadFromSTD = true;
			break;

		default:
			fprintf(stderr, "Error: unknown option %c specified\n", ch);
			return 1;
		}
	}

	HANDLE hFile1 = INVALID_HANDLE_VALUE;
	if (bReadFromSTD)
	{
		hFile1 = GetStdHandle(STD_INPUT_HANDLE);
	}
	else
	{
		hFile1 = CreateFile(szSourceFile, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,0, OPEN_EXISTING, 0, 0);
	}	
	if (hFile1==INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "[%s] Fail to open source fille with error code %d\n", szSourceFile, GetLastError());
		return 1;
	}

	char buf[188*1024];
	DWORD dwRead=0;
	DWORD dwToRead;
	
	ZQ::common::MD5ChecksumUtil md5;	

	do
	{
		dwToRead = sizeof(buf);
		dwRead = 0;
		if(!ReadFile(hFile1, buf, dwToRead, &dwRead, 0))
			break;

		if (dwRead)
		{
			md5.checksum(buf, dwRead);
		}
		else
		{
			break;		
		}
	}while(1);

	CloseHandle(hFile1);
	printf("%s", md5.lastChecksum());
	return 0;
}
