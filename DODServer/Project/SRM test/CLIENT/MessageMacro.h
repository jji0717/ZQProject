
#ifndef MESSAGEHEADER_H
#define MESSAGEHEADER_H

#define MESSAGEFLAG		_T("Message")
#define MESSAGEHEADER		_T("MessageHeader")
#define MESSAGECODE			_T("MessageCode")
#define MESSAGETIME			_T("MessageTime")
#define SESSION             _T("SessionID")
#define MESSAGERETURN		_T("BeReturn")
#define MESSAGEBODY				_T("MessageBody")
#define MESSAGECONFIG				_T("DODDeviceController")

#define MESSAGECARDID			_T("CardID") 
#define MESSAGEBAND				_T("BrandWith")

#define MESSAGEPORTID			_T("PortID")
#define MESSAGEPMTPID			_T("PMTPID")
#define MESSAGEBITRATE			_T("BitRate")
#define MESSAGETMPFILEPATH		_T("TmpFilePath")

#define MESSAGECHANNELID		_T("ChannelID")
#define MESSAGEINTERVAL			_T("Interval")
#define MESSAGEENABLED			_T("Enabled")
#define MESSAGEFORCED			_T("Forced")
#define MESSAGECHVERSION		_T("ChannelVersion")
#define MESSAGEOBJKEYLEN		_T("ObjKeyLength")
#define MESSAGECHIDXDESLEN		_T("ChIdxDesLen")
#define MESSAGEFILENAME			_T("FileName")
#define MESSAGEFREQUENCY		_T("Frequency")
#define MESSAGECATALOGNAME		_T("Catalog")

// Virtual Port Controller
#define MESSAGECODE_CONFIG				1001
#define MESSAGECODE_INITDEVICE			1002
#define MESSAGECODE_GETBANDWIDTH		1003
#define MESSAGECODE_GETCARDIPPORT		1004
#define MESSAGECODE_GETUSEDIPPORT		1005

// Port
#define MESSAGECODE_OPEN				2001
#define MESSAGECODE_CLOSE				2002
#define MESSAGECODE_PAUSE				2003
#define MESSAGECODE_RUN					2004
#define MESSAGECODE_STOP				2005
#define MESSAGECODE_GETPORTSTATE		2006
#define MESSAGECODE_SETPORTPMTPID		2007
#define MESSAGECODE_GETPORTPMTPID		2008
#define MESSAGECODE_SETTOTALBITRATE		2009
#define MESSAGECODE_GETTMPFILEPATH		2010
#define MESSAGECODE_SETTMPFILEPATH		2011
#define MESSAGECODE_GETPORT				2012

// Channel
#define MESSAGECODE_GETCHANNEL					3001
#define MESSAGECODE_ENABLECHANNELDETECTED		3002
#define MESSAGECODE_SETDETECTINTERVAL			3003
#define MESSAGECODE_FORCEREDETECTED				3004
#define MESSAGECODE_ADDCHANNELVERSION			3005
#define MESSAGECODE_RESETCHANNELVERSION			3006
#define MESSAGECODE_SETCHANNELVERSION			3007
#define MESSAGECODE_SETOBJKEYLENGTH				3008
#define MESSAGECODE_SETIDXDESLEN			3009
#define MESSAGECODE_ENABLECHANNEL			3010
#define MESSAGECODE_SETCHANNELBITRATE		3011

// Sub Channel
#define MESSAGECODE_GETSUBCHANNEL			3012
#define MESSAGECODE_SETSUBCHANNELFREQ		3013
#define MESSAGECODE_GETSUBCHANNELFREQ		3015

// append
#define MESSAGECODE_SETCATALOGNAME			3016
#define MESSAGECODE_GETCATALOGNAME			3017
//#define MESSAGECODE_UPDATECATALOGNAME		3018	
#define MESSAGECODE_DISABLECHANNELDETECTED	3019
#define MESSAGECODE_GETTOTALBITRATE			3020
#define MESSAGECODE_GETCHANNELDETECTED		3021
#define MESSAGECODE_GETDETECTINTERVAL		3022
#define MESSAGECODE_GETCHANNELVERSION		3023
#define MESSAGECODE_GETOBJKEYLENGTH			3024
#define MESSAGECODE_GETCHANNELIDXDESLEN		3025

#endif