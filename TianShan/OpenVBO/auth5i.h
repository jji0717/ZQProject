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
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : implmentation of	"eventIS VOD - Interface Part 5i- URL signing and validation v1.x.pdf"
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/OpenVBO/auth5i.h $
// 
// 12    5/22/14 3:06p Hongquan.zhang
// add a new method to auth C2 request
// 
// 11    5/05/14 11:28a Zhiqiang.niu
// check in for C2stream
// 
// 10    7/26/13 12:41p Hui.shao
// 
// 9     7/24/13 6:52p Hui.shao
// auth with case-insensitive
// 
// 8     7/24/13 11:08a Hui.shao
// 
// 7     7/19/13 11:04a Hui.shao
// code review
// 
// 6     7/19/13 10:00a Ketao.zhang
// 
// 5     7/15/13 3:41p Hui.shao
// 
// 4     7/15/13 10:35a Ketao.zhang
// 
// 3     7/11/13 4:42p Ketao.zhang
// 
// 2     7/02/13 2:01p Hui.shao
// file comment
// 
// 1     7/02/13 1:58p Hui.shao
// drafted the class prototype
// ===========================================================================

#ifndef __OpenVBO_auth5i_H__
#define __OpenVBO_auth5i_H__

#include "Log.h"

#include <string>
#include <map>

class Authen5i
{
public:
	typedef struct _SignParam
	{
		std::string sessionId;
		std::string ip;
		int64       expire;
	} SignParam;

	typedef struct
	{
		uint8 b[64];
	} Key5i_t ; // the 64B or 128c-hex key used in 5i
// 	typedef std::vector </*Key5i_t*/uint8*> KeyList;
//	typedef std::vector <uint8> Key5i_t;
	typedef std::map <int ,Key5i_t> KeyList;
	Authen5i(ZQ::common::Log& log);
	~Authen5i();
	/// loadKeyFile implements the steps on how to load the key file, defined in section 2.1 of 
	/// "eventIS VOD - Interface Part 5i- URL signing and validation v1.7.pdf", into member variable _keyList; 
	///@param pathname the path name of the key xml file to read
	bool loadKeyFile(const char* pathname);

	bool sign(std::string& url, int keyId=-1, const std::string& sessId=std::string(), const std::string& ip=std::string(), const std::string& expire=std::string());

	bool authen(const std::string& url);
	bool isExpired(const std::string& url);

	bool setKey(int idx, const char* keystr);

	size_t keySize() { return _keyList.size(); }

	/// calculateSignature implements the steps on how to load the key file, defined in section 2.3 of 
	/// "eventIS VOD - Interface Part 5i- URL signing and validation v1.7.pdf", into _keyList; 
	///@param[in] uri_path the path part of the URL
	///@param[in] keyId the index refer to _keyList read from key file
	///@param[in] sessId the session Id
	///@param[in] ip the ip address of the usage client
	///@param[in] expire the UTC expiry time of the signature, in the format of YYYYMMDDhhmmss, e.g. 20101213175333
	///@return the generated signature hex string, 16byte or 32ch hex string
	virtual std::string calculateSignature(const std::string& uri_path, int keyId, const std::string& sessId, const std::string& ip, const std::string& expire);

	bool authC2( const std::string& paid, const std::string& pid, 
		const std::string& txnId, const std::string& clientSessId,
		const std::string& expiration, const std::string& signature );

	static void EncodeToUTF8(const std::string& szSource, std::string& szFinal);
protected:
	KeyList _keyList;
	ZQ::common::Log& _log;

	
public:
	virtual std::string signC2PlaylistItem(const std::string& providerAssetId, const std::string providerId, const std::string& txnId, const std::string& clientSessionId, const std::string& expire);

};
#endif // __OpenVBO_auth5i_H__
