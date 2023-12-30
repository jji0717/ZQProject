
#ifndef _TAO_ROUTINE_
#define _TAO_ROUTINE_


#include "TAO/ORBSVCS/CosNamingC.h"
#include <TAO/ORBSVCS/CosNotifyCommC.h>
#include <TAO/ORBSVCS/CosNotifyChannelAdminS.h>
#include <TAO/ORBSVCS/CosNotifyChannelAdminC.h>


CosNaming::NamingContext_ptr getNamingContext(
		  CORBA::ORB_var& orb, 
		  char       * szErrMsg,
          const char  * szName, 
          const char  * szInitRef,
		  bool bBindIfNotFound = true);

bool setNameBinding(CORBA::ORB_var& orb,
		char *            szErrMsg,
         const char *       szName,
         CORBA::Object_ptr  objRef,
         const char *       szInitRef );

CORBA::Object_ptr getNameBinding(CORBA::ORB_var& orb, 
								 char      * szErrMsg,
                                  const char * szName,
                                  const char * szInitRef);

CosNotifyChannelAdmin::EventChannel_ptr getNotifyChannel(
			CORBA::ORB_var& orb, 
			 const char  *     szName, 
              bool              bCreateIfNotExist, 
              const char  *     szNameInitRef,
              const char  *     szNotificationInitRef );

bool pushStructuredEvent(CORBA::ORB_var& orb, 
				 CosNotification::StructuredEvent_var    & event,
                 CosNotifyChannelAdmin::StructuredProxyPushConsumer_var   & pushConsumer,
                 const char              * szNameInitRef,
                 const char              * szNotificationInitRef,
                 const char              * szEventChannelNsEntry );



#endif