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
// Ident : $Id: Noiser.cpp,v 1.1 2005-05-17 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : the implementation of Noiser class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/NoiseSim/Noiser.cpp $
// 
// 1     10-11-12 15:59 Admin
// Created.
// 
// 1     10-11-12 15:31 Admin
// Created.
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
// Noiser.cpp: implementation of the Filter class.
//
//////////////////////////////////////////////////////////////////////


/// @file "Noiser.cpp"
/// @brief the implementation file for Noiser class
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-5-19 9:52
/// @version 0.1.0
#include "Noiser.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Noiser::Noiser()
{
	m_bGen = true;
	m_bNoise = false;
	m_bInDist = false;
	m_dProbability = 0.0;
	m_iInterval = 100;
	m_ndtNoiseDistType = NDT_RANDOM;
	m_nuNoiseUnit = NU_BIT;
	m_dPkgCount = 0.0;
	m_dPkgNoiseCount = 0.0;
	m_dByteCount = 0.0;
	m_dByteNoiseCount = 0.0;
	m_dBitNoiseCount = 0.0;
	m_iSeed = (unsigned)time(NULL);
	srand(m_iSeed);
	setNoisePos(Random());
}

Noiser::~Noiser()
{

}

void Noiser::setNoisePos(int iNoisePos)
{
	m_iNoisePos = iNoisePos;
}

int Noiser::getNoisePos() const
{
	return m_iNoisePos;
}

int Noiser::Random(int iBase)
{
	return Random()%iBase;
}

int Noiser::Random()
{
	return rand();
}

bool Noiser::Noise(char *szBuff, int iLen)
{
	switch(m_nuNoiseUnit)
	{
	case NU_BIT:
		if(m_iNoisePos < 8*iLen)
		{
			m_bGen = true;
			if(m_bNoise)
				return NoiseBit(szBuff, iLen, m_iNoisePos);
			else
				return false;
		}
		else
		{
			m_iNoisePos -= 8*iLen;
			return false;
		}
		break;

	case NU_BYTE:
		if(m_iNoisePos < iLen)
		{
			m_bGen = true;
			if(m_bNoise)
				return NoiseByte(szBuff, iLen, m_iNoisePos);
			else
				return false;
		}
		else
		{
			m_iNoisePos -= iLen;
			return false;
		}
		break;

	case NU_PKG:
		if(m_iNoisePos == 0)
		{
			m_bGen = true;
			if(m_bNoise)
				return NoisePkg(szBuff, iLen);
			else
				return false;
		}
		else
		{
			--m_iNoisePos;
			return false;
		}
		break;

	default:
		return false;
		break;
	}

	return false;
}

bool Noiser::NoisePkg(char *szBuff, int iLen)
{
	int iRand;

	if(szBuff == 0x0 || iLen <= 0 )
		return false;

	iRand = Random(iLen);
	szBuff[iRand] = szBuff[iRand] ^ 0x1;

	++m_dPkgNoiseCount;
	
	return true;
}

// reverse the byte according iMask bit setted 1
// defalut to reverse the whole byte
bool Noiser::NoiseByte(char *szBuff, int iLen, int iPos, int iMask)
{
	if(szBuff == 0x0 || iLen <= 0 || iPos < 0 || iPos >= iLen || iMask < 0 )
		return false;

	szBuff[iPos] = szBuff[iPos] ^ iMask;

	++m_dByteNoiseCount;
	++m_dPkgNoiseCount;
	
	return true;
}

// reverse the iBit bit in the package
bool Noiser::NoiseBit(char *szBuff, int iLen, int iBit)
{
	int iByte, iBitOffset;

	if(szBuff == 0x0 || iLen <= 0 || iBit < 0 || iBit >= iLen*8)
		return false;

	iByte = iBit/8;
	iBitOffset = iBit%8;
	szBuff[iByte] = szBuff[iByte] ^ (0x1 << iBitOffset);

	++m_dBitNoiseCount;
	++m_dByteNoiseCount;
	++m_dPkgNoiseCount;

	return true;
}

NoiseDistType Noiser::getNoiseDistType() const
{
	return m_ndtNoiseDistType;
}

void Noiser::setNoiseDistType(NoiseDistType ndtType)
{
	m_ndtNoiseDistType = ndtType;
}

bool Noiser::doNoise(char *szBuff, int iLen)
{
	++m_dPkgCount;
	m_dByteCount += iLen;
	
	if(m_bGen)
	{
		m_bGen = false;
		m_bInDist = DistGen();
		setNoisePos(Random(m_iInterval));
	}

	if(m_bInDist)
	{
		m_bNoise = true;
	}
	else
	{
		m_bNoise = false;
	}

	return Noise(szBuff, iLen);
}

double Noiser::getProbability() const
{
	return m_dProbability;
}

void Noiser::setProbability(double dProbability)
{
	m_dProbability = dProbability;
}

int Noiser::getMask() const
{
	return m_iMask;
}

void Noiser::setMask(int iMask)
{
	m_iMask = iMask;
}

void Noiser::setNoiseUnit(NoiseUnit nuType)
{
	m_nuNoiseUnit = nuType;
}

NoiseUnit Noiser::getNoiseUnit() const
{
	return m_nuNoiseUnit;
}

bool Noiser::DistGen()
{
	switch(m_ndtNoiseDistType)
	{
	case NDT_RANDOM:
		return DistRandom();
		break;

	case NDT_POISSON:

	default:
		return false;
	}

	return true;
}

bool Noiser::DistRandom()
{
	int iRand;
	
	iRand = Random();

	if(iRand <= RAND_MAX * m_dProbability)
	{		
		return true;
	}
	else
		return false;
}

int Noiser::getInterval() const
{
	return m_iInterval;
}

void Noiser::setInterval(int iInterval)
{
	m_iInterval = iInterval > RAND_MAX ? RAND_MAX : iInterval;
	if(iInterval == 0)
		m_iInterval = 1;
}
