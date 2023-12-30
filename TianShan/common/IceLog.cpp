#include "IceLog.h"


namespace TianShanIce
{
namespace common
{

IceLogI::IceLogI(ZQ::common::Log* pLog)
{
	_pLog = pLog;
}

void IceLogI::print(const std::string& msg)
{
	writeLog(ZQ::common::Log::L_INFO, msg);
}

void IceLogI::trace(const std::string& category, const std::string& msg)
{
	std::string s = "[" + category + "]: " + msg;
	if( category.find("Berkeley DB") != std::string::npos )
		writeLog(ZQ::common::Log::L_ERROR, s);
	else
		writeLog(ZQ::common::Log::L_DEBUG, s);
}

void IceLogI::warning(const std::string& msg)
{
	ZQ::common::Log::loglevel_t level = ZQ::common::Log::L_WARNING;
	if (std::string::npos != msg.find("::ConnectionTimeoutException"))
//		|| std::string::npos != msg.find("::ConnectTimeoutException"))
		level = ZQ::common::Log::L_DEBUG;

	writeLog(level, msg);
}

typedef struct {
	char* keyword;
	char* alertmsg;
} ErrorConvert;

static const ErrorConvert FATAL_ERRORS[] = {
	{"Saving thread killed", "Saving thread killded due to damaged DB records, immediate DB maintenance is required" },
	{NULL, NULL} };


void IceLogI::error(const std::string& msg)
{
	writeLog(ZQ::common::Log::L_ERROR, msg);
	_pLog->flush();

	for (int i=0; NULL != FATAL_ERRORS[i].keyword; i++)
	{
		if (std::string::npos == msg.find(FATAL_ERRORS[i].keyword))
			continue;
	
		writeLog(ZQ::common::Log::L_ALERT, FATAL_ERRORS[i].alertmsg);
		return;
	}
}

void IceLogI::writeLog(ZQ::common::Log::loglevel_t level, const std::string& msg)
{
	std::string s = msg;
    for (int pos = s.find_first_of("\r\n"); std::string::npos != pos; pos = s.find_first_of("\r\n", pos+1))
         s.replace(pos, 1, ".");

	if (NULL != _pLog)
		(*_pLog)(level, CLOGFMT(ICE, "%s"), s.c_str());
}


	
}}
