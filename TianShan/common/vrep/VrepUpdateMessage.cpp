#include "VrepUpdateMessage.h"
#include <sstream>
#include <assert.h>

namespace ZQ {
namespace Vrep {
#define BytesToString(BS) std::string(BS.begin(), BS.end())
#define StringToBytes(STR) bytes(STR.begin(), STR.end())

#define Attr_Pos_Generic 0
#define Attr_Pos_WithdrawnRoutes 1
#define Attr_Pos_ReachableRoutes 2
#define Attr_Pos_NextHopServer 3
#define Attr_Pos_QAMNames 4
#define Attr_Pos_CASCapability 5
#define Attr_Pos_TotalBandwidth 6
#define Attr_Pos_AvailableBandwidth 7
#define Attr_Pos_Cost 8
#define Attr_Pos_EdgeInput 9
#define Attr_Pos_QAMParameters 10
#define Attr_Pos_UDPMap 11
#define Attr_Pos_Volume 12
#define Attr_Pos_ServiceStatus 13
#define Attr_Pos_MaxMPEGFlows 14
#define Attr_Pos_NextHopServerAlternates 15
#define Attr_Pos_OutputPort 16
#define Attr_Pos_OutputAddress 17
#define Attr_Pos_TransferProtocolCapabilities 18


UpdateAttributes::UpdateAttributes() {
    clear();
}
UpdateAttributes::~UpdateAttributes() {
}
void UpdateAttributes::setWithdrawnRoutes(const Routes& routes) {
    withdrawnRoutes_ = routes;
    setting_.set(Attr_Pos_WithdrawnRoutes);
}
bool UpdateAttributes::getWithdrawnRoutes(Routes& routes) const {
    if(setting_.test(Attr_Pos_WithdrawnRoutes)) {
        routes = withdrawnRoutes_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setReachableRoutes(const Routes& routes) {
    reachableRoutes_ = routes;
    setting_.set(Attr_Pos_ReachableRoutes);
}
bool UpdateAttributes::getReachableRoutes(Routes& routes) const {
    if(setting_.test(Attr_Pos_ReachableRoutes)) {
        routes = reachableRoutes_;
        return true;
    } else {
        return false;
    }
}
void UpdateAttributes::setNextHopServer(const NextHopServer& srv) {
    nextHopServer_ = srv;
    setting_.set(Attr_Pos_NextHopServer);
}
bool UpdateAttributes::getNextHopServer(NextHopServer& srv) const {
    if(setting_.test(Attr_Pos_NextHopServer)) {
        srv = nextHopServer_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setNextHopServerAlternates(const ServerAlternates& alts) {
    nhsAlternates_ = alts;
    setting_.set(Attr_Pos_NextHopServerAlternates);
}
bool UpdateAttributes::getNextHopServerAlternates(ServerAlternates& alts) const {
    if(setting_.test(Attr_Pos_NextHopServerAlternates)) {
        alts = nhsAlternates_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setQAMNames(const QAMNames& qamNames) {
    qamNames_ = qamNames;
    setting_.set(Attr_Pos_QAMNames);
}
bool UpdateAttributes::getQAMNames(QAMNames& qamNames) const {
    if(setting_.test(Attr_Pos_QAMNames)) {
        qamNames = qamNames_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setQAMParameters(const QAMParameters& qamParams) {
    qamParameters_ = qamParams;
    setting_.set(Attr_Pos_QAMParameters);
}
bool UpdateAttributes::getQAMParameters(QAMParameters& qamParams) const {
    if(setting_.test(Attr_Pos_QAMParameters)) {
        qamParams = qamParameters_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setTotalBandwidth(dword bw) {
    totalBandwidth_ = bw;
    setting_.set(Attr_Pos_TotalBandwidth);
}

bool UpdateAttributes::getTotalBandwidth(dword& bw) const {
    if(setting_.test(Attr_Pos_TotalBandwidth)) {
        bw = totalBandwidth_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setAvailableBandwidth(dword bw) {
    availableBandwidth_ = bw;
    setting_.set(Attr_Pos_AvailableBandwidth);
}
bool UpdateAttributes::getAvailableBandwidth(dword& bw) const {
    if(setting_.test(Attr_Pos_AvailableBandwidth)) {
        bw = availableBandwidth_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setCost(byte cost) {
    cost_ = cost;
    setting_.set(Attr_Pos_Cost);
}

bool UpdateAttributes::getCost(byte& cost) const {
    if(setting_.test(Attr_Pos_Cost)) {
        cost = cost_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setVolumes(const Volumes& vols) {
    volumes_ = vols;
    setting_.set(Attr_Pos_Volume);
}
bool UpdateAttributes::getVolumes(Volumes& vols) {
    if(setting_.test(Attr_Pos_Volume)) {
        vols = volumes_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setUDPMap(const UDPMap& udpMap) {
    udpMap_ = udpMap;
    setting_.set(Attr_Pos_UDPMap);
}
bool UpdateAttributes::getUDPMap(UDPMap& udpMap) const {
    if(setting_.test(Attr_Pos_UDPMap)) {
        udpMap = udpMap_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setServiceStatus(dword st) {
    serviceStatus_ = st;
    setting_.set(Attr_Pos_ServiceStatus);
}
bool UpdateAttributes::getServiceStatus(dword& st) const {
    if(setting_.test(Attr_Pos_ServiceStatus)) {
        st = serviceStatus_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setMaxMPEGFlows(dword nFlows) {
    maxMPEGFlows_ = nFlows;
    setting_.set(Attr_Pos_MaxMPEGFlows);
}
bool UpdateAttributes::getMaxMPEGFlows(dword& nFlows) const {
    if(setting_.test(Attr_Pos_MaxMPEGFlows)) {
        nFlows = maxMPEGFlows_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setOutputPort(dword port) {
    outputPort_ = port;
    setting_.set(Attr_Pos_OutputPort);
}
bool UpdateAttributes::getOutputPort(dword& port) const {
    if(setting_.test(Attr_Pos_OutputPort)) {
        port = outputPort_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setOutputAddress(const bytes& addr) {
    outputAddress_ = addr;
    setting_.set(Attr_Pos_OutputAddress);
}
bool UpdateAttributes::getOutputAddress(bytes& addr) const {
    if(setting_.test(Attr_Pos_OutputAddress)) {
        addr = outputAddress_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setTransferProtocolCapabilities(byte cap) {
    transferProtocolCap_ = cap;
    setting_.set(Attr_Pos_TransferProtocolCapabilities);
}
bool UpdateAttributes::getTransferProtocolCapabilities(byte& cap) const {
    if(setting_.test(Attr_Pos_TransferProtocolCapabilities)) {
        cap = transferProtocolCap_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setGenericAttributes(const GenericAttributes& attrs) {
    genericAttrs_ = attrs;
    setting_.set(Attr_Pos_Generic);
}
bool UpdateAttributes::getGenericAttributes(GenericAttributes& attrs) const {
    if(setting_.test(Attr_Pos_Generic)) {
        attrs = genericAttrs_;
        return true;
    } else {
        return false;
    }
}

void UpdateAttributes::setEdgeInputs(const EdgeInputs& edgeInputs){
    edgeInputs_ = edgeInputs;
    setting_.set(Attr_Pos_EdgeInput);
}
bool UpdateAttributes::getEdgeInputs(EdgeInputs& edgeInputs) const{
    if(setting_.test(Attr_Pos_EdgeInput)){
        edgeInputs = edgeInputs_;
        return true;
    }else{
        return false;
    }
}

bool UpdateAttributes::validate() const {
    // check the conditional mandatory
    if(setting_.test(Attr_Pos_ReachableRoutes)) {
        if(!setting_.test(Attr_Pos_NextHopServer))
            return false;

        // these are other attributes should be checked in D4, D5 and D6
        // but here is not a good place to do that.
    }
    return true;
}
static void buildByte(bytes& data, byte b) {
    data.clear();
    data.push_back(b);
}
static int parseByte(byte& b, const byte* data, size_t len) {
    if(data && len > 0) {
        b = data[0];
        return 1;
    } else {
        return -1;
    }
}
static void buildWord(bytes& data, word w) {
    data.clear();
    data.resize(2);
    MessageHelper helper(&data[0], data.size());
    helper.putWord(w, 0);
}
static int parseWord(word& w, const byte* data, size_t len) {
    if(data && len > 0) {
        MessageHelper helper((byte*)data, len);
        return helper.getWord(w, 0);
    } else {
        return -1;
    }
}
static void buildDword(bytes& data, dword dw) {
    data.clear();
    data.resize(4);
    MessageHelper helper(&data[0], data.size());
    helper.putDword(dw, 0);
}
static int parseDword(dword& dw, const byte* data, size_t len) {
    if(data && len > 0) {
        MessageHelper helper((byte*)data, len);
        return helper.getDword(dw, 0);
    } else {
        return -1;
    }
}
static int buildAttribute(byte* buf, size_t len, const GenericAttribute& attr) {
    if(VREP_MsgSize_UPDATEAttributeHeader + attr.data.size() <= len) {
        UPDATEAttributeHeader hdr;
        hdr.flags = attr.flags;
        hdr.type = attr.type;
        hdr.length = attr.data.size();
        size_t offset = buildUPDATEAttributeHeader(buf, len, hdr);
        MessageHelper helper(buf, len);
        offset += helper.put(attr.data, offset);
        return (int)offset;
    } else {
        return -1;
    }
}

static void buildRoutes(bytes& data, const Routes& routes) {
    data.clear();
    if(routes.empty()) {
        return;
    }

    // get the routes size
    size_t nSize = 0;
    for(size_t i = 0; i < routes.size(); ++i) {
        // TODO: The RouteName field is not clear
        nSize += (4 + 2 + routes[i].address.size()/* + 2 + routes[i].name.size()*/);
    }
    data.resize(nSize);
    MessageHelper helper(&data[0], data.size());
    size_t offset = 0;

    for(size_t i = 0; i < routes.size(); ++i) {
        offset += helper.putWord(routes[i].family, offset);
        offset += helper.putWord(routes[i].protocol, offset);
        // address
        offset += helper.putWord((word)routes[i].address.size(), offset);
        offset += helper.put(routes[i].address, offset);

        // TODO: The RouteName field is not clear
        // route name
        //offset += helper.putWord((word)routes[i].name.size(), offset);
        //offset += helper.put(routes[i].name, offset);
    }
}
static int parseRoutes(Routes& routes, const byte* data, size_t len) {
    if(NULL == data) {
        return -1;
    }
    routes.clear();
    size_t offset = 0;
    MessageHelper helper((byte*)data, len);
    while(offset < len) {
#define VREP_ParseFieldOrDie(_FIELD, _TYPE, _OFF, _DIEVAL) {\
    int tmpLenOfField = helper.get##_TYPE(_FIELD, _OFF);\
    if(tmpLenOfField >= 0) _OFF += tmpLenOfField; else return _DIEVAL;\
    }
#define VREP_ParseBytesOrDie(_FIELD, _Len, _OFF, _DIEVAL) {\
    int tmpLenOfField = helper.get(_FIELD, _Len, _OFF);\
    if(tmpLenOfField >= 0) _OFF += tmpLenOfField; else return _DIEVAL;\
    }
        Route r;
        VREP_ParseFieldOrDie(r.family, Word, offset, -1);
        VREP_ParseFieldOrDie(r.protocol, Word, offset, -1);
        word addrLen = 0;
        VREP_ParseFieldOrDie(addrLen, Word, offset, -1);
        VREP_ParseBytesOrDie(r.address, addrLen, offset, -1);
        // TODO: The RouteName field is not clear
        //word routeNameLen = 0;
        //VREP_ParseFieldOrDie(routeNameLen, Word, offset, -1);
        //VREP_ParseBytesOrDie(r.name, routeNameLen, offset, -1);
        routes.push_back(r);
    }
    return (int)offset;
}
static void buildNextHopServer(bytes& data, const NextHopServer& nhs) {
    data.clear();
    size_t nSize = 4 + 2 + nhs.componentAddress.size() + 2 + nhs.streamingZone.size();
    data.resize(nSize);
    MessageHelper helper(&data[0], data.size());
    size_t offset = 0;

    offset += helper.putDword(0, offset); // reserved
    // component address
    offset += helper.putWord((word)nhs.componentAddress.size(), offset);
    offset += helper.put(nhs.componentAddress, offset);
    // streaming zone name
    offset += helper.putWord((word)nhs.streamingZone.size(), offset);
    offset += helper.put(nhs.streamingZone, offset);
}

static int parseNextHopServer(NextHopServer& nhs, const byte* data, size_t len) {
    if(NULL == data) {
        return -1;
    }
    size_t offset = 0;
    MessageHelper helper((byte*)data, len);
    dword reserved = 0;
    VREP_ParseFieldOrDie(reserved, Dword, offset, -1);
    word compAddrLen = 0;
    VREP_ParseFieldOrDie(compAddrLen, Word, offset, -1);
    VREP_ParseBytesOrDie(nhs.componentAddress, compAddrLen, offset, -1);
    word szNameLen = 0;
    VREP_ParseFieldOrDie(szNameLen, Word, offset, -1);
    VREP_ParseBytesOrDie(nhs.streamingZone, szNameLen, offset, -1);
    return (int)offset;
}
static void buildQAMNames(bytes& data, const QAMNames& qamNames) {
    data.clear();
    size_t nSize = 0;
    for(size_t i = 0; i < qamNames.size(); ++i) {
        nSize += 2 + qamNames[i].size();
    }
    data.resize(nSize);
    MessageHelper helper(&data[0], data.size());
    size_t offset = 0;
    for(size_t i = 0; i < qamNames.size(); ++i) {
        const std::string& name = qamNames[i];
        offset += helper.putWord((word)name.size(), offset); // len
        offset += helper.put(StringToBytes(name), offset); // name
    }
}

static int parseQAMNames(QAMNames& qamNames, const byte* data, size_t len) {
    if(NULL == data) {
        return -1;
    }
    qamNames.clear();
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    while(offset < len) {
        word nameLen = 0;
        VREP_ParseFieldOrDie(nameLen, Word, offset, -1);
        bytes name;
        VREP_ParseBytesOrDie(name, nameLen, offset, -1);
        qamNames.push_back(BytesToString(name));
    }
    return (int)offset;
}

static void buildEdgeInputs(bytes& data,const EdgeInputs edgeInputs)
{
	data.clear();
	size_t nSize = 0;
	for(size_t i = 0; i < edgeInputs.size();++i)
	{
		nSize += 12 + edgeInputs[i].host.size() + edgeInputs[i].groupName.size();
	}
	data.resize(nSize);
	MessageHelper helper(&data[0],nSize);
	size_t offset = 0;
	for(size_t i=0; i < edgeInputs.size(); ++i)
	{
		offset += helper.putDword(edgeInputs[i].subnetMask,offset);
		offset += helper.putWord((word)edgeInputs[i].host.size(),offset);
		offset += helper.put(edgeInputs[i].host,offset);
		offset += helper.putDword(edgeInputs[i].portId,offset);
		offset += helper.putDword(edgeInputs[i].maxBW,offset);
		offset += helper.putWord((word)edgeInputs[i].groupName.size(),offset);
		offset += helper.put(edgeInputs[i].groupName,offset);
	}
}
static int parseEdgeInputs(EdgeInputs& edgeInputs, const byte * data, size_t len)
{
	if(NULL == data)
		return -1;
	edgeInputs.clear();
	MessageHelper helper((byte*)data,len);
	size_t offset = 0;
	while(offset < len)
	{
		EdgeInput edgeInput;
		dword subnetMask = 0;
		VREP_ParseFieldOrDie(subnetMask, Dword, offset, -1);
		edgeInput.subnetMask = subnetMask;
		word hostLenth = 0;
		VREP_ParseFieldOrDie(hostLenth, Word, offset, -1);
		bytes host;
		VREP_ParseBytesOrDie(host, hostLenth, offset, -1);
		edgeInput.host = host;
		dword portID = 0;
		VREP_ParseFieldOrDie(portID, Dword, offset, -1);
		edgeInput.portId = portID;
		dword maxBW = 0;
		VREP_ParseFieldOrDie(maxBW, Dword, offset, -1);
		edgeInput.maxBW = maxBW;
		word groupNameLength = 0;
		VREP_ParseFieldOrDie(groupNameLength, Word, offset, -1);
		bytes groupName;
		VREP_ParseBytesOrDie(groupName, groupNameLength, offset, -1);
		edgeInput.groupName = groupName;
		edgeInputs.push_back(edgeInput);
	}
	return (int)offset;
}

#define VREP_QAMParameters_Reserved 0xFFFF
static void buildQAMParameters(bytes& data, const QAMParameters& qamParams) {
    data.clear();
    size_t nSize = 12;
    data.resize(nSize);
    MessageHelper helper(&data[0], data.size());
    size_t offset = 0;
    offset += helper.putDword(qamParams.frequencyKHz, offset);
    offset += helper.putByte(qamParams.modulationMode, offset);
    offset += helper.putByte(qamParams.interleaver, offset);
    offset += helper.putWord(qamParams.tsid, offset);
    offset += helper.putByte(qamParams.annex, offset);
    offset += helper.putByte(qamParams.channelWidth, offset);
    offset += helper.putWord(VREP_QAMParameters_Reserved, offset);
}
static int parseQAMParameters(QAMParameters& qamParams, const byte* data, size_t len) {
    if(NULL == data) {
        return -1;
    }
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    VREP_ParseFieldOrDie(qamParams.frequencyKHz, Dword, offset, -1);
    VREP_ParseFieldOrDie(qamParams.modulationMode, Byte, offset, -1);
    VREP_ParseFieldOrDie(qamParams.interleaver, Byte, offset, -1);
    VREP_ParseFieldOrDie(qamParams.tsid, Word, offset, -1);
    VREP_ParseFieldOrDie(qamParams.annex, Byte, offset, -1);
    VREP_ParseFieldOrDie(qamParams.channelWidth, Byte, offset, -1);
    word reserved = 0;
    VREP_ParseFieldOrDie(reserved, Word, offset, -1);

    return (int)offset;
}

static void buildUDPMap(bytes& data, const UDPMap& udpMap) {
    data.clear();
    size_t nSize = 4 + 4 * udpMap.staticPorts.size() + 4 + 8 * udpMap.dynamicPorts.size();
    data.resize(nSize);
    MessageHelper helper(&data[0], data.size());
    size_t offset = 0;
    offset += helper.putDword(udpMap.staticPorts.size(), offset);
    for(size_t i = 0; i < udpMap.staticPorts.size(); ++i) {
        const PortMapItem& port = udpMap.staticPorts[i];
        offset += helper.putWord(port.port, offset);
        offset += helper.putWord(port.pn, offset);
    }
    offset += helper.putDword(udpMap.dynamicPorts.size(), offset);
    for(size_t i = 0; i < udpMap.dynamicPorts.size(); ++i) {
        const PortMapItem& port = udpMap.dynamicPorts[i];
        offset += helper.putWord(port.port, offset);
        offset += helper.putWord(port.pn, offset);
        offset += helper.putDword(port.count, offset);
    }
}
static int parseUDPMap(UDPMap& udpMap, const byte* data, size_t len) {
    if(NULL == data) {
        return -1;
    }
    udpMap.staticPorts.clear();
    udpMap.dynamicPorts.clear();
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    dword staticPortsCount = 0;
    VREP_ParseFieldOrDie(staticPortsCount, Dword, offset, -1);
    for(dword i = 0; i < staticPortsCount; ++i) {
        PortMapItem mapItem;
        VREP_ParseFieldOrDie(mapItem.port, Word, offset, -1);
        VREP_ParseFieldOrDie(mapItem.pn, Word, offset, -1);
        mapItem.count = 1;
        udpMap.staticPorts.push_back(mapItem);
    }
    dword dynamicPortsCount = 0;
    VREP_ParseFieldOrDie(dynamicPortsCount, Dword, offset, -1);
    for(dword i = 0; i < dynamicPortsCount; ++i) {
        PortMapItem mapItem;
        VREP_ParseFieldOrDie(mapItem.port, Word, offset, -1);
        VREP_ParseFieldOrDie(mapItem.pn, Word, offset, -1);
        VREP_ParseFieldOrDie(mapItem.count, Dword, offset, -1);
        udpMap.dynamicPorts.push_back(mapItem);
    }

    return (int)offset;
}

static void buildVolume(bytes& data, const Volumes& volumes) {
    data.clear();
    if(volumes.empty())
        return;

    size_t nSize = 0;
    for(size_t i = 0; i < volumes.size(); ++i) {
        nSize += 14 + volumes[i].name.size();
    }

    data.resize(nSize);
    MessageHelper helper(&data[0], data.size());
    size_t offset = 0;
    for(size_t i = 0; i < volumes.size(); ++i) {
        const Volume& vol = volumes[i];
        offset += helper.putWord((word)vol.name.size(), offset);
        offset += helper.put(vol.name, offset);
        offset += helper.putDword(vol.portId, offset);
        offset += helper.putDword(vol.readBw, offset);
        offset += helper.putDword(vol.writeBw, offset);
    }
}
static int parseVolume(Volumes& volumes, const byte* data, size_t len) {
    if(NULL == data) {
        return -1;
    }
    volumes.clear();
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    while(offset < len) {
        Volume vol;
        word nameLen = 0;
        VREP_ParseFieldOrDie(nameLen, Word, offset, -1);
        VREP_ParseBytesOrDie(vol.name, nameLen, offset, -1);
        VREP_ParseFieldOrDie(vol.portId, Dword, offset, -1);
        VREP_ParseFieldOrDie(vol.readBw, Dword, offset, -1);
        VREP_ParseFieldOrDie(vol.writeBw, Dword, offset, -1);
    }

    return (int)offset;
}

static void buildNextHopServerAlts(bytes& data, const ServerAlternates& alts) {
    data.clear();
    size_t nSize = 2;
    for(size_t i = 0; i < alts.size(); ++i) {
        nSize += 2 + alts[i].size();
    }

    data.resize(nSize);
    MessageHelper helper(&data[0], data.size());
    size_t offset = 0;
    offset += helper.putWord((word)alts.size(), offset); // NumAlts
    for(size_t i = 0; i < alts.size(); ++i) {
        const std::string& alt = alts[i];
        offset += helper.putWord((word)alt.size(), offset); // len
        offset += helper.put(StringToBytes(alt), offset); // addr
    }
}
static int parseNextHopServerAlts(ServerAlternates& alts, const byte* data, size_t len) {
    if(NULL == data) {
        return -1;
    }
    alts.clear();
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    word svrCount = 0;
    VREP_ParseFieldOrDie(svrCount, Word, offset, -1);
    for(dword i = 0; i < svrCount; ++i) {
        word svrNameLen;
        VREP_ParseFieldOrDie(svrNameLen, Word, offset, -1);
        bytes svrName;
        VREP_ParseBytesOrDie(svrName, svrNameLen, offset, -1);
        alts.push_back(BytesToString(svrName));
    }
    return (int)offset;
}

static void buildOutputAddress(bytes& data, const bytes& addr) {
    data.clear();
    data.resize(2 + addr.size());
    MessageHelper helper(&data[0], data.size());
    helper.putWord((word)addr.size(), 0);
    helper.put(addr, 2);
}

static int parseOutputAddress(bytes& addr, const byte* data, size_t len) {
    if(NULL == data) {
        return -1;
    }
    addr.clear();
    MessageHelper helper((byte*)data, len);
    size_t offset = 0;
    word addrLen;
    VREP_ParseFieldOrDie(addrLen, Word, offset, -1);
    VREP_ParseBytesOrDie(addr, addrLen, offset, -1);
    return (int)offset;
}

// parse&build
int UpdateAttributes::parse(const byte* buf, size_t len) {
    if(NULL == buf) {
        return -1;
    }
    clear();
    size_t offset = 0;
    while (offset < len) {
        UPDATEAttributeHeader hdr;
        int hdrLen = parseUPDATEAttributeHeader(hdr, buf + offset, len - offset);
        if(hdrLen >= 0) { // successful
            offset += hdrLen; // update the offset
            if(offset + hdr.length <= len) {
                const byte* attrVal = buf + offset;
                size_t attrLen = hdr.length;
                switch(hdr.type) {
#define VREP_ParseUpdateAttr(AttrName, AttrType, AttrField) {\
        int tmpParsedAttrLen = parse##AttrType(AttrField, attrVal, attrLen);\
        if(tmpParsedAttrLen == attrLen) {\
            setting_.set(Attr_Pos_##AttrName);\
        } else {\
            return -1;\
        }\
    }
                case VREP_Type_WithdrawnRoutes:
                    {
                        VREP_ParseUpdateAttr(WithdrawnRoutes, Routes, withdrawnRoutes_);
                        break;
                    }
                case VREP_Type_ReachableRoutes:
                    {
                        VREP_ParseUpdateAttr(ReachableRoutes, Routes, reachableRoutes_);
                        break;
                    }
                case VREP_Type_NextHopServer:
                    {
                        VREP_ParseUpdateAttr(NextHopServer, NextHopServer, nextHopServer_);
                        break;
                    }
                case VREP_Type_QAMNames:
                    {
                        VREP_ParseUpdateAttr(QAMNames, QAMNames, qamNames_);
                        break;
                    }
                case VREP_Type_CASCapability:
                    {
                        // TODO: impl
                        break;
                    }
                case VREP_Type_TotalBandwidth:
                    {
                        VREP_ParseUpdateAttr(TotalBandwidth, Dword, totalBandwidth_);
                        break;
                    }
                case VREP_Type_AvailableBandwidth:
                    {
                        VREP_ParseUpdateAttr(AvailableBandwidth, Dword, availableBandwidth_);
                        break;
                    }
                case VREP_Type_Cost:
                    {
                        VREP_ParseUpdateAttr(Cost, Byte, cost_);
                        break;
                    }
                case VREP_Type_EdgeInput:
                    {
                        // TODO: impl
                        VREP_ParseUpdateAttr(EdgeInput, EdgeInputs, edgeInputs_);
                        break;
                    }
                case VREP_Type_QAMParameters:
                    {
                        VREP_ParseUpdateAttr(QAMParameters, QAMParameters, qamParameters_);
                        break;
                    }
                case VREP_Type_UDPMap:
                    {
                        VREP_ParseUpdateAttr(UDPMap, UDPMap, udpMap_);
                        break;
                    }
                case VREP_Type_Volume:
                    {
                        VREP_ParseUpdateAttr(Volume, Volume, volumes_);
                        break;
                    }
                case VREP_Type_ServiceStatus:
                    {
                        VREP_ParseUpdateAttr(ServiceStatus, Dword, serviceStatus_);
                        break;
                    }
                case VREP_Type_MaxMPEGFlows:
                    {
                        VREP_ParseUpdateAttr(MaxMPEGFlows, Dword, maxMPEGFlows_);
                        break;
                    }
                case VREP_Type_NextHopServerAlternates:
                    {
                        VREP_ParseUpdateAttr(NextHopServerAlternates, NextHopServerAlts, nhsAlternates_);
                        break;
                    }
                case VREP_Type_OutputPort:
                    {
                        VREP_ParseUpdateAttr(OutputPort, Dword, outputPort_);
                        break;
                    }
                case VREP_Type_OutputAddress:
                    {
                        VREP_ParseUpdateAttr(OutputAddress, OutputAddress, outputAddress_);
                        break;
                    }
                case VREP_Type_TransferProtocolCapabilities:
                    {
                        VREP_ParseUpdateAttr(TransferProtocolCapabilities, Byte, transferProtocolCap_);
                        break;
                    }
                default:
                    {
                        GenericAttribute genAttr;
                        genAttr.flags = hdr.flags;
                        genAttr.type = hdr.type;
                        genAttr.data.assign(attrVal, attrVal + attrLen);
                        genericAttrs_.push_back(genAttr);
                        break;
                    }
#undef VREP_ParseUpdateAttr
                } // end switch
                offset += hdr.length; // update the offset
            } else { // bad header field
                return -1;
            }
        } else { // bad header format
            return -1;
        }
    } // end while
    return (int)offset;
}

int UpdateAttributes::build(byte* buf, size_t len) const {
    // TODO: check the mandatory attributes
    // WithdrawRoute
    MessageHelper helper(buf, len);
    size_t offset = 0;

#define VREP_BuildUpdateAttr(AttrName, AttrType, AttrField) \
    if(setting_.test(Attr_Pos_##AttrName)) {\
        GenericAttribute attr;\
        attr.flags = 0;\
        attr.type = VREP_Type_##AttrName;\
        build##AttrType(attr.data, AttrField);\
        int nSize = buildAttribute(buf + offset, len - offset, attr);\
        if(nSize >= 0) {\
            offset += nSize;\
        } else {\
            return -1;\
        }\
    }
    VREP_BuildUpdateAttr(EdgeInput,EdgeInputs,edgeInputs_);
    VREP_BuildUpdateAttr(WithdrawnRoutes, Routes, withdrawnRoutes_);
    VREP_BuildUpdateAttr(ReachableRoutes, Routes, reachableRoutes_);
    VREP_BuildUpdateAttr(NextHopServer, NextHopServer, nextHopServer_);
    VREP_BuildUpdateAttr(QAMNames, QAMNames, qamNames_);
    VREP_BuildUpdateAttr(TotalBandwidth, Dword, totalBandwidth_);
    VREP_BuildUpdateAttr(AvailableBandwidth, Dword, availableBandwidth_);
    VREP_BuildUpdateAttr(Cost, Byte, cost_);
    VREP_BuildUpdateAttr(QAMParameters, QAMParameters, qamParameters_);
    VREP_BuildUpdateAttr(UDPMap, UDPMap, udpMap_);
    VREP_BuildUpdateAttr(Volume, Volume, volumes_);
    VREP_BuildUpdateAttr(ServiceStatus, Dword, serviceStatus_);
    VREP_BuildUpdateAttr(MaxMPEGFlows, Dword, maxMPEGFlows_);
    VREP_BuildUpdateAttr(NextHopServerAlternates, NextHopServerAlts, nhsAlternates_);
    VREP_BuildUpdateAttr(OutputPort, Dword, outputPort_);
    VREP_BuildUpdateAttr(OutputAddress, OutputAddress, outputAddress_);
    VREP_BuildUpdateAttr(TransferProtocolCapabilities, Byte, transferProtocolCap_);
#undef VREP_BuildUpdateAttr
    return (int)offset;
}

void UpdateAttributes::clear() {
    setting_.reset();
    genericAttrs_.clear();
}

void UpdateAttributes::textDump(std::string& data) const {
    std::ostringstream buf;
    buf << "VREP UPDATE:";

// TODO: The RouteName field is not clear
#define PrintRoute(R_) buf << "{Family:" << std::hex << std::showbase << R_.family << ";" \
                           << "Protocol:" << std::hex << std::showbase << R_.protocol << ";" \
                           << "Address:" << BytesToString(R_.address) << ";" \
                           /* << "Name:" << BytesToString(R_.name) << ";" */<< "}"
     if(setting_.test(Attr_Pos_EdgeInput)){
        buf<<" EdgeInputs[";
        for (size_t i = 0;i < edgeInputs_.size();++i){
            buf << "(SubnetMask:"<<edgeInputs_[i].subnetMask
                << ";Host:"<<BytesToString(edgeInputs_[i].host)
                << ";PortID:"<<edgeInputs_[i].portId
                << ";MaxBW:"<<edgeInputs_[i].maxBW
                << ";GroupName:"<<BytesToString(edgeInputs_[i].groupName)
                << ")";
            }
            buf<<"]";
    }
    if(setting_.test(Attr_Pos_WithdrawnRoutes)) {
        buf << " WithdrawnRoutes[";
        for(size_t i = 0; i < withdrawnRoutes_.size(); ++i) {
            if(i != 0) buf << ", ";
            PrintRoute(withdrawnRoutes_[i]);
        }
        buf << "]";
    }
    if(setting_.test(Attr_Pos_ReachableRoutes)) {
        buf << " ReachableRoutes[";
        for(size_t i = 0; i < reachableRoutes_.size(); ++i) {
            if(i != 0) buf << ", ";
            PrintRoute(reachableRoutes_[i]);
        }
        buf << "]";
    }
    if(setting_.test(Attr_Pos_NextHopServer)) {
        buf << " NextHopServer{ComponentAddress:"
            << BytesToString(nextHopServer_.componentAddress)
            << ";StreamingZone:"
            << BytesToString(nextHopServer_.streamingZone)
            << ";}";
    }
    if(setting_.test(Attr_Pos_QAMNames)) {
        buf << " QAMNames[";
        for(size_t i = 0; i < qamNames_.size(); ++i) {
            if(i != 0) buf << ", ";
            buf << qamNames_[i];
        }
        buf << "]";
    }
    if(setting_.test(Attr_Pos_TotalBandwidth)) {
        buf <<" TotalBandwidth(" << std::dec << totalBandwidth_ << ")";
    }

    if(setting_.test(Attr_Pos_AvailableBandwidth)) {
        buf <<" AvailableBandwidth(" << std::dec << availableBandwidth_ << ")";
    }
    if(setting_.test(Attr_Pos_Cost)) {
        buf <<" Cost(" << std::dec << (unsigned int)cost_ << ")";
    }
    if(setting_.test(Attr_Pos_QAMParameters)) {
        buf << " QAMParameters{FrequencyKHz:"
            << std::dec << qamParameters_.frequencyKHz
            << ";ModulationMode:" << std::dec << (unsigned int)qamParameters_.modulationMode
            << ";Interleaver:" << std::dec << (unsigned int)qamParameters_.interleaver
            << ";TSID:" << std::dec << (unsigned int)qamParameters_.tsid
            << ";Annex:" << std::dec << (unsigned int)qamParameters_.annex
            << ";ChannelWidth:" << std::dec << (unsigned int)qamParameters_.channelWidth
            << ";}";
    }

    if(setting_.test(Attr_Pos_UDPMap)) {
        buf << " UDPMap{StaticPorts:[";
        for(size_t i = 0; i < udpMap_.staticPorts.size(); ++i) {
            buf << "(" << udpMap_.staticPorts[i].port
                << ", " << udpMap_.staticPorts[i].pn
                << ")";
        }
        buf << "] DynamicPorts:[";
        for(size_t i = 0; i < udpMap_.dynamicPorts.size(); ++i) {
            buf << "(" << udpMap_.dynamicPorts[i].port
                << ", " << udpMap_.dynamicPorts[i].pn
                << ", " << udpMap_.dynamicPorts[i].count
                << ")";
        }
        buf << "]}";
    }

#define PrintVolume(V_) buf << "{Name:" << BytesToString(V_.name) << ";" \
                            << "PortId:" << std::hex << std::showbase << V_.portId << ";" \
                            << "ReadBandwidth:" << std::dec << V_.readBw << ";" \
                            << "WriteBandwidth:" << std::dec << V_.writeBw << ";}"
    if(setting_.test(Attr_Pos_Volume)) {
        buf << " Volumes[";
        for(size_t i = 0; i < volumes_.size(); ++i) {
            if(i != 0) buf << ", ";
            PrintVolume(volumes_[i]);
        }
        buf << "]";
    }

    if(setting_.test(Attr_Pos_ServiceStatus)) {
        buf << " ServiceStatus(" << std::dec << serviceStatus_ << ")";
    }

    if(setting_.test(Attr_Pos_MaxMPEGFlows)) {
        buf << " MaxMPEGFlows(" << std::dec << maxMPEGFlows_ << ")";
    }

    if(setting_.test(Attr_Pos_NextHopServerAlternates)) {
        buf << " NextHopServerAlternates[";
        for(size_t i = 0; i < nhsAlternates_.size(); ++i) {
            if(i != 0) buf << ", ";
            buf << nhsAlternates_[i];
        }
        buf << "]";
    }

    if(setting_.test(Attr_Pos_OutputPort)) {
        buf << " OutputPort(" << std::dec << outputPort_ << ")";
    }

    if(setting_.test(Attr_Pos_OutputAddress)) {
        buf << " OutputAddress(" << BytesToString(outputAddress_) << ")";
    }
    if(setting_.test(Attr_Pos_TransferProtocolCapabilities)) {
        buf << " TransferProtocolCapabilities("
            << std::hex << std::showbase << (unsigned int)transferProtocolCap_
            << ")";
    }

    buf.str().swap(data);
}

}} // namespace ZQ::Vrep
