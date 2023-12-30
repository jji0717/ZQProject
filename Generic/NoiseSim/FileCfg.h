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
// Ident : $Id: FileCfg.h,v 1.1 2005-05-17 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : define FileCfg class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/NoiseSim/FileCfg.h $
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
//////////////////////////////////////////////////////////////////////

// FileCfg.h: interface for the FileCfg class.
//
//////////////////////////////////////////////////////////////////////


/// @file "FileCfg.h"
/// @brief the header file for FileCfg class
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-5-19 9:52
/// @version 0.1.0

/// @example "NoiseSim.cfg"
/// A example config file of NoiseSim
#if !defined(AFX_FILECFG_H__35FB1590_7686_47ED_90AA_4326F3D7EB1F__INCLUDED_)
#define AFX_FILECFG_H__35FB1590_7686_47ED_90AA_4326F3D7EB1F__INCLUDED_


#include <string>
#include <fstream>
#include <map>

using namespace std;

/// @class FileCfg "FileCfg.h"
/// @brief Config file class
///
/// Use this class, you can get the key/value pairs from a config file.
///
/// The format of each line is:
/// Key=Value
///
/// There should not have white space befor, after and among this line.
///
/// The line begin with "#" is a comment
///
/// The null line will be ignored
class FileCfg  
{
public:
	/// Open a config file
	/// @param[in] strFileName the config file name
	/// @return if succeed, return true, otherwise return false
	bool Open(string strFileName);
	
	/// Whether end of file
	bool isEof();
	
	/// Whether the file opened
	bool isOpen();
	
	/// Close config file
	bool Close();
	
	/// Open a config file
	bool Open();
	
	/// Get a line from config file
	string getCfgLine() const;
	
	/// Get value from key/value pairs line
	string getValue() const;
	
	/// Get key from key/value pairs line
	string getKey() const;
	
	/// Get current config file name
	string getFileName() const;
	
	/// Get the delimiter character
	char getDelim() const;

	/// Set the delimiter character
	/// @param char chDelim[in] the delimiter character
	void setDelim(char chDelim = '=');
	
	/// Get a line from config file, ignor comment line and null line
	/// @return if the line is a key/value pairs line, return ture. otherwise return false.
	bool GetLine();
	
	/// Constructor
	FileCfg(string strFileName);
	
	/// Constructor
	FileCfg();
	
	/// Destructor
	virtual ~FileCfg();

private:
	string m_strKey;		///< To reserve the key of key/value pairs
	string m_strValue;		///< To reserve the value of key/value pairs
	string m_strCfgLine;		///< To reserve the current line of config file
	char m_chDelim;			///< Delimiter character
	string m_strFileName;		///< Config file name
	ifstream m_ifsFile;		///< Input file stream
};

#endif // !defined(AFX_FILECFG_H__35FB1590_7686_47ED_90AA_4326F3D7EB1F__INCLUDED_)
