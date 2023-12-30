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
// Ident : $Id: Noiser.h,v 1.1 2005-05-17 15:30:26 Ouyang Exp $
// Branch: $Name:  $
// Author: Lin Ouyang
// Desc  : define Noiser class
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/NoiseSim/Noiser.h $
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
//////////////////////////////////////////////////////////////////////


/// @file "Noiser.h"
/// @brief the header file for Noiser class
/// @author (Lorenzo) Lin Ouyang
/// @date 2005-5-19 9:52
/// @version 0.1.0
// Noiser.h: interface for the Noiser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NOISER_H__954846CE_792D_416B_B560_0BC56A8D9917__INCLUDED_)
#define AFX_NOISER_H__954846CE_792D_416B_B560_0BC56A8D9917__INCLUDED_


#include <cstdlib>
#include <ctime>

/// Noise distribut type
typedef enum enumNoiseDistType 
{
		NDT_RANDOM,	///< average distribute
		NDT_POISSON,	///< poisson distribute, not supported now
		NDT_NOMORE	// end flag
} NoiseDistType;

/// Noise unit
typedef enum enumNoiseUnit
{
		NU_BIT,		///< noise one bit per time
		NU_BYTE,	///< noise one byte per time
		NU_PKG,		///< noise one package per time
		NU_NOMORE	// end flag
} NoiseUnit;

/// @class Noiser "Noiser.h"
/// @brief Noiser class
///
/// Generate noise to simulate the noisy network
class Noiser  
{
public:
	bool m_bGen;				///< Whether generate a random number for distribution function
	bool m_bInDist;				///< Whether the number in the distribution
	bool m_bNoise;				///< Whether to noise the message.
	
	/// @brief Noise distribution type
	///
	/// only support average distribution currently
	NoiseDistType m_ndtNoiseDistType;
	/// @brief Noise unit
	///
	/// support bit/byte/pkg unit
	NoiseUnit m_nuNoiseUnit;
	double m_dPkgCount;			///< Number of total message packages
	double m_dPkgNoiseCount;		///< Number of noised message packages
	double m_dByteCount;			///< Number of total bytes
	double m_dByteNoiseCount;		///< Number of Noised bytes
	double m_dBitNoiseCount;		///< Number of total bit
	double m_dProbability;			///< The distribution probability
	int m_iInterval;			///< The interval value of two noised unit
	int m_iSeed;				///< Seed for random

public:
	/// Set interval value of two noised unit
	void setInterval(int iInterval);
	
	/// Get interval value of two noised unit
	int getInterval() const;
	
	/// Average distribution
	///
	/// generate a random number inner, judge whether the number is in average distribution
	/// @return if in average distribution, return true, otherwise return false
	bool DistRandom();
	
	/// Generate distribution
	///
	/// according noise distribution type, call relevant distribution function.
	///
	/// for example, the m_ndtNoiseDistType is NDT_RANDOM, 
	/// it will call function DistRandom(),
	/// and return the value of DistRandom()
	/// @return if in relevant distribution, return true, otherwise return false
	bool DistGen();
	
	/// Get noise unit
	NoiseUnit getNoiseUnit() const;
	
	/// Set noise unit
	void setNoiseUnit(NoiseUnit nuType);
	
	/// @brief Set byte mask
	///
	/// used in @ref NU_BYTE unit mode
	void setMask(int iMask);
	
	/// Get current mask value
	int getMask() const;
	
	/// Set distribution probability
	void setProbability(double dProbability);
	
	/// Get current distribution probability
	double getProbability() const;
	
	/// running noise
	bool doNoise(char *szBuff, int iLen);
	
	/// Set noise distribution type
	/// @param[in] ndtType noise distribution type
	void setNoiseDistType(NoiseDistType ndtType);
	
	/// Get noise distribution type
	NoiseDistType getNoiseDistType() const;
	
	/// @brief Noise a package
	///
	/// if @ref m_bNoise is true, noise the package
	/// @param[in,out] szBuff message
	/// @param[in] iLen length of szBuff
	/// @return if the package is noised, return true, otherwise return false
	bool NoisePkg(char *szBuff, int iLen);
	
	/// @brief Noise a byte
	///
	/// if @ref m_bNoise is true, noise a byte according @ref m_iNoisePos
	/// @param[in,out] szBuff message
	/// @param[in] iLen length of szBuff
	/// @param[in] iPos the byte position will be noised
	/// @param[in] iMask bit mask of noise, 
	/// the bit setted 1 will be noised, the bit setted 0 will not be noised.
	/// default value is 0xff, means the whole byte will be noised
	/// @return if a byte is noised, return true, otherwise return false
	bool NoiseByte(char *szBuff, int iLen, int iPos, int iMask = 0xff);
	
	/// @brief Noise a bit
	///
	/// if @ref m_bNoise is true, noise a bit according @ref m_iNoisePos
	/// @param[in,out] szBuff message
	/// @param[in] iLen length of szBuff
	/// @param[in] iBit the position of bit in a byte will be noised
	/// @return if a bit is noised, return true, otherwise return false
	bool NoiseBit(char *szBuff, int iLen, int iBit);
	
	/// Get noise position
	int getNoisePos() const;
	
	/// @brief Generate a random number
	/// @return return the random number
	int Random();
	
	/// @brief Generate a random number between 0 to iBase
	/// @return return the random number
	int Random(int iBase);
	
	/// @brief Noise message
	///
	/// noise a message according @ref m_nuNoiseUnit
	/// @param[in,out] szBuff message
	/// @param[in] iLen length of szBuff
	/// @return if a bit is noised, return true, otherwise return false
	bool Noise(char *szBuff, int iLen);
	
	/// Constructor
	Noiser();
	
	/// Destructor
	virtual ~Noiser();
	
	/// Set noise position
	void setNoisePos(int iNoisePos);

protected:
	int m_iNoisePos;	///< Noise position
	int m_iMask;		///< Noise bit mask in @ref NU_BYTE unit
};


#endif // !defined(AFX_NOISER_H__954846CE_792D_416B_B560_0BC56A8D9917__INCLUDED_)
