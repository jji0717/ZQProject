module com {
  module izq {
	module todas {
     module integration {		
         module cod {
	
            dictionary <string, string> Properties;

	        struct SessionData {

	           string serverSessionId; 	// the RTSP server session ID, required
               string clientSessionId; 	// the RTSP client session ID, required
               Properties params;           // all remaining parameters wrapped in a HashMap like structure
	        }; 
					
	        struct SessionResultData {  	// not used by COD

	           int status;	 	        // not used at the moment - will always return success
	 	                                // the result of session setup or teardown;
		         		                // 1: success
                          		        // 2: error 
		       string errorCode;	    // not used at the moment 
			     	                    // 1: given time not in pause TV window
			     	                    // 2: no items on the given channel
			     	                    // 3: other
	       };

		   struct DeviceData {		// currently includes smardcardId, macAddress. it may add other ID later
		 
             string smardcardId;	// smartcard ID of STB
		     string macAddress;	    // MAC of STB
	       };

           struct BookmarkData {

		     string homeId;		     // home id, required
	         DeviceData device;	     // device data to include smardcardId, macAddress
		     string channelId;	     // Channel ID, required  
		     string assetId;		 // the asset id as hexadecimal, padded with 0 up to 8 characters; optional
		     string elementId;	     // the element id as hexadecimal, padded with 0 up to 8 characters; optional
		     string broadcastTime;   // The broadcast time, required. 
			                         // Format: 20061010T203059, local time in 24H.
		     float npt;		         // the NPT on the element, optional
		     string reasonCode;	     // the reasonCode for saving bookmark, optional
	       };

           struct BookmarkResultData {

		     int status;		     // the result of saving bookmark;
								     // 1: success
								     // 2: error
			 string errorCode;	     // 0: internal system error
								     // 1: given time not in pause TV window
								     // 2: non pause tv channel
								     // 3: bookmark cant be set: 
                                     //    either below gap from another bookmark or 
                                     //    the max number is reached
		   };
		
	       exception TodasException {

	         string errorCode;		    // TBD, required;
	         string errorDescription;	// TBD, optional;
	       };
					
	       interface TodasForCod {
	          SessionResultData sessionSetup(SessionData sd) throws TodasException;
		     ["ami"] SessionResultData sessionTeardown(SessionData sd) throws TodasException;
              BookmarkResultData saveBookmark(BookmarkData bd) throws TodasException;
	       };
         };
      };
    };
  };
};