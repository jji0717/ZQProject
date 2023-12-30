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
// Ident : $Id: TsAdpIceEx.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/SiteAdminSvc/SiteAdminSvc.ICE $
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 9     08-08-19 12:43 Hongquan.zhang
// 
// 8     08-04-21 15:50 Guan.han
// 
// 7     07-12-27 19:41 Hui.shao
// added resource restriction for virtual site
// 
// 6     07-12-13 19:43 Hui.shao
// 
// 5     07-12-10 18:51 Hui.shao
// 
// 4     07-12-10 18:47 Hui.shao
// moved event out of txn
// 
// 3     07-04-09 14:54 Hongquan.zhang
// 
// 2     07-03-28 18:35 Hui.shao
// 
// 1     07-03-15 19:02 Hui.shao
// ===========================================================================
// build steps:
// $(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. $(InputDir)\$(InputName).ice  
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --dict "TianShanIce::Site::SiteDict,string,TianShanIce::Site::VirtualSite" --dict-index "TianShanIce::Site::SiteDict,name" SiteDict $(InputDir)\$(InputName).ice  
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --dict "TianShanIce::Site::AppDict,string,TianShanIce::Site::AppInfo" --dict-index "TianShanIce::Site::AppDict,name" AppDict $(InputDir)\$(InputName).ice  
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::SiteToMount,TianShanIce::Site::AppMount,siteName,case-insensitive" SiteToMount $(InputDir)\$(InputName).ice  
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::AppToMount,TianShanIce::Site::AppMount,appName,case-insensitive" AppToMount $(InputDir)\$(InputName).ice  
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::SiteToTxn,TianShanIce::Site::LiveTxn,siteName,case-insensitive" SiteToTxn $(InputDir)\$(InputName).ice  
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::MountToTxn ,TianShanIce::Site::LiveTxn,mountedPath,case-insensitive" MountToTxn $(InputDir)\$(InputName).ice  
// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I.. --output-dir .. --index "TianShanIce::Site::TxnToEvent,TianShanIce::Site::TxnEvent,identTxn" TxnToEvent $(InputDir)\$(InputName).ice
// outputs:
// ..\$(InputName).h
// ..\$(InputName).cpp
// ..\SiteDict.cpp
// ..\SiteDict.h
// ..\AppDict.cpp
// ..\AppDict.h
// ..\SiteToMount.cpp
// ..\SiteToMount.h
// ..\AppToMount.cpp
// ..\AppToMount.h
// ..\MountToTxn.cpp
// ..\MountToTxn.h
// ..\SiteToTxn.cpp
// ..\SiteToTxn.h
// ..\TxnToEvent.cpp
// ..\TxnToEvent.h

#ifndef __ZQ_TianShanIce_SiteAdmin_ICE__
#define __ZQ_TianShanIce_SiteAdmin_ICE__

#include <TsSite.ICE>

#include <Ice/Identity.ice>

module TianShanIce
{

module Site
{

// -----------------------------
// class VirtualSite
// -----------------------------
/// describes a media site hosted in the system
struct VirtualSite
{
	string	name; ///< the name of site, unique key in the table, also is the hostname of the asset URI
	string  desc; ///< string description of the site
	
	long	maxDownstreamBwKbps; 	///< the per-site quota limitation on down stream bandwidth in Kbps, it will be used
					///< to validate the sum-up of the bandwidth of the sessions allocated for this virtual site
					///< this value should be read from the license files
	int     maxSessions; 		///< the per-site quota limitation on concurrent session count for this virtual site
					///< this value should be read from the license files
	long	currentLicExpiration;   ///< timestamp when the above maxDownstreamBwKbps and maxSessions gets expired

	string  privKey;		///< the private key for the site to decrypt license file
	string  pubKey;			///< the public key for licensing center to generate license files
	string  licenseFolder;		///< the directory where to save the license files

	SRM::ResourceMap restrictedResources;    ///< contains the per-site resource restriction on Storage, Streamer and Service Group	
	TianShanIce::Properties properties; ///< contains the per-site configurations	
};

/// a collection of site info
sequence <VirtualSite> VirtualSites;

// -----------------------------
// struct AppInfo
// -----------------------------
/// describes an application in the SRM
struct AppInfo
{
	string	name;		///< the name of application, unique key in the table
	string  endpoint;   	///< endpoint to access the application, will be formatted as "Application:<endpoint>"
	string  desc;		///< a brief decription of the application service
};

/// a collection of application info
sequence <AppInfo> AppInfos;

// -----------------------------
// class AppMount
// -----------------------------
/// describes a mount-age between site and applications
["freeze:write"]
class AppMount
{
	Ice::Identity ident;

	string	siteName;		///< key to the site information
	string  mountedPath;	///< the mounted path name of an application
	string  appName;		///< key to index the application
	string  desc;			///< a brief description of this mount-age
	TianShanIce::Properties properties; ///< contains the per-link configurations

	["cpp:const", "freeze:read"] string getMountedPath();
	["cpp:const", "freeze:read"] string getAppName();
	
	/// destroy the mountage
	void destroy();
};

/// a collection of accesses to mountages
sequence <AppMount* > AppMounts;

// -----------------------------
// struct TxnEvent
// -----------------------------
/// trace records within a transaction
["freeze:write"]
class TxnEvent
{
	Ice::Identity identTxn; // index, the Ident of LiveTxn

	string  stampUTC;		///< timestamp when the error is posted
	string  category;	///< the category of the error, normally is whom fires the error
	string  eventCode;	///< the sub-event code in the category
	string  eventMsg;	///< the event message

	// return informations about TxnEvent
	// params[in], specify the name of return parameters of TxnEvent, known parameters are:
	//	"stampUTC", 
	//	"category", 
	//	"eventCode", 
	//	"eventMsg", 
	["cpp:const", "freeze:read"] TianShanIce::Properties getEventInfo(TianShanIce::StrValues params);

	["cpp:const", "freeze:read"] void get(out Ice::Identity identTxn, out string stampUTC, out string category, out string eventCode, out string eventMsg);
};

// -----------------------------
// class LiveTxn
// -----------------------------
/// the live transaction link to the sessions for site quota management and audit
["freeze:write"]
class LiveTxn
{
	Ice::Identity ident;

	string sessId;			///< index, the Id of SRM::Session
	string siteName;        ///< index, the name of the virtual site
	string mountedPath;		///< index, the associated application name

	State  lastState;       ///< the last state of this transaction

	TianShanIce::SRM::Session*	SRMSess;	///< the associated SRM session
	/// The per-txn properties:
	TianShanIce::Properties properties; 

//	SubTxns sTxns; ///< history errors of this transaction

	///retrive the session id of the liveTxn
	["cpp:const", "freeze:read"] string getSessId();

	///retrive the site name attached to the liveTxn
	["cpp:const", "freeze:read"] string getSitename();

	///retrive the mounted application path of the liveTxn
	["cpp:const", "freeze:read"] string getPath();

	["cpp:const", "freeze:read"] TianShanIce::State getState();
	
	void setState(TianShanIce::State state);

	/// set a property
	void setProperty(string key, string value);

	/// get all of properties of the Txn
	["cpp:const", "freeze:read"] TianShanIce::Properties getProperties();

//	/// record an event on the transaction
//	["cpp:const", "freeze:read"] void trace(string category, string eventType, string eventMsg);

	/// destroy the live transaction, the transaction will be post to year-to-day database for auditing
	void destroy();

	
	//ontimer to check if session assoicated to current liveTxn is ok
	void	onTimer();
};

// -----------------------------
// Service SiteAdmin
// -----------------------------
/// extension of BusinessRouter to admin the hosted sites and applications
interface SiteAdmin extends BusinessRouter, TxnService
{
	/// list all the sites defined
	///@return a collection of definited sites
	["cpp:const", "freeze:read"] VirtualSites listSites()
	    throws ServerError, NotSupported;

	/// add or update a site definition
	///@param name	the name of the site about to update
	///@param desc	a description string of the hosted site
	///@return true if succeed
	bool updateSite(string name, string desc)
	    throws InvalidParameter, ServerError, NotSupported;

	/// remove a site definition
	///@param name	the name of the site about to delete
	///@return true if succeed
	///@note all the links to this site will also be destroyed
	bool removeSite(string name)
	    throws InvalidParameter, ServerError, NotSupported;

	/// add or update a site definition
	///@param name	the name of the site about to update
	///@param props	the new properties to apply onto the site
	///@return true if succeed
	bool setSiteProperties(string name, TianShanIce::Properties props)
	    throws InvalidParameter, ServerError, NotSupported;

	/// add or update a site definition
	///@param name	the name of the site about to update
	///@return		the properties of the site
	["cpp:const", "freeze:read"] TianShanIce::Properties getSiteProperties(string name)
	    throws InvalidParameter, ServerError, NotSupported;

	///update site's resource limited
	///@param maxBW max bandwidth can be used by the site
	///@param maxSessions max session count can be used by the site
	void updateSiteResourceLimited(string siteName, long maxBW, int maxSessions)
		throws InvalidParameter;

	///update site's resource restriction on storage, streamer and servicegroup
	///@param siteName the name of the virtual site
	///@param resources the resource map to restrict the site only uses those resources
	///@note only the restriction on resource type rtStorage, rtStreamer, rtServiceGroup will take effective for a virtual site
	void restrictSiteResources(string siteName, SRM::ResourceMap resources)
		throws InvalidParameter;

	///return site's resource restriction
	///@param siteName the name of the virtual site
	///@param resources the resource map to restrict the site only uses those resources
	///@return site's resource restriction
	///@sa restrictSiteResources
	["cpp:const", "freeze:read"] SRM::ResourceMap getSiteResourceRestricutions(string siteName)
		throws InvalidParameter;

	/// list all the applications
	///@return a collection of definited sites
	["cpp:const", "freeze:read"] AppInfos listApplications()
	    throws ServerError, NotSupported;

	/// add or update an application definition
	///@param name	the name of the application about to update
	///@param endpoint	the endpoint to access service TianShan::Application::AppService
	///@param desc	a description string of the application service
	///@return true if succeed
	bool updateApplication(string name, string endpoint, string desc)
	    throws InvalidParameter, ServerError, NotSupported;

	/// remove an application definition
	///@param name	the name of the application about to delete
	///@return true if succeed
	///@note all the links to this application will also be destroyed
	bool removeApplication(string name)
	    throws InvalidParameter, ServerError, NotSupported;

	/// list all the mount-age by a site
	///@param siteName	the name of the site
	///@return a collection of definited sites
	["cpp:const", "freeze:read"] AppMounts listMounts(string sitename)
	    throws InvalidParameter, ServerError, NotSupported;

	/// mount an application onto a site
	///@param[in] siteName	the name of the site about to be mounted with an application
	///@param[in] mountedPath	the pathname in the asset URI that refer to the mounted application
	///@param[in] appName	the name of the application about to mount
	///@return the access to the mount-age
	AppMount* mountApplication(string siteName, string mountedPath, string appName)
	    throws InvalidParameter, ServerError, NotSupported;

	/// unmount an application from a site
	///@param[in] siteName	the name of the site about to be mounted with an application
	///@param[in] mountedPath	the pathname in the asset URI that refer to the mounted application
	///@return true if succeed
	bool unmountApplication(string siteName, string mountedPath)
	    throws InvalidParameter, ServerError, NotSupported;

	/// find an application service endpoint by the site name and the mount path
	///@param siteName	the name of the site about to be mounted with an application
	///@param mountPath	the pathname in the asset URI that refer to the mounted application
	///@return the application info structure
	///@throw UnknownApplication if the input could not address a valid application info
	["cpp:const", "freeze:read"] AppInfo findApplication(string siteName, string mountPath)
	    throws InvalidParameter, ServerError, NotSupported;
};


};
};

#endif // __ZQ_TianShanIce_SiteAdmin_ICE__
