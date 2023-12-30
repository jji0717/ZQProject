#ifndef __XML_RTSPSERVERHANDLER__
#define __XML_RTSPSERVERHANDLER__

#include "XML_Handler.h"

#define RTSPSERVERIP		"ip"
#define RTSPSERVERPORT		"port"
#define CONNECTTYPE			"connectType"
#define	SHARECONNECTION		"Share"
#define	PERSESSCONNECTION	"perSess"

class XML_RtspServerHandler : public XML_Handler
{
public:
	XML_RtspServerHandler();
	~XML_RtspServerHandler();

	bool readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode);
	bool readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode);

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	::std::string	ip;
	::std::string	port;
	::std::string	connectType;
};

#endif __XML_RTSPSERVERHANDLER__
