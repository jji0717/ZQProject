#ifndef __XML_REQUESTHANDLER__
#define __XML_REQUESTHANDLER__

#include "XML_Handler.h"

#define REQUESTLINE "line"
#define REQUESTSKIP "skip"

class XML_RequestHandler : public XML_Handler
{
public:
	XML_RequestHandler();
	~XML_RequestHandler();

	bool getContent(::ZQ::common::XMLPreferenceEx *xmlNode);
	bool getContent(::ZQ::common::XMLUtil::XmlNode xmlNode);

	bool readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode);
	bool readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode);

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	::std::string formatRequest();

	vector<::std::string> RequestVec;

	bool bSkip;
	//::std::string	RequestVec;
};

#endif __XML_REQUESTHANDLER__