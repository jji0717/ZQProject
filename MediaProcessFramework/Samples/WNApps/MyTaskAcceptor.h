// MyTaskAcceptor.h: interface for the MyTaskAcceptor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYTASKACCEPTOR_H__F9791AEE_F58E_4F5A_9575_3369F24FA423__INCLUDED_)
#define AFX_MYTASKACCEPTOR_H__F9791AEE_F58E_4F5A_9575_3369F24FA423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyWork.h"
#include "WorkNode/TaskAcceptor.h"

USE_MPF_WORKNODE_NAMESPACE

class MyTaskAcceptor : public TaskAcceptor
{
public:
	MyTaskAcceptor(int port);
	virtual ~MyTaskAcceptor();

	bool OnTaskRequest(const char* pTaskType,const char*pSessionID
 							,const char*ManageNodeIP,const int ManageNodePort,const char*pTaskReqID);


	BaseWork*m_pWork;
};

#endif // !defined(AFX_MYTASKACCEPTOR_H__F9791AEE_F58E_4F5A_9575_3369F24FA423__INCLUDED_)
