// PlaylistSoapProxy.h: interface for the PlaylistSoapProxy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLAYLISTSOAPPROXY_H__5070B5B9_D2A9_42F5_B21A_30D75C68B27B__INCLUDED_)
#define AFX_PLAYLISTSOAPPROXY_H__5070B5B9_D2A9_42F5_B21A_30D75C68B27B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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
	PlaylistSoapProxy(wchar_t* par_WsdlFile);
	
	/// destructor
	virtual ~PlaylistSoapProxy();

public:
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
	/// log error
	///@param[in] soap			the soap instance
	void	logSoapError(struct soap *soap);

private:
	/// path of SOAP WSDL file
	wchar_t		_wszSoapWSDLFilePath[MAX_PATH];

	/// timeout, in milli-second
	DWORD		_dwTimeout;
};

#endif // !defined(AFX_PLAYLISTSOAPPROXY_H__5070B5B9_D2A9_42F5_B21A_30D75C68B27B__INCLUDED_)
