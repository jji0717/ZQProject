#include "VrepStates.h"
#include "VrepTransport.h"
#include "VrepMessages.h"
namespace ZQ {
namespace Vrep {
#define CtxLog (*ctx.trace)
#define StateFmt(C, M) CLOGFMT(C, "[%p] "M), ctx.fsm
#define StateMFmt(C, M) CLOGFMT(C, "[%p] "M), this
void buildOpenMessage(OpenMessage& msg, Context& ctx) {
    msg.setHoldTime(ctx.conf->defaultHoldTimeSec);
    msg.setIdentifier(ctx.conf->identifier);
    msg.parameters().setComponentName(ctx.conf->componentName);
    msg.parameters().setStreamingZone(ctx.conf->streamingZone);
    msg.parameters().setVendorString(ctx.conf->vendorString);
    msg.parameters().capacities().setSendReceiveCapacity(ctx.conf->sendReceiveMode);
}
// Idle state
StateDescriptor IdleState::descriptor() const
{
    return st_Idle;
}
StateDescriptor IdleState::onEvent(Event event, Context& ctx)
{
    using namespace ZQ::common;
    CtxLog(Log::L_DEBUG, StateFmt(IdleState, "onEvent() event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
    switch(event)
    {
    case e_Start:
        ctx.transportChannel->openConnection(ctx.fsm);
        CtxLog(Log::L_DEBUG, StateFmt(IdleState, "Open the transport channel"));

        ctx.connectRetryTimer->start(ctx.conf->connectRetryTimeSec * 1000);
        CtxLog(Log::L_DEBUG, StateFmt(IdleState, "Start the ConnectRetry timer with timeout(%d)"), ctx.conf->connectRetryTimeSec);

        return st_Connect;
    case e_TransportError:
        ctx.fsm->releaseResource();
        return st_Idle;
    default:
        CtxLog(Log::L_WARNING, StateFmt(IdleState, "Ignore event(%s) in Idle state."), showEvent(event));
        return st_Idle;
    }
}

// Connect state
StateDescriptor ConnectState::descriptor() const
{
    return st_Connect;
}
StateDescriptor ConnectState::onEvent(Event event, Context& ctx)
{
    using namespace ZQ::common;
    CtxLog(Log::L_DEBUG, StateFmt(ConnectState, "onEvent() event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
    switch(event)
    {
    case e_Start: // ignore
        CtxLog(Log::L_WARNING, StateFmt(ConnectState, "Ignore event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
        return descriptor();
    case e_ConnectionOpen:
        // complete initialization
        // clear ConnectRetry timer
        ctx.connectRetryTimer->clear();
        // send OPEN
        {
            OpenMessage msg;
            buildOpenMessage(msg, ctx);
            TransportHelper(ctx).sendOPEN(msg);
        }
        ctx.keepAliveTimer->start(ctx.conf->keepAliveTimeSec * 1000);
        CtxLog(Log::L_INFO, StateFmt(ConnectState, "Start the KeepAliveTimer with timeout(%d)"), ctx.conf->keepAliveTimeSec);

        ctx.holdTimer->start(ctx.conf->defaultHoldTimeSec * 1000);
        CtxLog(Log::L_INFO, StateFmt(ConnectState, "Start the HoldTimer with timeout(%d)"), ctx.conf->defaultHoldTimeSec);
        return st_OpenSent;
    case e_ConnectionOpenFailed:
        // restart ConnectRetry timer
        ctx.transportChannel->reset();
        CtxLog(Log::L_DEBUG, StateFmt(ConnectState, "Reset the transport channel."));
        ctx.connectRetryTimer->restart();
        CtxLog(Log::L_DEBUG, StateFmt(ConnectState, "Restart the ConnectRetry timer."));

        return st_Active;
    case e_ConnectRetryTimerExpired:
        // restart ConnectRetry timer
        ctx.transportChannel->reset();
        ctx.transportChannel->openConnection(ctx.fsm);
        CtxLog(Log::L_DEBUG, StateFmt(ConnectState, "Open the transport channel."));

        ctx.connectRetryTimer->restart();
        CtxLog(Log::L_DEBUG, StateFmt(ConnectState, "Restart the ConnectRetry timer."));
        return st_Connect;
    case e_TransportError:
        ctx.fsm->releaseResource();
        return st_Idle;
    default:
        // send Notification
        {
            CtxLog(Log::L_ERROR, StateFmt(ConnectState, "onEvent() Unexpected event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
            // send Cease notification
            NotificationMessage msg;
            msg.setCode(VREP_Code_Cease);
            msg.setSubcode(VREP_Subcode_Default);
            TransportHelper(ctx).sendNOTIFICATION(msg);
            // close connection
            ctx.fsm->releaseResource();
            return st_Idle;
        }
    }
}

// Active state
StateDescriptor ActiveState::descriptor() const
{
    return st_Active;
}
StateDescriptor ActiveState::onEvent(Event event, Context& ctx)
{
    using namespace ZQ::common;
    CtxLog(Log::L_DEBUG, StateFmt(ActiveState, "onEvent() event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
    switch(event)
    {
    case e_Start: // ignore
        CtxLog(Log::L_WARNING, StateFmt(ActiveState, "Ignore event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
        return descriptor();
    case e_ConnectionOpen:
        // complete initialization
        // clear ConnectRetry timer
        ctx.connectRetryTimer->clear();
        CtxLog(Log::L_DEBUG, StateFmt(ActiveState, "Clear the ConnectRetry timer."));
        // send OPEN
        {
            OpenMessage msg;
            buildOpenMessage(msg, ctx);
            TransportHelper(ctx).sendOPEN(msg);
        }
        ctx.keepAliveTimer->start(ctx.conf->keepAliveTimeSec * 1000);
        CtxLog(Log::L_INFO, StateFmt(ActiveState, "Start the KeepAliveTimer with timeout(%d)"), ctx.conf->keepAliveTimeSec);

        ctx.holdTimer->start(ctx.conf->defaultHoldTimeSec * 1000);
        CtxLog(Log::L_INFO, StateFmt(ActiveState, "Start the HoldTimer with timeout(%d)"), ctx.conf->defaultHoldTimeSec);

        return st_OpenSent;
    case e_ConnectionOpenFailed:
        // close connection
        ctx.transportChannel->reset();
        CtxLog(Log::L_DEBUG, StateFmt(ActiveState, "Reset the transport channel."));
        // restart ConnectRetry timer
        ctx.connectRetryTimer->restart();
        CtxLog(Log::L_DEBUG, StateFmt(ActiveState, "Restart the ConnectRetry timer."));

        return st_Active;
    case e_ConnectRetryTimerExpired:
        ctx.transportChannel->reset();
        ctx.transportChannel->openConnection(ctx.fsm);
        CtxLog(Log::L_DEBUG, StateFmt(ActiveState, "Open the transport channel."));

        // restart ConnectRetry timer
        ctx.connectRetryTimer->restart();
        CtxLog(Log::L_DEBUG, StateFmt(ActiveState, "Restart the ConnectRetry timer."));
        return st_Connect;
    case e_TransportError:
        ctx.fsm->releaseResource();
        return st_Idle;
    default:
        // send Notification
        {
            CtxLog(Log::L_ERROR, StateFmt(ActiveState, "onEvent() Unexpected event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
            // send Cease notification
            NotificationMessage msg;
            msg.setCode(VREP_Code_Cease);
            msg.setSubcode(VREP_Subcode_Default);
            TransportHelper(ctx).sendNOTIFICATION(msg);
            // close connection
            ctx.fsm->releaseResource();
            return st_Idle;
        }
        return st_Idle;
    }
}

// OpenSent state
StateDescriptor OpenSentState::descriptor() const
{
    return st_OpenSent;
}
StateDescriptor OpenSentState::onEvent(Event event, Context& ctx)
{
    using namespace ZQ::common;
    CtxLog(Log::L_DEBUG, StateFmt(OpenSentState, "onEvent() event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
    switch(event)
    {
    case e_Start: // ignore
        CtxLog(Log::L_WARNING, StateFmt(OpenSentState, "Ignore event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
        return descriptor();
    case e_ConnectionClosed:
        // close connection
        ctx.transportChannel->reset();
        // restart ConnectRetry timer
        ctx.connectRetryTimer->restart();
        CtxLog(Log::L_DEBUG, StateFmt(OpenSentState, "Reset the transport channal and restart the ConnectRetry timer."));
        return st_Active;
    case e_TransportError:
        ctx.fsm->releaseResource();
        return st_Idle;
    case e_ReceiveOPEN:
        // process OPEN message
        {
            OpenMessage msg;
            // get the received open
            if(!ctx.fsm->getReceivedOpen(msg)) {
                CtxLog(Log::L_ERROR, StateFmt(OpenSentState, "Failed to get the received OpenMessage."));
                // send Cease notification
                NotificationMessage msg;
                msg.setCode(VREP_Code_Cease);
                msg.setSubcode(VREP_Subcode_Default);
                TransportHelper(ctx).sendNOTIFICATION(msg);
                return st_Idle;
            }

            // check the version
            byte ver;
            msg.getVersion(ver);
            if(ver > VREP_Version_Current) {
                CtxLog(Log::L_ERROR, StateFmt(OpenSentState, "Unsupported version(%d). current version(%d)"), ver, VREP_Version_Current);
                // Unsupported Version Number
                NotificationMessage noti;
                noti.setCode(VREP_Code_OPENMessageError);
                noti.setSubcode(VREP_Subcode_UnsupportedVersion);
                noti.setData((byte) VREP_Version_Current);

                TransportHelper(ctx).sendNOTIFICATION(noti);
                return st_Idle;
            }
            // TODO: check Identifier & HoldTime
            // send KeepAlive message
            TransportHelper(ctx).sendKEEPALIVE();
            return st_OpenConfirm;
        }
    default:
        // send Notification
        {
            CtxLog(Log::L_ERROR, StateFmt(OpenSentState, "onEvent() Unexpected event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
            // send Cease notification
            NotificationMessage msg;
            msg.setCode(VREP_Code_Cease);
            msg.setSubcode(VREP_Subcode_Default);
            TransportHelper(ctx).sendNOTIFICATION(msg);
            // close connection
            ctx.fsm->releaseResource();
            return st_Idle;
        }
    }
}

// OpenConfirm state
StateDescriptor OpenConfirmState::descriptor() const
{
    return st_OpenConfirm;
}
StateDescriptor OpenConfirmState::onEvent(Event event, Context& ctx)
{
    using namespace ZQ::common;
    CtxLog(Log::L_DEBUG, StateFmt(OpenConfirmState, "onEvent() event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
    switch(event)
    {
    case e_Start: // ignore
        CtxLog(Log::L_WARNING, StateFmt(OpenConfirmState, "Ignore event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
        return descriptor();
    case e_ConnectionClosed:
        ctx.fsm->releaseResource();
        return st_Idle;
    case e_TransportError:
        ctx.fsm->releaseResource();
        return st_Idle;
    case e_KeepAliveTimerExpired:
        TransportHelper(ctx).sendKEEPALIVE();
        ctx.keepAliveTimer->restart();
        CtxLog(Log::L_DEBUG, StateFmt(OpenConfirmState, "Send KeepAliveMessage and restart the KeepAlive timer."));
        return st_OpenConfirm;
    case e_ReceiveKEEPALIVE:
        // complete initialization
        // restart hold timer
        ctx.holdTimer->restart();
        CtxLog(Log::L_DEBUG, StateFmt(OpenConfirmState, "Restart the Hold timer."));
        return st_Established;
    case e_ReceiveNOTIFICATION:
        {
            NotificationMessage msg;
            if(ctx.fsm->getReceivedNotification(msg)) {
                std::string txtmsg;
                msg.textDump(txtmsg);
                CtxLog(Log::L_INFO, StateFmt(OpenConfirmState, "Received Notification:%s"), txtmsg.c_str());
            } else {
                CtxLog(Log::L_ERROR, StateFmt(OpenConfirmState, "Can't get the NotificationMessage."));
            }
            // close connection
            ctx.fsm->releaseResource();
            return st_Idle;
        }
    default:
        // send Notification
        {
            CtxLog(Log::L_ERROR, StateFmt(OpenConfirmState, "onEvent() Unexpected event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
            // send Cease notification
            NotificationMessage msg;
            msg.setCode(VREP_Code_Cease);
            msg.setSubcode(VREP_Subcode_Default);
            TransportHelper(ctx).sendNOTIFICATION(msg);
            // close connection
            ctx.fsm->releaseResource();
            return st_Idle;
        }
    }
}

// Established state
StateDescriptor EstablishedState::descriptor() const
{
    return st_Established;
}
StateDescriptor EstablishedState::onEvent(Event event, Context& ctx)
{
    using namespace ZQ::common;
    CtxLog(Log::L_DEBUG, StateFmt(EstablishedState, "onEvent() event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
    switch(event)
    {
    case e_Start: // ignore
        CtxLog(Log::L_WARNING, StateFmt(EstablishedState, "Ignore event(%s) in state(%s)."), showEvent(event), showState(descriptor()));
        return descriptor();
    case e_ConnectionClosed:
        ctx.fsm->releaseResource();
        return st_Idle;
    case e_TransportError:
        ctx.fsm->releaseResource();
        return st_Idle;
    case e_KeepAliveTimerExpired:
        // send KeepAlive message
        TransportHelper(ctx).sendKEEPALIVE();
        // restart KeepAlive timer
        ctx.keepAliveTimer->restart();
        CtxLog(Log::L_DEBUG, StateFmt(EstablishedState, "Send KeepAliveMessage and restart the KeepAlive timer."));
        return st_Established;
    case e_ReceiveKEEPALIVE:
        // complete initialization
        // restart hold timer
        ctx.holdTimer->restart();
        CtxLog(Log::L_DEBUG, StateFmt(EstablishedState, "Restart the Hold timer."));
        return st_Established;
    case e_ReceiveUPDATE:
        {
            UpdateMessage msg;
            if(ctx.fsm->getReceivedUpdate(msg)) {
                // no further process here
                return st_Established;
            } else {
                return st_Idle;
            }
        }
    case e_ReceiveNOTIFICATION:
        {
            NotificationMessage msg;
            if(ctx.fsm->getReceivedNotification(msg)) {
                std::string txtmsg;
                msg.textDump(txtmsg);
                CtxLog(Log::L_INFO, StateFmt(EstablishedState, "Received Notification:%s"), txtmsg.c_str());
            } else {
                CtxLog(Log::L_ERROR, StateFmt(EstablishedState, "Can't get the NotificationMessage."));
            }
            // close connection
            ctx.fsm->releaseResource();
            return st_Idle;
        }
    case e_Stop:
        // send "Cease" notification
        {
            NotificationMessage msg;
            msg.setCode(VREP_Code_Cease);
            msg.setSubcode(VREP_Subcode_Default);
            TransportHelper(ctx).sendNOTIFICATION(msg);
        }
        // close connection
        ctx.fsm->releaseResource();
        return st_Idle;
    default:
        // send "Finite State Machine Error" notification
        {
            NotificationMessage msg;
            msg.setCode(VREP_Code_FiniteStateMachineError);
            msg.setSubcode(VREP_Subcode_Default);
            TransportHelper(ctx).sendNOTIFICATION(msg);
        }

        // release resources
        ctx.fsm->releaseResource();
        return st_Idle;
    }
}

// finite state machine
StateMachine::StateMachine():current_(st_Idle)
{
    quit_ = false;
    // reset the state setting
    for(size_t i = 0; i < VREP_StatesCount; ++i)
        states_[i] = NULL;
    // reset the context
    context_ = NULL;
};

void StateMachine::setup(Context& ctx)
{
    states_[st_Idle] = &stIdle_;
    states_[st_Connect] = &stConnect_;
    states_[st_Active] = &stActive_;
    states_[st_OpenSent] = &stOpenSent_;
    states_[st_OpenConfirm] = &stOpenConfirm_;
    states_[st_Established] = &stEstablished_;

    context_ = &ctx;
}
void StateMachine::issueEvent(Event event)
{
    eventQ_.issue(event);
}

void StateMachine::teardown() {
    context_ = NULL;
    for(size_t i = 0; i < VREP_StatesCount; ++i)
    {
        states_[i] = NULL;
    }   
}

void StateMachine::stop() {
    quit_ = true;
    issueEvent(e_Stop);
    if(isRunning())
        waitHandle(-1);
}

void StateMachine::onConnected(const char* localIp, u_short localPort, const char* remoteIp, u_short remotePort) {
    (*context_->trace)(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "*Connected: %s:%d  ->  %s:%d"), localIp, localPort, remoteIp, remotePort);

    issueEvent(e_ConnectionOpen);
}
void StateMachine::onConnectTimeout(size_t timeout) {
    (*context_->trace)(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "*Connect timeout:%d"), timeout);
    issueEvent(e_ConnectionOpenFailed);
}
void StateMachine::onMessage(const byte* data, size_t length) {
    context_->trace->hexDump(ZQ::common::Log::L_DEBUG, data, length, "Received:");
    while(nExpected_ <= length) {
        memcpy(msgBuf_ + nReceived_, data, nExpected_);
        nReceived_ += nExpected_;
        data += nExpected_;
        length -= nExpected_;

        // parsing the message
        VREPHeader hdr;
        parseVREPHeader(hdr, msgBuf_, nReceived_);
        if(nReceived_ == VREP_MsgSize_Header) {
            // only header received
            // check the length field
            if(hdr.length < VREP_MsgSize_Min
               || VREP_MsgSize_Max < hdr.length
               || (hdr.type == VREP_OPEN && hdr.length < VREP_MsgSize_OPEN_Min)
               || (hdr.type == VREP_UPDATE && hdr.length < VREP_MsgSize_UPDATE_Min)
               || (hdr.type == VREP_KEEPALIVE && hdr.length != VREP_MsgSize_Header)
               || (hdr.type == VREP_NOTIFICATION && hdr.length < VREP_MsgSize_NOTIFICATION_Min)) // bad message length 
            {
                (*(context_->trace))(ZQ::common::Log::L_WARNING, StateMFmt(StateMachine, "onMessage() Bad message length: type(%u) length(%u)"), hdr.type, hdr.length);
                NotificationMessage msg;
                msg.setCode(VREP_Code_MessageHeaderError);
                msg.setSubcode(VREP_Subcode_BadMessageLength);
                msg.setData(hdr.length);
                TransportHelper(*context_).sendNOTIFICATION(msg);
                // shutdown the fsm
                forceIdle();

                nReceived_ = 0;
                nExpected_ = VREP_MsgSize_Header;
                // no need to process the rest data
                return;
            }
            // check the message type
            if(hdr.type != VREP_OPEN && hdr.type != VREP_UPDATE && hdr.type != VREP_NOTIFICATION && hdr.type != VREP_KEEPALIVE) { // bad message type
                (*(context_->trace))(ZQ::common::Log::L_WARNING, StateMFmt(StateMachine, "onMessage() Bad message type: %u"), hdr.type);
                NotificationMessage msg;
                msg.setCode(VREP_Code_MessageHeaderError);
                msg.setSubcode(VREP_SubCode_BadMessageType);
                msg.setData(hdr.type);

                TransportHelper(*context_).sendNOTIFICATION(msg);
                // shutdown the fsm
                forceIdle();

                nReceived_ = 0;
                nExpected_ = VREP_MsgSize_Header;
                // no need receive the rest data
                return;
            }

            nExpected_ = hdr.length - VREP_MsgSize_Header;
            if(nExpected_ == 0) { // no data field
                if(hdr.type != VREP_KEEPALIVE) { // always no data field of KEEPALIVE
                    (*(context_->trace))(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "Received message with no data field. type(%d)"), hdr.type);
                }
                nReceived_ = 0;
                nExpected_ = VREP_MsgSize_Header;
                if(hdr.type == VREP_UPDATE) {
                    saveReceivedUpdate(UpdateMessage());
                    (*(context_->trace))(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "Receive EMPTY UpdateMessage."));
                    issueEvent(e_ReceiveUPDATE);
                } else if (hdr.type == VREP_KEEPALIVE) {
                    issueEvent(e_ReceiveKEEPALIVE);
                } else {
                    (*(context_->trace))(ZQ::common::Log::L_ERROR, StateMFmt(StateMachine, "Bad message format for type(%d): No data field."), hdr.type);
                    // TODO: send notification
                }
            }
            // continue receiving
        } else { // the data field is received
            if(hdr.type == VREP_OPEN) {
                OpenMessage msg;
                int l = msg.parse(msgBuf_ + VREP_MsgSize_Header, nReceived_ - VREP_MsgSize_Header);
                if(l > 0) {
                    std::string txt;
                    msg.textDump(txt);
                    (*(context_->trace))(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "Received: %s"), txt.c_str());
                    saveReceivedOpen(msg);
                    issueEvent(e_ReceiveOPEN);
                } else {
                    context_->trace->hexDump(ZQ::common::Log::L_ERROR, msgBuf_, nReceived_, "Failed to parse OpenMessage:");
                    // TODO: notification
                }
            } else if (hdr.type == VREP_UPDATE) {
                UpdateMessage msg;
                int l = msg.parse(msgBuf_ + VREP_MsgSize_Header, nReceived_ - VREP_MsgSize_Header);
                if(l > 0) {
                    std::string txt;
                    msg.textDump(txt);
                    (*(context_->trace))(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "Received: %s"), txt.c_str());
                    saveReceivedUpdate(msg);
                    issueEvent(e_ReceiveUPDATE);
                } else {
                    context_->trace->hexDump(ZQ::common::Log::L_ERROR, msgBuf_, nReceived_, "Failed to parse UpdateMessage:");
                    // TODO: notification
                }
            } else { // notification
                NotificationMessage msg;
                int l = msg.parse(msgBuf_ + VREP_MsgSize_Header, nReceived_ - VREP_MsgSize_Header);
                if(l > 0) {
                    std::string txt;
                    msg.textDump(txt);
                    (*(context_->trace))(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "Received: %s"), txt.c_str());
                    saveReceivedNotification(msg);
                    issueEvent(e_ReceiveNOTIFICATION);
                } else {
                    context_->trace->hexDump(ZQ::common::Log::L_ERROR, msgBuf_, nReceived_, "Failed to parse NotificationMessage:");
                    // TODO: notification
                }
            }
            nReceived_ = 0;
            nExpected_ = VREP_MsgSize_Header;
        }
    } // while(nExpected_ < length)
    memcpy(msgBuf_ + nReceived_, data, length);
    nReceived_ += length;
    nExpected_ -= length;
}

void StateMachine::onRemoteClosed() {
    (*context_->trace)(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "*Remote closed"));
    issueEvent(e_ConnectionClosed);
}

void StateMachine::onFatalError(const std::string& error) {
    (*context_->trace)(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "*Fatal error: %s"), error.c_str());
    issueEvent(e_TransportError);
}

int StateMachine::run()
{
    if(context_ == NULL)
    {
        // no context info attached
        return -1;
    }
    (*(context_->trace))(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "Start the state machine"));
    // set the initial state
    current_ = st_Idle;

    // reset the receiving buffer
    nReceived_ = 0;
    nExpected_ = VREP_MsgSize_Header;
    while(true)
    {
        // wait for next event
        eventQ_.check(-1);
        Event event = e_Start; // no default event is reserved, use the start instead
        while(eventQ_.fetch(event))
        {
            (*context_->trace)(ZQ::common::Log::L_DEBUG, StateMFmt(StateMachine, "*Event: %s"), showEvent(event));
            counter_.onEvent(event);
            if (event == e_ReceiveOPEN) {
                OpenMessage msg;
                if(getReceivedOpen(msg, true)) {
                    counter_.onOpenMessage(msg);
                } else {
                    (*context_->trace)(ZQ::common::Log::L_WARNING, StateMFmt(StateMachine, "No OpenMessage with Event:%s"), showEvent(event));
                }
            } else if (event == e_ReceiveUPDATE) {
                UpdateMessage msg;
                if(getReceivedUpdate(msg, true)) {
                    counter_.onUpdateMessage(msg);
                } else {
                    (*context_->trace)(ZQ::common::Log::L_WARNING, StateMFmt(StateMachine, "No UpdateMessage with Event:%s"), showEvent(event));
                }
            } else if (event == e_ReceiveNOTIFICATION) {
                NotificationMessage msg;
                if(getReceivedNotification(msg, true)) {
                    counter_.onNotificationMessage(msg);
                } else {
                    (*context_->trace)(ZQ::common::Log::L_WARNING, StateMFmt(StateMachine, "No NotifationMessage with Event:%s"), showEvent(event));
                }
            }
            StateDescriptor next = states_[current_]->onEvent(event, *context_);

            if(next != current_)
            { // state transition
                (*context_->trace)(ZQ::common::Log::L_DEBUG, StateMFmt(StateMachine, "*StateChanged: %s -> %s"), showState(current_), showState(next));

                counter_.onStateChanged(current_, next);
                current_ = next;
            }
        }

        if(current_ == st_Idle && quit_) {
            break;
        }
    } // while(true)
    (*(context_->trace))(ZQ::common::Log::L_INFO, StateMFmt(StateMachine, "Stop the state machine"));
    counter_.clear();
    return 0;
}

void StateMachine::releaseResource() {
    context_->transportChannel->reset();
    context_->connectRetryTimer->clear();
    context_->keepAliveTimer->clear();
    context_->holdTimer->clear();
}
void StateMachine::forceIdle() {
    // do the cleanup
    releaseResource();
    (*context_->trace)(ZQ::common::Log::L_DEBUG, StateMFmt(StateMachine, "forceIdle() *StateChanged: %s -> %s"), showState(current_), showState(st_Idle));
    counter_.onStateChanged(current_, st_Idle);
    current_ = st_Idle;
}

bool StateMachine::getReceivedOpen(OpenMessage& msg, bool peekOnly) {
    ZQ::common::MutexGuard guard(msgLock_);
    if(!openQ_.empty()) {
        msg = openQ_.front();
        if(!peekOnly) openQ_.pop();
        return true;
    } else {
        return false;
    }
}
bool StateMachine::getReceivedUpdate(UpdateMessage& msg, bool peekOnly) {
    ZQ::common::MutexGuard guard(msgLock_);
    if(!updateQ_.empty()) {
        msg = updateQ_.front();
        if(!peekOnly) updateQ_.pop();
        return true;
    } else {
        return false;
    }
}
bool StateMachine::getReceivedNotification(NotificationMessage& msg, bool peekOnly) {
    ZQ::common::MutexGuard guard(msgLock_);
    if(!notiQ_.empty()) {
        msg = notiQ_.front();
        if(!peekOnly) notiQ_.pop();
        return true;
    } else {
        return false;
    }
}

void StateMachine::saveReceivedOpen(const OpenMessage& msg) {
    ZQ::common::MutexGuard guard(msgLock_);
    openQ_.push(msg);
}
void StateMachine::saveReceivedUpdate(const UpdateMessage& msg) {
    ZQ::common::MutexGuard guard(msgLock_);
    updateQ_.push(msg);
}
void StateMachine::saveReceivedNotification(const NotificationMessage& msg) {
    ZQ::common::MutexGuard guard(msgLock_);
    notiQ_.push(msg);
}

void StateMachine::addMonitor(Monitor* m) {
    counter_.addMonitor(m);
}
void StateMachine::removeMonitor(Monitor* m) {
    counter_.removeMonitor(m);
}

void StateMachine::Counter::addMonitor(StateMachine::Monitor* m) {
    ZQ::common::MutexGuard guard(moniLock_);
    if(m && std::find(monitors_.begin(), monitors_.end(), m) == monitors_.end()) {
        monitors_.push_back(m);
    }
}
void StateMachine::Counter::removeMonitor(StateMachine::Monitor* m) {
    ZQ::common::MutexGuard guard(moniLock_);
    monitors_.erase(std::remove(monitors_.begin(), monitors_.end(), m), monitors_.end());
}
void StateMachine::Counter::clear() {
    ZQ::common::MutexGuard guard(moniLock_);
    monitors_.clear();
}
void StateMachine::Counter::onStateChanged(StateDescriptor from, StateDescriptor to) {
    ZQ::common::MutexGuard guard(moniLock_);
    for(size_t i = 0; i < monitors_.size(); ++i) {
        monitors_[i]->onStateChanged(from, to);
    }
}

void StateMachine::Counter::onEvent(Event e) {
    ZQ::common::MutexGuard guard(moniLock_);
    for(size_t i = 0; i < monitors_.size(); ++i) {
        monitors_[i]->onEvent(e);
    }
}

void StateMachine::Counter::onOpenMessage(const OpenMessage& msg) {
    ZQ::common::MutexGuard guard(moniLock_);
    for(size_t i = 0; i < monitors_.size(); ++i) {
        monitors_[i]->onOpenMessage(msg);
    }
}
void StateMachine::Counter::onUpdateMessage(const UpdateMessage& msg) {
    ZQ::common::MutexGuard guard(moniLock_);
    for(size_t i = 0; i < monitors_.size(); ++i) {
        monitors_[i]->onUpdateMessage(msg);
    }
}
void StateMachine::Counter::onNotificationMessage(const NotificationMessage& msg) {
    ZQ::common::MutexGuard guard(moniLock_);
    for(size_t i = 0; i < monitors_.size(); ++i) {
        monitors_[i]->onNotificationMessage(msg);
    }
}
}} // namespace ZQ::Vrep
