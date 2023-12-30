// CSILibTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "CSILibAPI.h"
#include <string>
using namespace CSILib;
using namespace std;

 void logCB(HANDLE hSession, const char *pClassString,  char *pMessageString )
 {

 }
int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("input content file path\n");
		return 0;
	}
	std::string filename =argv [1];

	printf("open file: %s\n", filename.c_str());
	FILE* fp = fopen(filename.c_str(), "rb");
	if(NULL == fp)
	{
		printf("failed to open file: %s\n", filename.c_str());
		return 0;
	}
   
	DWORD logLevel = 7;
	CSI_ERROR error = csiInitializeLibrary( 10, logCB, logLevel);
    if(error != CSI_MSG_SUCCESS)
	{
	  printf("failed to Initialize Library\n");
	  fclose(fp);
	  return 0;
	}

	HANDLE phSession = NULL;
	error =  csiAddSession(&phSession );
	if(error != CSI_MSG_SUCCESS)
	{
		printf("failed to Add Session\n");
		fclose(fp);
		csiCloseLibrary();
		return 0;
	}
    while( !feof(fp))
	{
		char buf[65536];
		int nread = fread(buf, 1, 65536, fp);
		if(nread >0)
		{
			CSI_STREAM_INFORMATION streamInfo;
			memset((void*)&streamInfo, 0, sizeof(CSI_STREAM_INFORMATION));
             
			error = csiProcessBuffer(phSession, (BYTE *)buf,nread, &streamInfo);

			if(! (streamInfo.flags &  CSI_INFO_VCODEC))
				continue;
			else
			{
				printf("%d, %d, %s, %s\n", streamInfo.flags, streamInfo.length, streamInfo.videoCodec, streamInfo.TBD);
				break;
			}

			/*if(error == CSI_MSG_SUCCESS)
			{
				continue;
			}
			else if(error == CSI_MSG_COMPLETE)
			{
				printf("%d, %d, %s, %s\n", streamInfo.flags, streamInfo.length, streamInfo.videoCodec, streamInfo.TBD);
				break;
			}
			else
			{
				printf("failed to process buffer with error %d\n", error);
				break;
			}
			*/
		}
	}

	csiCloseSession(phSession);
	fclose(fp);
	csiCloseLibrary();
	return 0;
}

