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
// Ident : $Id: ProvisionResourceBookTest.cpp $
// Branch: $Name:  $
// Author: Jie Zhang
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CPE/common/StreamDataDumperTest.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 1     08-11-18 11:21 Jie.zhang
// 
// 1     08-10-23 19:02 Jie.zhang
// ---------------------------------------------------------------------------

#include "CppUTest/TestHarness.h"
#include "StreamDataDumper.h"


TEST_GROUP(StreamDumpper)
{
	StreamDataDumper* _dumper;

	void setup()
	{
		_dumper = new StreamDataDumper();
	}
	void teardown()
	{
		delete _dumper;
	}
};

TEST(StreamDumpper, func_normal)
{
	_dumper->enable();
	_dumper->setPath("c:");
	_dumper->setFile("xxx");

	_dumper->init();

	char x[64*1024];
	int nSize = sizeof(x);
	_dumper->dump(x, nSize);
	_dumper->close(true);
}

TEST(StreamDumpper, func_not_enable)
{
	_dumper->setPath("c:");
	_dumper->setFile("xxx");
	_dumper->deleteOnSuccess(false);

	_dumper->init();

	char x[64*1024];
	int nSize = sizeof(x);
	_dumper->dump(x, nSize);
	_dumper->close(true);
}

TEST(StreamDumpper, func_error)
{
	_dumper->setPath("c:");
	_dumper->setFile("xxx");

	_dumper->init();

	char x[64*1024];
	int nSize = sizeof(x);
	_dumper->dump(x, nSize);
	_dumper->close(false);
}

