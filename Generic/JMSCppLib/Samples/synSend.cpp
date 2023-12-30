#include "..\jmscpp\jmshead.h"
#include <iostream.h>
#include <windows.h>

using namespace ZQ::JMSCpp;

void main()
{
   Context jndiContext("192.168.80.231:1099","org.jnp.interfaces.NamingContextFactory");  
   ConnectionFactory connectionFactory;  
   Connection connection;
   //(connectionFactory.CreateConnection());  
   Session writerSession;  
//   Session readerSession;
   Destination destination;
   Producer producer;
   Consumer consumer;
   TextMessage textmessage;
   MapMessage map;

   jndiContext.createConnectionFactory("ConnectionFactory",connectionFactory); 
   connectionFactory.createConnection(connection); 
   connection.createSession(writerSession);
 //  connection.createSession(readerSession);
   jndiContext.createDestination("topic/testTopic",destination);
   writerSession.createProducer(&destination,producer);
//   readerSession.createConsumer(&destination,consumer);
   
   connection.start();
   writerSession.textMessageCreate("This is a request message",textmessage);
//   writerSession.mapMessageCreate(map);
 //  map.setString("name","Seachange");
   Requestor rr(&writerSession,&destination);
   TextMessage response;
   rr.request(&textmessage,response,1000);
   
   char text[100];
   memset(text,0,100);
   response.getText(text,100);

 //  producer.send(&textmessage);
//   consumer.setMessageListener(&li);
 //  producer.send(&map);
//   while(symbol==0)
//   {
//	   Sleep(1000);
 //  }
   cout<<text<<"\n";
}