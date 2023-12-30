/*
#include "MetaLibImpl.h"

#include "Guid.h"
#include "Log.h"
#include "ZQResource.h"
#include "FileLog.h"

#include "ContentLibImpl.h"
#include "EventChannel.h"
//#include "GenEventSinkI.h"

extern "C"
{
#include <time.h>
#include <stdio.h>
#include <direct.h>
}

BOOL WINAPI ConsoleHandler(DWORD event);
bool bQuit = false;

#define CONTENT_LIB_APPNAME		"ContentLib"

int main(int argc, char* argv[])
{
	{
		if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE)==FALSE)
		{
			printf("Unable to install handler!                      \n");
			return -1;
		}

		std::string programRoot = ZQTianShan::getProgramRoot();
		programRoot += FNSEPS;

		::mkdir((programRoot + "logs" FNSEPS).c_str());

		ZQ::common::FileLog _logger((programRoot + "logs" FNSEPS "ContentLib.log").c_str(), ZQ::common::Log::L_DEBUG, 1024*1024*50);
		_logger.setFileSize(1024*1024*50);

		_logger.setVerbosity(ZQ::common::Log::L_DEBUG);

		ZQ::common::setGlogger(&_logger);

		int i =0;
		::Ice::InitializationData initData;
		initData.properties = Ice::createProperties(i, NULL);
		initData.properties->setProperty("Ice.Override.Timeout", "200000");
		initData.properties->setProperty("Ice.ThreadPool.Server.Size","5");
		initData.properties->setProperty("Ice.ThreadPool.Server.SizeMax","20");
		initData.properties->setProperty("Ice.ThreadPool.Client.Size","5");
		initData.properties->setProperty("Ice.ThreadPool.Client.SizeMax","20");

		Ice::CommunicatorPtr ic = Ice::initialize(initData);

		ZQADAPTER_DECLTYPE _pAdapter;
		Ice::ObjectAdapterPtr _evtAdap;
		try
		{
			_pAdapter = ZQADAPTER_CREATE(ic, "ContentLib", argv[3], _logger);
			_pAdapter->activate();

			_evtAdap = ic->createObjectAdapterWithEndpoints("ContentLibEventAdapter", argv[5]);
			_evtAdap->activate();
		}
		catch(const Ice::Exception& ex) 
		{
			_logger(ZQ::common::Log::L_ERROR, CLOGFMT(Environment, "create adapter(%s) caught(%s)"), argv[3], ex.ice_name().c_str());
			return false;
		}

		ZQ::common::NativeThreadPool* _pThreadPool = new ZQ::common::NativeThreadPool(atoi(argv[1]));
		char path[260];
		snprintf(path, sizeof(path), "%s", argv[2]);
		// initial MetaLib
		::ZQTianShan::MetaLib::MetaLibImpl::Ptr _lib = new ::ZQTianShan::MetaLib::MetaLibImpl(_logger, *_pThreadPool, ic, path);

		// initialize database
		::TianShanIce::Repository::MetaDataMap schema;
		::TianShanIce::Repository::MetaDataValue metaData;
		metaData.hintedType = ::TianShanIce::vtStrings;

		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(NETID, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(VOLUMENAME, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(CONTENTNAME, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(CONTENTSTATE, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_ProviderId, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_ProviderAssetId, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_nPVRCopy, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_SubscriberId, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_SourceUrl, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_ParentName, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_Comment, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_FileSize, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_SupportFileSize, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_PixelHorizontal, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_PixelVertical, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_BitRate, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_PlayTime, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_FrameRate, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_SourceType, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_LocalType, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_SubType, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_MD5CheckSum, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_ScheduledProvisonStart, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_ScheduledProvisonEnd, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_MaxProvisonBitRate, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(METADATA_nPVRLeadCopy, metaData));

		_lib->proxy()->registerMetaClass(CONTENTREPLICA, schema); // CONTENTREPLICA 

		schema.clear();
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(ENDPOINT, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(NETID, metaData));

		_lib->proxy()->registerMetaClass(CONTENTSTOREREPLICA, schema); // CONTENTSTOREREPLICA

		schema.clear();
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(NETID, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(VOLUMENAME, metaData));

		_lib->proxy()->registerMetaClass(METAVOLUME, schema); // METAVOLUME

// 		_pAdapter->ZQADAPTER_ADD(ic, _contentLib, CONTENT_LIB_APPNAME);

		// new ContentLib
		TianShanIce::Events::GenericEventSinkPtr _contentlib = new ContentLibImpl(*_lib);
		if (!_contentlib)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(main, "Fail to create ContentLibImpl"));
			return 0;
		}
		
		glog(ZQ::common::Log::L_NOTICE, CLOGFMT(ServerLoadEnv, "do connectEventChannel()"));

		bool bSuccessful = true;	
		TianShanIce::Events::EventChannelImpl::Ptr _eventChannel;
//		TianShanIce::Events::GenericEventSinkPtr test = new EventGateway::GenEventSinkI(glog);
		try
		{
			_eventChannel = new TianShanIce::Events::EventChannelImpl(_evtAdap, argv[4]);
			TianShanIce::Properties qos;
			_eventChannel->sink(_contentlib, qos);
//			_eventChannel->sink(test, qos);
		//	_eventChannel->start();
		}
		catch (TianShanIce::BaseException& ex)
		{
			bSuccessful = false;
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(main, "connectEventChannel(%s) caught(%s: %s)")
				, argv[4], ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (Ice::Exception& ex)
		{
			bSuccessful = false;
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(main, "connectEventChannel(%s) caught(%s)")
				, argv[4], ex.ice_name().c_str());
		}
		catch (...)
		{
			bSuccessful = false;
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(main, "connectEventChannel(%s) caught unexpect exception")
				, argv[4]);
		}

		if(bSuccessful)
			glog(ZQ::common::Log::L_NOTICE, CLOGFMT(main, "connectEventChannel(%s) successfully"), argv[4]);


		printf("\"Ctrl-C\" at any time to exit the program.              \n");
		while (!bQuit)
		{
			static const char* chs="-\\|/";
			static int chi=0;
			chi = ++chi %4;
			printf("\rContentLib is now listening %c", chs[chi]);
			Sleep(200);
		}

		ContentLibImpl::Ptr _contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_contentlib);

		//show all records in DB, just for test
		try
		{
			TianShanIce::Properties searchForMetaData;
			TianShanIce::StrValues expectedMetaDataNames;
			searchForMetaData.insert(TianShanIce::Properties::value_type(NETID, "AAA"));
			TianShanIce::Repository::MetaObjectInfos ret = _lib->proxy()->lookup("*", searchForMetaData, expectedMetaDataNames);
			for(TianShanIce::Repository::MetaObjectInfos::iterator it = ret.begin(); it != ret.end(); it++)
			{
				glog(ZQ::common::Log::L_INFO, CLOGFMT(main, "Record ID[%s] Type[%s]")
					, it->id.c_str(), it->type.c_str());
			}
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(main, "show records caught(%s)")
				, ex.ice_name().c_str());
		}
		catch (...)
		{
		}

		try
		{
			::TianShanIce::Repository::ContentStoreReplicaPrx csPrx = _contentLibPtr->toStoreReplica("AAA");
			if(csPrx)
			{
				Ice::Identity id = csPrx->getNetId();
				::TianShanIce::Repository::ContentStoreReplicaExPrx cePrx = ::TianShanIce::Repository::ContentStoreReplicaExPrx::checkedCast(csPrx);
				Ice::Long LastModified = cePrx->getLastSync();
			}

			::TianShanIce::Repository::MetaVolumePrx mvPrx = _contentLibPtr->toVolume("AAA$/V1");
			if(mvPrx)
			{
				::TianShanIce::Storage::VolumePrx volume = mvPrx->theVolume();
				TianShanIce::Storage::ContentInfos contentInfos;
				TianShanIce::StrValues metaDataNames;
				metaDataNames.push_back(METADATA_ProviderId);
				metaDataNames.push_back(METADATA_ProviderAssetId);
				contentInfos = volume->listContents(metaDataNames, "", -1);
			}

			::TianShanIce::Repository::ContentReplicaPrx ctPrx = _contentLibPtr->toContentReplica("AAA$/V2/zhj.mp3");
			if(ctPrx)
			{
				TianShanIce::Storage::ContentPrx content = ctPrx->theContent();
				TianShanIce::Properties metaData = content->getMetaData();
			}
		}
		catch (TianShanIce::EntityNotFound)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(main, "toStoreReplica() error : Entity Not Found"));
		}
		catch (TianShanIce::BaseException& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(main, "toStoreReplica() caught(%s: %s)")
				, ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(main, "toStoreReplica() caught(%s)")
				, ex.ice_name().c_str());
		}
		catch (...)
		{
		}

		if(_pThreadPool)
		{
			delete _pThreadPool;
			_pThreadPool = NULL;
		}
		
		printf("\rContentLib is quiting...                         ");
	}
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
	switch(CEvent)
	{
	case CTRL_C_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		bQuit = true;
		break;
	}
	return TRUE;
}
*/