#ifndef __ZQSNMPMANPKG_H__
#define __ZQSNMPMANPKG_H__

#include "ZQSnmp.h"

#ifdef __cplusplus
extern "C" {
#endif

bool SNMPOpenSession(const char *szServiceName, const char *szProductName, UINT processOid, ZQSNMP_CALLBACK fCallback = NULL, const char *szLogFileName = NULL, DWORD dwLoggingMask = 0, int nLogFileSize = 0xA00000 /* 10mb */, DWORD nProcessInstanceId = 0);
bool SNMPManageVariable(const char *szVarName, void *address, DWORD type, BOOL bReadOnly, UINT instId = 0);
bool SNMPCloseSession();

#ifdef __cplusplus
} // end extern "C"
#endif

#endif // _MANPKG_H_
