// DummySS.cpp : Defines the entry point for the console application.
//
#include <ZQ_common_conf.h>
#include <Log.h>

#include "DummySvc.h"
#include "DummySSCfg.h"
#include "DummyStreamSmith.h"
#include <stdio.h>

#ifdef ZQ_OS_MSWIN
DWORD gdwServiceType = 1;
DWORD gdwServiceInstance = 0;
#endif
DummySvc _gdummysvc;
ZQ::common::BaseZQServiceApplication *Application = &_gdummysvc;

DummyService*	_serviceInstance;

ZQ::common::Config::Loader<DummySSCfg> _gDummySSCfg("DummySvc.xml");
ZQ::common::Config::ILoader *configLoader = &_gDummySSCfg;

int pauseMax;
int pauseMin;

IceLogger::IceLogger(ZQ::common::Log& log):_logger(log)
{		
}
IceLogger::~IceLogger()
{	
}
void IceLogger::print(const ::std::string& message)
{
	ZQ::common::MutexGuard gd(_locker);
	_logger(ZQ::common::Log::L_DEBUG,message.c_str());
}
void IceLogger::trace(const ::std::string& category, const ::std::string& 

			    message)
{
	ZQ::common::MutexGuard gd(_locker);
	_logger(ZQ::common::Log::L_DEBUG,"catagory %s,message%s",category.c_str(),message.c_str());
}
void IceLogger::warning(const ::std::string& message)
{
	ZQ::common::MutexGuard gd(_locker);
	_logger(ZQ::common::Log::L_WARNING,message.c_str());
	_logger.flush ();
}
void IceLogger::error(const ::std::string& message)
{
	ZQ::common::MutexGuard gd(_locker);
	_logger(ZQ::common::Log::L_ERROR,message.c_str());
	_logger.flush ();
}
DummySvc::DummySvc()
{
	_dummySvcLog = NULL;
	_m_iceLogger = NULL;
}
DummySvc::~DummySvc()
{
	_m_iceLogger = NULL;
	if (NULL != _dummySvcLog)
		delete _dummySvcLog;
	_dummySvcLog = NULL;
}
HRESULT DummySvc::OnInit()
{
 	BaseZQServiceApplication::OnInit();

	pauseMax=_gDummySSCfg.pauseMaxCfg;
	pauseMin=_gDummySSCfg.pauseMinCfg;
	_dummySvcLog = new ZQ::common::FileLog( _gDummySSCfg.IceLogPath.c_str(), _gDummySSCfg.logLevel);
	ZQ::common::setGlogger(_dummySvcLog);
	_m_iceLogger=new IceLogger(*_dummySvcLog);
	return S_OK;
}

HRESULT DummySvc::OnStart()
{
	try
	{
		int i=0;
		::Ice::InitializationData initData;
		initData.properties = Ice::createProperties(i, NULL);
		initData.logger = _m_iceLogger;
		#if  ICE_INT_VERSION / 100 >= 306
			_ic=::Ice::initialize(i,NULL,initData);
		#else
			_ic=::Ice::initializeWithLogger(i,NULL,_m_iceLogger);
		#endif
	}
	catch( const Ice::Exception& ex )
	{
		glog(ZQ::common::Log::L_INFO, CLOGFMT(OnStart(),"catch Ice Exception when create object adapter\n"));
		return S_FALSE;			 
	}

	_iceProperPtr=_ic->getProperties();
	std::map<std::string, std::string>::iterator itProper;
	for( itProper = _gDummySSCfg.iceProperties.begin(); 
		itProper!=_gDummySSCfg.iceProperties.end();
		itProper++)
	{		
		_iceProperPtr->setProperty(itProper->first, itProper->second);
	}
	pauseMax = min( 10000, max(pauseMax,0) );
	pauseMin = min( 10000, max(pauseMin,0) );
	if ( pauseMin > pauseMax )
	{
		int pause = pauseMin;
		pauseMin = pauseMax;
		pauseMax = pause;
	}
	try
	{
		_objAdapter=_ic->createObjectAdapterWithEndpoints( "Dummy" , _gDummySSCfg.bindEndPoint);
	}
	catch ( const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnStart,"catch Ice Exception when create object adapter\n"));
		return S_FALSE;			 
	}
	_serviceInstance = new DummyService(_objAdapter,_gDummySSCfg.dataPath, _gDummySSCfg.nodeId, _gDummySSCfg.spigotIds, _gDummySSCfg.targetTime);
	_serviceInstance->listenerEndpoint = _gDummySSCfg.replicaSubscriberEndpoint;
	_serviceInstance->updateInterval = 60*1000;
	_service= _serviceInstance ;

	if( !_gDummySSCfg.eventChannel.empty() ) {
		_serviceInstance->connectToEventChannel(_gDummySSCfg.eventChannel);
	}

	ReplicaUpdater  replicaUpdater(*_serviceInstance);
	if( !_gDummySSCfg.replicaSubscriberEndpoint.empty() )
	{
		replicaUpdater.start();
	}
	try
	{
		_objAdapter->add(_service,_ic->stringToIdentity("DummySvc"));	
		_objAdapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(OnStart,"catch Ice Exception when create object adapter\n"));
		return S_FALSE;			 
	}
	return BaseZQServiceApplication::OnStart();
}

HRESULT DummySvc::OnUnInit()
{
	try{
		_service=NULL;
		if (_objAdapter)
		{
			_objAdapter->deactivate();
		}
		if (_ic)
		{
			_ic->destroy();
		}
	}catch(...){}
	ZQ::common::setGlogger();
	return BaseZQServiceApplication ::OnUnInit();	
}
HRESULT DummySvc::OnStop()
{
	_objAdapter->deactivate();
	return BaseZQServiceApplication::OnStop();
}
