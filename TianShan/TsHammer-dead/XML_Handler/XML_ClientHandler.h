#ifndef __XML_CLIENT_HANDLER__
#define __XML_CLIENT_HANDLER__

#include "XML_Handler.h"

#define RECEIVETHREADS      "receiveThreads"
#define PROCESSTHREADS      "processThreads"
#define SESSIONHREADS       "sessionThreads"

#define TAILORTYPECFG       "tailorType"
#define TAILORRANGE         "tailorRange"

class XML_CLIENTHandler : public XML_Handler
{
public:
	XML_CLIENTHandler();
	~XML_CLIENTHandler();

	bool readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode);
	bool readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode);

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	int             receiveThreads;
	int             processThreads;
	int             sessionThreads;

	std::string     strTailorType;
	int             tailorRange;
};

#endif // end for __XML_CLIENT_HANDLER__
