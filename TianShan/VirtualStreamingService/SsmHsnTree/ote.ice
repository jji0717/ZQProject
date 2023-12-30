module com {
	module izq {
		module ote {
			module tianshan {		
                               
				dictionary <string, string> Properties;
					
				struct SessionData{
				 	string serverSessionId;  // the RTSP server session ID, required
            				string clientSessionId;  // the RTSP client session ID, required
            				Properties params;       // all remaining parameters wrapped in a HashMap like structure
					 			 // String home-id(varchar(20))	
								 // String smartcard-id(varchar(20))	
								 // String device-id (mac address, varchar(20) - The mac address is typically passed without the colons. Example: 00FF414BABD5)
								 // String ViewBeginTime(YYYY-MM-DD HH:MM:SS)
								 // String application-id (60010001 or 60010002)	
								 // String purchase-id(varchar(20))
								 // String asset-id (hex 8digital; without prefix(0x) )	
								 // String provider-id(varchar(20))	
								 // String provider-asset-id(varchar(20))	

				}; 
										
				struct SessionResultData{
					
					string status;	  // the result of session setup or teardown;
							  // success or fail
								
					string errorCode; // the reason of fail
							  // success is null
							  
					int rentalDuration; // The rental duration of the current ticket
				};
				
				interface MoDIceInterface {
					
					SessionResultData sessionSetup(SessionData sd);
					["ami"] SessionResultData sessionTeardown(SessionData sd);
											
				};
			};
		};
	};
};
