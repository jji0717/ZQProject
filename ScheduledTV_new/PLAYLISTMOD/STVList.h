// STVList.h: interface for the STVList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STVLIST_H__CB508C05_8252_4734_91A2_4894518C23AD__INCLUDED_)
#define AFX_STVLIST_H__CB508C05_8252_4734_91A2_4894518C23AD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// local module include
#include "STVList_def.h"

// std includes
#pragma warning(disable: 4786)
#include <string>

#include "issapi.h"

// other module include
#include "../STVMainHeaders.h"
#include "../Filler/AutoFiller.h"
#include "../Filler/BarkerFiller.h"
#include "../IssStreamCtrl/stv_ssctrl.h"

// common lib include
#include "XMLPreference.h"

// IE contains all info in an IE item
typedef struct tag_IE{
	ASSETS		assetList;
	SYSTEMTIME	timeBegin;
	DWORD		dwLength;
}IE, *PIE;

typedef std::vector<IE>	IES;

/// class STVList is the base class for all kind of STV schedule list
class STVList  
{
	friend class STVChannel;
	
public:
	
	//////////////////////////////////////////////////////////////////////////
	// constructor & destructor
private:
	STVList(STVChannel&	chnl);

public:
	virtual ~STVList();

public:
	//////////////////////////////////////////////////////////////////////////
	// object attributes methods

	/// get database mirror file name
	///@return		the database mirror file name
	std::string		getDBFile() { return _dbFile; }

	/// get list type
	int				getType() { return _type; }

	/// check if list is global
	bool			isGlobal() { return bool(_type&LIST_GLOBAL); }

	/// get fill type
	FILLTYPE		getFillType() { return _fillType; }

	/// get schedule NO
	std::string		getScheduleNO() { return _scheduleNO; }

	/// set delete db file flag
	void			setDeleteFile(bool bdelete) { _deleteFile = bdelete; }

public:
	//////////////////////////////////////////////////////////////////////////
	// operations
	
	/// create a new list according to SM request
	///@param[in] listnode		the pointer to an XML structure containing each playlist information
	///@param[in] schNo			the schedule NO of this list
	///@param[in] listtype		the type of playlist, one among LISTTYPE_BAKKER, LISTYPE_NORMAL and LISTYPE_FILLER
	///@return error code, STVSUCCESS when create successfully
	int			create(ZQ::common::IPreference* listnode, const char* schNO, int listtype);

	/// create a new list from existed db mirror file
	///@param[in] dbfilename	name of the file, including path
	///@return error code, STVSUCCESS when create successfully
	int			create(const char* dbfilename);

	/// fetch asset elements from playlist XML structure
	///@param[in] ieIndex		the index of the ie in ie list
	///@param[out] listasset	the list containing output assets and asset elements
	///@return error code, STVSUCCESS when create successfully
	int			fetchAE(int ieIndex, ASSETS& listasset);

	/// get list time status
	///@param[in] currtime		current time
	///@param[out] position		which index of ie current time fit in. -1 if none fit.
	///@param[out] waittime		indicate how long should wait for next timepoint.  If no timepoint in future, return DEFAULT_SLEEPTIME
	///@return					LISTSTAT_EFFECTIVE, LISTSTAT_LEISURE, LISTSTAT_PREMATURE, or LISTSTAT_EXPIRED
	///@remarks		this function check the time attribute status of the list. 
	/// If the list is in effective for filler/barker (or should be on-going for normal list), it returns LISTSTAT_EFFECTIVE;
	/// If the list is sitting idle between different IEs (for normal playlist only), it returns LISTSTAT_LEISURE;
	/// if it has not reached effective time for filler/barker (or not reaching start time for normal list), it returns LISTSTAT_PREMATURE;
	/// if it has expired (or should have ended for normal list), it returns LISTSTAT_EXPIRED
	int			timeStat(SYSTEMTIME currtime, int& position, DWORD& waittime);

	/// get specified ie's begin time
	///@param[in] ieindex		the index of the ie
	///@param[out] timepoint	contains the begin time of the ie
	///@param[out] length		contains the length of the ie
	///@return		true if get success, false else
	bool		getIeTime(int ieindex, SYSTEMTIME& timepoint, DWORD& length);

	/// convert CueTime format to Seconds
	///@param[in] CueTime		the cue duration ITV uses, such as 12:30:45:011, which means \n
	/// 12 hours 30 minutes 45 seconds and 011 frames
	///@return					the calculated seconds of the duration
	static DWORD Cue2Sec(const char* CueTime);

	/// check if file name is a valid file for list
	///@param[in] filename		the name string of the file
	///@return					true if valid, false else
	static bool	isValidFile(const char* filename);

private:
	//////////////////////////////////////////////////////////////////////////
	// internal functions
	
	/// prepare all attributes of the list according to information past in
	int			prepare(ZQ::common::IPreference* listnode);

protected:
	//////////////////////////////////////////////////////////////////////////
	// object self attributes

	/// pointer to the channel object belongs to
	STVChannel&				_channel;

	/// list containing all information of the schedule
	IES						_ieList;

	/// index pointing to last playing ie
	int						_ieIndex;

	/// lock for ie list
	ZQ::common::Mutex		_ieMutex;

	/// mirror database file name, including path
	std::string				_dbFile;

	/// schedule NO
	std::string				_scheduleNO;

	/// list type
	int						_type;

	/// filler type (for filler/barker)
	FILLTYPE				_fillType;

	/// indicate whether delete the db file when destructed
	bool					_deleteFile;
};

#endif // !defined(AFX_STVLIST_H__CB508C05_8252_4734_91A2_4894518C23AD__INCLUDED_)
