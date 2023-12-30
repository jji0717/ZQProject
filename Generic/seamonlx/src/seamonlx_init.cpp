// init junk
//
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "common.h"

using namespace std;
using namespace seamonlx;


#define LOCAL 0
#define REMOTE 1

//
// globals used for initialization flags
//
int DEBUG						= 0;
int TRACING						= 0;
int NOXMLRPC					= 0;
int rchaIsLoaded				= 0;
int bccfgIsLoaded				= 0;
int seamonlx_shutdown			= 0;
int StartPackageChecks			= FAILURE;
int StartServiceChecks			= FAILURE;
int ALLOBJECTSSETUP				= FAILURE;


//
// globals structs and Arrays used in initialization
//
RPM_NAME			RPMNameArray[MAX_RPM_ALLOWED];
SERVICE_NAME		ServiceNameArray[MAX_SVC_ALLOWED];
CONFIG_STRUCT		SysConfigData;
BMC_STRUCT			BmcStructArray[2];						// 0 = LOCAL, 1 = REMOTE


//
//
//
extern void		shutdownwriteAlert(void);
extern void		shutdownAbyssServer(void);
extern void		listenersetup(void);

extern void		processAlertMsg(char *fac, char *sev, 
								char *component, char *desc, int alertid);

extern THREAD_INFO_ELEM	ThreadInfo[MAX_START_THREADS];

///////////////////////////////////////////////////////////////////////////////
///
//  signal_handler
///
/// Signal Handler.
///
/// \param  signo   - Signal number to handle.
///
///////////////////////////////////////////////////////////////////////////////

void signal_handler(int signo)
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "signal_handler, Enter");
	switch(signo) {
		case SIGINT:
		case SIGTERM:
			openlog("seamonlx",(LOG_CONS | LOG_PERROR | LOG_PID), LOG_DAEMON);
			syslog(LOG_INFO,"Terminating\n");
			closelog();
			seamonlx_shutdown = 1;
			
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "invoking shutdownwriteAlert" );
			shutdownwriteAlert(); 
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "invoking shutdownAbyssServer" );
			shutdownAbyssServer();
		
			sleep(1);
			break;
		case SIGALRM:
			break;
		case SIGPIPE:		// We must explicitly ignore this case for the Abyss Web Server
			break;
		case SIGUSR1:		// Placeholder for future handler to reload our configuration info
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "SIGUSR1 received. Will reload configuration..." );
			break;
		default:
			break;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "signal_handler, Exit");
}

//
// Inform user he can run as non daemon process or enable tracing
// used in debugging (gdb seamonlx); from within gbd run -d or from cmd line do same
//
//
void checkCmdLine(int argc, char *argv[])
{
	int c;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "checkCmdLine, Enter");
	while ((c = getopt (argc, argv, "DdNnTtHh")) != -1) {
		switch (c) {
			
			case 'D':
			case 'd':
				DEBUG = 1;    // global allows us to run independently not as a daemon
				break;

			case 'N':
			case 'n':
				NOXMLRPC = 1;    // global allows us to run independently not as a daemon
				break;
				
			case 'T':
			case 't':
				TRACING = 1;
				break;				

			case 'H' :
			case 'h': 
				printf("USAGE: seamonlx [-dnht] \n");
				printf("\tseamonlx [-d] DEBUG: do not start as daemon process on\n");
				printf("\tseamonlx [-t] TRACE: enable tracing to log file\n");
				printf("\tseamonlx [-n] NOXMLRPC: do NOT start the XML RPC thread\n");
				printf("\tseamonlx [-h] HELP: print this message\n");
				break;
				
			default:
				NOXMLRPC = 0;
				DEBUG = 0;
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "checkCmdLine, Exit");
}


//
// Fill in Global SreviceNameArray
// servicelist contains a space separated list of service names
// obtained from config.dat file lookup
//
static void BuildServiceArray(char *servicelist)
{
	char *token;
	char *saveptr;
	char seps[]   = " \n";
	int i = 0;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "BuildServiceArray, Enter");
	//
	// Build the Array to check for
	//
	token = strtok_r(servicelist, seps, &saveptr);
	while (token != NULL) {
		if ( i < MAX_SVC_ALLOWED) {
			sprintf(ServiceNameArray[i].name, "%s", token);
			token = strtok_r(NULL, seps, &saveptr);
			i++;
		} else {
			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "ServiceNameArray Bounds exceeded" );
			break;
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "BuildServiceArray, Exit");
}

//
// Parse the config.dat file by calling the shell script
// getsvcfromconfigdat.sh to get the list of services running
// 

static int CreateServicesList(void)
{
	FILE *fd;
	char locbuf[BUFF4K];
	char cmd[BUFF256];
	char msg[BUFF80];
	int	 rtn = FAILURE;
	
	char *testfgets;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CreateServicesList, Enter");
	memset(cmd,0, sizeof(cmd));
	memset(locbuf,0, BUFF4K);
	sprintf(cmd, "/bin/bash %s", GETSVCSSCRIPT);

traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		testfgets = fgets(locbuf, BUFF1K, fd);
		pclose(fd);
		if (testfgets != NULL) {
				BuildServiceArray(locbuf);
				rtn = SUCCESS;
		}

	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}
		
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CreateServicesList, Exit");
	return rtn;
}

//
// Fill in Global RPMNameArray and correct version
//
static void BuildRPMArray(char *packagelist)
{
	char *token;
	char *saveptr;
	char version[BUFF80];
	char seps[]   = " \n";
	int i = 0;
	int rtn = FAILURE;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "BuildRPMArray, Enter");
	//
	// Build the Array to check for
	//
	token = strtok_r(packagelist, seps, &saveptr);
	while (token != NULL) {
		rtn = FAILURE;
		//
		// insert dash because it is delimiter for 
		// version in POSTinstall file
		//
		sprintf(version, "%s-", token);
		rtn = getRPMversion(version);
		if (rtn == SUCCESS) {
			if ( i < MAX_RPM_ALLOWED) {
				//
				// remove residual newline
				//
				version[strlen(version)-1] = '\0';
				sprintf(RPMNameArray[i].name, "%s", version);

				//
				// get description of Rpm
				//
				getSummaryValue(RPMNameArray[i].name, RPMNameArray[i].summary);

				i++;
			} else {
				traceClass->LogTrace(ZQ::common::Log::L_INFO,  "RPMNameArray Bounds exceeded" );
				break;
			}
		}
		//
		// get next rpm item
		//
		token = strtok_r(NULL,seps, &saveptr);
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "BuildRPMArray, Exit");
}

//
// call shell to get the list of RPMS from the config.dat file
//
static int CreatePackagesList(void)
{
	FILE *fd;
	char *testfgets;
	char locbuf[BUFF1K];
	char cmd[BUFF80];
	char msg[BUFF80];
	int	 rtn = FAILURE;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CreatePackagesList, Enter");
	memset(locbuf, 0, BUFF1K);
	
	sprintf(cmd, "/bin/bash %s", GETRPMSSCRIPT);
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
			testfgets = fgets(locbuf, BUFF1K, fd);
			pclose(fd);
			BuildRPMArray(locbuf);
			rtn = SUCCESS;
		} else {
			sprintf(msg, "%s: %s", __FUNCTION__, "Rpms post install list missing");
			die(msg, 0, Gsv_AI_FILE_MISSING);
		}

	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}
		
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CreatePackagesList, Exit");
	return rtn;
}

//
// Insert the hostname of the partner node into global SysConfigStruct
//
void GetPartnerHostName(void)
{
	FILE	*fd;
	char	*testfgets;
	char	*token;
	char	*saveptr;

	char	cmd[BUFF256];
	char	tmpstr[BUFF256];
	char	locaddr[BUFF256];
	char	locbuf[BUFF256];

	char	seps[]   = "\n";
	
	if (strcasecmp(SysConfigData.NodeType,"primary") == 0) {
		ParseAndExtract(CONFIGDATAFILE, 
				"infiniband.ipv4.secondary.ipaddr",		"=", locaddr);
	} else {
		ParseAndExtract(CONFIGDATAFILE, 
				"infiniband.ipv4.primary.ipaddr",		"=", locaddr);
	}

	sprintf(cmd,"rsh %s hostname 2>/dev/null", locaddr);
traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		testfgets = fgets(locbuf, BUFF256, fd);
		pclose(fd);
		if (testfgets != NULL) {
			token = strtok_r(locbuf, seps, &saveptr);
			while (token != NULL) {
				//
				// remove extra newline char
				//
				sprintf(tmpstr, "%s", token);
				token = strtok_r(NULL, seps, &saveptr);
			}
			sprintf(SysConfigData.PartnerHostName, "%s", tmpstr);
		} else {
			sprintf(SysConfigData.PartnerHostName, "%s", "UNKNOWN");
		}
	}
}
//
// Stores config info into global data struct, used to present
// xmlrpc system config object
//
static void PopulateSysConfig(void)
{
	int rtn = FAILURE;
	char tmpstr[BUFF256];

	char *token;
	char *saveptr;
	char seps[]   = " -";

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "PopulateSysConfig, Enter");
	//
	// read prod type and desc form producttype
	// format UMx-yyyyy
	//
	rtn = ReadFromTypeFile(PRODUCTTYPEFILE, tmpstr);
	if (rtn == SUCCESS) {
		token = strtok_r(tmpstr, seps, &saveptr);
		if (token != NULL) {
			sprintf(SysConfigData.ProductType, "%s", token);
		}

		token = strtok_r(NULL, seps, &saveptr);
		if (token != NULL) {
			//
			// remove extra newline char
			//
			sprintf(SysConfigData.Description, "%s", token);
		}
	} else {
		sprintf(SysConfigData.ProductType,			"%s", "Server");
		sprintf(SysConfigData.Description,			"%s", "UNKNOWN");
	}

	rtn = ReadFromTypeFile(NICTYPEFILE, SysConfigData.NICType);
	rtn = ReadFromTypeFile(STORAGETYPEFILE, SysConfigData.StorageType);
	rtn = ReadFromTypeFile(SYSTEMTYPEFILE, SysConfigData.SystemType);
	rtn = ReadFromTypeFile(NODETYPEFILE, SysConfigData.NodeType);

	ParseAndExtract(CONFIGDATAFILE, "network.*.management.interface",		"=", 
		SysConfigData.ManagementIf);
	ParseAndExtract(CONFIGDATAFILE, "ipmi.ipv4.default.primary.ipaddr",		"=", 
		SysConfigData.BMCPrimaryIpAddr);
	ParseAndExtract(CONFIGDATAFILE, "ipmi.ipv4.default.secondary.ipaddr",	"=", 
		SysConfigData.BMCSecondaryIpAddr);
	ParseAndExtract(CONFIGDATAFILE, "ipmi.ipv4.default.gateway",			"=", 
		SysConfigData.BMCGatewayAddr);
	ParseAndExtract(CONFIGDATAFILE, "ipmi.user.name",						"=", 
		SysConfigData.BMCUserName);
	ParseAndExtract(CONFIGDATAFILE, "ipmi.user.password",					"=", 
		SysConfigData.BMCpassword);

	//
	// insert data for SysConfigData.PartnerHostName
	// if an error insert "UNKNOWN"
	//
	GetPartnerHostName();

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "PopulateSysConfig, Exit");
}


//
// Stores BMC info into global data struct, used to present
// xmlrpc system BMC Info object
//
static void PopulateSysBmcInfo(void)
{
	int rtn = FAILURE;
	int i = 0;
	char tmpstr[BUFF256];
	char cmdargs[BUFF1K] = {0};

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "PopulateSysBmcInfo, Enter");
	//
	// Populate known stuff from previous config data look
	//
    if ( strcasecmp(SysConfigData.NodeType,"primary") == 0 )
    {
        sprintf(BmcStructArray[LOCAL].PrimaryNodeType,	"%s", "true");
        sprintf(BmcStructArray[REMOTE].PrimaryNodeType,	"%s", "false");

        sprintf(BmcStructArray[LOCAL].BMCIpAddr,		"%s", SysConfigData.BMCPrimaryIpAddr);
        sprintf(BmcStructArray[REMOTE].BMCIpAddr,		"%s", SysConfigData.BMCSecondaryIpAddr);
    }
    else if ( strcasecmp(SysConfigData.NodeType,"secondary") == 0 )
    {
        sprintf(BmcStructArray[LOCAL].PrimaryNodeType,	"%s", "false");
        sprintf(BmcStructArray[REMOTE].PrimaryNodeType,	"%s", "true");

        sprintf(BmcStructArray[LOCAL].BMCIpAddr,			"%s", SysConfigData.BMCSecondaryIpAddr);
        sprintf(BmcStructArray[REMOTE].BMCIpAddr,		"%s", SysConfigData.BMCPrimaryIpAddr);
    }
    else
    {
        sprintf(BmcStructArray[LOCAL].PrimaryNodeType,	"%s", "UNKNOWN");
        sprintf(BmcStructArray[REMOTE].PrimaryNodeType,	"%s", "UNKNOWN");
        sprintf(BmcStructArray[LOCAL].BMCIpAddr,		"%s", "UNKNOWN");
        sprintf(BmcStructArray[REMOTE].BMCIpAddr,		"%s", "UNKNOWN");
    }
	
	sprintf(BmcStructArray[LOCAL].DefGatewayAddr, SysConfigData.BMCGatewayAddr);
	strcpy(BmcStructArray[REMOTE].DefGatewayAddr, BmcStructArray[LOCAL].DefGatewayAddr);

	sprintf(BmcStructArray[LOCAL].UserName,			"%s", SysConfigData.BMCUserName);
	strcpy(BmcStructArray[REMOTE].UserName, BmcStructArray[LOCAL].UserName);

	sprintf(BmcStructArray[LOCAL].Password,			"%s", SysConfigData.BMCpassword);
	strcpy(BmcStructArray[REMOTE].Password, BmcStructArray[LOCAL].Password);

	ParseAndExtract(CONFIGDATAFILE, "ipmi.ipv4.default.ipsrc",	"=",		
		BmcStructArray[LOCAL].IPAddrSource);
	
	strcpy(BmcStructArray[REMOTE].IPAddrSource, BmcStructArray[LOCAL].IPAddrSource);

	//
	// determine primary or secondary node we are on
	//
	rtn = ReadFromTypeFile(NODETYPEFILE, tmpstr);
	if (rtn == SUCCESS) {
		for (i = LOCAL; i <= REMOTE; i++) {
			if (i == LOCAL) {
				sprintf(cmdargs, "%s", "bmc info");  // local bmc info
			} else {
				//
				// remote node info
				//

                if ( strcasecmp(BmcStructArray[i].BMCIpAddr,"UNKNOWN") != 0 ) {
                    sprintf(cmdargs, "-I lanplus -H %s -U %s -P %s bmc info", 
                            BmcStructArray[i].BMCIpAddr, 
                            BmcStructArray[i].UserName, BmcStructArray[i].Password);
                }
			}

            // if the IP Addr was unknown we did not get the data from the REMOTE
            if ( cmdargs[0] != 0 )
            {
                ParseAndExtractIPMI(IPMITOOL, cmdargs, "IPMI Version",		BmcStructArray[i].IPMIvers);
                ParseAndExtractIPMI(IPMITOOL, cmdargs, "Firmware Revision", BmcStructArray[i].FirmwareVers);
                ParseAndExtractIPMI(IPMITOOL, cmdargs, "Device Available",	BmcStructArray[i].Status);

                if (strcasecmp(BmcStructArray[i].Status, "yes") == 0) {
                    sprintf(BmcStructArray[i].Status,"%s", "OK");
                } else {
                    if (!(strcasecmp(BmcStructArray[i].Status, "UNKNOWN") == 0)){
                        sprintf(BmcStructArray[i].Status,"%s", "Failed");
                    }
                }
            }

            cmdargs[0] = 0;

			if (i == LOCAL) {
				sprintf(cmdargs, "%s", "bmc watchdog get");  // get local bmc watchdog info
			} else {
				//
				// remote node watchdog info
				//
                if ( strcasecmp(BmcStructArray[i].BMCIpAddr,"UNKNOWN") != 0 ) {
                    sprintf(cmdargs, "-I lanplus -H %s -U %s -P %s bmc watchdog get", 
                            BmcStructArray[i].BMCIpAddr, 
                            BmcStructArray[i].UserName, BmcStructArray[i].Password);
                }
			}

            // if the IP Addr was unknown we did not get the data from the REMOTE
            if ( cmdargs[0] != 0 )
            {
                ParseAndExtractIPMI(IPMITOOL, cmdargs, "Present Countdown", BmcStructArray[i].WatchdogTimerInterval);
                ParseAndExtractIPMI(IPMITOOL, cmdargs, "Watchdog Timer Is", BmcStructArray[i].WatchdogTimerStatus);
            }
		}	
	} 

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "PopulateSysBmcInfo, Exit");
}


//
// helper just look for all files
//
static void CheckFilesExist(void)
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckFilesExist, Enter");
	AlertIfFileDoesNotExist(SEAMONLXCONFIGFILE);

	//
	// Check if SHAS should be running, set flag
	//
	if (access(SHASCONFIGFILE, F_OK) != -1) {
		SHAS_PRESENT = SUCCESS;
	}
	
	//
	// check for existence of baseline files
	//
	AlertIfFileDoesNotExist(NICTYPEFILE);	
	AlertIfFileDoesNotExist(STORAGETYPEFILE);
	AlertIfFileDoesNotExist(SYSTEMTYPEFILE);
	AlertIfFileDoesNotExist(PRODUCTTYPEFILE);
	AlertIfFileDoesNotExist(NODETYPEFILE);
	AlertIfFileDoesNotExist(CONFIGDATAFILE);
	
	AlertIfFileDoesNotExist(IPMITOOL);
	AlertIfFileDoesNotExist(MATRIXUTILITY);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckFilesExist, Exit");
}


//
// Populate default AlertIds
//
static void GetGlobalAlertIds(void)
{
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "GetGlobalAlertIds, Enter");
	Gsv_AI_FILE_MISSING = GetDefaultHexValue("AI_FILE_MISSING", AI_FILE_MISSING);
	Gsv_AI_THREAD_NOT_RUNNING = GetDefaultHexValue("AI_THREAD_NOT_RUNNING", AI_THREAD_NOT_RUNNING);
	Gsv_AI_POPEN_ERROR = GetDefaultHexValue("AI_POPEN_ERROR", AI_POPEN_ERROR);
	Gsv_AI_FOPEN_ERROR = GetDefaultHexValue("AI_FOPEN_ERROR", AI_FOPEN_ERROR);
	Gsv_AI_FILE_DELETE_ERROR = GetDefaultHexValue("AI_FILE_DELETE_ERROR", AI_FILE_DELETE_ERROR);

	Gsv_AI_MALLOC_FAILED = GetDefaultHexValue("AI_MALLOC_FAILED", AI_MALLOC_FAILED);
	Gsv_AI_THREAD_NOT_RUNNING = GetDefaultHexValue("AI_THREAD_NOT_RUNNING", AI_THREAD_NOT_RUNNING);
	Gsv_AI_THREAD_FAILED_TO_START = GetDefaultHexValue("AI_THREAD_FAILED_TO_START", AI_THREAD_FAILED_TO_START);
	Gsv_AI_SIGNAL_HANDLER_ERROR = GetDefaultHexValue("AI_SIGNAL_HANDLER_ERROR", AI_SIGNAL_HANDLER_ERROR);
	Gsv_AI_SOCKET_ERROR = GetDefaultHexValue("AI_SOCKET_ERROR", AI_SOCKET_ERROR);


	Gsv_AI_SVC_NOT_STARTED = GetDefaultHexValue("AI_SVC_NOT_STARTED", AI_SVC_NOT_STARTED);
	Gsv_AI_SVC_NOT_CURRENT_RUN_LEVEL = GetDefaultHexValue("AI_SVC_NOT_CURRENT_RUN_LEVEL", AI_SVC_NOT_CURRENT_RUN_LEVEL);

	Gsv_AI_RPM_NOT_INSTALLED = GetDefaultHexValue("AI_RPM_NOT_INSTALLED", AI_RPM_NOT_INSTALLED);
	Gsv_AI_RPM_WRONG_VERSION = GetDefaultHexValue("AI_RPM_WRONG_VERSION", AI_RPM_WRONG_VERSION);
	Gsv_AI_SVC_NOT_CURRENT_RUN_LEVEL = GetDefaultHexValue("AI_SVC_NOT_CURRENT_RUN_LEVEL", AI_SVC_NOT_CURRENT_RUN_LEVEL);


	Gsv_AI_DISK_OFFLINE = GetDefaultHexValue("AI_DISK_OFFLINE", AI_DISK_OFFLINE);
	Gsv_AI_DISK_SMART_ERROR = GetDefaultHexValue("AI_DISK_SMART_ERROR", AI_DISK_SMART_ERROR);
	Gsv_AI_DISK_TEMP_ERROR = GetDefaultHexValue("AI_DISK_TEMP_ERROR", AI_DISK_TEMP_ERROR);
	Gsv_AI_DISK_IO_ERROR = GetDefaultHexValue("AI_DISK_IO_ERROR", AI_DISK_IO_ERROR);
	Gsv_AI_DISK_BLKR_ERROR = GetDefaultHexValue("AI_DISK_BLKR_ERROR", AI_DISK_BLKR_ERROR);
	Gsv_AI_DISK_BLKW_ERROR = GetDefaultHexValue("AI_DISK_BLKW_ERROR", AI_DISK_BLKW_ERROR);
	Gsv_AI_DISK_UNKNOWN_ERROR = GetDefaultHexValue("AI_DISK_UNKNOWN_ERROR", AI_DISK_UNKNOWN_ERROR);


	Gsv_AI_ENC_STATUS = GetDefaultHexValue("AI_ENC_STATUS", AI_ENC_STATUS);
	Gsv_AI_ENC_PHY_LINK_STATUS = GetDefaultHexValue("AI_ENC_PHY_LINK_STATUS", AI_ENC_PHY_LINK_STATUS);
	Gsv_AI_ENC_PHY_LINK_RATE = GetDefaultHexValue("AI_ENC_PHY_LINK_RATE", AI_ENC_PHY_LINK_RATE);
	Gsv_AI_ENC_PHY_ERROR_COUNT = GetDefaultHexValue("AI_ENC_PHY_ERROR_COUNT", AI_ENC_PHY_ERROR_COUNT);
	Gsv_AI_ENC_TEMP_STATUS = GetDefaultHexValue("AI_ENC_TEMP_STATUS", AI_ENC_TEMP_STATUS);
	Gsv_AI_ENC_PWR_STATUS = GetDefaultHexValue("AI_ENC_PWR_STATUS", AI_ENC_PWR_STATUS);
	Gsv_AI_ENC_FANS_STATUS = GetDefaultHexValue("AI_ENC_FANS_STATUS", AI_ENC_FANS_STATUS);
	Gsv_AI_ENC_DISK_ELEM_SES_STATUS = GetDefaultHexValue("AI_ENC_DISK_ELEM_SES_STATUS", AI_ENC_DISK_ELEM_SES_STATUS);
	Gsv_AI_ENC_UNKNOWN_ERROR = GetDefaultHexValue("AI_ENC_UNKNOWN_ERROR", AI_ENC_UNKNOWN_ERROR);

	Gsv_AI_SA_STATUS = GetDefaultHexValue("AI_SA_STATUS", AI_SA_STATUS);
	Gsv_AI_SA_PHY_LINK_STATUS = GetDefaultHexValue("AI_SA_PHY_LINK_STATUS", AI_SA_PHY_LINK_STATUS);

	Gsv_AI_SHAS_FAIL_TO_GET_CORE_INFO = GetDefaultHexValue("AI_SHAS_FAIL_TO_GET_CORE_INFO", AI_SHAS_FAIL_TO_GET_CORE_INFO);
	Gsv_AI_SHAS_PARSE_MSG_ERROR = GetDefaultHexValue("AI_SHAS_PARSE_MSG_ERROR", AI_SHAS_PARSE_MSG_ERROR);

	Gsv_AI_FAIL_ON_UPDATE_DISKS = GetDefaultHexValue("AI_FAIL_ON_UPDATE_DISKS", AI_FAIL_ON_UPDATE_DISKS);
	Gsv_AI_UDEV_ADD_MODULE = GetDefaultHexValue("AI_UDEV_ADD_MODULE", AI_UDEV_ADD_MODULE);
	Gsv_AI_UDEV_REMOVE_MODULE = GetDefaultHexValue("AI_UDEV_REMOVE_MODULE", AI_UDEV_REMOVE_MODULE);
	Gsv_AI_UDEV_ADD_DISK = GetDefaultHexValue("AI_UDEV_ADD_DISK", AI_UDEV_ADD_DISK);
	Gsv_AI_UDEV_REMOVE_DISK = GetDefaultHexValue("AI_UDEV_REMOVE_DISK", AI_UDEV_REMOVE_DISK);
	Gsv_AI_UDEV_UNKNOWN_STATUS = GetDefaultHexValue("AI_UDEV_UNKNOWN_STATUS", AI_UDEV_UNKNOWN_STATUS);

	Gsv_AI_ALERT_RANGE_ERROR = GetDefaultHexValue("AI_ALERT_RANGE_ERROR", AI_ALERT_RANGE_ERROR);
	Gsv_AI_FAIL_TO_LOG = GetDefaultHexValue("AI_FAIL_TO_LOG", AI_FAIL_TO_LOG);

	Gsv_AI_LSPCI_CHANGED = GetDefaultHexValue("AI_LSPCI_CHANGED", AI_LSPCI_CHANGED);
	Gsv_AI_NETWORK_ADAPTER_ERROR = GetDefaultHexValue("AI_NETWORK_ADAPTER_ERROR", AI_NETWORK_ADAPTER_ERROR);
	Gsv_AI_NETWORK_INTERFACE_ERROR = GetDefaultHexValue("AI_NETWORK_INTERFACE_ERROR", AI_NETWORK_INTERFACE_ERROR);
	
	Gsv_AI_INFINIBAND_ADAPTER_ERROR = GetDefaultHexValue("AI_INFINIBAND_ADAPTER_ERROR", AI_INFINIBAND_ADAPTER_ERROR);
	Gsv_AI_INFINIBAND_INTERFACE_ERROR = GetDefaultHexValue("AI_INFINIBAND_INTERFACE_ERROR", AI_INFINIBAND_INTERFACE_ERROR);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "GetGlobalAlertIds, Exit");
}

//
// just populate all initial thread info
//
void InitializeThreadArray(void)
{
	int i = 0;
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InitializeThreadArray, Enter");
	ThreadInfo[i].threadHandle = AlertThread;
	ThreadInfo[i].threadstartfn = AlertThreadStart;
	sprintf(ThreadInfo[i].threadstartname, "%s", "AlertThreadStart");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = buildcompthread;
	ThreadInfo[i].threadstartfn = BuildCompLists;
	sprintf(ThreadInfo[i].threadstartname, "%s", "BuildCompLists");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = acceptconnectionsthread;
	ThreadInfo[i].threadstartfn = acceptconnections;
	sprintf(ThreadInfo[i].threadstartname, "%s", "acceptconnections");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = monitorSHASThread;
	ThreadInfo[i].threadstartfn = MonitorSHAS;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorSHAS");
	ThreadInfo[i].threadstate = THREAD_STOPPED;
	
	i++;
	ThreadInfo[i].threadHandle = monitorUDEVThread;
	ThreadInfo[i].threadstartfn = MonitorUDEV;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorUDEV");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = monitorServicesThread;
	ThreadInfo[i].threadstartfn = MonitorServices;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorServices");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = monitorPackagesThread;
	ThreadInfo[i].threadstartfn = MonitorPackages;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorPackages");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = xmlrpcThread;
	ThreadInfo[i].threadstartfn = xmlrpclistener;
	sprintf(ThreadInfo[i].threadstartname, "%s", "xmlrpclistener");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = monitorDRIVEThread;
	ThreadInfo[i].threadstartfn = MonitorDRIVE;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorDRIVE");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = monitorEnclosureProcThread;
	ThreadInfo[i].threadstartfn = MonitorENCLOSUREPROC;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorENCLOSUREPROC");
	ThreadInfo[i].threadstate = THREAD_STOPPED;
	
	i++;
	ThreadInfo[i].threadHandle = monitorStorageAdapterThread;
	ThreadInfo[i].threadstartfn = MonitorSTORAGEADAPTER;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorSTORAGEADAPTER");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = monitorServerEnvThread;
	ThreadInfo[i].threadstartfn = MonitorSERVERENV;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorSERVERENV");
	ThreadInfo[i].threadstate = THREAD_STOPPED;

	i++;
	ThreadInfo[i].threadHandle = monitornetworkadaptersthread;
	ThreadInfo[i].threadstartfn = MonitorNetworkAdapters;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorNetworkAdapters");
	ThreadInfo[i].threadstate = THREAD_STOPPED;
/*
	i++;
	ThreadInfo[i].threadHandle = monitorinfinibandadaptersthread;
	ThreadInfo[i].threadstartfn = MonitorInfinibandAdapters;
	sprintf(ThreadInfo[i].threadstartname, "%s", "MonitorInfinibandAdapters");
	ThreadInfo[i].threadstate = THREAD_STOPPED;
*/
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InitializeThreadArray, Exit");

}

//
// Initialize
//
// Set up signal handlers
// start up listener thread for UNICAST Alert messaging
// start threads
//
int InitSeamonlx(void)
{
	int rtn = FAILURE;
	char msg[BUFF256];
	
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InitSeamonlx, Enter");
	//
	// register signal handlers
	//
	if ((signal(SIGINT, signal_handler) == SIG_ERR)		|| (signal(SIGTERM, signal_handler) == SIG_ERR) || 
		(signal(SIGPIPE, signal_handler) == SIG_ERR)	|| (signal(SIGALRM, signal_handler) == SIG_ERR)) 
	{
			sprintf(msg, "%s:Exiting failed to setup signal handlers!!!",  __FUNCTION__);
			die(msg, 1, 0);
	
	} 
	else 
	{

		rtn = SUCCESS;

		if (ReadFromTypeFile(PRODUCTTYPEFILE, Gsv_ProductType) != SUCCESS) {
			strcpy(Gsv_ProductType, "UNKNOWN");
		}

		GetHostName();
		
		//
		// check for files present, Alert accordingly
		//
		CheckFilesExist();

		Gsv_MAXCOMPONENTLISTSIZE = GetDefaultValue("MAXCOMPONENTLISTSIZE", MAXCOMPONENTLISTSIZE);
		//
		// Get global Port values
		//
		Gsv_SEAMONLX_XMLRPC_PORT = GetDefaultValue("SEAMONLX_XMLRPC_PORT", SEAMONLX_XMLRPC_PORT);
		Gsv_SEAMONLX_ALERT_PORT = GetDefaultValue("SEAMONLX_ALERT_PORT", SEAMONLX_ALERT_PORT);
		Gsv_CALLHOME_PORTNUM = GetDefaultValue("CALLHOME_PORTNUM", CALLHOME_PORTNUM);

		//
		// set up default Global sleep values
		//
		Gsv_MONITOR_DRIVE_SLEEP_INTERVAL = GetDefaultValue("MONITOR_DRIVE_SLEEP_INTERVAL", MONITOR_DRIVE_SLEEP_INTERVAL);
		Gsv_MONITOR_ENCLOSURE_SLEEP_INTERVAL = GetDefaultValue("MONITOR_ENCLOSURE_SLEEP_INTERVAL", MONITOR_ENCLOSURE_SLEEP_INTERVAL);
		Gsv_MONITOR_STORAGEADAPTER_SLEEP_INTERVAL = GetDefaultValue("MONITOR_STORAGEADAPTER_SLEEP_INTERVAL", MONITOR_STORAGEADAPTER_SLEEP_INTERVAL);
		Gsv_MONITOR_SERVERENV_SLEEP_INTERVAL = GetDefaultValue("MONITOR_SERVERENV_SLEEP_INTERVAL", MONITOR_SERVERENV_SLEEP_INTERVAL);
		Gsv_MONITOR_PACKAGES_SLEEP_INTERVAL = GetDefaultValue("MONITOR_PACKAGES_SLEEP_INTERVAL", MONITOR_PACKAGES_SLEEP_INTERVAL);
		Gsv_MONITOR_SERVICES_SLEEP_INTERVAL = GetDefaultValue("MONITOR_SERVICES_SLEEP_INTERVAL", MONITOR_SERVICES_SLEEP_INTERVAL);
		Gsv_MONITOR_NETWORK_ADAPTERS_SLEEP_INTERVAL = GetDefaultValue("MONITOR_NETWORK_ADAPTERS_SLEEP_INTERVAL", MONITOR_NETWORK_ADAPTERS_SLEEP_INTERVAL);
		Gsv_MONITOR_INFINIBAND_ADAPTERS_SLEEP_INTERVAL = GetDefaultValue("MONITOR_INFINIBAND_ADAPTERS_SLEEP_INTERVAL", MONITOR_INFINIBAND_ADAPTERS_SLEEP_INTERVAL);
		
		GetGlobalAlertIds();
		InitializeThreadArray();
		PopulateSysConfig();
		PopulateSysBmcInfo();

		memset(ServiceNameArray,0, sizeof(ServiceNameArray));
		if (CreateServicesList() == SUCCESS) {
			getcurrrunlevel();
			StartServiceChecks = SUCCESS;
		}

		memset(RPMNameArray,0, sizeof(RPMNameArray));
		if (CreatePackagesList() == SUCCESS) {
			StartPackageChecks = SUCCESS;
		}

		//
		// set up listener
		//
		listenersetup();
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "InitSeamonlx, Exit");
	return rtn;
}
//
// At initialization check tot see if post manufactured slot changes
//
void ChecklspciDiffs(void)
{
	char cmd[BUFF256];
	char msg[BUFF256];
	int alertid;
	FILE *fd;
	char test;
	int	 rtn = FAILURE;

	//
	// checks for diffs of 2 files -- indifferent to what the diffs are 
	//
	sprintf(cmd, "%s", "/sbin/lspci -Dn > /tmp/lspci.tmp 2>&1; echo $?");

traceClass->LogTrace(ZQ::common::Log::L_INFO, "command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		rtn = fscanf(fd, "%c", &test);
		pclose(fd);
		if ((rtn != EOF) && (rtn != 0)) {
			rtn = IsFileDifferent(LSPCIPOSTINSTALL, "/tmp/lspci.tmp");
			if (rtn == SUCCESS) {
				// process alert
				alertid = Gsv_AI_LSPCI_CHANGED;
				sprintf(msg, "%s", "lspci Slot configuration has changed");
				processAlertMsg("seamonlx", "ERR", CN_SeaMon_LX, msg, alertid);
			}
		} 
		
		//
		// get rid oftmp file
		//
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,"%s", "/tmp/lspci.tmp");
		if (remove(cmd) != 0) {
			sprintf(msg, "%s: remove() failed for file /tmp/lspci.tmp", __FUNCTION__);
			die(msg, 0, Gsv_AI_FILE_DELETE_ERROR);
		}

	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}
}

//
// startAllThreads
//
void startthreads(void)
{
	int i, j = 0;
	char msg[BUFF256];

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "startthreads, Enter");
	for (i = 0; i < MAX_START_THREADS; i++) 
	{

		//
		// for debug purposes you may not want to start the XMLRPC thread
		//
		if (NOXMLRPC) 
		{
			//
			// cant monitor disks or enclosure if XMLRPC thread does not complete start up
			//
			if ((strcasecmp(ThreadInfo[i].threadstartname, "xmlrpclistener") == 0)			||
				(strcasecmp(ThreadInfo[i].threadstartname, "MonitorDRIVE") == 0)			||
				(strcasecmp(ThreadInfo[i].threadstartname, "MonitorSTORAGEADAPTER") == 0)	||
				(strcasecmp(ThreadInfo[i].threadstartname, "MonitorSERVERENV") == 0)		||
				(strcasecmp(ThreadInfo[i].threadstartname, "MonitorENCLOSUREPROC") == 0)) {

				ALLOBJECTSSETUP = SUCCESS;
				continue;
			}
		}

		//
		// check to see if thread already started
		//
		if (ThreadInfo[i].threadstate != THREAD_STARTED) 
		{

			if ((strcasecmp(ThreadInfo[i].threadstartname, "MonitorSHAS") == 0) && 
				(SHAS_PRESENT == SUCCESS)) {
				
				//
				// check to start Shas monitor thread, both modules required
				//
				if ((IsModuleLoaded("rcha") != SUCCESS)  || 
					(IsModuleLoaded("bccfg") != SUCCESS)) {
					continue;
				}

			}

			//
			// All XML RPC objects must complete their initialization first
			//
			if ((strcasecmp(ThreadInfo[i].threadstartname, "MonitorDRIVE") == 0)			||
				(strcasecmp(ThreadInfo[i].threadstartname, "MonitorSTORAGEADAPTER") == 0)	||
				(strcasecmp(ThreadInfo[i].threadstartname, "MonitorSERVERENV") == 0)	||
				(strcasecmp(ThreadInfo[i].threadstartname, "MonitorENCLOSUREPROC") == 0)) {
					for (j = 0; j < 20; j++) {
						if (ALLOBJECTSSETUP == SUCCESS) {
							break;
						}
						sleep(5);  // wait 5 secs try again
					}
			}

			traceClass->LogTrace(ZQ::common::Log::L_INFO,  "\tStarting Thread %d, Name %s", i, ThreadInfo[i].threadstartname );
			
			if (pthread_create(&ThreadInfo[i].threadHandle, NULL, ThreadInfo[i].threadstartfn, (void *) i) != 0) 
			{
				sprintf(msg, "%s: Cannot start %s thread", __FUNCTION__, ThreadInfo[i].threadstartname);
				die(msg, 0, Gsv_AI_THREAD_NOT_RUNNING);
			}
			traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "thread created seccessfully Thread.No:%lu ",ThreadInfo[i].threadHandle);
		}
	}
	
	//
	{
		for (i = 0; i < MAX_START_THREADS; i++) 
		if(0 == pthread_join(ThreadInfo[i].threadHandle,NULL))
			traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "The thread-No:%lu  exit normally",ThreadInfo[i].threadHandle);
		else
			traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "The thread-No:%lu  exit abnormally",ThreadInfo[i].threadHandle);
	}
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "startthreads, Exit");
}

