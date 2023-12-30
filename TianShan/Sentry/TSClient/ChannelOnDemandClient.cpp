#include "StdAfx.h"
#include <Ice/Ice.h>
#include "filelog.h"
#include "ChannelOnDemand.h"
#include "ChannelOnDemandClient.h"
#include <urlstr.h>

#define LOG_MODULE_NAME     "ChODClient"

#define GRID_GRIDNAME       "grid#gridname"
#define GRID_ENDPOINT       "grid#endpoint"
#define GRID_CHANNELNAME    "grid#channelname"

extern DWORD _iLogSlot; //tls index
#define LOGPTR          ((ZQ::common::Log*)(TlsGetValue(_iLogSlot)))
#define	TSLOG           (*LOGPTR)

namespace ChannelOnDemandClient
{
using namespace ZQTianShan::Layout;
using namespace ZQ::common;

static std::string int2str(int i)
{
    char buf[12] = {0};
    return itoa(i, buf, 10);
}
static std::string bool2str(bool b)
{
    return b ? "true" : "false";
}
static std::string long2str(__int64 l)
{
    char buf[22] = {0};
    return _i64toa(l, buf, 10);
}
int ChODPublisherHandler(::ZQTianShan::Layout::PLayoutCtx ctx)
{
    if(NULL == ctx)
    {
        return 0;
    }

    ILayoutCtx::lvalue val = {0};
    if(!ctx->get(GRID_ENDPOINT, val))
    {
        return 0;
    }
    std::string endpoint = val.value;
    std::string prxstr = std::string("ChannelPublisher:") + endpoint;
    const char* colnames[] = {
        "name",
        "desc",
        "maxBitrate"
    };
    if(!ctx->setColumns(colnames, sizeof(colnames) / sizeof(colnames[0])))
    {
        return 0;
    }

    int nRows = 0;
    Ice::CommunicatorPtr ic;
    try{
        int argc = 0;
        ic = Ice::initialize(argc, NULL);
        Ice::ObjectPrx prx = ic->stringToProxy(prxstr);
        ChannelOnDemand::ChannelPublisherPrx cp = ChannelOnDemand::ChannelPublisherPrx::uncheckedCast(prx);
        ::TianShanIce::StrValues channels = cp->list();

        ::TianShanIce::StrValues::iterator it;
        for(it = channels.begin(); it != channels.end(); ++it)
        {
            std::string chname = (*it);
            ChannelOnDemand::ChannelPublishPointPrx cpp = cp->open(chname);
            std::string chdesc = cpp->getDesc();
            std::string chmaxbitrate = int2str(cpp->getMaxBitrate());
            const char* rowval[] = 
            {
                chname.c_str(),
                chdesc.c_str(),
                chmaxbitrate.c_str()
            };
            ZQ::common::URLStr ref("http://./?", true);//always case sensitive
            ref.setVar(GRID_GRIDNAME, GridName_ChOD_ChannelPublishPoint);
            ref.setVar(GRID_ENDPOINT, endpoint.c_str());
            ref.setVar(GRID_CHANNELNAME, chname.c_str());

            if(ctx->addRow(rowval, ref.generate()))
            {
                ++nRows;
            }
        }

    }
    catch (const Ice::Exception& ex)
    {
        TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "catch Ice::Exception [%s]"), ex.ice_name().c_str());
    }
    catch (const char* msg)
    {
        TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "catch exception [%s]"), msg);
    }
    catch(...)
    {
        TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encounter unknown exception."));
    }

    if(ic)
    {
        ic->destroy();
    }
    return nRows;
}
int ChODPublishPointHandler(::ZQTianShan::Layout::PLayoutCtx ctx)
{
    if(NULL == ctx)
    {
        return 0;
    }

    ILayoutCtx::lvalue val = {0};
    if(!ctx->get(GRID_ENDPOINT, val))
    {
        return 0;
    }
    std::string prxstr = std::string("ChannelPublisher:") + val.value;
    if(!ctx->get(GRID_CHANNELNAME, val))
    {
        return 0;
    }
    std::string chname = val.value;

    const char* colnames[] = {
        "contentName",
        "broadcastStart",
        "expiration",
        "playable",
        "forceNormalSpeed",
        "inTimeOffset",
        "outTimeOffset",
        "spliceIn",
        "spliceOut"
    };
    if(!ctx->setColumns(colnames, sizeof(colnames) / sizeof(colnames[0])))
    {
        return 0;
    }

    int nRows = 0;
    Ice::CommunicatorPtr ic;
    try{
        int argc = 0;
        ic = Ice::initialize(argc, NULL);
        Ice::ObjectPrx prx = ic->stringToProxy(prxstr);
        ChannelOnDemand::ChannelPublisherPrx cp = ChannelOnDemand::ChannelPublisherPrx::uncheckedCast(prx);
        ChannelOnDemand::ChannelPublishPointPrx cpp = cp->open(chname);
        ::TianShanIce::StrValues items = cpp->getItemSequence();
        ::TianShanIce::StrValues::iterator it;
        for(it = items.begin(); it != items.end(); ++it)
        {
            ChannelOnDemand::ChannelItem item = cpp->findItem(*it);
            std::string playable = bool2str(item.playable);
            std::string forceNormalSpeed = bool2str(item.forceNormalSpeed);
            std::string inTimeOffset = long2str(item.inTimeOffset);
            std::string outTimeOffset = long2str(item.outTimeOffset);
            std::string spliceIn = bool2str(item.spliceIn);
            std::string spliceOut = bool2str(item.spliceOut);

            const char* rowval[] = 
            {
                item.contentName.c_str(),
                item.broadcastStart.c_str(),
                item.expiration.c_str(),
                playable.c_str(),
                forceNormalSpeed.c_str(),
                inTimeOffset.c_str(),
                outTimeOffset.c_str(),
                spliceIn.c_str(),
                spliceOut.c_str()
            };
            if(ctx->addRow(rowval))
            {
                ++nRows;
            }
        }
    }
    catch (const Ice::Exception& ex)
    {
        TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "catch Ice::Exception [%s]"), ex.ice_name().c_str());
    }
    catch (const char* msg)
    {
        TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "catch exception [%s]"), msg);
    }
    catch(...)
    {
        TSLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "encounter unknown exception."));
    }

    if(ic)
    {
        ic->destroy();
    }
    return nRows;
}

}