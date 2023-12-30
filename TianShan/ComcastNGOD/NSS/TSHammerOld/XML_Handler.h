#ifndef __XML_HANDLER__
#define __XML_HANDLER__

//base class for rtsp script(xml) handler
#include "ConfigHelper.h"
#include "FileLog.h"

#include <sstream>
using namespace std;

#define XMLLOG if (_log) (*_log)
//for all
const int iNameLen = 32;

//for SessionHandler
const int tmpAttrSize = 1024;

//for SessCtxHandler
const int iKeyLen = 32;
const int iValueLen = 1024;

//Element Header
#define	RTSPSERVERElement	"Server"
#define SESSIONElement		"Session"
#define SESSCTXElement		"SessCtx"
#define REQUESTElement		"Request"
#define RESPONSEElement		"Response"
#define SLEEPElement		"Sleep"

class XML_Handler
{
public:
	XML_Handler();
	virtual ~XML_Handler();

	virtual bool getAttributeValue(::ZQ::common::XMLUtil::XmlNode node, const void *attrname, void *value, int maxvaluesize = -1,  int charsize = -1);

	virtual bool getAttributeValue(::ZQ::common::XMLPreferenceEx *node, const void *attrname, void *value, int maxvaluesize = -1,  int charsize = -1);

	virtual void setLogger(::ZQ::common::Log *pLog){ _log = pLog; }

protected:
	::ZQ::common::Log *_log;
};

#endif __XML_HANDLER__