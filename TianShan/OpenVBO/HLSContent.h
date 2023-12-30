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
// Ident : $Id: HLSContent.h Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : Represent the a HLS-content stored on Aqua storage
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/OpenVBO/HLSContent.h $
// 
// 5     8/01/13 11:32a Hongquan.zhang
// 
// 4     7/31/13 10:12a Hui.shao
// relativeUriLevel in exportMasterM3u8()
// 
// 3     7/17/13 4:07p Hui.shao
// 
// 2     7/15/13 4:03p Hui.shao
// 
// 1     7/15/13 3:50p Hui.shao
// draft impl of HLSContent to read nPVR recording over Aqua
// ===========================================================================
#ifndef _HLSContent_H__
#define _HLSContent_H__

#include "CdmiFuseOps.h"
#include "m3u8.h"
#include "../../TianShan/OpenVBO/auth5i.h"

#define DEFAULT_SEGMENT_DUR  (10*1000) // 10sec
#define MIN_REFRESH_INTERVAL (DEFAULT_SEGMENT_DUR + DEFAULT_SEGMENT_DUR/2) // 15sec
#define DEFAULT_AQUA_TOKEN_TIMEOUT (4*60*60*1000) // 4hours
#define DEFAULT_CONTENT_TYPE "video/avi"

// -----------------------------
// class HLSContent
// -----------------------------
/// A FUSE-like implementation for Windows with the support of open source project Dokan
/// dokan homepage: http://dokan-dev.net 
class HLSContent
{
public:
	HLSContent(ZQ::common::Log& log, Authen5i& auth5i, CdmiFuseOps& cdmiClient, const std::string& contentName);
	virtual ~HLSContent() {}

	//set subname to bitrate mapping, you can only invoke this function while service is in starting stage
	static void	setSubname2Bitrate( const std::map<std::string,uint32>& mapping, uint32 defaultBitrate );
	
	/// export the top layer content m3u8
	///@param[in] hlsRootUri the root URI/URL to put as the prefix of URI in outgoing m3u8
	///@param[in] relativeUriLevel the level on how to export the uri in the output m3u8
	///               0-output the full URL if hlsRootUri presents "proto://server:port/"
	///               1-output the absolute path of uri start with '/'
	///               2-output the relative path of uri after value of hlsRootUri, no leading '/'
	bool exportMasterM3u8(std::string& m3u8Content, int keyId, const char* sessionId="000", const char* ipAddr="127.0.0.1", const char* hlsRootUri="/assets/", int relativeUriLevel=0);
	bool exportPlaylistM3u8(const std::string& subname, std::string& m2u8Content);

	typedef struct _PlaylistInfo
	{
		uint32 bitrate;
		std::string subName, mimeType;
		std::string streamInf;
		uint        sequenceStart;
		int64       stampLastRead;
		CdmiFuseOps::StrList     segments;
		CdmiFuseOps::Properties  xkey_since;
	} PlaylistInfo;

	typedef std::map <std::string, PlaylistInfo> PlaylistMap; // sub-container to playlist map

protected: 
	ZQ::common::Log& _log;
	Authen5i& _auth5i; 
	CdmiFuseOps& _cdmiClient;
	std::string  _contentName;
	CdmiFuseOps::Properties   _metadata;

	PlaylistMap  _playlistMap;
	int64        _stampLastRead;
	bool         _bStill;

	ZQ::common::Mutex _lk;

	// open the content-level container on Aqua and read the metadata and bitrates 
	bool read();
	bool readPlaylist(const std::string& subname);

};

#endif // _HLSContent_H__

/* sample test program
void hlstest()
{
	ZQ::common::FileLog testlog("./hlscontent.log", ZQ::common::Log::L_DEBUG);

	Authen5i auth5i(testlog);
	// auth5i.setKey(1, "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff");
	auth5i.loadKeyFile("./keyfile.xml");

	std::string rootUrl = "http://demo5:demo5@10.50.16.80:8080/aqua/rest/cdmi/";
	std::string homeContainer= "/demo5";
	CdmiFuseOps client(testlog, thpool, rootUrl, homeContainer, 0x3f);

	std::string contentName = "IronManIII", m3u8content;
	HLSContent hlsContent(testlog, auth5i, client, contentName);
	if (hlsContent.exportMasterM3u8(m3u8content, 1))
	{
		printf("master-m3u8:\n%s\n", m3u8content.c_str());
		testlog(ZQ::common::Log::L_INFO, "m3u8:\n%s", m3u8content.c_str());
	}

	for (int i=0; i< 10; i++)
	{
		if (!hlsContent.exportPlaylistM3u8("bitrate1", m3u8content))
			continue;
		printf("playlist-m3u8:\n%s\n", m3u8content.c_str());
		testlog(ZQ::common::Log::L_INFO, "m3u8:\n%s", m3u8content.c_str());
	}
}
*/
