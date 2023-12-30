// DODCSClient.cpp : Defines the entry point for the console application.
//


#include "TsStorage.h"
#include "DODContentStore.h"

#include <Ice/Ice.h>

void Usage();

bool ActiveProvision(TianShanIce::Storage::ContentStorePrx& csproxy, char* sourceFile, char* contentName);
bool DestroyContent(TianShanIce::Storage::ContentStorePrx& csproxy, char* contentName);



void Usage(char* moduleName)
{
	printf("you must use command as:\n");
	printf("%s <-provision> <sourceURL> <contentname> \n", moduleName);
	printf("OR\n");
	printf("%s <-destroy> <contentname> \n", moduleName);
}

int main(int argc, char* argv[])
{
	if(!( (argc == 3 && strcmp(argv[1], "-destroy") == 0) || (argc == 4 && strcmp(argv[1], "-provision") == 0) ) )
	{
		Usage(argv[0]);
		return 1;
	}

	int status = 0;
    Ice::CommunicatorPtr communicator;
    try 
	{
		Ice::PropertiesPtr properties = Ice::createProperties();

		properties->load("ProvisionTest.config");
		
		std::string propertyPrefix = "ContentStore.Proxy";
		std::string proxyname = properties->getProperty(propertyPrefix);

		int argcParam = 0;
		communicator = Ice::initializeWithProperties(argcParam, 0, properties);
		
		Ice::ObjectPrx base = communicator->stringToProxy(proxyname);

		TianShanIce::Storage::ContentStorePrx csPrx = TianShanIce::Storage::ContentStorePrx::checkedCast(base);
		if (!csPrx)
		{
			printf("Failed to create ConentStore Proxy\n");
			return 1;
		}

		if(strcmp(argv[1], "-provision") == 0)
		{
			bool ret = ActiveProvision(csPrx, argv[2], argv[3]);
			if(ret)
			{
				printf("provision succeed");
			}
			else
			{
				printf("provision failed");
			}
		}
		else if(strcmp(argv[1], "-destroy") == 0 )
		{
			DestroyContent(csPrx, argv[2]);
		}
	}
	catch(const TianShanIce::BaseException& ex)
	{
		std::cerr << ex.message.c_str() << std::endl;
		status = 1;
	}
	catch (const Ice::Exception & ex) 
	{
		std::cerr << ex << std::endl;
		status = 1;
    }
	catch (const char * msg) 
	{
		std::cerr << msg << std::endl;
		status = 1;
    }
	catch(...)
	{
		std::cerr << "Unknow exception" << std::endl;
		status = 1;
	}
    if (communicator) 
	{
		try 
		{
			communicator->destroy();
		} 
		catch (const Ice::Exception & ex) 
		{
			std::cerr << ex << std::endl;
			status = 1;
		}
    }
    return status;
}

bool ActiveProvision(TianShanIce::Storage::ContentStorePrx& csproxy, char* sourceURL, char* contentName)
{
	try
	{
		// get system UTC time
		SYSTEMTIME systime;
		GetSystemTime(&systime);

		// generate start UTC time
		char stime[24];
		sprintf(stime, "%d-%02d-%02dT%02d:%02d:%02d", systime.wYear, systime.wMonth, systime.wDay, 
			                                systime.wHour, systime.wMinute, systime.wSecond);

		std::string starttime = stime;

		// generate the content name
//		char contentName[24];
//		sprintf(contentName, "%d%02d%02d%02d%02d%02d", systime.wYear, systime.wMonth, systime.wDay, 
//			                                systime.wHour, systime.wMinute, systime.wSecond);
//
		// generate stop UTC time
		if(systime.wHour<23)
			systime.wHour++;
		else
			systime.wDay++;

		sprintf(stime, "%d-%02d-%02dT%02d:%02d:%02d", systime.wYear, systime.wMonth, systime.wDay, 
			                                systime.wHour, systime.wMinute, systime.wSecond);
		std::string endtime = stime; 

		DataOnDemand::DODContentPrx contentPrx = DataOnDemand::DODContentPrx::checkedCast(
			                csproxy->openContent(
												contentName,
												TianShanIce::Storage::ctDODTS, 
												true));

		std::string strSource = std::string(sourceURL);
		
		DataOnDemand::DataWrappingParam param;
		param.esPID = 100;
        param.streamType = 1;
        param.subStreamCount = 5;
        param.dataType = 0;
        param.withObjIndex = 1;
        param.objTag = "NAV";
        param.encryptType = 1;

		contentPrx->setDataWrappingParam(param);

		contentPrx->provision(strSource, 
							  TianShanIce::Storage::ctDODTS, 
							  true, 
							  starttime, 
							  endtime, 
							  3840*1024); // 3.75 * 1024 * 1024
		
	}
	catch(const TianShanIce::InvalidStateOfArt& ex)
	{
		std::cerr << ex.message << std::endl;
		return false;
	}
	catch (const Ice::Exception & ex) 
	{
		std::cerr << ex << std::endl;
		return false;
    } 
	catch (const char * msg) 
	{
		std::cerr << msg << std::endl;
		return false;
    }
	catch(...)
	{
		std::cerr << "Unknow exception" << std::endl;
		return false;
	}
	return true;
}

bool DestroyContent(TianShanIce::Storage::ContentStorePrx& csproxy, char* contentName)
{
	try
	{
		// open content, content name is the start time
		TianShanIce::Storage::ContentPrx contentPrx = csproxy->openContent(
							contentName,
							TianShanIce::Storage::ctMPEG2TS, 
							false);
	
		contentPrx->destroy();

		printf("Content %s was destroied\n", contentName);
		
		return true;
	}
	catch(const TianShanIce::Storage::NoResourceException& ex)
	{
		std::cerr << ex.message.c_str() << std::endl;
		return false;
	}
	catch(const TianShanIce::InvalidStateOfArt& ex)
	{
		printf("There is no such Content %s in the contentstore\n", contentName);
		std::cerr << ex.message.c_str() << std::endl;
		return false;
	}
	catch(const TianShanIce::BaseException& ex)
	{
		std::cerr << ex.message.c_str() << std::endl;
		return false;
	}
	catch (const Ice::Exception & ex) 
	{
		std::cerr << ex << std::endl;
		return false;
    } 
	catch (const char * msg) 
	{
		std::cerr << msg << std::endl;
		return false;
    }
	catch(...)
	{
		std::cerr << "Unknow exception" << std::endl;
		return false;
	}

	return true;
}