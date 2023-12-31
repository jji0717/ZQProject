// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SERVER_I_H
#define ICE_GRID_SERVER_I_H

#include <IceUtil/Mutex.h>
#include <Freeze/EvictorF.h>
#include <IceGrid/Activator.h>
#include <IceGrid/WaitQueue.h>
#include <IceGrid/Internal.h>
#include <set>

#ifndef _WIN32
#   include <sys/types.h> // for uid_t, gid_t
#endif

namespace IceGrid
{

class NodeI;
typedef IceUtil::Handle<NodeI> NodeIPtr;
class ServerAdapterI;
typedef IceUtil::Handle<ServerAdapterI> ServerAdapterIPtr;
class ServerCommand;
typedef IceUtil::Handle<ServerCommand> ServerCommandPtr;
class DestroyCommand;
typedef IceUtil::Handle<DestroyCommand> DestroyCommandPtr;
class StopCommand;
typedef IceUtil::Handle<StopCommand> StopCommandPtr;
class StartCommand;
typedef IceUtil::Handle<StartCommand> StartCommandPtr;
class PatchCommand;
typedef IceUtil::Handle<PatchCommand> PatchCommandPtr;
class LoadCommand;
typedef IceUtil::Handle<LoadCommand> LoadCommandPtr;

class ServerI : public Server, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    enum InternalServerState
    {
        Loading,
        Patching,
        Inactive,
        Activating,
        WaitForActivation,
        ActivationTimeout,
        Active,
        Deactivating,
        DeactivatingWaitForProcess,
        Destroying,
        Destroyed
    };

    enum ServerActivation
    {
        Always,
        Session,
        OnDemand,
        Manual,
        Disabled
    };

    ServerI(const NodeIPtr&, const ServerPrx&, const std::string&, const std::string&, int);
    virtual ~ServerI();

    virtual void start_async(const AMD_Server_startPtr&, const ::Ice::Current& = Ice::Current());
    virtual void stop_async(const AMD_Server_stopPtr&, const ::Ice::Current& = Ice::Current());
    virtual void sendSignal(const std::string&, const ::Ice::Current&);
    virtual void writeMessage(const std::string&, Ice::Int, const ::Ice::Current&);

    virtual ServerState getState(const ::Ice::Current& = Ice::Current()) const;
    virtual Ice::Int getPid(const ::Ice::Current& = Ice::Current()) const;

    virtual void setEnabled(bool, const ::Ice::Current&);
    virtual bool isEnabled(const ::Ice::Current& = Ice::Current()) const;
    virtual void setProcess_async(const AMD_Server_setProcessPtr&, const ::Ice::ProcessPrx&, const ::Ice::Current&);

    virtual Ice::Long getOffsetFromEnd(const std::string&, int, const Ice::Current&) const;
    virtual bool read(const std::string&, Ice::Long, int, Ice::Long&, Ice::StringSeq&, const Ice::Current&) const;

    bool isAdapterActivatable(const std::string&) const;
    const std::string& getId() const;
    InternalDistributionDescriptorPtr getDistribution() const;

    void start(ServerActivation, const AMD_Server_startPtr& = AMD_Server_startPtr());
    ServerCommandPtr load(const AMD_Node_loadServerPtr&, const InternalServerDescriptorPtr&, const std::string&);
    ServerCommandPtr destroy(const AMD_Node_destroyServerPtr&, const std::string&, int, const std::string&);
    bool startPatch(bool);
    bool waitForPatch();
    void finishPatch();

    void adapterActivated(const std::string&);
    void adapterDeactivated(const std::string&);
    void activationFailed(bool);
    void deactivationFailed();

    void activate();
    void kill();
    void deactivate();
    void update();
    void destroy();
    void terminated(const std::string&, int);

private:
    
    void updateImpl(const InternalServerDescriptorPtr&);
    void checkRevision(const std::string&, const std::string&, int) const;
    void updateRevision(const std::string&, int);
    void checkActivation();
    void checkDestroyed() const;
    void disableOnFailure();
    void enableAfterFailure(bool);

    void setState(InternalServerState, const std::string& = std::string());
    ServerCommandPtr nextCommand();
    void setStateNoSync(InternalServerState, const std::string& = std::string());
    
    void createOrUpdateDirectory(const std::string&);
    ServerState toServerState(InternalServerState) const;
    ServerActivation toServerActivation(const std::string&) const;
    ServerDynamicInfo getDynamicInfo() const;
    std::string getFilePath(const std::string&) const;

    const NodeIPtr _node;
    const ServerPrx _this;
    const std::string _id;
    const Ice::Int _waitTime;
    const std::string _serverDir;
    const int _disableOnFailure;

    InternalServerDescriptorPtr _desc;
#ifndef _WIN32
    uid_t _uid;
    gid_t _gid;
#endif
    InternalServerState _state;
    ServerActivation _activation;
    int _activationTimeout;
    int _deactivationTimeout;
    typedef std::map<std::string, ServerAdapterIPtr> ServerAdapterDict;
    ServerAdapterDict _adapters;
    std::set<std::string> _serverLifetimeAdapters;
    bool _processRegistered;
    Ice::ProcessPrx _process;
    std::set<std::string> _activatedAdapters;
    IceUtil::Time _failureTime;
    ServerActivation _previousActivation;
    WaitItemPtr _timer;
    bool _waitForReplication;
    std::string _stdErrFile;
    std::string _stdOutFile;
    Ice::StringSeq _logs;
    PropertyDescriptorSeq _properties;

    DestroyCommandPtr _destroy;
    StopCommandPtr _stop;
    LoadCommandPtr _load;
    PatchCommandPtr _patch;
    StartCommandPtr _start;
    
    int _pid;
};
typedef IceUtil::Handle<ServerI> ServerIPtr;

class ServerCommand : public IceUtil::SimpleShared
{
public:

    ServerCommand(const ServerIPtr&);
    virtual ~ServerCommand();

    virtual void execute() = 0;
    virtual ServerI::InternalServerState nextState() = 0;

protected:

    const ServerIPtr _server;
};
typedef IceUtil::Handle<ServerCommand> ServerCommandPtr;

class TimedServerCommand : public ServerCommand
{
public:

    TimedServerCommand(const ServerIPtr&, const WaitQueuePtr&, int);
    virtual void timeout(bool) = 0;

    void startTimer();
    void stopTimer();

private:

    WaitQueuePtr _waitQueue;
    WaitItemPtr _timer;
    int _timeout;
};
typedef IceUtil::Handle<TimedServerCommand> TimedServerCommandPtr;

class DestroyCommand : public ServerCommand
{
public:

    DestroyCommand(const ServerIPtr&, bool = false);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();

    void addCallback(const AMD_Node_destroyServerPtr&);
    void finished();
    bool loadFailure() const;

private:

    const bool _loadFailure;
    std::vector<AMD_Node_destroyServerPtr> _destroyCB;
};

class StopCommand : public TimedServerCommand
{
public:

    StopCommand(const ServerIPtr&, const WaitQueuePtr&, int);

    static bool isStopped(ServerI::InternalServerState);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();
    void timeout(bool destroyed);

    void addCallback(const AMD_Server_stopPtr&);
    void failed(const std::string& reason);
    void finished();

private:

    std::vector<AMD_Server_stopPtr> _stopCB;
};

class StartCommand : public TimedServerCommand
{
public:

    StartCommand(const ServerIPtr&, const WaitQueuePtr&, int);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();
    void timeout(bool destroyed);

    void addCallback(const AMD_Server_startPtr&);
    void failed(const std::string& reason);
    void finished();

private:

    std::vector<AMD_Server_startPtr> _startCB;
};

class PatchCommand : public ServerCommand, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    PatchCommand(const ServerIPtr&);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();

    bool waitForPatch();
    void destroyed();
    void finished();

private:

    bool _notified;
    bool _destroyed;
};

class LoadCommand : public ServerCommand
{
public:

    LoadCommand(const ServerIPtr&);

    bool canExecute(ServerI::InternalServerState);
    ServerI::InternalServerState nextState();
    void execute();

    void setUpdate(const InternalServerDescriptorPtr&, bool);
    bool clearDir() const;
    InternalServerDescriptorPtr getInternalServerDescriptor() const;
    void addCallback(const AMD_Node_loadServerPtr&);
    void failed(const Ice::Exception&);
    void finished(const ServerPrx&, const AdapterPrxDict&, int, int);

private:

    std::vector<AMD_Node_loadServerPtr> _loadCB;
    bool _clearDir;
    InternalServerDescriptorPtr _desc;
    std::auto_ptr<DeploymentException> _exception;
};

}

#endif
