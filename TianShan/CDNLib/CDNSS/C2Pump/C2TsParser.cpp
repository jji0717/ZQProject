#include "C2TsParser.h"
#include <iostream>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
//#include <unistd.h>
#include <assert.h>

#define PACKET_LENTH 188
#define PAT_PID 0x0000
#define PACKET_HEADER 4
#define DEBUG

namespace C2Streamer
{
    TsParser::TsParser(C2StreamerEnv& env)
        :_nPackageCount(0)
        ,_bPATinit(false)
        ,_bPMTinit(false)
        ,_nFrameLen(0)
        ,_nRestFrameLen(0)
        ,_nFramePacketCount(0)
        ,_bFrameLenZero(false)
        ,_ePacketType(PACKET_ELSE)
        ,_bFinished(false)
        ,_bFindStart(false)
        ,_bIsIFrame(false)
    {
        memset(&_ProgramInfo, 0, sizeof(TS_PAT_Program));
        memset(&_PATInfo, 0, sizeof(TS_PAT));
        memset(&_PMTStream, 0, sizeof(Ts_PMT_Stream));
    }

    TsParser::~TsParser(void)
    {
    }

    bool TsParser::nextFrame(FrameStat& framestat, unsigned char* buf, int buflen, bool bIsIFrame, bool validateEnd/* = false*/)
    {
        framestat.reset();
        if (buflen < PACKET_LENTH)
        {
            //长度不足一个包
            //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"The lenth of stream is %d,is shorter than 188\n"),buflen);
            return false;
        }

        int pos = 0;
        while (buf && pos < buflen)
        {
            //包头首字节全部是固定的0x47
            while (buf[pos] != 0x47 && pos < buflen)
            {
                pos ++;
            }

            if (buflen - pos < PACKET_LENTH)
            {
                //长度不足一个包
                //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"The lenth of stream is %d,is shorter than 188\n"),buflen);
                return false;
            }

            if (buf[pos+PACKET_LENTH] != 0x47) 
            {
                //找到的0x47不是包头
                pos++;
                continue;
            }

            int errorCount = 0;
            bool bSuccess = true;
            bool bFindedS = false;
            while (bSuccess && buflen - pos >= PACKET_LENTH) //parse another TS _PATInfo
            {
                if (errorCount == 3)
                {
                    //MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(TsParser,"errorcount = 3, so quit"));
                    return false;
                }

                unsigned char* posbuff = buf + pos;
                if (pos == 1651768)
                {
                    int xxxx = 0;
                }
                bool bSuccess = parseTsPackage(posbuff, pos);
                
                if (_ePacketType == PACKET_PES)
                {
                    if (bFindedS && _bFinished)
                    {
                        if (bIsIFrame)
                        {
                            if (_bIsIFrame)
                            {
                                framestat.endPos = pos;
                                if (_nFrameLen == 0)
                                {
                                    framestat.endPos -= PACKET_LENTH;
                                    _bFrameLenZero = false;
                                }
                                else
                                    pos += PACKET_LENTH;
                                return true;
                            }
                            
                            if (_nFrameLen == 0)
                            {
                                pos -= PACKET_LENTH;
                            }
                        }
                        else
                        {
                            framestat.endPos = pos;
                            if (_nFrameLen == 0)
                            {
                                framestat.endPos -= PACKET_LENTH;
                                _bFrameLenZero = false;
                            }
                            else
                                pos += PACKET_LENTH;
                            return true;
                        }
                        resetStatus();
                        bFindedS = false;
                    }

                    if (_bFindStart)
                    {
                        bFindedS = _bFindStart;
                        if (bIsIFrame)
                        {
                            if (_bIsIFrame)
                            {
                                framestat.startPos = pos;
                                if (!validateEnd)
                                {
                                    pos += PACKET_LENTH;
                                    return true;
                                }
                            }
                        }
                        else
                        {
                            framestat.startPos = pos;
                            if (!validateEnd)
                            {
                                pos += PACKET_LENTH;
                                return true;
                            }
                        }
                        _bFindStart = false;
                    }
                }
                pos += PACKET_LENTH;
            }
            framestat.reset();
            resetStatus();
        }
        return false;
    }

    bool TsParser::parseTsHeader( TS_packet_header& packetHeader, unsigned char* buf )
    {
        packetHeader.transport_error_indicator = buf[1]>>7;
        packetHeader.payload_unit_start_indicator = buf[1]>>6 & 0x01;
        packetHeader.transport_priority = ((buf[1]>>5) & 0x01);
        packetHeader.PID = ((buf[1] & 0x1F) << 8) | buf[2];
        packetHeader.transport_scrambling_control = buf[3] >> 6;
        packetHeader.adaption_field_control = ((buf[3]>>4) & 0x03);
        packetHeader.continuity_counter = buf[3]& 0x03;
        if (packetHeader.transport_error_indicator == 0x01)
            return false;  //_PATInfo error so return
        return true;
    }

    bool TsParser::parseTsPackage( unsigned char* buf, int pos )
    {
        //分析包头
        TS_packet_header TsHeader;
        memset(&TsHeader,0,sizeof(TsHeader));
        if (!parseTsHeader(TsHeader, buf))
        {
            return false;
        }

        ++_nPackageCount;

        if (TsHeader.PID == PAT_PID)
        {
            std::vector<TS_PAT_Program> vecTemp;
            if (_bPATinit)  //如果初始化过，判断此PAT跟上一次比较
            {
                TS_PAT patTemp;
                parsePAT(patTemp, vecTemp, TsHeader, buf);
                if (_PATInfo.section_length != patTemp.section_length
                    || _PATInfo.version_number != patTemp.version_number)
                {
                    //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"the PATInfo is different to another\n"));
                    return false;
                }
                _ePacketType = PACKET_PAT;
                return true;
            }
            parsePAT(_PATInfo,vecTemp, TsHeader, buf);
            if (vecTemp.size() != 1)
            {
                //这里讲道理不应该存在，因为我们的节目都是有且只有一个
                assert(false);
                return false;
            }
            _ProgramInfo.program_map_PID = vecTemp[0].program_map_PID;
            _ProgramInfo.program_number = vecTemp[0].program_number;
            _bPATinit = true;
            return true;
        }

        if (TsHeader.PID == _ProgramInfo.program_map_PID)
        {
            _ePacketType = PACKET_PMT;
            TS_PMT pmtInfo;
            std::vector<TS_PMT_STREAM> vecTemp;
            if (_bPMTinit)
            {
                if (_PMTStream.elementary_PID == 0x00)
                {
                    //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"the PMTStream info is wrong\n"));
                    return false;
                }
                return true;
            }
            parsePMT(pmtInfo, vecTemp, TsHeader, buf);
            for (std::vector<TS_PMT_STREAM>::iterator iter = vecTemp.begin(); iter != vecTemp.end(); ++iter)
            {
                if ( (iter->stream_type >= 0x0A && iter->stream_type <= 0x7F) 
                    || iter->stream_type ==0x01 || iter->stream_type == 0x02)//only deal with video 1110 xxxx
                {
                    _PMTStream.stream_type = iter->stream_type;
                    _PMTStream.elementary_PID = iter->elementary_PID;
                    _bPMTinit = true;
                    //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"the audio of this frame is %d\n"), _PMTStream.elementary_PID);
                    return true;
                }
            }
            assert(false);
            return false;
        }

        if (_PMTStream.elementary_PID != 0 && TsHeader.PID == _PMTStream.elementary_PID)
        {
            _ePacketType = PACKET_PES;
            int nAdaption_field_Len = 0;
            if (TsHeader.adaption_field_control == 0x03 || TsHeader.adaption_field_control == 0x02)//has adjust field
            {
                nAdaption_field_Len = 1;
                nAdaption_field_Len += buf[PACKET_HEADER];
            }

            buf += PACKET_HEADER + nAdaption_field_Len;
            if (TsHeader.payload_unit_start_indicator == 0x01)
                return parsePESData(buf, PACKET_HEADER + nAdaption_field_Len, true);
            else
                return parsePESData(buf, PACKET_HEADER + nAdaption_field_Len, false);
        }
        return true;
    }

    bool TsParser::parsePAT(TS_PAT& patInfo, std::vector<TS_PAT_Program>& vecProgramInfo, const TS_packet_header& patHeader, unsigned char* buf)
    {
        int nAdaption_field_Len = 0;
        if (patHeader.adaption_field_control == 0x03 || patHeader.adaption_field_control == 0x02)//has adjust field
        {
            nAdaption_field_Len = 1;
            nAdaption_field_Len += buf[PACKET_HEADER];
        }

        if (patHeader.payload_unit_start_indicator == 0x01) //has pointer_field
        {
            //表示为一帧数据或者一个PSI数据的首包
            int pointer_field_len = *((unsigned char* ) (buf+ PACKET_HEADER + nAdaption_field_Len));
            buf += PACKET_HEADER + nAdaption_field_Len + pointer_field_len+1;

            memset(&patInfo, 0, sizeof(TS_PAT));
            //patInfo = {0};
            patInfo.table_id = buf[0];
            patInfo.section_syntax_indicator = buf[1]>>7;
            patInfo.zero = buf[1] >>6 & 0x1;
            patInfo.reserved_1 = buf[1] >>4 & 0x3;
            patInfo.section_length  = (buf[1] & 0x0F) << 8 | buf[2];
            patInfo.transport_stream_id = buf[3] << 8 | buf[4];
            patInfo.reserved_2 = buf[5] >> 6;
            patInfo.version_number = buf[5] >> 1 &  0x1F;
            patInfo.current_next_indicator = (buf[5] <<7) >>7;
            patInfo.section_number = buf[6];
            patInfo.last_section_number = buf[7];
            int len = 0;
            len = 3 + patInfo.section_length;
            patInfo.CRC_32  =  (buf[len-4]& 0x000000FF) << 24
                | (buf[len-3] & 0x000000FF) <<16
                | (buf[len-2] & 0x000000FF) <<8
                | (buf[len-1] & 0x000000FF);

            int n = 0;
            for (n=0; n<patInfo.section_length -12;n+=4)
            {
                TS_PAT_Program programInfo;
                programInfo.program_number = buf[8+n] <<8 | buf[9+n];
                patInfo.reserved_3 = buf[10+n] >>5;
                patInfo.network_PID = 0x00;
                if (programInfo.program_number ==0x00)
                {
                    patInfo.network_PID = (buf[10+n] & 0x1F) <<8 | buf[11+n];
                }
                else
                {
                    programInfo.program_map_PID = (buf[10+n] & 0x1F) <<8 | buf[11+n];
                    vecProgramInfo.push_back(programInfo);
                    //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"programInfo in PAT:program_number is %d, program_map_PID is %d\n"),_ProgramInfo.program_number, _ProgramInfo.program_map_PID);
                }
            }
            return true;
        }
        assert(false);
        return false;        
    }

    bool TsParser::parsePMT(TS_PMT& pmtInfo, std::vector<TS_PMT_STREAM>& vecStreamInfo, const TS_packet_header& pmtHeader, unsigned char* buf)
    {
        int nAdaption_field_Len = 0;
        if (pmtHeader.adaption_field_control == 0x03  || pmtHeader.adaption_field_control == 0x02)//has adjust field
        {
            nAdaption_field_Len = 1;
            nAdaption_field_Len += buf[PACKET_HEADER];
        }

        if (pmtHeader.payload_unit_start_indicator == 0x01) //has pointer_field
        {
            //表示为一帧数据或者一个PSI数据的首包
            int pointer_field_len = *((unsigned char* ) (buf+ PACKET_HEADER + nAdaption_field_Len));
            buf += PACKET_HEADER + nAdaption_field_Len + pointer_field_len+1;

            memset(&pmtInfo, 0, sizeof(TS_PMT));
            //pmtInfo = {0};
            pmtInfo.table_id = buf[0];
            pmtInfo.section_syntax_indicator = buf[1]>>7;
            pmtInfo.zero = buf[1]>>6 & 0x01;
            pmtInfo.reserved_1 = buf[1]>>4 & 0x03;
            pmtInfo.section_length = (buf[1] & 0x0F) << 8 | buf[2];
            pmtInfo.program_number = buf[3]<<8 | buf[4];
            pmtInfo.reserved_2 = buf[5]>>6;
            pmtInfo.version_number = buf[5] >>1 & 0x1F;
            pmtInfo.current_next_indicator = (buf[5]<<7) >>7;
            pmtInfo.section_number = buf[6];
            pmtInfo.last_section_number = buf[7];
            pmtInfo.reserved_3 = buf[8] >>5;
            pmtInfo.PCR_PID = ((buf[8]<<8) | buf[9]) & 0x1FFF; //??????
            pmtInfo.reserved_4 = buf[10]>>4;
            pmtInfo.program_info_length = (buf[10]& 0x0F) <<8 | buf[11];

            int pos = 12;
            if (pmtInfo.program_info_length != 0)
                pos += pmtInfo.program_info_length;
            //get stream type and PID
            for (; pos<=(pmtInfo.section_length+2)-4;)
            {
                TS_PMT_STREAM pmtStream;
                pmtStream.stream_type = buf[pos];
                pmtStream.elementary_PID = ( buf[pos+1]<<8  | buf[pos+2]) & 0x1FFF;
                pmtStream.ES_info_length = (buf[pos+3] & 0x0F) << 8 | buf[pos+4];
                pmtStream.descriptor = 0x00;
                if (pmtStream.ES_info_length != 0)
                {
                    for (int len = 2;len <=pmtStream.ES_info_length;len++)
                    {
                        pmtStream.descriptor = pmtStream.descriptor<<8 | buf[pos +4+len];
                    }
                    pos += pmtStream.ES_info_length;
                }
                pos += 5;
                vecStreamInfo.push_back(pmtStream);
            }
            return true;
        }
        assert(false);
        return false;
    }

    bool TsParser::parsePESData( unsigned char* buf, int nOffset, bool bIsHeader )
    {
        int nPacketLen = 0;
        if (bIsHeader)//has PES frame header
        {
            if (_bFrameLenZero)
            {
                _bFinished = true;
                return true;
            }

            TS_PES_HEADER pesHeader;
            pesHeader.pes_stream_id = buf[3];
            pesHeader.pes_packet_len = buf[4]<<8 | buf[5];

            _nFrameLen = pesHeader.pes_packet_len;
            _nRestFrameLen = _nFrameLen;
            nPacketLen = PACKET_LENTH - nOffset - 6; //current packet's length     offset: adaptation control

            buf += nOffset + 6;
            if (_PMTStream.stream_type == 0x1b)
                _bIsIFrame = IsIFrameOfH264(buf, nPacketLen);
            else
                _bIsIFrame = IsIFrameOfOther(buf, nPacketLen);

            if (_nFrameLen == 0)
            {
                //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"this frame total lenth is zero\n"));
                _bFrameLenZero = true;
                _nRestFrameLen += nPacketLen;
                //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"first packet of frame.Total lenth : %d, Packet lenth : %d, Rest lenth : %d\n"), _nFrameLen, nPacketLen, _nRestFrameLen);
            }
            else
            {
                _nRestFrameLen -= nPacketLen;

                //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"first packet of frame.Total lenth : %d, Packet lenth : %d, Rest lenth : %d\n"), _nFrameLen, nPacketLen, _nRestFrameLen);
                if (_nRestFrameLen == 0)
                {
                    _bFinished = true;
                    return true;
                }
                else if (_nRestFrameLen < 0)
                {
                    //异常？
                    assert(false);
                    return false;
                }
            }
            ++_nFramePacketCount;
            _bFindStart = true;
            return true;
        }
        else //it's not the header
        {
            if (_bFindStart)
            {
                return true;
            }

            nPacketLen = PACKET_LENTH - nOffset;

            if (_bFrameLenZero)
            {
                _nRestFrameLen += nPacketLen;
                ++_nFramePacketCount;
                //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"the %dth packet of frame.Total lenth : %d, Packet lenth : %d, Rest lenth : %d\n"), _nFramePacketCount, _nFrameLen, nPacketLen, _nRestFrameLen);
            }
            else
            {
                if (_nFrameLen == 0)
                {
                    return true;
                }

                if (_nRestFrameLen < nPacketLen && _nRestFrameLen != 0)
                {
                    //异常？
                    assert(false);
                    return false;
                }
                else if (_nRestFrameLen >= nPacketLen)
                {
                    _nRestFrameLen -= nPacketLen;
                }
                //MLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(TsParser,"the %dth packet of frame.Total lenth : %d, Packet lenth : %d, Rest lenth : %d\n"), _nFramePacketCount, _nFrameLen, nPacketLen, _nRestFrameLen);
                ++_nFramePacketCount;
                if ( _nRestFrameLen == 0)
                {
                    _bFinished = true;
                    return true;
                }
                else if (_nRestFrameLen < 0)
                {
                    //异常？
                    assert(false);
                    return false;
                }
            }
        }
        return true;
    }

    bool TsParser::IsIFrameOfH264( unsigned char* buf, int buflen )
    {
        int nPos = 0;
        while (nPos <= buflen - 5)
        {
            if (buf[nPos] == 0x00 && buf[nPos+1] == 0x00)
            {
                if (buf[nPos+2] == 0x00 && buf[nPos+3])
                {
                    if (5 == buf[nPos+4] & 31)
                    {
                        return true;
                    }
                }

                if (buf[nPos+2] == 0x01)
                {
                    if (5 == (buf[nPos+3] & 31))
                    {
                        return true;
                    }
                    else if (1 == (buf[nPos+3] & 31))
                    {
                        return false;
                    }
                }
            }
            nPos++;
        }
        return false;
    }

    bool TsParser::IsIFrameOfOther( unsigned char* buf, int buflen )
    {
        int nPosTemp = 0;
        while (nPosTemp <= buflen - 6)
        {
            if (buf[nPosTemp] == 0x00 && buf[nPosTemp+1] == 0x00 && buf[nPosTemp+2] == 0x01 && buf[nPosTemp+3] == 0x00)
            {
                if (((buf[nPosTemp+5] >> 3) & 7) == 1)
                {
                    return true;
                }
            }
            nPosTemp++;
        }
        return false;
    }

    bool TsParser::GetTSFrameBorder(unsigned char* buf, int buflen, FrameStat& framestat, bool validateEnd /*= false*/ )
    {
        return nextFrame(framestat, buf, buflen, false, validateEnd);
    }

    bool TsParser::nextVideoIFrame( FrameStat& framestat, unsigned char* buf, int buflen, bool validateEnd /*= false*/ )
    {
        return nextFrame(framestat, buf, buflen, true, validateEnd);
    }

    void TsParser::resetStatus()
    {
        _bFindStart = false;
        _bFinished = false;
        _bIsIFrame = false;
        _bFrameLenZero = false;
    }
}