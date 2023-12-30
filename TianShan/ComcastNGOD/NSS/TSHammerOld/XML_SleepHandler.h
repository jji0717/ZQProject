#ifndef __XML_SLEEPHANDLER__
#define __XML_SLEEPHANDLER__

#include "XML_Handler.h"

#define SLEEPWAIT		"wait"

typedef struct
{
	int		wait;
}SleepNode;

class XML_SleepHandler : public XML_Handler
{
public:
	XML_SleepHandler();
	~XML_SleepHandler();

	bool readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode);
	bool readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode);

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	SleepNode _sleepNode;
};

#endif __XML_SLEEPHANDLER__