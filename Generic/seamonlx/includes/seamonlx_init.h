///////////////////////////////////////////////////////////////////////////////
//
//  Seamonlx.h
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SEAMONLX_INIT_H // {
#define SEAMONLX_INIT_H

extern void		die(char *s, int killit, int alertid);
extern void		signal_handler(int signo);
extern void		shutdownwriteAlert(void);
extern void		shutdownAbyssServer(void);

extern void checkCmdLine(int argc, char *argv[]);
extern void ChecklspciDiffs(void);
extern int InitSeamonlx(void);
extern void startthreads(void);

//
// global structs / arrays to initialize
//
extern RPM_NAME			RPMNameArray[MAX_RPM_ALLOWED];
extern SERVICE_NAME		ServiceNameArray[MAX_SVC_ALLOWED];
extern CONFIG_STRUCT	SysConfigData;
extern BMC_STRUCT		BmcStructArray[2];						// 0 = LOCAL, 1 = REMOTE

//
// globals initialization flag values
//
extern int seamonlx_shutdown;
extern int DEBUG;
extern int TRACING;
extern int NOXMLRPC;

extern int ALLOBJECTSSETUP;
extern int rchaIsLoaded;
extern int bccfgIsLoaded;
extern int StartPackageChecks;
extern int StartServiceChecks;


#endif // }
