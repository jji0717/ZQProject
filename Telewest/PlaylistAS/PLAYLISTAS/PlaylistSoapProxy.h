// PlaylistSoapProxy.h: interface for the PlaylistSoapProxy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYLISTSOAPPROXY_H__5070B5B9_D2A9_42F5_B21A_30D75C68B27B__INCLUDED_)
#define AFX_PLAYLISTSOAPPROXY_H__5070B5B9_D2A9_42F5_B21A_30D75C68B27B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdafx.h"
#include "IssApi.h"

#define ONSETUPCALLNAME		"getPlaylistDetailInformation"
#define ONPLAYCALLNAME		"playPlaylist"
#define ONTEARDOWNCALLNAME	"teardownPlaylist"

#define DEFAULT_TIMEOUT		10000

class PlaylistSoapProxy  
{
public:
	/// constructor
	///@param[in] par_WsdlFile		the file path of the WSDL file
	///@param[in] par_WsmlFile		the file path of the WSML file
	///@param[in] par_ServiceName	the service name of the SOAP web service
	///@param[in] par_Port			the port url of the SOAP web service
	///@param[in] par_Namespace		the namespace of the SOAP web service
	PlaylistSoapProxy(wchar_t* par_WsdlFile, wchar_t* par_WsmlFile, wchar_t* par_ServiceName, wchar_t* par_Port, wchar_t* par_Namespace);
	
	/// destructor
	virtual ~PlaylistSoapProxy();

public:
	// Initialize the proxy object.
	void Initialize();

	/// Soap Message Communication with WFES when stream was setup
	///@param[in] homeId		the home-id of the user
	///@param[in] deviceId		the set top device mac of the user
	///@param[in] playlistId	the user playlist id
	///@param[in] streamId		the stream id of this session
	///@return					all asset/element information
	AELIST	OnSetupSoapCall(char* homeId, char* deviceId, char* playlistId, char* streamId);

	/// Soap Message Communication with WFES when stream was played
	///@param[in] homeId		the home-id of the user
	///@param[in] deviceId		the set top device mac of the user
	///@param[in] playlistId	the user playlist id
	///@param[in] streamId		the stream id of this session
	///@return					reserved
	long	OnPlaySoapCall(char* homeId, char* deviceId, char* playlistId, char* streamId);
	
	/// Soap Message Communication with WFES when stream was down
	///@param[in] homeId		the home-id of the user
	///@param[in] deviceId		the set top device mac of the user
	///@param[in] playlistId	the user playlist id
	///@param[in] streamId		the stream id of this session
	///@param[in] errorCode		the teardown reason code
	///@return					reserved
	long	OnTeardownSoapCall(char* homeId, char* deviceId, char* playlistId, char* streamId, char* errorCode);

	/// free aelist block
	///@param[in] exAElist		the list to free
	void	releaseAEList(AELIST exAElist);
	
public:
	/// get timeout
	///@return		the timeout value, in milli-sec
	DWORD	getTimeout() { return _dwTimeout; }

	/// set timeout
	///@par_timeout	the timeout to set, in milli-sec
	void	setTimeout(DWORD par_timeout) { _dwTimeout = par_timeout; }

private:
	/// get the dispatch id of specified name
	///@param[in] lpName		the name to query
	///@return					the dispatch id
	DISPID GetIDOfName(LPOLESTR lpName);

private:
	/// com smart pointer to the MSSoapClient object
	ISoapClientPtr m_pSoapClient;

	/// if com object instanced
	bool m_bInstanced;

	static void RaiseError(LPOLESTR pMessage, HRESULT hr); 
	static void RaiseError(LPOLESTR pMessage, HRESULT hr, EXCEPINFO& excepinfo, DISPPARAMS& params, unsigned int uArgErr);

private:
	/// path of SOAP WSDL file
	wchar_t		_wszSoapWSDLFilePath[MAX_PATH];

	/// path of MSSOAP WSML file
	wchar_t		_wszSoapWSMLFilePath[MAX_PATH];

	/// SOAP web service name
	wchar_t		_wszSoapServiceName[MAX_PATH];

	/// SOAP web service port
	wchar_t		_wszSoapPort[MAX_PATH];

	/// SOAP web service namespace
	wchar_t		_wszSoapNamespace[MAX_PATH];

	/// timeout, in milli-second
	DWORD		_dwTimeout;
};

#endif // !defined(AFX_PLAYLISTSOAPPROXY_H__5070B5B9_D2A9_42F5_B21A_30D75C68B27B__INCLUDED_)
