#pragma once
#include <vector>

namespace C2Streamer
{
    class C2StreamerEnv;
    class TsParser
    {
    public:
        //Transport packet header
        typedef struct TS_packet_header
        {
            unsigned sync_byte       :8;
            unsigned transport_error_indicator       :1;
            unsigned payload_unit_start_indicator    :1;
            unsigned transport_priority              :1;
            unsigned PID                             :13;
            unsigned transport_scrambling_control    :2;
            unsigned adaption_field_control          :2;
            unsigned continuity_counter              :4;
        }TS_packet_header;

        typedef struct TS_PAT_Program
        {
            unsigned  program_number      :16;
            unsigned  program_map_PID     :13;
        }TS_PAT_Program;

        typedef struct Ts_PAT
        {
            unsigned table_id                       :8;
            unsigned section_syntax_indicator       :1;
            unsigned zero                           :1;
            unsigned reserved_1                     :2;
            unsigned section_length                 :12;
            unsigned transport_stream_id            :16;
            unsigned reserved_2                     :2;
            unsigned version_number                 :5;
            unsigned current_next_indicator         :1;
            unsigned section_number                 :8;
            unsigned last_section_number            :8;

            unsigned  reserved_3                    :3;
            unsigned  network_PID                   :13;
            unsigned  CRC_32                        :32;
        }TS_PAT;

        typedef struct Ts_PMT_Stream
        {
            unsigned stream_type                    :8;
            unsigned elementary_PID                 :13;
            unsigned ES_info_length                 :12;
            unsigned descriptor;
        }TS_PMT_STREAM;

        typedef struct Ts_PMT
        {
            unsigned   table_id                     :8;
            unsigned   section_syntax_indicator     :1;
            unsigned   zero                         :1;
            unsigned   reserved_1                   :2;
            unsigned   section_length               :12;
            unsigned   program_number               :16;
            unsigned   reserved_2                   :2;
            unsigned   version_number               :5;
            unsigned   current_next_indicator       :1;
            unsigned   section_number               :8;
            unsigned   last_section_number          :1;
            unsigned   reserved_3                   :3;
            unsigned   PCR_PID                      :13;
            unsigned   reserved_4                   :4;
            unsigned   program_info_length          :12;
            unsigned   reserved_5                   :3;
            unsigned   reserved_6                   :4;
            unsigned   CRC_32                       :32;
        }TS_PMT;

        typedef struct Adaptation_Field
        {
            unsigned adaptation_field_length        :8;
            unsigned discontinuity_indicator        :1;
            unsigned random_access_indicator        :1;
            unsigned elementary_stream_priority     :1;
            unsigned PCR_flag                       :1;
            unsigned OPCR_flag                      :1;
            unsigned splicing_point_flag            :1;
            unsigned transport_private_data_flag    :1;
            unsigned adaptation_field_extension_flag:1;
        }Adaptation_Field;

        typedef struct TS_PES_HEADER
        {
            unsigned pes_stream_id                  :8;
            unsigned pes_packet_len                 :16;
        }TS_PES_HEADER;

        struct FrameStat
        {
            bool         isValidPacket; 
            long int     firstPos; //pos of 0x47
            long int     startPos;//start pos of a PES frame
            long int     endPos;  //end pos of a PES frame
            FrameStat()
            {
                reset();
            }

            void reset()
            {
                isValidPacket = false;
                firstPos = -1;
                startPos = -1;
                endPos = -1;
            }
        };

        enum PACKET_TYPE
        {
            PACKET_ELSE = 0,                ///< 除了下面三种，其他的PES包
            PACKET_PAT,
            PACKET_PMT,
            PACKET_PES                      ///< 这里的PES指的是我们需要的video的帧的PES
        };

        TsParser(C2StreamerEnv& env);
        virtual ~TsParser(void);

        bool                    GetTSFrameBorder(unsigned char* buf, int buflen, FrameStat& framestat, bool validateEnd = false);
        bool                    nextVideoIFrame(FrameStat& framestat, unsigned char* buf, int buflen, bool validateEnd = false);
    public: ///< 可通用
        static bool             parseTsHeader(TS_packet_header& packetHeader, unsigned char* buf);
        static bool             parsePAT(TS_PAT& patInfo, std::vector<TS_PAT_Program>& vecProgramInfo, const TS_packet_header& patHeader, unsigned char* buf);
        static bool             parsePMT(TS_PMT& pmtInfo, std::vector<TS_PMT_STREAM>& vecStreamInfo, const TS_packet_header& pmtHeader, unsigned char* buf);

        static bool             IsIFrameOfH264(unsigned char* buf, int buflen);
        static bool             IsIFrameOfOther(unsigned char* buf, int buflen);
    private:
        bool                    nextFrame(FrameStat& framestat, unsigned char* buf, int buflen, bool bIsIFrame, bool validateEnd);

        bool                    parseTsPackage(unsigned char* buf, int pos);

        bool                    parsePESData(unsigned char* buf, int nOffset, bool bIsHeader);

        void                    resetStatus();
    private:
        //C2StreamerEnv&          mEnv;

        int                     _nPackageCount;     ///< 解析包计数
        TS_PAT_Program          _ProgramInfo;       ///< PAT中的PMT信息
        TS_PAT                  _PATInfo;           ///< PAT信息
        bool                    _bPATinit;          ///< PAT是否初始化
        bool                    _bPMTinit;          ///< PMT是否初始化
        TS_PMT_STREAM           _PMTStream;         ///< PMT中的节目信息(只处理Audio)

        int                     _nFrameLen;         ///< 一帧的数据总长度
        int                     _nRestFrameLen;     ///< 一帧的剩余长度
        int                     _nFramePacketCount; ///< 一帧所占的包的数量
        bool                    _bFrameLenZero;     ///< 帧数据总长度为0的情况

        PACKET_TYPE             _ePacketType;       ///< 当前正在解析的包类型

        bool                    _bFinished;         ///< 解析完毕
        bool                    _bFindStart;      ///< 找到帧头

        bool                    _bIsIFrame;
    };
}