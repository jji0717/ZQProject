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
// Name  : ManUtilOption.cpp
// Author: XiaoBai (daniel.wang@i-zq.com  Wang YuanOu)
// Date  : 2005-3-15
// Desc  : 
//
// Revision History:
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/Alarm/ISAAlarm/ManUtilOption.cpp $
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

#include "manutiloption.h"

#define MAN_LINE_MAX 2046

namespace ZQ
{
	namespace common
	{
		ManUtilBuilder::ManUtilBuilder()
		{
			m_arrVars.clear();
			m_arrColumn.clear();
		}

		void ManUtilBuilder::AddVariable(const char* strKey, int nVarType, const char* strValue)
		{
			ManUtilVar muv;
			muv.type		= nVarType;
			muv.key			= strKey;
			muv.value		= strValue;

			m_arrVars.push_back(muv);
		}

		void ManUtilBuilder::AddColumn(const char* strName, int nType, const std::vector<std::string>& arrRows)
		{
			ManUtilColumn muc;
			muc.name		= strName;
			muc.type		= nType;
			muc.rows		= arrRows;

			m_arrColumn.push_back(muc);
		}

		std::wstring ManUtilBuilder::BuildString(void)
		{
			std::string strRtn;

			char strTemp[MAN_LINE_MAX] = {0};

			sprintf(strTemp, "%d\t%d\n", m_arrVars.size(), m_arrColumn.size());
			strRtn += strTemp;

			for (int i = 0; i < m_arrVars.size(); ++i)
			{
				memset(strTemp, 0, MAN_LINE_MAX);
				sprintf(strTemp, "%d\t%.1000s\t%.1000s\n", m_arrVars[i].type,
					m_arrVars[i].key.c_str(), m_arrVars[i].value.c_str());
				strRtn += strTemp;
			}

			for (int j = 0; j < m_arrColumn.size(); ++j)
			{
				memset(strTemp, 0, MAN_LINE_MAX);
				sprintf(strTemp, "%d\t%d\t%d\t%.1000s\n", m_arrColumn[j].type,
					m_arrColumn[j].width, m_arrColumn[j].rows.size(), m_arrColumn[j].name.c_str());
				strRtn += strTemp;

				for (int k = 0; k < m_arrColumn[j].rows.size(); ++k)
				{
					memset(strTemp, 0, MAN_LINE_MAX);
					sprintf(strTemp, "%s\n", m_arrColumn[j].rows[k].c_str());
					strRtn += strTemp;
				}
			}

			strRtn += "\0\0";

			std::wstring strWideRtn;
			strWideRtn.resize(strRtn.size());
			std::copy(strRtn.begin(), strRtn.end(), strWideRtn.begin());

			return strWideRtn;
		}
	}
}
