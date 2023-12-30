#ifndef _RTSPTRANSPORTHEADER_H_
#define _RTSPTRANSPORTHEADER_H_

#include "../Common.h"

typedef struct RTSPTransportQamHeader
{
	string	strType;				//like MP2T/DVBC/UDP, etc.
	string	strClient_id;			//<client-id>
	string	strDestination;			//<destination>
	string	strBandwidth;			//<bandwidth>
	string	strQam_name;			//<qam_name>
	string	strQam_group;			//<qam_group>
	string	strAnnex;				//<annex>
	string	strChannel_width;		//<channel_width>
	string	strModulation;			//<modulation>
	string	strInterleaver;			//<interleaver>
}RTSPTransportQamHeader;

typedef vector<RTSPTransportQamHeader> RTSPTransportQamHeaderVec;

typedef struct RTSPTransportUdpHeader
{
	string	strType;				//like MP2T/DVBC/UDP, etc.
	string	strClient_id;			//<client-id>
	string	strQam_destination;		//<qam_destination>
	string	strDestination;			//<destination>
	string	strClient_port;			//<client-port>
	string	strSource;				//<source>
	string	strBandwidth;			//<bandwidth>
	string	strSop_name;			//<sop_name>
	string	strSop_group;			//<sop_group>
	string	strQam_name;			//<qam_name>
	string	strQam_group;			//<qam_group>
	string	strEdge_input_group;	//<edge_input_group>
	string	strAnnex;				//<annex>
	string	strChannel_width;		//<channel_width>
	string	strModulation;			//<modulation>
	string	strInterleaver;			//<interleaver>
	string	strEncryptor_group;		//<encryptor_group>
	string	strEncryptor_name;		//<encryptor_name>
}RTSPTransportUdpHeader;

typedef vector<RTSPTransportUdpHeader> RTSPTransportUdpHeaderVec;

typedef struct RTSPTransportHeader
{
	//RTSPTransportQamHeaderVec TransportQamHeader;
	RTSPTransportUdpHeaderVec TransportUdpHeader;
}RTSPTransportHeader;

//typedef vector<RTSPTransportHeader> RTSPTransportHeaderVec;

#endif