// MyWork.cpp: implementation of the MyWork class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyWork.h"

#include "RpcWpValue.h"
using namespace ZQ::rpc;

// these macros register OnTest() function with the UserActionID "Test"
BEGIN_USERMSG_MAP(MyCopyFileWork)
	ON_USERMSG("Test", OnTest)
END_USERMSG_MAP()

MyCopyFileWork::MyCopyFileWork(WorkFactory* factory, const char* TaskTypename, const char* sessionURL)
:BaseWork(factory,TaskTypename,sessionURL)
{
	addExpectedSessionAttr("CopyFileWork.SrceFile");
	addExpectedSessionAttr("CopyFileWork.DestFile");
}

MyCopyFileWork::~MyCopyFileWork()
{
	
}

void MyCopyFileWork::free()
{
#ifdef _DEBUG
	printf("MyCopyFileWork::run() work exiting.\n");
#endif
	BaseWork::free();
}

// you can add your code to begin your work,and this function must be blocked
// these follows code only are sample code
int MyCopyFileWork::run()
{
	//copy file work require two params
	
	if(!isDummySession())
	{
		char szSourFile[MAX_PATH];
		char szDestFile[MAX_PATH];

		ZeroMemory(szSourFile,MAX_PATH);
		ZeroMemory(szDestFile,MAX_PATH);

		/*
		char strTemp[256] = {0};
		_parameters.toXml(strTemp, 255);
		printf("!!!!parameters are: %s\n", strTemp);
		*/

		RpcValue DestFile = _parameters["CopyFileWork.DestFile"];
		RpcValue SourceFile = _parameters["CopyFileWork.SrceFile"];

		SourceFile.ToString(szSourFile,MAX_PATH);
		DestFile.ToString(szDestFile,MAX_PATH);

#ifdef _DEBUG
		printf("MyCopyFileWork::run() got extra parameters\n");
		printf("\t Srce File: %s\n", szSourFile);
		printf("\t Dest File: %s\n", szDestFile);
#endif
		
	}
	
#ifdef _DEBUG
	printf("MyCopyFileWork::run() working...\n");
#endif
	
	//this function is block
	for (int i = 1;i <=200; ++i)
	{
		Sleep(1000);

		RpcValue Attr2,expAttr2,retAttr2;
		Attr2=i;
		updateSession(TASK_PROGRESS_ACTION, Attr2,expAttr2,retAttr2);

	}

	return 0;
}


MyRemoveFileWork::MyRemoveFileWork(WorkFactory* factory, const char* TaskTypename, const char* sessionURL)
:BaseWork(factory,TaskTypename,sessionURL)
{
	addExpectedSessionAttr("RemoveFileWork.SrceFile");
}

MyRemoveFileWork::~MyRemoveFileWork()
{
	
}

void MyRemoveFileWork::free()
{
#ifdef _DEBUG
	printf("MyRemoveFileWork::run() work exiting.\n");
#endif
	BaseWork::free();
}

int MyRemoveFileWork::run()
{
	//copy file work require two params
	
	if(!isDummySession())
	{
		char szSourFile[MAX_PATH];

		ZeroMemory(szSourFile,MAX_PATH);

		/*
		char strTemp[256] = {0};
		_parameters.toXml(strTemp, 255);
		printf("!!!!parameters are: %s\n", strTemp);
		*/

		RpcValue SourceFile = _parameters["RemoveFileWork.SrceFile"];

		SourceFile.ToString(szSourFile,MAX_PATH);

#ifdef _DEBUG
		printf("MyRemoveFileWork::run() got extra parameters\n");
		printf("\t Srce File: %s\n", szSourFile);
#endif
		
	}
	
#ifdef _DEBUG
	printf("MyRemoveFileWork::run() working...\n");
#endif
	
	//this function is block
	for (int i = 1;i <=200; ++i)
	{
		Sleep(1000);

		RpcValue Attr2,expAttr2,retAttr2;
		Attr2=i;
		updateSession(TASK_PROGRESS_ACTION, Attr2,expAttr2,retAttr2);

	}

	return 0;
}




MyFileWorkFactory::MyFileWorkFactory(TaskAcceptor* pTaskApt)
	:WorkFactory(pTaskApt)
{
	workTypeRegister("CopyFileWork", 10);
	workTypeRegister("RemoveFileWork", 10);
}

MyFileWorkFactory::~MyFileWorkFactory()
{
}

BaseWork* MyFileWorkFactory::createWork(const char* workType, const char* sessionURL)
{
	std::string typeStr = workType;
	BaseWork* pRet = NULL;

	// only one type of work is supported now
	if(typeStr == "CopyFileWork")
	{
		if(workTypeAvailable(workType))
		{
			pRet = new MyCopyFileWork(this, workType, sessionURL);
		}
	}
	else if(typeStr == "RemoveFileWork")
	{
		if(workTypeAvailable(workType))
		{
			pRet = new MyRemoveFileWork(this, workType, sessionURL);
		}
	}
	else
	{
	}

	return pRet;
}