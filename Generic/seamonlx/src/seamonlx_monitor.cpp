// monitor threads
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <ctype.h>
#include <pthread.h>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include <sys/types.h>
#include <dirent.h>

#include <errno.h>


#include "common.h"
#include "seamonlx.h"

#include "NetworkAdapters.h"
#include "Disks.h"
#include "StorageAdapters.h"
#include "Server.h"
#include "ServerEnvXmlrpc.h"

#include "InfinibandAdapters.h"


using namespace std;
using namespace seamonlx;


extern RPM_NAME			RPMNameArray[MAX_RPM_ALLOWED];
extern SERVICE_NAME		ServiceNameArray[MAX_SVC_ALLOWED];
extern CONFIG_STRUCT	SysConfigData;
extern BMC_STRUCT		BmcStructArray[2];						// 0 = LOCAL, 1 = REMOTE

extern THREAD_INFO_ELEM	ThreadInfo[MAX_START_THREADS];



#define SERVERENV_ELIST_COUNT 4   // how many kind to monitor (fans, Pwr supp, Temp, Volts)

#define FANLIST					0
#define PWRLIST					1
#define TEMPLIST				2
#define VOLTLIST				3

//
// Global needed to get at ServerEnv
//
extern ServerEnvXmlrpc *GpObjServerEnv;

extern int	tailAlertLog(ALERT_STRUCT *alert, char *filepath);
extern int	getAlert(char *seqnum, ALERT_STRUCT *alert, FILE **fp, int *fileisopen, int doclose);
extern int	getAlertRange(ALERT_RANGE *alertrange);
extern int	writeAlert(void);

//
// setup the alert thread
//
void *AlertThreadStart(void *pParam)
{	
	long threadnum;
	int rtn = FAILURE;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "AlertThreadStart, Enter");
	threadnum = (long)((long *)pParam);	
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	while (!seamonlx_shutdown) {
		rtn = writeAlert();
		sleep(1);
	}
			
	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "AlertThreadStart, Exit");
}


//
// thread for monitoring services
//
void *MonitorServices(void *pParam)
{
	int i = 0;
	char msg[BUFF256];
	int alertid;
	long threadnum;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorkServices, Enter");
	threadnum = (long)((long *)pParam);
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;

	while (!seamonlx_shutdown) {
		i = 0;
		while (ServiceNameArray[i].name[0] != '\0')  {
			if ((IsServiceStarted(ServiceNameArray[i].name) != SUCCESS) &&
				(IsSvcAtRunLevel(ServiceNameArray[i].name) == SUCCESS)) {
					sprintf(msg, "%s service needs to be configured for this runlevel %c", ServiceNameArray[i].name, RUNLEVEL);
					alertid = Gsv_AI_SVC_NOT_CURRENT_RUN_LEVEL;
					processAlertMsg("seamonlx", "ERR", CN_SeaMon_LX, msg, alertid);
				} 
			i++;
		}
		sleep(Gsv_MONITOR_SERVICES_SLEEP_INTERVAL);      // 15 mins
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorkServices, Exit");
}


//
// Thread to check for RPMs and versions correct
// Alert if incorrect
//
void *MonitorPackages(void *pParam)
{
	int i = 0;
	char msg[BUFF256];
	int alertid;
	int rtn = FAILURE;
	long threadnum;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorPackages, Enter");
	threadnum = (long)((long *)pParam);	
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	while (!seamonlx_shutdown) {
		i = 0;
		while (RPMNameArray[i].name[0] != '\0')  {
			rtn = IsPackageInstalled(RPMNameArray[i].name);
			if (rtn == FAILURE) {
				// process alert call
				sprintf(msg, "%s Package not installed, check version closely", RPMNameArray[i].name);
				alertid = Gsv_AI_RPM_NOT_INSTALLED;
				processAlertMsg("seamonlx", "ERR", CN_SeaMon_LX, msg, alertid);
			}
			i++;
		}

		sleep(Gsv_MONITOR_PACKAGES_SLEEP_INTERVAL); // 24 hrs     
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorPackages, Exit");
}

//
// thread for monitoring drives
//

void *MonitorDRIVE(void *pParam)
{
	char msg[BUFF512];
	int alertid;
	string state;
	string newstate;
	string scsiaddr;
	string sgname;
	long threadnum;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorDRIVE, Enter");
	threadnum = (long)((long *)pParam);	
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	Disks *mydisk = Disks::instance();

	if (mydisk != NULL) {
		vector<Disks::DiskDrive *> disklist = mydisk->getDisks();
		vector<Disks::DiskDrive *>::iterator dit;

		while (!seamonlx_shutdown) {
			for (dit = disklist.begin(); dit != disklist.end(); dit ++) {
				pthread_mutex_lock(&EncMutex);
				(*dit)->update();
				pthread_mutex_unlock(&EncMutex);

				state = (*dit)->getState();
				if (strcasestr(state.c_str(),"online") != NULL) {
					continue;
				} else {
					//
					// if a bad disk then update
					//
					pthread_mutex_lock(&EncMutex);
					(*dit)->update();
					pthread_mutex_unlock(&EncMutex);

					newstate = (*dit)->getState();
					if (strcasestr(newstate.c_str(),"online") != NULL) {
						continue;

					} else if (strcasecmp(state.c_str(), newstate.c_str()) == 0)  {
						continue;

					} else {
						//
						// generate the Alert
						//
						scsiaddr = (*dit)->getScsiaddr();
						sgname = (*dit)->getSgname();
						if (strcasestr(newstate.c_str(),"offline") != NULL) {
							alertid = Gsv_AI_DISK_OFFLINE;
						} else if (strcasestr(newstate.c_str(), "SMART_Error") != NULL) {
							alertid = Gsv_AI_DISK_SMART_ERROR;
						} else if (strcasestr(newstate.c_str(), "Unknown") != NULL) {
							alertid = Gsv_AI_DISK_UNKNOWN_ERROR;
						} 
						sprintf(msg,"sgname = %s, state = %s, scsi addr = %s", 
							sgname.c_str(),
							newstate.c_str(), scsiaddr.c_str());

						pthread_mutex_lock(&EncMutex);
						(*dit)->update();
						pthread_mutex_unlock(&EncMutex);
						
						processAlertMsg("seamonlx", "ERR", CN_SeaMon_LX, msg, alertid);
					}
				}
			}
			sleep(Gsv_MONITOR_DRIVE_SLEEP_INTERVAL);      // 5 mins
		}
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "Cannot Monitor drive status, exiting thread");
		die(msg, 0, Gsv_AI_THREAD_NOT_RUNNING);
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorDRIVE, Enter");
}
//
// Helper thread to check health of all Phys  for given enclosure
//
// Input Enclosure iterator
static void CheckPhys(vector<EnclProcessors::Enclosure *>::iterator leit)
{
	char msg[BUFF512];
	unsigned int phyi;
	vector<StorageAdapters::Adapter::Phy> phys;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckPhys, Enter");
	//
	// get paraent returns storage adapter, then get Phy linkstatus
	// check if down
	//
	phys = (*leit)->getParent()->getPhys();		
	for (phyi = 0; phyi < phys.size(); phyi ++)	{
		if (strcasecmp(phys[phyi].linkstatus.c_str(), "down") == 0) {
			// gen alert
			sprintf(msg,"PhyNum = %s, linkstatus = down, enclosure ID = %s", 
			phys[phyi].phynum.c_str(), (*leit)->getId().c_str());

			pthread_mutex_lock(&EncMutex);
			(*leit)->updatePhys();
			pthread_mutex_unlock(&EncMutex);

			processAlertMsg("seamonlx", "ERR", CN_Storage_Configuration, msg, Gsv_AI_ENC_PHY_LINK_STATUS);
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckPhys, Exit");
}
//
// Helper thread to check health of all fans supplies for given enclosure
//
// Input Enclosure iterator
static void CheckFans(vector<EnclProcessors::Enclosure *>::iterator leit)
{
	char msg[BUFF512];
	vector<EnclProcessors::Enclosure::Envelemt> fans;
	unsigned int fancount;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckFans, Enter");
	fans = (*leit)->getFans();
	for (fancount = 0; fancount < fans.size(); fancount ++)	{
		if (strcasestr(fans[fancount].status.c_str(), "CRITICAL") != NULL) {
		// gen critical alert
			sprintf(msg,"%s, status = Critical, RPMS = %s, enclosure ID = %s", 
				fans[fancount].desc.c_str(), 
				fans[fancount].reading.c_str(), (*leit)->getId().c_str());
			
			pthread_mutex_lock(&EncMutex);
			(*leit)->updateFans();
			pthread_mutex_unlock(&EncMutex);

			processAlertMsg("seamonlx", "ERR", CN_Enclosure_Environmentals, msg, Gsv_AI_ENC_FANS_STATUS);
		}
		
		if (strcasestr(fans[fancount].status.c_str(), "WARNING") != NULL) {
			// gen warning alert
			sprintf(msg,"%s, status = Warning, RPMS = %s, enclosure ID = %s", 
				fans[fancount].desc.c_str(), 
				fans[fancount].reading.c_str(), (*leit)->getId().c_str());

			pthread_mutex_lock(&EncMutex);
			(*leit)->updateFans();
			pthread_mutex_unlock(&EncMutex);

			processAlertMsg("seamonlx", "WARN", CN_Enclosure_Environmentals, msg, Gsv_AI_ENC_FANS_STATUS);
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckFans, Exit");
}

//
// Helper thread to check health of all pwr supplies for given enclosure
//
// Input Enclosure iterator
static void CheckPwrs(vector<EnclProcessors::Enclosure *>::iterator leit)
{
	char msg[BUFF512];
	vector<EnclProcessors::Enclosure::Envelemt> powers; 
	unsigned int pwrcount;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckPwrs, Enter");
	powers = (*leit)->getPowers();
	for (pwrcount = 0; pwrcount < powers.size(); pwrcount ++)	{
		if (strcasestr(powers[pwrcount].status.c_str(), "CRITICAL") != NULL) {
			// gen critical alert
			sprintf(msg,"%s status = Critical, cur volt = %s, enclosure ID = %s", 
				powers[pwrcount].desc.c_str(), 
				powers[pwrcount].reading.c_str(), (*leit)->getId().c_str());
			
			pthread_mutex_lock(&EncMutex);
			(*leit)->updatePowers();
			pthread_mutex_unlock(&EncMutex);

			processAlertMsg("seamonlx", "ERR", CN_Enclosure_Environmentals, msg, Gsv_AI_ENC_PWR_STATUS);
		}

		if (strcasestr(powers[pwrcount].status.c_str(), "WARNING") != NULL) {
			// gen warning alert
			sprintf(msg,"%s status = Warning, cur volt = %s, enclosure ID = %s", 
				powers[pwrcount].desc.c_str(), 
				powers[pwrcount].reading.c_str(), (*leit)->getId().c_str());

			pthread_mutex_lock(&EncMutex);
			(*leit)->updatePowers();
			pthread_mutex_unlock(&EncMutex);

			processAlertMsg("seamonlx", "WARN", CN_Enclosure_Environmentals, msg, Gsv_AI_ENC_PWR_STATUS);
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckPwrs, Exit");
}
//
// Helper thread to check health of all temp sensors for given enclosure
//
// Input Enclosure iterator
static void CheckTemps(vector<EnclProcessors::Enclosure *>::iterator leit)
{
	char msg[BUFF512];
	vector<EnclProcessors::Enclosure::Envelemt> temps;
	unsigned int tempcount;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckTemps, Enter");
	temps = (*leit)->getTemps();
	for (tempcount = 0; tempcount < temps.size(); tempcount ++)	{
		if (strcasestr(temps[tempcount].status.c_str(), "Status: OK") != NULL) {
			continue;
		} else {
			if (strcasestr(temps[tempcount].status.c_str(), "Status: CRITICAL") != NULL) {
				// gen critical alert
				sprintf(msg,"%s status = Critical, Curr Temp = %s, enclosure ID = %s", 
					temps[tempcount].desc.c_str(), 
					temps[tempcount].reading.c_str(),(*leit)->getId().c_str());

				pthread_mutex_lock(&EncMutex);
				(*leit)->updateTemps();
				pthread_mutex_unlock(&EncMutex);

				processAlertMsg("seamonlx", "ERR", CN_Enclosure_Environmentals, msg, Gsv_AI_ENC_TEMP_STATUS);
			} 

			if (strcasestr(temps[tempcount].status.c_str(), "Status: WARNING") != NULL) {
				// gen warning alert
				sprintf(msg,"%s status = Warning, Curr Temp = %s, enclosure ID = %s", 
					temps[tempcount].desc.c_str(), 
					temps[tempcount].reading.c_str(),(*leit)->getId().c_str());

				pthread_mutex_lock(&EncMutex);
				(*leit)->updateTemps();
				pthread_mutex_unlock(&EncMutex);

				processAlertMsg("seamonlx", "WARN", CN_Enclosure_Environmentals, msg, Gsv_AI_ENC_TEMP_STATUS);
			}
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckTemps, Exit");
}

//
// Helper thread to check health of all disk for given enclosure
//
// Input Enclosure iterator
static void CheckDisks(vector<EnclProcessors::Enclosure *>::iterator leit)
{
	char msg[BUFF512];
	unsigned int diskcount;
	
	vector<EnclProcessors::Enclosure::Disk> disks;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckDisks, Enter");
	disks = (*leit)->getDisks();	
	for (diskcount = 0; diskcount < disks.size(); diskcount ++)	{
		if (strcasestr(disks[diskcount].status.c_str(), "CRITICAL") != NULL) {
			// gen critical alert
			sprintf(msg,"sgname = %s, status = Critical, BayNum = %s, enclosure ID = %s", 
				disks[diskcount].sgname.c_str(), 
				disks[diskcount].baynumber.c_str(), (*leit)->getId().c_str());

			pthread_mutex_lock(&EncMutex);
			(*leit)->updateDisklist();
			pthread_mutex_unlock(&EncMutex);

			processAlertMsg("seamonlx", "ERR", CN_Enclosure_Environmentals, msg, Gsv_AI_ENC_DISK_ELEM_SES_STATUS);
		}

		if (strcasestr(disks[diskcount].status.c_str(), "WARNING") != NULL) {
			// gen warning alert
			sprintf(msg,"sgname = %s, status = Warning, BayNum = %s, enclosure ID = %s", 
				disks[diskcount].sgname.c_str(), 
				disks[diskcount].baynumber.c_str(), (*leit)->getId().c_str());

			pthread_mutex_lock(&EncMutex);
			(*leit)->updateDisklist();
			pthread_mutex_unlock(&EncMutex);

			processAlertMsg("seamonlx", "WARN", CN_Enclosure_Environmentals, msg, Gsv_AI_ENC_DISK_ELEM_SES_STATUS);
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckDisks, Exit");
}

//
// thread for monitoring enclosure procs
//

void *MonitorENCLOSUREPROC(void *pParam)
{
	char msg[BUFF512];
	string estatus;
	string newestatus;
	string encId;
	long threadnum;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorENCLOSUREPROC, Enter");
	threadnum = (long)((long *)pParam);
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	EnclProcessors *myencl = EnclProcessors::instance();
	if (myencl != NULL) {
		vector<EnclProcessors::Enclosure *> enclist = myencl->getEncls();
		vector<EnclProcessors::Enclosure *>::iterator eit;

		while (!seamonlx_shutdown) {

			pthread_mutex_lock(&EncMutex);
			myencl->update();
			pthread_mutex_unlock(&EncMutex);
			
			for (eit = enclist.begin(); eit != enclist.end(); eit ++) {
				//
				//Check Indiv Ses Element status 
				//
				//
				// Only for UML do we have environmentals for enclosure
				//
				if (strcasestr(Gsv_ProductType, "UML") != NULL) { 
					CheckPhys(eit);
					CheckFans(eit);
					CheckPwrs(eit);
					CheckTemps(eit);
				}
				CheckDisks(eit);
			}
			sleep(Gsv_MONITOR_ENCLOSURE_SLEEP_INTERVAL);      // 5 mins
		}
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "Cannot Monitor Enclosure Procs status exiting thread");
		die(msg, 0, Gsv_AI_THREAD_NOT_RUNNING);
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorENCLOSUREPROC, Exit");
}


//
// Helper thread to check health of all Phys  for given enclosure
//
// Input Enclosure iterator
static void CheckSAPhysLinkStatus(vector<StorageAdapters::Adapter *>::iterator lit)
{
	char msg[BUFF512];
	unsigned int phyi;
	vector<StorageAdapters::Adapter::Phy> phys;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckPhys, Enter");
	//
	// get paraent returns storage adapter, then get Phy linkstatus
	// check if down
	//
	phys = (*lit)->getPhys();		
	for (phyi = 0; phyi < phys.size(); phyi ++)	{
		if (strcasecmp(phys[phyi].linkstatus.c_str(), "down") == 0) {
			// gen alert
			sprintf(msg,"PhyNum = %s, linkstatus = down, Device ID = %s", 
			phys[phyi].phynum.c_str(), (*lit)->getDeviceid().c_str());

			pthread_mutex_lock( &StorageAdaptersMutex );	
			(*lit)->update();
			pthread_mutex_unlock( &StorageAdaptersMutex );	
			
			processAlertMsg("seamonlx", "ERR", CN_Storage_Configuration, msg, Gsv_AI_SA_PHY_LINK_STATUS);
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckPhys, Exit");
}

//
// thread for monitoring enclosure procs
//

void *MonitorSTORAGEADAPTER(void *pParam)
{
	char msg[BUFF512];
	string estatus;
	string newestatus;
	string encId;
	long threadnum;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorSTORAGEADAPTER, Enter");
	threadnum = (long)((long *)pParam);
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	StorageAdapters *mysa = StorageAdapters::instance();
	if (mysa != NULL) {
		vector<StorageAdapters::Adapter *> salist = mysa->getAdapters();
		vector<StorageAdapters::Adapter *>::iterator sait;

		pthread_mutex_lock( &StorageAdaptersMutex );	
		mysa->update();
		pthread_mutex_unlock( &StorageAdaptersMutex );	
		
		while (!seamonlx_shutdown) {

			for (sait = salist.begin(); sait != salist.end(); sait ++) {
				estatus = (*sait)->getStatus();
				if (strcasecmp(estatus.c_str(), "CRITICAL") == 0) {
					// gen critical alert
					sprintf(msg,"PCI Addr = %s, status = Critical", 
						(*sait)->getPciaddr().c_str());

					pthread_mutex_lock( &StorageAdaptersMutex );	
					mysa->update();
					pthread_mutex_unlock( &StorageAdaptersMutex );

					processAlertMsg("seamonlx", "ERR", CN_Storage_Configuration, msg, Gsv_AI_SA_STATUS);
				}
				
				//
				//Check Indiv Ses Element status 
				//
				CheckSAPhysLinkStatus(sait);
				
			}
			sleep(Gsv_MONITOR_STORAGEADAPTER_SLEEP_INTERVAL);      // 5 mins
		}
	} else {
		sprintf(msg, "%s: %s", __FUNCTION__, "Cannot Monitor STORAGE ADAPTER status exiting thread");
		die(msg, 0, Gsv_AI_THREAD_NOT_RUNNING);
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorSTORAGEADAPTER, Exit");
}

//
// Local Helper func uses Global ptr GpObjServerEnv implicitly
// GpObjServerEnv  -- was set in seamonlxXMLRPC.cpp on initialization
// 
static void CheckServerEnvStatus(void)
{
	char msg[BUFF512];
	unsigned int elistcount;
	unsigned int idval;
	char textstr[256];
	int numelist = 0;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckServerEnvStatus, Enter");

	ServerEnv::EList locelist;

	ServerEnv::EList FanList  = GpObjServerEnv->getFans();
	ServerEnv::EList PWRList  =	GpObjServerEnv->getPowers();
	ServerEnv::EList TempList = GpObjServerEnv->getTempsensors();
	ServerEnv::EList VoltList = GpObjServerEnv->getVoltages();

	for (numelist = 0; numelist < SERVERENV_ELIST_COUNT; numelist++) {
		switch (numelist)
		{
			case FANLIST:
				locelist = FanList;
				idval = Gsv_AI_SVRENV_FAN_STATUS;
				sprintf(textstr, "Fan Num");
				break;

			case PWRLIST:
				locelist = PWRList;
				idval = Gsv_AI_SVRENV_PWR_STATUS;
				sprintf(textstr, "PWR Supp");
				break;

			case TEMPLIST:
				locelist = TempList;
				idval = Gsv_AI_SVRENV_TEMP_STATUS;
				sprintf(textstr, "Temp Sensor");
				break;

			case VOLTLIST:
				locelist = VoltList;
				sprintf(textstr, "Voltage Sensor");
				idval = Gsv_AI_SVRENV_VOLT_STATUS;
				break;

			default:
				break;
		}

		for (elistcount = 0; elistcount < locelist.size(); elistcount ++) {
			if (strcasecmp(locelist[elistcount].status.c_str(), "Critical") == 0) {
				sprintf(msg,"%s %d status = CRITICAL, Device ID = %s, val = %s", textstr, elistcount,
					locelist[elistcount].seacid.c_str(), locelist[elistcount].value.c_str());
				
				
				processAlertMsg("seamonlx", "ERR", CN_Server_Environmentals, msg, idval);
			}

			if (strcasecmp(locelist[elistcount].status.c_str(), "Warning") == 0) {
				sprintf(msg,"%s %d status = WARNING, Device ID = %s, val = %s",  textstr, elistcount,
					locelist[elistcount].seacid.c_str(), locelist[elistcount].value.c_str());
				
				
				processAlertMsg("seamonlx", "WARN", CN_Server_Environmentals, msg, idval);
			}
		}
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "CheckServerEnvStatus, Exit");
}
//
// thread for monitoring Server Env (Fans, Pwr supp, Temp sensors, Voltages)
//

void *MonitorSERVERENV(void *pParam)
{
	long threadnum;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorSERVERENV, Enter");
	threadnum = (long)((long *)pParam);
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;

	while (!seamonlx_shutdown) {
		pthread_mutex_lock(&GlobalObjMonThreadServerEnv);
		if (GpObjServerEnv != NULL) {
			GpObjServerEnv->update();
			CheckServerEnvStatus();
		}
		pthread_mutex_unlock(&GlobalObjMonThreadServerEnv);
		sleep(Gsv_MONITOR_SERVERENV_SLEEP_INTERVAL);      // 20 mins
	}
			
	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorSERVERENV, Exit");
}



//
// UpdateComponentList: Adds a AlertComponent to the particuular COMPONENT List
//
// Inputs:   locmutex - Mutex to protect adds to list
//			loclist  - ptr to specific global list<AlertComponent>
//			localert - ptr to contents for this alert which was extracted from Alert.log file
//
// Returns: SUCCESS or FAILURE = got exception on push_back() call
//
//

static int UpdateComponentList(pthread_mutex_t locmutex, list<AlertComponent> *loclist, ALERT_STRUCT *localert)
{
	AlertComponent locstruct;
	int rtn = FAILURE;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "UpdateComponentList, Enter");
	//
	// copy and initialize 
	//
	locstruct.ComponentName = localert->componentname;
	locstruct.TimeStamp		= localert->timestamp;
	locstruct.Severity		= localert->severity;
	locstruct.descr			= localert->description;
	locstruct.recc			= localert->recommendation;

	//
	// List to add to, may throw exception
	//
	pthread_mutex_lock(&locmutex);
	try {
		loclist->push_back(locstruct);
		loclist->sort();
		if ((int)(loclist->size()) > Gsv_MAXCOMPONENTLISTSIZE) {
			loclist->resize(Gsv_MAXCOMPONENTLISTSIZE);
		}
		rtn = SUCCESS;
	} catch( ... ) {
			rtn = FAILURE;
			char errmsg[BUFF512];
			sprintf(errmsg, "%s Failed on pushback call for %s", __FUNCTION__, locstruct.ComponentName.c_str());
			TRACE_LOG(errmsg);
	}

	pthread_mutex_unlock(&CN_Management_Port_M);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "UpdateComponentList, Exit");
	return rtn;
}
//
// AddAlertToComponentList: use Global List to call UpdateComponentList()
//
// Input: CurrAlert - ptr to Alert Struct filled in when getting Alert entry from Alert.log
//
// Returns: Success or Failure
//
static int AddAlertToComponentList(ALERT_STRUCT *CurrAlert)
{
	int rtn = FAILURE;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "AddAlertToComponentList, Enter");
	if (strcasecmp(CurrAlert->componentname, CN_Server_Environmentals) == 0) {
		rtn = UpdateComponentList(CN_Server_Environmentals_M, 
								&Server_Environmentals_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Management_Port) == 0) {
		rtn = UpdateComponentList(CN_Management_Port_M, 
								&Management_Port_complist, 
								CurrAlert);
	
	} else if (strcasecmp(CurrAlert->componentname, CN_Target_Ports) == 0) {
		rtn = UpdateComponentList(CN_Target_Ports_M, 
								&Target_Ports_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_InfiniBand) == 0) {
		rtn = UpdateComponentList(CN_InfiniBand_M, 
								&InfiniBand_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_OpenIB_Subnet_Mgmt) == 0) {
		rtn = UpdateComponentList(CN_OpenIB_Subnet_Mgmt_M, 
								&OpenIB_Subnet_Mgmt_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_SeaMon_LX) == 0) {
		rtn = UpdateComponentList(CN_SeaMon_LX_M, 
								&SeaMon_LX_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_BMC_IPMI) == 0) {
		rtn = UpdateComponentList(CN_BMC_IPMI_M, 
								&BMC_IPMI_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Enclosure_Environmentals) == 0) {
		rtn = UpdateComponentList(CN_Enclosure_Environmentals_M, 
								&Enclosure_Environmentals_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Storage_Interconnect) == 0) {
		rtn = UpdateComponentList(CN_Storage_Interconnect_M, 
								&Storage_Interconnect_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Storage_Configuration) == 0) {
		rtn = UpdateComponentList(CN_Storage_Configuration_M, 
								&Storage_Configuration_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_SHAS_State) == 0) {
		rtn = UpdateComponentList(CN_SHAS_State_M, 
								&SHAS_State_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_CIFS) == 0) {
		rtn = UpdateComponentList(CN_CIFS_M, 
								&CIFS_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_FTP) == 0) {
		rtn = UpdateComponentList(CN_FTP_M, 
								&FTP_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Software_Configuration) == 0) {
		rtn = UpdateComponentList(CN_Software_Configuration_M, 
								&Software_Configuration_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_System_Services) == 0) {
		rtn = UpdateComponentList(CN_System_Services_M, 
								&System_Services_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Kernel) == 0) {
		rtn = UpdateComponentList(CN_Kernel_M, 
								&Kernel_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Hyper_FS) == 0) {
		rtn = UpdateComponentList(CN_Hyper_FS_M, 
								&Hyper_FS_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_IPStor) == 0) {
		rtn = UpdateComponentList(CN_IPStor_M, 
								&IPStor_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_StreamSmith) == 0) {
		rtn = UpdateComponentList(CN_StreamSmith_M, 
								&StreamSmith_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_VFlow) == 0) {
		rtn = UpdateComponentList(CN_VFlow_M, 
								&VFlow_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_SeaFS) == 0) {
		rtn = UpdateComponentList(CN_SeaFS_M, 
								&SeaFS_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Sparse_Cache) == 0) {
		rtn = UpdateComponentList(CN_Sparse_Cache_M, 
								&Sparse_Cache_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Distributed_Cache) == 0) {
		rtn = UpdateComponentList(CN_Distributed_Cache_M, 
								&Distributed_Cache_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_Sentry_Service) == 0) {
		rtn = UpdateComponentList(CN_Sentry_Service_M, 
								&Sentry_Service_complist, 
								CurrAlert);

	} else if (strcasecmp(CurrAlert->componentname, CN_C2_Server) == 0) {
		rtn = UpdateComponentList(CN_C2_Server_M, 
								&C2_Server_complist, 
								CurrAlert);
	} else {
		// not yet defined
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "AddAlertToComponentList, Failed on call to UpdateComponentList()");
		rtn = FAILURE;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "AddAlertToComponentList, Exit");
	return rtn;
}



//
// ReadFromAlertLog: utilize getalert() call to access item from Alert.log
//
// Input CurrSeqNum -- ptr to locate entry in ALert.log
//
// Output: updated CurrSeqNum 
// if we reach end of log set AlertLogParseComplete 
//
// returns: SUCCESS or FAILURE
//
static int ReadFromAlertLog(long *CurrSeqNum)
{
	char						CurrSeqNumStr[BUFF32];
	int							i;
	ALERT_STRUCT				Last_alert;	
	ALERT_STRUCT				Curr_alert;	
	FILE						*fp;
	int							isopen = 0;
	int							rtn = FAILURE;
	char						errmsg[BUFF512];


	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "ReadFromAlertLog, Enter");
	if (tailAlertLog(&Last_alert, PRIMARYFILEPATH) == SUCCESS) {
		//
		// attempt to get 100 Alerts from Alert Log
		//
		for (i = 0; i < ALERT_LOG_REC_MAX; i++) {
			sprintf(CurrSeqNumStr, "%ld", (*CurrSeqNum));
			if (getAlert(CurrSeqNumStr, &Curr_alert, &fp, &isopen, 0) == SUCCESS) {
				pthread_mutex_lock(&ComponentListMutex);
				rtn = AddAlertToComponentList(&Curr_alert);
				pthread_mutex_unlock(&ComponentListMutex);
				if (Curr_alert.seqnum == Last_alert.seqnum) {
					//
					// we finished reading the Alert.log
					// be sure to point to next seqnum for adding next
					// to component list when we get a new alert via 
					// processAlertMsg() call in another thread
					//
					(*CurrSeqNum)++;
					AlertLogParseComplete = SUCCESS;
					break;
				} else {
					(*CurrSeqNum)++;
				}
			} else {
				//
				// can't use die some kind of error reading from alert.log
				//
				sprintf(errmsg, "%s: getAlert() failed", __FUNCTION__);
				TRACE_LOG(errmsg);
				rtn = FAILURE;
				break;
			}
		}
		//
		// close the file handle
		// after we have read 10 or reached last alert in file
		if (isopen)
			fclose(fp);
	}
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "ReadFromAlertLog, Exit");

	return rtn;
}

//
// BuildCompLists: Thread for building Component Lists utilizes getalertrange()
// and then call ReadFromAlertLog() until complete 
//
void *BuildCompLists(void *pParam)
{
	long threadnum;
	long  CurrSeqNum = 1;
	
	ALERT_RANGE  Loc_alertrange;
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "BuildCompLists, Enter");
	threadnum = (long)((long *)pParam);
	int rtn = FAILURE;
	struct stat filestatus;

	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	

	//
	// Start at top of Alert Log, initialize AlertLogParseComplete
	//
	while (rtn == FAILURE) {
		//
		// ensure the file exists
		//
		stat(PRIMARYFILEPATH, &filestatus );
		if (filestatus.st_size == 0) {
			sleep (5);
		} else {
			pthread_mutex_lock(&AlertMutex);
			rtn = getAlertRange(&Loc_alertrange);
			pthread_mutex_unlock(&AlertMutex);

			if (rtn == SUCCESS) {
				CurrSeqNum = Loc_alertrange.OldestSeqNum;
				break;
			}
			 
			sleep(5);  // wait 5 sec and try again
		}
	}

	//
	// ok now Reading from alert.log
	// keep reading until we've read the whole magilla
	// AlertLogParseComplete set to SUCCESS, also CurrSeqNum gets incremented
	// via ReadFromAlertLog() routine.
	//
	while (!seamonlx_shutdown) {

		pthread_mutex_lock(&AlertMutex);
		if (AlertLogParseComplete != SUCCESS) {
			ReadFromAlertLog(&CurrSeqNum);
		}
		pthread_mutex_unlock(&AlertMutex);

		sleep(2); // 2 secs
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "BuildCompLists, Exit");
	pthread_exit(NULL);
}

//
// Thread to Monitor Network Adapters and versions correct
//
//
void *MonitorNetworkAdapters(void *pParam)
{
	unsigned int vecadaptcount;
	unsigned int vecifcount;

	vector<NetworkAdapters::AdapterInfo> locadapts;
	vector<NetworkAdapters::InterfaceInfo> locifs;
	

	char msg[BUFF256];
	int alertid;
	long threadnum;
	string 			state = "unknown";
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorNetworkAdapters, Enter");
	threadnum = (long)((long *)pParam);	
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	
	while (!seamonlx_shutdown) {

		NetworkAdapters *myna = NetworkAdapters::instance();

		if (myna != NULL) {
			//
			// Get local instance, issue update
			//
			pthread_mutex_lock( &NetworkAdaptersMutex );	
			myna->update();
			pthread_mutex_unlock( &NetworkAdaptersMutex );	

			locadapts = myna->getVecAdapters();
			for (vecadaptcount = 0; vecadaptcount < myna->getVecAdapters().size(); vecadaptcount++)	{
				if ((locadapts[vecadaptcount].healthState.getHealthState()) != (ObjectHealthState::STATE_OK)) {
					sprintf(msg, "Network Adapter Error for index %d", vecadaptcount);
					alertid = Gsv_AI_NETWORK_ADAPTER_ERROR;

					pthread_mutex_lock( &NetworkAdaptersMutex );	
					myna->updateNetworkAdaptersList();
					pthread_mutex_unlock( &NetworkAdaptersMutex );	
					processAlertMsg("seamonlx", "ERR", CN_SeaMon_LX, msg, alertid);
				}
			}

			locifs = myna->getVecInterfaces();
			for (vecifcount = 0; vecifcount < myna->getVecInterfaces().size(); vecifcount++) {
				if ((locifs[vecifcount].healthState.getHealthState()) != (ObjectHealthState::STATE_OK)) {
					sprintf(msg, "Network Interface Error for index %d", vecifcount);
					alertid = Gsv_AI_NETWORK_INTERFACE_ERROR;
					
					pthread_mutex_lock( &NetworkAdaptersMutex );	
					myna->updateNetworkInterfacesList();
					pthread_mutex_unlock( &NetworkAdaptersMutex );	
					processAlertMsg("seamonlx", "ERR", CN_SeaMon_LX, msg, alertid);
				}
			}			
		}

		sleep(Gsv_MONITOR_NETWORK_ADAPTERS_SLEEP_INTERVAL); // 60 mins    
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorNetworkAdapters, Exit");
}

//
// Thread to Monitor Infinband Adapters and Interafces
//
//
void *MonitorInfinibandAdapters(void *pParam)
{
	unsigned int vecadaptcount;
	unsigned int vecifcount;

	vector<InfinibandAdapters::AdapterInfo> locadapts;
	vector<InfinibandAdapters::InterfaceInfo> locifs;
	

	char msg[BUFF256];
	int alertid;
	long threadnum;
	string 			state = "unknown";
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorInfinibandAdapters, Enter");
	threadnum = (long)((long *)pParam);	
	ThreadInfo[threadnum].threadstate = THREAD_STARTED;
	
	while (!seamonlx_shutdown) {

		InfinibandAdapters *myna = InfinibandAdapters::instance();

		if (myna != NULL) {
			//
			// Get local instance, issue update
			//
			pthread_mutex_lock( &InfinibandAdaptersMutex );	
			myna->update();
			pthread_mutex_unlock( &InfinibandAdaptersMutex );	

			locadapts = myna->getVecAdapters();
			for (vecadaptcount = 0; vecadaptcount < myna->getVecAdapters().size(); vecadaptcount++)	{
				if ((locadapts[vecadaptcount].healthState.getHealthState()) != (ObjectHealthState::STATE_OK)) {
					sprintf(msg, "Infiniband Adapter Error for index %d", vecadaptcount);
					alertid = Gsv_AI_INFINIBAND_ADAPTER_ERROR;

					pthread_mutex_lock( &InfinibandAdaptersMutex );	
					myna->updateInfinibandAdaptersList();
					pthread_mutex_unlock( &InfinibandAdaptersMutex );	
					processAlertMsg("seamonlx", "ERR", CN_SeaMon_LX, msg, alertid);
				}
			}

			locifs = myna->getVecInterfaces();
			for (vecifcount = 0; vecifcount < myna->getVecInterfaces().size(); vecifcount++) {
				if ((locifs[vecifcount].healthState.getHealthState()) != (ObjectHealthState::STATE_OK)) {
					sprintf(msg, "Infiniband Interface Error for index %d", vecifcount);
					alertid = Gsv_AI_INFINIBAND_INTERFACE_ERROR;
					
					pthread_mutex_lock( &InfinibandAdaptersMutex );	
					myna->updateInfinibandInterfacesList();
					pthread_mutex_unlock( &InfinibandAdaptersMutex );	
					processAlertMsg("seamonlx", "ERR", CN_SeaMon_LX, msg, alertid);
				}
			}			
		}

		sleep(Gsv_MONITOR_INFINIBAND_ADAPTERS_SLEEP_INTERVAL); // 60 mins    
	}

	ThreadInfo[threadnum].threadstate = THREAD_STOPPED;
	pthread_exit(NULL);

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "MonitorInfiniBandAdapters, Exit");
}

