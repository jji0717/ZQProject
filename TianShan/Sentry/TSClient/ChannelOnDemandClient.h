#ifndef __CHANNELONDEMANDCLIENT_H__
#define __CHANNELONDEMANDCLIENT_H__

#include <TsLayout.h>

namespace ChannelOnDemandClient
{

int ChODPublisherHandler(::ZQTianShan::Layout::PLayoutCtx ctx);
int ChODPublishPointHandler(::ZQTianShan::Layout::PLayoutCtx ctx);
#define GridName_ChOD_ChannelPublisher     "ChOD.ChannelPublisher"
#define GridName_ChOD_ChannelPublishPoint  "ChOD.ChannelPublishPoint"
}
#endif