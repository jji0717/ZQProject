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
// Ident : $Id: DODApp.ice$
// Branch: $Name:  $
// Author: 
// Desc  : 
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataTunnel/ice/TsAppDOD.ICE $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     09-03-09 10:33 Li.huang
// ---------------------------------------------------------------------------

// 1) name to channel
// 2) name to destination
// 3) channel to dest

// build steps:

// $(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/Tianshan/DataTunnel/Ice/
//		--output-dir .. ..\$(InputName).ice

// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/Tianshan/DataTunnel/Ice \
//		--output-dir .. --dict "DataOnDemand::ChannelDict,string,DataOnDemand::ChannelPublishPoint" NameToChannel ..\$(InputName).ice

// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/Tianshan/DataTunnel/Ice \
//		--output-dir .. --dict "DataOnDemand::DestinationDict,string,DataOnDemand::Destination" NameToDest ..\$(InputName).ice

// outputs:

#ifndef __ZQ_ChannelOnDemand_DoDApp_ICE__
#define __ZQ_ChannelOnDemand_DoDApp_ICE__

#include "TsApplication.ICE"
module TianShanIce
{

// -----------------------------
// namespace Application
// -----------------------------
/// Application represents the basic entry definition of a business application to TianShan architecture
module Application
{

///@briedf DataPublishPoint represents an application under TianShanIce architecture.
/// The application allow people publish a channel avaible for on-demand ordering, and can accept real-time ingestion from live broast channel
module DataOnDemand
{
exception DataAppException {

};

exception StreamerException extends DataAppException {

};

/// streamer break exception
exception StreamerUnavailableException extends StreamerException {

};

/// cannot retrieve object on streamer
exception StreamerObjectUnavailableException extends StreamerException {

};

exception StreamInvalidState extends DataAppException {

};

exception ObjectExistException extends DataAppException {

};

// -----------------------------
// ChannelType
// -----------------------------
	const string dataSharedFolder  = "ShareFolder";  /// file share
	const string dataMessage       = "Message";      /// message
	const string dataLocalFolder    = "LocalFolder"; /// local file
// -----------------------------
// enum EncryptMode
// -----------------------------
/// encyprt mode
enum EncryptMode {
	dodEncryptNone,		     ///  do not encrypt
	dodEncryptNormal,	     ///  normal encrypt
	dodEncryptSimple,	     ///  simple encrypt
	dodEncryptReserve,	     /// keep
	dodEncryptCompress,      /// only Compress
	dodEncryptNormalCompress,/// normal Compress
	dodEncryptSimpleCompress,/// simple compress
};

// -----------------------------
// struct DataPublishPointInfo
// -----------------------------
/// DataPublishPointInfo infomation
struct DataPublishPointInfo {
	string			          name;
	int			              subchannelCount;	/// default 1
	int			              streamId;			/// PID
	TianShanIce::IValues	  dataTypes;
	int			              streamType;
	EncryptMode		          encrypt;
	int			              tag;
	int      		          withDestination;
};

sequence < DataPublishPointInfo > DataPublishPointInfos;
// -----------------------------
// class DataPublishPoint
// -----------------------------
/// DataPublishPoint
class DataPublishPoint extends ::TianShanIce::Application::PublishPoint
{	 
	/// get DataPublishPoint infomation
	/// @return	DataPublishPoint infomation	
	 DataPublishPointInfo getInfo();	
};

// -----------------------------
// class DataPublishPoint
// -----------------------------
class Folder extends DataPublishPoint
{
	void onFullUpdate(int groupId, string rootPath, bool clear, int verNumber);

	void onPartlyUpdate(int groupId, string rootPath, string paths,int verNumber);

	void onFolderDeleted(int groupId, string paths, int verNumber);

	void onFileDeleted(int groupId, string paths, int verNumber);

	void onFileModified(int groupId, string rootPath, string paths, int verNumber);

	void onFileAdded(int groupId, string rootPath, string paths, int verNumber);
};

struct Message
{
	string msgDest;
	string msgBody;
	long expiration;
};

class MessageQueue extends DataPublishPoint
{
	void onMessageAdded(int groupId, string messageId, Message msgInfo);
	
	void onMessageDeleted(int groupId, string messageId);
};

// -----------------------------
// struct DataStreamInfo
// -----------------------------
/// DataStreamInfo info
struct DataStreamInfo {
	string	name;
	int		pmtPid;			/// PMT pid which pack to Ts package
	int		totalBandwidth;	/// bandwidth of destination  in bps
	string	destAddress;	/// destination address, in the format of "<IP>:<port>[; <IP>:<port>]"
	int		groupId;	/// group id
};
sequence < DataStreamInfo > DataStreamInfos;

// -----------------------------
// struct DataAttachInfo
// -----------------------------
/// DataAttachInfo info
struct DataAttachInfo
{
	DataPublishPoint*	datapp;
	string              dataPublishPointName;
	int                 minBitRate;
	int                 repeatTime;
};
sequence < DataAttachInfo > DataAttachInfos;

// -----------------------------
// class DataStream
// -----------------------------
/// destination object of DOD stream
class DataStream extends ::TianShanIce::Application::BroadcastPublishPoint
{
	/// @return current stream state
	/// @return the state of the session bound in BroadcastPublishPoint
	 ::TianShanIce::State getState(); // { return session->getState(); }

	 //when the input dataPPName is empty, list all the bound dataPPs
	 DataAttachInfos listAttachments(string dataPPName);

	/// 将一个Channel 与 一个流相关联
	/// @param	ch			要关联的 channel
	/// @return 
	void attachDataPublishPoint(string dataPPName, DataAttachInfo attachInfo)
		throws ::TianShanIce::InvalidParameter, StreamerException, 
		ObjectExistException;

	/// 取消一个Channel 与 一个流的关联
	/// @param	chName		要取消关联的 channel 名称
	/// @return 
	void detachDataPublishPoint(string dataPPName)
		throws ::TianShanIce::InvalidParameter, StreamerException;	

	/// 暂停播放 Destination
	/// @param
	/// @return 
	void pause();
};

enum DataEvent {
	  onFullUpdate,
	  onPartlyUpdate,
	  onFolderDeleted,
	  onFileAdded,
      onFileModified,
	  onFileDeleted,
	  onMessageAdded,
      onMessageDeleted
};

// -----------------------------
// Service DataPointPublisher
// -----------------------------
/// The facet of the ChannelOnDemand service to external channel administration such as the management UI.
/// The interface is also used to publish a channel for OnDemand ordering
interface DataPointPublisher extends ::TianShanIce::Application::PointPublisher
{
	/// 创建本地文件系统Channel
	/// @param	name		发布点名称
	///	@param	props		提供给Channel 的基本属性
	///	@param	path		文件系统发布点数据源路径
	/// @return	返回创建的Channel。失败时返回空。
	Folder* createLocalFolderPublishPoint(string name, 
		DataPublishPointInfo info, string path, string desc) 
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// 创建 SMB 共享文件Channel
	/// @param	name		Channel名称
	///	@param	info		提供给Channel 的属性集
	/// @return	返回创建的Channel。失败时返回空。
	Folder* createShareFolderPublishPoint(string name, 
		DataPublishPointInfo info, string desc)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// 创建消息数据源的Channel
	/// @param name		Channel名称
	/// @param info		提供给Channel 的基本属性
	/// @return	返回创建的Channel。失败时返回空。
	MessageQueue* createMessageQueue(string name, DataPublishPointInfo info, string desc)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// 根据名称获得Channel
	/// @param	name		Channel名称
	/// @return 返回获得的Channel。失败时返回空。
	DataPublishPoint* openDataPublishPoint(string name)
		throws ::TianShanIce::InvalidParameter;

	/// 创建流
	/// @param	destName		destination name
	///	@param	info			基本属性集	
	/// @return	成功返回 Destination 对象，否则为 NULL
	DataStream* broadcast(string destName, SRM::ResourceMap resourceRequirement, TianShanIce::Properties props, string desc)
//		throws ::TianShanIce::InvalidParameter, StreamerException;
//	BcastPublishPoint* createBcastPublishPoint(string name, SRM::ResourceMap resourceRequirement, TianShanIce::Properties props, string desc)
	throws
		InvalidParameter,
		NotSupported,
		ServerError;

	
	/// 根据名称获得 DataStream
	/// @param	name		DataStream 名称
	/// @return 返回获得的DataStream 失败时返回空。
	DataStream* openDataStream(string name)
		throws ::TianShanIce::InvalidParameter;

	/// 列出存在的所有 DataPublish
	/// @return	返回所有存在的 DataPublish 
	DataPublishPointInfos listDataPublishPoints(string searchFor);

	/// 列出存在的所有 Datastream info
	/// @return	返回所有存在的 Datastream
	 DataStreamInfos listDataStreams(string searchFor);
	 
	 ["amd", "ami"] void OnDataEvent(DataEvent event, TianShanIce::Properties params);
};

// -----------------------------
// class DODPurchase
// -----------------------------
/// DOD 的 Purchase
["freeze:write"]
class DataTunnelPurchase extends ::TianShanIce::Application::Purchase {

};

// -----------------------------
// service DODAppService
// -----------------------------
/// DODApp 与 Weiwoo 的接口
interface DataTunnelService extends ::TianShanIce::Application::AppService {

};
// modules
};
};
};

#endif // __ZQ_TianShanIce_AppDOD_ICE__




