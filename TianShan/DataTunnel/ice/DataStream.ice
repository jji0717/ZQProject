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
// Ident : $Id: DODStreamer.ice$
// Branch: $Name:  $
// Author: Xiao Tao
// Desc  : 
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/DataTunnel/ice/DataStream.ice $
// 
// 1     10-11-12 16:05 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 1     09-03-09 10:33 Li.huang

// ---------------------------------------------------------------------------

// build steps:
// $(ICE_VC7_ROOT)\bin\slice2cpp.exe -I$(ICE_VC7_ROOT)/slice -I$(ZQProjsPath)/TianShan/Ice -I$(ZQProjsPath)/DataOnDemand/Ice $(InputPath)

// $(ICE_VC7_ROOT)\bin\slice2freeze.exe -I$(ICE_VC7_ROOT)/slice -I$(ZQProjsPath)/DataOnDemand/Ice \
//		--output-dir .. --dict "DODStreamer::StreamSourceDict,string,DODStreamer::MuxItemInfo" \
//		StreamSourceDict ..\$(InputName).ice

// $(ICE_VC7_ROOT)\bin\slice2freeze.exe -I$(ICE_VC7_ROOT)/slice -I$(ZQProjsPath)/DataOnDemand/Ice \
//		--output-dir .. --dict "DODStreamer::StreamDict,string,DODStreamer::Stream" \
//		StreamDict ..\$(InputName).ice

// outputs:

#ifndef __ZQ_DataOnDemand_DataStream_ICE__
#define __ZQ_DataOnDemand_DataStream_ICE__

#include "Ice/BuiltinSequences.ice"
#include "TsStreamer.ice"

module TianShanIce
{
module Streamer
{

/// namespace of DataOnDemand
module DataOnDemand 
{

// -----------------------------
// exception DataStreamError
// -----------------------------
/// DODStreamer exception
exception DataStreamError extends ::TianShanIce::ServerError {

};

/// �����ظ�
exception NameDupException extends DataStreamError {

};

enum CacheType {
	dodCacheTypeSmb,	/// ͨ�� SMB ����Ŀ¼���� cache
	dodCacheTypeTcp,	/// ͨ�� TCP ���� cache
	dodCacheTypeUdp,	/// ͨ�� UDP ���� cache
	dodCacheTypeIce,	/// ͨ�� ICE ���� cache
};

/// basic infor of stream source
struct MuxItemInfo {
	string		name;
	int			streamId;	/// pid
	int			streamType;
	int			bandWidth;
	int			tag;

	/// the expiration of this stream item, in system time and in millisecond
	long		expiration; 

	int			repeatTime;
	CacheType	ctype;

	/// ������ctypeȷ��, smb ʱΪ��������ļ� cache Ŀ¼
	/// udp, tcp, ice ʱΪ���Э���ַ
	string		cacheAddr;
	
	int			encryptMode;
	int			subchannelCount;
};


// -----------------------------
// class MuxItem
// -----------------------------
/// mux item class
["freeze:write"]
class MuxItem {

	/// �õ� mux item name
	["cpp:const", "freeze:read"] string getName();

	/// ֪ͨ����
	void notifyFullUpdate(string fileName);
	void notifyFileAdded(string fileName);
	void notifyFileDeleted(string fileName);

	/// ��û�����Ϣ
	["cpp:const", "freeze:read"] MuxItemInfo getInfo();

	/// ���� mux item ����
	void destroy()
		throws DataStreamError;
		
	/// �޸� MuxItem �ĸ�������
	/// @param	props			Ҫ�޸����Լ�
	/// @return 
	void setProperties(TianShanIce::Properties props);

	/// ��������ӵ�����
	["cpp:const", "freeze:read"] TianShanIce::Properties getProperties();
};

/// basic infor of stream
struct StreamInfo {
	string		name;
	int			totalBandwidth;
	string		destAddress;
	int			pmtPid;
};

// -----------------------------
// class DataStream
// -----------------------------
/// data stream class
["freeze:write"]
class DataStream extends ::TianShanIce::Streamer::Stream {

	/// �õ� stream name
	["cpp:const", "freeze:read"]  string getName();

	/// �õ����Ļ�����Ϣ
	/// @return		basic infor of stream
	["cpp:const", "freeze:read"]  StreamInfo getInfo()
		throws DataStreamError;
	
	/// ����
	/// @param code			control code
	/// @param param		control parameter
	int control(int code, string param)
		throws ::TianShanIce::InvalidParameter;

	/// ����һ���µ� StreamSource
	/// @param	name		name of stream source
	/// @param	info		basic infor of stream source
	MuxItem* createMuxItem(MuxItemInfo info)
		throws DataStreamError, ::TianShanIce::InvalidParameter;

	/// ͨ�����ֲ���ָ���� Mux Item
	/// @param	name	Ҫ�õ���Mux Item�� name
	/// @return ����ָ�� stream source ����Ϣ
	MuxItem* getMuxItem(string name)
		throws DataStreamError;

	/// �õ����е� mux item �б�
	["cpp:const", "freeze:read"]  ::Ice::StringSeq listMuxItems();

	/// �ͻ��˶�ʱ�� stream ���� ping �ź�
	["cpp:const", "freeze:read"]  void ping();
};

// -----------------------------
// service DataStreamService
// -----------------------------
/// dod stream service
["freeze:write"]
interface DataStreamService extends ::TianShanIce::Streamer::StreamService {

	/// ���� DOD Stream
	/// @param	pathTicket	
	/// @param	name		stream name
	/// @param	info		���� stream �Ļ�������.
	DataStream* createStreamByApp(
		::TianShanIce::Transport::PathTicket* pathTicket, 
		string space, 
		StreamInfo info) 
		throws NameDupException, ::TianShanIce::InvalidParameter;
	
	["cpp:const", "freeze:read"] DataStream* getStream(string space, string name);
	
	["cpp:const", "freeze:read"] TianShanIce::StrValues listStreams(string space);
	
	void clear(string space);

	void destroy();
	
	["cpp:const", "freeze:read"]  void ping(string space);
};

}; /// module DataOnDemand
}; /// module StreamService
}; /// module TianShanIce
#endif // __ZQ_DataOnDemand_DataStream_ICE__
