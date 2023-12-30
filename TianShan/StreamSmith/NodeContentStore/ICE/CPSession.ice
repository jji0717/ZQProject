#ifndef __ZQ_TianShanIce_CPSession_ICE__
#define __ZQ_TianShanIce_CPSession_ICE__

#include "TianShanIce.ICE"

module TianShanIce {
module Storage {
module ContentProcess {


enum SessionStatus {SETUP, PROCESSING, FAILED, SUCCEEDED, EXPIRED};

struct CPSession {
	string type;
	string netID;

	SessionStatus status;
	string contentName;  
	string destType;     
	string sourceURL;         
	int startTime;         
	int endTime;           
	int bitrate;           
	Properties additionalInfo;
};

};
};
};

#endif