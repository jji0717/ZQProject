
#include "getopt.h"
#include <json/json.h>
#include <TimeUtil.h>
extern "C" {
#include<stdio.h>
#include <string.h>
}

#define  TIMEOFDAY (24 * 3600 * 1000)

bool writeFile(const std::string& fileName, const std::string& data)
{
	FILE* file = fopen(fileName.c_str(),"wb");
	if (file == NULL)
	{
		fclose(file);
		return false;
	}

	int res = fwrite(data.c_str(), sizeof(unsigned char), data.length() * sizeof(unsigned char), file);
	if ( res < data.length())
	{
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
}
void usage()
{
	printf("display the help information of the command line:\n");
	printf(	"-h                    display this screen\n"
		      "-l <filename>         this program should generate the license data such as[{\"rtspProxy\":{\"maxConnections\":10000,\"maxSessions\":10000}}] \n"
		      "-m <filename>        this program shoule generate the FingerPrintSchema data such as [{\"disks\":2,\"nics\":1,\"processors\":2}]\n"
		);
}
int main(int argc, char* argv[])
{
	bool boolLicense = true;
	char ch;
	std::string filePath;
	while((ch = getopt(argc, argv, "hl::m::")) != EOF)
	{
		switch (ch)
		{
		case 'h':
			{
				usage();
				return 0;
			}
		case 'l':
			{
				boolLicense = true;
				if (optarg != NULL)
					filePath.assign(optarg);
				break;
			}
		case 'm':
			{
				boolLicense = false;
				if(optarg != NULL)
					filePath.assign(optarg);
				break;
			}
		default:
			{
				break;
			}
		}
	}
	if (boolLicense)
	{
		char serviceName[256];
		memset(serviceName, '\0', 256);
		int maxSessions, maxConnections, expireDays;
		printf("please enter the expire days:\n");
		if(0 ==scanf("%d",&expireDays))
		{
			printf("enter the expire days error, the data should be int \n");
			return 0;
		}
		printf("please enter the service name:\n");
		if(0 ==scanf("%s",serviceName))
		{
			printf("enter the service name error, the data should be string\n");
			return 0;
		}
		printf("please enter the maxSessionsNum:\n");
		if (0 == scanf("%d", &maxSessions))
		{
			printf("enter the maxSessionsNum error, the data should be int\n");
			return 0;
		}
		printf("please enter the maxConnectiosnsNum:\n");
		if(0 == scanf("%d", &maxConnections))
		{
			printf("enter the maxConnectiosnsNum error, the data shoule be int\n");
			return 0;
		}
		char timeBuffer[64];
		memset(timeBuffer,'\0',64);
		int64 cTime = ((int64)TIMEOFDAY * expireDays);
		int64 eTime = ZQ::common::TimeUtil::now() + cTime;
		ZQ::common::TimeUtil::TimeToUTC(eTime,timeBuffer,sizeof(timeBuffer) - 1);
		Json::Value sessionValue;
		sessionValue["maxSessions"] = maxSessions;
		sessionValue["maxConnections"] = maxConnections;
		Json::Value JsonValue;
		JsonValue[serviceName] = sessionValue;
		JsonValue["expiration"] = std::string(timeBuffer);
		std::string licenseData = Json::FastWriter().write(JsonValue);
		if (filePath.empty())
			filePath = "license.txt";
		writeFile(filePath, licenseData);
	}
	else
	{
		int inData;
		Json::Value jsonFingerValue;
		Json::Value procValue;
		printf("please enter the cpu pos:\n");
		while(0 != scanf("%d", &inData))
		{
			procValue.append(inData);
		}
		jsonFingerValue["CPU"] = procValue;
		Json::Value nicValue;
		char t[32];
		gets(t);
		printf("please enter the nic pos:\n");
		while(0 != scanf("%d", &inData))
		{
			nicValue.append(inData);
		}
		jsonFingerValue["NIC"] = nicValue;
		Json::Value diskValue;
		gets(t);
		//fflush(stdin);
		printf("please enter the disk pos:\n");
		while(0 != scanf("%d", &inData))
		{
			diskValue.append(inData);
		}
		jsonFingerValue["DISK"] = diskValue;
		gets(t);
		//fflush(stdin);
		Json::FastWriter writer;
		std::string sysData = writer.write(jsonFingerValue);
		if (filePath.empty())
			filePath = "devInfo.txt";
		writeFile(filePath, sysData);
	}
	return 0;
}