// DsmccCRGTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "DsmccMsg.h"
using namespace ZQ::DSMCC;

//#include "CRMDmsccCfg.h"
/*void test()
{   
		uint8 buf[5];
	buf[0]= 12;
	buf[1]= 34;
	buf[2]= 56;
	buf[3]= 78;
	buf[4]= 90;
//	buf[5]= 00;
	reversalBytes(buf, 5);


	std::vector < uint8 > bytes;
	bytes.push_back(12);
	bytes.push_back(34);
	bytes.push_back(56);
	bytes.push_back(78);
	uint32 count = 0;
	memcpy(&count, &bytes[0], 4);
	std::string assetId= std::string(&bytes[0], &bytes[0] + 4);

	printf("%u\n", count);
	printf("%s\n", assetId.c_str());

	uint8 strHostBuf[64], strNetBuf[64];
	uint64 testlong = 2864;
	memcpy(strHostBuf, &testlong, sizeof(strHostBuf));

	uint64 netLongLong = htonll(testlong);
	memcpy(strNetBuf, &netLongLong, sizeof(strNetBuf));
    
	uint64 hostlonglong = ntohll(netLongLong);

	printf("testlong (%lld), toNetLong(%lld), tohostLong(%lld)\n", testlong, netLongLong, hostlonglong);

}
uint64 ntohlltest(uint64 netlonglong)
{
	uint32 lowData = (uint32)(netlonglong & 0xffffffff);
	uint32 highData = (uint32)(netlonglong >> 32);
	lowData = ntohl(lowData);
	highData = ntohl(highData);

	uint64 retLongLong = lowData;
	retLongLong = retLongLong << 32;
	retLongLong = retLongLong | highData;
	return retLongLong;
}
uint64 htonlltest(uint64 hostlonglong)
{
	uint32 lowData = (uint32)(hostlonglong & 0xffffffff);
	uint32 highData = (uint32)(hostlonglong >> 32);
	lowData = htonl(lowData);
	highData = htonl(highData);

	uint64 retLongLong = lowData;
	retLongLong = retLongLong << 32;
	retLongLong = retLongLong | highData;
	return retLongLong;
}
//112233445566
Ice::Long desMac = htonlltest(0x0000112233445566);
uint8 desMacTemp[8];
memset(desMacTemp, 0, 8);
memcpy(desMacTemp, &desMac, 8);
uint8 destinationMacAddress[6];
memcpy(destinationMacAddress, desMacTemp + 2, 6);
*/
/*	if(SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,true) == false)  
		printf("unable to register console handler\n");
	while(!bQuit)
	{
		Sleep(5000);
	}
	printf("exit \n");
	return -1;
bool bQuit = false;
bool WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_C_EVENT:
		break;
	case CTRL_BREAK_EVENT:

		break;
	case CTRL_CLOSE_EVENT:	
		printf("exit program\n");
		bQuit = true;
		break;
	case CTRL_LOGOFF_EVENT:
		break;
	case CTRL_SHUTDOWN_EVENT:
		break;
	}
	return true;
}*/

int main(int argc, char* argv[])
{   
	uint8 strBuf[1024];
	memset(strBuf, '0', 1024);
	FILE* fin = stdin;
	if (argc >1)
	{
		if (NULL == (fin = fopen(argv[1], "rb")))
			return -1;
	}

	size_t len = fread((char*)strBuf, 1, sizeof(strBuf), fin);

	if (NULL != fin && fin != stdin)
		fclose(fin);
    
	 size_t processedLength = 0;

	 ProtocolType protocolType = Protocol_MOTO;

	 ZQ::DSMCC::DsmccMsg::Ptr pMsg = ZQ::DSMCC::DsmccMsg::parseMessage(strBuf + 0x2A, len - 0x2A, processedLength, protocolType);

	 if(pMsg)
	 {
		 printf("1. parse Message: processedLength(%d)\n", processedLength);

		 ZQ::DSMCC::StringMap metadatas;
		 uint16 size = pMsg->toMetaData(metadatas);
		 printf("   MetaDataSize(%d)\n", size);
		  uint8 toBuf[1024] ="";
         int length =  pMsg->toMessage(toBuf, 1024);
         
		 pMsg = ZQ::DSMCC::DsmccMsg::parseMessage(toBuf, length, processedLength, protocolType);

         uint8 totoBuf[1024] ="";
		 length =  pMsg->toMessage(totoBuf, 1024);

		 for(int i = 0 ; i < length; i++)
		 {
			 if(totoBuf[i] != toBuf[i])
				 printf("diff [%d]\n", i) ;
		 }
		 ZQ::DSMCC::DsmccMsg::HardHeader tmpHeader;
		 tmpHeader.messageId = pMsg->getMessageId();
		 switch(tmpHeader.messageId)
		 {
		 case MsgID_SetupRequest:
			 pMsg = new ClientSessionSetupRequest(tmpHeader, protocolType);
			 break;
		 case MsgID_SetupConfirm:
			 pMsg = new ClientSessionSetupConfirm(tmpHeader, protocolType);
			 break;
		 case MsgID_ReleaseRequest:                //ClientSessionReleaseRequest
			 pMsg = new ClientSessionReleaseRequest(tmpHeader, protocolType);
			 break;
		 case MsgID_ReleaseConfirm:    
			 pMsg = new ClientSessionReleaseConfirm(tmpHeader, protocolType);
			 break;
		 case MsgID_ReleaseIndication:    
			 pMsg = new ClientSessionReleaseIndication(tmpHeader, protocolType);
			 break;
		 case MsgID_ReleaseResponse:   
			 pMsg = new ClientSessionReleaseResponse(tmpHeader, protocolType);
			 break;
		 case MsgID_ProceedingIndication: 
			 pMsg = new ClientSessionProceedingIndication(tmpHeader, protocolType);
			 break;
		 case MsgID_InProgressRequest: 
			 pMsg = new ClientSessionInProgressRequest(tmpHeader, protocolType);
			 break;
		 case 0x0000:
		 default:
			 break;
		 }

		 DsmccResources dsmssResources;
		 pMsg->readMetaData(metadatas);
		 pMsg->readResource(dsmssResources);

		 int length1 =  pMsg->toMessage(toBuf, 4096);
		 printf("2. to Message: orignal(%d),toMessage(%d)\n",processedLength,  length1);

		 pMsg = ZQ::DSMCC::DsmccMsg::parseMessage(toBuf, length1, processedLength, Protocol_Tangberg);
		 printf("3. parse Message: processedLenght(%d)\n", processedLength);
		 metadatas.clear();
		 if(pMsg)
		 {
			 size = pMsg->toMetaData(metadatas);
			 printf("   MetaDataSize(%d)\n", size);
		 }
	 }

/*	 ZQ::DSMCC::ClientSessionSetupRequest *msg = dynamic_cast<ZQ::DSMCC::ClientSessionSetupRequest*>(pMsg.get());
	 uint8 buf[4096];
	 memset(buf, 0, 4096);
	 int length = pMsg->toMessage(buf, 4096);
	 processedLength  = 0;
	 ZQ::DSMCC::DsmccMsg::parseMessage(buf, length,processedLength);
	 uint16 msgLenth =  pMsg->getMessageLength();*/
	return 0;
}
