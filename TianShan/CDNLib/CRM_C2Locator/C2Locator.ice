// build steps:
//	$(ICE_ROOT)\bin\slice2cpp.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE -I$(ZQProjsPath)/TianShan/common --output-dir . $(InputDir)\$(InputName).ice  
//	$(ICE_ROOT)\bin\slice2freeze.exe -I.. -I$(ICE_ROOT)/slice -I$(ZQProjsPath)/TianShan/ICE -I$(ZQProjsPath)/TianShan/common --index "TianShanIce::SCS::SessionIdx,TianShanIce::SCS::TransferSession,sessKey,case-sensitive" SessionIdx --output-dir . $(InputDir)\$(InputName).ice


#ifndef __ZQ_C2LOCATOR_ICE__
#define __ZQ_C2LOCATOR_ICE__

#include <Ice/Identity.ice>
#include "TsStreamer.ICE"
#include "TsEvents.ICE"
#include "TianShanUtils.ICE"

//TODO: compile command lines

module TianShanIce
{

module SCS
{

/// ClientTransfer --> in-memory
struct ClientTransfer
{
    string address;
    long ingressCapacity;
    long activeTransferCount;
    long consumedBandwidth;
};
sequence<ClientTransfer> ClientTransfers;

struct TransferSessionProp
{
    string        transferId;         ///< transfer id for this session(index)
    string        clientTransfer;     ///< which client interface this session from
    string        transferPort;       ///< which transfer port to stream this session, the transferport must be identified via "[<CDNCS-NetId>/]<CDNSS-NetId>/<DeviceId>"
    long          transferDelay;      ///< transfer delay required by session
    long          transferRate;	      ///< bandwidth of this session transfer
    long          allocatedBW;	      ///< bandwidth of this session transfer

    long          transferTimeout;    ///< timeout that the stream service reserve the transferId for the client
    ValueMap others; ///< the other properties
};
sequence<TransferSessionProp> TransferSessions;

/// TransferSession
["freeze:write"]
class TransferSession extends TianShanIce::Transport::PathTicket implements TianShanUtils::TimeoutObj
{
    string                          sessKey;          ///< the index of this freeze object, same as transferId
    //long                            lastUpdateTime;   ///< the update time point of this session

    TransferSessionProp             props;            ///< the session properties
    TransferSessionProp             getProps();
    void setProps(TianShanIce::ValueMap value);
	
    void setProps2 (TransferSessionProp props); // overwrite the properties
    void setProps3( TianShanIce::Properties props );

    TianShanIce::Streamer::Stream*  stream;           ///< the proxy string to CDNSS session
    TianShanIce::Streamer::Stream*  getStream();
    void setStream(TianShanIce::Streamer::Stream* s);
    
    /// when restart service, session set to restore from db
    //void OnRestore();
    
    /// after commit success, CDNSS should call this function to set transferId
	///@param[in] the transferId need to set


    /// set the session state
    void setState(TianShanIce::State st);
    /// set the SRM resources
    void setResources(TianShanIce::SRM::ResourceMap rs);
    /// set expiration
    void setExpiration(long timeoutMsec);
};

/// transfer port management data
struct TransferPort
{
    string name; // in format of "[<CDNCS-NetId>/]<CDNSS-NetId>/<DeviceId>"

    // status fields
    bool isUp; // port state, up or down;
    long capacity; // in bits per second

    StrValues addressListIPv4;
    StrValues addressListIPv6;
    string streamService;

    // usage fields
    long activeBandwidth; // in bits per second
    long activeTransferCount;

    // availability fields
    // include/exclude the port from the future transfer service
    bool enabled;

    long penalty; // penalty of this port
};
sequence<TransferPort> TransferPorts;

interface C2Locator extends ::TianShanIce::BaseService, ::TianShanIce::Events::GenericEventSink, ::TianShanIce::ReplicaSubscriber
{
	/// get a TransferSession proxy by transferId
	///@param[in] the sessionKey as an index, if not find the sessionKey object, create and add to evictor map
	///@return a point to the TransferSession, NULL if the TransferSession doesn't exist
    //TransferSession* createSession();
    
	/// get a TransferSession proxy by transferId
	///@param[in] the transferId as an index
	///@return a point to the TransferSession, NULL if the TransferSession doesn't exist
    TransferSession* openSessionByTransferId(string transferId);

    /// list the clients info
    /// @return the clients with the current state
    ClientTransfers listClients();

    /// list the transfer ports info
    /// @return the transfer ports with the current state
    TransferPorts listTransferPorts();

    /// list the sessions info by client transfer
    /// @param[in] client the client transfer
    /// @return the sessions info of the client
    TransferSessions listSessionsByClient(string client);

    /// list the sessions info by transfer port
    /// @param[in] port the transfer port's name
    /// @return the sessions info of the port
    TransferSessions listSessionsByPort(string port);

    /// update the availablility of transfer ports
    /// @param[in] ports the transfer ports' name
    /// @param[in] enabled true for enable the ports, false for disable
    void updatePortsAvailability(StrValues ports, bool enabled);
};

}; // SCS

}; // TianShanIce
#endif // __ZQ_C2LOCATOR_ICE__
