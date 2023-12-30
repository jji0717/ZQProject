#include "RemoteSyslog.h"

RemoteSyslog::RemoteSyslog(): _tag("TianShan"), _facility(LOG_DAEMON){}
RemoteSyslog::~RemoteSyslog(){}

bool RemoteSyslog::setup(const char* destHost, int port)
{
    ZQ::common::InetAddress dest;
    if(!dest.setAddress(destHost))
    {
        return false; 
    }

    _sock.setPeer(dest, port);

    // get host name
    char buf[64];
    if(0 != gethostname(buf, sizeof(buf)))
    {
        return false;
    }
    _host = buf;

    return true;
}

bool RemoteSyslog::open(const char* ident, int fac)
{
    // check the arguments here
    if((NULL == ident) || (32 < strlen(ident)) || strchr(ident, ' '))
    { // bad identity
        return false;
    }
    if((fac < 0) || (LOG_PRI(fac) != 0) || (LOG_FAC(fac) >= LOG_NFACILITIES))
    { // bad facility code
        return false;
    }

    _facility = fac;
    _tag = ident;

    return true;
}



int RemoteSyslog::facilityCode(const char* pFac)
{
    static struct
    {
        const char* name;
        int value;
    } facilitynames[] =
    {
        { "auth", LOG_AUTH },
        { "authpriv", LOG_AUTHPRIV },
        { "cron", LOG_CRON },
        { "daemon", LOG_DAEMON },
        { "ftp", LOG_FTP },
        { "kern", LOG_KERN },
        { "lpr", LOG_LPR },
        { "mail", LOG_MAIL },
        { "news", LOG_NEWS },
        { "syslog", LOG_SYSLOG },
        { "user", LOG_USER },
        { "uucp", LOG_UUCP },
        { "local0", LOG_LOCAL0 },
        { "local1", LOG_LOCAL1 },
        { "local2", LOG_LOCAL2 },
        { "local3", LOG_LOCAL3 },
        { "local4", LOG_LOCAL4 },
        { "local5", LOG_LOCAL5 },
        { "local6", LOG_LOCAL6 },
        { "local7", LOG_LOCAL7 },
        { NULL, -1 }
    };

    if(NULL == pFac)
        return -1;

    int i = 0;
    while(facilitynames[i].name)
    {
        if(0 == stricmp(pFac, facilitynames[i].name))
            return facilitynames[i].value;

        ++i;
    }
    return -1;
}

int RemoteSyslog::levelCode(const char* pLevel)
{
    static struct
    {
        const char* name;
        int value;
    } levelnames[] =
    {
        { "alert", LOG_ALERT },
        { "crit", LOG_CRIT },
        { "debug", LOG_DEBUG },
        { "emerg", LOG_EMERG },
        { "err", LOG_ERR },
        { "error", LOG_ERR },
        { "info", LOG_INFO },
        { "notice", LOG_NOTICE },
        { "warning", LOG_WARNING },
        { NULL, -1 }
    };
    if(NULL == pLevel)
        return -1;

    int i = 0;
    while(levelnames[i].name)
    {
        if(0 == stricmp(pLevel, levelnames[i].name))
            return levelnames[i].value;

        ++i;
    }
    return -1;
}

//convent time:<2010-03-11T16:30:15> to:<Mar  3 16:30:15>
char* RemoteSyslog::systime2stamp(const std::string& strTime, char* buf, int& len)
{
#define SYSLOG_TIMESTAMP_LEN 15
    if(strTime.length() < 19 ||NULL == buf || len < SYSLOG_TIMESTAMP_LEN)
    {
        return NULL;
    }

	char chd[4][3];
	int nyear, nmonth, nday, nhour, nmin, nsec;
	int nr = sscanf(strTime.c_str(), "%4d%[/-]%2d%[/-]%2d%[ T]%2d:%2d:%2d", &nyear,chd[0],&nmonth,chd[1],&nday,chd[2],&nhour,&nmin,&nsec);
	if(nr < 9)
	{
		return NULL;
	}
    static const char* month[] = {
        "",
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

	if(SYSLOG_TIMESTAMP_LEN != sprintf(buf, "%s %2d %02d:%02d:%02d", month[nmonth], nday, nhour, nmin, nsec))
    {
        return NULL;
    }
    len -= SYSLOG_TIMESTAMP_LEN;
    return (buf + SYSLOG_TIMESTAMP_LEN);
}

int RemoteSyslog::write(int level, const char* msg, const std::string& strLocalTime, const char* srcHost)
{
    // syslog message format:
    // <PriorityNum>TIMESTAMP HOSTNAME TAG:DETAIL
#define SYSLOG_MSGSIZE_MAX 1024
    // The hostname is limited in 64 Bytes base on STD13.
    // This length can cover the longest literal IPv6 address.
    // So we reserve 128 Bytes for the prefix.
#define SYSLOG_MSGSIZE_PREFIX 128 // > (5[PRI] + 15[TIMESTAMP] + 1[SPACE] + 64[HOSTNAME] + 1[SPACE] + 32[TAG] + 1[:])
#define SYSLOG_CONTENT_LIMIT (SYSLOG_MSGSIZE_MAX - SYSLOG_MSGSIZE_PREFIX)

    if(level < LOG_EMERG || LOG_DEBUG < level)
    { // bad log level
        return -1;
    }

    if(NULL == msg)
    {
        return -1;
    }

    int msgLen = strlen(msg);
    if(msgLen > SYSLOG_CONTENT_LIMIT)
    {
        return -1;
    }

    // prepare the internal buffer
    char buf[SYSLOG_MSGSIZE_MAX];
    char *p = buf;
    int len = SYSLOG_MSGSIZE_MAX;

    // generate the priority number
    int pri = _facility + level;
    sprintf(p, "<%d>", pri);
    p += strlen(p); len -= (p - buf);

    // generate the timestamp

    p = systime2stamp(strLocalTime, p, len);

    if(NULL == p)
    { // failure
        return -1;
    }

    (*p) = ' '; p += 1; len -= 1; // SPACE

    // append the host name
    // host name up to 64 Bytes
    if(srcHost && '\0' != (*srcHost) && strlen(srcHost) <= 64)
    {
        strcpy(p, srcHost);
        int hostLen = strlen(srcHost);
        p += hostLen; len -= hostLen;
    }
    else
    {
        strcpy(p, _host.c_str());
        p += _host.size(); len -= _host.size();
    }

    (*p) = ' '; p += 1; len -= 1; // SPACE

    // append the content
    // tag
    strcpy(p, _tag.c_str()); // program tag up to 32 Bytes
    p += _tag.size(); len -= _tag.size();

    (*p) = ':'; p += 1; len -= 1; // content delimiter

    ////////////////////////////////////////////
    //the buffer length is always ok until here
    // detail

    // assure the length is enough
    if(len < msgLen)
    {
        // failure
        return -1;
    }
    memcpy(p, msg, msgLen);
    p += msgLen; len -= msgLen;
    return _sock.send(buf, (p - buf));
}
