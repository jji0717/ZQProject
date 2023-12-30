
/*
 * 
 * common.cpp
 *
 *
 * Common utility file.
 *  
 *
 *
 *  Revision History
 *  
 *  03-10-2010 Created ( jie.zhang@schange.com)
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <list>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"

using namespace std;

//
// global 
char RUNLEVEL;

//
// Global MAX num entries for a Component List
//
int		Gsv_MAXCOMPONENTLISTSIZE					= MAXCOMPONENTLISTSIZE;

//
// Global Sleep interval values
//
int Gsv_MONITOR_DRIVE_SLEEP_INTERVAL				= MONITOR_DRIVE_SLEEP_INTERVAL; 
int Gsv_MONITOR_ENCLOSURE_SLEEP_INTERVAL			= MONITOR_ENCLOSURE_SLEEP_INTERVAL;
int Gsv_MONITOR_STORAGEADAPTER_SLEEP_INTERVAL		= MONITOR_STORAGEADAPTER_SLEEP_INTERVAL;
int Gsv_MONITOR_SERVERENV_SLEEP_INTERVAL			= MONITOR_SERVERENV_SLEEP_INTERVAL;
int Gsv_MONITOR_PACKAGES_SLEEP_INTERVAL				= MONITOR_PACKAGES_SLEEP_INTERVAL;
int Gsv_MONITOR_SERVICES_SLEEP_INTERVAL				= MONITOR_SERVICES_SLEEP_INTERVAL;
int Gsv_MONITOR_NETWORK_ADAPTERS_SLEEP_INTERVAL		= MONITOR_NETWORK_ADAPTERS_SLEEP_INTERVAL;
int Gsv_MONITOR_INFINIBAND_ADAPTERS_SLEEP_INTERVAL	= MONITOR_INFINIBAND_ADAPTERS_SLEEP_INTERVAL;

int Gsv_SEAMONLX_XMLRPC_PORT						= SEAMONLX_XMLRPC_PORT;
int Gsv_SEAMONLX_ALERT_PORT							= SEAMONLX_ALERT_PORT;
int Gsv_CALLHOME_PORTNUM							= CALLHOME_PORTNUM;

int AlertLogParseComplete = FAILURE;

int SHAS_PRESENT = FAILURE;

char Gsv_hostname[BUFF256];
char Gsv_ProductType[BUFF256];
	
pthread_t	        acceptconnectionsthread;
pthread_t			monitorSHASThread;
pthread_t			monitorUDEVThread;
pthread_t	        xmlrpcThread;
pthread_t			monitorPackagesThread;
pthread_t			monitorServicesThread;
pthread_t			monitorDRIVEThread;
pthread_t			monitorEnclosureProcThread;
pthread_t			monitorStorageAdapterThread;
pthread_t			monitorServerEnvThread;
pthread_t			AlertThread;
pthread_t			buildcompthread;
pthread_t			monitornetworkadaptersthread;
pthread_t			monitorinfinibandadaptersthread;


//
// Mutex global defs
//
pthread_mutex_t		GlobalObjMonThreadServerEnv = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; 
pthread_mutex_t		ServerEnvMutex				= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; 
pthread_mutex_t		AlertMutex					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t		ConnArrayMutex				= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t		ShasConfigMutex             = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; 
pthread_mutex_t		EncMutex					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; 
pthread_mutex_t		hscMutex					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; 
pthread_mutex_t		ShasCountersMutex			= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

//
// mutex for Alert Component List
//
pthread_mutex_t CN_Server_Environmentals_M		= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Management_Port_M			= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Target_Ports_M				= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_InfiniBand_M					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_OpenIB_Subnet_Mgmt_M			= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_SeaMon_LX_M					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_BMC_IPMI_M					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Enclosure_Environmentals_M 	= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Storage_Interconnect_M 		= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Storage_Configuration_M 		= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_SHAS_State_M 				= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_CIFS_M 						= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_FTP_M						= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Software_Configuration_M 	= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_System_Services_M 			= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Kernel_M 					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Hyper_FS_M 					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_IPStor_M 					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_StreamSmith_M 				= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_VFlow_M						= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_SeaFS_M 						= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Sparse_Cache_M 				= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Distributed_Cache_M 			= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_Sentry_Service_M				= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
pthread_mutex_t CN_C2_Server_M 					= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
//
// cond vars 
//
pthread_cond_t		AlertCond					= PTHREAD_COND_INITIALIZER;

//
// Declare Global Component List
//

pthread_mutex_t		ComponentListMutex			= PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

list<AlertComponent>Server_Environmentals_complist;
list<AlertComponent>Management_Port_complist;
list<AlertComponent>Target_Ports_complist;
list<AlertComponent>InfiniBand_complist;
list<AlertComponent>OpenIB_Subnet_Mgmt_complist;
list<AlertComponent>SeaMon_LX_complist;	
list<AlertComponent>BMC_IPMI_complist;
list<AlertComponent>Enclosure_Environmentals_complist;
list<AlertComponent>Storage_Interconnect_complist;
list<AlertComponent>Storage_Configuration_complist;
list<AlertComponent>SHAS_State_complist;
list<AlertComponent>CIFS_complist;
list<AlertComponent>FTP_complist;
list<AlertComponent>Software_Configuration_complist;
list<AlertComponent>System_Services_complist;
list<AlertComponent>Kernel_complist;
list<AlertComponent>Hyper_FS_complist;
list<AlertComponent>IPStor_complist;
list<AlertComponent>StreamSmith_complist;
list<AlertComponent>VFlow_complist;	
list<AlertComponent>SeaFS_complist;
list<AlertComponent>Sparse_Cache_complist;
list<AlertComponent>Distributed_Cache_complist;
list<AlertComponent>Sentry_Service_complist;
list<AlertComponent>C2_Server_complist;


//
THREAD_INFO_ELEM	ThreadInfo[MAX_START_THREADS];
/*******************************************************
 * 
 *                    trimSpaces()
 *
 * Helper function trims leading/trailing white spaces
 * off string
 * 
 *******************************************************/

void trimSpaces(string &str)
{
	size_t startpos = str.find_first_not_of(" \t\n\r");
	size_t endpos = str.find_last_not_of(" \t\n\r");

	if ((startpos == string::npos) ||
		(endpos == string::npos)){
		str = "";
		
	}
	else{
		str = str.substr( startpos, endpos-startpos+1 );
	}
}




/*******************************************************
 * 
 *                    stringSplit()
 *
 * Helper function that split the string by delim and
 * store the result into an vector.
 * 
 * 
 *******************************************************/

void stringSplit(string str, string delim, vector<string> &results)
{
	size_t cutAt;
	while( (cutAt = str.find_first_of(delim)) != str.npos){
		string part = str.substr(0, cutAt);
		trimSpaces(part);
		results.push_back(part);

		str = str.substr(cutAt + delim.length());
	}
	
	trimSpaces(str);
	
	if(str.length() > 0){
		results.push_back(str);
	}		
}



/*******************************************************
 * 
 *                 getPair()
 *
 * Helper function that create a pair from input string
 * that contains a ":". The substring before the ":" is
 * saved as key in the pair, and the substring after the
 * ":" is used as value in the pair.
 * 
 *******************************************************/

pair<string, string>
getPair(string in)
{
	string temp;

	size_t pos = in.find(":", 0);
	temp = in.substr(0, pos);
	in.erase(0, pos+1);

	trimSpaces(temp);
	trimSpaces(in);
	return make_pair(temp, in);
}


/**************** class rw_lock functions *****************/

/**
 * constructor
 */
rw_lock::rw_lock()
{
	pthread_mutex_init( &wr_mutex, NULL );
	pthread_mutex_init( &read_cond_mutex, NULL);
	pthread_mutex_init( &count_mutex, NULL);
	pthread_cond_init( &read_cond, NULL );
	pthread_mutex_init( &write_cond_mutex, NULL );
	pthread_cond_init( &write_cond, NULL );
	readyToRead = 0;
	readerCount = 0;
}

/**
 * destructor
 */
rw_lock::~rw_lock()
{
	pthread_mutex_destroy( &wr_mutex );
	pthread_mutex_destroy( &read_cond_mutex );
	pthread_mutex_destroy( &write_cond_mutex );
	pthread_mutex_destroy( &count_mutex );
	pthread_cond_destroy( &write_cond );
	pthread_cond_destroy( &read_cond );
}

/**
 * Function readlock
 */
void
rw_lock::read_lock()
{
	pthread_mutex_lock( &read_cond_mutex );
	while( !readyToRead ){
		pthread_cond_wait( &read_cond, &read_cond_mutex);
		
	}
	pthread_mutex_unlock( &read_cond_mutex );
	
	pthread_mutex_lock( &count_mutex );
	readerCount ++;
	pthread_mutex_unlock( &count_mutex );
	
}

/**
 * Function read_unlock
 */ 
void
rw_lock::read_unlock()
{
	pthread_mutex_lock( &count_mutex );
	readerCount --;
	pthread_mutex_unlock( &count_mutex );

	pthread_mutex_lock( &write_cond_mutex );
	pthread_cond_broadcast( &write_cond );
	pthread_mutex_unlock( &write_cond_mutex );
}

/**
 * Function write_lock
 */
void
rw_lock::write_lock()
{
	pthread_mutex_lock( &wr_mutex );
	readyToRead = 0;
	pthread_mutex_lock( &write_cond_mutex );
	while( readerCount > 0 ){
		pthread_cond_wait( &write_cond, &write_cond_mutex );
	}
	pthread_mutex_unlock( &write_cond_mutex );
}

void
rw_lock::write_unlock()
{
	pthread_mutex_lock( &read_cond_mutex );
	readyToRead = 1;
	pthread_cond_broadcast( &read_cond );
	pthread_mutex_unlock( &read_cond_mutex );
	pthread_mutex_unlock( &wr_mutex );
}


//
// Helper function to extract contents from config.dat file (eg.)
// network.*.managemnet.interface=eth0
//
// Inputs: filename = input file to search
//			cpmstring = string to find
//			fieldsep = delimiter ('=' or ':' etc)
//			rtnstring = holds result
//
int ParseAndExtract(char *filename, char *cmpstring, char *fieldsep, char *rtnstring)
{
	FILE *fd;
	char cmd[BUFF1K];
	char locbuf[BUFF256];
	char *token;
	char *saveptr;
	char seps[]   = "\n";
	char tmpstr[BUFF32];
	int rtn = FAILURE;
	char msg[BUFF80];
	char *testfgets;

	sprintf(tmpstr,"\'%s\'", fieldsep);
	sprintf(rtnstring, "%s", "UNKNOWN");

	sprintf (cmd, "grep -i \"^%s\" %s | awk -F %s '{print $2}'", cmpstring, filename, tmpstr);
TRACE_LOG("command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		testfgets = fgets(locbuf, BUFF256, fd);
		pclose(fd);
		if (testfgets != NULL) {
			token = strtok_r(locbuf, seps, &saveptr);
			rtn = SUCCESS;
			while (token != NULL) {
				//
				// remove extra newline char
				//
				sprintf(rtnstring, "%s", token);
				token = strtok_r(NULL, seps, &saveptr);
			}
		}
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}

	return rtn;
}

//
// Helper function to extract contents from ipmitool output 
// eg. Device Available : yes
//
// Input: filename = ipmitool path
//			args = ipmi cmds
//			cmpstring = string to find
//			rtn string = holds result
//
//
int ParseAndExtractIPMI(char *filename, char *args, char *cmpstring, char *rtnstring)
{
	FILE *fd;
	char cmd[BUFF1K];
	char msg[BUFF80];
	char locbuf[BUFF256];
	char tmpstr[BUFF256];
	char *token;
	char *testfgets;
	char *saveptr;
	char seps[]   = "\n";
	int  i = 0;
	int rtn = FAILURE;
	
	sprintf(rtnstring, "%s", "UNKNOWN");
	sprintf(cmd, "%s %s | grep -i \"%s\" | awk -F ':' '{print $2}'", filename, args, cmpstring);
TRACE_LOG("command: %s", cmd);
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
			//
			// remove spaces from beginning
			//
			while (tmpstr[i] == ' ') {
				i++;
			}
			strcpy(rtnstring, &tmpstr[i]);	
			rtn = SUCCESS;
		}
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}
	return rtn;
}


//
// Helper function reads from producttype, nodetype or some other file
// returns string to tokenize
//
int ReadFromTypeFile(char *filename, char *rtnstring)
{
	FILE	*fd;
	char	locbuf[BUFF256];
	int		rtn = FAILURE;
	char	*token;
	char	*saveptr;
	char	seps[]   = "\n";

	sprintf(rtnstring, "%s", "UNKNOWN");
	fd = fopen(filename, "r");
	if (fd != NULL) {
		if (fgets(locbuf, BUFF256, fd) != NULL) {
			token = strtok_r(locbuf, seps, &saveptr);
			while (token != NULL) {
				//
				// remove extra newline char
				//
				sprintf(rtnstring, "%s", token);
				token = strtok_r(NULL, seps, &saveptr);
			}
			rtn = SUCCESS;
		} else {
			TRACE_LOG( "Failed fgets() on  %s", filename);
		}
		fclose(fd);
	} else {
		TRACE_LOG( "Failed open on  %s", filename);
	}

	return rtn;
}

//
// Global hlper for testing existence of a file
//
// Input is full filename path
//
// returns (SUCCESS, FAILURE)
int DoesFileExist(char *filename)
{
	int		rtn = FAILURE;
	
	if (access(filename, F_OK) != -1) {
		 rtn = SUCCESS;
	}

	return rtn;
}

//
// Test if Package installed
// Input: char string is Name of RPM
//
int IsPackageInstalled(char *str)
{
	FILE *fd;
	char test;
	char cmd[BUFF80];
	char msg[BUFF80];
	int	 rtn = FAILURE;
	
	//
	// checks for RPM specific version 
	//
	sprintf(cmd, "/bin/rpm -q %s > /dev/null 2>&1; echo $?", str);

TRACE_LOG("command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		rtn = fscanf(fd, "%c", &test);
		pclose(fd);
		if ((rtn != EOF) && (rtn != 0)) {
			rtn = SUCCESS;
		} else {
			rtn = FAILURE;
			sprintf(msg, "%s: %s", __FUNCTION__, "Rpm version not installed");
			die(msg, 0, Gsv_AI_RPM_NOT_INSTALLED);
		}

	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}
	
	return rtn;
}

//
// Test if 2 files are different
// Inputs: orig - filename char string, cmp -- filename to be comapared
//
// returns: 
//
int IsFileDifferent(char *original, char *cmp)
{
	FILE *fd;
	char test;
	char msg[BUFF80];
	char cmd[BUFF80];
	int	 rtn = FAILURE;
	
	//
	// checks for diffs of 2 files -- indifferent to what the diffs are 
	//
	sprintf(cmd, "/usr/bin/diff  -w -i %s %s > /dev/null 2>&1; echo $?", original, cmp);

TRACE_LOG("command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		rtn = fscanf(fd, "%c", &test);
		pclose(fd);
		if ((rtn != EOF) && (rtn != 0)) {
			if (test == 1)
			{
				rtn = SUCCESS;
			}
		} else {
			rtn = FAILURE;
		}
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}
	
	return rtn;
}

//
// Test if service is started
// Input: char string for name of service
//
// If service is started the popen cmd
// read from fd will return 0 upon success
//

int IsServiceStarted(char *str)
{
	FILE *fd;
	char locbuf[BUFF80];
	char cmd[BUFF80];
	int	 rtn = FAILURE;
	char msg[BUFF80];

	TRACE_LOG("IsServiceStarted, Enter");
	memset(cmd,0, sizeof(cmd));
	memset(locbuf,0, sizeof(locbuf));
	sprintf(cmd, "/sbin/service %s status > /dev/null 2>&1; echo $?", str);
TRACE_LOG("command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		rtn = fscanf(fd, "%s", locbuf);
		pclose(fd);
		if ((rtn != EOF) && (rtn != 0) && (strcasestr(locbuf, "0") != NULL)) {
			rtn = SUCCESS;
		} else {
			rtn = FAILURE;
		}

	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}
	
	TRACE_LOG("IsServiceStarted, Exit");
	return rtn;
}

//
// Test if module is loaded use lsmod cmd line call
// Input: char string, name of module
//
// output: rtn SUCCESS = Module is loaded, FALURE = notloaded
//
int IsModuleLoaded(char *str)
{
	FILE *fd;
	char locbuf[BUFF80];
	char cmd[BUFF256];
	int	 rtn = FAILURE;
	char msg[BUFF80];

	TRACE_LOG("IsModuleLoaded, Enter");
	memset(cmd,0, sizeof(cmd));
	memset(locbuf,0, sizeof(locbuf));
	sprintf(cmd, "/sbin/lsmod | grep -e ^%s", str);

TRACE_LOG("command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		rtn = fscanf(fd, "%s", locbuf);
		pclose(fd);

		if ((rtn != EOF) && (rtn != 0) && (strcasestr(locbuf, str) != NULL)) {
			rtn = SUCCESS;
		} else {
			rtn = FAILURE;
		}

	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}
	
	TRACE_LOG("IsModuleLoaded, Exit");
	return rtn;
}

//
// Check to see if Service is configure for proper run level
// svcname, name of service to check on
//
int IsSvcAtRunLevel(char *svcname)
{
	FILE *fd;
	char locbuf[BUFF4K];
	char cmd[BUFF256];
	int	 rtn = FAILURE;
	char msg[BUFF80];

	TRACE_LOG("IsSvcAtRunLevel, Enter");
	memset(cmd,0, sizeof(cmd));
	memset(locbuf,0, BUFF4K);
	
	sprintf(cmd, "/sbin/chkconfig --list %s | grep -q %c:on; echo $?", svcname, RUNLEVEL);

TRACE_LOG("command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		rtn = fscanf(fd, "%s", locbuf);
		pclose(fd);
		if ((rtn != EOF) && (rtn != 0) && (strcasestr(locbuf, "0") != NULL)) {
			rtn = SUCCESS;
		} else {
			rtn = FAILURE;
		}
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}
		

	TRACE_LOG("IsSvcAtRunLevel, Exit");
	return rtn;
}

//
// call to get the current system runlevel
// the runlevel cmd returns a string eg. (N 5)
// we are interested in last value
//
void getcurrrunlevel()
{
	FILE *fd;
	char cmd[BUFF80];
	char msg[BUFF80];
	int	 rtn = FAILURE;
	char tmp;
	
	TRACE_LOG("getcurrrunlevel, Enter");
	sprintf(cmd, "/sbin/runlevel");
TRACE_LOG("command: %s", cmd);
	if ((fd = (FILE *)popen(cmd,"r")) != NULL) {
		rtn = fscanf(fd, "%c %c", &tmp, &RUNLEVEL);
		pclose(fd);
		if (rtn != 2) {
			sprintf(msg, "%s: %s", __FUNCTION__, "sbin runlevel failed");
			die(msg, 0, Gsv_AI_FILE_MISSING);
		}
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}

	TRACE_LOG("getcurrrunlevel, Exit");
}

//
// extract the version from file:
//                 /usr/local/seachange/install/rpmlist_postinstall.txt
//
int getRPMversion(char *verstring)
{
	FILE *fd;
	char locbuf[BUFF256];
	char cmd[BUFF256];
	char msg[BUFF80];
	int	 rtn = FAILURE;
	char *testfgets;
	
	TRACE_LOG("getRPMversion, Enter");
	sprintf(cmd, "/bin/grep %s %s", verstring, RPMPOSTINSTALLFILE);
	memset(locbuf,0,BUFF256);
TRACE_LOG("command: %s", cmd);
	if ((fd = (FILE *)popen(cmd, "r")) != NULL) {
		testfgets = fgets(locbuf, BUFF256, fd);
		pclose(fd);
		if (testfgets != NULL) {
			if (strcasestr(locbuf, verstring) != NULL) {
				//
				// update verstring to contain RPMname-x.x.-xx  whatever
				//
				sprintf(verstring, "%s", locbuf); 
				rtn = SUCCESS;
			}
		} else {
			sprintf(msg, "%s: %s", __FUNCTION__, "wrong Rpm version");
			die(msg, 0, Gsv_AI_RPM_WRONG_VERSION);
		}
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "popen failed");
		die(msg, 0, Gsv_AI_POPEN_ERROR);
	}

	TRACE_LOG("getRPMversion, Exit");
	return rtn;
}

//
// get RpmSummary
//
void getSummaryValue(char *rpmname, char *summmaryvalue)
{
	FILE *fd;
	char cmd[BUFF1K];
	char locbuf[BUFF256];
	char tmpstr[BUFF256];
	char *token;
	char *saveptr;
	char *testfgets;
	char seps[]   = "\n";
	int  i = 0;

	TRACE_LOG("getSummaryValue, Enter");
	sprintf(cmd, "rpm -qil %s | grep Summary | awk -F ':' '{print $2}'", rpmname);

TRACE_LOG("command: %s", cmd);
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

			while (tmpstr[i] == ' ') {
				i++;
			}
			strcpy(summmaryvalue, &tmpstr[i]);	
		}

	} else {
		memset(tmpstr, 0, sizeof(tmpstr));
		sprintf(tmpstr, "%s: %s", __FUNCTION__, "popen failed");
		die(tmpstr, 0, Gsv_AI_POPEN_ERROR);
	}

	TRACE_LOG("getSummaryValue, Exit");
}

//
// Extract values from seamonlx.conf file
// use defaults for values not found
//
// inputs : Name of deault value, value of default value
//
// returns decimal value
//
int GetDefaultValue(char *Name, int value)
{
	int rtn = FAILURE;
	char tmpstr[BUFF80];

	TRACE_LOG("GetDefaultValue, Enter");
	ParseAndExtract(SEAMONLXCONFIGFILE, Name,	"=", tmpstr);
	if (strcasecmp(tmpstr, "UNKNOWN") == 0) {
		rtn = value;
	} else {
		rtn = atoi(tmpstr);
	}

	TRACE_LOG("GetDefaultValue, Exit");
	return rtn;
}

//
// Extract values from seamonlx.conf file
// use defaults for values not found
//
// inputs : Name of default value, value of default value
//
// returns Hex value
//
int GetDefaultHexValue(char *Name, int value)
{
	int rtn = FAILURE;
	char tmpstr[BUFF80];

	TRACE_LOG("GetDefaultValue, Enter");
	ParseAndExtract(SEAMONLXCONFIGFILE, Name,	"=", tmpstr);
	if (strcasecmp(tmpstr, "UNKNOWN") == 0) {
		rtn = value;
	} else {
		sscanf(tmpstr, "%x", &rtn);
	}

	TRACE_LOG("GetDefaultValue, Exit");
	return rtn;
}
	
//
// helper to see if file exists
// Generate Alert for files missing
//
void AlertIfFileDoesNotExist(char *filename)
{	
	char msg[BUFF512];

	TRACE_LOG("AlertIfFileDoesNotExist, Enter");
	if (access(filename, F_OK) == -1) {
		//
		// write log to /var/log/messages
		//
		openlog("seamonlx",(LOG_CONS | LOG_PERROR | LOG_PID), LOG_DAEMON);
		sprintf(msg, "Missing file - %s", filename);
		syslog(LOG_INFO, msg);
		closelog();
	}

	TRACE_LOG("AlertIfFileDoesNotExist, Exit");
}

//
// use echo to get hostname
//
void GetHostName(void)
{
	FILE *fd;
	char cmd[BUFF1K];
	char locbuf[BUFF256];
	char tmpstr[BUFF256];
	char *token;
	char *saveptr;
	char *testfgets;
	char seps[]   = "\n";
	int  i = 0;
	
	TRACE_LOG("GetHostName, Enter");
	sprintf(cmd, "%s", "echo `hostname`");

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

			while (tmpstr[i] == ' ') {
				i++;
			}
			strcpy(Gsv_hostname, &tmpstr[i]);
		}

	} else {
		memset(tmpstr, 0, sizeof(tmpstr));
		sprintf(tmpstr, "%s: %s", __FUNCTION__, "popen failed");
		die(tmpstr, 0, Gsv_AI_POPEN_ERROR);
	}

	TRACE_LOG("GetHostName, Exit");
}

void FormatHelpString(string &helpString, char *locstr)
{
	int i, lenlocstr;

	lenlocstr = strlen(locstr);
	helpString.append(locstr);
	for (i = lenlocstr; i < HELPLINEEND; i++) {
		helpString.push_back(' ');
	}

	helpString.append("\n");
}

/*
//
// Helper routine extracts a value from a cmd grep, awk, etc.
// update stringtofill with the extracted value.
// inputs: string command, string &stringtofill
//
*/
void GetStringFromCmd(string command, string &stringtofill)
{
	FILE	*ifStream;
	char	line[BUFF2K];	
	
	TRACE_LOG("GetStringFromCmd: Enter");

	ifStream = popen(command.c_str(), "r");
	if ( ifStream != NULL ) {
		if (fgets(line, BUFF2K, ifStream) != NULL) {
			if( strlen( line ) > 0 ) {
				line[strlen(line) - 1] = '\0';
			}
			stringtofill.assign(line);
			trimSpaces(stringtofill);
		}
		pclose(ifStream);			// Close the stream	

	} else {
		TRACE_LOG("GetStringFromCmd: popen failed for command - %s", line);
	}

	TRACE_LOG("GetStringFromCmd: Exit");
}

/*
//
// Helper routine extracts a value from a cmd grep, awk, etc.
// update blocktofill with the extracted value. -- UP to 2K bytes
// inputs: string command, string &blocktofill
//
*/
void GetBlockOutput(string command, string &blocktofill)
{
	FILE	*ifStream;
	char	line[BUFF2K];
	int  	bytesRead;
	
	TRACE_LOG("GetBlockOutput: Enter");

TRACE_LOG("command: %s", command.c_str());
	ifStream = popen(command.c_str(), "r");
	if ( ifStream != NULL ) {
		memset(line,0,BUFF2K);
		if( (bytesRead = fread( line, 1, BUFF2K, ifStream )) > 0 ) {
			if( strlen( line ) > 0 ) {
				line[strlen(line) - 1] = '\0';
			}
			blocktofill.assign(line);
		}
		pclose(ifStream);			// Close the stream	

	} else {
		TRACE_LOG("GetBlockOutput: popen failed for command - %s", line);
	}

	TRACE_LOG("GetStringFromCmd: Exit");
}
/*
//
// Helper routine extracts a tag value a full string
// update stringtofill with the extracted value.
// inputs: string command, char tag to search for, string &stringtofill
//
*/
void GetTagValue(string fullLineString, char *sTag, string &stringtofill)
{
	size_t		len;
	size_t		location;
	size_t 		crpos;
	
	TRACE_LOG( "GetTagValue: Enter!" );
	//
	// Search for Class, Manufacturer(Vendor), Model, and Revision fields
	// Assumption here is that none of these fields will exceed BUFF80 length!
	//
	if( (location = fullLineString.find(sTag)) != string::npos ) {
		crpos = fullLineString.find( NEWLINE_CHARACTER, location+strlen(sTag)+1 );
		len = crpos - (location+strlen(sTag)+1);
		stringtofill = fullLineString.substr(location+strlen(sTag)+1, len);
	
	} else {	
		stringtofill.assign("unknown");
		TRACE_LOG( "\tDid not find Class string!" );
	}

	TRACE_LOG( "GetTagValue: Exit!" );
}
