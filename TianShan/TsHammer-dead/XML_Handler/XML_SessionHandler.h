#ifndef __XML_SESSIONHANDLER__
#define __XML_SESSIONHANDLER__

#include "XML_Handler.h"

#define SESSIONITERATION	"iteration"
#define SESSIONLOOP			"loop"
#define SESSIONINTERVAL		"interval"
#define SESSIONTIMEOUT		"timeout"

class XML_SessionHandler : public XML_Handler
{
public:
	XML_SessionHandler();
	~XML_SessionHandler();

	bool readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode);
	bool readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode);

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	int				iteration;
	int				loop;
	int				interval;
	int				timeout;
};

#endif __XML_SESSIONHANDLER__