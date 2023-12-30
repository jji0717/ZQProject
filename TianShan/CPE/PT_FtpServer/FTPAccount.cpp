

#include "FTPAccount.h"
#include "MD5CheckSumUtil.h"

using namespace ZQ::common;

char ExportAccountGen::nUserMd5Index[8] = {23,3,6,7,30,14,19,9};
char ExportAccountGen::nPassMd5Index[8] = {20,11,16,1,8,14,29,6};
int ExportAccountGen::nTimeEncKey = 0xb7e791c6;


//@nExpiredTime time(), in seconds
void ExportAccountGen::generate(const std::string& strFile, int nExpiredTime, int nTransferBitrate, std::string& strUser, std::string& strPass)
{
	MD5ChecksumUtil md5;
	char szUser[24];
	char szPass[24];
	char szUserPart1[12];
	char szUserPart2[12];
	char szPassPart1[12];
	char szPassPart2[12];

	std::string tempFile = strFile;
	if(strFile.size() > 20)
	{
       tempFile = strFile.substr(0, 20);
	}
	md5.checksum(tempFile.c_str(), tempFile.length());
	const char* pMD5 = md5.lastChecksum();

	unsigned int i;
	for(i=0;i<8;i++)
		szUserPart1[i] = pMD5[nUserMd5Index[i]];

	sprintf(szUserPart2, "%08x", nExpiredTime ^ nTimeEncKey);
	sprintf(szPassPart2, "%08x", nTransferBitrate ^ nTimeEncKey);
	for(i=0;i<8;i++)
	{
		szUser[i*2] = szUserPart1[i];
		szUser[i*2+1] = szUserPart2[i];
	}
	szUser[i*2] = '\0';
	strUser = szUser;

	md5.reset();
	md5.checksum(szUser, 16);
	md5.checksum(szPassPart2, 8);
	pMD5 = md5.lastChecksum();
	for(i=0;i<8;i++)
		szPassPart1[i] = pMD5[nPassMd5Index[i]];
	szPassPart1[i] = '\0';

	for(i=0;i<8;i++)
	{
		szPass[i*2] = szPassPart1[i];
		szPass[i*2+1] = szPassPart2[i];
	}
	szPass[i*2] = '\0';
	strPass = szPass;	
}

//verify account
bool ExportAccountGen::verify(const std::string& strUser, const std::string& strPass)
{
	if (strUser.length()!=16 || strPass.length()!=16)
		return false;

	char szPassMD5[12];
	unsigned int i;
	char szPassPart1[12];
	char szPassPart2[12];

	MD5ChecksumUtil md5;
	md5.checksum(strUser.c_str(), strUser.length());

	const char* pPass = strPass.c_str();
	for(i=0;i<8;i++)
	{
		szPassPart1[i] = pPass[i*2];
		szPassPart2[i] = pPass[i*2+1];
	}
	szPassPart1[i] = '\0';
	szPassPart2[i] = '\0';

	md5.checksum(szPassPart2, i);

	const char* pMD5 = md5.lastChecksum();
	for(i=0;i<8;i++)
		szPassMD5[i] = pMD5[nPassMd5Index[i]];
	szPassMD5[i] = '\0';

	if (strcmp(szPassMD5, szPassPart1))
		return false;

	return true;
}


//verify if we could download this file by this account
bool ExportAccountGen::verify(const std::string& strUser, const std::string& strPass, const std::string& strFile, int& nExpiredTime, int& nTransferBitrate)
{
	if (strUser.length()!=16 || strPass.length()!=16)
		return false;
	
	char szPassMD5[12];
	unsigned int i;
	char szPassPart1[12];
	char szPassPart2[12];
	char szFileMD5[12];
	char szUserPart1[12];
	char szUserPart2[12];

	
	MD5ChecksumUtil md5;
	md5.checksum(strUser.c_str(), strUser.length());
	
	const char* pPass = strPass.c_str();
	for(i=0;i<8;i++)
	{
		szPassPart1[i] = pPass[i*2];
		szPassPart2[i] = pPass[i*2+1];
	}
	szPassPart1[i] = '\0';
	szPassPart2[i] = '\0';
	
	md5.checksum(szPassPart2, i);
	
	const char* pMD5 = md5.lastChecksum();
	for(i=0;i<8;i++)
		szPassMD5[i] = pMD5[nPassMd5Index[i]];
	szPassMD5[i] = '\0';
	
	if (strcmp(szPassMD5, szPassPart1))
		return false;

	std::string tempFile = strFile;
	if(strFile.size() > 20)
	{
		tempFile = strFile.substr(0, 20);
	}

	md5.reset();
	md5.checksum(tempFile.c_str(), tempFile.length());
	pMD5 = md5.lastChecksum();

	const char* pUser = strUser.c_str();
	for(i=0;i<8;i++)
	{
		szFileMD5[i] = pMD5[nUserMd5Index[i]];
		szUserPart1[i] = pUser[i*2];
		szUserPart2[i] = pUser[i*2+1];
	}
	szFileMD5[i] = '\0';
	szUserPart1[i] = '\0';
	szUserPart2[i] = '\0';

	if(strncmp(szFileMD5, szUserPart1, 8))
		return false;

	int nEncTime;
	nEncTime = strtoul(szUserPart2, 0, 16);

	int nEncBitrate;
	nEncBitrate = strtoul(szPassPart2, 0, 16);

	nExpiredTime = nEncTime ^ nTimeEncKey;
	nTransferBitrate = nEncBitrate ^ nTimeEncKey;
	return true;
}
