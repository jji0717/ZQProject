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
// ---------------------------------------------------------------------------

#include "CppUTest/TestHarness.h"
#include "RDSBook.h"


using namespace TianShanIce::Storage::PushContentModule;


TEST_GROUP(RDSBook)
{
	RDSBook*				_rdsBook;

	void setup()
	{
		_rdsBook = new RDSBook();
	}
	void teardown()
	{
		delete _rdsBook;
	}
};

TEST(RDSBook, Test1)
{

}

TEST(RDSBook, ExceptionTest1)
{	
	_rdsBook->addRDS("m1", 1, 1);
	_rdsBook->addRDS("m2", 0, 1);
	_rdsBook->addRDS("m3", 1, 0);
	_rdsBook->addRDS("m4", 0, 0);

	bool bRet;

	std::string strIdent;
	unsigned int nPercentLoad;
	RDSBook::RDSCostList costList;
	bRet = _rdsBook->bookRDS(10, 20, 30, strIdent, nPercentLoad, costList);
	CHECK_EQUAL(bRet, false);
}

TEST(RDSBook, ExceptionTest2)
{	
	_rdsBook->addRDS("m4", 0, 0);

	bool bRet;

	std::string strIdent;
	unsigned int nPercentLoad;
	RDSBook::RDSCostList costList;
	bRet = _rdsBook->bookRDS(10, 20, 30, strIdent, nPercentLoad, costList);
	CHECK_EQUAL(bRet, false);
}

//
// test the book logic
//
TEST(RDSBook, FunctionBookTest1)
{	
	_rdsBook->addRDS("m1", 150, 6);

	_rdsBook->addBookedItem("m1", 5, 9, 10);
	_rdsBook->addBookedItem("m1", 5, 10, 20);
	_rdsBook->addBookedItem("m1", 5, 11, 30);
	_rdsBook->addBookedItem("m1", 10, 13, 40);
	_rdsBook->addBookedItem("m1", 10, 16, 20);
	_rdsBook->addBookedItem("m1", 13, 16, 30);
	_rdsBook->addBookedItem("m1", 13, 19, 10);
	_rdsBook->addBookedItem("m1", 17, 21, 20);
	_rdsBook->addBookedItem("m1", 20, 21, 30);

	bool bRet;
	std::string strIdent;
	unsigned int nPercentLoad;

	RDSBook::RDSCostList costList;
	bRet = _rdsBook->bookRDS(10, 20, 60, strIdent, nPercentLoad, costList);
	CHECK_EQUAL(bRet, true);
}

TEST(RDSBook, FunctionBookTest2)
{	
	_rdsBook->addRDS("m1", 150, 6);

	_rdsBook->addBookedItem("m1", 5, 9, 10);
	_rdsBook->addBookedItem("m1", 5, 10, 20);
	_rdsBook->addBookedItem("m1", 5, 11, 30);
	_rdsBook->addBookedItem("m1", 10, 13, 40);
	_rdsBook->addBookedItem("m1", 10, 16, 20);
	_rdsBook->addBookedItem("m1", 13, 16, 30);
	_rdsBook->addBookedItem("m1", 13, 19, 10);
	_rdsBook->addBookedItem("m1", 17, 21, 20);
	_rdsBook->addBookedItem("m1", 20, 21, 30);

	bool bRet;
	std::string strIdent;
	unsigned int nPercentLoad;

	RDSBook::RDSCostList costList;
	bRet = _rdsBook->bookRDS(10, 20, 61, strIdent, nPercentLoad, costList);
	CHECK_EQUAL(bRet, false);
}

TEST(RDSBook, FunctionBookTest3)
{	
	_rdsBook->addRDS("m1", 150, 6);
	_rdsBook->addRDS("m2", 120, 5);

	_rdsBook->addBookedItem("m1", 5, 9, 10);
	_rdsBook->addBookedItem("m1", 5, 10, 20);
	_rdsBook->addBookedItem("m1", 5, 11, 30);
	_rdsBook->addBookedItem("m1", 10, 13, 40);
	_rdsBook->addBookedItem("m1", 10, 16, 20);
	_rdsBook->addBookedItem("m1", 13, 16, 30);
	_rdsBook->addBookedItem("m1", 13, 19, 10);
	_rdsBook->addBookedItem("m1", 17, 21, 20);
	_rdsBook->addBookedItem("m1", 20, 21, 30);

	bool bRet;
	std::string strIdent;
	unsigned int nPercentLoad;

	RDSBook::RDSCostList costList;
	bRet = _rdsBook->bookRDS(10, 20, 60, strIdent, nPercentLoad, costList);
	CHECK_EQUAL(bRet, true);
	CHECK_EQUAL(strIdent=="m2", true);
}

TEST(RDSBook, FunctionBookTest4)
{	
	_rdsBook->addRDS("m1", 150, 6);
	_rdsBook->addRDS("m2", 120, 5);

	_rdsBook->addBookedItem("m1", 5, 9, 10);
	_rdsBook->addBookedItem("m1", 5, 10, 20);
	_rdsBook->addBookedItem("m1", 5, 11, 30);
	_rdsBook->addBookedItem("m1", 10, 13, 40);
	_rdsBook->addBookedItem("m1", 10, 16, 20);
	_rdsBook->addBookedItem("m1", 13, 16, 30);
	_rdsBook->addBookedItem("m1", 13, 19, 10);
	_rdsBook->addBookedItem("m1", 17, 21, 20);
	_rdsBook->addBookedItem("m1", 20, 21, 30);
	_rdsBook->addBookedItem("m2", 10, 20, 60);

	bool bRet;
	std::string strIdent;
	unsigned int nPercentLoad;

	RDSBook::RDSCostList costList;
	bRet = _rdsBook->bookRDS(10, 20, 59, strIdent, nPercentLoad, costList);
	CHECK_EQUAL(bRet, true);
	CHECK_EQUAL(strIdent=="m2", true);
}


TEST(RDSBook, FunctionBookTest5)
{	
	_rdsBook->addRDS("m1", 150, 6);
	_rdsBook->addRDS("m2", 120, 5);

	_rdsBook->addBookedItem("m1", 5, 9, 10);
	_rdsBook->addBookedItem("m1", 5, 10, 20);
	_rdsBook->addBookedItem("m1", 5, 11, 30);
	_rdsBook->addBookedItem("m1", 10, 13, 40);
	_rdsBook->addBookedItem("m1", 10, 16, 20);
	_rdsBook->addBookedItem("m1", 13, 16, 30);
	_rdsBook->addBookedItem("m1", 13, 19, 10);
	_rdsBook->addBookedItem("m1", 17, 21, 20);
	_rdsBook->addBookedItem("m1", 20, 21, 30);
	_rdsBook->addBookedItem("m2", 10, 20, 60);
	_rdsBook->addBookedItem("m2", 10, 20, 60);

	bool bRet;
	std::string strIdent;
	unsigned int nPercentLoad;

	RDSBook::RDSCostList costList;
	bRet = _rdsBook->bookRDS(10, 20, 60, strIdent, nPercentLoad, costList);
	CHECK_EQUAL(bRet, true);
	CHECK_EQUAL(strIdent=="m1", true);
}

TEST(RDSBook, FunctionBookTest6)
{	
	_rdsBook->addRDS("m1", 150, 6);
	_rdsBook->addRDS("m2", 120, 5);
	_rdsBook->addRDS("m3", 200, 5);
	_rdsBook->addRDS("m4", 190, 5);
	_rdsBook->addRDS("m5", 250, 5);

	_rdsBook->addBookedItem("m1", 5, 9, 10);
	_rdsBook->addBookedItem("m1", 5, 10, 20);
	_rdsBook->addBookedItem("m1", 5, 11, 30);
	_rdsBook->addBookedItem("m1", 10, 13, 40);
	_rdsBook->addBookedItem("m1", 10, 16, 20);
	_rdsBook->addBookedItem("m1", 13, 16, 30);
	_rdsBook->addBookedItem("m1", 13, 19, 10);
	_rdsBook->addBookedItem("m1", 17, 21, 20);
	_rdsBook->addBookedItem("m1", 20, 21, 30);
	_rdsBook->addBookedItem("m2", 10, 20, 60);
	_rdsBook->addBookedItem("m2", 10, 20, 60);
	_rdsBook->addBookedItem("m3", 10, 20, 120);
	_rdsBook->addBookedItem("m4", 10, 20, 120);
	_rdsBook->addBookedItem("m5", 10, 20, 190);

	bool bRet;
	std::string strIdent;
	unsigned int nPercentLoad;

	RDSBook::RDSCostList costList;
	bRet = _rdsBook->bookRDS(10, 20, 60, strIdent, nPercentLoad, costList);
	CHECK_EQUAL(bRet, true);
	CHECK_EQUAL(strIdent=="m3", true);
}