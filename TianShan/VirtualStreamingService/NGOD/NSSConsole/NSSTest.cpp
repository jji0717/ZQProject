#include <winsock2.h>
#include "NSSImpl.h"
#include "NSSConfig.h"
#include "NSSCfgLoader.h"
#include "IPathHelperObj.h"
#include "IceLog.h"

const char *DefaultConfigPath = "C:\\tianshan\\etc\\NSS_Sample.xml";
const int DefaultThreadPoolSize = 30;

#include <iostream> 
#include <strstream> 
using namespace std; 

extern int32 iTimeOut;

int main(int argc, char *argv[])
{
	//::ZQTianShan::NSS::NSSConfig pConfig(DefaultConfigPath);
	//pConfig.ConfigLoader();

	ZQ::common::Config::Loader<::ZQTianShan::NSS::NSSCfg> pConfig(DefaultConfigPath);
	pConfig.load(DefaultConfigPath, true);	
	iTimeOut = pConfig._timeOut.time * 1000;

	::ZQ::common::FileLog logFile(pConfig._nssLog.path.c_str(),
								  pConfig._nssLog.level,
								  pConfig._nssLog.logNum,
								  pConfig._nssLog.size,
								  pConfig._nssLog.buffer,
								  pConfig._nssLog.flushTimeout);

	// Initialize the thread pool
	int32 iThreadPoolSize = 0;
	iThreadPoolSize = pConfig._stBind.threadPoolSize;

	if (iThreadPoolSize > DefaultThreadPoolSize || iThreadPoolSize <= 0)
		iThreadPoolSize = DefaultThreadPoolSize;
	
	::ZQ::common::NativeThreadPool pool(iThreadPoolSize);
	// Initialize the Communicator.
	//
	::ZQ::common::FileLog iceLogFile(pConfig._nssIceLog.path.c_str(),
									 pConfig._nssIceLog.level,
									 pConfig._nssIceLog.size,
									 pConfig._nssIceLog.buffer,
									 pConfig._nssIceLog.flushTimeout);

	::TianShanIce::common::IceLogIPtr iceLog = new ::TianShanIce::common::IceLogI(&iceLogFile);
	//::TianShanIce::common::IceLogI iceLog(&iceLogFile);

	::Ice::CommunicatorPtr communicator;
	try
	{
		Ice::PropertiesPtr props = Ice::createProperties();
		for (ZQTianShan::NSS::IceProperties::props::iterator iceIter = pConfig._iceProperties._props.begin();
		iceIter != pConfig._iceProperties._props.end(); iceIter++)
		{
			props->setProperty((*iceIter).name.c_str(), (*iceIter).value);
		}
#if ICE_INT_VERSION / 100 >= 303

		::Ice::InitializationData initData;
		initData.properties = props;
		//initWithConfig(initData.properties);

		initData.logger = iceLog;

		communicator = Ice::initialize(initData);

#else
		communicator = Ice::initializeWithPropertiesAndLogger(argc, argv, props, iceLog);
#endif 
		//communicator->waitForShutdown();
	} 
	catch (const Ice::Exception& e)
	{
		cerr << e << endl;
		return -1;
	} 
	catch (const char* msg) 
	{
		cerr << msg << endl;
		return -1;
	}

	//get media server configuration
	string strServerPath;
	uint16 uServerPort;
	for (ZQTianShan::NSS::MediaCluster::Servers::const_iterator MCiter = pConfig._mediaCluster._servers.begin();
	MCiter != pConfig._mediaCluster._servers.end(); MCiter++)
	{
		
		strServerPath = MCiter->address.c_str();
		uServerPort = MCiter->port;
		break;
	}

	// Initialize the NSS Service		
	::ZQTianShan::NSS::NSSEnv env(logFile, pool, communicator, pConfig._iceStorm.endPoint.c_str(),
								pConfig._stBind.endPoint.c_str(),
								pConfig._dataBase.path.c_str(),
								pConfig._dataBase.runtimePath.c_str());

	::ZQTianShan::NSS::NGODStreamServiceImpl pNssService(logFile, 
											pool, communicator, strServerPath, uServerPort, 
											pConfig._sessionGroup._groups, env);

	// Create a Freeze database connection.
	//
	communicator->waitForShutdown();
	return 0;
}