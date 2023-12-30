#include <ZQ_common_conf.h>
#include "JMSMessageBase.h"
#include <iostream>
#include <MessageRecord.h>
#include <Message.h>
#include "SafeStore.h"
#include <FileLog.h>

int main(int argc, char* agrv[])
{
    using namespace std;
    ZQ::common::FileLog *pLog = new ZQ::common::FileLog("C:\\TestJms\\TestJms.log", ZQ::common::Log::L_DEBUG);
	ZQ::common::setGlogger(pLog);

#if 0
    {
        typedef EventGateway::JMS::SafeQueue<EventGateway::JMS::MessageRecord> JMSSafeQueue;
        int i = 0;
        Ice::CommunicatorPtr ic = Ice::initialize(i, NULL);
        Freeze::ConnectionPtr conn = Freeze::createConnection(ic, "d:\\temp");
        EventGateway::JMS::MessageRecord* record = NULL;
        try
        {
            record = new EventGateway::JMS::MessageRecord(conn, "asdf");
        }
        catch (Ice::Exception& e)
        {
            cout << e.ice_name() << endl;
            return 1;
        }
        JMSSafeQueue *sq = new JMSSafeQueue(conn, record);

        EventGateway::JMS::Message msg;
        std::string junk = ";sdi;[]. oneonr02jn3nmadf fasjdfaoidmonefonef";
        for(int i = 0; i < 10; ++i)
        {
            junk += "a";
            msg[junk] = junk;
        }
        sq->push(msg);

        for(int i = 0; i < 1000; ++i)
        {
            sq->push(msg);
        }
        //if(rand() % 2)
        sq->pop();


        size_t sz = sq->size();
        delete sq;
        delete record;
    }
    ic->destroy();
#endif
    using namespace EventGateway::JMS;
    ServerConfig server;
    server.URL = "10.50.12.23:13011";
    server.namingContextFactory = "org.jnp.interfaces.NamingContextFactory";
    server.connectionFactory = "ConnectionFactory";

    EventGateway::JMS::MessageBase base(glog, server, "C:\\TestJMS");

    ChannelConfig chann;
    chann.name = "no1";
    chann.destination = "topic/TianShan\\Provision\\StateChange";
    chann.msgPropertiesInt["MESSAGECODE"] = 3004;
    chann.msgPropertiesString["MESSAGECLASS"] = "NOTIFICATION";
    chann.TTL = 1000;
    chann.optionEnabled = 1;
    MessageChannel* channel = base.createChannel(chann);

    if(channel)
    {
        Message msg;
        //msg[""] = "<DMAMessage bundleID=\"383838\" status=\"43210\" destinationID=\"888888\" />";
        msg["1"] = "from chai xiao hui";
        msg["0"] = "Best Regards";
        channel->push(msg);
        Sleep(1000);
    }
    Sleep(100000);
    glog.flush();
    delete pLog;
    return 0;
}