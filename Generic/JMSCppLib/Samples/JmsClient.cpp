#include "..\JMSCpp\jmshead.h"
#include <iostream.h>
#include <windows.h>

using namespace ZQ::JMSCpp;

class Mylistener:public Listener
{
public:
	virtual void onMessage(Message *);
};

int symbol=0; 

void Mylistener::onMessage(Message *ms)
{
	char name[100];
	memset(name,0,100);
	TextMessage *tm = (TextMessage *)ms;
	tm->getText(name,100);
	//MessageBox(NULL,name,"lllll",MB_OK);
	cout<<name<<endl;
	symbol=1;
}

void main()
{
   Context jndiContext("192.168.80.231:1099","org.jnp.interfaces.NamingContextFactory");  
   ConnectionFactory connectionFactory;  
   Connection connection; 
   Session writerSession;  
   Session readerSession;
   Destination destination;
   Producer producer;
   Consumer consumer;
   TextMessage textmessage;

   Mylistener li;

   jndiContext.createConnectionFactory("ConnectionFactory",connectionFactory); 
   connectionFactory.createConnection(connection); 
   connection.createSession(writerSession);
   connection.createSession(readerSession);
   jndiContext.createDestination("topic/testTopic",destination);
   writerSession.createProducer(&destination,producer);
   readerSession.createConsumer(&destination,consumer);
    
   consumer.setMessageListener(&li);
   
   connection.start();
 //  writerSession.bytesMessageCreate(map);
   writerSession.textMessageCreate("Send a message and then receive it",textmessage);
//   map.setByte("ni",3);
 //  map.writeBoolean(1);
   producer.send(&textmessage);
   while(symbol==0)
   {
	   Sleep(1000);
   }
   cout<<"Program terminated\n";
}