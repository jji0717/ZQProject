#ifndef _STVLIST_DEF_H
#define _STVLIST_DEF_H

// common lib includes
#include "Locks.h"
#include "ScLog.h"

//////////////////////////////////////////////////////////////////////////

// the maximum number of sub-channels for a NVOD channel
#define	MAX_SUBCHNL				1000

#define	DEFAULT_FILLLENGTH		3600		// 1 hour

#define DEFAULT_OUTOFDATE_TIME	86400		// 1 day

#define DEFAULT_FIRST_CHANCE_NPT	20		// if a stream should have been started less than 20 seconds, will just start from the beginning

#define DEFAULT_LAST_CHANCE_NPT		10			// if a stream has less than this value left, will NOT start it from middle

#define	DEFAULT_SLEEPTIME		3000		// 3 seconds

#define UNKNOWN_UID				0xFFFFFFFF

#define	DEFAULT_RETRYTIMES		3

// playlist db mirror default path
#define DB_DEFAULT_PATH			("D:\\ITVPLAYBCK\\DB\\")
#define DB_FILE_EXT				("iel")
#define DB_CONF_NAME			("config.cfg")

//////////////////////////////////////////////////////////////////////////
// macros for STVList class

// indicate the expiration status of a list
#define LISTSTAT_PREMATURE		0		// not effective yet
#define	LISTSTAT_EFFECTIVE		1		// is effective now
#define LISTSTAT_LEISURE		2		// is idle between IEs
#define	LISTSTAT_EXPIRED		3		// has expired

// how much time should be held in advance when calculating list effective/start time
#define	LISTTIME_CRITPLAY		1

//////////////////////////////////////////////////////////////////////////
// XML file key words

// -----------------------------------------------------------
/*
db mirror keys
eg:

<DBMirrorList>
<Port ChannelID="1" SubChannelID="1" DeviceID="1" PortID="1" IPAdress="225.12.12.126" IPPort="257"/>
	<FillerList ListNumber="3" PlayMode="R" EffectiveDatetime="20040501 00:00:00" EndDatetime="20041031 23:59:59">
		<Asset NO="1" ID="50" Begin="20040501 07:00:00" Duration="100">
			<Element NO="2" ElementID="501" FileName="aa" Begin="20040501 07:00:00" Duration="1800" Xin="1" Xout="2" CueIn="00:01:30:100" CueOut="10:00:30:100" CueDuration="10:28:30:100" BitRate="3072"/>
		</Asset>
	</FillerList>
</DBMirrorList>

 <DBConfiguration><Configuration Type="2">
		<Port ChannelID="4" DeviceID="1" IPAddress="117.0.0.20" IPPort="80"/>
		<Channel ChannelID="5">
			<Port SubChannelID="1" DeviceID="1" IPAddress="117.0.0.10" IPPort="80"/>
			<Port SubChannelID="2" DeviceID="1" IPAddress="117.0.0.11" IPPort="80"/>
		</Channel>
	</Configuration>
</DBConfiguration>
*/
#define TAG_DBMIRROR		"DBMirrorList"
#define TAG_DBCONFIG		"DBConfiguration"

// -----------------------------------------------------------
/*
connect info
eg:

<ScheduleInformation>
	<Connect Flag="1" />
</ScheduleInformation>
*/
#define TAG_CONNECT			"Connect"
#define KEY_CONNECT_FLAG	"Flag"

// -----------------------------------------------------------
/*
channel info
eg:

<Port ChannelID="1" NodeGroupID="1" IPAddress="10.0.0.1" IPPort="1234"/>
<Port ChannelID="2" NodeGroupID="1" IPAddress="10.0.0.2" IPPort="1234"/>
<Channel ChannelID="3">
	<Port SubChannelID="1" NodeGroupID="2" IPAddress="10.0.0.3" IPPort="1234"/>
	<Port SubChannelID="2" NodeGroupID="2" IPAddress="10.0.0.3" IPPort="1235"/>
	<Port SubChannelID="3" NodeGroupID="2" IPAddress="10.0.0.3" IPPort="1236"/>
</Channel>
*/
#define TAG_CHANNEL						"Channel"
#define	TAG_PORT						"Port"
#define KEY_PORT_CHANNELID				"ChannelID"
#define KEY_PORT_SUBCHANNELID			"SubChannelID"
#define KEY_PORT_DEVICEID				"DeviceID"
#define KEY_PORT_PORTID					"PortID"
#define KEY_PORT_IPADRESS				"IPAddress"
#define KEY_PORT_IPPORT					"IPPort"

// -----------------------------------------------------------
/*
status feedback info
eg:

<StatusInformation>
	<Port ChannelID="1" SubChannelID="1" DeviceID="1" PortID="1" IPAdress="10.0.1.2" IPPort="12345" DeviceControllerID="1"/>
	<Event ID="1" Time="20041102 20:04:48" Status="1" ErrorCode="0"/>
	<Asset NO="41101122" AssetID="102" isFiller="1">
		<Element NO="41101123" ElementID="111"/>
	</Asset>                                                                                                                                                                                                                          
</StatusInformation>

*/
#define TAG_STATUSINFO					"StatusInformation"

#define TAG_STATUSINFO_EVENT			"Event"
#define KEY_STATUSINFO_EVENT_ID			"ID"
#define KEY_STATUSINFO_EVENT_TIME		"Time"
#define KEY_STATUSINFO_EVENT_STATUS		"Status"
#define KEY_STATUSINFO_EVENT_ERRORCODE	"ErrorCode"

#define TAG_STATUSINFO_ASSET			"Asset"
#define KEY_STATUSINFO_ASSET_NO			"NO"
#define KEY_STATUSINFO_ASSET_ASSETID	"AssetID"
#define KEY_STATUSINFO_ASSET_ISFILLER	"isFiller"

#define TAG_STATUSINFO_ASSET_ELE		"Element"
#define KEY_STATUSINFO_ASSET_ELE_NO		"NO"
#define KEY_STATUSINFO_ASSET_ELE_ELEID	"ElementID"

// -----------------------------------------------------------
/*
schedule info
eg:

<ScheduleInformation>
	<Schedule Date="20040501 07:00:00" Type="P">
		<Port ChannelID="1" SubChannelID="1" DeviceID="1" PortID="1" IPAdress="10.0.1.2" IPPort="12345"/>
		<PlayList ListNumber="5" ElementNumber="6">
			<Asset NO="1" ID="10" Type="I" Begin="20040501 07:00:00" Duration="1800">
				<Element NO="2" ElementID="111" Type="I" FileName="aa" Begin="20040501 07:00:00" 
					Duration="1800" Xin="1" Xout="2" CueIn="00:01:30:100" CueOut="10:00:30:100" CueDuration="10:28:30:100" BitRate="3072"/>
				<Element NO="3" ElementID="122" Type="P" FileName="bb" Begin="20040501 07:00:00" 
					Duration="1800" Xin="1" Xout="2" CueIn="00:01:30:100" CueOut="10:00:30:100" CueDuration="10:28:30:100" BitRate="3072"/>
			</Asset>
			<Asset NO="4" ID="102" Type="P" Begin="20040501 07:10:00" Duration="1000">
				<Element NO="5" ElementID="1331" Type="P" FileName="bb" Begin="20040501 07:10:00" 
					Duration="1800" Xin="1" Xout="2" CueIn="00:01:30:100" CueOut="10:00:30:100" CueDuration="10:28:30:100" BitRate="3072"/>
			</Asset>
			<Asset NO="6" ID="1012" Type="P" Begin="20040501 07:40:00" Duration="2400">
				<Element NO="7" ElementID="3311" Type="P" FileName="cc" Begin="20040501 07:40:00" 
					Duration="1800" Xin="1" Xout="2" CueIn="00:01:30:100" CueOut="10:00:30:100" CueDuration="10:28:30:100" BitRate="3072"/>
			</Asset>
			<Asset NO="8" ID="1023" Type="U" Begin="20040501 08:30:00" Duration="3600">
				<Element NO="9" ElementID="2222" Type="U" FileName="dd" Begin="20040501 08:30:00" 
					Duration="1800" Xin="1" Xout="2" CueIn="00:01:30:100" CueOut="10:00:30:100" CueDuration="10:28:30:100" BitRate="3072"/>
			</Asset>
			<Asset NO="10" ID="1032" Type="P" Begin="20040501 09:10:00" Duration="1800">
				<Element NO="11" ElementID="343" Type="P" FileName="ee" Begin="20040501 09:10:00" 
					Duration="1800" Xin="1" Xout="2" CueIn="00:01:30:100" CueOut="10:00:30:100" CueDuration="10:28:30:100" BitRate="3072"/>
			</Asset>
		</PlayList>
	</Schedule>
</ScheduleInformation>

*/

// -- schedule info
#define TAG_SCHEDULEINFO				"ScheduleInformation"
#define TAG_STATUS						"Status"
#define TAG_STATUS_CURRENT				"Current"
#define KEY_STATUS_CURRENT_STATE		"State"

// -- schedule
#define TAG_SCHEDULE					"Schedule"
#define KEY_SCHEDULE_DATE				"Date"
#define KEY_SCHEDULE_TYPE				"Type"
#define KEY_SCHEDULE_ID					"ID"
#define KEY_SCHEDULE_SEQ				"Sequence"

// -- list info
#define TAG_PLAYLIST					"PlayList"
#define TAG_FILLERLIST					"FillerList"

#define KEY_LIST_LISTNUMBER				"ListNumber"
#define KEY_LIST_ELEMENTNUMBER			"ElementNumber"
#define KEY_LIST_PLAYMODE				"PlayMode"
#define KEY_LIST_EFFECTTIME				"EffectiveDatetime"
#define KEY_LIST_ENDTIME				"EndDatetime"

// -- asset info
#define TAG_LIST_ASSET					"Asset"
#define KEY_LIST_ASSET_NO				"NO"
#define KEY_LIST_ASSET_ID				"ID"
#define KEY_LIST_ASSET_TYPE				"Type"
#define KEY_LIST_ASSET_BEGIN			"Begin"
#define KEY_LIST_ASSET_TIME				"Time"
#define KEY_LIST_ASSET_ISFILLER			"isFiller"
#define KEY_LIST_ASSET_DURATION			"Duration"
#define KEY_LIST_ASSET_PRIORITY			"Priority"
#define KEY_LIST_ASSET_AENUMBER			"AeNumber"

// -- element info	
#define TAG_LIST_ASSET_AE				"Element"
#define KEY_LIST_ASSET_AE_NO			"NO"
#define KEY_LIST_ASSET_AE_ID			"ElementID"
#define KEY_LIST_ASSET_AE_DURATION		"Duration"
#define KEY_LIST_ASSET_AE_BITRATE		"BitRate"
#define KEY_LIST_ASSET_AE_CUEIN			"CueIn"
#define KEY_LIST_ASSET_AE_CUEOUT		"CueOut"
#define KEY_LIST_ASSET_AE_CUEDURATION	"CueDuration"
#define KEY_LIST_ASSET_AE_EVENT			"Event"
#define KEY_LIST_ASSET_AE_STATUS		"Status"

//////////////////////////////////////////////////////////////////////////

#endif	// _STVLIST_DEF_H