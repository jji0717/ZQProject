#ifndef _RTSPR2HEADER_H_
#define	_RTSPR2HEADER_H_

#include "ZQ_common_conf.h"
#include "../Common.h"
#include "RTSPR6Header.h"
#include "RTSPNoticeHeader.h"
#include "RTSPPolicyHeader.h"
#include "RTSPInBandMarkerHeader.h"
#include "RTSPReasonHeader.h"
#include "RTSPRequierHeader.h"
#include "RTSPS4Header.h"
#include "RTSPSessionGroupHeader.h"
#include "RTSPStartPointHeader.h"
#include "RTSPStopPointHeader.h"
#include "RTSPStreamControlProtoHeader.h"
#include "RTSPTransportHeader.h"
#include "RTSPVolumeHeader.h"
#include "SDPContent.h"
#include "RTSPExtensionHeader.h"

typedef struct RTSPR2Header
{
	string					strSessionID;			//Session:(given by Video Server)
	//required header
	string					strClientSessionID;		//ClientSessionId:
	RTSPNoticeHeaderVec		Notice;					//Notice:
	string					strOnDemandSessionID;	//OnDemandSessionId:
	RTSPReasonHeader		Reason;					//Reason:
	RTSPRequireHeader		Require;				//Require:

	//optional header
	RTSPSessionGroupHeader	SessionGroup;			//SessionGroup:
	RTSPSessionGroupsHeader m_SessionGroups;		//session_groups:
	RTSPPolicyHeaderVec		Policy;					//Policy:
	RTSPStreamControlProto	StreamControlProto;		//StreamControlProto:
	RTSPInBandMarker		InBandMarker;
	RTSPVolumeHeader		Volume;					//Volume:
	RTSPR6Header			m_R6Header;				//R6 Header
	//Sop;
	//SopGroup;
	RTSPS4Header			m_S4Header;				//S4 Header

	RTSPTransportHeader		m_RTSPTransportHeader;	//setup request transport content
	SDPRequestContentVec	m_SDPRequestContent;	//setup request sdp content
	SDPResponseContent		m_SDPResponseContent;	//setup response sdp content

	//specify message header extensions
	GetPramameterReq_ExtHeader m_GetPramameterReq_ExtHeader;	//Get_Parameter request extension
	GetPramameterRes_ExtHeader m_GetPramameterRes_ExtHeader;	//Get_Parameter response extension
}RTSPR2Header;

#endif