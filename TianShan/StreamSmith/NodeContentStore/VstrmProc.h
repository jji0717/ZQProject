
#ifndef _VSTRM_PROC_
#define _VSTRM_PROC_

class VvxParser;

bool vsm_Initialize(HANDLE hVstrmClass );
void vsm_Uninitialize();

#define VSM_FILE_NOTFOUND			1
#define VSM_VSTRM_NOTREAD			2
#define VSM_OTHERERROR_NOTRETRY		3
// return value for this function 0, VSM_FILE_NOTFOUND, VSM_VSTRM_NOTREAD, VSM_OTHERERROR_NOTRETRY
HRESULT vsm_GetFileSize(const char* szFileName, LONGLONG& llFileSize);

bool vsm_ParseVvxFile(const char* szFileName, VvxParser& parser);

#endif