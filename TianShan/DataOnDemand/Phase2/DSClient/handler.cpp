// handler.cpp: implementation of the handler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "handler.h"
#include "DataStream.h"

#define		SETTING_ENDPOINT		"endpoint"
#define		SETTING_SPACE			"space"

extern Ice::CommunicatorPtr iceComm;

//////////////////////////////////////////////////////////////////////////

int initHandlers(Commander& cmder)
{
	cmder.add("ping", new PingCmd, 
		"{ping} send a ping message to service");

	cmder.add("create-stream", new CreateStreamCmd, 
		"{create-stream [name] [dest] [pmt_pid] [rate]} create a stream");

	cmder.add("destroy-stream", new DestroyStreamCmd, 
		"{destroy-stream [strm_name]} desotry a stream");

	cmder.add("list-stream", new ListStreamsCmd, 
		"{list-stream} list streams in the space");

	cmder.add("play", new PlayCmd, 
		"{play <strm_name>} play a stream");

	cmder.add("pause", new PauseCmd, 
		"{pause <strm_name>} pause a stream");

	cmder.add("resume", new ResumeCmd, 
		"{resume <strm_name>} resume a stream");

	cmder.add("dump-stream", new DumpStreamCmd, 
		"{dump-stream [strm_name]} dump information for a stream");

	cmder.add("add-muxitem", new AddMuxItemCmd, 
		"{add-muxitem <strm_name> [name] [streamId] [streamType] "
		"[bandWidth] [tag] [expiration] [repeatTime] [cacheType] "
		"[cacheAddr] [encryptMode] [subchannelCount]} "
		"add a muxitem into the stream");

	cmder.add("remove-muxitem", new RemoveMuxItemCmd, 
		"{remove-muxitem <strm_name> <muxitem_name>} "
		"remove a muxitem from the stream");

	cmder.add("notify", new NotifyCmd, 
		"{notify <method> <strm_name> <muxitem_name> <filename>} "
		"add a muxitem into the stream");

	cmder.add("dump-muxitem", new DumpMuxItemCmd, 
		"{dump-muxitem  <strm_name> <muxitem_name> "
		"dump information for a mux item");

	cmder.add("list-muxitem", new ListMuxItemCmd, 
		"{list-muxitem <strm_name>} list all mux items in the streams");

	cmder.add("total-muxitem", new TotalMuxItem, 
		"{total-muxitem} total all mux items in the space");

	char hostName[MAX_PATH];
	unsigned long len = sizeof(hostName);
	GetComputerNameA(hostName, &len);
	strlwr(hostName);
	cmder.setSettingString(SETTING_SPACE, hostName, true);
	cmder.setSettingString(SETTING_ENDPOINT, "default -p 10040", true);

	return 0;
}

//////////////////////////////////////////////////////////////////////////

std::string createStreamServiceProxy(Commander& cmder)
{
	char proxy[256];
	std::string endpoint;
	cmder.getSettingString(SETTING_ENDPOINT, endpoint);

	sprintf(proxy, "%s:%s", "DataStreamService", 
			endpoint.c_str());

	return proxy;
}

std::string createStreamProxy(Commander& cmder, 
							  const std::string& strmName)
{
	char proxy[256];

	std::string endpoint;
	cmder.getSettingString(SETTING_ENDPOINT, endpoint);
	std::string space;
	cmder.getSettingString(SETTING_SPACE, space);

	sprintf(proxy, "%s/%s:%s", space.c_str(), strmName.c_str(), 
		endpoint.c_str());

	return proxy;
}

std::string createMuxItemProxy(Commander& cmder, 
							   const std::string& strmName, 
							   const std::string& muxItemName)
{
	char proxy[256];

	std::string endpoint;
	cmder.getSettingString(SETTING_ENDPOINT, endpoint);
	std::string space;
	cmder.getSettingString(SETTING_SPACE, space);

	sprintf(proxy, "%s/%s\\\\\\/%s:%s", space.c_str(), 
		muxItemName.c_str(), strmName.c_str(), endpoint.c_str());
	return proxy;
}

::DataOnDemand::DataStreamServicePrx getStreamService(Commander& cmder)
{

	std::string proxy;

	try {
		
		proxy = createStreamServiceProxy(cmder);

		Ice::ObjectPrx obj = iceComm->stringToProxy(proxy);
		::DataOnDemand::DataStreamServicePrx strmSvc;
		if (obj == NULL) {
			return NULL;
		} else {
			strmSvc = ::DataOnDemand::DataStreamServicePrx::checkedCast(obj);
			return strmSvc;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		cmder.print("proxy string is: %s\n", proxy.c_str());
		return NULL;
	}
}

::DataOnDemand::DataStreamPrx getStream(Commander& cmder,
										const std::string& strmName)
{
	std::string proxy;
	proxy = createStreamProxy(cmder, strmName);
	try {
		Ice::ObjectPrx obj = iceComm->stringToProxy(proxy);
		::DataOnDemand::DataStreamPrx strm;
		if (obj == NULL) {
			cmder.print("stringToProxy failed\n");
			return NULL;
		} else {
			strm = ::DataOnDemand::DataStreamPrx::checkedCast(obj);
			return strm;
		}
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		cmder.print("proxy string is: %s\n", proxy.c_str());
		return NULL;

	}
}

::DataOnDemand::MuxItemPrx getMuxItem(Commander& cmder,
									  const std::string& strmName, 
									  const std::string& muxItemName)
{
	std::string proxy;
	proxy = createMuxItemProxy(cmder, strmName, muxItemName);
	try {
		Ice::ObjectPrx obj = iceComm->stringToProxy(proxy);
		::DataOnDemand::MuxItemPrx muxItem;
		if (obj == NULL) {
			cmder.print("stringToProxy failed\n");
			return NULL;
		} else {
			muxItem = ::DataOnDemand::MuxItemPrx::checkedCast(obj);
			return muxItem;
		}
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		cmder.print("proxy string is: %s\n", proxy.c_str());
		return NULL;

	}
}

//////////////////////////////////////////////////////////////////////////

int PingCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() > 1) {
		return CMD_R_INVLIADARGS;
	}
	
	int r;

	try {
		
		::DataOnDemand::DataStreamServicePrx strmSvc = getStreamService(cmder);

		if (strmSvc == NULL) {

			r = CMD_R_FAILED;

		} else {
			std::string space;
			cmder.getSettingString(SETTING_SPACE, space);
			strmSvc->ping(space);
			r = CMD_R_OK;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////

int DestroyStreamCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() != 2 && args.size() != 1) {
		return CMD_R_INVLIADARGS;
	}
	
	int r;
	try {
		
		if (args.size() == 1) {
			::DataOnDemand::DataStreamServicePrx strmSvc = getStreamService(cmder);

			if (strmSvc == NULL) {

				r = CMD_R_FAILED;

			} else {
				std::string space;
				cmder.getSettingString(SETTING_SPACE, space);
				::TianShanIce::StrValues strs = strmSvc->listStrems(space);
				::TianShanIce::StrValues::iterator it;				

				for (it = strs.begin(); it != strs.end(); it ++) {
					::DataOnDemand::DataStreamPrx strm = getStream(cmder, 
						*it);

					if (strm == NULL) {
						cmder.print("getStream(%s) failed\n", it->c_str());
						r = CMD_R_FAILED;
					} else {
						strm->destroy();
						r = CMD_R_OK;
					}
				}

				r = CMD_R_OK;
			}
		} else {

			::DataOnDemand::DataStreamPrx strm = getStream(cmder, args[1]);
			if (strm == NULL) {
				cmder.print("getStream(%s) failed\n", args[1].c_str());
				r = CMD_R_FAILED;
			} else {
				strm->destroy();
				r = CMD_R_OK;
			}
		}
	
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////

int ListStreamsCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() > 1) {
		return CMD_R_INVLIADARGS;
	}
	
	int r;

	try {
		
		::DataOnDemand::DataStreamServicePrx strmSvc = getStreamService(cmder);

		if (strmSvc == NULL) {

			r = CMD_R_FAILED;

		} else {

			std::string space;
			cmder.getSettingString(SETTING_SPACE, space);
			::TianShanIce::StrValues strs = strmSvc->listStrems(space);
			::TianShanIce::StrValues::iterator it;
			cmder.print("stream list(%d):\n", strs.size());
			for (it = strs.begin(); it != strs.end(); it ++) {
				cmder.print("  %s\n", it->c_str());
			}

			r = CMD_R_OK;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////

int CreateStreamCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() != 1 && args.size() != 5) {
		
		return CMD_R_INVLIADARGS;
	}

	::DataOnDemand::StreamInfo strmInfo;

	if (args.size() == 1) {
		strmInfo.name = cmder.inputStr("stream name", "test");
		strmInfo.destAddress = cmder.inputStr("dest address", 
			"UDP:192.168.81.101:777");
		strmInfo.pmtPid = cmder.inputInt("PMT PID", 127);
		strmInfo.totalBandwidth = cmder.inputInt("rate", 1000000);
	} else {

		strmInfo.name = args[1];
		strmInfo.destAddress = args[2];
		strmInfo.pmtPid = atoi(args[3].c_str());
		strmInfo.totalBandwidth = atoi(args[4].c_str());
	}

	int r;
	int i = 0;
	std::string proxy;
	try {
		
		::DataOnDemand::DataStreamServicePrx strmSvc = getStreamService(cmder);

		if (strmSvc == NULL) {

			r = CMD_R_FAILED;

		} else {

			std::string space;
			cmder.getSettingString(SETTING_SPACE, space);
			if (strmSvc->createStreamByApp(NULL, space, strmInfo) == NULL) {
				r = CMD_R_FAILED;
				cmder.print("createStream failed\n");
			} else
				r = CMD_R_OK;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////

int PlayCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() < 2) {
		return CMD_R_INVLIADARGS;
	}
	
	int r;
	try {
		
		::DataOnDemand::DataStreamPrx strm = getStream(cmder, args[1]);
		if (strm == NULL) {
			cmder.print("getStream() failed\n");
			r = CMD_R_FAILED;
		} else {
			if (strm->play())
				r = CMD_R_OK;
			else {
				cmder.print("play failed\n");
				r = CMD_R_FAILED;
			}
		}
	
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;

}
//////////////////////////////////////////////////////////////////////////

int PauseCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() < 2) {
		return CMD_R_INVLIADARGS;
	}
	
	int r;
	try {
		
		::DataOnDemand::DataStreamPrx strm = getStream(cmder, args[1]);
		if (strm == NULL) {
			cmder.print("getStream() failed\n");
			r = CMD_R_FAILED;
		} else {
			if (strm->pause())
				r = CMD_R_OK;
			else {
				cmder.print("pause failed\n");
				r = CMD_R_FAILED;
			}
		}
	
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;

}

//////////////////////////////////////////////////////////////////////////

int ResumeCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() < 2) {
		return CMD_R_INVLIADARGS;
	}
	
	int r;
	try {
		
		::DataOnDemand::DataStreamPrx strm = getStream(cmder, args[1]);
		if (strm == NULL) {
			cmder.print("getStream() failed\n");
			r = CMD_R_FAILED;
		} else {
			if (strm->resume())
				r = CMD_R_OK;
			else {
				cmder.print("resume failed\n");
				r = CMD_R_FAILED;
			}
		}
	
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;

}

//////////////////////////////////////////////////////////////////////////

std::string DumpStreamCmd::getStateName(int state)
{
	switch (state) {
	case TianShanIce::Streamer::stsSetup:
		return "Setup";
	case TianShanIce::Streamer::stsStreaming:
		return "Streaming";
	case TianShanIce::Streamer::stsStop:
		return "Stop";
	case TianShanIce::Streamer::stsPause:
		return "Pause";
	}

	return "Unknown";
}

int DumpStreamCmd::dumpStream(Commander& cmder, const std::string& strmName)
{
	int r;
	try {
		::DataOnDemand::DataStreamPrx strm = getStream(cmder, strmName);
		if (strm == NULL) {
			cmder.print("getStream() failed\n");
			r = CMD_R_FAILED;
		} else {
			::DataOnDemand::StreamInfo strmInfo = strm->getInfo();
			cmder.print("  stream name: %s\n", strmInfo.name.c_str());
			cmder.print("  dest address: %s\n", strmInfo.destAddress.c_str());
			cmder.print("  PMT PID: %d\n", strmInfo.pmtPid);
			cmder.print("  rate: %d\n", strmInfo.totalBandwidth);
			cmder.print("  state: %s\n", 
				getStateName(strm->getCurrentState()).c_str());

			_rateCount += strmInfo.totalBandwidth;

			r = CMD_R_OK;
		}
	
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;
}

int DumpStreamCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() > 2) {
		return CMD_R_INVLIADARGS;
	}
	
	int r;

	try {

		if (args.size() == 2) {
			return dumpStream(cmder, args[1]);
		}		

		::DataOnDemand::DataStreamServicePrx strmSvc = getStreamService(cmder);

		if (strmSvc == NULL) {

			r = CMD_R_FAILED;

		} else {
			std::string space;
			cmder.getSettingString(SETTING_SPACE, space);
			::TianShanIce::StrValues strs = strmSvc->listStrems(space);
			::TianShanIce::StrValues::iterator it;

			_rateCount = 0;
			cmder.print("dump streams(%d):\n", strs.size());
			for (it = strs.begin(); it != strs.end(); it ++) {
				dumpStream(cmder, *it);
				cmder.print("-----------------\n");
			}

			cmder.print("rate total: %d\n", _rateCount);

			r = CMD_R_OK;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////

int AddMuxItemCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() != 2 && args.size() != 13) {
		
		return CMD_R_INVLIADARGS;
	}

	::DataOnDemand::MuxItemInfo muxItemInfo;
	if (args.size() == 2) {
		muxItemInfo.name = cmder.inputStr("mux item name", "muxitem1");
		muxItemInfo.streamId = cmder.inputInt("streamId.", 721);
		muxItemInfo.streamType = cmder.inputInt("streamType.", 128);
		muxItemInfo.bandWidth = cmder.inputInt("rate", 100000);
		muxItemInfo.tag = cmder.inputInt("tag", 'TXT\0');
		muxItemInfo.expiration = cmder.inputInt("expiration", 100000);
		muxItemInfo.repeatTime = cmder.inputInt("repeatTime", 0);
		muxItemInfo.ctype = (::DataOnDemand::CacheType )
			cmder.inputInt("cacheType", 0);
		muxItemInfo.cacheAddr = cmder.inputStr("cacheAddr");
		muxItemInfo.encryptMode = cmder.inputInt("encryptMode", 0);
		muxItemInfo.subchannelCount = cmder.inputInt("subchannelCount", 0);
	} else {
		muxItemInfo.name = args[2];
		muxItemInfo.streamId = atoi(args[3].c_str());
		muxItemInfo.streamType = atoi(args[4].c_str());
		muxItemInfo.bandWidth = atoi(args[5].c_str());
		muxItemInfo.tag = atoi(args[6].c_str());
		muxItemInfo.expiration = atoi(args[7].c_str());
		muxItemInfo.repeatTime = atoi(args[8].c_str());
		muxItemInfo.ctype = (::DataOnDemand::CacheType )atoi(args[9].c_str());
		muxItemInfo.cacheAddr = args[10];
		muxItemInfo.encryptMode = atoi(args[11].c_str());
		muxItemInfo.subchannelCount = atoi(args[12].c_str());
	}

	int r;
	std::string proxy;
	try {
		
		::DataOnDemand::DataStreamPrx strm = getStream(cmder, args[1]);

		if (strm == NULL) {

			cmder.print("not foud the stream(%s)\n", args[1].c_str());

			r = CMD_R_FAILED;

		} else {

			if (strm->createMuxItem(muxItemInfo) == NULL) {
				cmder.print("createMuxItem failed\n");
				r = CMD_R_FAILED;
			} else 
				r = CMD_R_OK;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;
}

//////////////////////////////////////////////////////////////////////////

int RemoveMuxItemCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() != 3) {
		
		return CMD_R_INVLIADARGS;
	}

	int r;
	std::string proxy;
	try {
		
		::DataOnDemand::MuxItemPrx muxItem = getMuxItem(cmder, 
			args[1], args[2]);

		if (muxItem == NULL) {
			
			cmder.print("cannot get the mux item(%s::%s)\n", 
				args[1].c_str(), args[2].c_str());
			r = CMD_R_FAILED;

		} else {

			muxItem->destroy();

			r = CMD_R_OK;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;
}


//////////////////////////////////////////////////////////////////////////

int NotifyCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() != 5) {
		
		return CMD_R_INVLIADARGS;
	}

	try {
		::DataOnDemand::MuxItemPrx muxItem = getMuxItem(
			cmder, args[2], args[3]);

		if (args[1] == "full") {

			muxItem->notifyFullUpdate(args[4]);

		} else if (args[1] == "add") {

			muxItem->notifyFileAdded(args[4]);

		} else if (args[1] == "remove") {

			muxItem->notifyFileDeleted(args[4]);

		} else {
			return CMD_R_INVLIADARGS;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return CMD_R_OK;
}

//////////////////////////////////////////////////////////////////////////

const char* DumpMuxItemCmd::getEncryptName(int encryptMode)
{
	switch (encryptMode) {
	case 0:
		return "None";
	case 1:
		return "Normal";
	case 2:
		return "Simple";
	case 4:
		return "Compress";
	case 5:
		return "Normal&Compress";
	case 6:
		return "Simple&Compress";
	}

	return "Unknown";
}

int DumpMuxItemCmd::dumpMuxItem(Commander& cmder, 
								const std::string& strmName, 
								const std::string& muxItemName)
{
	int r;
	std::string proxy;

	char tag[4];

	try {
		
		::DataOnDemand::MuxItemPrx muxItem = getMuxItem(cmder, 
			strmName, muxItemName);

		if (muxItem == NULL) {

			r = CMD_R_FAILED;

		} else {

			DataOnDemand::MuxItemInfo muxItemInfo = muxItem->getInfo();

			cmder.print("  mux item name: %s\n", muxItemInfo.name.c_str());
			cmder.print("  streamId: %d\n", muxItemInfo.streamId);
			cmder.print("  streamType: %d\n", muxItemInfo.streamType);
			cmder.print("  rate: %d\n", muxItemInfo.bandWidth);
			
			tag[0] = (muxItemInfo.tag >> 24) & 0xff;
			tag[1] = (muxItemInfo.tag >> 16) & 0xff;
			tag[2] = (muxItemInfo.tag >> 8) & 0xff;
			tag[3] = 0;
			
			cmder.print("  tag: %d(0x%x) \'%s\'\n", muxItemInfo.tag, 
				muxItemInfo.tag, tag);

			cmder.print("  expiration: %d\n", muxItemInfo.expiration);
			cmder.print("  repeatTime: %d\n", muxItemInfo.repeatTime);
			cmder.print("  ctype: %d\n", muxItemInfo.ctype);
			cmder.print("  cacheAddr: %s\n", muxItemInfo.cacheAddr.c_str());
			cmder.print("  encryptMode: %d(%s)\n", muxItemInfo.encryptMode, 
				getEncryptName(muxItemInfo.encryptMode));
			cmder.print("  subchannelCount: %d\n", muxItemInfo.subchannelCount);

			_rateCount += muxItemInfo.bandWidth;
			
			r = CMD_R_OK;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;
}

int DumpMuxItemCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() < 2) {
		
		return CMD_R_INVLIADARGS;
	}

	if (args.size() == 3)
		return dumpMuxItem(cmder, args[1], args[2]);

	int r;
	try {
		
		::DataOnDemand::DataStreamPrx strm = getStream(cmder, args[1]);
		if (strm == NULL) {
			cmder.print("getStream() failed\n");
			r = CMD_R_FAILED;
		} else {
			Ice::StringSeq strs = strm->listMuxItems();

			_rateCount = 0;

			Ice::StringSeq::iterator it;
			cmder.print("dump mux items(%d):\n", strs.size());
			for (it = strs.begin(); it != strs.end(); it ++) {
				dumpMuxItem(cmder, args[1], *it);
				cmder.print("-----------------\n");
			}

			cmder.print("rate total: %d\n", _rateCount);

			r = CMD_R_OK;

		}
	
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;	
	

}

//////////////////////////////////////////////////////////////////////////

int ListMuxItemCmd::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() < 2) {
		return CMD_R_INVLIADARGS;
	}
	
	int r;
	try {
		
		::DataOnDemand::DataStreamPrx strm = getStream(cmder, args[1]);
		if (strm == NULL) {
			cmder.print("getStream() failed\n");
			r = CMD_R_FAILED;
		} else {
			Ice::StringSeq strs = strm->listMuxItems();

			Ice::StringSeq::iterator it;
			cmder.print("mux item list(%d):\n", strs.size());
			for (it = strs.begin(); it != strs.end(); it ++) {
				cmder.print("  %s\n", it->c_str());
			}

			r = CMD_R_OK;

		}
	
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;	
}

//////////////////////////////////////////////////////////////////////////


int TotalMuxItem::handler(Commander& cmder, const Arguments& args)
{
	if (args.size() > 1) {
		return CMD_R_INVLIADARGS;
	}
	
	int r;

	try {

		::DataOnDemand::DataStreamServicePrx strmSvc = getStreamService(cmder);

		if (strmSvc == NULL) {

			r = CMD_R_FAILED;

		} else {
			std::string space;
			cmder.getSettingString(SETTING_SPACE, space);
			::TianShanIce::StrValues strs = strmSvc->listStrems(space);
			::TianShanIce::StrValues::iterator it;

			int muxItemCount = 0;
			
			for (it = strs.begin(); it != strs.end(); it ++) {
				::DataOnDemand::DataStreamPrx strm = getStream(cmder, *it);
				if (strm != NULL) {
					Ice::StringSeq muxItems = strm->listMuxItems();
					cmder.print("  stream: %s, mux items: %d\n", 
						it->c_str(), muxItems.size());

					muxItemCount += muxItems.size();

				} else {
					cmder.print("cannot get stream: %s\n", it->c_str());
				}
			}

			cmder.print("stream count: %d\n", strs.size());
			cmder.print("total mux items: %d\n", muxItemCount);

			r = CMD_R_OK;
		}
		
	} catch (Ice::Exception& e) {
		cmder.print("exception has occurred. exception name is: %s\n",
			e.ice_name().c_str());
		return CMD_R_FAILED;
	}

	return r;

}
