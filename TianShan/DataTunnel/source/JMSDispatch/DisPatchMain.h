//#include <Ice/Ice.h>
#include <Tsappdod.h>
#include "NativeThread.h"
#include "Locks.h"
#include "JMSdispatchdll.h"

class JMSdispatchThread: public ZQ::common::NativeThread 
{
public:
	JMSdispatchThread(Ice::CommunicatorPtr& ic);
	virtual ~JMSdispatchThread();	
	virtual int run(void);
	void stop();
	JMSDISPATCHPARAMETER m_jmspar;
	Ice::CommunicatorPtr m_ic;
};
