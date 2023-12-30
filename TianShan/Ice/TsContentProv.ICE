// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: TsStorage.ICE $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Ice/TsContentProv.ICE $
// 
// 1     10-11-12 16:06 Admin
// Created.
// 
// 1     10-11-12 15:39 Admin
// Created.
// 
// 19    10-10-28 13:45 Fei.huang
// * merge from 1.10
// 
// 18    09-07-24 14:57 Xia.chen
// change the second parameter from contentName to contentKey for
// getExportUrl
// 
// 17    08-11-15 18:03 Jie.zhang
// add trick speed collection
// 
// 16    08-11-03 11:25 Jie.zhang
// merge from 1.7.8 about getExportUrl
// 
// 15    08-08-14 14:36 Hui.shao
// merged from 1.7.10
// 
// 14    08-07-07 15:40 Hui.shao
// 
// 13    08-06-20 12:32 Jie.zhang
// add listRegisteredCPE
// 
// 12    08-04-23 11:56 Hui.shao
// 
// 11    08-04-09 15:37 Hui.shao
// 
// 10    08-04-03 11:01 Hui.shao
// 
// 9     08-04-02 15:53 Hui.shao
// 
// 8     08-04-02 12:05 Hui.shao
// 
// 7     08-03-27 17:06 Jie.zhang
// 
// 7     08-03-17 20:07 Jie.zhang
// 
// 6     08-02-21 15:09 Hui.shao
// added paged list
// 
// 5     08-02-18 21:00 Hui.shao
// replaced priviate data with property
// 
// 4     08-02-18 18:25 Hui.shao
// added provision::cancel and list provision
// 
// 3     08-02-15 12:05 Hui.shao
// 
// 2     08-02-14 16:37 Hui.shao
// 
// 1     08-02-13 17:51 Hui.shao
// initial check in
// ===========================================================================
// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice $(InputName).ice

#ifndef __ZQ_TianShanIce_ContProv_ICE__
#define __ZQ_TianShanIce_ContProv_ICE__

#include "TianShanIce.ICE"
#include "TsEvents.ICE"
#include "TsStorage.ICE"
#include "TsSRM.ICE"
#include <Ice/Identity.ice>

module TianShanIce
{

// -----------------------------
// namespace ContentProvision
// -----------------------------
/// ContentProvision represents a media content provisioning within the TianShan architecture
module ContentProvision
{

/// The ProvisionState represents the status of a content provision session in CPE
enum ProvisionState
{
	cpsCreated,	 ///< the provision session was newly created
	cpsAccepted, ///< the provision session has been validated, and the resources have been reserved for the
	             ///< scheduled time window. The CPE will include the resource allocation when others book
				 ///< new provision. NO CPHSess will be created and linked to this session
	cpsWait,     ///< After ProvisionSession::commit() is called, the ProvisionSession enter this state, and
				 ///< wait for a near scheduled startTime arrive
	cpsReady,     ///< when the scheduled startTime is about met, the provision session will be resolved with a CPHSess,
				 ///< and perform the initialzation of CPHSess. After CPHSess is initialized, the provision session
				 ///< waits for the scheduled startTime, resources are acturally employed in this state.
	cpsProvisioning, ///<  when the scheduled startTime has been met, the provision is under processing from cpsReady
	cpsStopped   ///<  the provision is either completed, terminated or failed, and waiting to cleanup. The
	             ///< will automatically timeout and destroyed by CPE if it is at this state
};

/// The ProvisionState represents the type of a content provision session in CPE
enum ProvisionType
{
	ptUnknown,	///< the provision session type has not been determined by CPH
	ptCatcher,	///< the provision session is a catcher session
	ptPitcher,	///< the provision session is a pitcher session
	ptPassThru, ///< the provision session is a pass-thru session
};

/// The ProvisionState represents the status of a content provision session in CPE
enum StartType
{
	stScheduled,	///< The ProvisionSession will be started by CPE core based on the scheduled start time
					///< If the session is restored from safestore with a cpsProvisioning state, the
					///< session will be cleanup immediately
	stScheduledRestorable,	///< The ProvisionSession will be started by CPE core based on the scheduled start time
					///< If the session is restored from safestore with a cpsProvisioning state, the
					///< session will be committed again and restared. This is normally reserved for a propagation
					///< or offline content import execution
	stPushTrigger,	///< the provision session enters cpsProvisioning or can be started only based on the push event
					///< when the user starts feeding data, the setting of scheduledStart is only the reference for
					///< allocation and session validation. This session is non-restorable from a state cpsProvisioning
					///< but can be forced to start from the state cpsCreated, cpsAccepted, cpsWait, cpsReady
};

/// The ProvisionState represents the status of a content provision session in ContentProvisionEngine (CPE)
enum ProvisionOwnerType
{
	potCluster,	     ///< if the provision session is demanded by the ContentProvisionCluster (CPC),
	potStreamService, ///< if the provision session is demanded by the local stream service
	potDirect,	     ///< if the provision session is demanded thru other methods
};

struct ProvisionContentKey
{
	string contentStoreNetId;
	string volume;
	string content;
};

/// the mask is a 8 bit unsigned byte works as flags to filter the possible events fired by a session within a
/// helper during the provisioning
struct ProvisionSubscribeMask
{
	byte psmInProgress; ///< the emInProgress mask allow to specify 4 levels to subscribe the progress events
						///< @li 0 - no InProgress events should be notified
						///< @li 1 - only major progress events should be notified such as the progress no smaller than 10%
						///< @li 2 - the default inProgress notifications up to the helper's considerations
						///< @li 3 - the progress as detail as each buffer allocation
	bool psmStateChanged;	///< true if wish to be notified when a state change occurs, will always be on if it is owned by a CPC
	bool psmDestroy;		///< true if wish to be notified when a provison session is detroyed
	bool psmStreamable;		///< true if wish to be notified when a provision that support PWE becomes streamable
	bool psmStarted;		///< true if wish to be notified when a provision session is started
	bool psmStopped;		///< true if wish to be notified when a provision session is stopped
};

// -----------------------------
// exception ProvisionSinkNotFound
// -----------------------------
/// exception thrown when an refer-ed entity doesn't not exist
exception ProvisionOwnerNotFound extends EntityNotFound
{
	ProvisionContentKey content;
};

// -----------------------------
// callback ProvisionSessionBind
// -----------------------------
///@brief represents the callback class to the contentstore
/// By subscribe with this callback, client will be noticed with the content provision events. ProvisionTask, the entity on
/// CPC, inherints from this callback interface
interface ProvisionSessionBind
{
	/// event will be fired when a content's provision status is changed
	///@param[in] content the content combination key to identify a provision
	///@param[in] timeStamp the timestamp this event occurs because the callback may be invoked async-ly
	///@param[in] prevState the previous state of the provisioning
	///@param[in] currentState the new state of the provisioning
	///@param[in] params the additional parameters of this event
	///@throw ProvisionOwnerNotFound when the owner has gone, the CPE should reset ProvisionSession::owner if this is caught
	["ami"] void OnProvisionStateChanged(ProvisionContentKey content, long timeStamp, ProvisionState prevState, ProvisionState currentState, TianShanIce::Properties params)
	throws ProvisionOwnerNotFound;

	/// event will be fired when a content provision is processing with progress
	///@param[in] content the content combination key to identify a provision
	///@param[in] timeStamp the timestamp this event occurs because the callback may be invoked async-ly
	///@param[in] processed the processed provision units in the current step
	///@param[in] total the total provision units in the current step. The unit can percentage, or file size in KB, etc
	///@param[in] params the additional parameters of this event
	///@note progress (0, -1) is defined as an event of "process started"
	///@throw ProvisionOwnerNotFound when the owner has gone, the CPE should reset ProvisionSession::owner if this is caught
	["ami"] void OnProvisionProgress(ProvisionContentKey content, long timeStamp, long processed, long total, TianShanIce::Properties params)
	throws ProvisionOwnerNotFound;

	/// event will be fired when a content provision is started
	///@param[in] content the content combination key to identify a provision
	///@param[in] timeStamp the timestamp this event occurs because the callback may be invoked async-ly
	///@param[in] params the additional parameters of this event
	///@throw ProvisionOwnerNotFound when the owner has gone, the CPE should reset ProvisionSession::owner if this is caught
	["ami"] void OnProvisionStarted(ProvisionContentKey content, long timeStamp, TianShanIce::Properties params)
	throws ProvisionOwnerNotFound;

	/// event will be fired when a content provision is stopped
	///@param[in] content the content combination key to identify a provision
	///@param[in] timeStamp the timestamp this event occurs because the callback may be invoked async-ly
	///@param[in] errorOccurred true means the provision failed
	///@param[in] params the additional parameters of this event
	///@throw ProvisionOwnerNotFound when the owner has gone, the CPE should reset ProvisionSession::owner if this is caught
	["ami"] void OnProvisionStopped(ProvisionContentKey content, long timeStamp, bool errorOccurred, TianShanIce::Properties params)
	throws ProvisionOwnerNotFound;

	/// event will be fired when a content's provision become streamable
	///@param[in] content the content combination key to identify a provision
	///@param[in] timeStamp the timestamp this event occurs because the callback may be invoked async-ly
	///@param[in] streamable true if the provision supports PWE and the content become streamable
	///@param[in] params the additional parameters of this event
	///@throw ProvisionOwnerNotFound when the owner has gone, the CPE should reset ProvisionSession::owner if this is caught
	["ami"] void OnProvisionStreamable(ProvisionContentKey content, long timeStamp, bool streamable, TianShanIce::Properties params)
	throws ProvisionOwnerNotFound;

	/// event will be fired when a content provision session is destroyed
	///@param[in] content the content combination key to identify a provision
	///@param[in] timeStamp the timestamp this event occurs because the callback may be invoked async-ly
	///@param[in] params the additional parameters of this event
	///@throw ProvisionOwnerNotFound when the owner has gone, the CPE should reset ProvisionSession::owner if this is caught
	["ami"] void OnProvisionDestroyed(ProvisionContentKey content, long timeStamp, TianShanIce::Properties params)
	throws ProvisionOwnerNotFound;

	//TODO OnError
};

// -----------------------------
// exception OutOfResource
// -----------------------------
/// exception thrown when run out of resource to setup a provision
exception OutOfResource extends ServerError
{
};

/// a collection of trick speeds
sequence < float > TrickSpeedCollection;

// -----------------------------
// class ProvisionSession
// -----------------------------
/// the entity object managed in CPE as the context of a provisioning
["freeze:write"] 
class ProvisionSession
{
	ProvisionState state;
	ProvisionContentKey contentKey;     ///< an index to search by the primary content @sa primaryContent
	ProvisionType  provType;
	ProvisionOwnerType ownerType;
	StartType stType;

	Storage::Content* primaryContent;   ///< the primary content associated with this provision session
	                                    ///< @li the destination content if provisionType=ptCatcher
										///< @li the source content if provisionType=ptPitcher
	Storage::Content* secondaryContent; ///< reserved for future use

	string methodType;                  ///< the provision method type to associate content provision helper

	ProvisionSessionBind* owner;    ///< the callback interface to notice, this should be casted to the
	                                ///< ProvisionTask instance on ContentProvsionCluster (CPC) if ownerType=potCluster
	ProvisionSubscribeMask subMask;

	long scheduledStart;         ///< the scheduled start time, 0 if start as soon as possible
	long scheduledEnd;           ///< the scheduled end time, INFINITE if execute until finish
	SRM::ResourceMap		resources;
	TianShanIce::Properties	props;
	bool streamable;
	
	TrickSpeedCollection	trickSpeeds;		///< the trick speed collection

	["cpp:const", "freeze:read"] ProvisionState getState();
	["cpp:const", "freeze:read"] ProvisionContentKey getContentKey();
	["cpp:const", "freeze:read"] ProvisionType getProvisionType();
	["cpp:const", "freeze:read"] string getMethodType();
	["cpp:const", "freeze:read"] ProvisionSubscribeMask getSubscribeMask();

	void setSubscribeMask(ProvisionSubscribeMask mask);

	/// set the needed trick speeds of the content, default only one trick speed 7.5
	///@param[in]	speeds		a collection of supported trick speeds. i.e. <7.5,15,22.5,30>
	///@throws 1) NotImplemented: not implemented \n
	void setTrickSpeedCollection(TrickSpeedCollection speeds)
		throws NotImplemented;

	/// setProperty allows an object to update the property of the provision session.
	///@param key the keyname of the property
	///@param val the value of the property
	void setProperty(string key, string val)
	throws ServerError, InvalidParameter, InvalidStateOfArt;

	/// getProperties allows to get all the properties of a provision session
	///@return the properties of the session
	["cpp:const", "freeze:read"] TianShanIce::Properties getProperties();

	/// addResource
    /// The addResource operation allows the guts of a Resource to be sent to a Session. 
	/// The Session then instantiates a Resource object and adds it to its list. When the 
	/// negotiateResources operation is invoked, these will be the Resources that are negotiated.
    /// Each Resource is assigned a unique number by the Session.
	///@param type the enum value of ResouceType to identify which resource to be added
	///@param res the prepared Resource structure
	///@throw InvalidStateOfArt addResource is only allowed when the state is cpsCreated
	///@throw InvalidResource if the given resource is not acceptable in provision session
	///@note comparing with CPC and SRM, all the resources applied to CPE are raMandatoryNonNegotiable
    void addResource(SRM::ResourceType type, TianShanIce::ValueMap resData)
	throws
      SRM::InvalidResource,
	  NotSupported,
      InvalidStateOfArt;

	/// get all the resource collection bound in the session
	///@return the resource collection
	["cpp:const", "freeze:read"] SRM::ResourceMap getResources();

	/// get the scheduled time
	///@param[out] startTimeUTC the scheduled time to start, in the format of ISO-8601
	///@param[out] endTimeUTC the scheduled time to start, in the format of ISO-8601
	["cpp:const", "freeze:read"] void getScheduledTime(out string startTimeUTC, out string endTimeUTC);

	/// get the scheduled time
	///@param[in] startTimeUTC the new scheduled time to start, in the format of ISO-8601
	///              empty if no update on start is needed
	///              current time or earier to start as soon as possible
	///@param[in] endTimeUTC the new scheduled time to end, in the format of ISO-8601
	///              empty if no update on end is needed
	///              current time or earier will raise an exception InvalidParameter
	void updateScheduledTime(string startTimeUTC, string endTimeUTC)
	throws
	  InvalidParameter,
	  NotSupported,
      InvalidStateOfArt;

	/// event will be fired when a content provision is processing with progress
	///@param[out] processed the processed provision units in the current step
	///@param[out] total the total provision units in the current step. The unit can percentage, or file size in KB, etc
	///@throw ProvisionOwnerNotFound when the owner has gone, the CPE should reset ProvisionSession::owner if this is caught
	///@throw InvalidStateOfArt queryProgress is not allowed when the state is cpsCreated or cpsAccepted
	["cpp:const", "freeze:read"] void queryProgress(out long processed, out long total)
	throws
	  NotSupported,
      InvalidStateOfArt;

    /// setup a provision from state cpsCreated. At this step, the session will be associated with a proper
	/// CPHelper, and the CPHelper will verify the resources collected if they meet the necessary needs to
	/// start a session. The CPE will verify the resource availiability by schedule. Be aware that the CPHSess
	/// object MAY not actually created at this step
	/// a purchase of application, if necessary resouce requirement or private data have not been
	/// collected before provision() is invoked, this operation will be rejected
	///@param[in] startTimeUTC the new scheduled time to start, in the format of ISO-8601
	///                     empty if no update on start is needed
	///                     current time or earier to start as soon as possible
	///                     sometime one year later or any other negative will raise an exception InvalidParameter
	///@param[in] endTimeUTC the new scheduled time to end, in the format of ISO-8601
	///                      empty if no update on end is needed
	///                      current time or earier will raise an exception InvalidParameter
	///@note the destination state of the session will be turned to cpsAccepted if succeeded
	///@throw InvalidStateOfArt if try to setup from a illeagal state
	///@throw InvalidResource if a required resource has not been collected piror to this operation
	///@throw InvalidParameter if a required parameter has not been collected piror to this operation
	///@throw InvalidStateOfArt if the current state of session does not perform this operation
    ["amd"] void setup(string startTimeUTC, string endTimeUTC)
	throws
      SRM::InvalidResource,
	  InvalidStateOfArt,
      InvalidParameter,
      OutOfResource,
      NotSupported,
	  ServerError;

    /// perform the provision for the session. At this step, the session will be associated with
	/// a purchase of application, if necessary resouce requirement or private data have not been
	/// collected before provision() is invoked, this operation will be rejected
	///@note resource rtURI is a must to associate the purchase, all other necessary resource or
	///      runtime parameters are up to the application logic
	///@note the destination state of the session will be turned to cpsWait if succeeded
	///@throw InvalidStateOfArt if the current state of session does not perform this operation
    void commit()
	throws
	  InvalidStateOfArt,
      NotSupported,
	  ServerError;

    /// cancel a hosted provison. the provision will be terminated and cleaned up even if it has been started
	///@param[in] endTimeUTC the new scheduled time to end, in the format of ISO-8601
	///@note no status update will be applied onto the related content if the provision is completed
	///@throw InvalidStateOfArt if the current state of provision has already been cpsStopped, there is no way
	///       to cancel the stop processing that has already started or performed
    void cancel(int clientErrorCode, string reason)
	throws
	  InvalidStateOfArt,
      NotSupported,
	  ServerError;

    /// query if the content becomes streamble even if it is an on-going provision
	///@return true if the content is streamable regardless provision state
	///@note non-nonmutating API since the CPE may update the flag while this invocation
    bool isStreamable();
};

// -----------------------------
// struct ProvisionInfo
// -----------------------------
/// The provision session information exported from the ContentProvisionService
struct ProvisionInfo
{
	ProvisionContentKey contentKey;
	ProvisionState state;
	TianShanIce::Properties params;
};

/// a collection of ProvisionInfo
sequence <ProvisionInfo> ProvisionInfos;

// -----------------------------
// struct MethodInfo
// -----------------------------
/// The method information exported from the ContentProvisionService
struct MethodInfo
{
	string methodType;		///< the method type
	string vendor;			///< the vendor of this method
	string versionInfo;		///< the version of the implementation of this method

	long allocatedKbps;		///< the current allocated bandwidth of this method, in Kbps
	long maxKbps;			///< the max bandwidth, it may give a reference about the current load of this method, in Kbps
	long sessions;			///< the current allocated session instances of this method
	long maxsessions;		///< the current session instances that the CPE can host
};

/// a collection of MethodInfo
sequence <MethodInfo> MethodInfos;

struct ExportMethod
{
	string	protocal;		///< the protocal for export, ftp,http,etc
	string	rootUrl;		///< the root url of the specified protocal
	
	long allocatedBwKbps;	///< the current allocated bandwidth of this method, in Kbps
	long maxBwKbps;			///< the maximal bandwidth in bps that this protocal can accept
	long liveSessions;		///< the current allocated session instances of this method
	long maxSessions;		///< the current session instances that the CPE can host

};

/// a collection of ExportMethod
sequence <ExportMethod> ExportMethods;

// -----------------------------
// Service ContentProvisionService
// -----------------------------
/// represents the service interface of ContentProvisionEngine (CPE)
interface ContentProvisionService extends TianShanIce::BaseService
{
	/// get a network-wide unique id of this ContentProvisionService instance,
	/// normally this value could be read from the configuration of the ContentProvisionService
	///@return a network-wide unique id
	///@note no white space is allowed in the id string
	["cpp:const"] idempotent string getNetId();

	/// create a ProvisionSession with a specific initial parameters
	///@param[in] contentKey the content key to identify the provision session
	///@param[in] type	specify the type of the ProvisionSession to create
	///@param[in] methodType specify the method type of this provision.
	///             some CPE implementations may be able to automatically associate a proper method if this is empty
	///@param[in] ownerType	specify the owner type of the ProvisionSession to create
	///@param[in] primaryContent a reference access about the content that is being provisioned,
	///           may NULL if the caller wish not to specify
	///@param[in] owner	access to the owner who orders ProvisionSession. It may NULL if no one cares about the events of provision.
	///           if ownerType = potCluster, it will be a must and the owner can be casted to ProvisionTask on CPC
	///@return    a ProvisionSession instance, NULL if failed
	///@note	  resouce requirement must be specified by calling Session::addResource()
	ProvisionSession* createSession(ProvisionContentKey contentKey, string methodType, ProvisionOwnerType ownerType,
									Storage::Content* primaryContent, ProvisionSessionBind* owner)
	throws
		ServerError,
		InvalidParameter;

	/// open an existing ProvisionSession with a session id
	///@param[in] contentKey	the content key to find the ProvisionSession
	///@return    a ProvisionSession instance, NULL if failed
	ProvisionSession* openSession(ProvisionContentKey contentKey);

	/// list all the supported provision methods
	///@return a collection of MethodInfo
	///@throw ServerError
	["cpp:const", "amd"]
	MethodInfos listMethods()
		throws ServerError;


	/// list all the supported export methods
	///@return a collection of ExportMethod
	///@throw ServerError
	["cpp:const", "freeze:read"]
	ExportMethods listExportMethods()
		throws ServerError;


	/// get the content's url
	///@param[in]	transferProtocol the expected transfer protocol to export the content, if ContentStore does not support the
	///                              expected transfer protocol, InvalidStateOfArt exception will throw. 
	///@param[in]	content	the content name to export
	///@param[in]	transferBitrate  transfer bitrate the expected transfer protocol to export the content, if ContentStore does not support the
	///@param[out]	ttl		     time to live for this export url, if url expired, exporting is not allowed
	///@param[out]	permittedBitrate	if could not provide the specified transfer bitrate, then return the permitted bitrate, else permittedBitrate=transferBitrate
	///return    The url of the provisioned content in storage. This URL contains the user name and password as well. 
	///@throws 1) NotImplemented: not implemented \n
	///		   2) InvalidStateOfArt: the content is not ready in the storage current\n
	///                            Possible error code: csexpCodeUnsupportProto(103)
	string getExportURL(string transferProtocol, ProvisionContentKey contentKey, int transferBitrate, out int ttl, out int permittedBitrate) 
		throws InvalidStateOfArt, NotImplemented;


	/// list all the hosted provision sessions
	///@param[in] methodType specify a methodType to list the sessions; empty or "*" will list all regardless methodType
	///@param[in] paramNames specify the name of returning parameters of each provision
	///@param[in] startId  the start provision session Id. all the session Id listed in the result will be no smaller than it.
	//                     leave it empty if wish to list from the beginning
	///@param[in] maxCount the max count of the txns in the list result. 0 - means no limitation
	///@return a collection of ProvisionInfo
	///@throw ServerError
	["cpp:const", "amd", "ami"]
	ProvisionInfos listSessions(string methodType, TianShanIce::StrValues paramNames, string startId, int maxCount)
		throws ServerError;

};

/// a collection of ProvisionSession
sequence <ProvisionSession* > ProvisionSessions;

// -----------------------------
// class ProvisionTask
// -----------------------------
///@brief represents the entity on ContentProvsionCluster (CPC) bound to a provision session on ContentProvsionEngine (CPE)
class ProvisionTask implements ProvisionSessionBind
{
	Ice::Identity ident;

	TianShanIce::State state;
	ProvisionSessions  provisionsBound;     ///< a collection of provision sessions bound on this task, the first item
											///< of the collection is treated as the major provision session.

	/// addResource
    /// The addResource operation allows the guts of a Resource to be sent to a Session. 
	/// The Session then instantiates a Resource object and adds it to its list. When the 
	/// negotiateResources operation is invoked, these will be the Resources that are negotiated.
    /// Each Resource is assigned a unique number by the Session.
	///@param type the enum value of ResouceType to identify which resource to be added
	///@param res the prepared Resource structure
	///@throw InvalidStateOfArt addResource is only allowed when the state is cpsCreated
	///@throw InvalidResource if the given resource is not acceptable in provision session
	///@note comparing with CPC and SRM, all the resources applied to CPE are raMandatoryNonNegotiable
    void addResource(SRM::ResourceType type, TianShanIce::ValueMap resData)
	throws
      SRM::InvalidResource,
	  NotSupported,
      InvalidStateOfArt;

	/// get all the resource collection bound in the session
	///@return the resource collection
	["cpp:const"] SRM::ResourceMap getResources();

	void resolveProvision(bool desertLifecyleToProvsion)
	   throws 
	      NotSupported,
		  ServerError,
		  SRM::InvalidResource;
	
	["cpp:const"] ProvisionSessions getProvsionAttached()
	   throws 
	      InvalidStateOfArt,
		  ServerError;

	void destroy();
};

struct TaskInfo
{
	Ice::Identity ident;

	TianShanIce::State state;
	TianShanIce::Properties params;
};

/// a collection of TaskInfos
sequence <TaskInfo> TaskInfos;


/// a collection of ContentProvisionService
sequence <ContentProvisionService*> CPEInsts;

// -----------------------------
// Service ContentProvisionCluster
// -----------------------------
/// represents the service interface of ContentProvisionCluster (CPC)
interface ContentProvisionCluster extends TianShanIce::BaseService
{
	/// report an active engine from CPE, The ContentProvisionCluster may update its record about this CPE instance
	/// if it finds its recorded stampLastChange has been out of date 
	///@param netId the network id of CPE to identify the instance of CPE, this should be network-wide unique
	///@param CPE access to the CPE
	///@param stampLastChange indicate the last occured on the CPE, CPC may query if it find its data is odl
	///@return the timeout in sec for next report from CPE. CPC may treat it is offline if the CPE failed to report again in time
	["amd"] int reportEngine(string netId, ContentProvisionService* CPE, long stampLastChange);


	/// list all CPE services registered
	///@return a collection of ContentProvisionService
	///@throw ServerError
	["cpp:const"] CPEInsts listRegisteredCPE();
	
	
	/// open or create a provision task
	///@param[in] id the unique id of the task, it is suggested to use GUID if try to create a new task
	///@param[in] createIfNotExist true if to create a new task if no matched
	///@return pointer to the ProvisionTask object
	///@throws 1) NotImplemented: not implemented \n
	///		2) InvalidParameter: not exists while createIfNotExist=false \n
    ProvisionTask* openTask(string id, bool createIfNotExist)
	throws
		InvalidParameter,
		NotImplemented;
};

};
};

#endif // __ZQ_TianShanIce_ContProv_ICE__
