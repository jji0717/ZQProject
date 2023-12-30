// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Name  : PlaylistAppServ.h
// Author : Bernie Zhao (bernie.zhao@i-zq.com  Tianbin Zhao)
// Date  : 2005-1-31
// Desc  : class wrapped the PlaylistAS service
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Telewest/PlaylistAS_gSoap/PlaylistAppServ.h $
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 1     05-06-28 11:37 Bernie.zhao
// 
// 2     05-03-24 16:50 Bernie.zhao
// added soap call when acceptrdyasset failed
// 
// 1     05-02-16 11:56 Bernie.zhao
// ===========================================================================

///@mainpage Telewest: Playlist Application Server Document
///@section bk_sec Background
///@n 	Music Video Playback is an enhancement component for NG VOD Telewest product.  It uses same controls as Video Playback and added new features based on the introduction of Playlist Engine (PLE). The PLE
///@n	i.	Support the subscriber list all users defined and 3rd party defined playlist and their content 
///@n	ii.	Support the subscriber maintain the user defined playlist (create / delete / update)
///@n	iii.	Support various special feature while playing the playlist, sequential play, shuffle, next, skip, repeat, etc.
///@n 	There are three (3) components in the Music Video Playback. The first one is a Web Front End System(WFES) which will be enhanced to include the Music Playlists Actions to support STB (SetTopBox) operations. 
///@n	Secondly, there will be a Music Playlists Application (PlaylistAS) in the ITV system handling the Music Playlists submitted by STB. The application is tightly integrated with the WFES to provide required features and functions. The application is to handle assets playback and necessary logic. 
///@n	The last one is STB. The STB client application keeps and maintains playlist sequence and current play position and status. The STB application will response to client's requests, handle Music Playlist logic and send corresponding command to WFES and ITV system.
///@n 	The PlaylistAS component is actually a service which co-works with WFES as well as ICM and ISRM services in ITV system.  It provides extra stream resource allocation/modification and stream playback logic rules.
///@section pu_sec Purpose
///@n	The module PlaylistAS, as an extensive function module to ITV system for Music Video Playback, is to provide user personal or 3rd party playlist streaming service.  It communicates with WFES thru SOAP to access playlist information, necessary authorization, and handles assets streaming.

#if !defined(AFX_PLAYLISTAPPSERV_H__E86AF3BE_B48D_4770_854F_8115F8F79B62__INCLUDED_)
#define AFX_PLAYLISTAPPSERV_H__E86AF3BE_B48D_4770_854F_8115F8F79B62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "baseschangeserviceapplication.h"
#include "StreamSession.h"

using namespace ZQ::common;

class PlaylistAppServ : public BaseSchangeServiceApplication  
{
public:
	PlaylistAppServ();
	~PlaylistAppServ();

	//////////////////////////////////////////////////////////////////////////
	// service control operations	

public:
	// reserve for SeaChange Service
	HRESULT OnInit(void);
	HRESULT OnUnInit(void);

	HRESULT	OnStart(void);
	HRESULT OnStop(void);

	//////////////////////////////////////////////////////////////////////////
	// all component pointers
private:
	CStreamSession* _pTheSSMan;

	//////////////////////////////////////////////////////////////////////////
	// service attributes
private:
	

	///////////////////////////////////// statistic attributes for manutil /////////////////////////////////////
	// all the member variable which is started with "m_" must be static variable, so that the manutil can monitor the process

public:
	/// service status
	static bool			m_bRunning;

	/// service version
	static wchar_t		m_wszVerInfo[64];

	/// ISS application UID
	static DWORD		m_dwAppUID;

	/// ISS callback Type for PlaylistAS
	static DWORD		m_dwType;

	/// ISS callback instance id
	static DWORD		m_dwInst;

	/// SOAP timeout
	static DWORD		m_dwSoapTimeout;

	/// path of SOAP WSDL file
	static wchar_t		m_wszSoapWSDLFilePath[MAX_PATH];

	/// path of MSSOAP WSML file
	static wchar_t		m_wszSoapWSMLFilePath[MAX_PATH];

	/// SOAP web service name
	static wchar_t		m_wszSoapServiceName[MAX_PATH];

	/// SOAP web service port
	static wchar_t		m_wszSoapPort[MAX_PATH];

	/// SOAP web service namespace
	static wchar_t		m_wszSoapNamespace[MAX_PATH];

};

#endif // !defined(AFX_PLAYLISTAPPSERV_H__E86AF3BE_B48D_4770_854F_8115F8F79B62__INCLUDED_)
