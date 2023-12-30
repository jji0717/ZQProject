#include <string>
#include "TsStorage.h"

const std::string UNKNOWN = "Unknown";
const std::string PENDING = "Pending";
const std::string TRANSFER = "Transfer";
const std::string STREAMABLE = "Transfer/Play";
const std::string COMPLETE = "Complete";
const std::string CANCELED = "Canceled";
const std::string FAILED = "Failed";

//convert ContentState to string 
std::string convertState(::TianShanIce::Storage::ContentState contentState);

//convert string to ContentState 
::TianShanIce::Storage::ContentState convertState(std::string contentState);

/// convert TianShanIce::Storage::ContentState to TianShanIce::State
TianShanIce::State ContentStateToState(TianShanIce::Storage::ContentState contentState);

