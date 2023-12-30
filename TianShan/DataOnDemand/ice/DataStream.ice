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
// $Log: /ZQProjs/TianShan/DataOnDemand/ice/DataStream.ice $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:38 Admin
// Created.
// 
// 3     09-02-17 16:25 Li.huang
// 
// 2     08-12-09 17:24 Li.huang
// 
// 1     08-12-08 11:10 Li.huang
// 
// 13    08-10-29 15:27 Li.huang
// 
// 12    07-04-13 17:12 Cary.xiao
// 
// 11    07-04-13 16:23 Cary.xiao
// 
// 10    07-03-29 14:11 Cary.xiao
// 
// 9     07-03-19 17:05 Li.huang
// 
// 8     07-01-29 13:32 Cary.xiao
// 
// 7     07-01-23 12:18 Cary.xiao
// 
// 6     07-01-19 10:57 Cary.xiao
// 
// 5     07-01-10 17:29 Cary.xiao
// 
// 4     06-12-08 15:30 Cary.xiao
// 
// 3     06-12-08 15:17 Cary.xiao
// 
// 2     06-12-07 18:54 Hui.shao
// inital refine
// 1     06-12-07 18:54 Xiao Tao
// fixed something :P
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

/// namespace of DataOnDemand
module DataOnDemand {

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
	nonmutating string getName();

	/// ֪ͨ����
	void notifyFullUpdate(string fileName);
	void notifyFileAdded(string fileName);
	void notifyFileDeleted(string fileName);

	/// ��û�����Ϣ
	nonmutating MuxItemInfo getInfo();

	/// ���� mux item ����
	void destroy()
		throws DataStreamError;
		
	/// �޸� MuxItem �ĸ�������
	/// @param	props			Ҫ�޸����Լ�
	/// @return 
	void setProperies(TianShanIce::Properties props);

	/// ��������ӵ�����
	nonmutating TianShanIce::Properties getProperties();
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
	nonmutating string getName();

	/// �õ����Ļ�����Ϣ
	/// @return		basic infor of stream
	nonmutating StreamInfo getInfo()
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
	nonmutating ::Ice::StringSeq listMuxItems();

	/// �ͻ��˶�ʱ�� stream ���� ping �ź�
	nonmutating void ping();
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
	
	DataStream* getStream(string space, string name);
	
	nonmutating TianShanIce::StrValues listStrems(string space);
	
	void clear(string space);

	void destroy();
	
	/// �޸� DataStreamService �ĸ�������
	/// @param	props			Ҫ�޸����Լ�
	/// @return 
	void setProperies(TianShanIce::Properties props);

	/// ��������ӵ�����
	nonmutating TianShanIce::Properties getProperties();
	
	nonmutating void ping(string space);
};

}; /// module DataOnDemand

#endif // __ZQ_DataOnDemand_DataStream_ICE__
