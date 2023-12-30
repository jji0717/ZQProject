///////////////////////////////////////////////////////////////////////////////
//
//  Seamonlx.h
//
///////////////////////////////////////////////////////////////////////////////
#ifndef SEAMONLX_MONITOR_H // {
#define SEAMONLX_MONITOR_H

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
extern void *AlertThreadStart(void *pParam);
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
extern void *BuildCompLists(void *pParam);
extern void *MonitorNetworkAdapters(void *pParam);
extern void *MonitorInfinibandAdapters(void *pParam);

#endif // }
