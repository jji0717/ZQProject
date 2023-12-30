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
// $Log: /ZQProjs/TianShan/CPE/ProvisionResourceBookTest.cpp $
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 4     09-05-21 17:59 Jie.zhang
// merge from 1.10
// 
// 4     09-05-05 14:37 Jie.zhang
// 
// 3     09-03-06 16:45 Jie.zhang
// 
// 2     09-03-04 21:33 Jie.zhang
// reflactor the provision cost source
// 
// 1     08-11-18 11:24 Jie.zhang
// ---------------------------------------------------------------------------

#include "CppUTest/TestHarness.h"
#include "ProvisionResourceBook.h"
#include "FakeMethodCost.h"

using namespace ZQTianShan::ContentProvision;

TEST_GROUP(ProvisionBook)
{
	ProvisionResourceBook* _provisionBook;
	FakeMethodCostCol*			_fakeMC;

	void setup()
	{
		_fakeMC = new FakeMethodCostCol();
		_provisionBook = new ProvisionResourceBook(_fakeMC);		
	}
	void teardown()
	{
		delete _provisionBook;
		delete _fakeMC;
	}
};

TEST(ProvisionBook, MethodCost)
{
	_fakeMC->addResource("m1", 120, 5);

	unsigned int nCost = _fakeMC->evaluateCost("m1", 30, 5);
	CHECK_EQUAL(_fakeMC->isOverLoad(nCost), false);

	nCost = _fakeMC->evaluateCost("m1", 30, 2);
	CHECK_EQUAL(_fakeMC->isOverLoad(nCost), false);

	nCost = _fakeMC->evaluateCost("m1", 100, 2);
	CHECK_EQUAL(_fakeMC->isOverLoad(nCost), false);

	nCost = _fakeMC->evaluateCost("m1", 400, 1);
	CHECK_EQUAL(_fakeMC->isOverLoad(nCost), true);

	nCost = _fakeMC->evaluateCost("m1", 100, 6);
	CHECK_EQUAL(_fakeMC->isOverLoad(nCost), true);

	nCost = _fakeMC->evaluateCost("m1", 120, 4);
	CHECK_EQUAL(_fakeMC->isOverLoad(nCost), false);

	nCost = _fakeMC->evaluateCost("m1", 100, 5);
	CHECK_EQUAL(_fakeMC->isOverLoad(nCost), false);

	nCost = _fakeMC->evaluateCost("m1", 120, 5);
	CHECK_EQUAL(_fakeMC->isOverLoad(nCost), false);
}

TEST(ProvisionBook, ExceptionTest1)
{	
	_fakeMC->addResource("m1", 1, 1);
	_fakeMC->addResource("m2", 0, 1);
	_fakeMC->addResource("m3", 1, 0);
	_fakeMC->addResource("m4", 0, 0);

	bool bRet;

	bRet = _provisionBook->bookProvision("m1", 10, 20, 30);
	CHECK_EQUAL(bRet, false);

	bRet = _provisionBook->bookProvision("m1", 10, 20, 50);
	CHECK_EQUAL(bRet, false);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 50);
	CHECK_EQUAL(bRet, false);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 0);
	CHECK_EQUAL(bRet, false);

	bRet = _provisionBook->bookProvision("m3", 10, 20, 50);
	CHECK_EQUAL(bRet, false);

	bRet = _provisionBook->bookProvision("m4", 10, 20, 50);
	CHECK_EQUAL(bRet, false);
}

//
// test the book logic
//
TEST(ProvisionBook, FunctionBookTest1)
{	
	_fakeMC->addResource("m1", 150, 6);

	_provisionBook->addProvision("m1", 5, 9, 10);
	_provisionBook->addProvision("m1", 5, 10, 20);
	_provisionBook->addProvision("m1", 5, 11, 30);
	_provisionBook->addProvision("m1", 10, 13, 40);
	_provisionBook->addProvision("m1", 10, 16, 20);
	_provisionBook->addProvision("m1", 13, 16, 30);
	_provisionBook->addProvision("m1", 13, 19, 10);
	_provisionBook->addProvision("m1", 17, 21, 20);
	_provisionBook->addProvision("m1", 20, 21, 30);

	bool bRet;
	bRet = _provisionBook->bookProvision("m1", 10, 20, 60);
	CHECK_EQUAL(bRet, true);

	bRet = _provisionBook->bookProvision("m1", 10, 20, 1);
	CHECK_EQUAL(bRet, false);
}

//
// test the book logic
//
TEST(ProvisionBook, FunctionBookTest2)
{	
	_fakeMC->addResource("m1", 150, 6);

	_provisionBook->addProvision("m1", 5, 9, 10);
	_provisionBook->addProvision("m1", 5, 10, 20);
	_provisionBook->addProvision("m1", 5, 11, 30);
	_provisionBook->addProvision("m1", 10, 13, 40);
	_provisionBook->addProvision("m1", 10, 16, 20);
	_provisionBook->addProvision("m1", 13, 16, 30);
	_provisionBook->addProvision("m1", 13, 19, 10);
	_provisionBook->addProvision("m1", 17, 21, 20);
	_provisionBook->addProvision("m1", 20, 21, 30);

	bool bRet;
	bRet = _provisionBook->bookProvision("m1", 10, 20, 61);
	CHECK_EQUAL(bRet, false);
}

//
// test the book logic
//
TEST(ProvisionBook, FunctionBookTest3)
{	
	_fakeMC->addResource("m1", 150, 6);

	_provisionBook->addProvision("m1", 5, 9, 10);
	_provisionBook->addProvision("m1", 5, 10, 20);
	_provisionBook->addProvision("m1", 5, 11, 30);
	_provisionBook->addProvision("m1", 10, 13, 40);
	_provisionBook->addProvision("m1", 10, 16, 20);
	_provisionBook->addProvision("m1", 13, 16, 30);
	_provisionBook->addProvision("m1", 13, 19, 10);
	_provisionBook->addProvision("m1", 17, 21, 20);
	_provisionBook->addProvision("m1", 20, 21, 30);

	bool bRet;
	bRet = _provisionBook->bookProvision("m1", 12, 18, 60);
	CHECK_EQUAL(bRet, true);
}

//
// test the book logic
//
TEST(ProvisionBook, FunctionBookTest4)
{	
	_fakeMC->addResource("m1", 150, 6);

	_provisionBook->addProvision("m1", 5, 9, 10);
	_provisionBook->addProvision("m1", 5, 10, 20);
	_provisionBook->addProvision("m1", 5, 11, 30);
	_provisionBook->addProvision("m1", 10, 13, 40);
	_provisionBook->addProvision("m1", 10, 16, 20);
	_provisionBook->addProvision("m1", 13, 16, 30);
	_provisionBook->addProvision("m1", 13, 19, 10);
	_provisionBook->addProvision("m1", 17, 21, 20);
	_provisionBook->addProvision("m1", 20, 21, 30);

	bool bRet;
	bRet = _provisionBook->bookProvision("m1", 12, 18, 91);
	CHECK_EQUAL(bRet, false);
}

//
// test the book logic
//
TEST(ProvisionBook, FunctionBookTest5)
{	
	_fakeMC->addResource("m1", 150, 6);

	_provisionBook->addProvision("m1", 5, 9, 10);
	_provisionBook->addProvision("m1", 5, 10, 20);
	_provisionBook->addProvision("m1", 5, 11, 30);
	_provisionBook->addProvision("m1", 10, 13, 40);
	_provisionBook->addProvision("m1", 10, 16, 20);
	_provisionBook->addProvision("m1", 13, 16, 30);
	_provisionBook->addProvision("m1", 13, 19, 10);
	_provisionBook->addProvision("m1", 17, 21, 20);
	_provisionBook->addProvision("m1", 20, 21, 30);

	bool bRet;
	bRet = _provisionBook->bookProvision("m1", 8, 22, 60);
	CHECK_EQUAL(bRet, true);
}

//
// test the book logic
//
TEST(ProvisionBook, FunctionBookTest6)
{	
	_fakeMC->addResource("m1", 150, 6);

	_provisionBook->addProvision("m1", 5, 9, 10);
	_provisionBook->addProvision("m1", 5, 10, 20);
	_provisionBook->addProvision("m1", 5, 11, 30);
	_provisionBook->addProvision("m1", 10, 13, 40);
	_provisionBook->addProvision("m1", 10, 16, 20);
	_provisionBook->addProvision("m1", 13, 16, 30);
	_provisionBook->addProvision("m1", 13, 19, 10);
	_provisionBook->addProvision("m1", 17, 21, 20);
	_provisionBook->addProvision("m1", 20, 21, 30);

	bool bRet;
	bRet = _provisionBook->bookProvision("m1", 8, 22, 61);
	CHECK_EQUAL(bRet, false);
}

//
// test the book logic
//
TEST(ProvisionBook, FunctionBookTest7)
{	
	_fakeMC->addResource("m1", 150, 6);

	_provisionBook->addProvision("m1", 5, 9, 10);
	_provisionBook->addProvision("m1", 5, 10, 20);
	_provisionBook->addProvision("m1", 5, 11, 30);
	_provisionBook->addProvision("m1", 10, 13, 40);
	_provisionBook->addProvision("m1", 10, 16, 20);
	_provisionBook->addProvision("m1", 13, 16, 30);
	_provisionBook->addProvision("m1", 13, 19, 10);
	_provisionBook->addProvision("m1", 17, 21, 20);
	_provisionBook->addProvision("m1", 20, 21, 30);

	bool bRet;
	bRet = _provisionBook->bookProvision("m1", 8, 22, 50);
	CHECK_EQUAL(bRet, true);
}

//
// test the base instance limit
//
TEST(ProvisionBook, FunctionTest2)
{	
	_fakeMC->addResource("m1", 150, 4);

	_provisionBook->addProvision("m1", 10, 20, 10);
	_provisionBook->addProvision("m1", 10, 20, 50);
	_provisionBook->addProvision("m1", 10, 20, 20);
	_provisionBook->addProvision("m1", 10, 20, 40);

	bool bRet;
	bRet = _provisionBook->bookProvision("m1", 10, 20, 20);
	CHECK_EQUAL(bRet, false);
}

//
// test the base instance/bandwidth limit
//
TEST(ProvisionBook, FunctionTest3)
{	
	_fakeMC->addResource("m2", 122, 5);

	_provisionBook->addProvision("m2", 10, 20, 50);
	_provisionBook->addProvision("m2", 10, 20, 10);
	_provisionBook->addProvision("m2", 10, 20, 30);
	_provisionBook->addProvision("m2", 10, 20, 30);

	bool bRet;

	bRet = _provisionBook->bookProvision("m2", 10, 20, 1);
	CHECK_EQUAL(bRet, true);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 1);
	CHECK_EQUAL(bRet, false);
}

//
// test the book via multiple methods
//
TEST(ProvisionBook, FunctionTest4)
{	
	_fakeMC->addResource("m1", 150, 4);
	_fakeMC->addResource("m2", 122, 5);

	_provisionBook->addProvision("m1", 10, 20, 10);
	_provisionBook->addProvision("m1", 10, 20, 50);
	_provisionBook->addProvision("m1", 10, 20, 20);
	_provisionBook->addProvision("m1", 10, 20, 40);

	bool bRet;

	bRet = _provisionBook->bookProvision("m2", 10, 20, 2);
	CHECK_EQUAL(bRet, false);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 1);
	CHECK_EQUAL(bRet, false);
}

//
// test the book via multiple methods
//
TEST(ProvisionBook, FunctionTest5)
{	
	_fakeMC->addResource("m1", 160, 6);
	_fakeMC->addResource("m2", 122, 5);

	_provisionBook->addProvision("m1", 10, 20, 10);
	_provisionBook->addProvision("m1", 10, 20, 50);
	_provisionBook->addProvision("m1", 10, 20, 20);
	_provisionBook->addProvision("m1", 10, 20, 40);

	bool bRet;

	bRet = _provisionBook->bookProvision("m2", 10, 20, 2);
	CHECK_EQUAL(bRet, true);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 10);
	CHECK_EQUAL(bRet, false);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 5);
	CHECK_EQUAL(bRet, false);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 2);
	CHECK_EQUAL(bRet, false);

}

//
// test the book via multiple methods
//
TEST(ProvisionBook, FunctionTest6)
{	
	_fakeMC->addResource("m1", 150, 6);
	_fakeMC->addResource("m2", 122, 5);

	_provisionBook->addProvision("m1", 10, 20, 10);
	_provisionBook->addProvision("m1", 10, 20, 50);
	_provisionBook->addProvision("m1", 10, 20, 20);

	bool bRet;

	bRet = _provisionBook->bookProvision("m2", 10, 20, 2);
	CHECK_EQUAL(bRet, true);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 10);
	CHECK_EQUAL(bRet, true);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 5);
	CHECK_EQUAL(bRet, false);

	bRet = _provisionBook->bookProvision("m2", 10, 20, 2);
	CHECK_EQUAL(bRet, false);

}
