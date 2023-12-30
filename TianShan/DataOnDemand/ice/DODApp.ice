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
/// ChannelPublishPoint ����
enum ChannelType {
	dodSharedFolder,	/// SMB �ļ�����
	dodMessage,			/// ��Ϣ
	dodLocalFolder, 	/// �����ļ�
};

// -----------------------------
// enum EncryptMode
// -----------------------------
/// ����ģʽ
enum EncryptMode {
	dodEncryptNone,		/// ������
	dodEncryptNormal,	/// ��ͨ����
	dodEncryptSimple,	/// �򵥼���
	dodEncryptReserve,	/// ����
	dodEncryptCompress, /// ֻѹ��
	dodEncryptNormalCompress, 
	dodEncryptSimpleCompress, 	
};

// -----------------------------
// struct ChannelInfo
// -----------------------------
/// ChannelPublishPoint �Ļ�����Ϣ
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

	/// @return ChannelPublishPoint ��
	nonmutating string getName();

	/// ���� ChannelPublishPoint ������ local folder / share folder / message
	/// @return	ChannelPublishPoint ������
	nonmutating ChannelType getType();

	/// �õ� ChannelPublishPoint �Ļ�����Ϣ
	/// @return	ChannelPublishPoint �Ļ�����Ϣ	
	nonmutating ChannelInfo getInfo();

	/// ��ø������ԡ�����չ
	/// @param	name		������
	/// @return	����ֵ
	nonmutating TianShanIce::Properties getProperties();

	/// �޸Ļ����Ӹ������ԡ�����չ
	/// @param props	���Լ�
	void setProperties(TianShanIce::Properties props);
		
	/// ���� ChannelPublishPoint ����
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
/// Destination ����Ϣ
struct DestInfo {
	string	name;
	int		pmtPid;			/// ����� TS �е� PMT pid
	int		totalBandwidth;	/// destination �� bandwidth in bps
	string	destAddress;	/// Ŀ���ַ, in the format of "<IP>:<port>[; <IP>:<port>]"
	int		groupId;	/// ��id
};

// -----------------------------
// enum DestState
// -----------------------------
/// destination ��״̬
enum DestState {
	DestInit,		/// ��ʼ
	DestRunning,		/// ����
	DestPause,		/// ��ͣ
	DestStopped,		/// ֹͣ
};

// -----------------------------
// class Destination
// -----------------------------
/// DOD����Ŀ�����
class Destination {

	/// @return Destination ��
	nonmutating string getName();

	/// @return ��ǰ״̬
	nonmutating DestState getState();

	nonmutating ::TianShanIce::StrValues listChannels();

	/// ��һ��Channel �� һ���������
	/// @param	ch			Ҫ������ channel
	/// @return 
	void attachChannel(string channelName, int minBitRate, int repeatTime)
		throws ::TianShanIce::InvalidParameter, StreamerException, 
		ObjectExistException;

	/// ȡ��һ��Channel �� һ�����Ĺ���
	/// @param	chName		Ҫȡ�������� channel ����
	/// @return 
	void detachChannel(string channelName)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	void getChannelAttachedInfo(string channelName, 
		out int minBitRate, out int repeatTime);

	/// ���� destination �Ļ�����Ϣ
	/// @return  �ɹ�ʱ��� destination �Ļ�����Ϣ
	nonmutating DestInfo getInfo()
		throws ::TianShanIce::InvalidParameter;

	/// ��ʼ����
	/// @param
	/// @return 
	void serve()
		throws StreamerException;

	/// �޸� Destination �ĸ�������
	/// @param	props			Ҫ�޸����Լ�
	/// @return 
	void setProperies(TianShanIce::Properties props);		

	/// ��������ӵ�����
	nonmutating TianShanIce::Properties getProperties();

	/// ֹͣ���� Destination
	/// @param
	/// @return 
	void stop()
		throws StreamerException;

	/// ��ͣ���� Destination
	/// @param
	/// @return 
	void pause();

	/// ���� Destination ����
	void destroy();

};

//////////////////////////////////////////////////////////////////////////

// -----------------------------
// class DODPurchase
// -----------------------------
/// DOD �� Purchase
["freeze:write"]
class DODPurchase extends ::TianShanIce::Application::Purchase {

};

// -----------------------------
// service DODAppService
// -----------------------------
/// DODApp �� Weiwoo �Ľӿ�
interface DODAppService extends ::TianShanIce::Application::AppService {

};

// -----------------------------
// service DataPublisher
// -----------------------------
/// DODApp �� BackOffice �Ľӿ�
class DataPublisher {

	/// ���������ļ�ϵͳChannel
	/// @param	name		����������
	///	@param	props		�ṩ��Channel �Ļ�������
	///	@param	path		�ļ�ϵͳ����������Դ·��
	/// @return	���ش�����Channel��ʧ��ʱ���ؿա�
	FolderChannel* createLocalFolderChannel(string name, 
		ChannelInfo info, string path) 
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// ���� SMB �����ļ�Channel
	/// @param	name		Channel����
	///	@param	info		�ṩ��Channel �����Լ�
	/// @return	���ش�����Channel��ʧ��ʱ���ؿա�
	FolderChannel* createShareFolderChannel(string name, 
		ChannelInfo info)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// ������Ϣ����Դ��Channel
	/// @param name		Channel����
	/// @param info		�ṩ��Channel �Ļ�������
	/// @return	���ش�����Channel��ʧ��ʱ���ؿա�
	MessageChannel* createMsgChannel(string name, ChannelInfo info)
		throws ::TianShanIce::InvalidParameter, StreamerException;

	/// �������ƻ��Channel
	/// @param	name		Channel����
	/// @return ���ػ�õ�Channel��ʧ��ʱ���ؿա�
	ChannelPublishPoint* getChannel (string name)
		throws ::TianShanIce::InvalidParameter;

	/// ������
	/// @param	destName		destination name
	///	@param	info			�������Լ�	
	/// @return	�ɹ����� Destination ���󣬷���Ϊ NULL
	Destination* createDestination(string destName, DestInfo info)
		throws ::TianShanIce::InvalidParameter, StreamerException;
	
	/// �������ƻ�� Destination
	/// @param	name		Destination ����
	/// @return ���ػ�õ�Destination��ʧ��ʱ���ؿա�
	Destination* getDestination(string name)
		throws ::TianShanIce::InvalidParameter;

	/// �г����ڵ����� channel
	/// @return	�������д��ڵ� channel ��
	nonmutating TianShanIce::StrValues listChannels();

	/// �г����ڵ����� destination
	/// @return	�������д��ڵ� dest ��
	nonmutating TianShanIce::StrValues listDestinations();

	/// ֪ͨ����
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
