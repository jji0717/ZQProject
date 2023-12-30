// STVChannel.h: interface for the STVChannel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STVCHANNEL_H__3269CE5C_54D3_4545_9EFD_ED71B4DF9728__INCLUDED_)
#define AFX_STVCHANNEL_H__3269CE5C_54D3_4545_9EFD_ED71B4DF9728__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "STVList.h"

class STVChannel  
{
public:
	enum Stat
	{
		STAT_IDLE =0,		// channel sitting idle
		STAT_STARTING,		// channel being setup via RTSP
		STAT_PLAYING,		// channel playing
		STAT_SUSPENDING,	// channel being paused, or reached end/start of stream
		STAT_STOPPING,		// channel being tore down via RTSP
	};

public:
	//////////////////////////////////////////////////////////////////////////
	
	STVChannel(DWORD chid, const char* ip, const char* port, const char* nodegroup, const char* portid);
	virtual ~STVChannel();

	/// set base mirror path for all channels
	static	void		setBasePath(const char* base) 
	{ 
		_basepath = base; 
		char ch = _basepath[_basepath.length()-1];
		if(ch != '\\')	
			_basepath += "\\";
	}
	
	/// get base mirror path for all channels
	static std::string	getBasePath() { return _basepath; }

	/// get database mirror path
	std::string			getDBPath(){ return _dbpath; }

	/// get channel status
	Stat				getStatus() 
	{
		return _status; 
	}
	
	/// set channel status
	void				setStatus(Stat st) 
	{ 
		_status = st; 
	}

	/// get channel id
	DWORD				id() { return _chnlID; }

	/// get channel ip
	std::string			getIPAddr() { return _ipStr; }
	/// set channel ip
	void				setIPAddr(const char* ip) { _ipStr = ip; }

	/// get channel port
	std::string			getIPPort() { return _ipportStr; }
	/// set channel port
	void				setIPPort(const char* port) { _ipportStr = port; }
	
	/// get channel nodegroup
	std::string			getNodegroup() { return _nodeStr; }
	/// set channel nodegroup
	void				setNodegroup(const char* node) { _nodeStr = node; }

	/// get channel port ID
	std::string			getPortID() { return _portID; }
	/// set channel port ID
	void				setPortID(const char* portid) { _portID = portid; }

	
	/// check if channel is enabled
	bool				isActive() { return _enabled; }
	
public:
	//////////////////////////////////////////////////////////////////////////
	// channel operations

	/// enable a channel
	int					enable();

	/// disable a channel
	int					disable();
	
	/// collect channel stubs
	int					collectStubs(std::string& scheNO, int& type, int& ieIndex, int& assetIndex, int&aeIndex);

	/// clear channel stubs, this should be called when stream reached end or stopped
	int					clearStubs();

	/// get last composed feedback
	bool				getLastFeedback(SSAENotification& feedback);

	/// get channel NPT stubs
	DWORD				getNpt();

	/// get prepared AEList
	int					allocAEList(AELIST& outlist);

	/// free prepared AEList
	int					freeAEList();

public:
	//////////////////////////////////////////////////////////////////////////
	// list operations

	/// create a new list according to SM request
	///@param[in] listnode		the pointer to an XML structure containing each playlist information
	///@param[in] schNo			the schedule NO of this list
	///@param[in] listtype		the type of playlist, one among LISTTYPE_BAKKER, LISTYPE_NORMAL and LISTYPE_FILLER
	///@return					error code, STVSUCCESS when create successfully
	int					createList(ZQ::common::IPreference* listnode, const char* schNO, int listtype);

	/// restore all lists from channel path
	///@return					the number of lists restored
	int					loadLists();

	/// query a list according to schedule NO
	///@param[in] schNo			the schedule NO of this list
	///@return					the pointer to the list found, or NULL if failed to find
	STVList*			queryList(const char* schNO);

	/// extract a list from pools to start
	///@param[out] schStr		the schedule number of the extracted list, or NULL if none suitable
	///@param[out] ieIndex		the index of ie in the extracted list, or -1 if none suitable
	///@param[out] type			the type of the schedule list
	///@return					the calculated time how long should timer sleep for next wakeup
	///@remarks		there are 3 kind of result combinations \n
	///1. schStr is empty, then this channel has nothing to stream, should stop \n
	///2. schStr is not empty, but ieIndex is -1, then this channel is just stream what it should be streaming, do nothing \n
	///3. schStr is not empty, and ieIndex is not -1, then this channel should start a new stream according to nextList and ieIndex
	DWORD				extract(std::string& schStr, int& ieIndex, int& type);

	/// compose ISS AElist map
	///@param[in] schNO			the schedule NO of the list that need compose
	///@param[in] ieIndex		the index of ie in the list that need compose
	///@return		error code, STVSUCCESS if succeeded
	int					compose(const char* schNO, int ieIndex);

	/// update current playing asset NO and generate xml feedback stream
	///@param[in]	feedback		the status feedback from ISS table
	///@param[out]	backstream		the xml stream of asset element status, which will be sent to SM
	///@param[in,out] backcount		the max count of stream input, and after the call succeeded, the actual count of output stream
	///@return		error code, STVSUCCESS if succeeded
	int					updatePlayingElem(const SSAENotification& feedback, char* backstream, DWORD* backcount);
	
private:
	//////////////////////////////////////////////////////////////////////////
	// pool operations
	
	/// register new list to channel
	bool				regList(STVList* newlist);

	/// unregister list from channel
	bool				unregList(STVList* exlist);

	/// clear all expired lists
	void				clearExpiredLists();

	/// delete old duplicated lists
	bool				clearSerialLists(const char* schNO, int listtype);

private:
	//////////////////////////////////////////////////////////////////////////
	// channel attributes

	/// channel id
	DWORD				_chnlID;

	/// channel status
	Stat				_status;

	/// mutex for status
	ZQ::common::Mutex	_statusMutex;

	/// channel enable/disable flag
	bool				_enabled;

	/// channel ip
	std::string			_ipStr;

	/// channel port
	std::string			_ipportStr;

	/// channel nodegroup
	std::string			_nodeStr;

	/// channel port ID
	std::string			_portID;
	
	/// base db mirror path. eg: "D:\\ITVPLAYBACK\\DB\\"
	static std::string	_basepath;

	/// channel db mirror path. eg: "D:\\ITVPLAYBACK\\DB\\1000"
	std::string			_dbpath;

private:
	//////////////////////////////////////////////////////////////////////////
	// list pools
	
	std::vector<STVList*>			_normalPool;

	std::vector<STVList*>			_fillerPool;

	std::vector<STVList*>			_barkerPool;

	ZQ::common::Mutex				_listMutex;

private:
	//////////////////////////////////////////////////////////////////////////
	// stream stubs

	/// Mutex for stubs
	ZQ::common::Mutex				_stubMutex;

	/// stub indicating whether this stub is attached with a list, or alone
	bool							_stubIsAlone;
		
	/// stub containing current playing Ie
	ASSETS							_stubIe;

	/// stub containing current playing AEList
	AELIST							_stubAE;

	/// stub containing current playing list
	std::string						_stubSchedNO;

	/// stub containing current playing list type
	int								_stubType;

	/// stub containing current playing ie index of the list
	int								_stubListIeIdx;

	/// stub containing current playing asset index of the ie
	int								_stubIeAssetIdx;

	/// stub containing current playing element index of the ie
	int								_stubIeAeIdx;

	/// stub containing current playing element status
	bool							_stubIeAeActive;

	/// stub containing last composed feedback
	SSAENotification				_stubFeedback;

	/// stub containing npt time for this IE
	DWORD							_stubNPT;
};

#endif // !defined(AFX_STVCHANNEL_H__3269CE5C_54D3_4545_9EFD_ED71B4DF9728__INCLUDED_)
