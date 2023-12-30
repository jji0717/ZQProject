// MyWork.h: interface for the MyWork class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYWORK_H__B9E3B497_92AF_4CBC_AA81_590E3E165C3E__INCLUDED_)
#define AFX_MYWORK_H__B9E3B497_92AF_4CBC_AA81_590E3E165C3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WorkFactory.h"

using namespace ZQ::MPF::WorkNode;

class MyCopyFileWork  : public BaseWork
{
	friend class MyFileWorkFactory;

	DECLARE_USRMSG_MAP()
public:
	MyCopyFileWork(WorkFactory* factory, const char* TaskTypename= "CopyFileWork", const char* sessionURL=NULL);
	virtual ~MyCopyFileWork();

	int		run();

	void	free();
	
private:
	void	OnTest(RpcValue& in, RpcValue& out)
	{ printf("Ok, you have invoked OnTest()\n"); }

};

class MyRemoveFileWork : public BaseWork
{
	friend class MyFileWorkFactory;
public:
	MyRemoveFileWork(WorkFactory* factory, const char* TaskTypename= "RemoveFileWork", const char* sessionURL=NULL);
	virtual ~MyRemoveFileWork();

	int run();
	void free();
	
};

class MyFileWorkFactory : public WorkFactory
{
public:
	MyFileWorkFactory(TaskAcceptor* pTaskApt);
	virtual ~MyFileWorkFactory();

	virtual BaseWork*	createWork(const char* workType, const char* sessionURL);
};
#endif // !defined(AFX_MYWORK_H__B9E3B497_92AF_4CBC_AA81_590E3E165C3E__INCLUDED_)
