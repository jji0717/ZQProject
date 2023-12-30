#include "StdAfx.h"
#include "GridHandler.h"
#include "ChannelOnDemandClient.h"

static
struct {
    const char *_name;
    GridHandler _func;
}
_GridHandlerTbl[] =
{
    {GridName_ChOD_ChannelPublisher,    ChannelOnDemandClient::ChODPublisherHandler},
    {GridName_ChOD_ChannelPublishPoint, ChannelOnDemandClient::ChODPublishPointHandler}
};

GridHandler GetGridHandler(const char *gridname)
{
    if(NULL == gridname)
    {
        return NULL;
    }
    GridHandler gh = NULL;
    for(size_t i = 0; i < (sizeof(_GridHandlerTbl) / sizeof(_GridHandlerTbl[0])); ++i)
    {
        if(0 == strcmp(gridname, _GridHandlerTbl[i]._name))
        {
            gh = _GridHandlerTbl[i]._func;
            break;
        }
    }
    return gh;
}