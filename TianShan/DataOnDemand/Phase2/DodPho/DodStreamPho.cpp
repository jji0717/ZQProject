// DodStreamPho.cpp: implementation of the DodStreamPho class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DodStreamPho.h"
//#include <Ice/IdentityUtil.h>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define RESKEY_STREAMRATE	"totalBandWidth"
#define RESKEY_DESTADDRESS	"destAddress"

static ZQTianShan::ConfItem DodStreamPhoConf[] = {
	{ "totalBandwidth",	::TianShanIce::vtInts,	false, "94371840"}, // 90 Mbps
	{ "network", ::TianShanIce::vtStrings, false, "" },
	{ NULL, ::TianShanIce::vtInts, true, NULL },
};

DodStreamPho::DodStreamPho(ZQTsAcPath::IPHOManager& mgr):
	IStreamLinkPHO(mgr), _phoManager(mgr)
{

}

DodStreamPho::~DodStreamPho()
{

}

/// Implementaions of IPathHelperObject
bool DodStreamPho::getSchema(const char* type, 
	TianShanIce::PDSchema& schema)
{
	_TRACE("enter DodStreamPho::getSchema()");

	if (strncmp(type, DOD_STREAMPHO_NAME, strlen(DOD_STREAMPHO_NAME))) {
		_TRACE("DodStreamPho::getSchema(%s) failed.", type);
		return false;
	}

	TianShanIce::PDElement elem;
	int confCount = sizeof(DodStreamPhoConf) / sizeof(DodStreamPhoConf[0]) - 1;
	
	_TRACE("DodStreamPho::getSchema() confCount = %d\n", confCount);

	for (int i = 0; i < confCount; i ++) {
		_TRACE("DodStreamPho::getSchema() i = %d\n", i);

		::TianShanIce::PDElement elem;
		elem.keyname = DodStreamPhoConf[i].keyname;
		elem.optional = DodStreamPhoConf[i].optional;
		elem.defaultvalue.type= DodStreamPhoConf[i].type;
		schema.push_back(elem);
	}

	_TRACE("leave DodStreamPho::getSchema()\n");
	return true;
}

#ifdef _DEBUG

void dbgPrint(const char* str)
{
	_TRACE("%s\n", str);
}

#endif

void DodStreamPho::validateConfiguration(const char* type, 
	const char* strmLinkIdentStr, 
	::TianShanIce::ValueMap& configPD)
{
	_TRACE("DodStreamPho::validateConfiguration(): %s\n", 
		strmLinkIdentStr);

	if (strncmp(type, DOD_STREAMPHO_NAME, strlen(DOD_STREAMPHO_NAME))) {
		_TRACE("DodStreamPho::getSchema(%s) failed.\n", type);
		return;
	}

#ifdef _DEBUG
	char buf[128];
	sprintf(buf, "%s:", DOD_STREAMPHO_NAME);
	::ZQTianShan::dumpValueMap(configPD, buf, dbgPrint);
#endif

	StrmLinkInfo strmLinInfo;
	::TianShanIce::Variant var;
	try {
		var = ::ZQTianShan::PDField(configPD, "totalBandwidth");
		strmLinInfo.bandWidth = var.ints[0];
		var = ::ZQTianShan::PDField(configPD, "network");
		strmLinInfo.network = var.strs[0];
	} catch(::Ice::Exception& e) {
		glog(ZQLIB::Log::L_ERROR, "DodStreamPho::validateConfiguration(): "
			"occurred a exception: %s", e.ice_name().c_str());
		return;
	}

	strmLinInfo.strmLinkId = strmLinkIdentStr;
	strmLinInfo.remainder = strmLinInfo.bandWidth;
	_strmLinkInfos[strmLinInfo.strmLinkId] = strmLinInfo;
}

ZQTsAcPath::IStorageLinkPHO::NarrowResult DodStreamPho::doNarrow(
	const TsTrans::PathTicketPtr& ticket, 
	const ZQTsAcPath::SessCtx& sess)
{
	return ZQTsAcPath::IStorageLinkPHO::NR_Narrowed;
}

void DodStreamPho::doCommit(const ::TianShanIce::Transport::PathTicketPtr& ticket, 
		const ZQTsAcPath::SessCtx& sessCtx)
{
	_TRACE("DodStreamPho::doCommit()\n");

}

void DodStreamPho::doFreeResources(const TsTrans::PathTicketPtr& ticket)
{
	_TRACE("DodStreamPho::doFreeResource");

	TsTrans::StreamLinkPrx strmLink = NULL;

   try
   { 
	    strmLink =  _phoManager.openStreamLink(ticket->streamLinkIden);
	   if(strmLink == NULL)
	   {
		   glog(ZQLIB::Log::L_ERROR, 
			   "doFreeResource() Stream Link Null Proxy");
		   return;
	   }
   }
   catch (const Ice::Exception& ex)
   {
	   glog(ZQLIB::Log::L_ERROR, 
		   "doFreeResource() cauht ice exception %s", 
		   ex.ice_name().c_str());
	   return;
   }

   if (strmLink->getType() != DOD_STREAMPHO_NAME) {
	   _TRACE("DodStreamPho::doFreeResource()\tUnknown link type");
	   return;
   }

	std::string strmLinkId = strmLink->getIdent().name;

	StrmLinkInfoMap::iterator it = _strmLinkInfos.find(strmLinkId);

	if (it == _strmLinkInfos.end()) {

		glog(ZQLIB::Log::L_ERROR, 
			"doFreeResource() can't find streamlink id = %s", 
			strmLinkId.c_str());

		return;
	}

	StrmLinkInfo& strmLinkInfo = it->second;
	
	int bandwidth = 0;

	if (strmLinkInfo.remainder < strmLinkInfo.bandWidth) {
		TianShanIce::Variant var;		
		try {
			TianShanIce::ValueMap ticketVals = ticket->privateData;
			var = ::ZQTianShan::PDField(ticketVals, RESKEY_STREAMRATE);
			bandwidth = var.ints[0];
		} catch(::Ice::Exception& e) {
			bandwidth = 0;
			glog(ZQLIB::Log::L_ERROR, 
				"DodStreamPho::doFreeResource()\toccurred a exception: %s", 
				e.ice_name().c_str());
		}

		strmLinkInfo.remainder += bandwidth;
	}

	_TRACE("DodStreamPho::doFreeResource() strmLinkId = %s, "
		"baindWidth = %d, remainder = %d", strmLinkId.c_str(), 
		bandwidth, strmLinkInfo.remainder );
}

void split( const std::string& src, char delimiter, 
		   std::vector<std::string>& result)
{
	std::string::const_iterator it, beginPos = src.begin();
	for (it = src.begin(); it != src.end(); it ++) {
		if (*it == delimiter) {
			std::string str(beginPos, it);
			beginPos = it + 1;
			result.push_back(str);
		}
	}

	std::string str(beginPos, it);
	result.push_back(str);
}

static bool matchNetwork(const std::string& dest, const std::string& network)
{
	_TRACE("matchNetwork()\tnetwork: %s, dest: %s", network.c_str(), 
		dest.c_str());

	if (network.size() == 0)
		return true;

	unsigned long destAddr;
	std::vector<std::string> destItems;
	split(dest, ':', destItems);

	if (destItems.size() != 3)
		return true;

	destAddr = inet_addr(destItems[1].c_str());
	if (destAddr == INADDR_NONE)
		return true;

	destAddr = ntohl(destAddr);
	
	std::vector<std::string> nets;
	split(network, ';', nets);
	std::vector<std::string>::iterator it = nets.begin();

	unsigned long addr, maskLen, mask;
	std::vector<std::string> target;
	for (; it != nets.end(); it ++) {
		target.clear();
		split(*it, '/', target);
		if (target.size() == 2) {
			addr = inet_addr(target[0].c_str());
			if (destAddr == INADDR_NONE)
				return true;

			addr = ntohl(addr);
			sscanf(target[1].c_str(), "%u", &maskLen);
			if (maskLen > 32)
				return true;
			
			mask = ~0 << (32 - maskLen);
			if ((destAddr & mask) == addr)
				return true;
		} else {
			return true;
		}
	}

	return false;
}

/// Implementations of IStreamLinkPHO
Ice::Int DodStreamPho::doEvaluation(
			ZQTsAcPath::LinkInfo& linkInfo, const ZQTsAcPath::SessCtx& sessCtx,
			TianShanIce::ValueMap& hintPD, const ::Ice::Int oldCost)
{
	int cost;

	_TRACE("DodStreamPho::doEvaluation()\n");
	TsTrans::StreamLinkPrx strmLink = NULL;
	try
	{
		strmLink = TsTrans::StreamLinkPrx::checkedCast(linkInfo.linkPrx);
	}
	catch(Ice::Exception& ex)
	{
		glog(ZQLIB::Log::L_ERROR, 
			"DodStreamPho::doEvaluation() get StreamLinkPrx errorcode = %s\n",
			ex.ice_name().c_str());
		
		return TsTrans::OutOfServiceCost;
	}

	if (linkInfo.linkType != DOD_STREAMPHO_NAME) {
		_TRACE("Unknown link type\n");
		return TsTrans::OutOfServiceCost;
	}

	std::string strmLinkId = linkInfo.linkIden.name;

	StrmLinkInfoMap::iterator it = _strmLinkInfos.find(strmLinkId);
	if (it == _strmLinkInfos.end()) {

		glog(ZQLIB::Log::L_ERROR, 
			"DodStreamPho::doEvaluation() can't find streamlink id = %s\n", 
			strmLinkId.c_str());

		return TsTrans::OutOfServiceCost;
	}

	int bandwidth = 0;
	std::string dest;

	try {
		TianShanIce::ValueMap privData = sessCtx.privdata;

		TianShanIce::Variant var = ::ZQTianShan::PDField(privData, 
			RESKEY_STREAMRATE);
		bandwidth = var.ints[0];

		var = ::ZQTianShan::PDField(privData, RESKEY_DESTADDRESS);
		dest = var.strs[0];
	} catch(Ice::Exception& e) {

		glog(ZQLIB::Log::L_ERROR, 
			"DodStreamPho::doEvaluation()\toccurred a exception: %s", 
			e.ice_name().c_str());

		cost = TsTrans::OutOfServiceCost;
	}

	StrmLinkInfo& strmLinkInfo = it->second;

	if (!matchNetwork(dest, strmLinkInfo.network)) {
		
		_TRACE("DodStreamPho::doEvaluation()\tnetwork cannot match."
			"network: %s, dest: %s", strmLinkInfo.network.c_str(), 
			dest.c_str());

		cost = TsTrans::OutOfServiceCost;
	} else {
	
		int remainder = strmLinkInfo.remainder - bandwidth;
		if (remainder <= 0) {
			cost = TsTrans::OutOfServiceCost;
		} else {
		
			float x = strmLinkInfo.bandWidth / 10000.0f;
			cost = (int )((strmLinkInfo.bandWidth - remainder) / x);
			strmLinkInfo.remainder -= bandwidth;
		}
	}

	_TRACE("DodStreamPho::doEvaluation() strmLinkId = %s, baindWidth = %d, "
		"remainder = %d, cost = %d\n", strmLinkId.c_str(), bandwidth, 
		strmLinkInfo.remainder, cost );

	return cost;
}
