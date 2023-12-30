

#ifndef _CPE_FTP_EXPORT_ACCOUNT_MGR_
#define _CPE_FTP_EXPORT_ACCOUNT_MGR_

#include <string>

class ExportAccountGen
{
public:
	//@nExpiredTime time(), in seconds
	static void generate(const std::string& strFile, int nExpiredTime, int nTransferBitrate, std::string& strUser, std::string& strPass);

	//verify account
	static bool verify(const std::string& strUser, const std::string& strPass);

	//verify if we could download this file by this account
	static bool verify(const std::string& strUser, const std::string& strPass, const std::string& strFile, int& nExpiredTime, int& nTransferBitrate);
private:
	static char nUserMd5Index[8];
	static char nPassMd5Index[8];
	static int nTimeEncKey;
};


#endif
