// BoostRegHandler.cpp: implementation of the BoostRegHandler class.
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable:4786)

#include "ChannelMessageQueue.h"
#include "BoostRegHandler.h"
#include "StringFuncton.h"
#include "Log.h"

using namespace ZQ::common;

#define MAX_LINE_LEN	1024*40
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

BoostRegHandler::BoostRegHandler()
{
}

BoostRegHandler::~BoostRegHandler()
{

}

bool BoostRegHandler::init(int channelID, const char* syntax, ChannelMessageQueue* channelQueue)
{
	if(!BaseMessageHandler::init(channelID, syntax, channelQueue))
		return false;

	try
	{
		_regSyntax.assign(syntax);
	}
	catch(boost::bad_expression& ex)
	{
		glog(Log::L_ERROR, L"Syntax [%S] error at %S", syntax, ex.what());
		return false;
	}

	if (_regSyntax.empty())
		return false;

	return true;
}

bool BoostRegHandler::handleMessage(const char* msg)
{
//	char sOutput[MAX_LINE_LEN];
	
	typedef boost::match_results<std::string::const_iterator> res_t;
	res_t results;
	std::string value;
	
	if (!boost::regex_match(msg, results, _regSyntax))
		return false;
	
	MessageFields* pMessage = new MessageFields();

	std::vector<struct OUTPUT_FIELD>::iterator it;
	for(it=_outputs.begin();it!=_outputs.end();it++)
	{
		std::string output = it->strValue;

		for (int i = results.size() - 1; i >=0; --i)
		{
			value.assign(results[i].first, results[i].second);
			char strbuf[10];
			
			int len = sprintf(strbuf, "$%d", i);
			
			for (int pos = output.find(strbuf, 0); pos != std::string::npos; pos = output.find(strbuf, pos))
				output.replace(pos,len,value.c_str());
		}

		MESSAGE_FIELD mf;
		mf.key = it->strKey;

		// apply string function
		char szValue[512];
		if (StrFuncDispatch(output.c_str(), szValue, sizeof(szValue)))
		{
			mf.value = szValue;
		}
		else
		{
			mf.value = output;
		}
		
		pMessage->push_back(mf);
	}

	if(_channelQueue)
		_channelQueue->addChannelMessage(_channelID, pMessage);

	return true;
}

