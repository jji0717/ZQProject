#include <Ice/Ice.h>
#include <Dodapp.h>
class DODAppThread: public IceUtil::Thread {

public:
	DODAppThread();
	virtual ~DODAppThread();
	virtual void  run();
	void stop();
};
