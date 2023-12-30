

#ifndef _ObserverWrite_INTERFACE_HEADER_
#define _ObserverWrite_INTERFACE_HEADER_


#include <string>

class ObserverWriteI
{
public:
	virtual bool notifyWrite(const std::string& file, void* pBuf, int nLen) = 0;
	virtual void notifyDestroy() = 0;
};


#endif