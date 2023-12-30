
#include "NTPUtils.h"

namespace NTPSync
{

#define NTP_100_NSEC  10000000UL  //100 nanosecond

#define NTP_JAN_1970   2208988800UL	/* 1970 - 1900 in seconds */
#define NTP_FRAC	   4294967296. /* 2^32 as a double */ 


#ifdef ZQ_OS_MSWIN
	void getInt64FromNTPTime(NTP_Time nt, ULONGLONG& T0)
	{
		// now get the 100 nano secondes resoultion
		// There will be a loss of resolution due to the conversion to double
		// But file time resolution is only in 100s nano seconds.
		double dTimeStamp = (double) nt.dwIntSec;
		dTimeStamp += ((double)nt.dwFracSec )/(double)NTP_FRAC ;	/* 2^32 as a double */
		dTimeStamp *= NTP_100_NSEC;
		T0 = (ULONGLONG) dTimeStamp;

		//NTP time to FILE time
		//NTP time  1 Jan 1900
		//FILETIME January 1, 1601
		FILETIME ft2;
		SYSTEMTIME st;
		st.wYear = 1900;
		st.wMonth = 1;
		st.wDayOfWeek = 0;
		st.wDay = 1;
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		st.wMilliseconds = 0;
		SystemTimeToFileTime(&st, &ft2);  // ft is the file time for Jan 1 1900.
		ULONGLONG T1;
		FILETIME_to_UNIT64(ft2, T1);
		T0 += T1; // adjust the offset.
	}

	void UINT64_to_FILTETIME(ULONGLONG ull, FILETIME& ft)
	{
		ft.dwLowDateTime = (DWORD) (ull & 0xFFFFFFFF );
		ft.dwHighDateTime = (DWORD) (ull >> 32 ); 
	}

	void FILETIME_to_UNIT64(FILETIME ft, ULONGLONG& ull)
	{
		ull = (((ULONGLONG) ft.dwHighDateTime) << 32) + ft.dwLowDateTime;
	}

	void getSystemTimeAsNTPTime(NTP_Time& ntpTime)
	{
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);

		NTP_Time nt;
		getNTPTimeFromFileTime(ft, nt);
		ntpTime.dwIntSec = htonl(nt.dwIntSec); // net ntp time;
		ntpTime.dwFracSec = htonl(nt.dwFracSec);
	}

	void getNTPTimeFromFileTime(FILETIME ft, NTP_Time& ntpTime)
	{
		FILETIME ft2;
		SYSTEMTIME st;
		st.wYear = 1900;
		st.wMonth = 1;
		st.wDayOfWeek = 0;
		st.wDay = 1;
		st.wHour = 0;
		st.wMinute = 0;
		st.wSecond = 0;
		st.wMilliseconds = 0;
		SystemTimeToFileTime(&st, &ft2);  // ft2 is the file time for Jan 1 1900.

		ULONGLONG T0, T1;
		FILETIME_to_UNIT64(ft, T0);
		FILETIME_to_UNIT64(ft2, T1);
		T0 -= T1; // delete the offset.

		double d1 = (double)(__int64)( T0/NTP_100_NSEC); // get the integer part.
		double d2 = (double)(__int64)(T0%NTP_100_NSEC);  // get the fraction.
		double d4 = (d2/(double)NTP_100_NSEC) * (double)NTP_FRAC;
		ntpTime.dwIntSec = (DWORD)d1;
		ntpTime.dwFracSec = (DWORD)d4;
	}
#else
extern "C"
{
#include <sys/time.h>
#include <arpa/inet.h>
}
	void getSystemTimeAsNTPTime(NTP_Time& ntpTime)
	{
		struct timeval tval;
		gettimeofday(&tval, NULL);
		
		double dfrac = NTP_FRAC*((double)tval.tv_usec/1000000.0);

		ntpTime.dwIntSec = htonl(tval.tv_sec + NTP_JAN_1970);
		ntpTime.dwFracSec = htonl((uint32_t)dfrac); 
		
	}
	
	void getInt64FromNTPTime(NTP_Time nt, uint64& T70)
	{
		uint32 ntpsec = ntohl(nt.dwIntSec);
		uint32 ntpfrac = ntohl(nt.dwFracSec);
		double dFracNS = ((double)ntpfrac)/(double)NTP_FRAC ;	/* 2^32 as a double */
		dFracNS *= NTP_100_NSEC;
		T70 = (ntpsec - NTP_JAN_1970)*NTP_100_NSEC;
		T70 += (uint64)dFracNS;
	}
	
	void getTimevalFromInt64(uint64& T70, struct timeval& tval)
	{
		tval.tv_sec = (time_t)(T70/NTP_100_NSEC);
		tval.tv_usec = (T70%NTP_100_NSEC)/10;
	}
	
	void getInt64FromTimeval(struct timeval& tval, uint64& T70)
	{
		T70 = ((uint64)tval.tv_sec)*NTP_100_NSEC;
		T70 += tval.tv_usec*10;
	}

#endif
}
