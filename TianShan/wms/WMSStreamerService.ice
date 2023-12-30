#include "TsStreamer.ice"

module TianShanIce
{

sequence<string> PlaylistItems;
sequence<string> PublishingPointNames;

enum PublishingPointType {
	OnDemandPublishingPoint, 
	BroadcastPublishingPoint, 
};

exception AlreadyExist extends TianShanIce::BaseException {

};

class WMSStreamerService extends Streamer::StreamService {

	Streamer::Playlist* createPublishingPoint(string pubPtName, 
		PlaylistItems items, PublishingPointType type, 
		string destination, out string reloc) 
		throws AlreadyExist;

	Streamer::Playlist* openPublishingPoint(string pubPtName)
		throws NotFound;

	bool deletePublishingPoint(string pubPtName)
		throws NotFound;

	PublishingPointNames listPublishingPoints();

	bool getAllowClinetsToConnect();
	bool setAllowClinetsToConnect(bool allow);
};

};
