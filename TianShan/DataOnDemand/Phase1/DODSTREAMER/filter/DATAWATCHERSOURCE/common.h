
#ifndef CommonH
#define CommonH

//#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <direct.h>

#define  CATALOGPATHLEN	1024
#define  FILENAMELEN	1024
#define  READFILELEN	8192		// >1024 for SearchFile and DealFile

//Data Watcher Source filter status:
#define STATE_ERROR						0x0001		//Error
#define STATE_MONITORING				0x0002		//Monitoring
#define STATE_REPORTING					0x0004		//Reporting
#define STATE_STOP						0x0006		//Stop
#define STATE_READY						0x0008		//Ready

//Error Code -DataWatcherSource-
#define OPEN_FILE_FAILED				0x11101		//Open file failed
#define READ_FILE_FAILED				0x11102		//Read file failed
#define OUTOFMEMORY						0x11103		//Out of memory
#define ERROR_FILENAMEEXCED_RANGE		0x11104		//Catalog name larger than CATALOGPATHLEN
#define CREATE_EVENT_FAILED				0x11105		//Create event failed
#define CREATE_THREAD_FAILED			0x11106		//Create thread failed
#define MONITORCATALOG_THREAD_EXCEPTION 0x11107		//Monitor catalog thread occur exception
#define GETTIME_FAILED					0x11108		//SyncReadAligned interface function sample gettime failed
#define GETPOINTER_FAILED				0x11109		//SyncReadAligned interface function sample getpointer failed

//Error Code -DataWrapperFilter-
#define WRAPPERFILTER_OUTOFMEMORY			0x11201		//Out of memory
#define WRAPPERFILTER_CREATE_THREAD_FAILED	0x11202		//Create thread failed
#define SET_OBJKEYLENGTH_OVERFLOW			0x11204		//set the objectkeylength overflow
#define SET_TAGLENGTH_OVERFLOW				0x11208		//set the tag length overflow
#define SET_VERSIONNUMBER_OVERFLOW			0x11216		//set the version number overflow
#define SET_INDEXDESCLENGTH_OVERFLOW		0x11232		//set the indexdesclength overflow

// common with the FilterGraph control.
#define MAXOBJECTKEYLENGTH			16
#define MAXTAGLENGTH				32
#define DEFAULTTAGLENGTH			4
#define DEFAULTVERSION				1
#define DEFAULTOBJECTKEYLENGTH		4
#define DEFAULTINDEXDESCLENGTH		16
#define DEFAULTMAXSIZEOFSAMPLE		1024*1024
#define DEFAULTMINSIZEOFSAMPLE		64*1024

#define WRAP_MODE_COMMON				0x00
#define WRAP_MODE_WITHINDEXTABLE		0x01	// WRAP_MODE_MSG
#define WRAP_MODE_TABLEIDFROMFILENAME	0x02	// WRAP_MODE_MSG
#define WRAP_MODE_MULTISTREAM			0x04	// WRAP_MODE_NE

#define ENCRYPT_MODE_NONE				0x00
#define ENCRYPT_MODE_ILP_NORMAL			0x01
#define ENCRYPT_MODE_ILP_SIMPLE			0x02

using namespace std;

typedef std::vector< int >	IntVector;
typedef std::vector< int >::iterator	IntVectorItor;

typedef struct catalog 
{
	WORD 		wID;						//ID for catalog,set by sw itself	
	WORD		wCount;						//Files Count		    
	char 		szPath[CATALOGPATHLEN];		//Catalog path 
} CCatalog;

//extern std::vector<CCatalog * > Catalogs;

#define MAXOBJECTKEYLENGTH		16
#define MAXTAGLENGTH			32

struct subchannellist
{
public:
	WORD		wID;						//ID for catalog,set by sw itself
	char		szFileName[FILENAMELEN];	//Full path and file name
	//char		szPureFileName[FILENAMELEN];	// file name no path and no extension.
	DWORD		dwFileSize;					//File size
	DWORD		dwRepeat;					//File repeat times.
	FILETIME	ftCreate;					//time the file was created
	FILETIME	ftAccess;					//time the file was last accessed
	FILETIME	ftWrite;					//time the file was last written to.

	BYTE	byObjectKeyLength;				// the length of object key field, default is 4.
	char	sObjectKey[MAXOBJECTKEYLENGTH]; // the object key of the sub channel, used by DataWrapperFilter
	BYTE	byTableID;						// table id denoted, used by DataWrapperFilter
	WORD	wTableIDExtension;				// the first table id extension denoted, used by DataWrapperFilter
	WORD	wReserve;						// used for version number.
	BYTE	byControlFlag;					// used to judge the start and end of file stream.
	BYTE	byTableCount;					// how many chips need this file be splitted.

	long	lChannelPID;
	BYTE 	byTagLength;					// < 32, describe the length of tag field, default is 4.
	char	sChannelTag[MAXTAGLENGTH];		// the tag of channel, e.g. "pic\0", "hie\0", "auh\0"

	//long	lContentSize;
	//char	*pContent;

	subchannellist()
	{
		memset( szFileName, 0, FILENAMELEN );
		dwRepeat = 1;

		wReserve = 0;
		byControlFlag = 1;
		byTableCount = 0;

		byObjectKeyLength = DEFAULTOBJECTKEYLENGTH;
		memset( sObjectKey, 0, MAXOBJECTKEYLENGTH );
		byTagLength = DEFAULTTAGLENGTH;
		memset( sChannelTag, 0, MAXTAGLENGTH );

		//lContentSize = 0;
		//pContent = NULL;
	}
	//bool operator< ( const subchannellist& left, const subchannellist& right )
	//{
	//	return left.dwRepeat < right.dwRepeat;
	//}
} ;
typedef struct subchannellist DWS_SubChannelList;

typedef struct ObjectIndexDescriptor
{
	BYTE	byTag;
	BYTE	byLength;
	BYTE	byTableCount;
	BYTE	byIDLength;									// = DEFAULTOBJECTKEYLENGTH
	char	pchID[DEFAULTOBJECTKEYLENGTH];
	BYTE	byDescLength;								// = DEFAULTINDEXDESCLENGTH
	char	pchDesc[DEFAULTINDEXDESCLENGTH];
	ObjectIndexDescriptor()
	{
		byTag = 0x80;
		byLength = sizeof(ObjectIndexDescriptor) - 2;
		byTableCount = 0;
		byIDLength = DEFAULTOBJECTKEYLENGTH;
		byDescLength = DEFAULTINDEXDESCLENGTH;
	}
} ObjectDescriptor;

class FileSort{
public:  
	bool operator() ( DWS_SubChannelList left, DWS_SubChannelList right ) {
		return strcmp(left.szFileName, right.szFileName) < 0;
	}
};

class FileEqual{
public:  
	bool operator() ( DWS_SubChannelList left, DWS_SubChannelList right ) {
		return ( (left.ftWrite.dwHighDateTime == right.ftWrite.dwHighDateTime) && (left.ftWrite.dwLowDateTime == right.ftWrite.dwLowDateTime) );
	}
};

class IsLessRepeat {
public:  
	bool operator() ( DWS_SubChannelList left, DWS_SubChannelList right ) {
		return left.dwRepeat > right.dwRepeat;
	}
};

// common with the FilterGraph control.
#define DEFAULTTAGLENGTH			4
#define DEFAULTVERSION				1
#define DEFAULTOBJECTKEYLENGTH		4
#define DEFAULTINDEXDESCLENGTH		16
#define DEFAULTMAXSIZEOFSAMPLE		1024*1024
#define DEFAULTMINSIZEOFSAMPLE		64*1024

typedef struct DataWrapperPara
{
	long	lChannelPID;		// the PID of current channel.
	BYTE 	byTagLength;			// < 32, describe the length of tag field, default is 4.
	char	sChannelTag[MAXTAGLENGTH];	// the tag of channel, e.g. "pic\0", "hie\0", "auh\0"
	BYTE	byVersion;			// the version of channel, default is 1.
	BYTE	byObjectKeyLength;	// the length of object key field, default is 4.
	BYTE	byIndexDescLength;	// the length of index desc-info field, default is 16.
	long 	lMaxSizeOfSample;	// the max size of sample. Default:1024*1024 Bytes
	long 	lMinSizeOfSample;	// the min size of sample. Default:64*1024 Bytes
	BYTE	byMode;				// added in 2005-10-19 for MSG channel.
	int		nStreamCount;		// added in 2005-11-03 for NE channel.

	DataWrapperPara()
	{
		byTagLength = DEFAULTTAGLENGTH;
		byVersion = DEFAULTVERSION;
		byObjectKeyLength = DEFAULTOBJECTKEYLENGTH;
		byIndexDescLength = DEFAULTINDEXDESCLENGTH;
		lMaxSizeOfSample = DEFAULTMAXSIZEOFSAMPLE;
		lMinSizeOfSample = DEFAULTMINSIZEOFSAMPLE;

		lChannelPID = 0x01;
		byTagLength = DEFAULTTAGLENGTH;
		strcpy( sChannelTag, TEXT("nav") );
		byMode = 0;
		nStreamCount = 1;
	}
} DW_ParameterSet;

typedef struct StrObjectHeader{
	char		sObjectTag[DEFAULTTAGLENGTH];
	char		dwReserved[4];
	char		nObjectKeyLength;
	char		sObjectKey[DEFAULTOBJECTKEYLENGTH];
	char		dwObjectContentLength[4];
	//char		*pchObjectContent;
	StrObjectHeader()
	{
		memset( sObjectTag, 0, DEFAULTTAGLENGTH );
		memset( dwReserved, 0xFF, 4 );
		nObjectKeyLength = DEFAULTOBJECTKEYLENGTH;
		memset( sObjectKey, 0, DEFAULTOBJECTKEYLENGTH );
		memset( dwObjectContentLength, 0, 4 );
	}
} ObjectHeader;		// by test, sizeof(ObjectHeader) = 17

typedef struct SampleBlock{
	DWORD		dwProtocol; 	// 0x01: the wrapping rule used currently.
	DWORD		dwVersion; 	// 0x01: the version number of wrapping rule current.
	DWORD		dwPID;		// flag the element stream in which the content of sample is located.
	char		chTag[4];	// the affect of field is the same as the dwPID, e.g. ※pic\0§, ※hie\0§.
	BYTE        nControlFlag;  //add by simin : 0 每 start, 1 每 continue, 2 - end
	DWORD		dwCount;	// the count of object
	DWORD		dwLength;	// the length of content of the sample
	//char		*pchContent;	// content
} SampBlock;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PATTAG							0
#define PMTTAG							2
#define NITTAG							3
#define PDTTAG							4
#define PFTTAG							5

#define SHORTSECTION					1012
#define LONGSECTION						4084

#define PRISECTIONMAXLEN				4096

#define  TABLEPAYLOADMAXLEN				1045504			// 4084*256 table payload.
#define  TABLEMAPMAXLEN					1106944			// 23*188*256 : 4096 bytes need 23 ts packets to load.
#define  OUTPUTSAMPLEMAXLEN				(TABLEMAPMAXLEN+sizeof(SampBlock))
#define  OBJECTPAYLOADMAXLEN			(TABLEPAYLOADMAXLEN-sizeof(ObjectHeader))			// TABLEPAYLOADMAXLEN - sizeof(ObjectHeader)

#define		SECTION_PAYLOAD_SIZE		4084	// 4096 - 8 - 4 ( 8: section_header  4: CRC32 )
#define		SECTION_TOTAL_SIZE			4097	// 4096 + 1 ( 1: pointer_field )
#define		SECTION_HEADER_SIZE			13		// sizeof( Section_Header )+sizeof(pointer_field)
#define		PACKET_UNIT_SIZE			188
#define		PACKET_PAYLOAD_SIZE			184
#define		PACKET_HEADER_SIZE			4		// sizeof( Packet_Header )
#define		PACKETS_IN_SECTION			23		// 4097/184

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//interface for ICatalogState;
extern const IID IID_IWrapperControl; 
extern const IID IID_IStatusReport;

///////////////////////////////
//for wrapper filter
typedef void (*OnCatalogChanged)(vector< DWS_SubChannelList > pSubChannel, LPVOID lParam);

//for DataWatcherSource filter
//extern vector<DWS_SubChannelList > SubChannellists;
//extern vector<DWS_SubChannelList > TempSubChannellists;


typedef std::vector<DWS_SubChannelList > SubChannelVector;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// component guid
extern const GUID CLSID_IDataWatcherSource;
extern const GUID CLSID_CatalogConfigurePage;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//interafce iid.
//interface for ICatalogConfigure;
extern const IID IID_ICatalogConfigure;
//interface for ICatalogRountine;
extern const IID IID_ICatalogRountine;
//interface for ICatalogState;
extern const IID IID_ICatalogState; 

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//extern TCHAR g_szMsg[];
#endif
#ifndef CommonH
#define CommonH

//#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <direct.h>


#define  CATALOGPATHLEN	768
#define  FILENAMELEN	256
#define  READFILELEN	8192		// >1024 for SearchFile and DealFile

//Data Watcher Source filter status:
#define STATE_ERROR						0x0001		//Error
#define STATE_MONITORING				0x0002		//Monitoring
#define STATE_REPORTING					0x0004		//Reporting
#define STATE_STOP						0x0006		//Stop
#define STATE_READY						0x0008		//Ready

//Error Code -DataWatcherSource-
#define OPEN_FILE_FAILED				0x11101		//Open file failed
#define READ_FILE_FAILED				0x11102		//Read file failed
#define OUTOFMEMORY						0x11103		//Out of memory
#define ERROR_FILENAMEEXCED_RANGE		0x11104		//Catalog name larger than CATALOGPATHLEN
#define CREATE_EVENT_FAILED				0x11105		//Create event failed
#define CREATE_THREAD_FAILED			0x11106		//Create thread failed
#define MONITORCATALOG_THREAD_EXCEPTION 0x11107		//Monitor catalog thread occur exception
#define GETTIME_FAILED					0x11108		//SyncReadAligned interface function sample gettime failed
#define GETPOINTER_FAILED				0x11109		//SyncReadAligned interface function sample getpointer failed

//Error Code -DataWrapperFilter-
#define WRAPPERFILTER_OUTOFMEMORY			0x11201		//Out of memory
#define WRAPPERFILTER_CREATE_THREAD_FAILED	0x11202		//Create thread failed
#define SET_OBJKEYLENGTH_OVERFLOW			0x11204		//set the objectkeylength overflow
#define SET_TAGLENGTH_OVERFLOW				0x11208		//set the tag length overflow
#define SET_VERSIONNUMBER_OVERFLOW			0x11216		//set the version number overflow
#define SET_INDEXDESCLENGTH_OVERFLOW		0x11232		//set the indexdesclength overflow

// common with the FilterGraph control.
#define MAXOBJECTKEYLENGTH			16
#define MAXTAGLENGTH				32
#define DEFAULTTAGLENGTH			4
#define DEFAULTVERSION				1
#define DEFAULTOBJECTKEYLENGTH		4
#define DEFAULTINDEXDESCLENGTH		16
#define DEFAULTMAXSIZEOFSAMPLE		1024*1024
#define DEFAULTMINSIZEOFSAMPLE		64*1024

using namespace std;

typedef struct catalog 
{
	WORD 		wID;						//ID for catalog,set by sw itself	
	WORD		wCount;						//Files Count		    
	char 		szPath[CATALOGPATHLEN];		//Catalog path 
} CCatalog;

//extern std::vector<CCatalog * > Catalogs;

#define MAXOBJECTKEYLENGTH		16
#define MAXTAGLENGTH			32

struct subchannellist
{
public:
	WORD		wID;						//ID for catalog,set by sw itself
	char		szFileName[FILENAMELEN];	//Full path and file name
	DWORD		dwFileSize;					//File size
	DWORD		dwRepeat;					//File repeat times.
	FILETIME	ftCreate;					//time the file was created
	FILETIME	ftAccess;					//time the file was last accessed
	FILETIME	ftWrite;					//time the file was last written to.

	BYTE	byObjectKeyLength;				// the length of object key field, default is 4.
	char	sObjectKey[MAXOBJECTKEYLENGTH]; // the object key of the sub channel, used by DataWrapperFilter
	BYTE	byTableID;						// table id denoted, used by DataWrapperFilter
	WORD	wTableIDExtension;				// the first table id extension denoted, used by DataWrapperFilter
	WORD	wReserve;						// used for version number.
	BYTE	byControlFlag;					// used to judge the start and end of file stream.
	BYTE	byTableCount;					// how many chips need this file be splitted.

	long	lChannelPID;
	BYTE 	byTagLength;					// < 32, describe the length of tag field, default is 4.
	char	sChannelTag[MAXTAGLENGTH];		// the tag of channel, e.g. "pic\0", "hie\0", "auh\0"

	//long	lContentSize;
	//char	*pContent;

	subchannellist()
	{
		memset( szFileName, 0, FILENAMELEN );
		dwRepeat = 1;

		wReserve = 0;
		byControlFlag = 1;
		byTableCount = 0;

		byObjectKeyLength = DEFAULTOBJECTKEYLENGTH;
		memset( sObjectKey, 0, MAXOBJECTKEYLENGTH );
		byTagLength = DEFAULTTAGLENGTH;
		memset( sChannelTag, 0, MAXTAGLENGTH );

		//lContentSize = 0;
		//pContent = NULL;
	}
	//bool operator< ( const subchannellist& left, const subchannellist& right )
	//{
	//	return left.dwRepeat < right.dwRepeat;
	//}
} ;
typedef struct subchannellist DWS_SubChannelList;

typedef struct ObjectIndexDescriptor
{
	BYTE	byTag;
	BYTE	byLength;
	BYTE	byTableCount;
	BYTE	byIDLength;									// = DEFAULTOBJECTKEYLENGTH
	char	pchID[DEFAULTOBJECTKEYLENGTH];
	BYTE	byDescLength;								// = DEFAULTINDEXDESCLENGTH
	char	pchDesc[DEFAULTINDEXDESCLENGTH];
	ObjectIndexDescriptor()
	{
		byTag = 0x80;
		byLength = sizeof(ObjectIndexDescriptor) - 2;
		byTableCount = 0;
		byIDLength = DEFAULTOBJECTKEYLENGTH;
		byDescLength = DEFAULTINDEXDESCLENGTH;
	}
} ObjectDescriptor;

class FileSort{
public:  
	bool operator() ( DWS_SubChannelList left, DWS_SubChannelList right ) {
		return stricmp(left.szFileName, right.szFileName) > 0;
	}
};

class FileEqual{
public:  
	bool operator() ( DWS_SubChannelList left, DWS_SubChannelList right ) {
		return ( (left.ftWrite.dwHighDateTime == right.ftWrite.dwHighDateTime) && (left.ftWrite.dwLowDateTime == right.ftWrite.dwLowDateTime) );
	}
};

class IsLessRepeat {
public:  
	bool operator() ( DWS_SubChannelList left, DWS_SubChannelList right ) {
		return left.dwRepeat > right.dwRepeat;
	}
};

// common with the FilterGraph control.
#define DEFAULTTAGLENGTH			4
#define DEFAULTVERSION				1
#define DEFAULTOBJECTKEYLENGTH		4
#define DEFAULTINDEXDESCLENGTH		16
#define DEFAULTMAXSIZEOFSAMPLE		1024*1024
#define DEFAULTMINSIZEOFSAMPLE		64*1024

typedef struct DataWrapperPara
{
	long	lChannelPID;		// the PID of current channel.
	BYTE 	byTagLength;			// < 32, describe the length of tag field, default is 4.
	char	sChannelTag[MAXTAGLENGTH];	// the tag of channel, e.g. "pic\0", "hie\0", "auh\0"
	BYTE	byVersion;			// the version of channel, default is 1.
	BYTE	byObjectKeyLength;	// the length of object key field, default is 4.
	BYTE	byIndexDescLength;	// the length of index desc-info field, default is 16.
	long 	lMaxSizeOfSample;	// the max size of sample. Default:1024*1024 Bytes
	long 	lMinSizeOfSample;	// the min size of sample. Default:64*1024 Bytes
	DataWrapperPara()
	{
		byTagLength = DEFAULTTAGLENGTH;
		byVersion = DEFAULTVERSION;
		byObjectKeyLength = DEFAULTOBJECTKEYLENGTH;
		byIndexDescLength = DEFAULTINDEXDESCLENGTH;
		lMaxSizeOfSample = DEFAULTMAXSIZEOFSAMPLE;
		lMinSizeOfSample = DEFAULTMINSIZEOFSAMPLE;

		lChannelPID = 0x01;
		byTagLength = DEFAULTTAGLENGTH;
		strcpy( sChannelTag, TEXT("hie") );
	}
} DW_ParameterSet;

typedef struct StrObjectHeader{
	char		sObjectTag[DEFAULTTAGLENGTH];
	char		dwReserved[4];
	char		nObjectKeyLength;
	char		sObjectKey[DEFAULTOBJECTKEYLENGTH];
	char		dwObjectContentLength[4];
	//char		*pchObjectContent;
} ObjectHeader;		// by test, sizeof(ObjectHeader) = 17

typedef struct SampleBlock{
	DWORD		dwProtocol; 	// 0x01: the wrapping rule used currently.
	DWORD		dwVersion; 	// 0x01: the version number of wrapping rule current.
	DWORD		dwPID;		// flag the element stream in which the content of sample is located.
	char		chTag[4];	// the affect of field is the same as the dwPID, e.g. ※pic\0§, ※hie\0§.
	BYTE        nControlFlag;  //add by simin : 0 每 start, 1 每 continue, 2 - end
	DWORD		dwCount;	// the count of object
	DWORD		dwLength;	// the length of content of the sample
	//char		*pchContent;	// content
} SampBlock;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define PATTAG							0
#define PMTTAG							2
#define NITTAG							3
#define PDTTAG							4
#define PFTTAG							5

#define SHORTSECTION					1012
#define LONGSECTION						4084

#define		SECTION_PAYLOAD_SIZE		4084	// 4096 - 8 - 4 ( 8: section_header  4: CRC32 )
#define		SECTION_HEADER_SIZE			13		// sizeof( Section_Header )+sizeof(pointer_field)

#define PRISECTIONMAXLEN				4096

#define  TABLEPAYLOADMAXLEN				1045504			// 4084*256 table payload.
#define  TABLEMAPMAXLEN					1106944			// 23*188*256 : 4096 bytes need 23 ts packets to load.
#define  OUTPUTSAMPLEMAXLEN				(TABLEMAPMAXLEN+sizeof(SampBlock))
#define  OBJECTPAYLOADMAXLEN			(TABLEPAYLOADMAXLEN-sizeof(ObjectHeader))			// TABLEPAYLOADMAXLEN - sizeof(ObjectHeader)

#define		SECTION_PAYLOAD_SIZE		4084	// 4096 - 8 - 4 ( 8: section_header  4: CRC32 )
#define		SECTION_TOTAL_SIZE			4097	// 4096 + 1 ( 1: pointer_field )
#define		SECTION_HEADER_SIZE			13		// sizeof( Section_Header )+sizeof(pointer_field)
#define		PACKET_UNIT_SIZE			188
#define		PACKET_PAYLOAD_SIZE			184
#define		PACKET_HEADER_SIZE			4		// sizeof( Packet_Header )
#define		PACKETS_IN_SECTION			23		// 4097/184

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//interface for ICatalogState;
extern const IID IID_IWrapperControl; 
extern const IID IID_IStatusReport;

///////////////////////////////
//for wrapper filter
typedef void (*OnCatalogChanged)(vector< DWS_SubChannelList > pSubChannel, LPVOID lParam);

//for DataWatcherSource filter
//extern vector<DWS_SubChannelList > SubChannellists;
//extern vector<DWS_SubChannelList > TempSubChannellists;


typedef std::vector<DWS_SubChannelList > SubChannelVector;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// component guid
extern const GUID CLSID_IDataWatcherSource;
extern const GUID CLSID_CatalogConfigurePage;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//interafce iid.
//interface for ICatalogConfigure;
extern const IID IID_ICatalogConfigure;
//interface for ICatalogRountine;
extern const IID IID_ICatalogRountine;
//interface for ICatalogState;
extern const IID IID_ICatalogState; 

//////////////////////////////////////////////////////////////////////////
// added by Cary

#ifndef _NO_FIX_LOG
extern ISvcLog* service_log;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//extern TCHAR g_szMsg[];
#endif