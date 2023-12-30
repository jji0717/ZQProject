#ifndef __XML_REQUESTHANDLER__
#define __XML_REQUESTHANDLER__

#include "XML_Handler.h"

#define REQUESTLINE "line"
#define REQUESTSKIP "skip"

//typedef struct
//{
//	std::string strRequest;
//	bool bSkip;
//}RequestNode;

class XML_RequestHandler : public XML_Handler
{
public:
	XML_RequestHandler();
	~XML_RequestHandler();
//public:
//	bool getRequest(::ZQ::common::XMLPreferenceEx *xmlNode);

public:
	bool getContent(::ZQ::common::XMLPreferenceEx *xmlNode);
	bool getContent(::ZQ::common::XMLUtil::XmlNode xmlNode);

	bool readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode);
	bool readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode);

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	std::string strRequest;

	bool bSkip;
	//::std::string	RequestVec;
//private:
//	::std::string formatRequest();

};

#endif __XML_REQUESTHANDLER__