#if !defined(AFX_JMSDISPATCHDLL_H__7F879E1B_B172_47F4_B6C2_5D534B2D4156__INCLUDED_)
#define AFX_JMSDISPATCHDLL_H__7F879E1B_B172_47F4_B6C2_5D534B2D4156__INCLUDED_

#pragma once

#ifdef MYJMSDISPATCH_EXPORTS
#define MYJMSDISPATCH_API __declspec(dllexport)
#else
#define MYJMSDISPATCH_API __declspec(dllimport)
#endif

typedef struct JmsDispatchParameter
{
	std::string JbossIpport;
	std::string ConfigQueueName;
	long        comfigTimeOut;	
	int         UsingJboss;
	std::string CacheFolder;
	std::string DataTunnelEndPoint;
}JMSDISPATCHPARAMETER;

extern "C" MYJMSDISPATCH_API
BOOL JmsPortInitialize(JMSDISPATCHPARAMETER& jmsinfo,
					   Ice::CommunicatorPtr& ic,
					   ZQ::common::Log* pLog);

extern "C" MYJMSDISPATCH_API
BOOL JmsPortUnInitialize();

#endif // !defined(AFX_JMSDISPATCHDLL_H__7F879E1B_B172_47F4_B6C2_5D534B2D4156__INCLUDED_)
