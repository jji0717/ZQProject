#include "ContentImpl.h"
#include "FtpClient.h"
#include "FtpListParse.h"


namespace ZQTianShan {
namespace ContentStore {

#define FTPCSLOG if(_pLog) (*_pLog)
#define FTPLIST_DEFINTERVAL    15*60//15 minute
#define FTPLIST_MAXINTERVAL	   60*60//1 hour

class FTPContentInfo : virtual public Ice::LocalObject//IceUtil::Shared
{
public:
	FTPContentInfo()
		:fileSize(0){};

	std::string strName;
	std::string strPath;
	int64		fileSize;	
	typedef ::IceInternal::Handle< FTPContentInfo > Ptr;
};

//class FtpCSControl
class FtpCSControl : public ZQ::common::NativeThread
{
public:
	FtpCSControl(ContentStoreImpl& store);
	virtual ~FtpCSControl();
	
	void setLog(ZQ::common::Log* pLog);
	void setLogInInfo(char* chHostName, int nHostPort, char* chUserName, char* chPass, char* chWorkPath,  char* chLocalIP);
	void getLogInInfo(std::string& strHostName, int& nHostPort, std::string& strUserName, std::string& strPassWord, std::string& strWorkDir, std::string& strLocalIP);
	void setListInterval(int nInterval);
	
	bool listContents(std::string& chPath,std::vector<nsFTP::FILEINFO>& fileIV);
	bool deleteContent(std::string& strDelFile,int& reCode);
	void quit(void);
//	std::string getHostURL(){return _strHost;}
protected:
	virtual bool init(void);
	virtual int run(void);

public:
	HANDLE		_hBegin;
	std::map<std::string,FTPContentInfo::Ptr> _contentsM;//map for contents
	ZQ::common::Mutex	_lock;
private:
	ContentStoreImpl&	_store;
	bool		_bQuit;	
	HANDLE		_hStop;
	int			_nInterval;//thread get contents time interval,unit is second
	ZQ::common::Log* _pLog;


	//login info
	std::string _strHostName;
	int			_nHostPort;
	std::string _strUser;
	std::string _strPass;
	std::string	_strLocalIP;
	std::string _strWorkPath;
};

FtpCSControl::FtpCSControl(ContentStoreImpl& store)
:_store(store),_bQuit(false),_hBegin(NULL),_hStop(NULL),_nHostPort(0),_nInterval(FTPLIST_DEFINTERVAL)
{
}

FtpCSControl::~FtpCSControl()
{
	quit();
}

bool FtpCSControl::init()
{
	if(_hStop == NULL)
		_hStop = CreateEvent(NULL,false,false,NULL);
	if(_hBegin == NULL)
		_hBegin = CreateEvent(NULL,false,false,NULL);
	return true;
}

int FtpCSControl::run()
{
	DWORD interval = 0;//milsec
	bool bInte = true;
	while(!_bQuit)
	{
		DWORD re = WaitForSingleObject(_hStop,interval);
		if(re == WAIT_OBJECT_0)//quit
			break;
		
		std::vector<nsFTP::FILEINFO> fileIV;
		if(listContents(_strWorkPath,fileIV))
		{
			interval = _nInterval*1000;
			if(bInte)
			{					
				std::string strPathName;
				ZQ::common::MutexGuard mg(_lock);
				_contentsM.clear();
				std::vector<nsFTP::FILEINFO>::iterator it;
				std::string strPP;
//				it = fileIV.begin();
//				if(it < fileIV.end())
//					strPP = it->_strPath;
//				size_t sEnd = strPP.length(); 
//				if(strPP[sEnd-1] != '/' && strPP[sEnd-1] != '\\')
//					strPP += "/";
				for(it = fileIV.begin(); it < fileIV.end(); it++)
				{
					if(!it->_bTryCwd)//is file
					{
						strPathName = strupr((char*)(strPP + it->_strName).c_str());

						FTPContentInfo::Ptr ptr = new FTPContentInfo();
						ptr->strPath = it->_strPath;
						ptr->strName = it->_strName;
						ptr->fileSize = it->_lSize;
						_contentsM.insert(std::pair<std::string,FTPContentInfo::Ptr>(strPathName,ptr));
				
					}
				}
				bInte = false;
				SetEvent(_hBegin);
				FTPCSLOG(ZQ::common::Log::L_DEBUG,"FtpCSControl::run() init content list is OK");
			}
			else
			{
				std::map<std::string,FTPContentInfo::Ptr>::iterator itM;
				std::vector<nsFTP::FILEINFO>::iterator itV;
				std::string strPathName;
				bool bFind = false;

				std::string strPP;
//				itV = fileIV.begin();
//				if(itV < fileIV.end())
//					strPP = itV->_strPath;
//				size_t sEnd = strPP.length(); 
//				if(strPP[sEnd-1] != '/' && strPP[sEnd-1] != '\\')
//					strPP += "/";

				ZQ::common::MutexGuard mg(_lock);
				for(itM = _contentsM.begin(); itM != _contentsM.end(); itM++)
				{
					bFind = false;
					
					for(itV = fileIV.begin(); itV < fileIV.end(); itV++)
					{
						strPathName = strupr((char*)(strPP + itV->_strName).c_str());
						if(stricmp(itM->first.c_str(),strPathName.c_str()) == 0)
						{
							bFind = true;
							fileIV.erase(itV);
							break;
						}							
					}
					//not find
					if(!bFind)
					{
						std::map<std::string,FTPContentInfo::Ptr>::iterator itNew = itM;
						if(itNew == _contentsM.begin())
							itNew = _contentsM.end();
						else
							itNew--;
						strPathName = itM->first;
						itM->second = NULL;
						_contentsM.erase(itM);
						
						if(itNew == _contentsM.end())
							itM = _contentsM.begin();
						else
							itM = itNew;
												
						::TianShanIce::Properties params;
						_store.OnFileEvent(::TianShanIce::Storage::fseFileDeleted, strPathName, params, ::Ice::Current());

					}
				}
				//the leave is created
				for(itV = fileIV.begin(); itV < fileIV.end(); itV++)
				{
					strPathName = strPP + itV->_strName;
					if(!itV->_bTryCwd)//is file then add
					{
						FTPContentInfo::Ptr ptr = new FTPContentInfo();
						ptr->strPath = itV->_strPath;
						ptr->strName = itV->_strName;
						ptr->fileSize = itV->_lSize;
						_contentsM.insert(std::pair<std::string,FTPContentInfo::Ptr>(strPathName,ptr));
					}
				}
			}
		}
		else
			interval = 5*1000;//interval time is 5 second

	}
	return 0;
}

void FtpCSControl::setLog(ZQ::common::Log* pLog)
{
	if(pLog)
		_pLog = pLog;
}

void FtpCSControl::setLogInInfo(char* chHostName, int nHostPort, char* chUserName, char* chPass, char* chWorkPath, char* chLocalIP)
{
	if(chHostName)
		_strHostName = chHostName;
	if(nHostPort <= 0)
		_nHostPort = 21;
	else
		_nHostPort = nHostPort;
	if(chUserName)
		_strUser = chUserName;
	else
		_strUser = "anonymous";
	if(chPass)
		_strPass = chPass;
	else
		_strPass = "anonymous";
	if(chLocalIP)
		_strLocalIP = chLocalIP;
	else
		_strLocalIP = "0.0.0.0";
	if(chWorkPath)
		_strWorkPath = chWorkPath;
}

void FtpCSControl::getLogInInfo(std::string& strHostName, int& nHostPort, std::string& strUserName, std::string& strPassWord, std::string& strWorkDir, std::string& strLocalIP)
{
	strHostName = _strHostName;
	nHostPort = _nHostPort;
	strUserName = _strUser;
	strPassWord = _strPass;
	strWorkDir = _strWorkPath;
	strLocalIP = _strLocalIP;
}
void FtpCSControl::setListInterval(int nInterval)
{
	if(nInterval <= 0 )
		return;
	
	if(nInterval > FTPLIST_MAXINTERVAL)
		_nInterval = FTPLIST_MAXINTERVAL;
	else
		_nInterval = nInterval;
}

bool FtpCSControl::listContents(std::string& strPath,std::vector<nsFTP::FILEINFO>& fileIV)
{
	FtpClient ftpC(_strHostName,_nHostPort,_strUser,_strPass,_strLocalIP);
	ftpC.SetLog(_pLog);

	if(!ftpC.Login())
	{
		FTPCSLOG(ZQ::common::Log::L_ERROR,"FtpCSControl::listContents() login failed,host name '%s',host port '%d',user name '%s',password '%s'",_strHostName.c_str(),_nHostPort,_strUser.c_str(),_strPass.c_str());
		return false;
	}
	
	
	if(strPath.empty())//get work directory
	{
		std::string strWD;
		if(ftpC.PrintWorkingDirectory(strWD))
			strPath = strWD;
	}

	std::string strR;

	if(!ftpC.ExecuteDatachannelCommand(strPath, strR, FtpClient::FTP_LIST, FtpClient::FTP_ASCII))
	{
		FTPCSLOG(ZQ::common::Log::L_ERROR,"FtpCSControl::listContents() execute data command 'LIST' failed");
		ftpC.Logout();
		return false;
	}

	if(strR.empty())
	{
		FTPCSLOG(ZQ::common::Log::L_ERROR,"FtpCSControl::listContents() not get response data");
		ftpC.Logout();
		return false;
	}

	nsFTP::CFTPListParse ftpListP;

	size_t nB = 0;
	size_t nE = 0;

	while((nE = strR.find("\r\n", nB)) != std::string::npos)
	{		
		nsFTP::FILEINFO fileInfo;
		std::string strSL = strR.substr(nB,nE-nB);
		nB = nE + 2;

		if(ftpListP.Parse(fileInfo,strSL))
		{
			fileInfo._strPath = strPath;
			fileIV.push_back(fileInfo);
		}
		else
		{
			FTPCSLOG(ZQ::common::Log::L_WARNING,"FtpCSControl::listContents() can not parse string '%s'",strSL.c_str());
		}
		
	};

	ftpC.Logout();

	if(fileIV.size() == 0)
	{
		FTPCSLOG(ZQ::common::Log::L_ERROR,"FtpCSControl::listContents() can not parse any line form response data");
		return false;
	}

	return true;
}

bool FtpCSControl::deleteContent(std::string& strDelFile,int& reCode)
{
	FtpClient ftpC(_strHostName,_nHostPort,_strUser,_strPass,_strLocalIP);
	ftpC.SetLog(_pLog);

	if(!ftpC.Login())
	{
		FTPCSLOG(ZQ::common::Log::L_ERROR,"FtpCSControl::deleteContent() login failed,host name '%s',host port '%d',user name '%s',password '%s'",_strHostName.c_str(),_nHostPort,_strUser.c_str(),_strPass.c_str());
		return false;
	}

	std::string strR;
	if(!ftpC.Delete(strDelFile,strR))
	{
		reCode = atoi(strR.substr(0,3).c_str());
		FTPCSLOG(ZQ::common::Log::L_WARNING,"FtpCSControl::deleteContent() delete file '%s'",strDelFile.c_str());;
		return false;
	}
	
	return true;

}

void FtpCSControl::quit(void)
{
	_bQuit = true;
	
	if(_hBegin)
	{
		CloseHandle(_hBegin);
		_hBegin = NULL;
	}
	if(_hStop)
	{
		SetEvent(_hStop);
		CloseHandle(_hStop);
		_hStop = NULL;
	}
}



//portal of class ContentStoreImpl
void ContentStoreImpl::initializePortal(ContentStoreImpl& store)
{
	if (store._ctxPortal != NULL)
		return;

	FtpCSControl* pFtpCSCtrl = new FtpCSControl(store);
	store._ctxPortal = reinterpret_cast<void*>(pFtpCSCtrl);
	if (store._ctxPortal ==NULL)
		return;

#pragma message ( __MSGLOC__ "TODO: config item here")
	pFtpCSCtrl->setLog(&(store._log));
	pFtpCSCtrl->setLogInInfo("192.168.81.107",21,"anonymous","anonymous",NULL,"192.168.81.114");
	pFtpCSCtrl->setListInterval(0);

	pFtpCSCtrl->start();
}

void ContentStoreImpl::uninitializePortal(ContentStoreImpl& store)
{
	if (store._ctxPortal !=NULL)
	{
		FtpCSControl* pCtrl = reinterpret_cast<FtpCSControl*>(store._ctxPortal);
		pCtrl->quit();

		delete ((FtpCSControl*)store._ctxPortal);
	}

	store._ctxPortal = NULL;
}

std::string ContentStoreImpl::fixupPathname(ContentStoreImpl& store, const std::string& pathname)
{
	return pathname;
}

void ContentStoreImpl::getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB)
{
}

bool ContentStoreImpl::validateMainFileName(ContentStoreImpl& store, std::string& fileName, const std::string& contentType)
{
	if(fileName.find("\n",0) != std::string::npos || fileName.find(" ",0) != std::string::npos
		|| fileName.find("\r",0) != std::string::npos || fileName.find("\t",0) != std::string::npos
		|| fileName.find("\\",0) != std::string::npos || fileName.find("/",0) != std::string::npos)
		return false;

	fileName = strupr((char*)fileName.c_str());
	return true;
}


std::string ContentStoreImpl::filenameToContentName(ContentStoreImpl& store, const std::string& filename)
{
	return strupr((char*)filename.c_str());;
}

bool ContentStoreImpl::fileExists(ContentStoreImpl& store, const std::string& contentName)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return false;
	}
	FtpCSControl* pCtrl = reinterpret_cast<FtpCSControl*>(store._ctxPortal);
	
	ZQ::common::MutexGuard mg(pCtrl->_lock);
	std::map<std::string,FTPContentInfo::Ptr>::iterator it;
	std::string strName = strupr((char*)contentName.c_str());
	it = pCtrl->_contentsM.find(strName);
	if(it != pCtrl->_contentsM.end())
		return true;

//	for(it = pCtrl->_contentsM.begin(); it != pCtrl->_contentsM.end(); it++)
//		if(stricmp(contentName.c_str(),it->first.c_str()) == 0)
//			return true;

	return false;
}

TianShanIce::StrValues ContentStoreImpl::listMainFiles(ContentStoreImpl& store)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		TianShanIce::StrValues sv;
		return sv;
	}

	FtpCSControl* pCtrl = reinterpret_cast<FtpCSControl*>(store._ctxPortal);
	//wait until get the Content list
	WaitForSingleObject(pCtrl->_hBegin,INFINITE);

	TianShanIce::StrValues strV;
	{
		ZQ::common::MutexGuard mg(pCtrl->_lock);
		std::map<std::string,FTPContentInfo::Ptr>::iterator it;
		for(it = pCtrl->_contentsM.begin(); it != pCtrl->_contentsM.end(); it++)
			strV.push_back(it->first);
	}
	return strV;
}

bool ContentStoreImpl::deleteByContent(ContentStoreImpl& store, const ::std::string& contentName)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return false;
	}

	FtpCSControl* pCtrl = reinterpret_cast<FtpCSControl*>(store._ctxPortal);
	int nCode = 0;
	std::string strName = contentName;
	if(!pCtrl->deleteContent(strName,nCode))
	{
		if(nCode > 0)
		{
			if(nCode >= 400 && nCode < 500)//try again
				return false;
			else
				return true;
		}
		else
			return false;
	}
	
	return true;
}

bool ContentStoreImpl::completeRenaming(ContentStoreImpl& store, const ::std::string& oldName, const ::std::string& newName)
{
	return true;
}

::TianShanIce::Storage::ContentState ContentStoreImpl::populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& contentName)
{
	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return TianShanIce::Storage::csOutService;
	}

	FtpCSControl* pCtrl = reinterpret_cast<FtpCSControl*>(store._ctxPortal);
	std::map<std::string,FTPContentInfo::Ptr>::iterator it;
	ZQ::common::MutexGuard mg(pCtrl->_lock);
	it = pCtrl->_contentsM.find(contentName);
	if(it == pCtrl->_contentsM.end())//not the content
		return ::TianShanIce::Storage::csOutService;
		
	if(it->second == NULL)//the content not attribute
		return ::TianShanIce::Storage::csInService;

	char chFom[30] = {0};
	if(it->second->fileSize)
	{			
		sprintf(chFom,"%lld",it->second->fileSize);
		content.metaData[METADATA_FileSize] = chFom;
//		memset(chFom,0,sizeof(chFom));
	}

	return TianShanIce::Storage::csInService;
}


//provision implement
TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::submitProvision(ContentStoreImpl& store, ContentImpl& content,
						const ::std::string& sourceUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	return NULL;
}

TianShanIce::ContentProvision::ProvisionSessionPrx ContentStoreImpl::bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content,
						::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate)
			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{
	return NULL;
}
std::string ContentStoreImpl::getExportURL(ContentStoreImpl& store, ContentImpl& content, const ::std::string& targetCSType, const ::std::string& transferProtocol, ::TianShanIce::Properties& params)
{
	if(stricmp(transferProtocol.c_str(),"ftp") != 0)
	{
		store._log(ZQ::common::Log::L_ERROR,"The protocol is not ftp");
		return "";
	}

	if(store._ctxPortal == NULL)
	{
		store._log(ZQ::common::Log::L_ERROR,"The point to the portal is NULL");
		return "";
	}

	FtpCSControl* pCtrl = reinterpret_cast<FtpCSControl*>(store._ctxPortal);
	std::string strName = content.ident.name;
	std::string strHost,strUser,strPass,strWork,strIP;
	int nPort = 0;
	pCtrl->getLogInInfo(strHost,nPort,strUser,strPass,strWork,strIP);
	if(strWork[strWork.length()-1] != '/' && strWork[strWork.length()-1] != '\\')
		strWork += "/";
	char chPort[10] = {0};
	sprintf(chPort,"%d",nPort);
	std::string url = transferProtocol + "://" + strUser + "." + strPass + "@" + strHost + ":" + chPort + strWork + strName;
	
	return url;
}

void ContentStoreImpl::cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx)
		throw (::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt)
{

}

}
}
