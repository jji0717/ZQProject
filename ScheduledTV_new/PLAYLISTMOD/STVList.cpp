// STVList.cpp: implementation of the STVList class.
//
//////////////////////////////////////////////////////////////////////

#include "STVList.h"
#include "STVPMTimerMan.h"
#include "STVChannel.h"
#include "STVPlaylistManager.h"
#include "ScLog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

STVList::STVList(STVChannel& chnl)
:_channel(chnl)
{
	_dbFile			= "";
	_type			= LISTTYPE_UNKNOWN;
	_fillType		= FILLTYPE_SERIAL;	
	_scheduleNO		= "";

	_deleteFile		= false;
	
	_ieList.clear();
	_ieIndex = -1;
}

STVList::~STVList()
{
	for(IES::iterator ieIter = _ieList.begin(); ieIter != _ieList.end(); ieIter++)
	{
		ASSETS* pAssetList = &(ieIter->assetList);
		for(ASSETS::iterator assetIter = pAssetList->begin(); assetIter != pAssetList->end(); assetIter++)
		{
			assetIter->AssetElements.clear();
		}
		pAssetList->clear();
	}
	_ieList.clear();

	// delete db file if delete flag is set
	if(_deleteFile)
		::DeleteFileA(_dbFile.c_str());
}

int STVList::create(ZQ::common::IPreference* listnode, const char* schNO, int listtype)
{
	// -- get db directory
	
	std::string path = "";
	path = _channel.getDBPath();

	if(path[path.length()-1] != '\\')	// add '\' if necessary
		path += "\\";

	// -- prepare schedule number and list type

	_type = listtype;
	_scheduleNO = schNO;

	// -- generate db file
	// !!! Notice that the file name could NOT exceed MAX_PATH long
	
	_dbFile = path;						// add path
	if(ISGLOBALTYPE(listtype))
		_dbFile += "G";
	else
		_dbFile += "_";

	switch(BASICTYPE(listtype))			// add list type to file name
	{
	case LISTTYPE_NORMAL:
		_dbFile += "P@";
		break;
	case LISTTYPE_BARKER:
		_dbFile += "B@";
		break;
	case LISTTYPE_FILLER:
		_dbFile += "F@";
		break;
	default:
		break;
	}
	_dbFile += _scheduleNO;				// add schedule NO
	_dbFile += ".";
	_dbFile += DB_FILE_EXT;				// add ".iel"

	ZQ::common::XMLPrefDoc	xmldoc(STVPMTimerMan::_coInit);
	xmldoc.open(_dbFile.c_str(), XMLDOC_CREATE);
	ZQ::common::PrefGuard	rootGd(xmldoc.newElement(TAG_DBMIRROR));
	rootGd.pref()->addNextChild(listnode);	// add list to root
	xmldoc.set_root(rootGd.pref());
	xmldoc.save();

	return prepare(listnode);
}

int STVList::create(const char* dbfilename)
{
	_dbFile = dbfilename;
	if(_dbFile.empty())
	{
		GTRACEERR;
		return STVERRFORMAT;
	}

	// -- prepare schedule NO and list type
	// the file may seems like _P@505231111.iel

	char	ch_type;
	size_t	slashidx = _dbFile.find_last_of('\\');
	if(slashidx+1>=_dbFile.length())
	{
		GTRACEERR;
		return STVERRFORMAT;
	}

	ch_type = _dbFile[slashidx+1];		// get global character
	switch(ch_type) 
	{
	case 'G':
	case 'g':
		_type |= LIST_GLOBAL;
		break;
	case '_':
		_type &= 0xF;
		break;
	default:
		GTRACEERR;
		return STVERRFORMAT;
		break;
	}

	ch_type = _dbFile[slashidx+2];		// get type character
	switch(ch_type) 
	{
	case 'P':
	case 'p':
		_type |= LISTTYPE_NORMAL;
		break;
	case 'B':
	case 'b':
		_type |= LISTTYPE_BARKER;
		break;
	case 'F':
	case 'f':
		_type |= LISTTYPE_FILLER;
		break;
	default:
		GTRACEERR;
		return STVERRFORMAT;
		break;
	}

	_scheduleNO = _dbFile.substr(slashidx+2+2);
	size_t		dotidx = _scheduleNO.find('.');
	if(std::string::npos == dotidx)
	{
		GTRACEERR;
		return STVERRFORMAT;
	}
	
	_scheduleNO = _scheduleNO.substr(0, dotidx);
	if(_scheduleNO.empty())
	{
		GTRACEERR;
		return STVERRFORMAT;
	}

	// -- open xml file
	// !!! Notice that the file name could NOT exceed MAX_PATH long
	
	ZQ::common::XMLPrefDoc xmldoc(STVPMTimerMan::_coInit);
	xmldoc.open(_dbFile.c_str());

	ZQ::common::PrefGuard	rootGd(xmldoc.root());
	if(!rootGd.valid())
	{
		GTRACEERR;
		return STVERRFORMAT;
	}

	ZQ::common::PrefGuard	listGd(rootGd.pref()->firstChild());
	if(!listGd.valid())
	{
		GTRACEERR;
		return STVERRFORMAT;
	}

	return prepare(listGd.pref());
}

// this is the function that parse everything into memory
int STVList::prepare(ZQ::common::IPreference* listnode)
{
	char buff[256];

	SYSTEMTIME beginTime, endTime;
	beginTime.wYear		= 1970;
	beginTime.wMonth	= 1;
	beginTime.wDay		= 1;
	beginTime.wHour		= 0;
	beginTime.wMinute	= 0;
	beginTime.wSecond	= 0;
	beginTime.wDayOfWeek	= 7;
	beginTime.wMilliseconds = 0;
	endTime.wYear		= 2050;
	endTime.wMonth		= 12;
	endTime.wDay		= 30;
	endTime.wHour		= 23;
	endTime.wMinute		= 59;
	endTime.wSecond		= 59;
	endTime.wDayOfWeek		= 7;
	endTime.wMilliseconds	= 0;
	try
	{
		// -- prepare fill type, time attributes (for filler/barker only)
		if(BASICTYPE(_type) != LISTTYPE_NORMAL)
		{
			buff[0]='\0';
			if(listnode->has(KEY_LIST_PLAYMODE))		// fill type
			{
				listnode->get(KEY_LIST_PLAYMODE, buff);
				std::string modestr = buff;
				if(modestr == "S")
					_fillType = FILLTYPE_SERIAL;
				else if(modestr == "R")
					_fillType = FILLTYPE_RANDOM;
			}

			buff[0]='\0';
			if(listnode->has(KEY_LIST_EFFECTTIME))		// effective time
			{
				listnode->get(KEY_LIST_EFFECTTIME, buff);
				std::string timestr = buff;
				beginTime.wYear			= atol(timestr.substr(0,4).c_str());
				beginTime.wMonth		= atol(timestr.substr(4,2).c_str());
				beginTime.wDay			= atol(timestr.substr(6,2).c_str());
				beginTime.wHour			= atol(timestr.substr(9,2).c_str());
				beginTime.wMinute		= atol(timestr.substr(12,2).c_str());
				beginTime.wSecond		= atol(timestr.substr(15,2).c_str());
				beginTime.wDayOfWeek= 7;		// don't calculate weekdays
				beginTime.wMilliseconds = 0;	// don't care about milli seconds
			}
			
			buff[0]='\0';
			if(listnode->has(KEY_LIST_ENDTIME))			// end date time
			{
				listnode->get(KEY_LIST_ENDTIME, buff);
				std::string timestr = buff;
				endTime.wYear			= atol(timestr.substr(0,4).c_str());
				endTime.wMonth			= atol(timestr.substr(4,2).c_str());
				endTime.wDay			= atol(timestr.substr(6,2).c_str());
				endTime.wHour			= atol(timestr.substr(9,2).c_str());
				endTime.wMinute			= atol(timestr.substr(12,2).c_str());
				endTime.wSecond			= atol(timestr.substr(15,2).c_str());
				endTime.wDayOfWeek= 7;			// don't calculate weekdays
				endTime.wMilliseconds = 0;		// don't care about milli seconds
			}
		}

		// -- go through all assets and get information of them
		ZQ::common::MutexGuard	tmpGd(_ieMutex);
		ZQ::common::PrefGuard assetGd(listnode->firstChild());
		bool	b1stAsset = true;
		IE		ieItem;

		ieItem.assetList.clear();

		// parse assets
		for(;assetGd.valid();assetGd.pref(listnode->nextChild()) )
		{
			ASSET	assetItem;

			if(!assetGd.pref()->has(KEY_LIST_ASSET_ID) && !assetGd.pref()->has(KEY_LIST_ASSET_NO))
			{
				GTRACEERR;
				return STVERRFORMAT;
			}

			if(assetGd.pref()->has(KEY_LIST_ASSET_TYPE) )	
			{
				buff[0]='\0';
				assetGd.pref()->get(KEY_LIST_ASSET_TYPE, buff);	// asset type

				// filler and barker can ONLY have 1 IE, no matter what asset type it is
				// so ignore all type 'I' except for the first asset
				if(b1stAsset || ( (buff[0] == 'I' || buff[0] == 'i') && BASICTYPE(_type) == LISTTYPE_NORMAL ) )	// if is independent asset, or 1st asset, push previous ie into IeList
				{
					if(b1stAsset)
					{
						// 1st asset, no previous ie
						b1stAsset = false;
					}
					else if(BASICTYPE(_type) == LISTTYPE_NORMAL)
					{
						// another ie starts, push previous ie into IeList
						_ieList.push_back(ieItem);
					}
					
					// set new ie attributes
					ieItem.assetList.clear();
					ieItem.dwLength = 0;
					buff[0]='\0';
					if(BASICTYPE(_type)!=LISTTYPE_NORMAL)	// filler/barker, timeBegin=effectiveTime, dwLength=effectiveSpan
					{
						ieItem.timeBegin = beginTime;
						ieItem.dwLength	= STVPlaylistManager::timeSub(endTime, beginTime);
					}
					else if(assetGd.pref()->has(KEY_LIST_ASSET_BEGIN))
					{
						assetGd.pref()->get(KEY_LIST_ASSET_BEGIN, buff);
						std::string timestr = buff;
						ieItem.timeBegin.wYear			= atol(timestr.substr(0,4).c_str());
						ieItem.timeBegin.wMonth			= atol(timestr.substr(4,2).c_str());
						ieItem.timeBegin.wDay			= atol(timestr.substr(6,2).c_str());
						ieItem.timeBegin.wHour			= atol(timestr.substr(9,2).c_str());
						ieItem.timeBegin.wMinute		= atol(timestr.substr(12,2).c_str());
						ieItem.timeBegin.wSecond		= atol(timestr.substr(15,2).c_str());
						ieItem.timeBegin.wDayOfWeek= 7;			// don't calculate weekdays
						ieItem.timeBegin.wMilliseconds = 0;		// don't care about milli seconds
					}
					else
					{
						GTRACEERR;
						return STVERRFORMAT;
					}
				
				}
			}

			buff[0]='\0';
			assetGd.pref()->get(KEY_LIST_ASSET_ID, buff);		// asset id
			assetItem.dwAssetUID = atol(buff);

			buff[0]='\0';
			assetGd.pref()->get(KEY_LIST_ASSET_NO, buff);		// asset NO
			assetItem.dwNO = atol(buff);

			if(assetGd.pref()->has(KEY_LIST_ASSET_DURATION))	// asset duration
			{
				buff[0]='\0';
				assetGd.pref()->get(KEY_LIST_ASSET_DURATION, buff);	
				assetItem.dwPlayTime = atol(buff);
			}
			else
			{
				assetItem.dwPlayTime = 0;
			}
			
			if(assetGd.pref()->has(KEY_LIST_ASSET_PRIORITY))	// asset priority
			{
				buff[0]='\0';
				assetGd.pref()->get(KEY_LIST_ASSET_PRIORITY, buff);	
				assetItem.dwWeight = (atol(buff)<1)? 1 : atol(buff);
			}
			else
			{
				assetItem.dwPlayTime = 1;
			}

			// parse elements
			for(ZQ::common::PrefGuard elementGd(assetGd.pref()->firstChild()); elementGd.valid(); elementGd.pref(assetGd.pref()->nextChild()))
			{
				ASSETELEMENT	aeItem;
				DWORD	hh,mm,ss,frame;

				if(!elementGd.pref()->has(KEY_LIST_ASSET_AE_ID) && !elementGd.pref()->has(KEY_LIST_ASSET_AE_NO))
				{
					GTRACEERR;
					return STVERRFORMAT;
				}

				buff[0]='\0';
				elementGd.pref()->get(KEY_LIST_ASSET_AE_ID, buff);	// element id
				aeItem.dwAEUID = atol(buff);

				buff[0]='\0';
				elementGd.pref()->get(KEY_LIST_ASSET_AE_NO, buff);	// element NO
				aeItem.dwNO = atol(buff);

				if(elementGd.pref()->has(KEY_LIST_ASSET_AE_BITRATE))	// element bitrate
				{
					buff[0]='\0';
					elementGd.pref()->get(KEY_LIST_ASSET_AE_BITRATE, buff);
					aeItem.dwBitRate = atol(buff);
				}
				else
				{
					aeItem.dwBitRate = 0;
				}

				if(elementGd.pref()->has(KEY_LIST_ASSET_AE_CUEIN))	// element CueIn
				{		
					buff[0]='\0';
					elementGd.pref()->get(KEY_LIST_ASSET_AE_CUEIN, buff);
					sscanf(buff, "%ld:%ld:%ld:%ld", &hh,&mm,&ss,&frame);
					aeItem.dwCueIn = hh*3600+mm*60+ss;
				}
				else 
				{
					aeItem.dwCueIn = 0;
				}
				
				if(elementGd.pref()->has(KEY_LIST_ASSET_AE_CUEOUT)) // element CueOut
				{		
					buff[0]='\0';
					elementGd.pref()->get(KEY_LIST_ASSET_AE_CUEOUT, buff);
					sscanf(buff, "%ld:%ld:%ld:%ld", &hh,&mm,&ss,&frame);
					aeItem.dwCueOut = hh*3600+mm*60+ss;
				}
				else 
				{
					aeItem.dwCueOut = 0;
				}

				if(elementGd.pref()->has(KEY_LIST_ASSET_AE_CUEDURATION)) // element CueDuration
				{		
					buff[0]='\0';
					elementGd.pref()->get(KEY_LIST_ASSET_AE_CUEDURATION, buff);
					sscanf(buff, "%ld:%ld:%ld:%ld", &hh,&mm,&ss,&frame);
					aeItem.dwPlayTime = hh*3600+mm*60+ss;
				}
				else 
				{
					aeItem.dwPlayTime = 0;
				}

				// add to asset item
				assetItem.AssetElements.push_back(aeItem);
			}

			// add to ie item
			if(assetItem.AssetElements.empty())
			{
				GTRACEERR;
				return STVERRFORMAT;
			}
			
			ieItem.assetList.push_back(assetItem);
			if(BASICTYPE(_type)==LISTTYPE_NORMAL)
				ieItem.dwLength += assetItem.dwPlayTime;
			
		}

		// push last ie into list
		if(!ieItem.assetList.empty())
		{
			_ieList.push_back(ieItem);
		}

	}
	catch(std::exception &e){
		glog(ZQ::common::Log::L_CRIT, "STVList::prepare()  Got std exception: %s", e.what());
	}	
	catch (...) {
		glog(ZQ::common::Log::L_CRIT, "STVList::prepare()  Got unknown exception");
	}
	
	return STVSUCCESS;
}

int STVList::fetchAE(int ieIndex, ASSETS& listasset)
{
	for(int i=0; i<_ieList[ieIndex].assetList.size(); i++)
	{	// for each asset
		ASSET assetItem;
		assetItem.dwAssetUID	= _ieList[ieIndex].assetList[i].dwAssetUID;
		assetItem.dwNO			= _ieList[ieIndex].assetList[i].dwNO;
		assetItem.dwPlayTime	= _ieList[ieIndex].assetList[i].dwPlayTime;
		assetItem.dwWeight		= _ieList[ieIndex].assetList[i].dwWeight;

		for(int j=0; j<_ieList[ieIndex].assetList[i].AssetElements.size(); j++)
		{	// for each element
			ASSETELEMENT	aeItem;
			aeItem.dwAEUID		= _ieList[ieIndex].assetList[i].AssetElements[j].dwAEUID;
			aeItem.dwNO			= _ieList[ieIndex].assetList[i].AssetElements[j].dwNO;
			aeItem.dwBitRate	= _ieList[ieIndex].assetList[i].AssetElements[j].dwBitRate;
			aeItem.dwCueIn		= _ieList[ieIndex].assetList[i].AssetElements[j].dwCueIn;
			aeItem.dwCueOut		= _ieList[ieIndex].assetList[i].AssetElements[j].dwCueOut;
			aeItem.dwPlayTime	= _ieList[ieIndex].assetList[i].AssetElements[j].dwPlayTime;

			assetItem.AssetElements.push_back(aeItem);
		}

		listasset.push_back(assetItem);
	}

	return STVSUCCESS;
}

int STVList::timeStat(SYSTEMTIME currtime, int& position, DWORD& waittime)
{
	int ret=LISTSTAT_PREMATURE, idealIndex=-1;
	DWORD sleeptime = DEFAULT_SLEEPTIME, elapsetime = 0;

	ZQ::common::MutexGuard	tmpGd(_ieMutex);
	for(int i=0; i<_ieList.size(); i++)
	{
		if(STVPlaylistManager::timeComp(_ieList[i].timeBegin, currtime) <=0)
		{	// this ie starts early then now, see if it has over

			elapsetime = STVPlaylistManager::timeSub(currtime, _ieList[i].timeBegin);
			if(elapsetime < _ieList[i].dwLength)
			{	// maybe still streaming
				idealIndex = i;
				ret = LISTSTAT_EFFECTIVE;
			}
			else if(BASICTYPE(_type)==LISTTYPE_NORMAL)
			{	// although expired, we do not care about normal STV list
				idealIndex = i;
				ret = LISTSTAT_EFFECTIVE;
			}
			else
			{	// already expired
				ret = LISTSTAT_EXPIRED;
			}
		}
		else if(STVPlaylistManager::timeComp(_ieList[i].timeBegin, currtime) >0)
		{	// this ie starts later than now, we must have found the idealIndex up to now
			DWORD t = 1000*STVPlaylistManager::timeSub(_ieList[i].timeBegin, currtime);
			if(t<=1000*LISTTIME_CRITPLAY)
			{	// only less then LISTTIME_CRITPLAY seconds later
				idealIndex = i;
				ret = LISTSTAT_EFFECTIVE;
			}
			else
			{	// seems that we have to wait some time for this ie to start
				if(t<sleeptime)
				{
					sleeptime = t - 1000*LISTTIME_CRITPLAY;
				}
				
				if(ret == LISTSTAT_EXPIRED)	// still exists later ie, so should be leisure
					ret = LISTSTAT_LEISURE;
			}

			break;
		}
	}
	
	position = idealIndex;
	waittime = sleeptime;
	return ret;
}

bool STVList::getIeTime(int ieindex, SYSTEMTIME& timepoint, DWORD& length)
{
	ZQ::common::MutexGuard		tmpGd(_ieMutex);
	
	if(ieindex<0 || ieindex>=_ieList.size())
	{
		GTRACEERR;
		return false;
	}

	timepoint = _ieList[ieindex].timeBegin;
	length	  = _ieList[ieindex].dwLength;
	return true;
}

DWORD STVList::Cue2Sec(const char* CueTime)
{
	DWORD dwRet=0;
	DWORD factor=3600;		// time factor, 3600 for hour, 60 for min, 1 for sec
	size_t pos;		// position for colon ':'
	std::string substr=CueTime;
	std::string timestr;

	if(substr.empty())
	{
		GTRACEERR;
		return 0;
	}
	
	// calculate hour, min and sec
	for(int i=3; i>0; i--)
	{	
		pos = substr.find_first_of(":");
		if(pos==std::string::npos)
		{
			GTRACEERR;
			return 0;
		}
		timestr = substr.substr(0, pos);
		dwRet += atol(timestr.c_str())*factor;
		substr = substr.substr(pos+1);
		factor/=60;
	}
	
	return dwRet;
}

bool STVList::isValidFile(const char* filename)
{
	std::string nameStr = filename;
	
	if(nameStr.empty())
	{
		GTRACEERR;
		return false;
	}

	char	ch_type;

	// check file name length
	size_t	slashidx = nameStr.find_last_of('\\');
	if(slashidx+1>=nameStr.length())
	{
		GTRACEERR;
		return false;
	}

	// check global character
	ch_type = nameStr[slashidx+1];		
	if(ch_type != 'G' && ch_type != 'g' && ch_type != '_')
	{
		GTRACEERR;
		return false;
	}
	
	// check type character
	ch_type = nameStr[slashidx+2];		
	if(ch_type != 'P' && ch_type != 'p'
		&& ch_type != 'B' && ch_type != 'b'
		&& ch_type != 'F' && ch_type != 'f')
	{
		GTRACEERR;
		return false;
	}
	
	// check file extension
	nameStr = nameStr.substr(slashidx+2+2);
	size_t		dotidx = nameStr.find('.');
	if(std::string::npos == dotidx)
	{
		GTRACEERR;
		return false;
	}
	
	// check schedule number
	nameStr = nameStr.substr(0, dotidx);
	if(nameStr.empty())
	{
		GTRACEERR;
		return false;
	}

	return true;
	
}