// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: IPathHelperObject.h $
// Branch: $Name:  $
// Author: huang li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/GB_IMODHelperObj.h $
// 
// 14    10/24/14 9:13a Li.huang
// 
// 13    2/26/14 4:07p Li.huang
// 
// 12    11/27/12 12:24p Li.huang
// 
// 11    11/26/12 3:04p Li.huang
// 
// 10    11/08/12 2:56p Li.huang
// 
// 9     7/26/12 5:16p Li.huang
// 
// 8     6/29/11 9:47a Li.huang
// add  GB AAA
// 
// 7     4/14/11 11:42a Fei.huang
// * migrated to linux
// 
// 6     4/13/11 10:56a Li.huang
// 
// 5     4/13/11 9:40a Li.huang
// 
// 4     4/12/11 4:42p Fei.huang
// 
// 3     4/12/11 4:28p Li.huang
// implement TV-Now_Advertisement
// 
// 2     4/12/11 4:27p Fei.huang
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 8     10-06-29 16:32 Li.huang
// 
// 7     09-04-28 16:08 Li.huang
// modify MHO InitMHO interface define
// 
// 6     09-03-16 11:04 Li.huang
// modify interface datatype defination
// 
// 5     08-07-22 14:32 Li.huang
// 
// 4     08-03-31 12:45 Li.huang
// 
// 3     08-03-24 17:04 Li.huang
// 
// 2     08-03-12 11:22 Li.huang
// 
// 1     08-03-04 11:15 Li.huang
// ===========================================================================

#ifndef __ZQTianShan_MODPlugInHelper_H__GB
#define __ZQTianShan_MODPlugInHelper_H__GB

#include "ZQ_common_conf.h"
#include "Exception.h"
#include "Locks.h"
#include "DynSharedObj.h"
#include "TianShanDefines.h"

namespace ZQTianShan {
namespace Application{
namespace MOD{

#ifdef ZQ_OS_MSWIN
#define MHO_FILENAME_PREFIX		"MHO_"
#define MHO_EXT		".dll"
#else
#define MHO_FILENAME_PREFIX		"libMHO_"
#define MHO_EXT		".so"
#endif
	
class IMHOManager;
class IAuthorization;
class IPlayListQuery;
class IAdsReplacement;
class IAAA;
// -----------------------------
// interface IMHOManager
// -----------------------------
/// A MOD helper object assistants AccreditPath on one specific type of StorageLink or StreamLink.
/// interface IStorageLinkPHO defines the basic entries of a StorageLink helper object
class IMHOManager
{
public:
	virtual ~IMHOManager() {}
	/// register a Authorization object, called by a MODHelp plugin during InitMODHelper() entry
	///@param[in] type	 the string type of Authorization about to associated by this helper object
	///@param[in] helper the helper object
	///@return    pointer to the helper if succeeded, otherwise NULL
	virtual IAuthorization* registerAuthorization(const char* type, IAuthorization& helper, void* pCtx) =0;
	virtual IAuthorization* unregisterAuthorization(const char* type) =0;
	virtual IAuthorization* findAuthorizationHelper(const char* type) =0;

	/// register a PlayListQuery object, called by a MODHelp plugin during InitPathHelper() entry
	///@param[in] type	 the string type of PlayListQuery about to associated by this helper object
	///@param[in] helper the helper object
	///@return    pointer to the helper if succeeded, otherwise NULL
	virtual IPlayListQuery* registerPlayListQuery(const char* type, IPlayListQuery& helper, void* pCtx) =0;
	virtual IPlayListQuery* unregistPlayListQuery(const char* type) =0;
	virtual IPlayListQuery* findPlayListQueryHelper(const char* type) =0;

	/// register a AdsReplacement object, called by a MODHelp plugin during InitPathHelper() entry
	///@param[in] type	 the string type of PlayListQuery about to associated by this helper object
	///@param[in] helper the helper object
	///@return    pointer to the helper if succeeded, otherwise NULL
	virtual IAdsReplacement* registerAdsReplacement(const char* type, IAdsReplacement& helper, void* pCtx) =0;
	virtual IAdsReplacement* unregistAdsReplacement(const char* type) =0;
	virtual IAdsReplacement* findAdsReplacement(const char* type) =0;


	virtual IAAA* registerAAA(const char* type, IAAA& helper, void* pCtx)= 0;
	virtual IAAA* unregistAAA(const char* type) = 0;
	virtual IAAA* findAAA(const char* type) = 0;
    ///list all supported Authorization types
	///@return a list of Authorization string types
    virtual ::TianShanIce::StrValues listSupportedAuthorizationTypes() =0;

    ///list all supported Play List Query types
	///@return a list of Play List Query string types
    virtual ::TianShanIce::StrValues listSupportedPlayListQueryTypes() =0;

	///list all supported AdsReplacement Query types
	///@return a list of AdsReplacement Query string types
	virtual ::TianShanIce::StrValues listSupportedAdsReplacementTypes() =0;

	virtual ::TianShanIce::StrValues listSupportedAAATypes() = 0;
};
typedef std::vector<std::string> StringCollection;
typedef std::map<std::string, std::string> AttributesMap;

typedef struct _AEInfo
{
	std::string aeUID;	// the uid of asset element, Hex format, 8 bits and capital
	int    bandWidth;		// bandWidth, is in bps (bit per second)
	int    cueIn;			// When it is -1, means no cue in point, CueIn is in millisecond
	int    cueOut;			// When it is -1, means no cue out point, CueOut is in millisecond
	//		std::string      name;  //only content name, without volume name
	StringCollection nasUrls;
	StringCollection volumeList;
	AttributesMap    attributes; //for extending purpose  
}AEInfo;

// the collection of AE is a sequence or vector
typedef std::vector<AEInfo> AssetElementCollection;

// the collection of net_id is a sequence or vector
typedef std::vector<std::string> NetIDCollection;

typedef struct _AEReturnData
{
	NetIDCollection netIDList; // the uid of net_id, optional
	AssetElementCollection aeList; // AE list
	int  useNasURL;  //1: yes, 0 : no
}AEReturnData;

// -----------------------------
// plugin facet IAuthorization
// -----------------------------
/// A mod helper object assistants on one specific type of Authorization.
/// interface IAuthorization defines the basic entries of a Authorization helper object
class IAuthorization
{
public:
	typedef struct _AuthorInfo
	{
		::std::string endpoint;
		::std::string serverSessionId;
		::std::string clientSessionId;
		::Ice::Identity ident;
	}AuthorInfo;

	enum AuthorError{AUTHORSUCCESS = 0, AUTHORFAILED, ICEEXCEPTION, UNKNOWNTYPE, INTERNAL, UNKNOWN};
public:
	virtual ~IAuthorization() {}
	virtual int OnAuthPurchase(AuthorInfo& authorInfo, ::TianShanIce::ValueMap& privData)  = 0;		
	virtual	int OnDestroyPurchase(AuthorInfo& authorInfo, const ::TianShanIce::Properties& prop) = 0;
};

// -----------------------------
// plugin facet IGetPlayList
// -----------------------------
/// A path helper object on one specific type of PlayListQuery.
/// interface IPlayListQuery defines the basic entries of a PlayListQuery helper object
class IPlayListQuery
{
public:
	enum {getaeList = 0,  getaeListWithAppUID, getaeListByPIDandAID, getAeListforSurf};
	
	typedef struct _PlayListInfo
	{
		int nType;
		::std::string UID1; //
		::std::string UID2; //
		::std::string UID3; 
		::std::string endpoint;//
		::Ice::Identity ident;
	}PlayListInfo;

    enum PlayListQueryError{PLQUERYSUCCESS = 0, PLNOTEXIST, ICEEXCEPTION, UNKNOWNTYPE, INTERNAL, NOTSUPPORT, UNKNOWN};

public:
	virtual ~IPlayListQuery() {}
	virtual int getPlayList(PlayListInfo& plinfo, AEReturnData& aedata) = 0;
};
// -----------------------------
// plugin facet IAdsReplacement
// -----------------------------
/// A path helper object on one specific type of AdsReplacement.
/// interface IPlayListQuery defines the basic entries of a AdsReplacement helper object
class IAdsReplacement
{
public:

	typedef struct _AdsInfo
	{
		::std::string endpoint;
		::std::string serverSessionId;
		::std::string clientSessionId;
		::Ice::Identity ident;
	}AdsInfo;

	enum AdsReplacementError{ADSQUERYSUCCESS = 0, PLNOTEXIST, ICEEXCEPTION, UNKNOWNTYPE, INTERNAL, NOTSUPPORT, UNKNOWN};

public:
	virtual ~IAdsReplacement() {}
	virtual int getAdsReplacement(AdsInfo& adsinfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData) = 0;
};

// -----------------------------
// plugin facet Accounting Authentication Authorization
// -----------------------------
/// A path helper object on one specific type of AdsReplacement.
/// interface IPlayListQuery defines the basic entries of a AdsReplacement helper object
class IAAA
{
public:

	typedef struct _AAAInfo
	{
		std::string endpoint;
		std::string serverSessionId;
	    std::string clientSessionId;
		Ice::Identity ident;

		std::string sessionID; //session id
		std::string entitlementCode; //可选，由AAA生成后通告门户系统下发给机顶盒，在点播请求中传递给SM后，SM再传递给AAA进行鉴权比对
		std::string userID;  // 点播用户标识
		//teardown parameter
		std::string commmand;  //此处应当取值为Release或Setup
		std::string contentId; //点播节目资产id
		float        stopNPT;   //会话发生的系统时间；对于Setup会话建立成功的消息，应当以会话管理器从机顶盒收到SETUP请求时间为准；对于Release会话释放成功的消息，应当以会话管理器从机顶盒收到TEARDOWN请求时间为准
		std::string timeStamp; //timeStamp
		std::string tearDownReason;
		std::string terminateReason;

		///add for HENanAAA
		int locality;
		std::string deviceId;
		int stopAssetIndex;
		std::string playListId;
		std::string usage;

		int  errorCode; //当Author失败后, 由AAA模块返回
		TianShanIce::Properties prop;	
	}AAAInfo;

	enum AAAError{AAAQUERYSUCCESS = 0, AUTHORFAILED, ICEEXCEPTION, UNKNOWNTYPE, INTERNAL, NOTSUPPORT, UNKNOWN};

public:
	virtual ~IAAA() {}
	virtual int OnAuthorize(AAAInfo& aaaInfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData) = 0;
	virtual	int OnStatusNotice(AAAInfo& aaaInfo, const ::TianShanIce::Properties& prop) = 0;
};

class MHOPluginFacet : public ZQ::common::DynSharedFacet
{
	// declare this Facet object as a child of DynSharedFacet
	DECLARE_DSOFACET(MHOPluginFacet, DynSharedFacet);

	// declare the API prototypes
	DECLARE_PROC(void, InitMHO, (IMHOManager& mgr, void* pCtx,const char * configfile, const char* logFolder, const char * modinstanceID, Ice::CommunicatorPtr& ic));
	DECLARE_PROC(void, UnInitMHO, (void));

	// map the external APIs
	DSOFACET_PROC_BEGIN();
		DSOFACET_PROC(InitMHO);
		DSOFACET_PROC(UnInitMHO);
	DSOFACET_PROC_END();
};


}}} // namespace

#endif // __ZQTianShan_MODPlugInHelper_H__GB
