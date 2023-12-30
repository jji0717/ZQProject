 // SMXmlProc.h: interface for the CSMXmlProc class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMXMLPROC_H__E9687A74_1FFC_4639_A474_5BFD436C5ED2__INCLUDED_)
#define AFX_SMXMLPROC_H__E9687A74_1FFC_4639_A474_5BFD436C5ED2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "xmlpreference.h"


class ScheduleTV;
class CSMConnector;
#include "Log.h"
using namespace ZQ::common;
/**
 *    This class is for processing xml request from SM and make xml response toSM, 
 *  and make some xml request to SM
 */
class CSMXmlProc  
{
public:
	
	CSMXmlProc();
	
	virtual ~CSMXmlProc();


	/// set the main control object pointer, so this process can call the main control's mothed
	///@param[in]	pSITV		SITV object pointer
	void SetMainControl(ScheduleTV* pSITV)
	{
		_pSITV = pSITV;
	}

	
	/// set the Connection object, so the process can call that to send reponse to SM when need
	void SetConnectionObject(CSMConnector* pConnector)
	{
		_pConnector = pConnector;
	}

	/// call com initialize, since the coinitialize need in the same thread with com calling
	void CoInit()
	{
		if (!init)
			init = new ZQ::common::ComInitializer;
	}


	/// call com uninitialize
	void CoUnInit()
	{
		if (init)
		{
			delete init;
			init = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// process the xml command
	///@param[in]	sXml			the input xml string.	
	void XmlProc(const char* sXml);



	/// make the xml status string by the status information content string
	/// this function is to add the stuats xml with "<Message ...>...</Message>" tag
	///@param[in]	sContent		the status string, in xml format
	///@param[out]	sOutputXml		the buf to hold the returned status string
	///@param[out]	nOutput			the address to hold the status string length
	void XmlStatusFeedback(		
		const char* sContent, 
		char* sOutputXml,
		int* nOutput);


	
	///make enquire schedule xml string
	///@param[in]	sContent			informaton content
	///@param[out]	sOutputXml			buffer to hold xml
	///@param[out]	nOutput				length of sOutputXml string
	void XmlEnquireSchedule(
		const char* sContent, 
		char* sOutputXml,
		int* nOutput);



	///make the response xml string
	///@param[out]	sXml			buffer to hold the returned xml
	///@param[in]	nMsgID			source message code
	///@param[in]	bSuccess		if procee the command right
	///@param[in]	nErrorCode		error code when bSuccess is false, defaut 0 
	///@param[in]	sErrorString	error desc when bSuccess is false, defaut NULL
	int XmlResponse(
		char* sXml, 
		int nMsgID,
		bool bSuccess, 
		int nErrorCode = 0,
		const char* sErrorString = NULL);


	/// make the handshake xml string
	///@param[out]	sXml			buffer to hold the returned xml
	///@param[in]	nClientID		Client ID
	int XmlHandShakeMsg(char* sXml, int nClientID);



	//////////////////////////////////////////////////////////////////////////
	/// make query filler list xml string
	///@param[in]	sContent			informaton content
	///@param[out]	sOutputXml			buffer to hold xml
	///@param[out]	nOutput				length of sOutputXml string
	void XmlQueryFillerList(
		const char* sContent, 
		char* sOutputXml,
		int* nOutput);


	/// make the enquire configuration xml string
	///@param[out]	sXml			buffer to hold the returned xml
	///@param[in]	nClientID		Client ID
	int XmlEnquireConfig(char* sXml, int nClientID);


protected:

	/*! since the coinitialize need in the same thread with calling  */
	ZQ::common::ComInitializer* init;		///< initialize COM object

	ScheduleTV*				_pSITV;				///< main control pointer

	CSMConnector*		_pConnector;		///< connection object pointer

	DWORD	_dwNo;		///< Generated No identify each message , from 1-100000
};

#endif // !defined(AFX_SMXMLPROC_H__E9687A74_1FFC_4639_A474_5BFD436C5ED2__INCLUDED_)
