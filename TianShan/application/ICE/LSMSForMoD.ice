module com {
	module izq {
		module lsms {
		module integration{
 			module mod{ 
			dictionary <string, string> Properties;
			struct SessionData{
			string serverSessionId;  // the RTSP server session ID, required
            		string clientSessionId;  // the RTSP client session ID, required
                                Properties params;  // all remaining parameters wrapped in a HashMap like structure
			}; 
			struct SessionResultData{
				string status;  // the result of session setup or teardown; success or fail
				string errorCode; // the reason of fail. success is null
			};
			interface LSMSForMoD {
				SessionResultData sessionSetup(SessionData sd);
				["ami"] SessionResultData sessionTeardown(SessionData sd);	
			};
		};
		};
	};
};
};
