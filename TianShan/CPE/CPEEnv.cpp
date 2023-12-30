// ===========================================================================
// Copyright (c) 2006 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/CPEEnv.cpp $
// 
// 21    4/01/15 10:01a Build
// cleaned old snmp
// 
// 16    9/07/12 3:31p Li.huang
// 
// 15    9/07/12 2:43p Hui.shao
// merged from V1.15
// 
// 14    9/07/12 2:11p Hui.shao
// merged from main tree
// 
// 13    9/04/12 5:26p Hongquan.zhang
// merge from maintree
// 
// 12    9/04/12 11:36a Hongquan.zhang
// merge from maintree
// 
// 14    9/04/12 11:31a Zonghuan.xiao
// change  snmp  table variable  type

// 11    9/03/12 3:22p Hongquan.zhang
// merge from maintree
// 
// 10    8/10/12 1:45p Hongquan.zhang
// merge from maintree for snmp table view
// 
// 9     8/17/11 2:19p Li.huang
// add DirectIO for linux
// 
// 8     4/02/11 2:19p Fei.huang
// 
// 7     3/25/11 3:03p Li.huang
// modify log
// 
// 6     3/21/11 2:17p Li.huang
// fix bug 13454 1#
// 
// 5     1/07/11 12:38p Li.huang
// 
// 4     1/07/11 11:10a Li.huang
// 
// 3     10-12-15 14:13 Li.huang
// use new bufferpool
// 
// 2     10-12-03 15:35 Li.huang
// 13454
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 34    09-12-22 17:37 Fei.huang
// - remove quit()
// 
// 33    09-09-11 17:55 Xia.chen
// change default value for regedit
// 
// 32    09-06-25 17:18 Yixin.tian
// 
// 31    09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 35    09-05-08 14:16 Xia.chen
// 
// 34    09-05-06 17:17 Xia.chen
// add envname log display
// 
// 33    09-05-05 17:25 Xia.chen
// add sessions to provisionstore map
// 
// 32    09-05-05 13:20 Jie.zhang
// 31    09-04-15 16:39 Jie.zhang
// add provision session evictor size setting
// 
// 30    09-04-14 19:23 Jie.zhang
// remove the booking logs
// 
// 29    09-03-09 14:53 Yixin.tian
// modify LONGLONG to int64
// 
// 28    09-03-05 16:21 Jie.zhang
// 
// 27    09-03-04 23:19 Jie.zhang
// 
// 26    09-03-04 21:33 Jie.zhang
// reflactor the provision cost source
// 
// 25    09-01-22 15:53 Yixin.tian
// 
// 24    09-01-20 17:59 Jie.zhang
// 
// 23    08-12-19 15:58 Yixin.tian
// modify for Linux OS
// 
// 22    08-12-16 11:13 Jie.zhang
//
// 21    08-11-19 20:17 Jie.zhang
// 
// 20    08-11-18 10:59 Jie.zhang
// merge from TianShan1.8
// 
// 21    08-08-21 18:19 Xia.chen
// 
// 20    08-08-15 16:49 Xia.chen
// 
// 19    08-08-13 11:27 Xia.chen
// 
// 18    08-08-12 17:06 Xia.chen
// 
// 17    08-06-26 12:15 Jie.zhang
// add init() to cpeenv
// 
// 16    08-06-06 19:30 Jie.zhang
// 
// 15    08-05-17 19:09 Jie.zhang
// 
// 14    08-05-14 22:11 Jie.zhang
// 
// 13    08-05-13 11:31 Jie.zhang
// 
// 12    08-04-25 18:05 Jie.zhang
// 
// 11    08-04-25 16:07 Jie.zhang
// 
// 14    08-04-22 18:28 Jie.zhang
// 
// 13    08-03-25 14:03 Jie.zhang
// 
// 12    08-03-24 19:41 Jie.zhang
// 
// 11    08-03-17 19:56 Jie.zhang
// 
// 10    08-03-07 21:29 Jie.zhang
// 
// 9     08-03-04 17:13 Jie.zhang
// 
// 8     08-02-28 16:17 Jie.zhang
// 
// 7     08-02-21 18:27 Jie.zhang
// logs change and bug fixs
// 
// 6     08-02-21 13:04 Jie.zhang
// 
// 5     08-02-20 16:16 Jie.zhang
// 
// 4     08-02-18 18:46 Jie.zhang
// changes check in
// 
// 3     08-02-15 12:23 Jie.zhang
// changes check in
// 
// 2     08-02-14 16:26 Hui.shao
// added ami callbacks
// 
// 1     08-02-13 17:48 Hui.shao
// initial checkin
// ===========================================================================

#include "CPEEnv.h"
#include "ProvisionCmds.h"
#include "Log.h"
#include "CPECfg.h"
#include "CPHInc.h"
#include "ProvisionResourceBook.h"
#include "ProvisionState.h"


#ifdef ZQ_OS_MSWIN
#include "zqappshell.h"
#include <Winreg.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

namespace ZQTianShan {
namespace CPE {

// -----------------------------
// class CPEEnv
// -----------------------------
CPEEnv::CPEEnv(ZQ::common::Log& log, 
						   ZQ::common::NativeThreadPool& threadPool, 
						   Ice::CommunicatorPtr& communicator,
						    ZQ::common::NativeThreadPool& timethreadPool
						)
:_communicator(communicator), _adapter(NULL), _thpool(threadPool), 
 _watchDog(*this), _log(log),
 // configuration starts here
 _ftpServer(*this), _scheduleErrorWindow(DEFAULT_SCH_ERROR_WIN), _errorProc(this),_timerthpool(timethreadPool)
{
	_errorProc.setLog(&_log);

	_mediasamplebuffer = 65536;
	_maxBufferPoolSize = 12000;
	_minBufferPoolSize = 50;
	_alignTo = 4096;
	_pEventLog = NULL;

//	_cpsvcSnmpTableAgent = new ZQ::Snmp::Subagent( 700, 3 );
}

void CPEEnv::setMediaSampleBuffer(int mediasamplebuffer, int maxBufferPoolSize, int minBufferPoolSize)
{
	_mediasamplebuffer = mediasamplebuffer;
	_maxBufferPoolSize = maxBufferPoolSize;
	_minBufferPoolSize = minBufferPoolSize;
}

//using namespace ZQ::Snmp;
//template<typename Type>
//class ModulesVar: public ZQ::Snmp::IVariable
//{
//public:
//	ModulesVar(Type var):val_(var){}
//
//	~ModulesVar(){}
//
//	virtual bool get(SmiValue& val, AsnType desiredType)
//	{
//		return smivalFrom(val, val_, desiredType);
//	}
//
//	virtual bool set(const SmiValue& val)
//	{
//		return smivalTo(val, val_);
//	}
//
//	virtual bool validate(const SmiValue& val) const
//	{
//		return true;
//	}
//
//private:
//	Type val_;
//};
//
//class MethodListTableMediator: public IManaged
//{
//public:
//	MethodListTableMediator(const ZQ::Snmp::Oid & subid, CPEEnv & cpeEnv)
//		:_subid(subid), _triggerSubid("1.1"), _cpeEnv(cpeEnv)
//	{
//        _methodListTable = initMethodListTable();
//	};
//
//	virtual ~MethodListTableMediator(){};
//
//	virtual Status get(const Oid& subid, SmiValue& val)
//	{
//		if (0 == _triggerSubid.compare(0, subid.length(), subid))
//			_methodListTable = initMethodListTable();//refresh table
//
//		return _methodListTable->get(subid, val); 
//	};
//
//	virtual Status set(const Oid& subid, const SmiValue& val)
//	{
//		return _methodListTable->set(subid, val);
//	};
//
//	virtual Status next(const Oid& subid, Oid& nextId) const
//	{
//		if (0 == _triggerSubid.compare(0, subid.length(), subid))
//		{
//			MethodListTableMediator* tempThis = const_cast<MethodListTableMediator*>(this);
//			tempThis->_methodListTable = tempThis->initMethodListTable();//refresh table
//		}
//
//		return _methodListTable->next(subid, nextId); 
//	};
//
//	virtual Status first(Oid& firstId) const
//	{
//		return _methodListTable->first(firstId);
//	};
//
//	bool addColumn(uint32 colId, AsnType type, Access access)
//	{
//		return _methodListTable->addColumn(colId, type, access);
//	};
//
//	bool addRowData(uint32 colId, Oid rowIndex, VariablePtr var)
//	{
//		return _methodListTable->addRowData(colId, rowIndex, var);
//	};
//
//	Oid buildIndex(const std::string& idx)
//	{
//		return _methodListTable->buildIndex(idx);
//	};
//
//	Oid buildIndex(uint32 idx)
//	{
//		return _methodListTable->buildIndex(idx);
//	};
//
//private:
//	TablePtr 	initMethodListTable();
//
//private:
//	Oid       _subid;	
//	Oid       _triggerSubid;
//	CPEEnv &  _cpeEnv;
//    TablePtr  _methodListTable;
//};
//
//
//TablePtr  MethodListTableMediator::initMethodListTable()
//{
//    TablePtr tbMethodListUsage(new ZQ::Snmp::Table());
//	enum ListTableColunm
//	{
//        CPE_METHOD = 1,
//		CPE_SESS_SUBTOTAL,
//		CPE_BW_SUBTOTAL,
//		CPE_SESS_MAX,
//		CPE_BW_MAX,
//		TABLE_COLUNM_COUNT
//	};
//
//	tbMethodListUsage->addColumn( CPE_METHOD,	      ZQ::Snmp::AsnType_Octets,		    ZQ::Snmp::aReadOnly);//cpeMethod
//	tbMethodListUsage->addColumn( CPE_SESS_SUBTOTAL,  ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly);//cpeSessSubtotal
//	tbMethodListUsage->addColumn( CPE_SESS_MAX,	      ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly);//cpeSessMax
//	tbMethodListUsage->addColumn( CPE_BW_SUBTOTAL,	  ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly);//cpeBwSubtotal
//	tbMethodListUsage->addColumn( CPE_BW_MAX,	      ZQ::Snmp::AsnType_Integer,		ZQ::Snmp::aReadOnly);//cpeBwMax
//	
//	::TianShanIce::StrValues methods = _cpeEnv._provisionFactory->listSupportedMethods();
//    (_cpeEnv._log)(ZQ::common::Log::L_DEBUG, CLOGFMT(MethodListTableMediator, "snmp method list table created, column[%d], methods size[%d]"), TABLE_COLUNM_COUNT -  1, methods.size());
//
//	int rowIndex = 1;
//	for (size_t sizeStep =0; sizeStep < methods.size(); sizeStep++)
//	{
//		try
//		{
//			::TianShanIce::ContentProvision::MethodInfo info;
//			info.methodType = methods[sizeStep];
//
//			uint32  allocatedKbps, maxKbps;
//			uint sessions, maxsessions;
//
//			ICPHelper* pHelper = _cpeEnv._provisionFactory->findHelper(info.methodType.c_str());
//			if (NULL != pHelper && pHelper->getLoad(info.methodType.c_str(), allocatedKbps, maxKbps, sessions, maxsessions))
//			{
//				ZQ::Snmp::Oid indexOid = ZQ::Snmp::Table::buildIndex( rowIndex );
//				tbMethodListUsage->addRowData( CPE_METHOD,        indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<std::string>(info.methodType) )); //cpeMethod 
//				tbMethodListUsage->addRowData( CPE_SESS_SUBTOTAL, indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<int>(sessions) ));         //cpeSessSubtotal 
//				tbMethodListUsage->addRowData( CPE_BW_SUBTOTAL,   indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<int>(allocatedKbps) ));    //cpeBwSubtotal;
//				tbMethodListUsage->addRowData( CPE_SESS_MAX,      indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<int>(maxsessions) ));      //cpeSessMax 
//				tbMethodListUsage->addRowData( CPE_BW_MAX,        indexOid , ZQ::Snmp::VariablePtr( new ModulesVar<int>(maxKbps) ));          //cpeBwMax 
//				++rowIndex;
//			}																														 
//		}
//		catch (...) 
//		{
//			(_cpeEnv._log)(ZQ::common::Log::L_DEBUG, CLOGFMT(MethodListTableMediator, "snmp method list table add data error, row[%d], methods size[%d]"), rowIndex, methods.size());
//		}
//	}
//
//	(_cpeEnv._log)(ZQ::common::Log::L_DEBUG, CLOGFMT(MethodListTableMediator, "snmp method list table end, colunm[%d], row[%d], methods size[%d]"), TABLE_COLUNM_COUNT -  1, rowIndex, methods.size());
//	return tbMethodListUsage;
//};

bool CPEEnv::init(const char* endpoint, const char* databasePath, const char* runtimeDBFolder)
{
	_endpoint = (endpoint && strlen(endpoint)>0) ? endpoint : DEFAULT_ENDPOINT_CPE;

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CPEEnv, "open adapter %s at %s"),
			ADAPTER_NAME_CPE, _endpoint.c_str());

	try
	{
		 _adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_CPE, _endpoint.c_str(), _log);
	}
	catch(Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR,CLOGFMT(CPEEnv,"Create adapter failed with endpoint=%s and exception is %s"),
							_endpoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	
	char path[MAX_PATH];
#ifdef ZQ_OS_MSWIN
	if (::GetModuleFileName(NULL, path, MAX_PATH-1)>0)
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
				*p='\0';
		}
		_programRootPath = path;
	}
	else 
		_programRootPath = ".";

#else
	if(readlink("/proc/self/exe",path, MAX_PATH-1) > 1)
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if(NULL != p && (0 ==strcasecmp(FNSEPS "bin", p) || 0 ==strcasecmp(FNSEPS "bin64", p) || 0 == strcasecmp(FNSEPS "exe", p)))
				*p='\0';
		}
		_programRootPath = path;
	}
	else 
		_programRootPath = ".";
	
	if(_programRootPath.find("TianShan") == std::string::npos )
		_programRootPath += "/TianShan";

#endif

	_programRootPath += FNSEPS;

	try
	{
		openDB(databasePath,runtimeDBFolder);
	}
	catch(...)
	{
		return false;
	}

	if (!_ftpServer.Initialize())
	{
		_log(ZQ::common::Log::L_ERROR,CLOGFMT(CPEEnv, "failed to init ftp server"));
	}
	_pEventLog  = new ZQ::common::SysLog("CPESvc");
	if(!_pEventLog)
	{
		_log(ZQ::common::Log::L_ERROR,CLOGFMT(CPEEnv, "failed to init system event log"));
	}
	// init the object factory for CPE database objects
	_factory = new CPEFactory(*this);
	_provisionFactory = new ProvisionFactory(*this);

	if (_provisionFactory->populate() < 1)
	{
		_log(ZQ::common::Log::L_ERROR,CLOGFMT(CPEEnv,"No plugin loaded, force to stop"));
		return false;
	}
	int maxSessions = _provisionFactory->getMaxSessioOfGroup("");
	if(maxSessions > _gCPECfg._dwThreadPool)
	{
/*		if(maxSessions >  _gCPECfg._dwMaxThreadPool)
		{
			_log(ZQ::common::Log::L_ERROR,CLOGFMT(CPEEnv, "max session count [%d] over than max threadpool size[%d]"), maxSessions, _gCPECfg._dwMaxThreadPool);
			if(_pEventLog)
			{
				(*_pEventLog)(ZQ::common::Log::L_ERROR,CLOGFMT(CPEEnv, "max session count [%d] over than max threadpool size[%d]"), maxSessions, _gCPECfg._dwMaxThreadPool);
			}
			maxSessions = _gCPECfg._dwMaxThreadPool;
		}
*/
		_log(ZQ::common::Log::L_INFO,CLOGFMT(CPEEnv,"resize threadpool size from [%d] to [%d]"), _gCPECfg._dwThreadPool, maxSessions + 10);
		_thpool.resize(maxSessions + 10);
	}
#ifdef ZQ_OS_MSWIN
 	checkRegEdit();
#else
	checkXmlCfg();
#endif

	//if(NULL != _cpsvcSnmpTableAgent)
	//{
	//	_cpsvcSnmpTableAgent->setLogger(_pEventLog);
	//	ZQ::Snmp::Oid tableOid("1.1.1");
	//	_cpsvcSnmpTableAgent->addObject(tableOid, ManagedPtr( new MethodListTableMediator(tableOid, *this) ) );
	//	_cpsvcSnmpTableAgent->start();
	//}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CPEEnv, "CPEEnv initialized successful"));
	return true;
}

CPEEnv::~CPEEnv()
{
	_log(ZQ::common::Log::L_DEBUG,CLOGFMT(CPEEnv, "~CPEEnv()"));
	_cpcClient.unInitModule();	
	_watchDog.quit();		
	_log(ZQ::common::Log::L_DEBUG,CLOGFMT(CPEEnv, "~CPEEnv() stop adapter"));
	_adapter=NULL;
	_log(ZQ::common::Log::L_INFO,CLOGFMT(CPEEnv, "~CPEEnv() adapter stopped"));
	closeDB();	

	_ftpServer.Stop();	

	_log(ZQ::common::Log::L_INFO,CLOGFMT(CPEEnv, "~CPEEnv() release provision factory"));
	_provisionFactory = NULL;
	_log(ZQ::common::Log::L_INFO,CLOGFMT(CPEEnv, "~CPEEnv() provision factory released"));

	_ftpServer.Uninitialize();

	try
	{
		if(_pEventLog)
			delete _pEventLog;
		_pEventLog = NULL;

		//if (NULL != _cpsvcSnmpTableAgent)
		//	delete _cpsvcSnmpTableAgent;

		//_cpsvcSnmpTableAgent = NULL;
	}
	catch (...)
	{

	}
}

bool CPEEnv::start()
{
	if (!_ftpServer.Start())
	{
		//???
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CPEEnv, "failed to start ftp server"));
	}

	try
	{
		_adapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CPEEnv, "activate adapter caught exception: %s"), ex.ice_name().c_str());
	}
	
	_cpcClient.initModule(_communicator);
	return true;
}

#define ProvSessDataSubDir "CPSess"

bool CPEEnv::openDB(const char* databasePath,const char* dbRuntimePath)
{
	closeDB();

	if (NULL == databasePath || strlen(databasePath) <1)
		_dbPath = _programRootPath + "data" FNSEPS;
	else 
		_dbPath = databasePath;

	if (FNSEPC != _dbPath[_dbPath.length()-1])
		_dbPath += FNSEPS;

	if ( NULL == dbRuntimePath || strlen(dbRuntimePath)<1 ) 
	{
		_dbRuntimePath = _dbPath;
	}
	else
	{
		_dbRuntimePath = dbRuntimePath;
	}
	if (FNSEPC != _dbRuntimePath[_dbRuntimePath.length()-1])
		_dbRuntimePath += FNSEPS;
	try 
	{	
		
		// open the Indexes
#define INSTANCE_INDEX(_IDX) _log(ZQ::common::Log::L_DEBUG, CLOGFMT(CPEEnv, "create index: " #_IDX)); \
		                     _idx##_IDX = new TianShanIce::ContentProvision::_IDX(INDEXFILENAME(_IDX))
		_log(ZQ::common::Log::L_INFO, CLOGFMT(CPEEnv, "opening runtime database at path: %s"), _dbRuntimePath.c_str());
#ifdef ZQ_OS_MSWIN
		CreateDirectory((_dbRuntimePath + ProvSessDataSubDir FNSEPS).c_str(), NULL);
#else
		mkdir((_dbRuntimePath + ProvSessDataSubDir FNSEPS).c_str(), 0755);
#endif
		INSTANCE_INDEX(ContentToProvision);
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxContentToProvision);
			
			std::string dbenv = _dbRuntimePath + ProvSessDataSubDir FNSEPS;
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CPEEnv, "create evictor %s with index %s, evictor size %d, envname %s"), DBFILENAME_ProvisionSession, "ContentToProvision", _gCPECfg._nProSessEvictorSize,dbenv.c_str());
#if ICE_INT_VERSION / 100 >= 303
			_eProvisionSession = ::Freeze::createBackgroundSaveEvictor(_adapter, dbenv, DBFILENAME_ProvisionSession, 0, indices);
#else
			_eProvisionSession = Freeze::createEvictor(_adapter, dbenv, DBFILENAME_ProvisionSession, 0, indices);
#endif
			_adapter->addServantLocator(_eProvisionSession, DBFILENAME_ProvisionSession);
			_eProvisionSession->setSize(_gCPECfg._nProSessEvictorSize);
		}
	}
	catch(const ::Freeze::DatabaseException& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log,"CPEEnv",1001,CLOGFMT(CPEEnv, "openDB() caught DatabaseException: %s"), ex.message.c_str());
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log,"CPEEnv",1001,CLOGFMT(CPEEnv, "openDB() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log,"CPEEnv",1002, CLOGFMT(CPEEnv, "openDB() caught unkown exception"));
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CPEEnv, "database ready"));

	return true;
}

void CPEEnv::closeDB(void)
{
	if (!_idxContentToProvision && !_eProvisionSession)
		return;

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CPEEnv, "close local database"));

	_idxContentToProvision	= NULL;
	_eProvisionSession      = NULL;
	_log(ZQ::common::Log::L_INFO, CLOGFMT(CPEEnv, "local database closed"));
}

void CPEEnv::logProvisionSessionBindAmiCBException(const char* APIname, const ::Ice::Exception& ex0)
{
	if (NULL == APIname)
		APIname = "";

	try {
		ex0.ice_throw(); // re-throw then test the exception type
	}
	catch(const ::TianShanIce::ContentProvision::ProvisionOwnerNotFound& ex)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(ProvisionSessionBind, "%s() caught exception[ProvisionOwnerNotFound] when nodify owner: (%d) %s"), APIname, ex.errorCode, ex.message.c_str());
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(ProvisionSessionBind, "%s() caught exception[%s] when nodify owner: (%d) %s"), APIname, ex.ice_name().c_str(), ex.errorCode, ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(ProvisionSessionBind, "%s() caught exception[%s] when nodify owner"), APIname, ex.ice_name().c_str());
	}
}

#ifdef ZQ_OS_MSWIN
bool CPEEnv::checkRegEdit()
{
	LPCTSTR   data_Set="Software\\SeaChange\\PacedIndex\\VVX\\";
	LPCTSTR   data_Set1="Software\\SeaChange\\PacedIndex\\VV2\\";
	
	char  defaultModule[80] = "PacedVVX.dll";
	char  secondModule[80] = "PacedVV2.dll";

	if (!SetRegEdit(data_Set,defaultModule))
		return false;

	if (!SetRegEdit(data_Set1,secondModule))
		return false;

	return TRUE;
}

bool CPEEnv::SetRegEdit(LPCTSTR dataset,char* module)
{
	HKEY hKEY;
	DWORD typesz=REG_SZ;
	DWORD typed =REG_DWORD;

	DWORD onlyFlag;
	DWORD defaultOnlyFlag=1;//atol("1");
	DWORD interval;
	DWORD defaultInterva=1000;//atol("1000");
	DWORD cbData=80;
	DWORD position;

	std::string firstV = "PaceMainFileOnly";
	std::string secondV = "PacingIntervalMs";
	std::string thirdV = "PacingModule";

	long ret0=(::RegCreateKeyEx(HKEY_LOCAL_MACHINE,dataset,0,"",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hKEY,&position));
	if(ret0!=ERROR_SUCCESS)
	{
		if (position == REG_CREATED_NEW_KEY)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CPEEnv, "The key %s did not exist then failed to create it"),dataset);
		}
		else
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CPEEnv, "The key %s existed then failed to open it"),dataset);
		}
		return FALSE;
	}

	long ret1=::RegQueryValueEx(hKEY,(LPCSTR)firstV.c_str(),NULL,&typed,(LPBYTE)&onlyFlag,&cbData);
	if(ret1!=ERROR_SUCCESS)
	{
		ret1=::RegSetValueEx(hKEY,(LPCSTR)firstV.c_str(),NULL,REG_DWORD,(LPBYTE)&defaultOnlyFlag,sizeof(DWORD));
		if(ret1!=ERROR_SUCCESS)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CPEEnv, "Failed to set configuration [%s] = [%u],error is %d"),firstV.c_str(),defaultOnlyFlag,GetLastError());
			::RegCloseKey(hKEY);
			return FALSE;
		}
		_log(ZQ::common::Log::L_INFO, CLOGFMT(CPEEnv, "Set configuration [%s] = [%u],error is %d"),firstV.c_str(),defaultOnlyFlag,GetLastError());

	}

	long ret2=::RegQueryValueEx(hKEY,(LPCSTR)secondV.c_str(),NULL,&typed,(LPBYTE)&interval,&cbData);
	if(ret2!=ERROR_SUCCESS)
	{
		ret1=::RegSetValueEx(hKEY,(LPCSTR)secondV.c_str(),NULL,REG_DWORD,(LPBYTE)&defaultInterva,sizeof(DWORD));
		if(ret1!=ERROR_SUCCESS)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CPEEnv, "Failed to set configuration [%s] = [%u],error is %d"),secondV.c_str(),defaultInterva,GetLastError());
			::RegCloseKey(hKEY);
			return FALSE;
		}
		_log(ZQ::common::Log::L_INFO, CLOGFMT(CPEEnv, "Set configuration [%s] = [%u]"),secondV.c_str(),defaultInterva);

	}

	LPBYTE temp = new BYTE[80];
	long ret3 =::RegQueryValueEx(hKEY,(LPCSTR)thirdV.c_str(),NULL,&typesz,temp,&cbData);
	if(ret3!=ERROR_SUCCESS)
	{
		ret1=::RegSetValueEx(hKEY,(LPCSTR)thirdV.c_str(),NULL,REG_SZ,(LPBYTE)module,strlen(module)+1);
		if(ret1!=ERROR_SUCCESS)
		{	
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CPEEnv, "Failed to set configuration [%s] = [%u],error is %d"),thirdV.c_str(),module,GetLastError());

			::RegCloseKey(hKEY);
			delete []temp;
			return FALSE;
		}
		_log(ZQ::common::Log::L_INFO, CLOGFMT(CPEEnv, "Set configuration [%s] = [%u]"),thirdV.c_str(),module);
	}

	::RegCloseKey(hKEY);
	delete []temp;
	return true;
}

#else
bool CPEEnv::checkXmlCfg()
{
	return true;
}
bool CPEEnv::setXmlCfg(std::string& dataset, std::string& module)
{
	return true;
}

#endif

Ice::Long TimetToTianShanTime(time_t t)
{ 
//	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;		// to filetime
	int64 ll = t*10000000L + 116444736000000000LL;		// to filetime
	return (ll/10000);  //convert nsec to msec
}

// return the RDS session count
int CPEEnv::getProvisioningSessCount()
{
	int nCount=0;
	IdentCollection Idents;
	
	try	{
		::Freeze::EvictorIteratorPtr itptr = _eProvisionSession ->getIterator("", 100);
		while (itptr && itptr->hasNext())
		{
			Idents.push_back(itptr->next());
		}
	}
	catch (const ::Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CPEEnv, "caught exception[%s] when enumerate provisons"), ex.ice_name().c_str());
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CPEEnv, "caught unknown exception when enumerate provisons"));
	}

	for (IdentCollection::iterator it= Idents.begin(); it != Idents.end(); it++)
	{
		try {
			::TianShanIce::ContentProvision::ProvisionSessionPrx provision = IdentityToObjEnv(*this, ProvisionSession, *it);
			::TianShanIce::ContentProvision::ProvisionState state = provision->getState();
			if (state == ::TianShanIce::ContentProvision::cpsProvisioning)
			{
				nCount++;
			}				
		}
		catch (...) {}
	}

	return nCount;
}

bool CPEEnv::stopReceiveRequest()
{
	glog(Log::L_WARNING, CLOGFMT(CPEEnv, "request to stop receiving requests"));

	return true;
}

bool CPEEnv::restartApplication()
{
	//ask to restart service
#ifdef ZQ_OS_MSWIN
	gbAppShellSetKeepAlive = false;
#else
//	Application->quit();
#endif

	glog(Log::L_ERROR, CLOGFMT(CPEEnv, "stop keep-alive event to service shell to get restarting"));	
	return true;
}

bool CPEEnv::canRestartApplication()
{
	int nProvisionSession = getProvisioningSessCount();
	glog(Log::L_INFO, CLOGFMT(CPEEnv, "[check if we could restart app] Provisioning session count is %d, can_restart[%d]"), nProvisionSession, !nProvisionSession);	

	return !nProvisionSession;
}

void CPEEnv::processProvisionError( bool success, const std::string& strError, const std::string& strCode )
{
	if (_gCPECfg._dwRestartOnCertainError)
	{
		(new ErrorProcessCmd(_timerthpool, _errorProc, success, strError, strCode))->start();		
	}
}

MethodCost* CPEEnv::getMethodCost(const std::string& methodType)
{
	ZQTianShan::ContentProvision::ICPHelper* pHelper = _provisionFactory->findHelper(methodType.c_str());
	if (!pHelper)
	{
		return 0;
	}

	return pHelper->getMethodCost(methodType);
}

bool CPEEnv::provisionCost(const Ice::Identity& provisionIdent, const std::string& methodType, unsigned int timeStart, unsigned int timeEnd, unsigned int bandwidthKBps )
{
	ZQ::common::MutexGuard	op(_resourceBookLock);

	ProvisionResourceBook	resourceBook(this);

	resourceBook.setTimeFilter(timeStart, timeEnd);
	
	_pProvStore.addResourceBookForProvision(resourceBook);

	bool bRet = resourceBook.bookProvision(methodType, timeStart, timeEnd, bandwidthKBps);	
	if (bRet)
	{
		_pProvStore.addProvisionStore(provisionIdent,methodType,timeStart,timeEnd,bandwidthKBps);
		glog(Log::L_INFO, CLOGFMT(ProvisionCost, "[%s] book provision method[%s] with bandwidth[%d] successful"), 
			provisionIdent.name.c_str(), methodType.c_str(), bandwidthKBps);	
	}
	else
	{
		glog(Log::L_WARNING, CLOGFMT(ProvisionCost, "[%s] failed to book provision method[%s] with bandwidth[%d]"), 
			provisionIdent.name.c_str(), methodType.c_str(), bandwidthKBps);	
		//output some debug information?
	}
	return bRet;
}

}} // namespace
