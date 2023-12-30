
// the ICE interface is for the integration of Surf with Tianshan.
// when STB session setup for surfplaylist, Tianshan queries Surf to get AE list.

module com {
	module izq {
		module surf {
			module integration {
				module tianshan {

					struct AEInfo {
						string aeUID;			// the uid of asset element, Hex format, 8 bits and capital
						int    bandWidth;		// bandWidth, is in bps (bit per second)
						int    cueIn;			// When it is -1, means no cue in point, CueIn is in millisecond
						int    cueOut;			// When it is -1, means no cue out point, CueOut is in millisecond
					};

					// the collection of AE is a sequence or vector
					sequence<AEInfo> AssetElementCollection;
					
					// the collection of net_id is a sequence or vector
					sequence<string> NetIDCollection;

					struct AEReturnData {
						NetIDCollection netIDList; // the uid of net_id, optional
						AssetElementCollection aeList; // AE list
					};

					exception SurfException {
					
						string errorCode;		// TBD, required;
						string errorDescription;	// TBD, optional;
					};

			                interface SurfForTianshan {
			        	        // get AE list by surfPlaylistID, surfPlaylistID is string
			        	        AEReturnData getAEList(string surfPlaylistID) throws SurfException;
			            
			                };
				};
	    	        };
		};
	};
};
