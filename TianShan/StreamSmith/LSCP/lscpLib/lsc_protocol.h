////////////////////////////////////////////////////////////////////////////////
// Title:       lsc_protocol.h
//
// Author:      Christiaan Lutzer <clutzer@schange.com>
//
// Purpose:     LSC (Lightweight Stream Control) protocol connection.
//
// Copyright:   (C) 1997-2003 by
//                  
//              SeaChange International, Inc. 124 Acton Street
//              Maynard, Massachusetts 01754
//              United States of America
//------------------------------------------------------------------------------
// All Rights Reserved.  Unpublished rights reserved under the  copyright laws
// of the United States.
// 
// The software contained on this media is proprietary to and embodies the
// confidential technology of SeaChange International Inc.  Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from SeaChange International Inc.
// 
// This software is furnished under a license and may be used and copied only in
// accordance with the terms of such license and with the inclusion of the above
// copyright notice.  This software or any other copies thereof may not be
// provided or otherwise made available to any other person.  No title to and
// ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by SeaChange International Inc.
// 
// SeaChange assumes no responsibility for the use or reliability of its
// software on equipment which is not supplied by SeaChange.
// 
// RESTRICTED RIGHTS LEGEND Use, duplication, or disclosure by the U.S.
// Government is subject to restrictions as set forth in Subparagraph (c)(1)(ii)
// of DFARS 252.227-7013, or in FAR 52.227-19, as applicable.
//------------------------------------------------------------------------------
//! author  = "Christiaan Lutzer"
//! email   = "clutzer@schange.com"
//! package = "Protocol Library"
////////////////////////////////////////////////////////////////////////////////
#ifndef __LSC_PROTOCOL_H__
#define __LSC_PROTOCOL_H__

#include "ZQ_common_conf.h"
#ifdef ZQ_OS_LINUX
#include <arpa/inet.h>
#endif

typedef signed char			int8_t;
typedef unsigned char		uint8_t;

typedef signed short		int16_t;
typedef unsigned short		uint16_t;

typedef signed int			int32_t;
typedef unsigned int		uint32_t;


namespace lsc
{
    static const uint8_t            protocolId          = 0x01;
    static const uint8_t            rtpProtocolId       = 0x02;

    static const uint8_t            protocolVersion     = 0x01;
    static const uint8_t            rtpProtocolVersion  = 0x01;

    static const uint8_t            LSC_REPLY_FLAG      = 0x80;

    static const uint32_t           LSC_POSITION_NOW    = 0x80000000;
    static const uint32_t           LSC_POSITION_EOS    = 0x7FFFFFFF;
    static const uint32_t           LSC_POSITION_BOS    = 0x0;

    enum OperationCode
    {
        LSC_PAUSE                   = 0x01,
        LSC_PAUSE_REPLY             = LSC_PAUSE | LSC_REPLY_FLAG,
        
        LSC_RESUME                  = 0x02,
        LSC_RESUME_REPLY            = LSC_RESUME | LSC_REPLY_FLAG,

        LSC_STATUS                  = 0x03,
        LSC_STATUS_REPLY            = LSC_STATUS | LSC_REPLY_FLAG,

        LSC_RESET                   = 0x04,
        LSC_RESET_REPLY             = LSC_RESET | LSC_REPLY_FLAG,

        LSC_JUMP                    = 0x05,
        LSC_JUMP_REPLY              = LSC_JUMP | LSC_REPLY_FLAG,

        LSC_PLAY                    = 0x06,
        LSC_PLAY_REPLY              = LSC_PLAY | LSC_REPLY_FLAG,

        LSC_DONE                    = 0x40,
    };

    enum ResponseCode
    {
        LSC_OK                      = 0x00,
        LSC_BAD_REQUEST             = 0x10,
        LSC_BAD_STREAM              = 0x11,
        LSC_WRONG_STATE             = 0x12,
        LSC_UNKNOWN                 = 0x13,
        LSC_NO_PERMISSION           = 0x14,
        LSC_BAD_PARAM               = 0x15,
        LSC_NO_IMPLEMENT            = 0x16,
        LSC_NO_MEMORY               = 0x17,
        LSC_IMP_LIMIT               = 0x18,
        LSC_TRANSIENT               = 0x19,
        LSC_NO_RESOURCES            = 0x1a,
        LSC_SERVER_ERROR            = 0x20,
        LSC_SERVER_FAILURE          = 0x21,
        LSC_BAD_SCALE               = 0x30,
        LSC_BAD_START               = 0x31,
        LSC_BAD_STOP                = 0x32,
        LSC_MPEG_DELIVERY           = 0x40,

        LSC_PVR                     = 0x90,
        LSC_TRICKMODE_LIMIT         = 0x91,
        LSC_APP_SHUTDOWN            = 0x92,
    };

    enum ServerMode
    {
        LSC_O_MODE                  = 0x00, // Stream reset (initial state) or after RESET
        LSC_P_MODE                  = 0x01, // Stream PAUSED
        LSC_ST_MODE                 = 0x02, // Stream Search -> Transport
        LSC_T_MODE                  = 0x03, // Stream Transporting
        LSC_TP_MODE                 = 0x04, // Stream Transport -> Pause
        LSC_STP_MODE                = 0x05, // Stream Search -> Transport -> Pause
        LSC_PST_MODE                = 0x06, // Stream Paused -> Search -> Transport
        LSC_MODE_EOS                = 0x07, // Stream at END OF STREAM
    };

#ifdef ZQ_OS_MSWIN
#include <pshpack1.h>
#else
#pragma pack(push,1)
#endif

    //: Standard LSC message header: 8 bytes.
    //
    struct StandardHeader_t
    {
        uint8_t         version;
        uint8_t         transactionId;
        uint8_t         opCode;
        uint8_t         statusCode;
        uint32_t        streamHandle;

//        inline StandardHeader_t()
//            : version(protocolVersion), transactionId(0), opCode(0),
//            statusCode(0), streamHandle(0)
//        {
//		}

        inline StandardHeader_t & ntoh()
        {
            streamHandle = ntohl(streamHandle);
            return *this;
        }

        inline StandardHeader_t & hton()
        {
            streamHandle = htonl(streamHandle);
            return *this;
        }

        inline size_t CalculateMessageLength() const;
    };

    //: LSC Play data: 12 bytes.
    //
    struct PlayData_t
    {
        uint32_t        startNpt;
        uint32_t        stopNpt;
        int16_t         numerator;
        uint16_t        denominator;

//        inline PlayData_t()
//            : startNpt(0), stopNpt(0), numerator(0), denominator(0)
//        {}

        inline PlayData_t & ntoh()
        {
            startNpt	= ntohl(startNpt);
            stopNpt		= ntohl(stopNpt);
            numerator	= ntohs(numerator);
            denominator	= ntohs(denominator);
            return *this;
        }

        inline PlayData_t & hton()
        {
            startNpt	= htonl(startNpt);
            stopNpt		= htonl(stopNpt);
            numerator	= htons(numerator);
            denominator = htons(denominator);
            return *this;
        }
    };

    //: LSC Play message: 8 + 12 = 20 bytes.
    //
    struct PlayMessage_t
    {
        lsc::StandardHeader_t header;
        lsc::PlayData_t	data;

//        inline PlayMessage_t()
//        {}

        inline PlayMessage_t & ntoh()
        {
            header.ntoh();
            data.ntoh();
            return *this;
        }

        inline PlayMessage_t & hton()
        {
            header.hton();
            data.hton();
            return *this;
        }
    };

    typedef PlayMessage_t JumpMessage_t;

    //: LSC Pause message body: 8 + 4 = 12 bytes.
    //
    struct PauseMessage_t
    {
        lsc::StandardHeader_t	header;
        uint32_t				stopNpt;

//        inline PauseMessage_t()
//            : stopNpt(0)
//        {}

        inline PauseMessage_t & ntoh()
        {
            header.ntoh();
            stopNpt = ntohl(stopNpt);
            return *this;
        }

        inline PauseMessage_t & hton()
        {
            header.hton();
            stopNpt = htonl(stopNpt);
            return *this;
        }
    };

    //: LSC Resume message body: 8 + 8 = 16 bytes.
    //
    struct ResumeMessage_t
    {
        lsc::StandardHeader_t header;
        uint32_t        startNpt;
        int16_t         numerator;
        uint16_t        denominator;

//        inline ResumeMessage_t()
//            : startNpt(0), numerator(0), denominator(0)
//        {}

        inline ResumeMessage_t & ntoh()
        {
            header.ntoh();
            startNpt	= ntohl(startNpt);
            numerator	= ntohs(numerator);
            denominator = ntohs(denominator);
            return *this;
        }

        inline ResumeMessage_t & hton()
        {
            header.hton();
            startNpt	= htonl(startNpt);
            numerator	= htons(numerator);
            denominator = htons(denominator);
            return *this;
        }
    };

    //: LSC Reset message: 8 + 0 = 8 bytes
    //
    struct ResetMessage_t
    {
        lsc::StandardHeader_t header;
    };

    //: LSC Status message: 8 + 0 = 8 bytes
    //
    struct StatusMessage_t
    {
        lsc::StandardHeader_t header;
    };

    //: LSC Response data: 9 bytes.
    //
    struct ResponseData_t
    {
        uint32_t        currentNpt;
        int16_t         numerator;
        uint16_t        denominator;
        uint8_t         mode;

//        inline ResponseData_t()
//            : currentNpt(0), numerator(0), denominator(0), mode(0)
//        {}

        inline ResponseData_t & ntoh()
        {
            currentNpt	= ntohl(currentNpt);
            numerator	= ntohs(numerator);
            denominator = ntohs(denominator);
            return *this;
        }

        inline ResponseData_t & hton()
        {
            currentNpt	= htonl(currentNpt);
            numerator	= htons(numerator);
            denominator = htons(denominator);
            return *this;
        }
    };

    //: LSC Response message: 8 + 9 = 17 bytes.
    //
    struct ResponseMessage_t
    {
        lsc::StandardHeader_t header;
        lsc::ResponseData_t data;

//        inline ResponseMessage_t()
//        {}

        inline ResponseMessage_t & ntoh()
        {
            header.ntoh();
            data.ntoh();
            return *this;
        }

        inline ResponseMessage_t & hton()
        {
            header.hton();
            data.hton();
            return *this;
        }
    };

    inline size_t StandardHeader_t::CalculateMessageLength() const
    {
        switch (opCode)
        {
        case LSC_PAUSE:     return sizeof(lsc::PauseMessage_t);
        case LSC_RESUME:    return sizeof(lsc::ResumeMessage_t);
        case LSC_STATUS:    return sizeof(lsc::StatusMessage_t);
        case LSC_RESET:     return sizeof(lsc::ResetMessage_t);
        case LSC_JUMP:      return sizeof(lsc::JumpMessage_t);
        case LSC_PLAY:      return sizeof(lsc::PlayMessage_t);
        case LSC_DONE:
        case LSC_PAUSE_REPLY:
        case LSC_RESUME_REPLY:
        case LSC_STATUS_REPLY:
        case LSC_RESET_REPLY:
        case LSC_JUMP_REPLY:
        case LSC_PLAY_REPLY:return sizeof(lsc::ResponseMessage_t);
        }
        return 0;
    }
	
	typedef union
	{
		PauseMessage_t		pause;
		ResumeMessage_t		resume;
		StatusMessage_t		status;
		ResetMessage_t		reset;
		JumpMessage_t		jump;
		PlayMessage_t		play;
		ResponseMessage_t response;		
	}LSCMESSAGE;

#ifdef ZQ_OS_MSWIN
#include <poppack.h>
#else
#pragma pack(pop)
#endif

}


//------------------------------------------------------------------------------
#endif
