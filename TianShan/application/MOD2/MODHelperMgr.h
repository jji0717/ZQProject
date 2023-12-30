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
// Ident : $Id: PathHelperMgr.h $
// Branch: $Name:  $
// Author: huang li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/application/MOD2/MODHelperMgr.h $
// 
// 4     12/08/16 3:12p Li.huang
// modidy aesst to  asset
// 
// 3     12/08/16 9:32a Li.huang
// add IPTV and AssetLocation
// 
// 2     1/15/15 5:22p Li.huang
// fix bug 20607
// 
// 1     4/02/12 4:36p Li.huang
// Megre From 1.15
// 
// 4     4/02/12 10:17a Li.huang
// add LookUp Pid method
// 
// 3     4/14/11 11:41a Fei.huang
// * migrated to linux
// 
// 2     4/12/11 4:04p Li.huang
// implement TV-Now_Advertisement
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 4     09-04-28 16:10 Li.huang
// 
// 3     08-07-22 14:31 Li.huang
// 
// 2     08-03-24 17:04 Li.huang
// 
// 1     08-03-10 16:58 Li.huang
// ===========================================================================

#ifndef __ZQTianShan_MODHelperMgr_H__
#define __ZQTianShan_MODHelperMgr_H__

#include "ZQ_common_conf.h"
#include "Exception.h"
#include "Locks.h"

#include "../../common/IMODHelperObj.h"

namespace ZQTianShan {
namespace Application{
namespace MOD{

class MODAppHelperMgr : public ::Ice::LocalObject, public IMHOManager
{
public:

	MODAppHelperMgr();
	virtual ~MODAppHelperMgr();

	void setExtraData(const char* configFile,const char* logFolder, std::string MODinstanceID, Ice::CommunicatorPtr& ic);
	int populate(const char* pathToMHO);
	int populate(const std::vector<std::string>& plugins);

    bool getAuthorizationInfo(const ::std::string& type, ::ZQ::common::DynSharedObj::ImageInfo& info);
    bool getPlayListQueryInfo(const ::std::string& type, ::ZQ::common::DynSharedObj::ImageInfo& info);

public: // impl of IMHOManager

	virtual IAuthorization* registerAuthorization(const char* type, IAuthorization& helper, void* pCtx);
	virtual IAuthorization* unregisterAuthorization(const char* type);
	virtual IAuthorization* findAuthorizationHelper(const char* type);

	virtual IPlayListQuery* registerPlayListQuery(const char* type, IPlayListQuery& helper, void* pCtx);
	virtual IPlayListQuery* unregistPlayListQuery(const char* type);
	virtual IPlayListQuery* findPlayListQueryHelper(const char* type);

	virtual IAdsReplacement* registerAdsReplacement(const char* type, IAdsReplacement& helper, void* pCtx);
	virtual IAdsReplacement* unregistAdsReplacement(const char* type);
	virtual IAdsReplacement* findAdsReplacement(const char* type);

	virtual IProviderQuery* registerProviderQuery(const char* type, IProviderQuery& helper, void* pCtx);
	virtual IProviderQuery* unregistProviderQuery(const char* type);
	virtual IProviderQuery* findProviderQuery(const char* type);

	virtual IAssetLocation* registerAssetLocation(const char* type, IAssetLocation& helper, void* pCtx);
	virtual IAssetLocation* unregistAssetLocation(const char* type);
	virtual IAssetLocation* findAssetLocation(const char* type);

  virtual TianShanIce::StrValues listSupportedAuthorizationTypes();
	
  virtual TianShanIce::StrValues listSupportedPlayListQueryTypes();

  virtual TianShanIce::StrValues listSupportedAdsReplacementTypes();

  virtual TianShanIce::StrValues listSupportedProviderQueryTypes();

  virtual TianShanIce::StrValues listSupportedAssetLocationTypes();

  int OnAuthPurchase(const char* type, IAuthorization::AuthorInfo& authorInfo, ::TianShanIce::ValueMap& privData);		
  int OnDestroyPurchase(const char* type, IAuthorization::AuthorInfo& authorInfo, const ::TianShanIce::Properties& prop);
  int getPlayList(const char* type, IPlayListQuery::PlayListInfo& plinfo, AEReturnData& aedata);
  int getAdsRepalcement(const char* type, IAdsReplacement::AdsInfo& adsInfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData);
  int getAssetLocation(const char* type, IAssetLocation::AssetLocationInfo& adsInfo, AEReturnData& aedata, ::TianShanIce::ValueMap& privData);

  int getProviderId(const char* type, IProviderQuery::ProviderInfo& pidInfo);

  const char * getErrorMsg(int errCode);

protected:

	typedef struct _MhoAuthorNode
	{
		IAuthorization* pAuthorMHO;
		std::string        fnPlugin;
	} MHOAuthorNode;

	typedef std::map<std::string, MHOAuthorNode> MHOAuthorMap; ///< a map of link-type to StorageLinkHelper object

	typedef struct _MhoPLQueryNode
	{
		IPlayListQuery* pPLQueryMHO;
		std::string        fnPlugin;
	} MHOPLQueryNode;
	
	typedef std::map<std::string, MHOPLQueryNode> MHOPLQueryMap;

	typedef struct _MhoAdsNode
	{
		IAdsReplacement*   pAdsMHO;
		std::string        fnPlugin;
	} MHOAdsNode;

	typedef std::map<std::string, MHOAdsNode> MHOAdsMap;


	typedef struct _MhoAssetLocationNode
	{
		IAssetLocation*   pAssetLocMHO;
		std::string        fnPlugin;
	} MHOAssetLocNode;

	typedef std::map<std::string, MHOAssetLocNode> MHOAssetLocsMap;

	typedef struct _MhoPidNode
	{
		IProviderQuery*   pPidMHO;
		std::string        fnPlugin;
	} MHOPidNode;

	typedef std::map<std::string, MHOPidNode> MHOPidMap;

	MHOAuthorMap	_AuthorizationMap;
	ZQ::common::Mutex   _lockAuthorizationMap;

	MHOPLQueryMap	_PLQueryMap;
	ZQ::common::Mutex   _lockPLQueryMap;

	MHOAdsMap	_AdsMap;
	ZQ::common::Mutex   _lockAdsMap;

	MHOAssetLocsMap	_AssetLocMap;
	ZQ::common::Mutex   _lockAssetLocMap;

	MHOPidMap	_PidMap;
	ZQ::common::Mutex   _lockPidMap;

protected:

	ZQ::common::DynSharedObj* loadMHO(const char* filename);
	typedef std::map<std::string, ZQ::common::DynSharedObj* > MHOMap; ///< a map of link-type to MHOHelp object
	MHOMap  _MHOMap;
	ZQ::common::Mutex   _lockMHOMap;
	std::string			_strLogFolder;
	std::string   _strConfigFile;
	std::string   m_InstanceID;
	Ice::CommunicatorPtr m_ic;
};

}}}// namespace

#endif // __ZQTianShan_PathHelperMgr_H__
