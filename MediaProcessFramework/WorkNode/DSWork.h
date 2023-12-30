// ===========================================================================
// Copyright (c) 2005 by
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
// Ident : $Id: DSWork.h,v 1.0 2005/05/08 16:34:35 Gu Exp $
// Branch: $Name:  $
// Author: Hongye Gu
// Desc  : define task creation APIs
//
// Revision History: 
// ---------------------------------------------------------------------------
// DSWork.h: interface for the DSWork class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __DSWORK_H__
#define __DSWORK_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseWork.h"

#define DEFAULT_DS_WAITTIME		5000

MPF_WORKNODE_NAMESPACE_BEGIN

class TaskAcceptor;

///@brief DSWork is a derived class from BaseWork type, which provides filter container \n
/// and message propagation for DirectShow interface. \n
/// User could derived from DSWork class to implement specified DirectShwo work
class DLL_PORT  DSWork : public BaseWork  
{
protected:
	/// constructor
	///@param[in]	factory			-the pointer of WorkFactory class object which gives birth to it
	///@param[in]	TaskTypename	-the task type string of this work
	///@param[in]	sessionURL		-the session URL string which contains the manager node information who requests this work
	DSWork(WorkFactory* factory, const char* TaskTypename= "DSWork", const char* sessionURL=NULL);

	/// destructor
	virtual ~DSWork();

public:

	///force to stop work
	void				Stop();
	

	///release graph builder created by GetGraphBuilder
	void				ReleaseGraphBuilder();


	///create graph builder
	///@return	the pointer to GraphBuilder object
	void	*			GetGraphBuilder();


	///this function is pure virtual,overwrited by app to use to process event
	///sended by the lower filter
	///Please refer to MS DirectX 'IMediaEventEx' interface for details
	///@param[in]		evtCode		-variable that contains the event code. 
	///@param[in]		Param1		-variable that contains the first event parameter
	///@param[in]		Param2		-variable that contains the second event parameter
	virtual void		OnNotifyEvent(long evtCode,long Param1,long Param2)=0;

protected:
	/// get the progress information
	///@param[out]		attrs		-variable that receives the formatted progress information
	///@return						-True if success, False else
	///@remarks						-the format is like	\n
	/// ZQ.MPF.Attr						\n
	///     |- ZQ.MPF.BeginPos			\n
	///     |- ZQ.MPF.CurrentPos		\n
	///     |- ZQ.MPF.EndPos			\n
	virtual bool		OnGetProgress(ZQ::rpc::RpcValue& attrs);
	

	int					run();

private:

	/// pointer to GraphBuilder object
	void	*	m_pGraphBuilder;

	/// pointer to MediaEvent object
	void	*	m_pMediaEvent;

	/// pointer to MediaSeeking object
	void	*	m_pMediaSeeking;

	/// pointer to MediaControl object
	void	*	m_pMediaControl;

	/// event handler for exit the thread
	HANDLE		m_evThreadExit;

};

MPF_WORKNODE_NAMESPACE_END

#endif
