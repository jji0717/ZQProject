// File Name: NTPUtils.h
// Date: 2009-01
// Description: definition of ntp packet and ntp timestamp

#ifndef __NTPUTIL_H__
#define __NTPUTIL_H__

#include "ZQ_common_conf.h"
#include <time.h>


namespace NTPSync
{
	typedef struct NTP_PacketTag
	{
		int32 Control_Word;
		int32 root_delay;
		int32 root_dispersion;
		int32 reference_identifier;
		int32 reference_timestamp_seconds;
		int32 reference_timestamp_fractions;
		int32 originate_timestamp_seconds;
		int32 originate_timestamp_fractions;
		int32 receive_timestamp_seconds;
		int32 receive_timestamp_fractions;
		int32 transmit_timestamp_seconds;
		int32 transmit_timestamp_fractions;
	}NTP_Packet;

	typedef struct NTP_TimeTag
	{
		uint32 dwIntSec; // NTP fixed point Interger part
		uint32 dwFracSec; // NTP fixed point fraction part
	}NTP_Time;

	///get system time as ntp time
	///@raram ntpTime[out]
	///@return void
	void getSystemTimeAsNTPTime(NTP_Time& ntpTime);

#ifdef ZQ_OS_MSWIN
	///get ntp time from file time,FILETIME since January 1, 1601,NTP time since January 1, 1900
	///@raram ft[in]
	///@Param ntpTime[out]
	///@return    void         
	void getNTPTimeFromFileTime(FILETIME ft, NTP_Time& ntpTime);

	///get _int64 from ntp time
	///@param nt[in]    
	///@param ULONGLONG T0
	///@Return voidvoid getInt64FromNTPTime(NTP_Time nt, ULONGLONG& T0);

	///translate FILETIME struct to unsigned int 64
	///@param     FILETIME ft[in]
	///@param     ULONGLONG& ull[out]
	///@return    void
	void FILETIME_to_UNIT64(FILETIME ft, ULONGLONG& ull);

	///translate unsigned int 64 to FILETIME struct 
	///@param ULONGLONG& ull[in]
	///@param FILETIME& ft[out]
	///@return void
	void UINT64_to_FILTETIME(ULONGLONG ull, FILETIME& ft);

	///get int 64 form ntp time
	///@param NTP_Time nt[in]
	///@param ULONGLONG T0[out]
	void getInt64FromNTPTime(NTP_Time nt, ULONGLONG& T0);
#else
	//change ntp time format to 64-bit nanosecond form 1970
	void getInt64FromNTPTime(NTP_Time nt, uint64& T70);

	void getTimevalFromInt64(uint64& T70, struct timeval& tval);

	void getInt64FromTimeval(struct timeval& tval, uint64& T70);
#endif
}

#endif

