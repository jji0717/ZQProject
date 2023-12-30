#ifndef __XML_RESPONSEHANDLER__
#define __XML_RESPONSEHANDLER__

#include "XML_Handler.h"
#include "XML_SessCtxHandler.h"

#define RESPONSENAME "name"
#define RESPONSESYNTAX "syntax"

typedef struct
{
	string name;
	string syntax;
	string key;
	string value;
}ResponseNode;

typedef vector<ResponseNode> ResponseNodeVector;

class XML_ResponseHandler : public XML_Handler
{
public:
	XML_ResponseHandler();
	~XML_ResponseHandler();

	bool getResponseSessCtx(::ZQ::common::XMLPreferenceEx *xmlNode);
	bool getResponseSessCtx(::ZQ::common::XMLUtil::XmlNode xmlNode);

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	void updateSessCtxHandler(XML_SessCtxHandler &xml_SessCtxHandler);

	bool parseResponse(const ::std::string &strMsg);

	ResponseNodeVector ResponseVec;
};

#endif __XML_RESPONSEHANDLER__