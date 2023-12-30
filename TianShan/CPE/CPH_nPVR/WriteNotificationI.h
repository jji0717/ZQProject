

#ifndef _WriteNotification_INTERFACE_HEADER_
#define _WriteNotification_INTERFACE_HEADER_


#include <string>

class WriteNotificationI
{
public:
	virtual bool notifyWrite(const std::string& file, void* pBuf, int nLen) = 0;
};




#endif