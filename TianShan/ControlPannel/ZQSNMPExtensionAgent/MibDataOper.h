// MibDataOper.h: interface for the CMibDataOper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MIBDATAOPER_H__55EF3214_DE97_46D7_8EEB_EE53337EBE53__INCLUDED_)
#define AFX_MIBDATAOPER_H__55EF3214_DE97_46D7_8EEB_EE53337EBE53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning(disable:4786 )
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <malloc.h>
#include <tchar.h>
#include <time.h>
#include <string>
#include <snmp.h>
#include "mclog.h"
#include "log.h"
#include "FileLog.h"

#include "MibDescriptor.h"
#include "XMLFileOper.h"
#include "zqcfgpkg.h"
#include "ZQSNMPManPkg.h"

const DWORD DEFAULT_REFRESH_INTERVAL = 30;

using std::map;
using std::iterator;
using std::string;

// add by dony 20070122 for OID/Varname Mode's switch
#define  VARNAMEIN_MODE    1
#define  OIDSTDSTRING_MODE 1
#define  FILELOGMODE       1

// this is the our branch starting point (clled prefix)
#define OID_SIZEOF( Oid )      ( sizeof Oid / sizeof(UINT) )
#define MAL_LENTH       256
#define OID_LENTH       150


#define VAR_NAME_LENGTH 100

#define SIZEOFOID( Oid )      ( sizeof ( Oid ) / sizeof( UINT ) )
// Macro to determine the number of subidentifiers in an OID

// modify by dony 20070122 for OID/Varname Mode's switch
#ifdef OIDSTDSTRING_MODE
	typedef map<string,MibDescriptor*> MIBMAP;
	typedef MIBMAP::value_type       MIBMAPTYPE;
	typedef MIBMAP::iterator         MIBMAPITERTMP;
#else
	typedef  map<LPTSTR,MibDescriptor*> MIBMAP;
	typedef  MIBMAP::value_type      MIBMAPTYPE;
	typedef  MIBMAP::iterator        MIBMAPITERTMP;
#endif

/*
typedef  map<INT,LPTSTR>         FILEMAP;
typedef  FILEMAP::value_type     FILEMAPTYPE;
typedef  FILEMAP::iterator       FILEMAPITER;
*/

typedef struct _tagFILEDATAS
{
	TCHAR *szFileName;    // 
}FILEDATAS,*LFILEDATAS;


class CMibDataOper  
{
public:
	CMibDataOper();
	virtual ~CMibDataOper();
public:
	MibDescriptor* FindMibDescriptorByOID(AsnObjectIdentifier* pOID);
	MibDescriptor* FindPreviousMibDescriptor(AsnObjectIdentifier* pOID);
	MibDescriptor* FindMibDescriptorByKey(TCHAR *szKey);
	INT  GetFileCount(TCHAR *rootDir);
	bool InitializeMibDescriptors(void);
	bool InitializeMibData(TCHAR *szXMLFileName);
	bool GetRegistryValues();
	void SetFileNameMemory(TCHAR *rootDir,FILEDATAS ** pFileData);
	void GetFilesVector(TCHAR *rootDir);
	void CreateAndStoreNewMibObject(TCHAR *currentOID, 
								TCHAR *name,
								BYTE currentType,
								DWORD RefreshInterval,
								UINT access,
								TCHAR *value,
								bool IsTable,
								bool IsTableCol,
								INT  iVarType = 1
								);
	void TestResult();
public:
	TCHAR            m_strFile[MAL_LENTH];	
protected:
	CXMLFileOper  * m_pXMLFile;
//	FILEMAP         m_FileMap;
	MIBMAP			m_MibMap;
	TCHAR			m_szXMLFileDirectory[MAL_LENTH];
	TCHAR			m_szXMLFileName[MAL_LENTH];
	TCHAR			m_sCurrentName[MAL_LENTH];
	TCHAR			m_sCurrentOID[OID_LENTH];
	TCHAR			m_sCurrentServiceOID[OID_LENTH];
	DWORD			m_dwRefreshInterval;	
	INT             m_FileCount;
	INT             m_iCurCount;
	INT             m_iCompanyNumber;
	MibDescriptor	*m_pLastMib;
};

#endif // !defined(AFX_MIBDATAOPER_H__55EF3214_DE97_46D7_8EEB_EE53337EBE53__INCLUDED_)
