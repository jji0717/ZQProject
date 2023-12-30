module com {
module izq {
	module ads {
	module tianshan {	
		
sequence <string> StringCollection;
  
dictionary <string, string> AttributesMap;
  	
dictionary <string, string> Properties;

	struct SessionData{
	        string serverSessionId; 	// the RTSP server session ID, required
            string clientSessionId; 	// the RTSP client session ID, required
            Properties params;           // all remaining parameters wrapped in a HashMap like structure
	}; 
	
  struct AEInfo3 {
    string           name;       //only content name, without volume name
    int              bandWidth;  // bandWidth
    int              cueIn;
    int              cueOut;
    StringCollection nasUrls;
    StringCollection volumeList;
    AttributesMap    attributes; //for extending purpose   
  };
  sequence<AEInfo3> AEInfo3Collection;
  //~
	exception ADSException {
	string errorCode;		// TBD, required;
	string errorDescription;	// TBD, optional;
	};
					
	interface ADSForTianshan {
	AEInfo3Collection PlacementReqeust(SessionData sd, AEInfo3Collection origianlAEList) throws ADSException ;
	};
};
};
};
};
