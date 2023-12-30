
#ifndef _EXPT_HANDLE_H_

#define _EXPT_HANDLE_H_

#define TIANSHANICE_INVALID_PARAMETER_HANDLE	catch(const ::TianShanIce::InvalidParameter& ex)\
	{\
		myGlog(ZQ::common::Log.L_ERROR,"		(%s:%s)Catch an %s:%s",retSessionID.c_str(),sMethod.c_str(),ex.ice_name().c_str(),ex.message.c_str());\
		sprintf(gBuff,"%s: %s",ex.ice_name().c_str(),ex.message.c_str());\
		ErrorResponse(pResponse,RESPONSE_BAD_REQUEST,sMethod.c_str(),gBuff,retSessionID.c_str());\
		return false;\
	}

#define TIANSHANICE_SERVER_ERROR_HANDLE catch(const ::TianShanIce::ServerError& ex)\
	{\
		myGlog(ZQ::common::Log.L_ERROR,"		(%s:%s)Catch an %s:%s",retSessionID.c_str(),sMethod.c_str(),ex.ice_name().c_str(),ex.message.c_str());\
		sprintf(gBuff,"%s: %s",ex.ice_name().c_str(),ex.message.c_str());\
		ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());\
		return false;\
	}

#define TIANSHANICE_INVALID_STATE_OF_ART_HANDLE catch(const ::TianShanIce::InvalidStateOfArt& ex)\
	{\
		myGlog(ZQ::common::Log.L_ERROR,"		(%s:%s)Catch an %s:%s",retSessionID.c_str(),sMethod.c_str(),ex.ice_name().c_str(),ex.message.c_str());\
		sprintf(gBuff,"%s: %s",ex.ice_name().c_str(),ex.message.c_str());\
		ErrorResponse(pResponse,RESPONSE_BAD_REQUEST,sMethod.c_str(),gBuff,retSessionID.c_str());\
		return false;\
	}

#define TIANSHANICE_BASE_EXCEPTION_HANDLE catch(const ::TianShanIce::BaseException& ex)\
	{\
		myGlog(ZQ::common::Log.L_ERROR,"		(%s:%s)Catch an %s:%s",retSessionID.c_str(),sMethod.c_str(),ex.ice_name().c_str(),ex.message.c_str());\
		sprintf(gBuff,"%s: %s",ex.ice_name().c_str(),ex.message.c_str());\
		ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());\
		return false;\
	}

#define ICE_PROXY_PARSE_EXCEPTION_HANDLE catch(const Ice::ProxyParseException& ex)\
	{\
		myGlog(ZQ::common::Log.L_ERROR,"		(%s:%s)Catch an %s",retSessionID.c_str(),sMethod.c_str(),ex.ice_name().c_str());\
		sprintf(gBuff,"%s",ex.ice_name().c_str());\
		ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());\
		return false;\
	}

#define ICE_EXCEPTION_HANDLE catch(const ::Ice::Exception& ex)\
	{\
		myGlog(ZQ::common::Log::L_ERROR,"		(%s:%s)Catch an %s",retSessionID.c_str(),sMethod.c_str(),ex.ice_name().c_str());\
		sprintf(gBuff,"%s",ex.ice_name().c_str());\
		ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());\
		return false;\
	}
#define ALL_EXCEPTION_HANDLE	catch(...)\
	{\
		myGlog(ZQ::common::Log.L_ERROR,"		(%s:%s)Catch an unknown exception",retSessionID.c_str(),sMethod.c_str());\
		sprintf(gBuff,"Catch an unknown exception");\
		ErrorResponse(pResponse,RESPONSE_INTERNAL_ERROR,sMethod.c_str(),gBuff,retSessionID.c_str());\
		return false;\
	}

#endif // _EXPT_HANDLE_H_