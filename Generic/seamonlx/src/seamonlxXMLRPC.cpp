/** @file seamonlxXMLRPC.cpp
 *
 *  
 *  SeaChange International
 *  
 *  Revision History
 *  
 *  06-02-2010 mjc    This header added to file
 * 06-14-2010 mjc		Added Seamonlx Debug support for the backdoor Shutdown command via RPC2  
 * 07-01-2010 mjc		Added NetworkAdapters and Interfaces module.
 *  
 * 
 */
 
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <cassert>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include "ServerHwXmlrpc.h"
#include "ServerEnvXmlrpc.h"
#include "StorageAdapterXmlrpc.h"
#include "EnclProcessorXmlrpc.h"
#include "DiskXmlrpc.h"
#include "MgmtPortXmlrpc.h"
#include "SystemHealthXmlrpc.h"
#include "SHASConfigXmlrpc.h"
#include "SHASCountersXmlrpc.h"
#include "RpmsXmlrpc.h"
#include "SvcsXmlrpc.h"
#include "ConfigXmlrpc.h"
#include "AlertXmlrpc.h"
#include "BMCXmlrpc.h"
#include "NetworkAdaptersXmlrpc.h"
#include "InfinibandAdaptersXmlrpc.h"

#include "seamonlx.h"
#include "common.h"

// Globals used in common and tracing
Trace   *traceClass = NULL;
bool    seamonlx::Trace::traceIsActive = FALSE;
xmlrpc_c::serverAbyss  *myAbyssServer = NULL;

ServerEnvXmlrpc *GpObjServerEnv = NULL;

extern int ALLOBJECTSSETUP;

extern THREAD_INFO_ELEM	ThreadInfo[MAX_START_THREADS];

using namespace std;
using namespace seamonlx;

/**
*
* Seamon DEBUG Class Code for the Shutdown RPC cpommand
* Invoke via xmlrpc call to "Seamonlx Shurtdown" as the arguments.
*
*/

/**
* External functions
*/
extern void		shutdownwriteAlert(void);
extern void		shutdownAbyssServer(void);

/**
* Debug class 
*/
class SeamonlxDebugXmlrpc : public xmlrpc_c::method {
	
  public:

	/**
	 * A constructor
	 */ 
	SeamonlxDebugXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method.
	 */ 
    void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	
};



/**
* Function: xmlrpclistener()
* Thread to start all of the XMLRPC class instances and get the AbyssSever running.
*
*/
void *xmlrpclistener(void *pParam)
{
	long threadnum;
	
	threadnum = (long)((long *)pParam);
	
	if( ThreadInfo[threadnum].threadstate == THREAD_STARTED ) {
		traceClass->LogTrace(ZQ::common::Log::L_DEBUG,  "+++++++      xmlrpclistener thread %d CALLED AGAIN and it's already started!   +++++++", threadnum );
		pthread_exit(NULL);
	}
	
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	while (!seamonlx_shutdown) {
		// jie
		try {

			traceClass->LogTrace(ZQ::common::Log::L_INFO, "xmlrpclistener, starting... (thread pParam value is %d)", threadnum );
			
            /* Instantiate the registry*/
			xmlrpc_c::registry myRegistry;
			
			/* instantiate objects */
			xmlrpc_c::methodPtr const seamonlxdebugP(new SeamonlxDebugXmlrpc);
			xmlrpc_c::methodPtr const serverHwP(new ServerHwXmlrpc);
			xmlrpc_c::methodPtr const serverEnvP(new ServerEnvXmlrpc);
			xmlrpc_c::methodPtr const storageAdaptersP(new StorageAdapterXmlrpc);
			xmlrpc_c::methodPtr const enclosuresP( new EnclProcessorXmlrpc );
			xmlrpc_c::methodPtr const disksP( new DiskXmlrpc );

			xmlrpc_c::methodPtr const shasConfigP( new SHASConfigXmlrpc );
			xmlrpc_c::methodPtr const shasCountersP( new SHASCountersXmlrpc );

			/* update objects */
			xmlrpc_c::method * adaptersP = dynamic_cast<xmlrpc_c::method *>(storageAdaptersP.get());
			StorageAdapterXmlrpc *adtp = dynamic_cast<StorageAdapterXmlrpc *>(adaptersP);
			adtp->update();
			
			xmlrpc_c::method * enclsP = dynamic_cast<xmlrpc_c::method *>(enclosuresP.get());
			EnclProcessorXmlrpc *elp = dynamic_cast<EnclProcessorXmlrpc *>(enclsP);
			elp->update();
			
			xmlrpc_c::method * diskdrivesP = dynamic_cast<xmlrpc_c::method *>(disksP.get());
			DiskXmlrpc *drp = dynamic_cast<DiskXmlrpc *>(diskdrivesP);
			drp->update();

			xmlrpc_c::methodPtr const systemAlertP(new SystemAlertXmlrpc);
			xmlrpc_c::methodPtr const systemRpmsP(new SystemRpmsXmlrpc);
			xmlrpc_c::methodPtr const systemSvcsP(new SystemSvcsXmlrpc);
			xmlrpc_c::methodPtr const systemConfigP(new SystemConfigXmlrpc);
			xmlrpc_c::methodPtr const systemBMCP(new SystemBMCXmlrpc);

			xmlrpc_c::methodPtr const networkAdaptersP(new NetworkAdaptersXmlrpc);

			xmlrpc_c::methodPtr const infinibandAdaptersP(new InfinibandAdaptersXmlrpc);
						
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "xmlrpclistener, creating MgmtPortXmlrpc instance");
			xmlrpc_c::methodPtr const mgmtPortP(new MgmtPortXmlrpc);
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "xmlrpclistener, creating SystemHealthXmlrpc instance");
			xmlrpc_c::methodPtr const systemHealthP(new SystemHealthXmlrpc);

			/* An example of updating the content of one object */
			//  	xmlrpc_c::method * serverHardwareP = dynamic_cast<xmlrpc_c::method *>(serverHwP.get());
            //   	ServerHwXmlrpc *shp = dynamic_cast<ServerHwXmlrpc *>(serverHardwareP);
            // 	shp->update();

			// Get our System Health Object class pointer
			xmlrpc_c::method * sysHealthP = dynamic_cast<xmlrpc_c::method *>(systemHealthP.get());
            SystemHealthXmlrpc *pObjSystemHealth = dynamic_cast<SystemHealthXmlrpc *>(sysHealthP);
			
			// Get our Server Environment Object class pointer
			xmlrpc_c::method * serverEnvironmentP = dynamic_cast<xmlrpc_c::method *>(serverEnvP.get());
            ServerEnvXmlrpc *pObjServerEnv = dynamic_cast<ServerEnvXmlrpc *>(serverEnvironmentP);
			GpObjServerEnv = pObjServerEnv;

			traceClass->LogTrace(ZQ::common::Log::L_INFO, "xmlrpclistener, setting SystemHealth Obj Class ptr for ServerEnv instance");			

			// Get our MgmtPort class ptr
			xmlrpc_c::method * managementPortP = dynamic_cast<xmlrpc_c::method *>(mgmtPortP.get());
            MgmtPortXmlrpc *pObjMgmtPort = dynamic_cast<MgmtPortXmlrpc *>(managementPortP);

            // Set the object pointers in the system health class object for it's future reference to them
			pObjSystemHealth -> setObjClassPointer( pObjServerEnv );
			pObjSystemHealth -> setObjClassPointer( pObjMgmtPort );
			
            /* Register methods  */
			if (DEBUG) {
				myRegistry.addMethod("Seamonlx", seamonlxdebugP);
			}

			myRegistry.addMethod("Server", serverHwP);
			myRegistry.addMethod("ServerEnv", serverEnvP);
			myRegistry.addMethod("StorageAdapter", storageAdaptersP);

			if (strcasestr(Gsv_ProductType, "UML") != NULL) {
				myRegistry.addMethod("Enclosure", enclosuresP);
			}

			myRegistry.addMethod("Disk", disksP);
			myRegistry.addMethod("SystemHealth", systemHealthP);
			myRegistry.addMethod("MgmtPort", mgmtPortP);

			//
			// Only register if shas is configured
			//
			if ((strcasestr(Gsv_ProductType, "UML") != NULL) && 
				(SHAS_PRESENT == SUCCESS)) {
				myRegistry.addMethod("SHASConfig", shasConfigP);
				myRegistry.addMethod("SHASCounters", shasCountersP);
			}

			myRegistry.addMethod("SystemAlert", systemAlertP);
			myRegistry.addMethod("SystemRpms", systemRpmsP);
			myRegistry.addMethod("SystemSvcs", systemSvcsP);
			myRegistry.addMethod("SystemConfig", systemConfigP);
			myRegistry.addMethod("SystemBMC", systemBMCP);

			myRegistry.addMethod("NetworkAdapters", networkAdaptersP);
			myRegistry.addMethod("InfinibandAdapters", infinibandAdaptersP);
			
			
			// Register the shutdown capability with the server
			xmlrpc_c::serverAbyss::shutdown shutdown(myAbyssServer);
			myRegistry.setShutdown(&shutdown);
			
			traceClass->LogTrace(ZQ::common::Log::L_INFO, "xmlrpclistener, starting Abyss Web Server.");		
            myAbyssServer = new xmlrpc_c::serverAbyss( xmlrpc_c::serverAbyss::constrOpt()
                                      .registryP(&myRegistry)
                                      .portNumber(Gsv_SEAMONLX_XMLRPC_PORT)
                                      .logFileName("/tmp/xmlrpc_log")
									  .serverOwnsSignals(false)
									  );
			//
			// global flag only set here
			//
			ALLOBJECTSSETUP = SUCCESS;

			myAbyssServer->run();

			traceClass->LogTrace(ZQ::common::Log::L_INFO, "xmlrpclistener, serverAbyss run call completed.");
			
		} catch (exception const& e) {
			cerr << "xmlrpclistener catch: Something failed.  " << e.what() << endl;
		}

	}

	traceClass->LogTrace(ZQ::common::Log::L_INFO, "xmlrpclistener, cleaning up classes.");

#if 0	
	Disks* disk_p = Disks::instance();
	if( disk_p ){
		disk_p->clear();
		delete disk_p;
	}

	EnclProcessors* encl_p = EnclProcessors::instance();
	if( encl_p){
		encl_p->clear();
		delete encl_p;
	}

	StorageAdapters* adpt_p = StorageAdapters::instance();
	if(adpt_p){
		adpt_p->clear();
		delete adpt_p;
	}
#endif	
	//
	// Grab this Mutex and not release upon exit
	// This mUtex used in MonitorSERVERENV() thread
	//
	pthread_mutex_lock(&GlobalObjMonThreadServerEnv);
	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);
}

//
// shutdownAbyssServer
// Invokes the shutdown of the Abyss Server.
//
void shutdownAbyssServer( void )
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "shutdownAbyssServer Enter");
	if (myAbyssServer != NULL) {
		myAbyssServer->terminate();
	}
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "shutdownAbyssServer Exit");
}


/**
* SeamonlxDebugXmlrpc Constructor
*/
SeamonlxDebugXmlrpc::SeamonlxDebugXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	/* Constructor */
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SeamonlxDebugXmlrpc::SeamonlxDebugXmlrpc, Enter");

	this->_signature = "A:s";

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", "*** This for debugging purposes only ***");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s", "*** cmd is Seamonlx Shutdown         ***");
	FormatHelpString(formattedstring, locstr);

	this->_help = formattedstring; 

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SeamonlxDebugXmlrpc::SeamonlxDebugXmlrpc, Exit");
}

/**
* SeamonlxDebugXmlrpc execute method
*/
void 
SeamonlxDebugXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const  retvalP)
{
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SeamonlxDebugXmlrpc::execute, Enter");

	if( retvalP ) ;   // Gets rid of compiler warning
	
	paramList.verifyEnd(1);
	string param1 =  paramList.getString(0);
	
	if( param1.size() <= 0 )	{
		throw(xmlrpc_c::fault("Additional parameter not specified and is required",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));	
	}
	
   /* parse the parameter */
   if( param1.compare( "Shutdown" ) == 0 ) {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "\tShutdown command received. Setting seamonlx_shutdown flag.");
		seamonlx_shutdown = 1;
		shutdownwriteAlert(); 
		shutdownAbyssServer();
		throw(xmlrpc_c::fault("Shutdown invoked via debug path",
							  xmlrpc_c::fault::CODE_UNSPECIFIED));	
   }
   

    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SeamonlxDebugXmlrpc::execute, Exit");
}

/* End of file seamonXMLRPC.cpp */
