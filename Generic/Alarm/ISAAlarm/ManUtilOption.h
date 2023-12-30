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
// Name  : ManUtilOption.h
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-3-15
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ManUtilOption.h $
// 
// 1     10-11-12 15:58 Admin
// Created.
// 
// 1     10-11-12 15:30 Admin
// Created.
// 
// 1     05-07-12 4:19p Daniel.wang
// 
// 1     5/14/05 2:57p Daniel.wang
// 
// 1     05-03-30 14:57 Daniel.wang
// ===========================================================================

#ifndef _ZQ_MANUTILOPTION_H_
#define _ZQ_MANUTILOPTION_H_

#pragma warning(disable:4786)
#include <vector>
#include <string>

namespace ZQ
{
	namespace common
	{
		struct ManUtilVar
		{
			int					type;
			std::string			key;
			std::string			value;
		};

		struct ManUtilColumn
		{
			std::string					name;
			int							type;
			int							width;
			std::vector<std::string>	rows;
		};

		class ManUtilBuilder
		{
		private:
			std::vector<ManUtilVar>		m_arrVars;
			std::vector<ManUtilColumn>	m_arrColumn;

		public:
			ManUtilBuilder();

			void AddVariable(const char* strKey, int nVarType, const char* strValue);
			void AddColumn(const char* strName, int nType, const std::vector<std::string>& arrRows);

			std::wstring BuildString(void);
		};
	}
}

#endif//_ZQ_MANUTILOPTION_H_
