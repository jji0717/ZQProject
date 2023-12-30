#include "StdAfx.h"
#include ".\queuemanagement.h"
//#using <mscorlib.dll>

CQueueManageMent::CQueueManageMent(void)
{
	m_queueVect.clear();
}

CQueueManageMent::~CQueueManageMent(void)
{
	
}
BOOL CQueueManageMent::QueueIsExist(CString strQueueName,int nFormat,BOOL IsQueue)
{
	if (m_queueVect.size() >=1)
	{

		zqJmsQueueInfoVector::iterator itrQueue;

		for(itrQueue= m_queueVect.begin();itrQueue != m_queueVect.end();++itrQueue)
		{		
			if ((strQueueName.Compare(itrQueue->strQueueName) == 0) && 
				(itrQueue->nFomrat == nFormat) && (itrQueue->IsQueue == IsQueue))
				/// if all attrib is equal.The queue is exist.
				return TRUE;	
		}
	}
	ZQJmsQueueInfo  info;
	info.IsQueue=IsQueue;
	info.nFomrat=nFormat;
	info.strQueueName=strQueueName;

	m_queueVect.push_back(info);
	return FALSE;
}
