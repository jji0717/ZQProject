// FileName : A3Client.h
// Author   : Zheng Junming
// Date     : 2009-05
// Desc     : 

#ifndef __CRG_PLUGIN_A3SERVER_A3CLIENT_H__
#define __CRG_PLUGIN_A3SERVER_A3CLIENT_H__

#include "HttpClient.h"
#include "Locks.h"

namespace CRG
{
namespace Plugin
{
namespace A3Server
{

class A3Client
{
public:
	A3Client();
	~A3Client();

public:
	int SendRequest(IN const std::string& url, IN OUT std::string& buffer);

private:
	/// forbidden copy and assign
	A3Client(const A3Client& a3Client);
	A3Client& operator=(const A3Client& a3Client);
private:
	ZQ::common::Mutex _lock;
	long _sequence;
};

} // end for A3Server
} // end for Plugin
} // end for CRG

#endif

