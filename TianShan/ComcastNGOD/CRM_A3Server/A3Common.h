// FileName : A3Ccommon.h
// Author   : Zheng Junming
// Date     : 2009-05
// Desc     : common functions and constants

#ifndef __CRG_PLUGIN_A3SERVER_A3COMMON_H__
#define __CRG_PLUGIN_A3SERVER_A3COMMON_H__

#include <string>
#include "TsStorage.h"

const std::string XML_HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";

// A3 Content State
const std::string UNKNOWN = "Unknown";
const std::string PENDING = "Pending";
const std::string TRANSFER = "Transfer";
const std::string STREAMABLE = "Transfer/Play";
const std::string COMPLETE = "Complete";
const std::string CANCELED = "Canceled";
const std::string FAILED = "Failed";

/// convert contentState to A3 Content state
std::string convertState(::TianShanIce::Storage::ContentState contentState);

/// convert content event state to A3 Content state
std::string eventStateToA3State(std::string strEventContentState);

/// convert TianShanIce::Storage::ContentState to TianShanIce::State
TianShanIce::State ContentStateToState(TianShanIce::Storage::ContentState contentState);

/// generate current UTC time
std::string GenerateUTCTime();

typedef std::map<std::string, std::string> StringMap;
typedef std::vector<std::string> StringVector;

#endif

