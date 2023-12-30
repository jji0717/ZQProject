#ifndef __LICENSEGENERATE_H__
#define __LICENSEGENERATE_H__
#include "FingerPrint.h"
#include "SystemInfo.h"
#include "getopt.h"
#include "FileLog.h"

#include <json/json.h>
extern "C" {
#include <string.h>
#include<stdio.h>
}

#define ARGC_H 2
#define ARGC_I_S_O 7

class LicenseGenerCmd
{
public:
	LicenseGenerCmd(ZQ::common::Log& log, std::string jsonFingerPrintSchema):_licenseLog(log)
	{
		_pLicenseGen = new ZQ::common::LicenseGenerater(log, jsonFingerPrintSchema);
	}
	~LicenseGenerCmd()
	{
		if (_pLicenseGen != NULL)
			delete _pLicenseGen;
		_pLicenseGen = NULL;
	}

	//issue license and save result in Sig_Licence
	std::string issueLicenseData();

	void setSignatureData(std::string& data){_signatureData = data;}
	void setLicenseData(std::string& data){_licenseData = data;}
	void showSigData();
		
	static bool writeFile(const std::string& fileName, const std::string& data);
	static bool readFile(const std::string& fileName, std::string& data);

protected:
	std::string _signatureData;
	std::string _licenseData;

	ZQ::common::Log&					_licenseLog;
	ZQ::common::LicenseGenerater*		_pLicenseGen;
};



#endif