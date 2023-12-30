// ===========================================================================
// Copyright (c) 1997, 1998 by
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
// Ident : $Id: FileCfg.cpp,v 1.1 2005-05-17 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : the implementation of FileCfg class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/NoiseSim/FileCfg.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
// 
// 3     05-07-08 13:36 Lin.ouyang
// 
// 2     05-05-19 12:09 Lin.ouyang
// by: lorenzo(lin ouyang)
// add comment for doxgen
// 
// 1     05-05-17 15:48 Lin.ouyang
// init version
// 
// Revision 1.1  2005-05-17 15:30:26  Ouyang
// initial created
// ===========================================================================
//


/// @file "FileCfg.cpp"
/// @brief the implementation file for FileCfg class
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-5-19 9:52
/// @version 0.1.0

//////////////////////////////////////////////////////////////////////
// FileCfg.cpp: implementation of the FileCfg class.
//
//////////////////////////////////////////////////////////////////////

#include "FileCfg.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileCfg::FileCfg()
{
	m_chDelim = '=';
}

FileCfg::~FileCfg()
{
	if(isOpen())
		Close();
}

FileCfg::FileCfg(string strFileName)
{
	m_strFileName = strFileName;
	m_chDelim = '=';
}

// get a line which include the delimiter(m_chDelim)
// if there is no m_chDelim in this line or eof, return false
// otherwise return true
bool FileCfg::GetLine()
{
	char szTemp[1024];
	string::size_type idx;

	// eof, return false
	if(isEof())
		return false;

	m_ifsFile.getline(szTemp, sizeof(szTemp));
	m_strCfgLine = szTemp;

	// the line is a comment, return false
	idx = m_strCfgLine.find('#');
	if(idx == 0)
		return false;

	idx = m_strCfgLine.find(m_chDelim);
	// no delimiter in this line, return false
	if(string::npos == idx)
		return false;
	
	m_strKey = m_strCfgLine.substr(0, idx);
	m_strValue = m_strCfgLine.substr(idx+1);

	if(m_strKey.empty() || m_strValue.empty())
		return false;
	
	return true;
}

char FileCfg::getDelim() const
{
	return m_chDelim;
}

void FileCfg::setDelim(char chDelim)
{
	m_chDelim = chDelim;
}

string FileCfg::getFileName() const
{
	return m_strFileName;
}

string FileCfg::getKey() const
{
	return m_strKey;
}

string FileCfg::getValue() const
{
	return m_strValue;
}

string FileCfg::getCfgLine() const
{
	return m_strCfgLine;
}

bool FileCfg::Open()
{
	m_ifsFile.open(m_strFileName.c_str());
	return isOpen();
}

bool FileCfg::Close()
{
	m_ifsFile.close();
	return !isOpen();
}

bool FileCfg::isOpen()
{
	if(m_ifsFile.is_open())
		return true;
	else
		return false;
}

bool FileCfg::isEof()
{
	return m_ifsFile.eof();
}

bool FileCfg::Open(string strFileName)
{
	m_strFileName = strFileName;
	return Open();
}
