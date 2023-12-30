#include "..\JMSCpp\jmshead.h"
#include <iostream.h>
#include <windows.h>
#include <fstream>
#include <stdio.h>
#include <conio.h>
using namespace	std;
using namespace ZQ::JMSCpp;


bool			bConnectionOK=false;
void ConnMonitor(int ErrType, void * lpData)
{
	cout<<"connection lost with errType="<<ErrType<<endl;
	bConnectionOK=false;
}
void main(int narg,char** varg)
{
#define		BUFLEN	8192
	if(narg!=5)
	{
		cout<<"wrong parameter"<<endl;
		cout<<"using:receivedata.exe  IP:Port logFilePath topicName durable(1 for true 0 for false)"<<endl;
		return ;
	}
   char name[BUFLEN];
   memset(name,0,BUFLEN);
   Context jndiContext(varg[1],"org.jnp.interfaces.NamingContextFactory");  
   ConnectionFactory connectionFactory;  
   Connection connection;
   Session writerSession;  
   Session readerSession;
   Destination destination;
   Destination temp;
   Producer producer;
   Consumer consumer;

	bConnectionOK=true;
	bool	bExit=false;
   while(1)
   {
	   
	   consumer.close();
	   producer.close();
	   destination.destroy();
	   writerSession.close();
	   readerSession.close();
	   connection.close();
	   connectionFactory.Destroy();

	   if(! (jndiContext.createConnectionFactory("ConnectionFactory",connectionFactory) && connectionFactory._connectionFactory!=NULL ))
		   continue;
	   if(! (connectionFactory.createConnection(connection) && connection._connection!=NULL) )
		   continue;
	   connection.SetConnectionCallback(ConnMonitor,NULL);
	   if(!connection.createSession(writerSession))
		   continue;
	   if(!connection.createSession(readerSession))
		   continue;
	   //if(!jndiContext.createDestination("topic/testDurableTopic",destination))
	   if(!jndiContext.createDestination(varg[3],destination))
		   continue;
	   
	   if(strcmp(varg[4],"0")==0)
	   {
		   readerSession.createConsumer(&destination,consumer);
	   }
	   else
	   {
		   char	chSelector[]="ABC";
		   readerSession.createDurableSubscriber(&destination,"ABC",NULL,consumer);
	   }

	   
	   connection.start();
	   
	   cout<<"start"<<endl;
	   bConnectionOK=true;
	   int i=0;
	   char szBuf[2048];
	   while (bConnectionOK)
	   {
		   MapMessage mapMsg;
		   if(_kbhit()&&getch()=='q')
		   {
			   bExit=true;
			   break;
		   }
		   if(consumer.receive(1000,mapMsg))
		   {
			   if(mapMsg._message)
			   {
				   FILE* pFile=fopen(varg[2],"a+");
				   if(!pFile)
				   {
					   cout<<"can't create file"<<endl;
					   int i;
					   cin>>i;
				   }

				   cout<<"Get a new message " <<++i<<endl;
				   {
					   sprintf(szBuf,"get a new Message %d \n",i);
					   fwrite(szBuf,1,strlen(szBuf),pFile);
					   char strKey[512];
					   const char* key = mapMsg.getFirstKey(strKey,sizeof(strKey));
					   char szValue[1024];
					   while(key)
					   {
						   ZeroMemory(szValue,sizeof(szValue));
						   mapMsg.getString((char*)key,szValue,sizeof(szValue)-1);
						   sprintf(szBuf,"Key[%s]\t\t\tValue[%s]\n",key,szValue);
						   fwrite(szBuf,1,strlen(szBuf),pFile);
						   key=mapMsg.getNextKey(strKey,sizeof(strKey));
					   };
				   }
				   fflush(pFile);
			       fclose(pFile);
			   }
		   }
	   }
	   if(strcmp(varg[4],"0")!=0)
		   readerSession.unSubscribe("ABC");
	   if(bExit)
		   break;
	   Sleep(1000);	   
   }
   q
   cout<<"end"<<"\n";

}