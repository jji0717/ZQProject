#ifndef _ZQ_ICE_LOG_
#define _ZQ_ICE_LOG_

#include "Log.h"
#include "Ice/Logger.h"

namespace TianShanIce
{
namespace common
{

class IceLogI : public Ice::Logger
{
public:	
    IceLogI(ZQ::common::Log* pLog);
	~IceLogI() {};

public:
    virtual void print(const std::string&);
    virtual void trace(const std::string&, const std::string&);
    virtual void warning(const std::string&);
    virtual void error(const std::string&);

	virtual ::std::string getPrefix(){return "";}
	virtual ::Ice::LoggerPtr cloneWithPrefix(const ::std::string&){ return NULL;}
 private:
	virtual void writeLog(ZQ::common::Log::loglevel_t level, const std::string& msg);
	
	ZQ::common::Log* _pLog;
};
	
typedef IceUtil::Handle<IceLogI> IceLogIPtr;

}}

#endif // _ZQ_ICE_LOG_
