#include "FingerPrint.h"
#include "FileLog.h"
#include "TimeUtil.h"
#include <json/json.h>


using namespace ZQ::common;

bool writeFile(const std::string& fileName, const std::string& data)
{
	FILE* file = fopen(fileName.c_str(),"wb");
	if (file == NULL)
	{
		fclose(file);
		return false;
	}
	int res = fwrite(data.c_str(), sizeof(unsigned char), data.length() * sizeof(unsigned char), file);
	if (res < data.length())
	{
		fclose(file);
		return false;
	}
	fclose(file);
	return true;
}
bool readFile(const std::string& fileName, std::string& data)
{
	data.clear();
	FILE* file = fopen(fileName.c_str(),"rb");
	if (file == NULL)
	{
		fclose(file);
		return false;
	}
	unsigned char* buffer = new unsigned char[512];
	memset(buffer,0,512);
	int res = 0;
	while((res = fread(buffer, sizeof(unsigned char), 512, file)) != 0)
	{
		data.append((char *)buffer, res);
	}
	fclose(file);
	delete[] buffer;
	return true;
}
int main()
{
	ZQ::common::FileLog fingerPrintLog("FingerPrint.log",7);
	MachineFingerPrint machinePrint(fingerPrintLog);
	Json::Value jsonFingerValue;
	
	Json::Value processValue;
	processValue.append(0);
	processValue.append(1);
	jsonFingerValue["CPU"] = processValue;

	Json::Value nicValue;
	nicValue.append(0);
	nicValue.append(2);
	jsonFingerValue["NIC"] = nicValue;
	
	Json::Value diskValue;
	diskValue.append(1);
	diskValue.append(2);
	jsonFingerValue["DISK"] = diskValue;

	Json::FastWriter writer;
	std::string jsonFingerPrintSchema = writer.write(jsonFingerValue);
	LicenseGenerater licenseGener(fingerPrintLog, jsonFingerPrintSchema);
	//LicenseGenerater::generateKey();
	std::string machineData = machinePrint.getFingerPrint();
 	std::string outFile1= "outFile.bin";
  	writeFile(outFile1,machineData);
 	std::string outData;
 	readFile(outFile1,outData);
	int64 timeNow = ZQ::common::TimeUtil::now();
	timeNow += 60000 * 5;
	char timeBuffer[64];
	memset(timeBuffer,'\0',64);
	ZQ::common::TimeUtil::TimeToUTC(timeNow,timeBuffer,sizeof(timeBuffer) - 1);
	Json::Value licenseValue;
	Json::Value serviceValue;
	serviceValue["maxSessions"] = 6;
	serviceValue["maxConnections"] = 10;
	licenseValue["rtspProxy"] = serviceValue;
	licenseValue["expiration"] = timeBuffer;
	std::string jsonLicense = Json::FastWriter().write(licenseValue);
	std::string licenseData = licenseGener.issue(outData,jsonLicense);
	std::string outFile = "license.bin";
	writeFile(outFile,licenseData);
	licenseData.clear();
	readFile(outFile,licenseData);
	std::string result = machinePrint.loadLicense(licenseData);
	std::cout<<result<<std::endl;
	return 0;
}