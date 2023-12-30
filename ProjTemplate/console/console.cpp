// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: McastRcv.cpp,v 1.8 2004/07/28 09:30:34 shao Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : the multicast package receiver utility
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/ProjTemplate/console/console.cpp $
// 
// 1     10-11-12 16:01 Admin
// Created.
// 
// 1     10-11-12 15:33 Admin
// Created.
// 
// 1     3/24/05 1:49p Hui.shao
// ===========================================================================

#include "getopt.h"

extern "C"
{
#include <stdio.h>
}

void usage()
{
	printf("Usage: console [-h]\n");
	printf("ZQ console project template.\n");
	printf("options:\n");
	printf("\t-h   display this help\n");
}

int main(int argc, char* argv[])
{
	// parse the command options
	if (argc <2)
	{
		usage();
		return -1;
	}

	int ch;
	while((ch = getopt(argc, argv, "h")) != EOF)
	{
		switch (ch)
		{
		case '?':
		case 'h':
			usage();
			return 0;
		}
	}

	printf("this is a ZQ console program template\n");
	return 0;
}

