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
// Ident : $Id: CdmiFuseCmd.cpp Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : FUSE implementation for Windows based on Dokan
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/OpenVBO/auth5icli/main.cpp $
// 
// 5     7/26/13 1:05p Hui.shao
// 
// 4     7/26/13 1:03p Hui.shao
// 
// 3     7/26/13 12:41p Hui.shao
// 
// 2     7/24/13 6:53p Hui.shao
// test if expired
// 
// 1     7/24/13 11:10a Hui.shao
// utility to test i5 sign and auth
// ===========================================================================

#include "../auth5i.h"
#include "FileLog.h"
#include "getopt.h"

extern "C"
{
#include <stdio.h>
}

void usage(char* progName)
{
	char* p = strrchr(progName, FNSEPC);
	if (NULL !=p) progName=++p;
	printf("Usage: %s [OPTIONS] -u \"<URL>\"\n", progName);
	printf("       %s [-h]\n", progName);
	printf("OpenVBO 5i sign/auth utility\n\n");
	printf("Options:\n");
	printf(	"    -h                display this help screen\n"
			"    -u <URL>          specify the URL to auth or sign, must be bracked by \"\"\n"
			"    -f <KeyFile>      specify the keyfile to load, default .\\Keyfile.xml\n"
			"    -k <keyId>        specify a keyId defined in the keyfile\n"
			"    -a                to perform auth only instead of sign\n");
	printf(	"    for example:\n"
			"        %s -k 3 -u \"http://10.0.0.1:8080/assets/batman?I=34&E=20131231120000&A=192.168.1.2\"\n",
			progName);
}

int main(int argc, char* argv[])
{
	// parse the command options
	if (argc <2)
	{
		usage(argv[0]);
		return -1;
	}

	int ch;
	int keyId=-1;
	bool bToAuth=false;
	std::string url, keyfilename = ".\\Keyfile.xml";
	while((ch = getopt(argc, argv, "hf:k:au:")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage(argv[0]);
			return 0;

		case 'f':
			if (optarg)
				keyfilename = optarg;
			break;

		case 'k':
			keyId = atoi(optarg);
			break;

		case 'a':
			bToAuth =true;
			break;

		case 'u':
			if (optarg)
				url = optarg;
			break;

		default:
			fprintf(stderr, "unknown option: %c\n", ch);
			return -1;
		}
	}

	ZQ::common::Log nillog;

	Authen5i auth5i(nillog);
//	auth5i.setKey(1, "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff");

	if (url.empty())
	{
		printf("no URL specified\n");
		return -2;
	}

	if (!auth5i.loadKeyFile(keyfilename.c_str()))
	{
		printf("failed to load keyfile[%s]\n", keyfilename.c_str());
		return -3;
	}

	if (bToAuth)
	{
		bool expired = auth5i.isExpired(url);
		bool succ = auth5i.authen(url);

		printf("Authenticate[%s/%s]: url[%s]\n", succ ? "PASSED":"DENIED", expired?"EXPIRED":"ACTIVE", url.c_str());
		return succ?0:(-4);
	}

	if (auth5i.sign(url, keyId))
	{
		printf("%s\n", url.c_str());
		return 0;
	}

	printf("ERROR: illegal input to sign\n");
	return -5;
}
