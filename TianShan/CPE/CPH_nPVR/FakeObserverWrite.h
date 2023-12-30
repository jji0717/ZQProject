

#ifndef _Fake_ObserverWrite_HEADER_
#define _Fake_ObserverWrite_HEADER_


#include <string>

class FakeObserverWrite : public ObserverWriteI
{
public:
	virtual bool notifyWrite(const std::string& file, void* pBuf, int nLen)
	{
		printf("This[%08x] notifyWrite\n", this);
		return true;
	}

	virtual void notifyDestroy()
	{
		printf("This[%08x] notifyDestroy\n", this);

		
	}
};


#endif