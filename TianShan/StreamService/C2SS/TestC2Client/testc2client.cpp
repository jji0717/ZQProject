#include <stdio.h>
#include <vector>
#include <C2Client.h>
#include <FileLog.h>
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include <TimeUtil.h>

ZQ::common::FileLog g_log("c2client.log", ZQ::common::Log::L_DEBUG);

class TestC2ClientCB;
typedef ZQ::common::Pointer<TestC2ClientCB> TestC2ClientCBPtr;
int g_locateIndex = 0;
int g_getIndex = 0;

class TestC2ClientCB : public ZQ::StreamService::C2ClientBind
{
    virtual void OnC2LocateResponse(const std::string& contentName, const ZQ::StreamService::AttrMap& locRespParamters)
	{
		g_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TestC2ClientCB, "OnC2LocateResponse() entry, index = %d"), g_locateIndex++);
		std::string transport;
		std::string transferID;

        for (ZQ::StreamService::AttrMap::const_iterator it= locRespParamters.begin(); it != locRespParamters.end(); it++)
		{
			printf("%s \t: %s\n", it->first.c_str(), it->second.c_str());

			if (C2CLIENT_TransferPort == it->first)
			{
				transport = it->second;
			}

			if (C2CLIENT_TransferID == it->first)
			{
				transferID = it->second;
			}
		}

        ZQ::StreamService::C2ClientConf conf;
        conf.upstreamIP = "192.168.81.107";
        conf.clientTransfer = "192.168.81.107";
        conf.getIP = transport;
        conf.getPort = 12000;
        ZQ::StreamService::C2ClientAsyncPtr  getClPtr = new ZQ::StreamService::C2ClientAsync(g_log, conf);
		this->_getClientPtr = getClPtr;
#if 0
        getClPtr->setHttpProxy("http://10.15.10.50:3128");
#endif

        TestC2ClientCBPtr	cbPtr = new TestC2ClientCB();
        ZQ::StreamService::AttrMap reqMap;
        //bool ret = _locateClientPtr->sendLocateRequest(cbPtr, "/cacheserver", "cdntest1234567892010xor.com", "index", reqMap);
        /*_locateClientPtr->setUrl(transport);
        _locateClientPtr->setPort(12000);*/
		bool ret = getClPtr->sendGetRequest(this, transferID, "CDNTEST1234567892010xor.com", reqMap);
		if (!ret)
		{
			g_log(ZQ::common::Log::L_ERROR, CLOGFMT(TestC2ClientCB, "send get request failure"));
			return;
		}
	}

    virtual void OnC2GetResponse(const std::string& contentName, const ZQ::StreamService::AttrMap& locRespParamters)
	{
		g_log(ZQ::common::Log::L_DEBUG, CLOGFMT(TestC2ClientCB, "OnC2GetResponse() entry, index = %d"), g_getIndex++);
        for (ZQ::StreamService::AttrMap::const_iterator it= locRespParamters.begin(); it != locRespParamters.end(); it++)
		{
			printf("%s \t: %s\n", it->first.c_str(), it->second.c_str());
		}

       /* ZQ::StreamService::AttrMap reqMap;
        TestC2ClientCBPtr	cbPtr = new TestC2ClientCB();
        bool ret = _locateClientPtr->sendLocateRequest(this, "/cacheserver", "cdntest1234567892010xor.com", "index", reqMap);*/
	}

	virtual void OnError(int errCode, const std::string& errMsg)
	{
		//error
		printf("%d : %s", errCode, errMsg.c_str());
	}
};

int main(int argc, char* argv[])
{
#ifdef _SYNC_C2CLIENT
	int count = 1;
	while(count-- > 0)
	{
        ZQ::StreamService::C2ClientSync::AttrMap reqMap;
		ZQ::StreamService::C2ClientSync::AttrMap respMap;

		//locate request
		ZQ::StreamService::C2ClientSync::Ptr syncC2Client= new ZQ::StreamService::C2ClientSync(g_log, "10.15.10.74");

        syncC2Client->setProxy("10.15.10.74", 3128);
		bool ret = syncC2Client->sendLocateRequest("/cacheserver", "CDNTEST1234567891021xor.com", "index", reqMap, respMap);

		if (ret)
		{
			g_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientSync, "receive locate response success, index:%d"), count);

			//get request
			std::string strPortNum = respMap[C2CLIENT_PortNum].empty() ? "12000" : respMap[C2CLIENT_PortNum];
			int portNum = atoi(strPortNum.c_str());
			ZQ::StreamService::C2ClientSync::Ptr getC2Client= new ZQ::StreamService::C2ClientSync(g_log, respMap[C2CLIENT_TransferPort], portNum);
			ret = getC2Client->sendGetRequest(respMap[C2CLIENT_TransferID], "CDNTEST1234567891021xor.com", reqMap, respMap);

			if (ret)
			{
				g_log(ZQ::common::Log::L_DEBUG, CLOGFMT(C2ClientSync, "send get request success"));

				for (ZQ::StreamService::C2ClientSync::AttrMap::const_iterator it = respMap.begin(); it != respMap.end(); it++)
				{
					printf("%s \t: %s\n", it->first.c_str(), it->second.c_str());
				}
				printf("index\t: %d\n\n", count);
			} 
			else
			{
				g_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "send get request failure"));
			}
		}
		else{
			g_log(ZQ::common::Log::L_ERROR, CLOGFMT(C2ClientSync, "send locate request failure"));
		}

		
	}
#else

    LibAsync::HttpClient::setup(5);
    ZQ::StreamService::C2ClientConf conf;
    conf.upstreamIP = "192.168.81.107";
    conf.clientTransfer = "192.168.81.107";
    conf.locateIP = "10.15.10.73";
    conf.locatePort = 10080;

    ZQ::StreamService::C2ClientAsyncPtr	c2clPtr = new ZQ::StreamService::C2ClientAsync(g_log, conf);

#if 0
    c2clPtr->setHttpProxy("http://10.15.10.50:3128");
#endif

	int count = 1;
    ZQ::StreamService::AttrMap reqMap, respMap;
	TestC2ClientCBPtr	cbPtr;
	while(count--)
	{
		cbPtr = new TestC2ClientCB();
		cbPtr->_locateClientPtr = c2clPtr;
		bool ret = c2clPtr->sendLocateRequest(cbPtr, "/cacheserver", "CDNTEST1234567892010xor.com", "index", reqMap);
		if (!ret)
		{
			g_log(ZQ::common::Log::L_ERROR, CLOGFMT(Main, "send locate request failure"));
			return 0;
		}

        SYS::sleep(10);
	}
	
#endif

	while(true)
	{
        SYS::sleep(1000);
	}
}
