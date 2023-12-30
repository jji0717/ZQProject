#ifndef __SIGNATUREGENERATE_H__
#define __SIGNATUREGENERATE_H__
#include "FingerPrint.h"
#include "SystemInfo.h"
#include "getopt.h"
#include "FileLog.h"
extern "C" {
#include<stdio.h>
}
#define ARGC_H   2
#define ARGC_G_O 4

class SignatureGenerCmd
{
public:
	SignatureGenerCmd(ZQ::common::Log& log):_sigLog(log)
	{
		_sigData.clear();
		_pFingerPrint = new ZQ::common::MachineFingerPrint(_sigLog);
	}
	 ~SignatureGenerCmd()
	 {
		 if (_pFingerPrint != NULL)
			 delete _pFingerPrint;
		 _pFingerPrint = NULL;
	 }

	//issue license and save result in Sig_Licence
	std::string getSigData(){return _sigData;}
	
	void getFingerPrint();

	static bool writeFile(const std::string& fileName, const std::string& data);
	static bool readFile(const std::string& fileName, std::string& data);


#ifdef _SHOWLICENSE
	std::string loadLicense(std::string& data);
#endif

protected:
	std::string								_sigData;
	ZQ::common::Log&					_sigLog;
	ZQ::common::MachineFingerPrint*		_pFingerPrint;
};



#endif