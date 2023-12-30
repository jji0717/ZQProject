// MyTaskAcceptor.cpp: implementation of the MyTaskAcceptor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MyTaskAcceptor.h"
#include "Log.h"


MyTaskAcceptor::MyTaskAcceptor(int port)
:TaskAcceptor(port)
{
	//tell TaskAcceptor what type task can be supported
	//in this work node
	Register("netCopyFile");
	Register("localCopyFile");
}

MyTaskAcceptor::~MyTaskAcceptor()
{
	
}


//here you can create work according task type
bool MyTaskAcceptor::OnTaskRequest(const char* pTaskType,const char*pSessionID
 							,const char*ManageNodeIP,const int ManageNodePort,const char*pTaskReqID)
{
	
	//
	if(stricmp(pTaskType,"netCopyFile")==0)
	{
		//1. first new a work object
		m_pWork=new MyCopyFileWork(this,pTaskReqID,_port);
		
		if(m_pWork==NULL)
		{
			printf("new work fail\n");
			return false;
		}
	
		
		//2. second call setup()
		if(!m_pWork->Setup(pSessionID,ManageNodeIP,ManageNodePort))
		{
			delete m_pWork;
			m_pWork=NULL;
			
			printf("setup work fail\n");
			return false;
		}
		

		//3. then start()
		if(!m_pWork->Start())
		{
			delete m_pWork;
			m_pWork=NULL;
			
			printf("work start fail\n");
			
			return false;
		}
		else return true;
		
	}
		
	return false;
}


