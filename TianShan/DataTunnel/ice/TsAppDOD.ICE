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

	/// ��һ��Channel �� һ���������
	/// @param	ch			Ҫ������ channel
	/// @return 
	void attachDataPublishPoint(string dataPPName, DataAttachInfo attachInfo)
		throws ::TianShanIce::InvalidParameter, StreamerException, 
		ObjectExistException;

	/// ȡ��һ��Channel �� һ�����Ĺ���
	/// @param	chName		Ҫȡ�������� channel ����
	/// @return 
	void detachDataPublishPoint(string dataPPName)
		throws ::TianShanIce::InvalidParameter, StreamerException;	

	/// ��ͣ���� Destination
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
	/// ���������ļ�ϵͳChannel
	/// @param	name		����������
	///	@param	props		�ṩ��Channel �Ļ�������
	///	@param	path		�ļ�ϵͳ����������Դ·��
	/// @return	���ش�����Channel��ʧ��ʱ���ؿա�
	Folder* createLocalFolderPublishPoint(string name, 
		DataPublishPointInfo info, string path, string desc) 
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// ���� SMB �����ļ�Channel
	/// @param	name		Channel����
	///	@param	info		�ṩ��Channel �����Լ�
	/// @return	���ش�����Channel��ʧ��ʱ���ؿա�
	Folder* createShareFolderPublishPoint(string name, 
		DataPublishPointInfo info, string desc)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// ������Ϣ����Դ��Channel
	/// @param name		Channel����
	/// @param info		�ṩ��Channel �Ļ�������
	/// @return	���ش�����Channel��ʧ��ʱ���ؿա�
	MessageQueue* createMessageQueue(string name, DataPublishPointInfo info, string desc)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// �������ƻ��Channel
	/// @param	name		Channel����
	/// @return ���ػ�õ�Channel��ʧ��ʱ���ؿա�
	DataPublishPoint* openDataPublishPoint(string name)
		throws ::TianShanIce::InvalidParameter;

	/// ������
	/// @param	destName		destination name
	///	@param	info			�������Լ�	
	/// @return	�ɹ����� Destination ���󣬷���Ϊ NULL
	DataStream* broadcast(string destName, SRM::ResourceMap resourceRequirement, TianShanIce::Properties props, string desc)
//		throws ::TianShanIce::InvalidParameter, StreamerException;
//	BcastPublishPoint* createBcastPublishPoint(string name, SRM::ResourceMap resourceRequirement, TianShanIce::Properties props, string desc)
	throws
		InvalidParameter,
		NotSupported,
		ServerError;

	
	/// �������ƻ�� DataStream
	/// @param	name		DataStream ����
	/// @return ���ػ�õ�DataStream ʧ��ʱ���ؿա�
	DataStream* openDataStream(string name)
		throws ::TianShanIce::InvalidParameter;

	/// �г����ڵ����� DataPublish
	/// @return	�������д��ڵ� DataPublish 
	DataPublishPointInfos listDataPublishPoints(string searchFor);

	/// �г����ڵ����� Datastream info
	/// @return	�������д��ڵ� Datastream
	 DataStreamInfos listDataStreams(string searchFor);
	 
	 ["amd", "ami"] void OnDataEvent(DataEvent event, TianShanIce::Properties params);
};

// -----------------------------
// class DODPurchase
// -----------------------------
/// DOD �� Purchase
["freeze:write"]
class DataTunnelPurchase extends ::TianShanIce::Application::Purchase {

};

// -----------------------------
// service DODAppService
// -----------------------------
/// DODApp �� Weiwoo �Ľӿ�
interface DataTunnelService extends ::TianShanIce::Application::AppService {

};
// modules
};
};
};

#endif // __ZQ_TianShanIce_AppDOD_ICE__



