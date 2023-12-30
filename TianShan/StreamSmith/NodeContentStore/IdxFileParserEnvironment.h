
#ifndef __VSTRM_INDEX_FILE_PARSER_ENVIRONMENT_HEADER_FILE_H__
#define __VSTRM_INDEX_FILE_PARSER_ENVIRONMENT_HEADER_FILE_H__

#include <Log.h>
#include <DynSharedObj.h>
#include <assert.h>

extern "C"
{	
#ifdef ZQ_OS_MSWIN
	#include <vstrmtypedef.h>
	#include <vstrmtypes.h>
#else
	#include "dataTypeHelper.h"
#endif

#ifndef EXCLUDE_VSTRM_API
	#include <VstrmUser.h>
#endif

	#pragma pack(8)
	#include <vvx.h>
	#pragma pack()
}



//#define EXCLUDE_VSTRM_API

#ifndef VOD_ASSET_MAIN_FILE_INDEX
	//dummy definition
	#define VOD_ASSET_MAX_SUBFILES					9
	#define VOD_ASSET_MAX_SUBFILE_EXTENSION			32
	#define VOD_ASSET_MAX_INDEXFILE_EXTENSION		32
	#define VOD_ASSET_VV2_READ_BUFFER_SIZE			1024

	#define VOD_ASSET_VVX_EXTENSION					"VVX"
	#define VOD_ASSET_VV2_EXTENSION					"VV2"
	#define VOD_ASSET_VVC_EXTENSION					"INDEX"
	#define VOD_ASSET_FILE_EXTENSION_DELIMITER 		"."


	typedef enum _VOD_ASSET_INDEX_TYPE
	{
		VOD_ASSET_INDEX_IS_UNKNOWN = 0,
		VOD_ASSET_INDEX_IS_VVX,
		VOD_ASSET_INDEX_IS_VV2,
		VOD_ASSET_INDEX_IS_VVC,
		VOD_ASSET_INDEX_MAXIMUM,
	} VOD_ASSET_INDEX_TYPE;

	typedef enum _VOD_ASSET_LOCATION
	{
		VOD_ASSET_LOCATION_INVALID	= 0,
		VOD_ASSET_LOCATION_LOCAL,
		VOD_ASSET_LOCATION_REMOTE,
	} VOD_ASSET_LOCATION;

	typedef struct _VOD_ASSET_SUBFILE_DESC
	{
		UINT64		startByteOffset;						// Starting byte offset
		UINT64		fileLength;								// Length of file
		ULONG		playTimeInMilliSeconds;					// Play time
		ULONG		bitRate;								// Bit rate (valid for normal speed file)
		LONG		speedNumerator;							// File speed
		ULONG		speedDenominator;						//   "   "
		char		extension[(VOD_ASSET_MAX_SUBFILE_EXTENSION + 1)];	// Subfile extension (null-terminated)
	} VOD_ASSET_SUBFILE_DESC, *PVOD_ASSET_SUBFILE_DESC;

	#define VOD_ASSET_MAIN_FILE_INDEX			0

	typedef struct _VOD_ASSET_INFO
	{
		int						majorVersion;					// Index major version
		int						minorVersion;					// Index minor version
		int						numberOfSubFiles;				// Number of subfiles in index
		VOD_ASSET_SUBFILE_DESC	subFile[VOD_ASSET_MAX_SUBFILES];// Array of subfile descriptions ([0] is normal speed file)
		VOD_ASSET_INDEX_TYPE	indexType;						// Type of index (set to UNKNOWN if file not found)
		VOD_ASSET_LOCATION		indexLocation;					// Is the asset local or remote?
		char					indexExtension[VOD_ASSET_MAX_INDEXFILE_EXTENSION];
	} VOD_ASSET_INFO, *PVOD_ASSET_INFO;
#endif 


typedef VSTATUS (*PVstrmClassLoadBriefAssetInfo)( HANDLE,LPCTSTR,PULONG,PULONG,VOD_ASSET_INDEX_TYPE*,VOD_ASSET_LOCATION);

typedef	VSTATUS	(*PVstrmClassLoadFullAssetInfo)(HANDLE,LPCTSTR,PVOD_ASSET_INFO,PULONG);

typedef VSTATUS (*PVstrmClassLoadBriefAssetInfoEx)( HANDLE,LPCTSTR,PULONG,PULONG,VOD_ASSET_INDEX_TYPE*,VOD_ASSET_LOCATION,PUCHAR);

typedef	VSTATUS	(*PVstrmClassLoadFullAssetInfoEx)(HANDLE,LPCTSTR,PVOD_ASSET_INFO,PULONG,PUCHAR);


namespace ZQ
{
namespace IdxParser
{

	

class VstrmLoadAssetInfo:public ZQ::common::DynSharedFacet
{
public:
	DECLARE_DSOFACET(VstrmLoadAssetInfo, DynSharedFacet);
	
	DECLARE_PROC(VSTATUS, LoadBriefAssetInfo, ( HANDLE,LPCTSTR,PULONG,PULONG,VOD_ASSET_INDEX_TYPE*,VOD_ASSET_LOCATION));
	DECLARE_PROC(VSTATUS, LoadFullAssetInfo, (HANDLE,LPCTSTR,PVOD_ASSET_INFO,PULONG));

	// map the external APIs
	DSOFACET_PROC_BEGIN();
		DSOFACET_PROC_SPECIAL(LoadBriefAssetInfo,"VstrmClassLoadBriefAssetInfo");
		DSOFACET_PROC_SPECIAL(LoadFullAssetInfo, "VstrmClassLoadFullAssetInfo");
	DSOFACET_PROC_END();
};

class VstrmLoadAssetInfoEx:public ZQ::common::DynSharedFacet
{
public:
	DECLARE_DSOFACET(VstrmLoadAssetInfoEx, DynSharedFacet);

	DECLARE_PROC(VSTATUS, LoadBriefAssetInfoEx, ( HANDLE,LPCTSTR,PULONG,PULONG,VOD_ASSET_INDEX_TYPE*,VOD_ASSET_LOCATION,PUCHAR));
	DECLARE_PROC(VSTATUS, LoadFullAssetInfoEx, (HANDLE,LPCTSTR,PVOD_ASSET_INFO,PULONG,PUCHAR));

	// map the external APIs
	DSOFACET_PROC_BEGIN();
		DSOFACET_PROC_SPECIAL(LoadBriefAssetInfoEx,"VstrmClassLoadBriefAssetInfoEx");
		DSOFACET_PROC_SPECIAL(LoadFullAssetInfoEx, "VstrmClassLoadFullAssetInfoEx");
	DSOFACET_PROC_END();
};

class IdxParserEnv
{
public:
	IdxParserEnv();
	~IdxParserEnv( );
public:

	///attach a available logger to IdxFileParserEnvironment
	///It's caller's responsibility to initialize the logger
	///IdxParserEnv does not have responsibility to create or destroy the logger
	/// @param logger the logger instance to be attached to IdxParserEnv
	///					if the logger is NULL , IIdxParserEnv will create a default logger
	void					AttchLogger( ZQ::common::Log* logger = NULL );
	
	///detach the logger
	/// after this call , no available logger can be used in IdxParserEnv
	void					DetachLogger( );

#ifndef EXCLUDE_VSTRM_API
	///initialize vstrm environment
	///@param vstrmHandle the vstrm handle resource for future use
	///						if the handle is NULL , IdxParserEnv will create a default vstrm handle
	void					InitVstrmEnv( VHANDLE vstrmHandle = NULL );

	///un-initialize vstrm environment
	void					UninitVstrmEnv( );

	void					setUseVstrmIndexParseAPI( bool bUse );

	bool					canUseVstrmIndexParseAPI( ) const;

#endif //EXCLUDE_VSTRM_API

	

public:

	///get logger
	inline ZQ::common::Log*		GetLogger( ) const;

	void						setSkipZeroByteFile( bool bSkip )
	{
		mbSkipZeroByteFile = bSkip;
	}

	inline bool					skipZeroByteFile( ) const
	{
		return mbSkipZeroByteFile;
	}

#ifndef EXCLUDE_VSTRM_API
	
	void						setUseVsOpenAPI( bool bUseVsOpen )
	{
		mbUseVsOpen = bUseVsOpen;
	}

	inline bool					useVsOpenAPI( ) const
	{
		return mbUseVsOpen;
	}
	///get vstrm handle
	inline VHANDLE				GetVstrmHandle( ) const;

	bool						isVstrmLoadInfoMethodAvailable( ) const
	{
		return mpVstrmLoadAssetInfoMethod != NULL;
	}
	VstrmLoadAssetInfo*			getLoadInfoMethod( )
	{
		return mpVstrmLoadAssetInfoMethod;
	}
	VstrmLoadAssetInfoEx*		getLoadInfoExMethod( )
	{
		return mpVstrmLoadAssetInfoExMethod;
	}

#endif //EXCLUDE_VSTRM_API

	

protected:

private:

#ifndef EXCLUDE_VSTRM_API
	//If we use sea file system , we should initialize this handle
	VHANDLE				vstrmHandle;
	bool				bVstrmInitialByCaller;
#endif//EXCLUDE_VSTRM_API

	ZQ::common::Log*	logger;
	bool				bLoggerIntialiByCaller;

private:

	bool								mbCanUseVstrmAPIToParseIndexFile;

	ZQ::common::DynSharedObj			mVstrmLoadAssetInfoObj;
	VstrmLoadAssetInfo					*mpVstrmLoadAssetInfoMethod;
	VstrmLoadAssetInfoEx				*mpVstrmLoadAssetInfoExMethod;
	bool								mbSkipZeroByteFile;
	bool								mbUseVsOpen;

};

inline ZQ::common::Log* IdxParserEnv::GetLogger() const
{	
	return (logger);
}
#ifndef EXCLUDE_VSTRM_API
inline VHANDLE IdxParserEnv::GetVstrmHandle() const
{
	assert( vstrmHandle!= NULL );
	return vstrmHandle;
}
#endif//EXCLUDE_VSTRM_API

extern IdxParserEnv _idxParserEnv;


}}//namespace ZQ::IdxParser

#endif //__VSTRM_INDEX_FILE_PARSER_ENVIRONMENT_HEADER_FILE_H__
