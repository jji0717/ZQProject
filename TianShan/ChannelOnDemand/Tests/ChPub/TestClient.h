// TestClient.h: interface for the TestClient class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTCLIENT_H__FA3EE9A9_C19A_4B89_8D76_42202DE00282__INCLUDED_)
#define AFX_TESTCLIENT_H__FA3EE9A9_C19A_4B89_8D76_42202DE00282__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define USE_OLD_NS  // The switch to use the old namespace

#ifdef USE_OLD_NS
#  include "ChannelOnDemand.h"
#  define  NS_PREFIX(_CLS) _CLS
#  define  CI_NS_PREFIX(_CLS) ChannelOnDemand::_CLS
#else
#  include "TsAppChOD.h"
#  define  NS_PREFIX(_CLS) TianShanIce::Application::_CLS
#  define  CI_NS_PREFIX(_CLS) TianShanIce::Application::_CLS
#endif // USE_OLD_NS

#include <string>
#include <Ice/Ice.h>
#include "ChannelOnDemandEx.h"
#include "TsStreamer.h"

bool g_IsValidNode(CTreeCtrl& treectrl, HTREEITEM hNode);

class TestClient  
{
public:
	TestClient();
	virtual ~TestClient();

	::std::string connect(::std::string endpoint);
	::std::string disconnect();
	::std::string list(CTreeCtrl& treectrl, HTREEITEM hRoot);
	::std::string publish(CTreeCtrl& treectrl, HTREEITEM hRoot, ::std::string name, ::std::string desc);
	::std::string open(CTreeCtrl& treectrl, HTREEITEM hChannel, ::std::string& name, ::std::string& desc);
	::std::string destroy(CTreeCtrl& treectrl, HTREEITEM hChannel);
	
	::std::string appendItem(CTreeCtrl& treectrl, HTREEITEM hChannel, ::std::string content, ::std::string& start, ::std::string& expiration, BOOL playable, BOOL forceNormalSpeed, long inTimeOffset, long outTimeOffset, BOOL spliceIn, BOOL spliceOut);
	::std::string findItem(CTreeCtrl& treectrl, HTREEITEM hItem, ::std::string& content, ::std::string& start, ::std::string& expiration,BOOL& playable, BOOL& forceNormalSpeed, long& inTimeOffset, long& outTimeOffset, BOOL& spliceIn, BOOL& spliceOut);
	::std::string replaceItem(CTreeCtrl& treectrl, HTREEITEM hItem, ::std::string oldContent, ::std::string content, ::std::string& start, ::std::string& expiration, BOOL playable, BOOL forceNormalSpeed, long inTimeOffset, long outTimeOffset, BOOL spliceIn, BOOL spliceOut);
	::std::string expireItem(CTreeCtrl& treectrl, HTREEITEM hItem);
	
	void createPurchase();

private:
	bool					_isConnected;
	::std::string			_endpoint;
	::Ice::CommunicatorPtr	_ic;
	NS_PREFIX(ChannelOnDemand::ChannelPublisherExPrx)	_publisherPrx;
	NS_PREFIX(ChannelOnDemand::ChannelOnDemandAppPrx)  _pAppPrx;
	::TianShanIce::Streamer::StreamServicePrx  _pStreamSPrx;
};

#endif // !defined(AFX_TESTCLIENT_H__FA3EE9A9_C19A_4B89_8D76_42202DE00282__INCLUDED_)
