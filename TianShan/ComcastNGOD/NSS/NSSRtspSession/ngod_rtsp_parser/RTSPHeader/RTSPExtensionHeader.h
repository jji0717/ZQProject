#ifndef _RTSPEXTENSIONHEADER_H_
#define _RTSPEXTENSIONHEADER_H_

#include "ZQ_common_conf.h"
#include "../Common.h"

//GET_PARAMETER message header extension
static const char *pGetPramameter_ExtHeader[] = {"presentation_state", "position", "scale", "connection_timeout", "session_list"};
#define INITSTATE	"init"
#define READYSTATE	"ready"
#define PLAYSTATE	"play"
#define PAUSESTATE	"pause"

typedef enum
{
	presentation_state	= 0,
	position			= 1,
	scale				= 2,
	connection_timeout	= 3,
	session_list		= 4
}GETPARAMETER_EXT;

typedef vector<GETPARAMETER_EXT> vecGETPARAMETER_EXT;
typedef struct GetPramameterReq_ExtHeader
{
	vecGETPARAMETER_EXT header;
}GetPramameterReq_ExtHeader;

typedef struct Session_list
{
	string strRTSPSessionID;//<rtsp-session-id>
	string strOnDemandSessionID;//<on-demand-session-id>
}Session_list;

typedef struct GetPramameterRes_ExtHeader
{
	string					strPresentation_state;	//<presentation_state>
	string					strPosition;			//<position>
	string					strScale;				//<scale>
	string					strConnection_timeout;	//<connection_timeout>
	vector<Session_list>	vstrSession_list;		//<session_list>
}GetPramameterRes_ExtHeader;

//use list to support more session group
class FindByRTSPSessionID
{
public:
	FindByRTSPSessionID(string &strRTSPSessionID):m_strRTSPSessionID(strRTSPSessionID){}

	bool operator() (Session_list &pSession_list)
	{
		if (m_strRTSPSessionID.compare(pSession_list.strRTSPSessionID) == 0)
			return true;
		else
			return false;
	}
private:
	string m_strRTSPSessionID;
};

class FindByOnDemandSessionID
{
public:
	FindByOnDemandSessionID(string &strOnDemandSessionID):m_strOnDemandSessionID(strOnDemandSessionID){}

	bool operator() (Session_list &pSession_list)
	{
		if (m_strOnDemandSessionID.compare(pSession_list.strOnDemandSessionID) == 0)
			return true;
		else
			return false;
	}
private:
	string m_strOnDemandSessionID;
};

#endif