#ifndef __ZQ_DATAONDEMAND_DATASTREAMEX_ICE__
#define __ZQ_DATAONDEMAND_DATASTREAMEX_ICE__

#include "DataStream.ice"

/// namespace of DataOnDemand
module DataOnDemand {

class DataStreamEx;

// -----------------------------
// class MuxItemEx
// -----------------------------
/// mux item class
class MuxItemEx extends MuxItem {

	void activate();
	
	MuxItemInfo				myInfo;
	DataStreamEx*			myParent;
	TianShanIce::Properties	myProps;
};


dictionary<string, MuxItemEx* > MuxItemExDict;
class DataStreamServiceEx;

// -----------------------------
// class DataStreamEx
// -----------------------------
/// data stream class
class DataStreamEx extends DataStream {

	void removeMuxItem(string name);
	void activate();
	
	StreamInfo							myInfo;
	DataStreamServiceEx*				myParent;
	MuxItemExDict						myMuxItems;
	TianShanIce::Properties				myProps;
	TianShanIce::Streamer::StreamState	myState;
	string								mySpace;
};


dictionary<Ice::Identity, DataStreamEx* > DataStreamExDict;

// -----------------------------
// service DataStreamServiceEx
// -----------------------------
/// dod stream service
class DataStreamServiceEx extends DataStreamService {

	void removeStream(string space, string name);
	void activate();
	void checkTimeout(int msec);
	
	DataStreamExDict		myStreams;
	TianShanIce::Properties	myProps;
};

}; /// module DataOnDemand

#endif // __ZQ_DATAONDEMAND_DATASTREAMEX_ICE__
