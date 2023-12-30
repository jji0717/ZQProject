#include "..\JMSCpp\jmshead.h"
#include <iostream.h>
#include <windows.h>

using namespace ZQ::JMSCpp;
void main()
{
   char name[100];
   memset(name,0,100);

   Context jndiContext("192.168.80.231:1099","org.jnp.interfaces.NamingContextFactory");  
   ConnectionFactory connectionFactory;  
   Connection connection;
   Session writerSession;  
   Session readerSession;
   Destination destination;
   Destination temp;
   Producer producer;
   Consumer consumer;
   TextMessage textmessage;
 //  MapMessage map;

   jndiContext.createConnectionFactory("ConnectionFactory",connectionFactory); 
   connectionFactory.createConnection(connection); 
   connection.createSession(writerSession);
   connection.createSession(readerSession);
   jndiContext.createDestination("topic/testTopic",destination);
 //  writerSession.createProducer(&destination,producer);
   readerSession.createConsumer(&destination,consumer);
   
   connection.start();
 //  writerSession.textMessageCreate("First Message",textmessage);
 //  writerSession.mapMessageCreate(map);
 //  map.setString("name","Seachange");
 //  producer.send(&textmessage);
//   consumer.setMessageListener(&li);
   consumer.receive(0,textmessage);
   textmessage.getText(name,100);
   textmessage.getReplyTo(temp);
   writerSession.createProducer(&temp,producer);
   writerSession.textMessageCreate("Reply to the request",textmessage);
   producer.send(&textmessage);
//   textmessage.getText(name,20);
//  map.getString("name",name,20);
//	tm->getText(name,20);
//	Sleep(10000);
   cout<<name<<"\n";

}