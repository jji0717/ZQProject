
#include <Log.h>
#include <TianShanIceHelper.h>
#include "PhoIpEdge.h"

namespace ZQTianShan {
namespace AccreditedPath {

///schema for STRMLINK_TYPE_IPEDGE_IP
static ConfItem IpEdge_IP[] = {
	{ "TotalBandwidth",		::TianShanIce::vtLongs,		false,	"20000",	false }, // in Kbps
	{ "MaxStreamCount",		::TianShanIce::vtInts,		false,	"80",		false },
	{ "SourceIp" ,			::TianShanIce::vtStrings,	false,	"0.0.0.0",	false },
	{ "DestMac" ,			::TianShanIce::vtStrings,	false,	"",			false },
	{ "SourcePort" ,		::TianShanIce::vtInts ,		false,	"30000",	false },
	{ NULL,					::TianShanIce::vtInts,		true,	"" ,		false },
};

///schema for STRMLINK_TYPE_IPEDGE_IPSHARELINK
static ::ZQTianShan::ConfItem IpEdge_IP_ShareLink[]= {
	{ "LinkId",	::TianShanIce::vtStrings,	false,	"",	false },
	{ NULL,		::TianShanIce::vtInts,		true,	"",	false },
};

///schema for STRMLINK_TYPE_IPEDGE_DVBC
static ::ZQTianShan::ConfItem IpEdge_DVBC[] = {	
	{ "Qam.modulationFormat",	::TianShanIce::vtInts,			false, "0x10" ,				false },
	{ "Qam.IP",					::TianShanIce::vtStrings,		false, "192.168.80.138",	false },
	{ "Qam.Mac",				::TianShanIce::vtStrings,		false,	"a:b:c:d:e:f",		false },
	{ "Qam.basePort",			::TianShanIce::vtInts,			false, "4001",				false },
	{ "Qam.portMask",			::TianShanIce::vtInts,			false, "65280",				false },
	{ "Qam.portStep",			::TianShanIce::vtInts,			false,	"1",				false },
	{ "Qam.symbolRate",			::TianShanIce::vtInts,			false, "50000",				false },
	{ "Qam.frequency",			::TianShanIce::vtInts,			false, "1150",				false },
	{ "PN",						::TianShanIce::vtInts,			false, " 5 ~ 20 ",			true  },
	{ "TotalBandwidth",			::TianShanIce::vtLongs,			false, "20000",				false }, // in Kbps
	{ NULL, ::TianShanIce::vtInts, true, "",false },
};

///schema for STRMLINK_TYPE_IPEDGE_DVBCSHARELINK
static ::ZQTianShan::ConfItem IpEdge_DVBC_ShareLink[]= {
	{"LinkId",	::TianShanIce::vtStrings,	false,	"",	false },
	{NULL,		::TianShanIce::vtInts,		true,	"",	false },
};

PhoIpEdge::PhoIpEdge( IPHOManager& mgr )
:IStreamLinkPHO(mgr)
{
}

PhoIpEdge::~PhoIpEdge(void)
{
}

bool PhoIpEdge::getSchema( const char *type ,::TianShanIce::PDSchema& schema )
{	
	::ZQTianShan::ConfItem *config = NULL;

	if (0 == strcmp(type, STRMLINK_TYPE_IPEDGE_IP))
		config = IpEdge_IP;
	else if (0 == strcmp(type, STRMLINK_TYPE_IPEDGE_DVBC))
		config = IpEdge_DVBC;
	else if ( 0 == strcmp(type, STRMLINK_TYPE_IPEDGE_DVBCSHARELINK) ) 
		config = IpEdge_DVBC_ShareLink;
	else if ( 0 == strcmp(type , STRMLINK_TYPE_IPEDGE_IPSHARELINK ) )
		config = IpEdge_IP_ShareLink;

	// no matches
	if ( NULL == config )
		return false;

	// copy the schema to the output
	for (::ZQTianShan::ConfItem *item = config; item && item->keyname; item ++ )
	{
		::TianShanIce::PDElement elem;
		elem.keyname			= item->keyname;
		elem.optional			= item->optional;
		elem.defaultvalue.type	= item->type;
		elem.defaultvalue.bRange= item->bRange;

		switch(item->type) 
		{
		case TianShanIce::vtInts:
			{				
				elem.defaultvalue.ints.clear();
				if(!elem.defaultvalue.bRange)
				{
					elem.defaultvalue.ints.push_back(atoi(item->hintvalue));
				}				
				else
				{
					int a,b;
					sscanf(item->hintvalue,"%d ~ %d",&a,&b);
					elem.defaultvalue.ints.push_back(a);
					elem.defaultvalue.ints.push_back(b);
				}
			}
			break;
		case TianShanIce::vtLongs:
			{
				elem.defaultvalue.lints.clear();
				if (!elem.defaultvalue.bRange)
				{					
					elem.defaultvalue.lints.push_back( _atoi64(item->hintvalue));
				}
				else
				{
					Ice::Long a,b;
					sscanf(item->hintvalue,"%I64d ~ %I64d",&a,&b);
					elem.defaultvalue.lints.push_back(a);
					elem.defaultvalue.lints.push_back(b);
				}
			}
			break;
		case TianShanIce::vtStrings:
			{
				elem.defaultvalue.strs.clear();
				elem.defaultvalue.strs.push_back(item->hintvalue);
			}
			break;
		default:
			break;
		}
		schema.push_back(elem);
	}

	return true;
}

void PhoIpEdge::validateConfiguration(const char* type, const char* identStr, ::TianShanIce::ValueMap& configPD )
{
	if( type == 0 || type[0] = 0 )
	{
		MLOG(PhoIpEdge,"validateConfiguration() NULL type passed in, rejected");
		return;
	}

}


}}//namespace ZQTianShan::AccreditedPath
