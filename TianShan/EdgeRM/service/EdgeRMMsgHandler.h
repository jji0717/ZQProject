#ifndef __ZQTianShan_EdgeRMMsgHandler_H__
#define __ZQTianShan_EdgeRMMsgHandler_H__

#include "RtspInterface.h"
#include <vector>
#include <map>

namespace ZQTianShan {
namespace EdgeRM {

class StringUtil
{
public:
	static std::string trimString(const std::string &s, const std::string &chs = " ");
	static std::string trimWS(const std::string &s);
	static void splitString(std::vector< std::string > &result, const std::string &str, const std::string &delimiter);
};

class RtspMsgHandler :public ::ZQRtspCommon::IHandler
{
public:
	virtual bool HandleMsg(ZQRtspCommon::IRtspReceiveMsg* receiveMsg, ZQRtspCommon::IRtspSendMsg* sendMsg);
private:
	void getTransportElements(const std::string& strTransport, std::vector<std::string> &elements);
	void getTransportElementProperties(const std::string& element, 
		std::map<std::string, std::string>& properties);
};

} // end for EdgeRM
} // end for ZQTianShan
#endif // end for __ZQTianShan_EdgeRMMsgHandler_H__
