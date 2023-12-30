      ///@mainpage SM Interface Module v1.0

///@section Description
///   SM Interface connect to SM, and recevie the command from SM, send status of playback
/// and some other request to SM. This module support connect failover. It will reconnect
/// the SM automatic when any network error occur.


#if !defined(AFX_SMCONNECTOR_H__EFC06C85_FD47_42B1_99BD_B42A601C7176__INCLUDED_)
#define AFX_SMCONNECTOR_H__EFC06C85_FD47_42B1_99BD_B42A601C7176__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000







#include "..\STVMainHeaders.h"
#include "Locks.h"
#include "socket.h"
#include "zqthread.h"
#include <queue>
#include "SMXmlProc.h"


class ScheduleTV;




/*!
 *	This class is the interface with other module.
 *  The Usage of this class:
 *	-# new or define a instance
 *	-# call SetParam to set params
 *	-# call start to start work, connect the sm, begin to work
 *	-# call close to close it
 *  -# can call isConnected, wait, and current class refrence
 */


class CSMConnector : 
	public ZQ::common::Socket, 
	public ZQ::comextra::Thread  
{
public:
		
	CSMConnector();
	
	virtual ~CSMConnector();

	/// set connection parameter
	///@param[in]	pSITV			pointer to SITV main control object
	///@param[in]	nClientID		Client ID
	///@param[in]	sBindIP			bind local ip address
	///@param[in]   sSMIP			the SM server ip address
	///@param[in]	nSMPort			the SM server port	
	void SetParam(ScheduleTV* pSITV, int nClientID, const char* sBindIP, const char* sSMIP, int nSMPort);


	//////////////////////////////////////////////////////////////////////////
	/// send status Feedback
	///@param[in]	sContent		status information content(<Message>'s sub node content)	
	void SendStatusFeedback(const char* sContent);




	//////////////////////////////////////////////////////////////////////////
	///  send Configuration enquire command
	void SendEnquireConfig();


	
	//////////////////////////////////////////////////////////////////////////
	///send enquire schedule command
	///@param[in]	sContent		Enquire information content(<Message>'s sub node content)
	void SendEnquireSchedule(const char* sContent);



	//////////////////////////////////////////////////////////////////////////
	/// send query filler list command
	///@param[in]	sContent		Enquire information content(<Message>'s sub node content)
	void SendQueryFillerList(const char* sContent);


	//////////////////////////////////////////////////////////////////////////
	/// shutdown thread and close the connection.	
	void close();

protected:

	friend class CSMXmlProc;

	//////////////////////////////////////////////////////////////////////////
	/// send data to SM
	///@param[in]	sBuf		data address to send
	///@param[in]	nLen		data length to send
	///@retval errSuccess if success, other for error	
	int SendData(const char* sBuf, int nLen);



	//////////////////////////////////////////////////////////////////////////
	/// send response to SM when after receve a command
	///@param[in]	nMsgID			source message code
	///@param[in]	bSuccess		if procee the command right
	///@param[in]	nErrorCode		error code when bSuccess is false, defaut 0 
	///@param[in]	sErrorString	error desc when bSuccess is false, defaut NULL
	///@remark		this is called inner this module
	void SendResponse(int nMsgID, bool bSuccess, int nErrorCode = 0, const char* sErrorString = NULL);



	/// send handshake message(send when just connected to SM)
	int SendHandShakeMsg();


	/// close the socket and create a new socket 
	void resetSocket();

	//////////////////////////////////////////////////////////////////////////
	/// bind local ip and connect to SM
	///@param[in]	sBindIP			local bind ip
	///@param[in]	sSMIP			ip of SM
	///@param[in]	nSMPort			port of the SM
	///@return errSuccess if success, other for else	
	int ConnectSM(const char* sBindIP, const char* sSMIP, int nSMPort);

	//////////////////////////////////////////////////////////////////////////
	/// read the specific number bytes data from socket, function return when 
	/// have read the specific number bytes of data or error occur or the value of 
	/// what bWorking point to have changed to false(means the app exit)
	///@param[out]	sBuf			buffer address that hold the returned data
	///@param[in]	nLen			data length that want to read
	///@param[in]	bWorking		point to a bool type value, by which we can stop this block operation
	///@return errSuccess if success, other for else	
	int RecvData(char* sBuf, int nLen, bool* bWorking);

	//////////////////////////////////////////////////////////////////////////
	///override, the thread procedure	
	virtual int run();



	struct SEND_DATA_MSG{ 
		char*		pData;
		int			nLen;
	};


	
	std::queue<SEND_DATA_MSG*>		_qSendData;		///< data send queue

	ZQ::common::Mutex				_mutexSendQueue;	///< protect the send queue


	
	
	
	char _sBindIP[128];		///< save the local bind ip
	
	char _sSMIP[128];		///< save the SM ip
	
	int  _nPort;			///< save the port of SM

	int	 _nLocalPort;		///< the port of local connect socket


	int  _nClientID;			///< save the Client ID


	ScheduleTV*		_pSTV;

	
	char _sXmlBuf[MAX_XML_BUF];		///< the xml string buffer, used when recevie data & send data




	CSMXmlProc xmlProc;			///< this object is to process the xml command 


	bool	_bWorking;			///< this variable is the flag to sign if we quit 
};

#endif // !defined(AFX_SMCONNECTOR_H__EFC06C85_FD47_42B1_99BD_B42A601C7176__INCLUDED_)
