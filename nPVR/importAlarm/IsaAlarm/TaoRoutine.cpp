

#include "TaoRoutine.h"
#include <string>
#include "Log.h"


#ifdef _DEBUG
#pragma comment(lib, "TAO_Messagingd.lib")
#pragma comment(lib, "TAO_CosNotificationd.lib")
#pragma comment(lib, "TAO_Strategiesd.lib")
#pragma comment(lib, "TAO_PortableServerd.lib")
#pragma comment(lib, "TAO_CosNamingd.lib")
#pragma comment(lib, "TAO_CosNotificationd.lib")
#pragma comment(lib, "TAO_CosEventd.lib")
#pragma comment(lib, "TAOd.lib")
#pragma comment(lib, "aced.lib")
#else
#pragma comment(lib, "TAO_Messaging.lib")
#pragma comment(lib, "TAO_CosNotification.lib")
#pragma comment(lib, "TAO_Strategies.lib")
#pragma comment(lib, "TAO_PortableServer.lib")
#pragma comment(lib, "TAO_CosNaming.lib")
#pragma comment(lib, "TAO_CosNotification.lib")
#pragma comment(lib, "TAO_CosEvent.lib")
#pragma comment(lib, "TAO.lib")
#pragma comment(lib, "ace.lib")

#endif

using namespace ZQ::common;
using namespace std;


std::string getNameFromPath(const char * szName)
{
    std::string strName = szName;
    unsigned long pos = strName.find_last_of('/');
    if (pos == std::string::npos) 
        return strName;

    // else : one or more levels of context exists
    return strName.substr(pos + 1, strName.length() - pos - 1);
}

bool setNameBinding(CORBA::ORB_var& orb,
					char *            szErrMsg,
                     const char *       szName,
                     CORBA::Object_ptr  objRef,
                     const char *       szInitRef )
{
    CosNaming::NamingContext_var ctx = getNamingContext(orb, szErrMsg, szName, 
                                                        szInitRef);
    if (CORBA::is_nil(ctx))
    {
        return false;
    }

    string strName = getNameFromPath(szName);

    // Bind the name in the last level of naming context
    //
    try{
        CosNaming::Name objName;
        objName.length(1);

        unsigned long dotPos = strName.find_last_of('.');
        if (dotPos == string::npos) // no 'kind'
        {
            objName[0].id = CORBA::string_dup(strName.c_str());
        }
        else
        {
            objName[0].id = CORBA::string_dup(strName.substr(0, dotPos).c_str());
            objName[0].kind = CORBA::string_dup(
                    strName.substr(dotPos+1, strName.length()-dotPos-1).c_str());
        }

        ctx -> rebind(objName, objRef);
    }
    catch(CosNaming::NamingContext::NotFound&) {
        sprintf( szErrMsg, "Caught CosNaming::NamingContext::NotFound in rebind() - %s - %s", 
                  szName, szInitRef );
        return false;
    }
    catch(CosNaming::NamingContext::CannotProceed&)  {
        sprintf( szErrMsg, "Caught CosNaming::NamingContext::CannotProceed in resolve() - %s - %s", 
                  szName, szInitRef );
        return false;
    }
    catch(CosNaming::NamingContext::InvalidName&)   {
        sprintf( szErrMsg, "Caught CosNaming::NamingContext::InvalidName in resolve() - %s - %s", 
                  szName, szInitRef );
        return false;
    }
    catch(const CORBA::Exception & ex) {
        sprintf (szErrMsg, "Caught %s in CosNaming::NamingContext::rebind() - %s - %s",
                  ex._rep_id(), szName, szInitRef );

        return false;
    }

    return true;
}

CosNaming::NamingContext_ptr getNamingContext(CORBA::ORB_var& orb, 
											  char       * szErrMsg,
                                              const char  * szName, 
                                              const char  * szInitRef,
											  bool bBindIfNotFound)
{
    CORBA::Object_var            refObj;
    CosNaming::NamingContext_var ctx;

    // Step 0. Try to resolve NamingService
    //
    try
	{
        refObj = orb -> resolve_initial_references(szInitRef);
    } 
	catch (CORBA::ORB::InvalidName &) 
	{
        sprintf( szErrMsg, "Exception CORBA::ORB::InvalidName in resolve_initial_references() - %s - %s",
                  szName, szInitRef );
        return CosNaming::NamingContext::_nil();
    }

    if ( CORBA::is_nil(refObj) ) 
    {
        sprintf( szErrMsg, "Initial referece is NULL - %s", szInitRef );
        return CosNaming::NamingContext::_nil();
    }
    
    try 
	{
        ctx = CosNaming::NamingContext::_unchecked_narrow(refObj);
    }
	catch (const CORBA::Exception & ex) 
	{        
        sprintf( szErrMsg, "Caught %s in CosNaming::NamingContext::_unchecked_narrow() - %s",
                  ex._rep_id(), szInitRef );
        return CosNaming::NamingContext::_nil();
    }
    catch (...) 
	{
        sprintf( szErrMsg, "CosNaming::NamingContext::_unchecked_narrow() - %s", szInitRef );
        return CosNaming::NamingContext::_nil();
    }
    
    if( CORBA::is_nil(ctx) ) 
    {
        sprintf( szErrMsg, "NamingContext of initial reference is NULL - %s", szInitRef);
        return CosNaming::NamingContext::_nil();
    }

    // Step 1. Go though the contexts, if there is any
    //
    string strName = szName;
    unsigned long pos = strName.find_first_of('/');
    while (pos != string::npos) // one or more levels of context exists
    {
        CosNaming::Name ctxName;
        ctxName.length(1);
        string strCtx = strName.substr(0, pos);
        unsigned long dotPos = strCtx.find_last_of('.');
        if (dotPos == string::npos) // no 'kind'
        {
            ctxName[0].id = CORBA::string_dup(strCtx.c_str());
        }
        else
        {
            ctxName[0].id = CORBA::string_dup(strCtx.substr(0, dotPos).c_str());
            ctxName[0].kind = CORBA::string_dup(
                    strCtx.substr(dotPos+1, strCtx.length()-dotPos-1).c_str());
        }

        try{
            refObj  = ctx -> resolve(ctxName);
        }
        catch (CosNaming::NamingContext::NotFound & )
        {
			if (bBindIfNotFound)
            {
                try 
				{
                    ctx -> bind_new_context(ctxName);
                    refObj = ctx -> resolve(ctxName);
                }
				catch (const CORBA::Exception& ex) 
				{
					sprintf( szErrMsg, "Caught %s in bind_new_context() / resolve() - %s - %s", 
						ex._rep_id(), szName, szInitRef );
                    return CosNaming::NamingContext::_nil();
                }
            }
            else
            {
				sprintf( szErrMsg, "CosNaming::NamingContext::NotFound in resolve() - %s - %s", 
					szName, szInitRef );
                return CosNaming::NamingContext::_nil();
            }			
        }
        catch (const CORBA::Exception & ex)
        {
            sprintf( szErrMsg, "Caught %s in CosNaming::NamingContext::resolve() - %s - %s",
                      ex._rep_id(), szName, szInitRef );
            return CosNaming::NamingContext::_nil();
        }

        try {
            ctx = CosNaming::NamingContext::_unchecked_narrow(refObj);
        } catch (const CORBA::Exception & ex)
        {
            sprintf( szErrMsg, "Caught %s in CosNaming::NamingContext::_unchecked_narrow() - %s - %s", 
                      ex._rep_id(), szName, szInitRef );
            return CosNaming::NamingContext::_nil();
        }

        if( CORBA::is_nil(ctx) ) 
        {
            sprintf( szErrMsg, "NamingContext is NULL - %s - %s", szName, szInitRef);
            return CosNaming::NamingContext::_nil();
        }

        strName = strName.substr(pos + 1, strName.length() - pos - 1);
        pos = strName.find_first_of('/');
    }

    CosNaming::NamingContext::_duplicate(ctx);
    return ctx;
}

CORBA::Object_ptr getNameBinding(CORBA::ORB_var& orb, 
								 char      * szErrMsg,
                                  const char * szName,
                                  const char * szInitRef )
{
    CosNaming::NamingContext_var ctx = getNamingContext(orb, szErrMsg, szName, szInitRef);
    if (CORBA::is_nil(ctx))
    {		
        return CORBA::Object::_nil();
    }

    CORBA::Object_var   refObj;

    string strName = getNameFromPath(szName);

    // Resolve the name in the last level of naming context
    //
    try
	{
        CosNaming::Name objName;
        objName.length(1);
//        objName[0].id = CORBA::string_dup(strName.c_str());
        unsigned long dotPos = strName.find_last_of('.');
        if (dotPos == string::npos) // no 'kind'
        {
            objName[0].id = CORBA::string_dup(strName.c_str());
        }
        else
        {
            objName[0].id = CORBA::string_dup(strName.substr(0, dotPos).c_str());
            objName[0].kind = CORBA::string_dup(
                    strName.substr(dotPos+1, strName.length()-dotPos-1).c_str());
        }

        refObj = ctx -> resolve(objName);
    }
    catch(CosNaming::NamingContext::NotFound&) {
        sprintf ( szErrMsg,
                   "CosNaming::NamingContext::NotFound in resolve() - %s - %s", 
                   szName, szInitRef );
        return CORBA::Object::_nil();
    }
    catch(CosNaming::NamingContext::CannotProceed&)  {
        sprintf ( szErrMsg, 
                   "CosNaming::NamingContext::CannotProceed in resolve() - %s - %s", 
                   szName, szInitRef );
        return CORBA::Object::_nil();
    }
    catch(CosNaming::NamingContext::InvalidName&)   {
        sprintf ( szErrMsg, 
                   "CosNaming::NamingContext::InvalidName in resolve() - %s - %s", 
                   szName, szInitRef );
        return CORBA::Object::_nil();
    }
    catch(const CORBA::Exception &) {
        sprintf ( szErrMsg, 
                   "Caught CORBA::Exception in resolve() - %s - %s", 
                   szName, szInitRef );
        return CORBA::Object::_nil();
    }

    return CORBA::Object::_duplicate(refObj);
}

///////////////////////////////////////////////////////////////////////////////
// Helper function that find the requested event channel in NameService. 
// If no such binding exists in the NameService, and bCreateIfNotExist is true,
// a event channel will be created and registered 
//
CosNotifyChannelAdmin::EventChannel_ptr getNotifyChannel(
			CORBA::ORB_var& orb, 
			 const char  *     szName, 
              bool              bCreateIfNotExist, 
              const char  *     szNameInitRef,
              const char  *     szNotificationInitRef )
{
    CosNotifyChannelAdmin::EventChannel_var nsChannel;
    char  szErrMsg [256];
	szErrMsg[0]= '\0';

    // try to resolve event channel from NameServerice
    CORBA::Object_var obj;
    try 
	{
        obj = getNameBinding(orb, szErrMsg, szName, szNameInitRef);
    } 
	catch (...)
    {
		glog(Log::L_ERROR, szErrMsg);
        return CosNotifyChannelAdmin::EventChannel::_nil(); 
    }

    // resolved from nameservice 
	if (! CORBA::is_nil(obj) ) 
    {
        try 
        {
            nsChannel = CosNotifyChannelAdmin::EventChannel::_unchecked_narrow(obj);
            return CosNotifyChannelAdmin::EventChannel::_duplicate(nsChannel); 
        }
        catch (CORBA::OBJECT_NOT_EXIST &)
        {
            glog(Log::L_ERROR, "The EventChannel registered in %s no longer exists in %s",
                     szNameInitRef, szNotificationInitRef );
        }
        catch (const CORBA::Exception & ) 
        {
            glog(Log::L_ERROR, " in CosNotifyChannelAdmin::EventChannel::_unchecked_narrow() - %s - %s",
                               szName, szNameInitRef );
            return CosNotifyChannelAdmin::EventChannel::_nil(); 
        }
    }

    // ==> The event channel is not in namesevice 
    if (!bCreateIfNotExist) 
    {
		glog(Log::L_ERROR, szErrMsg);
        return CosNotifyChannelAdmin::EventChannel::_nil(); 
    }

    // ==> Can't find name in NameService && bCreateIfNotExist == true
    // Now create the requested event channel

    // first, resolve the Notification Service initial reference 
    try 
	{
        obj = getNameBinding(orb, szErrMsg, szNotificationInitRef, szNameInitRef);
    }
	catch (...) // (CORBA::ORB::InvalidName &) 
    {
        glog(Log::L_ERROR, " in resolve_initial_references() - %s", szNotificationInitRef );
        return CosNotifyChannelAdmin::EventChannel::_nil(); 
    }

    //next, resolve the Event Channel Factory
    CosNotifyChannelAdmin::EventChannelFactory_var eventChannelFactory;
    try 
	{
        eventChannelFactory = CosNotifyChannelAdmin::EventChannelFactory::_unchecked_narrow(obj);
    }
    catch (const CORBA::Exception & ) 
	{
        glog(Log::L_ERROR, " in CosNotifyChannelAdmin::EventChannelFactory::_unchecked_narrow() - %s - %s",
                           szName, szNotificationInitRef );
        return CosNotifyChannelAdmin::EventChannel::_nil(); 
    }

    if (CORBA::is_nil(eventChannelFactory))
    {
        glog(Log::L_ERROR, "NotificationFactory Object is NULL - %s", szNotificationInitRef );
        return CosNotifyChannelAdmin::EventChannel::_nil(); 
    }

    // create the new channel
    CosNotification::QoSProperties initialQoS; 
    CosNotification::AdminProperties initialAdmin; 
    CosNotifyChannelAdmin::ChannelID channelId; 
    try
    {
        nsChannel = eventChannelFactory->create_channel(initialQoS,
                                                        initialAdmin,
                                                        channelId);
        // Register the newly created channel in name service.
        char szErrMsg [256];
        if (! setNameBinding(orb, szErrMsg, szName, nsChannel, szNameInitRef))
        {
			glog(Log::L_ERROR, szErrMsg);
            return CosNotifyChannelAdmin::EventChannel::_nil(); 
        }

        CORBA::String_var strRef_channel = orb -> object_to_string(nsChannel);                
        glog(Log::L_DEBUG, "New event channel registered as name : \"%s\"\n%s",
                szName, (const char*)strRef_channel );

        return CosNotifyChannelAdmin::EventChannel::_duplicate(nsChannel);
    }
    catch(const CORBA::Exception & ) // CosNotification::UnsupportedQoS, CosNotification::UnsupportedAdmin
    {
        glog(Log::L_ERROR, " in CosNotifyChannelAdmin::EventChannelFactory::create_channel() - %s - %s",
                           szName, szNotificationInitRef);
    }

    return (CosNotifyChannelAdmin::EventChannel::_nil());
}

bool pushStructuredEvent(CORBA::ORB_var& orb, 
				 CosNotification::StructuredEvent_var    & event,
                 CosNotifyChannelAdmin::StructuredProxyPushConsumer_var   & pushConsumer,
                 const char              * szNameInitRef,
                 const char              * szNotificationInitRef,
                 const char              * szEventChannelNsEntry )
{
    if (CORBA::is_nil(pushConsumer))
    {
        CosNotifyChannelAdmin::EventChannel_var nsChannel =
            getNotifyChannel(orb, szEventChannelNsEntry, true, szNameInitRef, szNotificationInitRef);

        if ( CORBA::is_nil(nsChannel) )
		{
			glog(Log::L_ERROR, L"Fail to getNotifyChannel(), NameInitRef[%S] NotifycationInitRef[%S] EventChannelName[%S]",
				szNameInitRef, szNotificationInitRef, szEventChannelNsEntry);
			return false;
		}
				
		// Obtain and set the StructuredProxyPushConsumer on the factory
		CosNotifyChannelAdmin::SupplierAdmin_var sa_var;
		try
		{
			sa_var = nsChannel->default_supplier_admin();
		}
		catch(CORBA::SystemException& ex)
		{
			glog(Log::L_ERROR, L"Fail to call default_supplier_admin(), info: %S", ex._info().c_str());
			return false;
		}
		
		CosNotifyChannelAdmin::ProxyID proxyId;
		CosNotifyChannelAdmin::ProxyConsumer_var proxyConsumer;
		try
		{
			proxyConsumer = sa_var -> obtain_notification_push_consumer(
			CosNotifyChannelAdmin::STRUCTURED_EVENT, proxyId);
		}
		catch(CosNotifyChannelAdmin::AdminLimitExceeded&)
		{
			glog(Log::L_ERROR, L"Fail to call obtain_notification_push_consumer(), info: AdminLimitExceeded");
			return false;
		}
		catch(CORBA::SystemException& ex)
		{
			glog(Log::L_ERROR, L"Fail to call obtain_notification_push_consumer(), info: %S", ex._info().c_str());
			return false;
		}

		pushConsumer = CosNotifyChannelAdmin::StructuredProxyPushConsumer::_unchecked_narrow(proxyConsumer);
		
		try
		{
			pushConsumer -> connect_structured_push_supplier(CosNotifyComm::StructuredPushSupplier::_nil());
		}
		catch(CORBA::SystemException& ex)
		{
			glog(Log::L_ERROR, L"Fail to call connect_structured_push_supplier(), info: %S", ex._info().c_str());
			return false;
		}
		glog(Log::L_DEBUG, L"NotifyChannel [%S] connected", szEventChannelNsEntry);
    }

    try    
	{
        pushConsumer -> push_structured_event(event);
    }
	catch(const CosEventComm::Disconnected&)
    {
        glog(Log::L_ERROR, L"Caught exception CosEventComm::Disconnected during push_structured_event()");
		return false;
    }

	return true;
}