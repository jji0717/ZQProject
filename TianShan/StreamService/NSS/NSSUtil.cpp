#include "NSSUtil.h"

TianShanIce::Streamer::StreamState convertState(std::string state)
{
	std::transform(state.begin(), state.end(), state.begin(), tolower);
	if (state.compare("play") == 0)
	{
		return TianShanIce::Streamer::stsStreaming;
	}
	else if (state.compare("ready") == 0)
	{
		return TianShanIce::Streamer::stsStop;
	}
	else if (state.compare("init") == 0)
	{
		return TianShanIce::Streamer::stsSetup;
	}
	else if (state.compare("pause") == 0)
	{
		return TianShanIce::Streamer::stsPause;
	}
	return TianShanIce::Streamer::stsSetup;
}
