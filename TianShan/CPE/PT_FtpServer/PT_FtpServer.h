
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PT_FTPSERVER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PT_FTPSERVER_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef PT_FTPSERVER_EXPORTS
#define PT_FTPSERVER_API __declspec(dllexport)
#else
#define PT_FTPSERVER_API __declspec(dllimport)
#endif


class PushSessIMgr;

extern "C"
{
	/// read the configuration xml, and start ftp server
	PT_FTPSERVER_API bool PT_ModuleInit(const char* cfgPath, PushSessIMgr* pMgr);
	
	/// do the uninitialize
	PT_FTPSERVER_API void PT_ModuleUninit();
}
