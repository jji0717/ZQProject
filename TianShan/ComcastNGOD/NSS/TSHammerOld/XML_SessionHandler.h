#ifndef __XML_SESSIONHANDLER__
#define __XML_SESSIONHANDLER__

#include "XML_Handler.h"

#define SESSIONSEQID		"seqId"
#define SESSIONDESC			"desc"
#define SESSIONITERATION	"iteration"
#define SESSIONLOOP			"loop"
#define SESSIONINTERVAL		"interval"
#define SESSIONTIMEOUT		"timeout"

typedef struct
{
	::std::string	seqId;
	::std::string	desc;
	int				iteration;
	int				loop;
	int				interval;
	int				timeout;
}SessionNode;

class XML_SessionHandler : public XML_Handler
{
public:
	XML_SessionHandler();
	~XML_SessionHandler();

	bool readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode);
	bool readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode);

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	SessionNode _sessionNode;
};

#endif __XML_SESSIONHANDLER__