#ifndef __XML_RTSPSERVERHANDLER__
#define __XML_RTSPSERVERHANDLER__

#include "XML_Handler.h"
#include "RTSP_common_structure.h"

#define RTSPSERVERIP		"ip"
#define RTSPSERVERPORT		"port"
#define CONNECTTYPE			"connectType"
#define	SHARECONNECTION		"Share"
#define	PERSESSCONNECTION	"perSess"
#define BUFFERSIZE			"bufferSize"

typedef struct
{
	::std::string	ip;
	int				port;
	::std::string	type;
	int				bufferSize;
}RtspServerNode;

class XML_RtspServerHandler : public XML_Handler
{
public:
	XML_RtspServerHandler();
	~XML_RtspServerHandler();

	bool readAttribute(::ZQ::common::XMLUtil::XmlNode xmlNode);
	bool readAttribute(::ZQ::common::XMLPreferenceEx *xmlNode);

	void setLogger(::ZQ::common::Log *pLog){XML_Handler::setLogger(pLog);}

	RtspServerNode _rtspServerNode;

	bool initSessionSocket();
	bool connectServer();
	SessionSocket	_sessionSocket;
};

#endif __XML_RTSPSERVERHANDLER__