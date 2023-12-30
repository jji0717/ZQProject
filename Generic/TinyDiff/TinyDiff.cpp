// TinyDiff.cpp : Defines the entry point for the console application.
//

#include "stdio.h"
#include <windows.h>

#define BUFFSIZE	1024*1024*2

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		printf("tinydiff <file 1> <file 2> [buffer size in MB]\n");
		return -1;
	}

	HANDLE	f1=INVALID_HANDLE_VALUE, f2=INVALID_HANDLE_VALUE;
	HANDLE	dH1=INVALID_HANDLE_VALUE, dH2=INVALID_HANDLE_VALUE;

	f1 = ::CreateFile(
		argv[1],
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if(f1 == INVALID_HANDLE_VALUE)
	{
		printf("Can not open file \"%s\"!\n", argv[1]);
		return -1;
	}

	f2 = ::CreateFile(
		argv[2],
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	
	if(f2 == INVALID_HANDLE_VALUE)
	{
		printf("Can not open file \"%s\"!\n", argv[2]);
		return -1;
	}

	long buffer_size = BUFFSIZE;
	if(argc>=4)
	{
		sscanf(argv[3], "%ld", &buffer_size);
		if(buffer_size<BUFFSIZE/1024/1024 || buffer_size>=100)
			buffer_size = BUFFSIZE;
		else
			buffer_size = buffer_size*1024*1024;
	}

	bool dump = false;
//	if(argc>=5)
//	{
//		char csdump[32] ={0};
//		sscanf(argv[4], "%s", csdump);
//		if(csdump[0]=='d' || csdump[0]=='D')
//			dump = true;
//	}

	if(dump)
	{
		char	diffName1[MAX_PATH]={0}, diffName2[MAX_PATH]={0};
		strcpy(diffName1, argv[1]);
		strcpy(diffName2, argv[2]);
		strcat(diffName1, ".diff");
		strcat(diffName2, ".diff");
		dH1 = ::CreateFile(
			diffName1,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		
		dH2 = ::CreateFile(
			diffName2,
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		
		if(dH1 == INVALID_HANDLE_VALUE || dH2 == INVALID_HANDLE_VALUE)
			dump = false;
	}

	printf("\n                        TinyDiff                         \n");
	printf("  compares files and gives out different offset.  -by B.Z.\n\n");
	printf("Buffer size is [%d] MB\n", buffer_size/1024/1024);
	printf("Comparing files");
	bool bContinue = true;
	DWORD offset1=0, offset2=0;
	char	*tmpBuff1=0, *tmpBuff2=0;
	bool bEnd1=false, bEnd2=false;
	for(int times=0; ;times++)
	{
		if(tmpBuff1)
		{
			delete[] tmpBuff1;
			tmpBuff1 = NULL;
		}
		if(tmpBuff2)
		{
			delete[] tmpBuff2;
			tmpBuff2 = NULL;
		}

		if(!dump && !bContinue)
			break;
		else if(dump && !bContinue)
			printf("dumping result...");
		else if(times%5==0)
			printf(".");
	
		tmpBuff1 = new char[buffer_size];
		tmpBuff2 = new char[buffer_size];

		DWORD	readLen1=0, readLen2=0;
		int r1=0,r2=0;
		if(!bEnd1)
			r1 = ::ReadFile(f1, tmpBuff1, buffer_size-1, &readLen1, NULL);
		if(!bEnd2)
			r2 = ::ReadFile(f2, tmpBuff2, buffer_size-1, &readLen2, NULL);

		if(dump && !bContinue)
		{
			if(r1 == 0 || r2 == 0)
			{
				printf("\nDone.\n");
				break;
			}
			
			if(readLen1==0 && readLen2==0)
			{	
				printf("\nDone.\n");
				break;
			}

			DWORD writeLen1=0, writeLen2=0;
			if(r1!=0 && readLen1>0)
			{
				::WriteFile(dH1, tmpBuff1, readLen1, &writeLen1, NULL);
			}
			if(r2!=0 && readLen2>0)
			{
				::WriteFile(dH2, tmpBuff2, readLen2, &writeLen2, NULL);
			}
			continue;
		}


		if(r1 == 0 || r2 == 0)
		{
			printf("\nError when reading files!  Error code:%08X\n", ::GetLastError());
			break;
		}

		if(readLen1==0 && readLen2==0)
		{	
			printf("\nFiles 1 and 2 are identical!\n");
			break;
		}
		else if(readLen1!=0 && readLen2!=0)
		{
			
			if(readLen1<readLen2)
			{
				printf("\nFile 1 is smaller and reaches end on offset %u!\n", offset1);
				bContinue = false;
				continue;
			}
			else if(readLen1>readLen2)
			{
				printf("\nFile 2 is smaller and reaches end on offset %u!\n", offset2);
				bContinue = false;
				continue;
			}
			else
			{
				for(DWORD i=0; i<readLen1; i++)
				{
					if(tmpBuff1[i]!=tmpBuff2[i])
					{
						bContinue = false;
						break;
					}
				}
				
				if(i!=readLen1)
				{
					printf("\nFiles branch from offset %u!\n", offset1+i);
					continue;
				}
			}
			offset1 += readLen1;
			offset2 += readLen2;
		}
		else
		{
			printf("\nFile %d is smaller and reaches end on offset %u!\n", (readLen1==0?1:2), (readLen1==0?offset1:offset2));
			bContinue = false;
			continue;
		}

		::Sleep(1);	// give out turn of CPU
	}

	

	if(tmpBuff1)
	{
		delete[] tmpBuff1;
		tmpBuff1 = NULL;
	}
	if(tmpBuff2)
	{
		delete[] tmpBuff2;
		tmpBuff2 = NULL;
	}

	if(f1!=INVALID_HANDLE_VALUE)
		CloseHandle(f1);
	if(f2!=INVALID_HANDLE_VALUE)
		CloseHandle(f2);
	if(dH1!=INVALID_HANDLE_VALUE)
		CloseHandle(dH1);
	if(dH2!=INVALID_HANDLE_VALUE)
		CloseHandle(dH2);
	return 0;
}
