// FtpClientTest.cpp : Defines the entry point for the console application.
//
#include "FtpClient.h"
#include "FTPMSClient.h"
#include "urlstr.h"
#include "FileLog.h"
using namespace ZQTianShan::ContentProvision;
int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		return 0;
	}
	std::string ftpurl = argv[1];
	std::string outputfile = argv[2];

	FTPClient::FTPMode mode = FTPClient::active;
	if(argc > 3)
		mode = (FTPClient::FTPMode)atoi(argv[3]);

	ZQ::common::URLStr srcUrl(ftpurl.c_str());
	std::string strServer = srcUrl.getHost();
	std::string strOrgFile = srcUrl.getPath();
	std::string strUserName = srcUrl.getUserName();
	std::string strPassword = srcUrl.getPwd();
	int nPort = srcUrl.getPort();

	std::string logPath;
#ifdef ZQ_OS_MSWIN
	logPath = "c:\\ftpclientTest.log";
#else
	logPath = "/opt/TianShan/logs/ftpclientTest.log";
#endif

	ZQ::common::FileLog filelog;
	filelog.open((char*)logPath.c_str(), ZQ::common::Log::L_DEBUG);

	std::auto_ptr<FTPClient>	 _pFTPDownloader;
	FTPMSClient* pFtpclient = new FTPMSClient();
	if(!pFtpclient)
		return 0; 
	_pFTPDownloader.reset(pFtpclient); 

	_pFTPDownloader->setLog(&filelog);
	_pFTPDownloader->setTransmitMode(FTPClient::binary);
	_pFTPDownloader->setFTPMode(mode);
	_pFTPDownloader->setIoTimeout(400000);

	if (!_pFTPDownloader->open(strServer, nPort, strUserName, strPassword, "")) // open fail
	{
		std::string strErr = "failed to connect to url " + ftpurl + " with error: " + _pFTPDownloader->getLastError();
		printf("failed to connnet to %s\n", ftpurl.c_str());
	}
	else if(!_pFTPDownloader->downloadFile(strOrgFile, outputfile))
	{
		printf("failed to download file\n");
	}
	else
	{
		printf("download file success\n");
	}

//	_pFTPDownloader = NULL;
	return 0;
}

//ftp://SeaChange:SeaChange@10.50.4.32/a.exe d:\\a.exe  0
//ftp://wm:itv@192.168.81.15/C2FE.xml d:\\C2FE.xml  0									Quick 'n Easy FTP
//ftp://izq:6e8ezp36@192.168.87.8/2015-11-26/HeNan/NSS5.zip d:\\NSS5.zip 0				Serv-U_FTP
//ftp://wzg:abc@192.168.81.7/sdk/set_main.bat c:\\set_main.bat 0						FileZilla



