// tslicsign.cpp : Defines the entry point for the console application.
//
#include "LicenseGenerate.h"
#include <TimeUtil.h>

#define  TIMEOFDAY (24 * 3600 * 1000)
#define  MOLOG (_licenseLog)
//write file
bool LicenseGenerCmd::writeFile(const std::string& fileName, const std::string& data)
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
//read file
bool LicenseGenerCmd::readFile(const std::string& fileName, std::string& data)
{
	data.clear();
	FILE* file = fopen(fileName.c_str(),"rb");
	if (file == NULL)
	{
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

void LicenseGenerCmd::showSigData()
{
	std::string data = _pLicenseGen->getSigData(_signatureData);
	printf("the signature data is : \n%s\n", data.c_str());
}
std::string LicenseGenerCmd::issueLicenseData()
{
	if (_signatureData.empty() || _licenseData.empty())
	{
		MOLOG(ZQ::common::Log::L_ERROR,CLOGFMT(LicenseGenerCmd,"issueLicenseData() the signature data or the license data is empty"));
		return "";
	}
	return _pLicenseGen->issue(_signatureData,_licenseData);
}
//print usage
void usage()
{
	printf("display the help information of the command line:\n");
	printf(	"-h                              display this screen\n"
			"-d                              display the signature data\n"
			"-i <clear_license_file>         specify the name of the input clear licensefile\n"
			"-o <signed_license_file>        specify the name of the output license file\n"
			"-s <signature_file>             specify the name of the input signature file\n"
			"-m <schema_file>                specify the name of the input schema file\n"
			"-u                              no expire time for license\n"
			"-e <expire_time>                specify the expire time\n\
			        example: -e +100 specify will expire in 100 days\n"

		);
}
std::string getSchemaStr(std::string& filePath)
{
	std::string strResult;
	strResult.clear();
	if (!filePath.empty())
		LicenseGenerCmd::readFile(filePath, strResult);
	if (strResult.empty())
	{
		Json::Value jsonFingerValue;
		Json::Value procValue;
		procValue.append(0);
		procValue.append(1);
		jsonFingerValue["CPU"] = procValue;
		Json::Value nicValue;
		nicValue.append(0);
		nicValue.append(2);
		jsonFingerValue["NIC"] = nicValue;
		Json::Value diskValue;
		diskValue.append(1);
		diskValue.append(2);
		jsonFingerValue["DISK"] = diskValue;
		Json::FastWriter writer;
		strResult = writer.write(jsonFingerValue);
	}
	return strResult;
}
std::string getLicenseStr(std::string& filePath, std::string& expireTime, bool forever = false)
{
	std::string strResult;
	strResult.clear();
	LicenseGenerCmd::readFile(filePath, strResult);
	if (forever)
	{
		std::string foreverTime = "2100-12-22T23:00:00+08:00";
		Json::Value licenseValue;
		Json::Reader().parse(strResult, licenseValue);
		licenseValue["expiration"] = foreverTime;
		strResult = Json::FastWriter().write(licenseValue);
	}
	else if (!expireTime.empty())
	{
		size_t nPos = expireTime.find_first_of('+');
		if (nPos != std::string::npos)
			expireTime.erase(expireTime.begin(), expireTime.begin() + nPos);
		int addTime = atoi(expireTime.c_str());
		int64 eTime = ZQ::common::TimeUtil::now() + (int64) TIMEOFDAY * addTime;
		char timeBuffer[64];
		memset(timeBuffer,'\0',64);
		ZQ::common::TimeUtil::TimeToUTC(eTime,timeBuffer,sizeof(timeBuffer) - 1);
		Json::Value licenseValue;
		Json::Reader().parse(strResult, licenseValue);
		licenseValue["expiration"] = std::string(timeBuffer);
		strResult = Json::FastWriter().write(licenseValue);
	}
	return strResult;
}
int main(int argc, char* argv[])
{
	/*if( ARGC_I_S_O >= argc )
	{
		usage();
		return 1;
	}*/
	char ch;
	bool showSigData = false , forever = false;
	std::string licenseFile, signatureFile, systemFile, outPutFile, expireTime;
	while((ch = getopt(argc, argv, "hdui:s:o:m:e:")) != EOF)
	{
		switch (ch)
		{
		case 'h':
			{
				usage();
				return 0;
			}
		case 'd':
			{
				showSigData = true;
				break;
			}
		case 'i':
			{
				licenseFile.assign(optarg);
				break;
			}
		case 's':
			{
				signatureFile.assign(optarg);
				break;
			}
		case 'o':	
			{			
				outPutFile .assign(optarg);
				break;
			}
		case 'm':
			{
				systemFile.assign(optarg);
				break;
			}
		case 'e':
			{
				expireTime.assign(optarg);
				break;
			}
		case 'u':
			{
				forever = true;
				break;
			}
		default:
			{
				printf("unknown option: %c\n", (char)optopt);
				break;
			}
		}
	}
	if (signatureFile.empty())
	{
		printf("the signature file is not exist.\n");
		usage();
		return 0;
	}
	ZQ::common::FileLog fingerPrintLog("tslicsign.log",7);
	std::string  jsonFingerPrintSchema = getSchemaStr(systemFile);
	LicenseGenerCmd licGenerCMD(fingerPrintLog, jsonFingerPrintSchema);
	std::string data = "";
	LicenseGenerCmd::readFile(signatureFile, data);
	licGenerCMD.setSignatureData(data);
	if (showSigData)
		licGenerCMD.showSigData();
	if (licenseFile.empty() || outPutFile.empty())
	{
		if (showSigData)
			return 0;
		usage();
		return 0;
	}
	data.clear();
	data = getLicenseStr(licenseFile, expireTime,forever);
	licGenerCMD.setLicenseData(data);
	data.clear();
	data = licGenerCMD.issueLicenseData();
	if (data.empty())
		printf("failed to issue the license data \n");
	else
	{
		if (!LicenseGenerCmd::writeFile(outPutFile,data))
			printf("failed to write the licnse file of path [%s] \n",outPutFile.c_str());
	}
	return 0;
}

