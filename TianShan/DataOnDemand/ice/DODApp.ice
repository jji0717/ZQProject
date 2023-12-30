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
// Author: Xiao Tao
// Desc  : 
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataOnDemand/ice/DODApp.ice $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 2     08-12-09 17:24 Li.huang
// 
// 1     08-12-08 11:10 Li.huang
// 
// 25    08-10-29 15:27 Li.huang
// 
// 24    07-11-28 11:10 Li.huang
// 
// 23    07-04-10 12:34 Li.huang
// 
// 22    07-03-19 17:05 Li.huang
// 
// 21    07-03-06 17:21 Li.huang
// 
// 20    07-01-26 13:32 Cary.xiao
// 
// 19    07-01-26 11:34 Li.huang
// 
// 18    07-01-25 14:28 Cary.xiao
// 
// 16    07-01-22 14:00 Cary.xiao
// 
// 15    07-01-20 12:15 Cary.xiao
// 
// 14    07-01-16 13:41 Cary.xiao
// 
// 13    07-01-10 17:29 Cary.xiao
// 
// 12    07-01-03 16:47 Cary.xiao
// 
// 11    07-01-03 12:08 Li.huang
// 
// 10    06-12-21 10:37 Li.huang
// 
// 9     06-12-20 17:36 Li.huang
// 
// 8     06-12-20 17:09 Cary.xiao
// 
// 7     06-12-20 17:08 Cary.xiao
// 
// 6     06-12-12 17:40 Cary.xiao
// 
// 5     06-12-11 16:09 Cary.xiao
// 
// 4     06-12-08 15:24 Cary.xiao
// 
// 3     06-12-08 15:17 Cary.xiao
// 
// 2     06-12-07 18:52 Hui.shao
// review and refine
// ---------------------------------------------------------------------------

// 1) name to channel
// 2) name to destination
// 3) channel to dest


// build steps:

// $(ICE_ROOT)\bin\slice2cpp.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/DataOnDemand/Ice \
//		--output-dir .. ..\$(InputName).ice

// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/DataOnDemand/Ice \
//		--output-dir .. --dict "DataOnDemand::ChannelDict,string,DataOnDemand::ChannelPublishPoint" NameToChannel ..\$(InputName).ice

// $(ICE_ROOT)\bin\slice2freeze.exe -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/DataOnDemand/Ice \
//		--output-dir .. --dict "DataOnDemand::DestinationDict,string,DataOnDemand::Destination" NameToDest ..\$(InputName).ice

// outputs:

#ifndef __ZQ_ChannelOnDemand_DoDApp_ICE__
#define __ZQ_ChannelOnDemand_DoDApp_ICE__

#include "TsApplication.ICE"

/// namespace of DataOnDemand
module DataOnDemand {

// -----------------------------
// enum ChannelType
// -----------------------------
/// ChannelPublishPoint 类型
enum ChannelType {
	dodSharedFolder,	/// SMB 文件共享
	dodMessage,			/// 消息
	dodLocalFolder, 	/// 本地文件
};

// -----------------------------
// enum EncryptMode
// -----------------------------
/// 加密模式
enum EncryptMode {
	dodEncryptNone,		/// 不加密
	dodEncryptNormal,	/// 普通加密
	dodEncryptSimple,	/// 简单加密
	dodEncryptReserve,	/// 保留
	dodEncryptCompress, /// 只压缩
	dodEncryptNormalCompress, 
	dodEncryptSimpleCompress, 	
};

// -----------------------------
// struct ChannelInfo
// -----------------------------
/// ChannelPublishPoint 的基本信息
struct ChannelInfo {
	string			name;
	int				subchannelCount;	/// default 1
	int				streamId;			/// PID
	TianShanIce::IValues	dataTypes;
	int				streamType;
	EncryptMode		encrypt;
	int				tag;
	int      		withDestination;
};

exception DODAppException {

};

exception StreamerException extends DODAppException {

};

/// streamer break exception
exception StreamerUnavailableException extends StreamerException {

};

/// cannot retrieve object on streamer
exception StreamerObjectUnavailableException extends StreamerException {

};

exception StreamInvalidState extends DODAppException {

};

exception ObjectExistException extends DODAppException {

};

// -----------------------------
// class ChannelPublishPoint
// -----------------------------
/// ChannelPublishPoint
class ChannelPublishPoint {

	/// @return ChannelPublishPoint 名
	nonmutating string getName();

	/// 返回 ChannelPublishPoint 的类型 local folder / share folder / message
	/// @return	ChannelPublishPoint 的类型
	nonmutating ChannelType getType();

	/// 得到 ChannelPublishPoint 的基本信息
	/// @return	ChannelPublishPoint 的基本信息	
	nonmutating ChannelInfo getInfo();

	/// 获得附加属性。供扩展
	/// @param	name		属性名
	/// @return	属性值
	nonmutating TianShanIce::Properties getProperties();

	/// 修改或增加附加属性。供扩展
	/// @param props	属性集
	void setProperties(TianShanIce::Properties props);
		
	/// 销毁 ChannelPublishPoint 对象
	void destroy();
};

class FolderChannel extends ChannelPublishPoint {

	void notifyFullUpdate(string rootPath, bool clear, 
		int groupId,int verNumber);

	void notifyPartlyUpdate(string rootPath, string paths, 
		int groupId,int verNumber);

	void notifyFolderDeleted(string paths, int groupId,int verNumber);

	void notifyFileDeleted(string paths, int groupId,int verNumber);

	void notifyFileModified(string rootPath, string paths, 
		int groupId,int verNumber);

	void notifyFileAdded(string rootPath, string paths, 
		int groupId,int verNumber);
};

class MessageChannel extends ChannelPublishPoint {

	void notifyMessageAdded(string messageId, string dest, 
		string messageBody, long exprie, int groupId);
	
	void notifyMessageDeleted(string messageId, int groupId);
};

// -----------------------------
// struct DestInfo
// -----------------------------
/// Destination 的信息
struct DestInfo {
	string	name;
	int		pmtPid;			/// 打包进 TS 中的 PMT pid
	int		totalBandwidth;	/// destination 的 bandwidth in bps
	string	destAddress;	/// 目标地址, in the format of "<IP>:<port>[; <IP>:<port>]"
	int		groupId;	/// 组id
};

// -----------------------------
// enum DestState
// -----------------------------
/// destination 的状态
enum DestState {
	DestInit,		/// 初始
	DestRunning,		/// 运行
	DestPause,		/// 暂停
	DestStopped,		/// 停止
};

// -----------------------------
// class Destination
// -----------------------------
/// DOD流的目标对象
class Destination {

	/// @return Destination 名
	nonmutating string getName();

	/// @return 当前状态
	nonmutating DestState getState();

	nonmutating ::TianShanIce::StrValues listChannels();

	/// 将一个Channel 与 一个流相关联
	/// @param	ch			要关联的 channel
	/// @return 
	void attachChannel(string channelName, int minBitRate, int repeatTime)
		throws ::TianShanIce::InvalidParameter, StreamerException, 
		ObjectExistException;

	/// 取消一个Channel 与 一个流的关联
	/// @param	chName		要取消关联的 channel 名称
	/// @return 
	void detachChannel(string channelName)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	void getChannelAttachedInfo(string channelName, 
		out int minBitRate, out int repeatTime);

	/// 返回 destination 的基本信息
	/// @return  成功时输出 destination 的基本信息
	nonmutating DestInfo getInfo()
		throws ::TianShanIce::InvalidParameter;

	/// 开始推流
	/// @param
	/// @return 
	void serve()
		throws StreamerException;

	/// 修改 Destination 的附加属性
	/// @param	props			要修改属性集
	/// @return 
	void setProperies(TianShanIce::Properties props);		

	/// 获得流附加的属性
	nonmutating TianShanIce::Properties getProperties();

	/// 停止播放 Destination
	/// @param
	/// @return 
	void stop()
		throws StreamerException;

	/// 暂停播放 Destination
	/// @param
	/// @return 
	void pause();

	/// 销毁 Destination 对象
	void destroy();

};

//////////////////////////////////////////////////////////////////////////

// -----------------------------
// class DODPurchase
// -----------------------------
/// DOD 的 Purchase
["freeze:write"]
class DODPurchase extends ::TianShanIce::Application::Purchase {

};

// -----------------------------
// service DODAppService
// -----------------------------
/// DODApp 与 Weiwoo 的接口
interface DODAppService extends ::TianShanIce::Application::AppService {

};

// -----------------------------
// service DataPublisher
// -----------------------------
/// DODApp 与 BackOffice 的接口
class DataPublisher {

	/// 创建本地文件系统Channel
	/// @param	name		发布点名称
	///	@param	props		提供给Channel 的基本属性
	///	@param	path		文件系统发布点数据源路径
	/// @return	返回创建的Channel。失败时返回空。
	FolderChannel* createLocalFolderChannel(string name, 
		ChannelInfo info, string path) 
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// 创建 SMB 共享文件Channel
	/// @param	name		Channel名称
	///	@param	info		提供给Channel 的属性集
	/// @return	返回创建的Channel。失败时返回空。
	FolderChannel* createShareFolderChannel(string name, 
		ChannelInfo info)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// 创建消息数据源的Channel
	/// @param name		Channel名称
	/// @param info		提供给Channel 的基本属性
	/// @return	返回创建的Channel。失败时返回空。
	MessageChannel* createMsgChannel(string name, ChannelInfo info)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// 根据名称获得Channel
	/// @param	name		Channel名称
	/// @return 返回获得的Channel。失败时返回空。
	ChannelPublishPoint* getChannel (string name)
		throws ::TianShanIce::InvalidParameter;

	/// 创建流
	/// @param	destName		destination name
	///	@param	info			基本属性集	
	/// @return	成功返回 Destination 对象，否则为 NULL
	Destination* createDestination(string destName, DestInfo info)
		throws ::TianShanIce::InvalidParameter, StreamerException;
	
	/// 根据名称获得 Destination
	/// @param	name		Destination 名称
	/// @return 返回获得的Destination。失败时返回空。
	Destination* getDestination(string name)
		throws ::TianShanIce::InvalidParameter;

	/// 列出存在的所有 channel
	/// @return	返回所有存在的 channel 名
	nonmutating TianShanIce::StrValues listChannels();

	/// 列出存在的所有 destination
	/// @return	返回所有存在的 dest 名
	nonmutating TianShanIce::StrValues listDestinations();

	/// 通知更新
	void notifyFolderFullUpdate(int groupId, int dataType, 
		string rootPath, bool clear, int VerNumber);

	void notifyFolderPartlyUpdate(int groupId, int dataType, 
		string rootPath, string paths, int VerNumber);

	void notifyFolderDeleted(int groupId, int dataType, 
		string paths, int VerNumber);

	void notifyFileAdded(int groupId, int dataType, 
		string rootPath, string paths, int VerNumber);

	void notifyFileModified(int groupId, int dataType, 
		string rootPath, string paths, int VerNumber);

	void notifyFileDeleted(int groupId, int dataType, 
		string paths, int VerNumber);

	void notifyMessageAdded(int groupId, int dataType, 
		string messageId, string dest, string messageBody, long exprie);

	void notifyMessageDeleted(int groupId, int dataType, 
		string messageId);

};

}; /// module DataOnDemand

#endif // __ZQ_ChannelOnDemand_DoDApp_ICE__
