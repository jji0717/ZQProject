//#include <Ice/Ice.h>
#include <Dodapp.h>
#include "NativeThread.h"
#include "Locks.h"

typedef struct JmsParameter 
{
	std::string JbossIpport;
	std::string ConfigQueueName;
	long   comfigTimeOut;	
	int    UsingJboss;
	std::string CacheFolder;
	std::string DODEndPoint;
}JMSPARAMETER;

class DODAppThread: public ZQ::common::NativeThread 
{
public:
	DODAppThread();
	virtual ~DODAppThread();	
	virtual int run(void);
	void stop();
	JMSPARAMETER m_jmspar;
};
