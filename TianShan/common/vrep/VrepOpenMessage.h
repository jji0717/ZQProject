#ifndef __ZQ_Vrep_OpenMessage_H__
#define __ZQ_Vrep_OpenMessage_H__
#include "VrepMessage.h"
namespace ZQ {
namespace Vrep {

#define VREP_Version_Current 2
#define VREP_Parameter_Capacity 1
#define VREP_Parameter_StreamingZone 2
#define VREP_Parameter_ComponentName 3
#define VREP_Parameter_VendorString 4

#define VREP_Capacity_RouteTypesSupported 1
#define VREP_Capacity_SendReceiveCapacity 2

#define VREP_AddressFamily_NGOD 32769
#define VREP_AppProtocol_Preprovisioned 32766
#define VREP_AppProtocol_R6_SessionParameterOnly 32768
#define VREP_AppProtocol_R6_WithProvisioning 32769
#define VREP_AppProtocol_R4 32770
#define VREP_AppProtocol_A3 32771
#define VREP_AppProtocol_R2 32772
#define VREP_AppProtocol_S6_RTSP 32773
#define VREP_AppProtocol_S6_TBD 32774
#define VREP_AppProtocol_S4 32775
#define VREP_AppProtocol_S3 32776

#define VREP_SendReceiveMode 1
#define VREP_SendOnlyMode 2
#define VREP_ReceiveOnlyMode 3
#define VREP_Invalid_SendReceiveCapacity (dword)(~0)

#define VREP_HoldTime_Default 3
#define VREP_Invalid_Identifier (dword)(~0)

class OpenCapacities
{
public:
    OpenCapacities();

    void setSendReceiveCapacity(dword mode);
    bool getSendReceiveCapacity(dword& mode) const;

    typedef std::vector<RouteType> RouteTypes;
    void setSupportedRouteTypes(const RouteTypes& rts);
    bool getSupportedRouteTypes(RouteTypes& rts) const;
    struct GenericCapacity {
        word code;
        bytes data;
    };
    typedef std::vector<GenericCapacity> GenericCapacities;
    void setGenericCapacities(const GenericCapacities& gcs);
    bool getGenericCapacities(GenericCapacities& gcs) const;

    bool validate() const;
    int parse(const byte* buf, size_t len);
    int build(byte* buf, size_t len) const;
    void clear();
private:
    RouteTypes supportedRouteTypes_;
    dword sendReceiveCapacity_;
    GenericCapacities genericCapacities_;
};

class OpenParameters
{
public:
    OpenParameters();

    void setStreamingZone(const std::string& sz);
    bool getStreamingZone(std::string& sz) const;

    void setComponentName(const std::string& cn);
    bool getComponentName(std::string& cn) const;

    void setVendorString(const std::string& vs);
    bool getVendorString(std::string& vs) const;

    OpenCapacities& capacities();
    const OpenCapacities& capacities() const;

    struct GenericParameter {
        word type;
        bytes data;
    };
    typedef std::vector<GenericParameter> GenericParameters;
    void setGenericParameters(const GenericParameters& gps);
    bool getGenericParameters(GenericParameters& gps) const;

    bool validate() const;
    int parse(const byte* buf, size_t len);
    int build(byte* buf, size_t len) const;
    void clear();
private:
    // mandatory parameters
    std::string streamingZone_;
    std::string componentName_;
    // optional parameters
    std::string vendorString_;
    OpenCapacities capacities_;
    // generic parameters
    GenericParameters genericParameters_;
};

class OpenMessage
{
public:
    OpenMessage();

    void setVersion(byte ver);
    void getVersion(byte& ver) const;

    void setHoldTime(word ht);
    void getHoldTime(word& ht) const;

    void setIdentifier(dword id);
    void getIdentifier(dword& id) const;

    OpenParameters& parameters();
    const OpenParameters& parameters() const;

    bool validate() const;
    // parse&build
    int parse(const byte* buf, size_t len);
    int build(byte* buf, size_t len) const;
    void clear();

    void textDump(std::string& data) const;
private:
    byte version_;
    word holdTime_;
    dword identifier_;
    OpenParameters parameters_;
};
}} // namespace ZQ::Vrep

#endif
