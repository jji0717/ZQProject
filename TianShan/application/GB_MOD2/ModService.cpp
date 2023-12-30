#include "ModService.h"

#ifdef ZQ_OS_MSWIN
#include <minidump.h>
#include <io.h>
#endif

#include "ModCfgLoader.h"
#include <sstream>
#include "TianShanIceHelper.h"
#include "FileSystemOp.h"

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType=1110;
DWORD gdwServiceInstance=1;

ZQ::common::MiniDump			_crashDump;
#else
extern const char* DUMP_PATH;
#endif

ZQMODApplication::ModService gDummyMODServiceInstance;
extern ZQ::common::BaseZQServiceApplication *Application = &gDummyMODServiceInstance;

ZQ::common::Config::Holder<ModCfg> gNewCfg("");
ZQ::common::Config::Loader<MODCFGGROUP> gNewCfgGroup("GBMovieOnDemand.xml");

ZQ::common::Config::ILoader *configLoader = &gNewCfgGroup;

namespace ZQMODApplication
{

//#define err_300 300
#define err_317 317
#define err_318 318

void dumpLine(const char* line, void* pCtx)
{
	glog(ZQ::common::Log::L_DEBUG, line);
}
bool GenerateDBConfig(::std::string dbEnvPath, 
							  int	cacheSize	= 160*1024*1024,
							  int		cacheBlock	= 1,
							  int	maxlocks	= 100 * 1000,
							  int	maxObjs		= 100 * 1000, 
							  int	maxLockers	= 100 * 1000 )
{
	std::ostringstream oss;
	oss<<"set_lk_max_locks " << maxlocks <<"\n";
	oss<<"set_lk_max_objects " << maxObjs <<"\n";
	oss<<"set_lk_max_lockers " << maxLockers << "\n";
	oss<<"set_cachesize 0 " << cacheSize << " "<< cacheBlock << "\n";
	std::string str = oss.str();
	std::string confPath = ZQTianShan::Util::fsConcatPath( dbEnvPath , "DB_CONFIG" );
	if( -1 == ::access( confPath.c_str(), 0 ) )
	{
		FILE * fConf = fopen( confPath.c_str() , "w+b");
		if( !fConf) return false;
		fwrite( str.c_str() , 1, str.length() ,fConf );
		fclose( fConf );
	}
	return true;
}

/*
bool makeDir(const char* str)
{
	std::string dbPath = (NULL != str) ? str : "";
	if (dbPath.size() == 0)
		return false;
	if (dbPath[dbPath.size() - 1] == '\\' || dbPath[dbPath.size() - 1] == '/')
		dbPath[dbPath.size() - 1] = '\0';
	std::vector<std::string> paths;
	std::string tmp_path;
	paths.push_back(dbPath);
	tmp_path = String::getLeftStr(dbPath, "\\/", false);
	while (tmp_path.size())
	{
		paths.push_back(tmp_path);
		tmp_path = String::getLeftStr(tmp_path, "\\/", false);
	}
	int paths_size = paths.size();
	for (int i = paths_size - 1; i >= 0; i--)
	{
		::CreateDirectoryA(paths[i].c_str(), NULL);
	}
	return true;
}
*/

#ifdef ZQ_OS_MSWIN
bool isDir(const char* str)
{
	bool bDir = false;
	std::string dbPath = (NULL != str) ? str : "";
	if (dbPath.size() == 0)
		return false;
	if (dbPath[dbPath.size() - 1] == '\\' || dbPath[dbPath.size() - 1] == '/')
		dbPath[dbPath.size() - 1] = '\0';
	WIN32_FIND_DATA filedata; 
	HANDLE hHandle;
	hHandle = ::FindFirstFile(dbPath.c_str(), &filedata);
	if(hHandle != INVALID_HANDLE_VALUE && (filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		bDir = true;
	::FindClose(hHandle);
	return bDir;
}
#endif

std::string String::leftStr(const std::string& cstStr, int pos)
{
	int size = cstStr.size();
	if (size -1 < pos)
	{
		return cstStr;
	}
	int cur = 0;
	std::string strRet;
	for (; cur < pos; cur++)
	{
		strRet += cstStr[cur];
	}
	return strRet;
}

std::string String::getLeftStr(const std::string& cstStr, const std::string& splitStr, bool first/* = true*/)
{
	std::string::size_type null_pos = -1, find_pos = -1;
	if (first)
	{
		find_pos = cstStr.find_first_of(splitStr);
		if (find_pos != null_pos) 
		{
			return (leftStr(cstStr,find_pos));
		}
		else 
		{
			return std::string("");
		}
	}
	else 
	{
		find_pos = cstStr.find_last_of(splitStr);
		if (find_pos != null_pos) 
		{
			return (leftStr(cstStr,find_pos));
		}
		else 
		{
			return std::string("");
		}
	}
}

std::string String::rightStr(const std::string& cstStr, int pos)
{
	if (pos <= -1)
	{
		return cstStr;
	}
	std::string strRet;
	int len = cstStr.size();
	for (int cur = pos + 1; cur < len; cur++)
	{
		strRet += cstStr[cur];
	}
	return strRet;
}

std::string String::getRightStr(const std::string& cstStr, const std::string& splitStr, bool first/* = true*/)
{
	std::string::size_type null_pos = -1, find_pos = -1;
	if (first)
	{
		find_pos = cstStr.find_first_of(splitStr);
		if (find_pos != null_pos)
		{
			return rightStr(cstStr,find_pos);
		}
		else
		{
			return std::string("");
		}
	}
	else
	{
		find_pos = cstStr.find_last_of(splitStr);
		if (find_pos != null_pos)
		{
			return rightStr(cstStr,find_pos);
		}
		else
		{
			return std::string("");
		}
	}
}

std::string String::getPath(const std::string& cstStr)
{
	return getLeftStr(cstStr, "\\/",false);
}

std::string String::midStr(const std::string& cstStr, int f_pos, int l_pos)
{
	if (f_pos >= l_pos)
	{
		return "";
	}
	if (f_pos < -1)
	{
		f_pos = -1;
	}
	int size = cstStr.size();
	if (l_pos > size)
	{
		l_pos = size;
	}
	int cur = f_pos + 1;
	std::string strRet;
	for (; cur < l_pos; cur++) {
		strRet += cstStr[cur];
	}
	return strRet;
}

void String::splitStr(const std::string& cstStr, const std::string split, std::vector<std::string>& strVect)
{
	std::string tmp;
	strVect.clear();
	std::string::size_type null_pos = -1, find_pos = -1, last_find_pos = -1;
	find_pos = cstStr.find_first_of(split);
	while (find_pos != null_pos)
	{
		tmp = midStr(cstStr, last_find_pos, find_pos);
		if (tmp.size() > 0)
			strVect.push_back(tmp);
		last_find_pos = find_pos;
		find_pos = cstStr.find_first_of(split, last_find_pos + 1);
	}
	tmp = midStr(cstStr, last_find_pos, cstStr.size());
	if (tmp.size() > 0)
		strVect.push_back(tmp);
}

std::string String::replaceChar(std::string& Str, const char& from, const char& to)
{
	int size = Str.size();
	for (int i = 0; i < size; i++)
	{
		if (Str[i] == from)
		{
			Str[i] = to;
		}
	}
	return Str;
}

std::string String::replaceChars(std::string& Str, const std::string& from, const char& to)
{
	int size = from.size();
	for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
	{
		replaceChar(Str, from[tmp_cur], to);
	}
	return Str;
}

std::string String::replaceStr(std::string& Str, const std::string& from, const std::string& to)
{
	int size = Str.size();
	int f_size = from.size();
	char* Str_copy = new char[size+1];
	strcpy(Str_copy, Str.c_str());
	Str = "";
	char* tmp_copy = Str_copy;
	char* ret_str = strstr(tmp_copy, from.c_str());
	while (ret_str != NULL)
	{
		char* temp = NULL;
		temp = tmp_copy;
		if (ret_str != temp)
		{
			char* tmp_buff = new char[ret_str - temp + 1];
			strncpy(tmp_buff, temp, ret_str - temp);
			tmp_buff[ret_str - temp] = '\0';
			Str += tmp_buff;
			delete[] tmp_buff;
		}
		Str += to;
		tmp_copy = ret_str + f_size;
		ret_str = strstr(tmp_copy, from.c_str());
	}
	if (strlen(tmp_copy))
		Str += tmp_copy;
	delete[] Str_copy;
	return Str;
}

// Check if the std::string "Str" has any of the char in std::string "identifier", if true pos will store the
// position of the first char which is equal to some one of the std::string "idetifier"
bool String::hasChars(const std::string& Str, const std::string& idetifier, int& pos)
{
	std::string::size_type null_pos = -1;
	std::string::size_type find_pos = -1;
	find_pos = Str.find_first_of(idetifier);
	if (find_pos != null_pos)
	{
		pos = find_pos;
		return true;
	}
	else 
	{
		pos = -1;
		return false;
	}
}

// if ch can be found in Str, the function will return true, otherwise false.
// and pos will be set with value -1 if not found, or other value more than -1
// if ch can be found.
bool String::hasChar(const std::string& Str, const char& ch, int& pos)
{
	int size = Str.size();
	for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
	{
		if (Str[tmp_cur] == ch)
		{
			pos = tmp_cur;
			return true;
		}
	}
	pos = -1;
	return false;
}

// if ch can be found in Str, all the found position will be stored into vector which stores int values.
// and function will return true, otherwise there will be no items in poss and function will return false.
bool String::hasChar(const std::string& Str, const char& ch, std::vector<int>& poss)
{
	poss.clear();
	int size = Str.size();
	for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
	{
		if (Str[tmp_cur] == ch)
		{
			poss.push_back(tmp_cur);
		}
	}
	if (poss.size())
		return true;
	return false;
}

void String::removeChars(std::string& Str, const std::string& idetifier)
{
	std::string copy_str(Str);
	Str = "";
	int size = copy_str.size();
	for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
	{
		int ident_size = idetifier.size();
		bool bFound = false;
		for (int i = 0; i < ident_size; i ++)
		{
			if (copy_str[tmp_cur] == idetifier[i])
			{
				bFound = true;
				break;
			}
		}
		if (false == bFound)
			Str += copy_str[tmp_cur];
	}
}

void String::removeChar(std::string& Str, const char& ch)
{
	std::string copy_str(Str);
	Str = "";
	int size = copy_str.size();
	for (int tmp_cur = 0; tmp_cur < size; tmp_cur++)
	{
		if (copy_str[tmp_cur] != ch)
			Str += copy_str[tmp_cur];
	}
}

std::string String::nLeftStr(const std::string& cstStr, int num)
{
	return leftStr(cstStr, num);
}

std::string String::nRightStr(const std::string& cstStr, int num)
{
	return rightStr(cstStr, cstStr.size() - num - 1);
}

void String::trimRight(std::string& Str)
{
	std::string::size_type null_pos = -1;
	std::string::size_type find_pos = -1;
	find_pos = Str.find_last_not_of(" ");
	if (find_pos != null_pos)
		Str = midStr(Str, -1, find_pos + 1);
}

void String::trimLeft(std::string& Str)
{
	std::string::size_type null_pos = -1;
	std::string::size_type find_pos = -1;
	find_pos = Str.find_first_not_of(" ");
	if (find_pos != null_pos)
		Str = rightStr(Str, find_pos - 1);
}

void String::trimAll(std::string& Str)
{
	trimLeft(Str);
	trimRight(Str);
}

std::string String::getTrimRight(const std::string& Str)
{
	std::string::size_type null_pos = -1;
	std::string::size_type find_pos = -1;
	find_pos = Str.find_last_not_of(" ");
	if (find_pos != null_pos)
		return midStr(Str, -1, find_pos + 1);
	else 
		return "";
}

std::string String::getTrimLeft(const std::string& Str)
{
	std::string::size_type null_pos = -1;
	std::string::size_type find_pos = -1;
	find_pos = Str.find_first_not_of(" ");
	if (find_pos != null_pos)
		return rightStr(Str, find_pos - 1);
	else 
		return "";
}

std::string String::getTrimAll(const std::string& Str)
{
	std::string tmpStr(getTrimLeft(Str));
	return getTrimRight(tmpStr);
}


//////////////////////////////////////////////////////////////////////////
// Implement ModEnv
//////////////////////////////////////////////////////////////////////////
ModEnv::ModEnv() : _iceComm(NULL), _iceAdap(NULL), _pIceLog(NULL), _logForIce(NULL), _frzConn(NULL), _pWatchDog(NULL), 
				_modApp(NULL), _objFactory(NULL), _stream2Purchase(NULL), _evctPurchase(NULL), _evctPurItem(NULL)
{
}

ModEnv::~ModEnv()
{
	try
	{
		doUninit();
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "doUninit() caught unexpect exception"));
	}

	_iceComm = NULL;
	_iceAdap = NULL;
	_pIceLog = NULL;
	_logForIce = NULL;
	_frzConn = NULL;
	_modApp = NULL;
}

bool ModEnv::initIceRuntime()
{
	// create ice communictor object
	try
	{
		int i=0;
		Ice::PropertiesPtr iceProps = Ice::createProperties(i, NULL);
		if (iceProps)
		{
		std::map<std::string, std::string>::iterator itor = gNewCfg.icePropMap.begin();
		for (; itor != gNewCfg.icePropMap.end(); itor ++)
		{
			iceProps->setProperty(itor->first, itor->second);
			glog(ZQ::common::Log::L_INFO, CLOGFMT(ModEnv, "ice property [%s] = [%s]"), 
				itor->first.c_str(), itor->second.c_str());
		}
		}
		Ice::InitializationData iceInitData;
		iceInitData.properties = iceProps;
		if (gNewCfg.enableIceTrace == 1) // record ice message in filelog
		{
		try
		{
			std::string icefilename =  gNewCfg.iceLogPath+ FNSEPS"GBMODSvc_Ice.log";
			_pIceLog = new ZQ::common::FileLog();
			_pIceLog->open(icefilename.c_str(), gNewCfg.iceLogLevel, gNewCfg.iceLogCount, gNewCfg.iceLogSize);
			_logForIce = new ModIceLog(*_pIceLog);
			iceInitData.logger = _logForIce;		}
		catch (...)
		{
			glog(ZQ::common::Log::L_WARNING, CLOGFMT(ModEnv, "create ice log [%s] failed"), gNewCfg.iceLogPath.c_str());
		}
		}
		_iceComm = Ice::initialize(iceInitData);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "create ice communicator caught %s"), ex.ice_name().c_str());
		return false;
	}

	// create ice object adapter
	try
	{
		_iceAdap = ZQADAPTER_CREATE(_iceComm, "MODAppService", gNewCfg.adapEndPoint.c_str(), glog);
		_iceAdap->activate();
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "create local adapter caught %s, endpoint is %s"), 
			ex.ice_name().c_str(), gNewCfg.adapEndPoint.c_str());
		return false;
	}

	// create object factory
	try
	{
		_objFactory = new ModFactory(*this);
		_iceComm->addObjectFactory(_objFactory, ZQTianShan::Application::MOD::ModPurchase::ice_staticId());
	}
	catch (const Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "create object factory caught %s: %s"), 
			ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "create object factory caught %s"), ex.ice_name().c_str());
		return false;
	}

	return true;
}

void ModEnv::uninitIceRuntime()
{
	try
	{
		if(_iceAdap)
			_iceAdap->deactivate();
	}
	catch (...)
	{
	}
	_iceAdap = NULL;
	try
	{
		if(_iceComm)
			_iceComm->destroy();
	}
	catch (...)
	{
	}	

	_evctPurchase = NULL;
	_iceComm = NULL;
}
#define FREEZEPROPENV(x,y)	std::string("Freeze.DbEnv.")+x+y
bool ModEnv::openSafeStore()
{
	// create directories for save db data
	//	std::string dbPath = gNewCfg.safeStorePath;
	std::string dbPath = gNewCfg.runtimePath;

	if (dbPath[dbPath.size() - 1] != FNSEPC)
		dbPath += FNSEPS; // if last char not equal to '\', add it.
#ifdef ZQ_OS_MSWIN
	dbPath += servname;
#else
	dbPath += Application->getServiceName();
#endif
	dbPath += FNSEPS; // push back MOD to the db path.
	if (FS::createDirectory(dbPath.c_str(), true))
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(ModEnv, "fix safe store path to [%s]"), dbPath.c_str());
	}
/*
	if (!isDir(dbPath.c_str()))
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "create safe store path [%s] failed"), dbPath.c_str());
		return false;
	}
*/

	if(!GenerateDBConfig(dbPath))
	{
		glog(ZQ::common::Log::L_WARNING, CLOGFMT(ModEnv, "failed to generate DB_CONFIG for ModSvc"));
	}

	try
	{
		Ice::PropertiesPtr proper = _iceAdap->getCommunicator()->getProperties();

		proper->setProperty(FREEZEPROPENV(dbPath,".CheckpointPeriod") ,gNewCfg.checkpointPeriod );
		proper->setProperty(FREEZEPROPENV(dbPath,".DbRecoverFatal" ) , gNewCfg.dbRecoverFatal );

		std::string evictorAttrPrefix = std::string("Freeze.Evictor.");
		proper->setProperty(evictorAttrPrefix + (std::string)Servant_ModPurchase+ ".$default.BtreeMinKey",      "16");
		proper->setProperty(evictorAttrPrefix + (std::string)Servant_ModPurchase ".PageSize",      "8192");

		// create freeze connection
		_frzConn = ::Freeze::createConnection(_iceComm, dbPath);
		glog(ZQ::common::Log::L_NOTICE, CLOGFMT(ModEnv, "freeze connection created on [%s]"), dbPath.c_str());

		// create purchase evictor
		_stream2Purchase = new ZQTianShan::Application::MOD::Stream2Purchase(INDEXFILENAME(Stream2Purchase));
		::std::vector<Freeze::IndexPtr> purIdxs;
		purIdxs.clear();
		purIdxs.push_back(_stream2Purchase);
		
#if ICE_INT_VERSION / 100 >= 303
		_evctPurchase = ::Freeze::createBackgroundSaveEvictor(_iceAdap, dbPath.c_str(), Servant_ModPurchase, 0, purIdxs);
#else
		_evctPurchase = ::Freeze::createEvictor(_iceAdap, dbPath.c_str(), Servant_ModPurchase, 0, purIdxs);
#endif
		_evctPurchase->setSize(gNewCfg.evctSize);
		_iceAdap->addServantLocator(_evctPurchase, Servant_ModPurchase);
	}
	catch (Freeze::DatabaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "open save store caught %s: %s"), 
			ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "open save store caught %s"), ex.ice_name().c_str());
		return false;
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ModEnv, "iterator the purchases in safestore"));
	int nSessionNum = 0;
	_pWatchDog->clear();
	::Freeze::EvictorIteratorPtr tItor = _evctPurchase->getIterator("", gNewCfg.initRecordBufferSize);
	while (tItor->hasNext())
	{
		int64 rn_value = gNewCfg.purchaseTimeout + (20 * nSessionNum++);
		Ice::Identity ident = tItor->next();
		_pWatchDog->watch(ident.name, rn_value);
	}
	glog(ZQ::common::Log::L_INFO, CLOGFMT(ModEnv, "there are %d purchases in safestore"), nSessionNum);

	return true;
}

void ModEnv::closeSafeStore()
{
}

bool ModEnv::loadMho()
{
	std::string path = FS::getImagePath();
	std::string::size_type pos = path.rfind(FNSEPC);
	if (pos != std::string::npos)
	{
		path = path.substr(0, pos);
		pos = path.rfind(FNSEPC);
		if(pos != std::string::npos)
		{
			path = path.substr(0, pos);
		}
		_programRootPath = path;
	}
	else _programRootPath = ".";

	_programRootPath += FNSEPS;
		
	_MHOHelperMgr.setExtraData(_configFilePath.c_str(),_logFolder.c_str(),m_InstanceID, _iceComm);
#ifdef _DEBUG
	_MHOHelperMgr.populate(_programRootPath.c_str());
	_MHOHelperMgr.populate((_programRootPath+ "bin" FNSEPS).c_str());
	_MHOHelperMgr.populate((_programRootPath+ "exe" FNSEPS).c_str());
#endif // _DEBUG
	
	if(_MHOHelperMgr.populate(gNewCfg.plugins.files) == 0)
	{
		glog(ZQ::common::Log::L_ERROR, "no plugIn loaded, exit");
		return false;
	}
//	_MHOHelperMgr.populate((_programRootPath+ "modules" FNSEPS).c_str());
	return true;
}

#ifdef ZQ_OS_MSWIN
void WINAPI CrashExceptionCallBack(DWORD ExceptionCode, PVOID ExceptionAddress)
{
	DWORD dwThreadID = GetCurrentThreadId();
	
	glog(ZQ::common::Log::L_ERROR,  L"Crash exception callback called,ExceptonCode 0x%08x, ExceptionAddress 0x%08x, Current Thread ID: 0x%04x",
		ExceptionCode, ExceptionAddress, dwThreadID);
	
	glog.flush();
}
#endif

void ModEnv::setCrushDump()
{
	std::string dumpPath = gNewCfg.crushDumpPath;
#ifdef ZQ_OS_MSWIN
	if (dumpPath[dumpPath.size() - 1] != '\\' && dumpPath[dumpPath.size() - 1] != '/')
		dumpPath += '\\';
//	dumpPath += "ModSvc";
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(ModEnv, "fix crush dump directory to [%s]"), dumpPath.c_str());

//	makeDir(dumpPath.c_str());

	if (!isDir(dumpPath.c_str()))
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "creat crash dump directory [%s] failed"), dumpPath.c_str());

	_crashDump.setDumpPath((char*)dumpPath.c_str());
	_crashDump.enableFullMemoryDump(gNewCfg.enableCrushDump);
	_crashDump.setExceptionCB(CrashExceptionCallBack);
#else
	DUMP_PATH = dumpPath.c_str();
#endif
}

bool ModEnv::doInit(const char * logfolder, std::string InstanceID)
{
	_logFolder = logfolder;
	m_InstanceID = InstanceID;

	setCrushDump();

	// init ice run-time
	if (!initIceRuntime())
		return false;

	if(!loadMho())
		return false;

	// should code here, because openSafeStore will use the watchdog to watch all purhcase
	_pWatchDog = new WatchDog(*this);
	_pWatchDog->start();

	// open db
	if (!openSafeStore())
		return false;

	try
	{
		_modApp = new ModApplication(*this);
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "malloc Mod application object failed"));
		return false;
	}

	// add mod application servant used to create purchase, called by site admin
	try
	{
		_iceAdap->ZQADAPTER_ADD(_iceComm, _modApp, ModAppServant);
	}
	catch (const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "add Mod application servant caught %s"), ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModEnv, "add Mod application servant caught unexpect exception"));
		return false;
	}

	return true;
}

void ModEnv::doUninit()
{
	if (NULL != _pIceLog)
		delete _pIceLog;
	_pIceLog = NULL;

	// close db
	closeSafeStore();

	if (NULL != _pWatchDog)
		delete _pWatchDog;
	_pWatchDog = NULL;

	// uninit ice run-time
	uninitIceRuntime();
}


//////////////////////////////////////////////////////////////////////////
// class ModApplication used to create purchase
//////////////////////////////////////////////////////////////////////////

#define CreateFormat(_C, _X) CLOGFMT(_C, "[%s] " _X), weiwooId.c_str()


ModApplication::ModApplication(ModEnv& env) : _env(env)
{
}

ModApplication::~ModApplication()
{
}

::TianShanIce::Application::PurchasePrx ModApplication::createPurchase(const ::TianShanIce::SRM::SessionPrx& weiwooSessPrx, 
															const ::TianShanIce::Properties& siteProperties, 
															const ::Ice::Current& c)
{
	std::string weiwooId = weiwooSessPrx->getId();
	
	ZQTianShan::Application::MOD::ModPurchasePtr pPurchase = new ZQMODApplication::ModPurchaseImpl(_env);
	pPurchase->weiwoo = weiwooSessPrx;
	pPurchase->ident.name = IceUtil::generateUUID();
	pPurchase->ident.category = Servant_ModPurchase;
	pPurchase->serverSessionId = weiwooId;
	pPurchase->enableAuthorize = false; // by default don't need authorization
	pPurchase->bInService = false;
	pPurchase->assetElements.clear();
	pPurchase->appPath = "";
	pPurchase->authEndpoint = "";
	pPurchase->purPrivData.clear();
	pPurchase->volumesLists.clear();
	//pPurchase->assetProps.clear();

	glog(ZQ::common::Log::L_INFO, CreateFormat(ModApplication, "create purchase [%s]"), pPurchase->ident.name.c_str());
	Ice::ObjectPrx objPrx = NULL;
	try
	{
//		ZQ::common::MutexGuard lk(_env._lockPurchase);
		objPrx = _env._evctPurchase->add(pPurchase, pPurchase->ident);
	}
	catch (const Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "ModApplication", err_317, "[%s] save purchase caught %s: %s", 
			weiwooId.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError>(glog, "ModApplication", err_318, "[%s] save purchase caught %s", 
			weiwooId.c_str(), ex.ice_name().c_str());
	}

	_env._pWatchDog->watch(pPurchase->ident.name, MinRenewValue);

	glog(ZQ::common::Log::L_DEBUG, CreateFormat(ModApplication, "purchase [%s] saved"), pPurchase->ident.name.c_str());

	return ::TianShanIce::Application::PurchasePrx::uncheckedCast(objPrx);
}

::std::string ModApplication::getAdminUri(const ::Ice::Current& c)
{
	return "";
}

::TianShanIce::State ModApplication::getState(const ::Ice::Current& c)
{
	::TianShanIce::State st;
	st=TianShanIce::stInService;
	return st;
}

ModService::ModService()
{
}

ModService::~ModService()
{
}

HRESULT ModService::OnInit()
{
	// assign glog with an existing log.
	// ZQ::common::setGlogger(m_pReporter);

	try
	{	
// 		gNewCfg.setLogger(m_pReporter);
// 		gNewCfg.testItems.enable = 0;
// 		gNewCfg.testAuthor.enable = 0;
// 
// 		if(gNewCfg.loadInFolder(m_wsConfigFolder))
// 		{
// 			gNewCfg.snmpRegister("MovieOnDemand.xml");
// 		}
		std::string strInstanceID = "1";
        if(m_argc == 1 && strlen(m_argv[0]))
		{
          glog(ZQ::common::Log::L_INFO, CLOGFMT(ModService, "netID = %s"),m_argv[0]);
		  strInstanceID = m_argv[0];
		}
		else
		{
			glog(ZQ::common::Log::L_INFO, CLOGFMT(ModService, "netID = 1"));
		}
		std::map< std::string, Config::Holder<ModCfg> >::iterator itor =
										gNewCfgGroup.modcfg.find(strInstanceID);
		if(itor == gNewCfgGroup.modcfg.end())
		{
			glog(ZQ::common::Log::L_ERROR, 
				CLOGFMT(ModService, "Can't find configration info with netID = %s"), strInstanceID.c_str());
			return S_FALSE;
		}


		gNewCfg = itor->second;
        
		if(gNewCfg.urlpattern.size() < 1)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModService, 
				"MOD services configration NO URLPattern, or all URLPattern disenable"));
			return S_FALSE;
		}

		gNewCfg.snmpRegister("");

		if(gNewCfg.purchaseTimeout < 120000)
			gNewCfg.purchaseTimeout = 120000;

		if(gNewCfg.noticeTime < 60000)
			gNewCfg.noticeTime = 60000;

#ifdef  _DEBUG	
//		showMODCfgGroup(gNewCfgGroup);

		printf("This Service use netID = %s\n", strInstanceID.c_str());

		showModCfg(gNewCfg);
//		showModCfg(itor->second);
#endif

#ifdef ZQ_OS_MSWIN
		std::string logDir = m_wsLogFolder;
		_env._configFilePath = std::string(m_wsConfigFolder) + "GBMovieOnDemand.xml";
#else
		std::string logDir = _logDir;
		_env._configFilePath = _configDir + "GBMovieOnDemand.xml";
#endif
		if (!_env.doInit(logDir.c_str(),strInstanceID))
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModService, "ModEnv doInit() failed"));
			return S_FALSE;
		}
		
	}
	catch(ZQ::common::CfgException& ex)
	{
		const char* pstr = ex.getString();
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModService, 
			"ModEnv doInit() parser MovieOnDemand.xml error , please check XML format, errormsg = %s"),
			pstr);
		return S_FALSE;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModService, "ModEnv doInit() caught unexpect exception"));
		return S_FALSE;
	}

	return BaseZQServiceApplication::OnInit();	 
}

HRESULT ModService::OnStart()
{
	return BaseZQServiceApplication::OnStart();
}

HRESULT ModService::OnStop()
{
	return BaseZQServiceApplication::OnStop();
}

HRESULT ModService::OnUnInit()
{
	try 
	{
		_env.doUninit();
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ModService, "ModEnv doUninit() caught unexpect exception"));
	}

	return BaseZQServiceApplication::OnUnInit();
}
void ModService::OnSnmpSet(const char *varName)
{
	if(stricmp(varName, "IceLog/level") == 0)
	{
		if(_env._pIceLog)
			_env._pIceLog->setVerbosity(gNewCfg.iceLogLevel);
	}
	if(stricmp(varName, "IceLog/size"))
	{
		if(_env._pIceLog)
			_env._pIceLog->setFileSize(gNewCfg.iceLogSize);
	}
	
}
} // namespace ZQMODApplication

