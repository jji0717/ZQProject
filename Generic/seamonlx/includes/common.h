/*
 * @file common.h
 *
 *
 *  header file contains common utility functions
 *  
 *
 *
 *  Revision History
 *  
 *  03-10-2010 	jz     	Created ( jie.zhang@schange.com)
 *  04-09-2010 	mjc  	Update for Object State and Event Tracing classes
 *  05-26-2010 	mjc  	Added StringResource.h inclusion
 *  06-22-2010 	mjc  	Added NEWLINE_CHARACTER define; Added a few more BUFF size defines;
 *								added getHealthStateString() method.
 * 
 */

#ifndef SEAMONCOMMON_H
#define SEAMONCOMMON_H

#include <vector>
#include <utility>
#include <list>
#include <syslog.h>
#include "trace.h"
#include "StringResource.h"

#include "ZQ_common_conf.h"
#include "Log.h"
#include "FileLog.h"


#include "AlertId.h"
#include "AlertComponent.h"

using namespace std;



#define THREAD_STARTED  1
#define THREAD_STOPPED  0

#define MAX_START_THREADS  13

#define SUCCESS 0
#define FAILURE -1

#define	MAX_SVC_ALLOWED		100
#define	MAX_RPM_ALLOWED		100

#define ALERT_LOG_REC_MAX    100


#define BUFF4K					4096
#define BUFF2K					2048
#define BUFF1K					1024
#define BUFF512					512
#define BUFF256					256
#define BUFF128				    128
#define BUFF80					80
#define BUFF64					64
#define BUFF32					32
#define BUFF16					16
#define BUFF8					8

extern int DEBUG;
extern int SHAS_PRESENT;
extern int AlertLogParseComplete;
extern char RUNLEVEL;



#define			MONITOR_DRIVE_SLEEP_INTERVAL 				300			// 5 minutes
#define			MONITOR_ENCLOSURE_SLEEP_INTERVAL 			300			// 5 minutes
#define			MONITOR_STORAGEADAPTER_SLEEP_INTERVAL 		300			// 5 minutes
#define			MONITOR_SERVERENV_SLEEP_INTERVAL 			1200		// 20 minutes
#define			MONITOR_PACKAGES_SLEEP_INTERVAL				86400		// 24 hrs
#define			MONITOR_SERVICES_SLEEP_INTERVAL				900			// 15 mins
#define			MONITOR_NETWORK_ADAPTERS_SLEEP_INTERVAL		3600		// 60 mins
#define			MONITOR_INFINIBAND_ADAPTERS_SLEEP_INTERVAL		3600		// 60 mins

#define			SEAMONLX_XMLRPC_PORT						59732
#define			SEAMONLX_ALERT_PORT							59733
#define			CALLHOME_PORTNUM							3000
#define			MAXCOMPONENTLISTSIZE						100

#define			HELPLINEEND                                  80

//
// define Component Names
//
#define CN_Server_Environmentals		"Server_Environmentals"
#define CN_Management_Port				"Management_Port"
#define CN_Target_Ports					"Target_Ports"
#define CN_InfiniBand					"InfiniBand"
#define CN_OpenIB_Subnet_Mgmt			"OpenIB_Subnet_Mgmt"
#define CN_SeaMon_LX					"SeaMon_LX"
#define CN_BMC_IPMI						"BMC_IPMI"
#define CN_Enclosure_Environmentals 	"Enclosure_Environmentals"
#define CN_Storage_Interconnect 		"Storage_Interconnect"
#define CN_Storage_Configuration 		"Storage_Configuration"
#define CN_SHAS_State 					"SHAS_State"
#define CN_CIFS 						"CIFS"
#define CN_FTP							"FTP"
#define CN_Software_Configuration 		"Software_Configuration"
#define CN_System_Services 				"System_Services"
#define CN_Kernel 						"Kernel"
#define CN_Hyper_FS 					"Hyper_FS"
#define CN_IPStor 						"IPStor"
#define CN_StreamSmith 					"StreamSmith"
#define CN_VFlow 						"VFlow"
#define CN_SeaFS 						"SeaFS"
#define CN_Sparse_Cache 				"Sparse_Cache"
#define CN_Distributed_Cache 			"Distributed_Cache"
#define CN_Sentry_Service 				"Sentry_Service"
#define CN_C2_Server 					"C2_Server"



//
// Paths to files and executables 
//
#define RPMPOSTINSTALLFILE		"/usr/local/seachange/install/rpmlist_postinstall.txt"	
#define	GETRPMSSCRIPT			"/usr/local/seamonlx/bin/getrpmsfromconfigdat.sh"
#define	GETSVCSSCRIPT			"/usr/local/seamonlx/bin/getsvcfromconfigdat.sh"
#define LSPCIPOSTINSTALL		"/usr/local/seachange/install/lspci_postinstall.txt"
#define NICTYPEFILE				"/usr/local/seachange/configdata/nictype"
#define STORAGETYPEFILE			"/usr/local/seachange/configdata/storagetype"
#define SYSTEMTYPEFILE			"/usr/local/seachange/configdata/systemtype"
#define PRODUCTTYPEFILE			"/usr/local/seachange/configdata/producttype"
#define NODETYPEFILE			"/usr/local/seachange/configdata/nodetype"
#define CONFIGDATAFILE			"/usr/local/seachange/configdata/config.dat"
#define SEAMONLXCONFIGFILE		"/usr/local/seamonlx/config/seamonlx.conf"
#define IPMITOOL				"/usr/bin/ipmitool"
#define SHASCONFIGFILE			"/usr/local/shas/shascfg"
#define MATRIXUTILITY			"/usr/local/seachange/bin/matrixUtility"

#define				NEWLINE_CHARACTER				'\n'

#define WHERE __LINE__, __FILE__

typedef struct	_THREAD_INFO_ELEM
{
	void *(*threadstartfn)(void *);
	char							threadstartname[BUFF80];
	pthread_t						threadHandle;
	int								threadstate;
}THREAD_INFO_ELEM, *PTHREAD_INFO_ELEM;


typedef struct _SERVICE_NAME
{
	char	name[BUFF80];							// Service Name
}SERVICE_NAME, *PSERVICE_NAME;


typedef struct _RPM_NAME
{
	char	name[BUFF80];							// RPM Package Name
	char	summary[BUFF256];
}RPM_NAME, *PRPM_NAME;


typedef struct _CONFIG_STRUCT						// System Config data
{
	char	ProductType[BUFF80];
	char	NodeType[BUFF80];
	char	Description[BUFF80];
	char	ManagementIf[BUFF80];
	char	BMCPrimaryIpAddr[BUFF80];
	char	BMCSecondaryIpAddr[BUFF80];
	char	BMCGatewayAddr[BUFF80];
	char	BMCUserName[BUFF80];
	char	BMCpassword[BUFF80];
	char	NICType[BUFF80];
	char	StorageType[BUFF80];
	char	SystemType[BUFF80];
	char	PartnerHostName[BUFF80];
} CONFIG_STRUCT, *PCONFIG_STRUCT;


typedef struct _BMC_STRUCT					// BMC Info 
{
	char	IPMIvers[BUFF80];				
	char	FirmwareVers[BUFF80];			
	char	WatchdogTimerStatus[BUFF80];	
	char	WatchdogTimerInterval[BUFF80];	
	char	PrimaryNodeType[BUFF80];		
	char	BMCIpAddr[BUFF80];				
	char	IPAddrSource[BUFF80];
	char	DefGatewayAddr[BUFF80];
	char	UserName[BUFF80];				
	char	Password[BUFF80];				
	char	Status[BUFF80];					
} BMC_STRUCT, *PBMC_STRUCT;

//
// Global extern sleep intervals 
//
extern int Gsv_MONITOR_DRIVE_SLEEP_INTERVAL; 
extern int Gsv_MONITOR_ENCLOSURE_SLEEP_INTERVAL;
extern int Gsv_MONITOR_STORAGEADAPTER_SLEEP_INTERVAL;
extern int Gsv_MONITOR_SERVERENV_SLEEP_INTERVAL;
extern int Gsv_MONITOR_PACKAGES_SLEEP_INTERVAL;
extern int Gsv_MONITOR_SERVICES_SLEEP_INTERVAL;
extern int Gsv_MONITOR_NETWORK_ADAPTERS_SLEEP_INTERVAL;
extern int Gsv_MONITOR_INFINIBAND_ADAPTERS_SLEEP_INTERVAL;


//
// Global MAX num entries for a Component List
//
extern int		Gsv_MAXCOMPONENTLISTSIZE;

//
// Global port nums for sockets
//
extern int Gsv_SEAMONLX_XMLRPC_PORT;
extern int Gsv_SEAMONLX_ALERT_PORT;
extern int Gsv_CALLHOME_PORTNUM;

extern char Gsv_hostname[BUFF256];
extern char Gsv_ProductType[BUFF256];
//
// extern Global Component List
//
extern pthread_mutex_t		ComponentListMutex;

extern list<AlertComponent>Server_Environmentals_complist;
extern list<AlertComponent>Management_Port_complist;
extern list<AlertComponent>Target_Ports_complist;
extern list<AlertComponent>InfiniBand_complist;
extern list<AlertComponent>OpenIB_Subnet_Mgmt_complist;
extern list<AlertComponent>SeaMon_LX_complist;	
extern list<AlertComponent>BMC_IPMI_complist;
extern list<AlertComponent>Enclosure_Environmentals_complist;
extern list<AlertComponent>Storage_Interconnect_complist;
extern list<AlertComponent>Storage_Configuration_complist;
extern list<AlertComponent>SHAS_State_complist;
extern list<AlertComponent>CIFS_complist;
extern list<AlertComponent>FTP_complist;
extern list<AlertComponent>Software_Configuration_complist;
extern list<AlertComponent>System_Services_complist;
extern list<AlertComponent>Kernel_complist;
extern list<AlertComponent>Hyper_FS_complist;
extern list<AlertComponent>IPStor_complist;
extern list<AlertComponent>StreamSmith_complist;
extern list<AlertComponent>VFlow_complist;	
extern list<AlertComponent>SeaFS_complist;
extern list<AlertComponent>Sparse_Cache_complist;
extern list<AlertComponent>Distributed_Cache_complist;
extern list<AlertComponent>Sentry_Service_complist;
extern list<AlertComponent>C2_Server_complist;



//
// Mutex and cond vars
//
extern pthread_mutex_t 			InfinibandAdaptersMutex; 
extern pthread_mutex_t 			NetworkAdaptersMutex; 
extern pthread_mutex_t 			StorageAdaptersMutex; 

extern pthread_mutex_t			GlobalObjMonThreadServerEnv;
extern pthread_mutex_t			AlertMutex;
extern pthread_mutex_t			ConnArrayMutex;
extern pthread_mutex_t			ServerEnvMutex;
extern pthread_mutex_t			ShasConfigMutex;
extern pthread_mutex_t			ShasCountersMutex;
extern pthread_mutex_t			EncMutex; 
extern pthread_mutex_t			hscMutex; 


//
// for AlertComponentLists
//
extern pthread_mutex_t			CN_Server_Environmentals_M;
extern pthread_mutex_t			CN_Management_Port_M;
extern pthread_mutex_t			CN_Target_Ports_M;
extern pthread_mutex_t			CN_InfiniBand_M;
extern pthread_mutex_t			CN_OpenIB_Subnet_Mgmt_M;
extern pthread_mutex_t			CN_SeaMon_LX_M;
extern pthread_mutex_t			CN_BMC_IPMI_M;
extern pthread_mutex_t			CN_Enclosure_Environmentals_M;
extern pthread_mutex_t			CN_Storage_Interconnect_M;
extern pthread_mutex_t			CN_Storage_Configuration_M;
extern pthread_mutex_t			CN_SHAS_State_M;
extern pthread_mutex_t			CN_CIFS_M;
extern pthread_mutex_t			CN_FTP_M;
extern pthread_mutex_t			CN_Software_Configuration_M;
extern pthread_mutex_t			CN_System_Services_M;
extern pthread_mutex_t			CN_Kernel_M;
extern pthread_mutex_t			CN_Hyper_FS_M;
extern pthread_mutex_t			CN_IPStor_M;
extern pthread_mutex_t			CN_StreamSmith_M;
extern pthread_mutex_t			CN_VFlow_M;
extern pthread_mutex_t			CN_SeaFS_M;
extern pthread_mutex_t			CN_Sparse_Cache_M;
extern pthread_mutex_t			CN_Distributed_Cache_M;
extern pthread_mutex_t			CN_Sentry_Service_M;
extern pthread_mutex_t			CN_C2_Server_M;
//
// cond vars
//
extern pthread_cond_t			AlertCond;

//
// Thread start handles
//
extern pthread_t	        acceptconnectionsthread;
extern pthread_t			monitorSHASThread;
extern pthread_t			monitorUDEVThread;
extern pthread_t	        xmlrpcThread;
extern pthread_t			monitorPackagesThread;
extern pthread_t			monitorServicesThread;
extern pthread_t			monitorDRIVEThread;
extern pthread_t			monitorEnclosureProcThread;
extern pthread_t			monitorStorageAdapterThread;
extern pthread_t			monitorServerEnvThread;
extern pthread_t			AlertThread;
extern pthread_t			buildcompthread;
extern pthread_t			monitornetworkadaptersthread;
extern pthread_t			monitorinfinibandadaptersthread;


//
// Thread start routines
//
extern void *acceptconnections(void *);
extern void *MonitorServices(void *pParam);
extern void *MonitorPackages(void *pParam);
extern void *MonitorDRIVE(void *pParam);
extern void *MonitorENCLOSUREPROC(void *pParam);
extern void *MonitorSTORAGEADAPTER(void *pParam);
extern void *MonitorSERVERENV(void *pParam);
extern void *MonitorSHAS(void *pParam);
extern void *MonitorUDEV(void *pParam);
extern void *xmlrpclistener(void *pParam);
extern void *AlertThreadStart(void *pParam);
extern void *BuildCompLists(void *pParam);
extern void *MonitorNetworkAdapters(void *pParam);
extern void *MonitorInfinibandAdapters(void *pParam);


// Function prototypes
void trimSpaces(string &str);
void stringSplit(string str, string delim, vector<string> &results);
pair<string, string> getPair(string in);

// External Function Declarations & references for tracing
extern seamonlx::Trace   *traceClass;    	// Class definition in trace.h
extern int TRACING;                      			// Tracing flag


extern int	ParseAndExtract(char *filename, char *cmpstring, char *fieldsep, char *rtnstring);
extern int	ParseAndExtractIPMI(char *filename, char *args, char *cmpstring, char *rtnstring);
extern int	ReadFromTypeFile(char *filename, char *rtnstring);
extern int	DoesFileExist(char *filename);
extern int	IsPackageInstalled(char *str);
extern void die(char *s, int killit, int alertid);
extern void AlertIfFileDoesNotExist(char *filename);
extern int	GetDefaultHexValue(char *Name, int value);
extern int	GetDefaultValue(char *Name, int value);
extern void getSummaryValue(char *rpmname, char *summmaryvalue);
extern int	getRPMversion(char *verstring);
extern void getcurrrunlevel(void);
extern int	IsSvcAtRunLevel(char *svcname);
extern int	IsModuleLoaded(char *str);
extern int	IsServiceStarted(char *str);
extern int	IsFileDifferent(char *original, char *cmp);
extern void	GetHostName(void);

extern void FormatHelpString(string &helpString, char *locstr);

extern void GetStringFromCmd(string command, string &stringtofill);
extern void GetTagValue(string fullLineString, char *sTag, string &stringtofill);
extern void GetBlockOutput(string command, string &blocktofill);

/*
*
* Classes
*
*/

//todo add rwlock class
class rw_lock
{
  public:
	/**
	 * constructor
	 */
	rw_lock();

	/**
	 * destructor
	 */
	~rw_lock();

	/**
	 * member functions
	 */
	void read_lock();
	void read_unlock();
	void write_lock();
	void write_unlock();

  private:
	pthread_mutex_t    wr_mutex;
	pthread_mutex_t    read_cond_mutex;
	pthread_mutex_t    write_cond_mutex;
	pthread_mutex_t    count_mutex;
	pthread_cond_t     read_cond;
	pthread_cond_t     write_cond;
	int                readyToRead;
	int                readerCount;
};

//
// Object Health State Class
// Used by objects to represent their overall system health state
//
class ObjectHealthState
{
  public:
	/**
	 * constructor
	 */
	ObjectHealthState()
	{
		objectHealthState = STATE_UNKNOWN;
		reasonDescription.clear();
	}

	/**
	 * destructor
	 */
	~ObjectHealthState()
		{
		};

	// Object State enumerator
	typedef enum {
		STATE_OK = 0,
		STATE_DEGRADED,
		STATE_CRITICAL,
		STATE_UNKNOWN
	} ObjHealthStateEnumerator;	
	
	/**
	 * member functions
	 */
	inline void setHealthState( ObjHealthStateEnumerator state )
		{
			objectHealthState = state;
		}
		
	inline string getHealthStateString( ObjectHealthState::ObjHealthStateEnumerator state ) 
	{
		switch( state ) {
		    case ObjectHealthState::STATE_OK:
				return string("Ok");
				break;
				
			case ObjectHealthState::STATE_DEGRADED:
				return string("Degraded");
				break;

		    case ObjectHealthState::STATE_CRITICAL:
				return string("Critical");
				break;
				
			case ObjectHealthState::STATE_UNKNOWN:
				return string("Unknown");
				break;
			}
		return string("UNDEFINED");
	}			
			
	inline ObjHealthStateEnumerator getHealthState( void )
		{
			return objectHealthState;
		}
		
		// Reason description methods /assesors
	inline void appendHealthReasonDescription( string reason )
		{
			size_t	found;
			
			// Append the reason description if it's not already in the string
			// so this method can be called multiple times safely with the same string
			found = reasonDescription.find( reason );
		
			if ( found == string::npos )
			{
				if( !reasonDescription.empty() )
				{
					// For multiples, separate each reason with a semicolon and space
					reasonDescription.append( "; " );
				}
				reasonDescription.append( reason );
			}
		}
			
	inline string getHealthReasonDescription( void )
		{
			return reasonDescription;
		}
		
	inline void clearHealthReasonDescription( void )
		{
			reasonDescription.clear();
		}
		
		
  private:

	ObjHealthStateEnumerator			objectHealthState;
	string										reasonDescription;		// String describing all states but OK
	
};

#endif /* COMMON_H */
