// FtpXferEx.h: interface for the FtpXferEx class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FTPXFEREX_H__1C1BE8CD_59F5_446C_BF07_13CBAD4F6CFA__INCLUDED_)
#define AFX_FTPXFEREX_H__1C1BE8CD_59F5_446C_BF07_13CBAD4F6CFA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FtpsXfer.h"

namespace ZQTianShan 
{
	namespace ContentProvision
	{
		class FileIoFactory;
		class IOInterface;
	}
}

using namespace ZQTianShan::ContentProvision;

class FtpXferEx : public FtpsXfer  
{
public:
	FtpXferEx(FtpConnection& ftps, FtpSite& site, FtpSock* pasv_sock, FtpSock *port_sock, ZQ::common::NativeThreadPool& Pool, context_t* pContext=NULL);
	virtual ~FtpXferEx();

	virtual bool sendFile();	

	virtual bool sendList();


	int64 GetFileSize(const char* szFile);
	virtual bool init(void);
protected:
	int64 sendFileData();
	//void sendListData(context_t& ctxt,size_t& nbytes);

	//bool buildListLine(std::string& line, const char *fullpath, const char *filename, const int flagdir, const char *thisyear);
//
//#ifdef ZQ_OS_MSWIN
//	bool buildListLine(std::string& line, WIN32_FIND_DATA& data);
//#else
//	
//#endif

private:
	FileIoFactory*    _pFileIoFactory;
	IOInterface*      _pIOInterface;

};

#endif // !defined(AFX_FTPXFEREX_H__1C1BE8CD_59F5_446C_BF07_13CBAD4F6CFA__INCLUDED_)
