////////////////////////////////////////////////////////////////////////////////
//http://cr.yp.to/ftpparse.html
// Currently covered formats:
// EPLF.
// UNIX ls, with or without gid.
// Microsoft FTP Service.
// Windows NT FTP Server.
// VMS.
// WFTPD.
// NetPresenz (Mac).
// NetWare.
// MSDOS.
//
// Definitely not covered: 
// Long VMS filenames, with information split across two lines.
// NCSA Telnet FTP server. Has LIST = NLST (and bad NLST for directories).
//
////////////////////////////////////////////////////////////////////////////////

#ifndef INC_FTPLISTPARSE_H
#define INC_FTPLISTPARSE_H

#include <string>
#include <time.h>


namespace nsFTP
{


typedef struct FTPFileStatus
{

enum T_enSizeType 
{
    stUnknown,
    stBinary,        ///< size is the number of octets in TYPE I
    stASCII,         ///< size is the number of octets in TYPE A
};

enum T_enMTimeType 
{
    mttUnknown,
    mttLocal,        ///< time is local
    mttRemoteMinute, ///< time zone and secs are unknown
    mttRemoteDay,    ///< time zone and time of day are unknown
};

	std::string _strName;
	std::string _strPath;
	bool		_bTryCwd;
	bool		_bTryRetr;
	T_enSizeType	_enSizeType;
	long		_lSize;
	T_enMTimeType	_enTimeType;
	time_t		_mTime;
	std::string _strAttributes;
	std::string	_strUID;
	std::string _strGID;
#ifdef _DEBUG
	std::string _strMTime;
#endif

	FTPFileStatus()
	{
		_bTryCwd = false;
		_bTryRetr = false;
		_enSizeType = stUnknown;
		_lSize = 0;
		_enTimeType = mttUnknown;
		_mTime = 0;
	}

}FILEINFO;

/// Implements the parsing of the string returned by the LIST command.
class CFTPListParse
{
public:
	CFTPListParse();
	~CFTPListParse();

public:
	bool Parse(FILEINFO& ftpFileStatus, const std::string& strLineToParse);

	bool IsEPLS(const char* pszLine);
	bool ParseEPLF(FILEINFO& ftpFileStatus, const char* pszLine, int iLength);

	bool IsUNIXStyleListing(const char* pszLine);
	bool ParseUNIXStyleListing(FILEINFO& ftpFileStatus, const char* pszLine, int iLength);

	bool IsMultiNetListing(const char* pszLine);
	bool ParseMultiNetListing(FILEINFO& ftpFileStatus, const char* pszLine, int iLength);

	bool IsMSDOSListing(const char* pszLine);
	bool ParseMSDOSListing(FILEINFO& ftpFileStatus, const char* pszLine, int iLength);

private:
	bool CheckMonth(const char* pszBuffer, const char* pszMonthName) const;
	int  GetMonth(const char* pszBuffer, int iLength) const;
	bool GetLong(const char* pszLong, int iLength, long& lResult) const;
	long GetYear(time_t time) const;

	long ToTAI(long lYear, long lMonth, long lMDay) const;
	long GuessTAI(long lMonth, long lMDay);

private:
	time_t			m_tmBase;       // time() value on this OS at the beginning of 1970 TAI
	long			m_lCurrentYear; // approximation to current year
	static char*	m_Months[12];
};

}

#endif // INC_FTPLISTPARSE_H
