#ifndef _GB_CS_REQ_
#define _GB_CS_REQ_

#include <fstream>
#include "HttpClient.h"
#include "Log.h"
#include "XMLPreferenceEx.h"
#include "TianShanIce.h"
#include <string>
#include <boost/shared_ptr.hpp>

namespace ZQTianShan {
namespace ContentStore {

class IGBCSCmd
{
public:
	IGBCSCmd();
	virtual ~IGBCSCmd(){}

	std::string getUUID(void);

	virtual std::string  makeHttpContent(void);
	virtual std::map<std::string, std::string>  parseHttpResponse(std::string & httpResponse);

public:
	virtual std::string getCmdStr(void) = 0;	

private:
	virtual std::string  makeContentBody(void) = 0;
	virtual std::string  makeContentHeader(void) = 0;

private:
	std::string _uuid;
};


class GBCSCmdUtil
{
public:
	static  std::string  setAttrStr(const char* key, const char* value);
	static  std::string  setAttrStr(const char* key, const std::string & value);
	static  std::string  setAttrStr(const std::string & key, const std::string & value);

	static  std::string  setElementStr(const std::string & tag,      const std::string & attrStr);//<tag  attrStr />
	static  std::string  setElementStr(const std::string & tag,      const std::string & attrStr,  int endTagErase);// <tag  attrStr> </tag>
	static  std::string  setElementStr(const std::string & tagStart, const std::string & entity,   const std::string & tagEnd);//<tag> entity </tag>
	static  std::string  setElementStr(const std::string & tagStart, const std::string attrStr,    const std::string & entity, const std::string & tagEnd);//<tag attrStr> entity </tag>

public:
	typedef enum _XmlParserStatus{
		SUCCEED       = 0,
		INPUT_XML_EMPTY  ,
		READ_FAILED      ,	
		GET_ROOTPREFERENCE_FAILED,
		GET_HEADER_FAILED,
		GET_BODY_FAILED,
		OPCODE_FAILED,
		STATUS_COUNT
	}XmlParserStatus;

	static char * _parserStatus[STATUS_COUNT + 1];
};

class   GBCSReq
{
public:
	explicit GBCSReq(ZQ::common::Log*  log, std::string  reqHost);

	int  sendRequest(int timeout = 0);
	int  sendRequest(IGBCSCmd*  reqGBCmd, int timeout = 0);
	int  setHttpHeader(char* key = NULL, char* val = NULL);

	std::map<std::string, std::string>      getStatusMsg(void);
    std::string  getHttpReponseBuf(void){return _reqHttpReponseBuf;}

	IGBCSCmd* setReqGBCmd(IGBCSCmd*  reqGBCmd){return  _reqGBCmd = reqGBCmd;}

	int setHttpReponseBuf(const std::string& httpResponse)
	{
		_reqHttpReponseBuf = httpResponse;
		return true;
	}

private:
	int  httpSendRequest(void);

private:
	IGBCSCmd*     _reqGBCmd;

	std::string   _reqHost;
	std::string   _reqHttpSendBuf;
	std::string   _reqHttpReponseBuf;

	ZQ::common::Log*  _log;
	ZQ::common::HttpClient  _http;

	static ZQ::common::Mutex _lockXmlParse;
};


typedef IGBCSCmd  IGBA4Req;
typedef IGBCSCmd  IGBA5Req;
typedef boost::shared_ptr< IGBCSCmd > GBCSCmdPtr;
}//namespace  ContentStore
}//	namespace ZQTianShan 

#endif//_GB_CS_REQ_