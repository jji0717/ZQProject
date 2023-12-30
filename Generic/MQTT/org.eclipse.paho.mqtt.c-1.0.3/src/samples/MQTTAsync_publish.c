/*******************************************************************************
 * Copyright (c) 2012, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *******************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#ifdef  WIN32
#include <Windows.h>
#endif 

#include "string.h"
#include "MQTTAsync.h"

#if !defined(WIN32)
#include <unistd.h>
#endif


//#define ADDRESS     "tcp://m2m.eclipse.org:1883"
#define ADDRESS     "tcp://10.15.10.78:1883"
#define CLIENTID    "ExampleClientPub"
#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

char message[4096];
char topicName[512];

volatile MQTTAsync_token deliveredtoken;

int finished = 0;

void connlost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
 		finished = 1;
	}
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
	printf("Successful disconnection\n");
	finished = 1;
}


void onSend(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	printf("Message with token value %d delivery confirmed\n", response->token);

	opts.onSuccess = onDisconnect;
	opts.context = client;

	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		exit(-1);	
	}

}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response ? response->code : 0);
	finished = 1;
}


void onConnect(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;
	int i ;

	printf("Successful connection\n");
	
	opts.onSuccess = onSend;
	opts.context = client;

	pubmsg.payload = message;
	pubmsg.payloadlen = strlen(message);
	pubmsg.qos = QOS;
	pubmsg.retained = 1;
	pubmsg.dup = 1;
	deliveredtoken = 0;
	for(i =0 ; i < 5; i++)
	{
		if ((rc = MQTTAsync_sendMessage(client, topicName, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
		{
			printf("Failed to start sendMessage, return code %d\n", rc);
			exit(-1);	
		}
	}
}

void help()
{
	printf("	MQTTAsync_publish: <rabbitMqServer><TopicName><UserName><PassWord><Message>\n");
	printf("	default: rabbitMqServer: tcp://10.15.10.78:1883\n");
	printf("	default: TopicName:\"MQTT Examples\" \n");
	printf("	UserName: guest \n");
	printf("	PassWord: centos \n");
	printf("	Message:\"Hello World!\" \n");
	printf("	  example: tcp://10.15.10.78:1883 \"MQTT Examples\" guest centos \"Hello World!\" \n");
}
int main(int argc, char* argv[])
{
//	help();
	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	MQTTAsync_token token;
	int rc;
	char* rabbitMqServer = ADDRESS;
	char* userName ="guest";
	char* passWord ="centos";
	char* clientId = CLIENTID;

	strcpy(message, PAYLOAD);
	strcpy(topicName, TOPIC);

	if(argc > 1)
		rabbitMqServer = argv[1];
	if(argc > 2)
		strcpy(topicName, argv[2]);
	if(argc > 3)
		userName = argv[3];
	if(argc > 4)
		passWord = argv[4];
	if(argc > 5)
		strcpy(message, argv[5]);

	if(argc > 6)
		clientId = argv[6];

	MQTTAsync_create(&client, rabbitMqServer, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(client, NULL, connlost, NULL, NULL);

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 0;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	conn_opts.username = userName;
	conn_opts.password = passWord;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		exit(-1);	
	}

	printf("Waiting for publication of %s\n on topic %s for client with ClientID: %s\n", message, topicName, clientId);
	while (!finished)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif

	MQTTAsync_destroy(&client);
 	return rc;
}
  
