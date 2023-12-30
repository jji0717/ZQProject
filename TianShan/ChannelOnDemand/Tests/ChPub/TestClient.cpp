// TestClient.cpp: implementation of the TestClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ChPub.h"
#include "TestClient.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define ADAPTER_NAME_ChOD				"ChannelOnDemandAdapter"
#define DEFAULT_ENDPOINT_ChOD			"tcp -h 10.15.10.250 -p 9832"

#define SERVICE_NAME_ChannelPublisher	"ChannelPublisher"
#define SERVICE_NAME_ChannelPublisherEx	"ChannelPublisherEx"
#define CHANNEL_ONDEMAND_APPNAME		"ChannelOnDemandApp"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TestClient::TestClient()
{

}

TestClient::~TestClient()
{
	if(_isConnected)
	{
		disconnect();
	}
}

::std::string TestClient::connect(::std::string endpoint)
{
	std::string errmsg = "unknown error";
	if(_isConnected)
	{
		_ic->destroy();
		_isConnected = false;
	}

	_publisherPrx = NULL;


	int i = 0;
	_ic = ::Ice::initialize(i, NULL);

	if(endpoint.empty())
	{
		errmsg = "Invalid endpoint \""+ endpoint + "\"!";
		return errmsg;
	}

//	{
//		try
//		{
//			_pStreamSPrx = ::TianShanIce::Streamer::StreamServicePrx::checkedCast(_ic->stringToProxy(::std::string("StreamSmithAdmin:tcp -h 10.15.10.250 -p 10000")));
//		}
//		catch(::Ice::ProxyParseException e)
//		{
//			errmsg = "There was an error while parsing the stringified proxy \"ChannelPublisher:" + endpoint + "\"!";
//			return errmsg;
//		}
//		catch(::Ice::NoEndpointException e)
//		{
//			errmsg = "Endpoint no found!";
//			return errmsg;
//		}
//		catch(::Ice::ObjectNotFoundException e)
//		{
//			errmsg = "Can not found object!";
//			return errmsg;
//		}
//		catch(::Ice::ObjectNotExistException e)
//		{
//			errmsg = "Can not found object!";
//			return errmsg;
//		}
//		catch(...)
//		{
//			errmsg = "Unknown exception got when parsing \"ChannelPublisher:" + endpoint + "\"!";
//			return errmsg;
//		}
//		
//		if(!_pStreamSPrx)
//		{
//			errmsg = "Failed to connect to ChannelPublisher \"ChannelPublisher:" + endpoint + "\"!";
//			return errmsg;
//		}
//	}
//	
	{
		_pAppPrx = NULL;
		try
		{
			_pAppPrx = NS_PREFIX(ChannelOnDemand::ChannelOnDemandAppPrx)::checkedCast(_ic->stringToProxy(::std::string(CHANNEL_ONDEMAND_APPNAME ":") + endpoint));
		}
		catch(::Ice::ProxyParseException e)
		{
			errmsg = "There was an error while parsing the stringified proxy \"ChannelPublisher:" + endpoint + "\"!";
			return errmsg;
		}
		catch(::Ice::NoEndpointException e)
		{
			errmsg = "Endpoint no found!";
			return errmsg;
		}
		catch(::Ice::ObjectNotFoundException e)
		{
			errmsg = "Can not found object!";
			return errmsg;
		}
		catch(::Ice::ObjectNotExistException e)
		{
			errmsg = "Can not found object!";
			return errmsg;
		}
		catch (::Ice::Exception& e)
		{
			errmsg = "Can not found object: " + e.ice_name();
			return errmsg;
		}
		catch(...)
		{
			errmsg = "Unknown exception got when parsing \"ChannelPublisher:" + endpoint + "\"!";
			return errmsg;
		}
		
		if(!_pAppPrx)
		{
			errmsg = "Failed to connect to ChannelPublisher \"ChannelPublisher:" + endpoint + "\"!";
			return errmsg;
		}

		
	}

	try
	{
		_publisherPrx = NS_PREFIX(ChannelOnDemand::ChannelPublisherExPrx)::checkedCast(_ic->stringToProxy(::std::string(SERVICE_NAME_ChannelPublisher ":") + endpoint));
	}
	catch(::Ice::ProxyParseException e)
	{
		errmsg = "There was an error while parsing the stringified proxy \"ChannelPublisher:" + endpoint + "\"!";
		return errmsg;
	}
	catch(::Ice::NoEndpointException e)
	{
		errmsg = "Endpoint no found!";
		return errmsg;
	}
	catch(::Ice::ObjectNotFoundException e)
	{
		errmsg = "Can not found object!";
		return errmsg;
	}
	catch(::Ice::ObjectNotExistException e)
	{
		errmsg = "Can not found object!";
		return errmsg;
	}
	catch (::Ice::Exception& e)
	{
		errmsg = "Can not found object: " + e.ice_name();
		return errmsg;
	}
	catch(...)
	{
		errmsg = "Unknown exception got when parsing \"ChannelPublisher:" + endpoint + "\"!";
		return errmsg;
	}

	if(!_publisherPrx)
	{
		errmsg = "Failed to connect to ChannelPublisher \"ChannelPublisher:" + endpoint + "\"!";
		return errmsg;
	}

#if 0
	std::string xxdfef = _publisherPrx->getChannelByStream("b52ac6ca-f171-468c-a399-da8a9cdc133b");


	::ChannelOnDemand::ChannelPublishPointPrx channelPrx;
	{
		try
		{
			channelPrx = _publisherPrx->open("CCTV1");
		}
		catch (::TianShanIce::InvalidParameter& ex)
		{
			errmsg = "Replace item failed with " + ex.message;
		}
		catch (::TianShanIce::ServerError& ex)
		{
			errmsg = "Replace item failed with " + ex.message;
		}
		catch (Ice::Exception& ex)
		{
			errmsg = "Replace item failed with " + ex.ice_name();
		}
	
		if (!channelPrx)
		{
			try
			{
				channelPrx = _publisherPrx->publish("CCTV1", 0, "cctv1");
				if (!channelPrx)
				{
					errmsg = "Fail to create CCTV1 channel";
					return errmsg;
				}
			}
			catch(...)
			{
				errmsg = "CCTV1 not found";
				return errmsg;
			}
		}
	}
	
	::ChannelOnDemand::ChannelItem newItem;
	newItem.broadcastStart = "2006-11-13T06:00:00";
	newItem.forceNormalSpeed = false;
	newItem.inTimeOffset = 0;
	newItem.outTimeOffset = 0;
	newItem.playable = true;
	newItem.spliceIn = 0;
	newItem.spliceOut = 0;

	try
	{
		newItem.contentName = "0008000b";
		channelPrx->appendItem(newItem);
		newItem.contentName = "0008000d";
		newItem.broadcastStart = "2006-11-13T06:15:00";

		channelPrx->appendItem(newItem);
		newItem.contentName = "0008001a";
		newItem.broadcastStart = "2006-11-13T06:30:00";

		channelPrx->appendItem(newItem);
		newItem.contentName = "0008002e";
		newItem.broadcastStart = "2006-11-13T06:40:00";

		channelPrx->appendItem(newItem);
		newItem.contentName = "00080054";
		newItem.broadcastStart = "2006-11-13T06:50:00";

		channelPrx->appendItem(newItem);
	}
	catch(...)
	{
		
	}

#endif

#if 0
	{
		::ChannelOnDemand::ChannelPublishPointPrx channelPrx;
		try
		{
			channelPrx = _publisherPrx->open("CCTV2");
		}
		catch (::TianShanIce::InvalidParameter& ex)
		{
			errmsg = "Replace item failed with " + ex.message;
		}
		catch (::TianShanIce::ServerError& ex)
		{
			errmsg = "Replace item failed with " + ex.message;
		}
		catch (Ice::Exception& ex)
		{
			errmsg = "Replace item failed with " + ex.ice_name();
		}

		if (!channelPrx)
		{
			try
			{
				channelPrx = _publisherPrx->publish("CCTV2", 0, "cctv2");
				if (!channelPrx)
				{
					errmsg = "Fail to create CCTV2 channel";
					return errmsg;
				}
			}
			catch(...)
			{
				errmsg = "CCTV2 not found";
				return errmsg;
			}
		}
		
		::ChannelOnDemand::ChannelItem newItem;
		newItem.contentName = "00080018";
		newItem.broadcastStart = "2006-11-13T06:00:00";
		newItem.forceNormalSpeed = false;
		newItem.inTimeOffset = 0;
		newItem.outTimeOffset = 0;
		newItem.playable = true;
		newItem.spliceIn = 0;
		newItem.spliceOut = 0;
		
		try
		{
			channelPrx->appendItem(newItem);
			newItem.contentName = "0008001c";
newItem.broadcastStart = "2006-11-13T06:10:00";
			channelPrx->appendItem(newItem);
			newItem.contentName = "00080016";
newItem.broadcastStart = "2006-11-13T06:25:00";
			channelPrx->appendItem(newItem);
			newItem.contentName = "0008000e";
newItem.broadcastStart = "2006-11-13T06:35:00";
			channelPrx->appendItem(newItem);
			newItem.contentName = "00080019";
newItem.broadcastStart = "2006-11-13T06:45:00";
			channelPrx->appendItem(newItem);
			newItem.contentName = "00080010";
newItem.broadcastStart = "2006-11-13T06:55:00";
			channelPrx->appendItem(newItem);

		}
		catch(...)
		{
			
		}
	}
#endif

	_isConnected = true;
	errmsg = "";
	return errmsg;
}

::std::string TestClient::disconnect()
{
	if(_isConnected)
	{
		_ic->destroy();
		_isConnected = false;
	}
	return "";
}

::std::string TestClient::list(CTreeCtrl& treectrl, HTREEITEM hRoot)
{
	std::string errmsg = "unknown error";
	if(!_publisherPrx)
	{
		errmsg = "ChannelPublisher not open!";
		return errmsg;
	}

	treectrl.DeleteAllItems();

	HTREEITEM hTmpRoot = treectrl.InsertItem(ROOTSTR, 0, 0);

	TianShanIce::StrValues channels = _publisherPrx->list();

	if(channels.empty())
	{
		HTREEITEM hNoCh = treectrl.InsertItem(NOCHANNELSTR, 99, 99, hTmpRoot);	// no channel
	}
	else
	{
		// go thru all channels
		for(size_t i=0; i<channels.size(); i++)
		{
			HTREEITEM hTmpCh = treectrl.InsertItem(channels[i].c_str(), 1, 1, hTmpRoot);

			// open channel and get programs
			::std::string tmpName, tmpDesc;

			try
			{
				open(treectrl, hTmpCh, tmpName, tmpDesc);
			}
			catch(Ice::Exception& ex)
			{
				
			}
			catch(...)
			{

			}
		}
	}

	treectrl.Expand(hTmpRoot, TVE_EXPAND);
	errmsg = "";
	return errmsg;
}

::std::string TestClient::publish(CTreeCtrl& treectrl, HTREEITEM hRoot, ::std::string name, ::std::string desc)
{
	std::string errmsg = "unknown error";
	if(!_publisherPrx)
	{
		errmsg = "ChannelPublisher not open!";
		return errmsg;
	}
	if(!g_IsValidNode(treectrl, hRoot))
	{
		errmsg = "Invalid tree node selected!";	// no valid tree node, such as a "... no channels ..." node
		return errmsg;
	}

#ifdef USE_OLD_NS
	::ChannelOnDemand::ChannelPublishPointPrx channelPrx = NULL;
#else
	::TianShanIce::Application::PublishPointPrx channelPrx = NULL;
#endif //USE_OLD_NS

	channelPrx = _publisherPrx->publish(name, 0, desc);
	if(!channelPrx)
	{
		errmsg = "Publish channel failed!";
		return errmsg;
	}

	HTREEITEM hFirstCh = treectrl.GetChildItem(hRoot);
	if(!g_IsValidNode(treectrl, hFirstCh))
		treectrl.DeleteItem(hFirstCh);

	treectrl.InsertItem(name.c_str(), 1, 1, hRoot);
	
	treectrl.Expand(hRoot, TVE_EXPAND);

	errmsg = "";
	return errmsg;
}

::std::string TestClient::open(CTreeCtrl& treectrl, HTREEITEM hChannel, ::std::string& name, ::std::string& desc)
{
	std::string errmsg = "unknown error";
	if(!_publisherPrx)
	{
		errmsg = "ChannelPublisher not open!";
		return errmsg;
	}
	if(!g_IsValidNode(treectrl, hChannel))
	{
		errmsg = "Invalid tree node selected!";	// no valid tree node, such as a "... no channels ..." node
		return errmsg;
	}

	CString chName = treectrl.GetItemText(hChannel);

	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;
#ifdef USE_OLD_NS
	channelPrx = _publisherPrx->open((LPCSTR)chName);
#else

	pubPrx = _publisherPrx->open((LPCSTR)chName);
	channelPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
//	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = _publisherPrx->open((LPCSTR)chName);

	if(!channelPrx)
	{
		errmsg = "Open channel failed!";
		return errmsg;
	}

	// delete all children first
	if (treectrl.ItemHasChildren(hChannel))
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = treectrl.GetChildItem(hChannel);
		
		while (hChildItem != NULL)
		{
			hNextItem = treectrl.GetNextItem(hChildItem, TVGN_NEXT);
			treectrl.DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}

	// re-query all children items
	TianShanIce::StrValues items = channelPrx->getItemSequence();
	name = channelPrx->getName();
	desc = channelPrx->getDesc();
	for(size_t i = 0; i<items.size(); i++)
	{
		::std::string contentName = items[i];
		CI_NS_PREFIX(ChannelItem) tmpItem = channelPrx->findItem(contentName);
		if(!tmpItem.contentName.empty())
		{
			treectrl.InsertItem(tmpItem.contentName.c_str(), 2, 2, hChannel);
		}
	}

	treectrl.Expand(hChannel, TVE_EXPAND);
	
	errmsg = "";
	return errmsg;
}

::std::string TestClient::destroy(CTreeCtrl& treectrl, HTREEITEM hChannel)
{
	std::string errmsg = "unknown error";
	if(!_publisherPrx)
	{
		errmsg = "ChannelPublisher not open!";
		return errmsg;
	}
	if(!g_IsValidNode(treectrl, hChannel))
	{
		errmsg = "Invalid tree node selected!";	// no valid tree node, such as a "... no channels ..." node
		return errmsg;
	}

	CString name = treectrl.GetItemText(hChannel);

	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;

#ifdef USE_OLD_NS
	channelPrx = _publisherPrx->open((LPCSTR)name);
#else
	pubPrx = _publisherPrx->open((LPCSTR)name);
	channelPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS

//	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = _publisherPrx->open((LPCSTR)name);
	
	if(!channelPrx)
	{
		errmsg = "Open channel failed!";
		return errmsg;
	}

	channelPrx->destroy();

	// delete all children first
	if (treectrl.ItemHasChildren(hChannel))
	{
		HTREEITEM hNextItem;
		HTREEITEM hChildItem = treectrl.GetChildItem(hChannel);
		
		while (hChildItem != NULL)
		{
			hNextItem = treectrl.GetNextItem(hChildItem, TVGN_NEXT);
			treectrl.DeleteItem(hChildItem);
			hChildItem = hNextItem;
		}
	}
	// delete self
	treectrl.DeleteItem(hChannel);
	
	HTREEITEM hTmpRoot = treectrl.GetRootItem();
	if(NULL == treectrl.GetChildItem(hTmpRoot))
		HTREEITEM hNoCh = treectrl.InsertItem(NOCHANNELSTR, 99, 99, hTmpRoot);	// no channel

	errmsg = "";
	return errmsg;
}

::std::string TestClient::appendItem(CTreeCtrl& treectrl, HTREEITEM hChannel, ::std::string content, ::std::string& start, ::std::string& expiration,
									 BOOL playable, BOOL forceNormalSpeed, long inTimeOffset, long outTimeOffset, BOOL spliceIn, BOOL spliceOut)
{
	std::string errmsg = "unknown error";
	if(!_publisherPrx)
	{
		errmsg = "ChannelPublisher not open!";
		return errmsg;
	}
	if(!g_IsValidNode(treectrl, hChannel))
	{
		errmsg = "Invalid tree node selected!";	// no valid tree node, such as a "... no channels ..." node
		return errmsg;
	}

	CString name = treectrl.GetItemText(hChannel);

	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;

#ifdef USE_OLD_NS
	channelPrx = _publisherPrx->open((LPCSTR)name);
#else

	pubPrx = _publisherPrx->open((LPCSTR)name);
	channelPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS

//	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = _publisherPrx->open((LPCSTR)name);
	
	if(!channelPrx)
	{
		errmsg = "Open channel failed!";
		return errmsg;
	}

	CI_NS_PREFIX(ChannelItem) newItem;
	newItem.contentName = content;
	newItem.broadcastStart = start;
	newItem.expiration = expiration;
	newItem.forceNormalSpeed = forceNormalSpeed;
	newItem.inTimeOffset = inTimeOffset;
	newItem.outTimeOffset = outTimeOffset;
	newItem.playable = playable;
	newItem.spliceIn = spliceIn;
	newItem.spliceOut = spliceOut;

	try
	{
		channelPrx->appendItem(newItem);
	}
	catch (::TianShanIce::InvalidParameter& ex)
	{
		errmsg = "Replace item failed with " + ex.message;
		return errmsg;		
	}
	catch (::TianShanIce::ServerError& ex)
	{
		errmsg = "Replace item failed with " + ex.message;
		return errmsg;		
	}
	catch (Ice::Exception& ex)
	{
		errmsg = "Replace item failed with " + ex.ice_name();
		return errmsg;		
	}

	HTREEITEM hFirstCh = treectrl.GetChildItem(hChannel);
	if(!g_IsValidNode(treectrl, hFirstCh))
		treectrl.DeleteItem(hFirstCh);
	treectrl.InsertItem(newItem.contentName.c_str(), 2, 2, hChannel);

	treectrl.Expand(hChannel, TVE_EXPAND);

	errmsg = "";
	return errmsg;
}

::std::string TestClient::findItem(CTreeCtrl& treectrl, HTREEITEM hItem, ::std::string& content, ::std::string& start, ::std::string& expiration,
								   BOOL& playable, BOOL& forceNormalSpeed, long& inTimeOffset, long& outTimeOffset, BOOL& spliceIn, BOOL& spliceOut)
{
	std::string errmsg = "unknown error";
	if(!_publisherPrx)
	{
		errmsg = "ChannelPublisher not open!";
		return errmsg;
	}
	if(!g_IsValidNode(treectrl, hItem))
	{
		errmsg = "Invalid tree node selected!";	// no valid tree node, such as a "... no channels ..." node
		return errmsg;
	}
	
	HTREEITEM hChannel = treectrl.GetParentItem(hItem);
	CString name = treectrl.GetItemText(hChannel);

	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;

#ifdef USE_OLD_NS
	channelPrx = _publisherPrx->open((LPCSTR)name);
#else

	pubPrx = _publisherPrx->open((LPCSTR)name);
	channelPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS

//	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = _publisherPrx->open((LPCSTR)name);
	
	if(!channelPrx)
	{
		errmsg = "Open channel failed!";
		return errmsg;
	}
	
	::std::string contentName = (LPCSTR)treectrl.GetItemText(hItem);
	try
	{
		CI_NS_PREFIX(ChannelItem) item = channelPrx->findItem(contentName);
		content		= item.contentName;
		start		= item.broadcastStart;
		expiration	= item.expiration;
		playable = item.playable;
		forceNormalSpeed = item.forceNormalSpeed;
		inTimeOffset = item.inTimeOffset;
		outTimeOffset = item.outTimeOffset;
		spliceIn = item.spliceIn;
		spliceOut = item.spliceOut;
	}
	catch(::TianShanIce::InvalidStateOfArt& e)
	{
		errmsg = "Find item failed!";
		return errmsg;
	}
	catch(::Ice::Exception& e)
	{
		errmsg = std::string("Find item failed!") + e.ice_name() ;
		return errmsg;
	}
		
	errmsg = "";
	return errmsg;
}

::std::string TestClient::replaceItem(CTreeCtrl& treectrl, HTREEITEM hItem, ::std::string oldContent, ::std::string content, ::std::string& start, ::std::string& expiration,
									  BOOL playable, BOOL forceNormalSpeed, long inTimeOffset, long outTimeOffset, BOOL spliceIn, BOOL spliceOut)
{
	std::string errmsg = "unknown error";
	if(!_publisherPrx)
	{
		errmsg = "ChannelPublisher not open!";
		return errmsg;
	}
	if(!g_IsValidNode(treectrl, hItem))
	{
		errmsg = "Invalid tree node selected!";	// no valid tree node, such as a "... no channels ..." node
		return errmsg;
	}

	HTREEITEM hChannel = treectrl.GetParentItem(hItem);
	CString name = treectrl.GetItemText(hChannel);

	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;

#ifdef USE_OLD_NS
	channelPrx = _publisherPrx->open((LPCSTR)name);
#else
	pubPrx = _publisherPrx->open((LPCSTR)name);
	channelPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
//	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = _publisherPrx->open((LPCSTR)name);
	
	if(!channelPrx)
	{
		errmsg = "Open channel failed!";
		return errmsg;
	}
	
	CI_NS_PREFIX(ChannelItem) newItem;
	newItem.contentName = content;
	newItem.broadcastStart = start;
	newItem.expiration = expiration;
	newItem.forceNormalSpeed = forceNormalSpeed;
	newItem.inTimeOffset = inTimeOffset;
	newItem.outTimeOffset = outTimeOffset;
	newItem.playable = playable;
	newItem.spliceIn = spliceIn;
	newItem.spliceOut = spliceOut;
	
	try
	{
		channelPrx->replaceItem(oldContent, newItem);
	}
	catch (::TianShanIce::InvalidParameter& ex)
	{
		errmsg = "Replace item failed with " + ex.message;
		return errmsg;		
	}
	catch (::TianShanIce::ServerError& ex)
	{
		errmsg = "Replace item failed with " + ex.message;
		return errmsg;		
	}
	catch (Ice::Exception& ex)
	{
		errmsg = "Replace item failed with " + ex.ice_name();
		return errmsg;		
	}

	treectrl.SetItemText(hItem, content.c_str());
	
	errmsg = "";
	return errmsg;
}

::std::string TestClient::expireItem(CTreeCtrl& treectrl, HTREEITEM hItem)
{
	std::string errmsg = "unknown error";
	if(!_publisherPrx)
	{
		errmsg = "ChannelPublisher not open!";
		return errmsg;
	}
	if(!g_IsValidNode(treectrl, hItem))
	{
		errmsg = "Invalid tree node selected!";	// no valid tree node, such as a "... no channels ..." node
		return errmsg;
	}
	
	HTREEITEM hChannel = treectrl.GetParentItem(hItem);
	CString name = treectrl.GetItemText(hChannel);

	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = NULL;
	::TianShanIce::Application::PublishPointPrx pubPrx = NULL;

#ifdef USE_OLD_NS
	channelPrx = _publisherPrx->open((LPCSTR)name);
#else
	pubPrx = _publisherPrx->open((LPCSTR)name);
	channelPrx = ::TianShanIce::Application::ChannelOnDemand::ChannelPublishPointPrx::checkedCast(pubPrx);
#endif //USE_OLD_NS
//	NS_PREFIX(ChannelOnDemand::ChannelPublishPointPrx) channelPrx = _publisherPrx->open((LPCSTR)name);
	
	if(!channelPrx)
	{
		errmsg = "Open channel failed!";
		return errmsg;
	}
	
	CString content = treectrl.GetItemText(hItem);
	try
	{
		channelPrx->removeItem((LPCSTR)content);
	}
	catch (::TianShanIce::InvalidParameter& ex)
	{
		errmsg = "Replace item failed with " + ex.message;
		return errmsg;		
	}
	catch (::TianShanIce::ServerError& ex)
	{
		errmsg = "Replace item failed with " + ex.message;
		return errmsg;		
	}
	catch (Ice::Exception& ex)
	{
		errmsg = "Replace item failed with " + ex.ice_name();
		return errmsg;		
	}

//	{
//		errmsg = "Expire item failed!";
//		return errmsg;
//	}
	::std::string tmpName, tmpDesc;
	open(treectrl, hChannel, tmpName, tmpDesc);

	errmsg = "";
	return errmsg;
}
#define	CLIENT_SESSION_ID		"clientSessionId"
#define CHANNEL_ID				"channelId"
#define PROVIDER_ID				"providerId"
#define PROVIDER_ASSETID		"providerAssetId"
#define URL_SITE				"urlSite"
#define URL_PATH				"urlPath"		
#define CLIENT_IP				"clientIP"
#define CLIENT_PORT				"clientPort"
#define CSEQ					"CSeq"
#define TRANSPORT				"Transport"
#define USER_AGENT				"User-Agent"
#define SEACHANGE_VERSION		"SeaChange-Version"
#define SEACHANGE_MAYNOTIFY		"SeaChange-MayNotify"
#define SEACHANGE_SERVER_DATA	"SeaChange-Server-Data"
#define SEACHANGE_TRANSPORT		"SeaChange-Transport"
#define SEACHANGE_NOTICE		"SeaChange-Notice"

#define NODE_GROUP_ID			"node-group-id"
#define DRV_SMARTDCARD_ID		"smartcard-id"
#define HOME_ID					"home-id"
#define DRV_MAC_ADDRESS			"device-id"


void TestClient::createPurchase()
{
//	::TianShanIce::Streamer::StreamPrx sPrx;
//	::TianShanIce::Properties props;
//	::TianShanIce::Properties propsSite;
//	::ChannelOnDemand::ChannelPurchasePrx  purchasePrx;
//	TianShanIce::Weiwoo::SessionPrx   wsPrx;
//
//
//	props[CHANNEL_ID] = "CCTV2";
//	props[NODE_GROUP_ID] = 1;
//	props[CLIENT_SESSION_ID] = "001100110011";
//	props[HOME_ID] = "0";
//	props[DRV_SMARTDCARD_ID] = "0";
//
//	try
//	{
//		purchasePrx = _pAppPrx->createPurchaseByCR(props, propsSite);
//		purchasePrx->render(sPrx, wsPrx);
//	}
//	catch(::TianShanIce::InvalidParameter& ex)
//	{
//		std::string err = "caught InvalidParameter exception: %s";			
//	}
//	catch(::TianShanIce::InvalidStateOfArt& ex)
//	{
//		std::string err = "caught InvalidParameter exception: %s";
//	}
//	catch(::TianShanIce::ServerError& ex)
//	{
//		std::string err = "caught InvalidParameter exception: %s";
//	}
//	catch(::Ice::Exception& ex)
//	{
//		std::string err = "caught InvalidParameter exception: %s";
//	}
//	catch(...)
//	{
//		std::string err = "caught InvalidParameter exception: %s";
//	}
//
//
//	string errmsg;
//	::TianShanIce::StrValues strs;
//	::TianShanIce::ValueMap& vMap_in;
//	::TianShanIce::ValueMap& vMap_out;
//
//	{
//		try
//		{
//			purchasePrx->getParameters(strs, vMap_in, vMap_out);
//		}
//		catch(::Ice::ProxyParseException e)
//		{
//			errmsg = "There was an error while parsing the stringified proxy \"ChannelPublisher:" + endpoint + "\"!";
//			return errmsg;
//		}
//		catch(::Ice::NoEndpointException e)
//		{
//			errmsg = "Endpoint no found!";
//			return errmsg;
//		}
//		catch(::Ice::ObjectNotFoundException e)
//		{
//			errmsg = "Can not found object!";
//			return errmsg;
//		}
//		catch(::Ice::ObjectNotExistException e)
//		{
//			errmsg = "Can not found object!";
//			return errmsg;
//		}
//		catch (::Ice::Exception& e)
//		{
//			errmsg = "Can not found object: " + e.ice_name();
//			return errmsg;
//		}
//		catch(...)
//		{
//			errmsg = "Unknown exception got when parsing \"ChannelPublisher:" + endpoint + "\"!";
//			return errmsg;
//		}
//		
//
//		
//	}


}

bool g_IsValidNode(CTreeCtrl& treectrl, HTREEITEM hNode)
{
	int imageN, imageSelN;
	treectrl.GetItemImage(hNode, imageN, imageSelN);
	if(imageN==99 || imageSelN==99)
		return false;
	return true;
}

void test_getParameters(const char* endpoint)
{

	
}