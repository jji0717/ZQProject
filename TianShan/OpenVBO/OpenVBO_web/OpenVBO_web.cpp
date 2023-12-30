// OpenVBO.cpp : Defines the entry point for the DLL application.
//
#include <ZQ_common_conf.h>
#include "OpenVBOClient.h"
#include <Log.h>
#include <httpdInterface.h>
#include <Locks.h>
#include <strHelper.h>

#define LOG (*gData.log)

struct GData{
    ZQ::common::Log* log;
    Ice::CommunicatorPtr comm;
    ZQ::common::Mutex lock;
    bool init(ZQ::common::Log* pLog) {
        if(log && comm) {
            return true;
        } else {
            comm = NULL;
            log = NULL;
        }

        if(NULL == pLog)
            return false;
        log = pLog;
        try { // init the communicator
            Ice::InitializationData initData;
            comm = Ice::initialize(initData);
            return true;
        } catch (const Ice::Exception& e) {
            (*log)(ZQ::common::Log::L_WARNING, CLOGFMT(OpenVBO_web, "init() Got %s when initialize the communicator."), e.ice_name().c_str());
            return false;
        }
    }
    bool safeInit(ZQ::common::Log* pLog) {
        ZQ::common::MutexGuard guard(lock);
        return init(pLog);
    }
    void clear() {
        comm = NULL;
        log = NULL;
    }
    void safeClear() {
        ZQ::common::MutexGuard guard(lock);
        clear();
    }
} gData;

#ifdef _MANAGED
#pragma managed(push, off)
#endif

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        gData.safeClear();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        gData.safeClear();
        break;
    }
    return TRUE;
}
#endif// ZQ_OS_MSWIN

#ifdef _MANAGED
#pragma managed(pop)
#endif

extern "C"
{
    // lib init func
    __EXPORT bool LibInit(ZQ::common::Log* pLog)
    {
        return gData.safeInit(pLog);
    }
    // lib uninit func
    __EXPORT void LibUninit( )
    {
    }
    // your page
    __EXPORT bool Show(IHttpRequestCtx *ctx)
    {
        ZQ::common::MutexGuard guard(gData.lock);
        // implement you page here
        if(NULL == ctx)
            return false;
        IHttpResponse& out = ctx->Response();
        const char* ep = ctx->GetRequestVar("ep");
        if(NULL == ep || '\0' == ep) {
            out.SetLastError("No endpoint provided");
            return false;
        }

        const char* tmpl = ctx->GetRequestVar("#template");
        if(NULL == tmpl || '\0' == tmpl) {
            out.SetLastError("No layout file provided");
            return false;
        }

        OpenVBOClient client(*gData.log, gData.comm);
        if(!client.connect(ep)) {
            out.SetLastError((std::string("Failed to connect ") + ep).c_str());
            return false;
        }
        std::string successPhrase;
        const char* action = ctx->GetRequestVar("action");
        if(NULL == action) {
            // just show the page
        } else if (0 == strcmp(action, "enable") || 0 == strcmp(action, "disable")) {
            const char* streamers = ctx->GetRequestVar("streamers");
            if(NULL == streamers || '\0' == *streamers) {
                out.SetLastError((std::string("No streamers provided when ") + action + " streamers on " + ep).c_str());
                return false;
            }
            TianShanIce::StrValues streamerList;
            ZQ::common::stringHelper::SplitString(streamers, streamerList, ",");
            bool enabled = (0 == strcmp(action, "enable"));
            if(client.enableStreamers(streamerList, enabled)) {
                successPhrase = (enabled ? "Enable" : "Disable");
                successPhrase += " streamers OK. Streamers: ";
                successPhrase += streamers;
            } else {
                out.SetLastError((std::string("Failed to ") + action + " streamers on " + ep).c_str());
                return false;
            }
        } else if (0 == strcmp(action, "reset")) {
            if(client.resetCounters()) {
                successPhrase = "Reset counters OK.";
            } else {
                out.SetLastError((std::string("Failed to reset counters on ") + ep).c_str());
                return false;
            }
        } // else: show the page
        if(!successPhrase.empty()) {
            out << "<script type=\"text/javascript\">\n"
                << "<!--\n";
            out << "var toolbar = new Toolbar();\n"
                << "var iconBack = new Icon();\n"
                << "iconBack.setImageSrc('images/toolbar/back.gif');\n"
                << "iconBack.setImageHoverSrc('images/toolbar/back_over.gif');\n"
                << "iconBack.setLabel('Back');\n"
                << "iconBack.setAction(\"document.getElementById('backform').submit()\");\n"
                << "iconBack.setTips('show the current status');\n";
            out << "toolbar.addIcon(iconBack);\n";
            out << "document.getElementById('toolbar').innerHTML = toolbar.build();\n";
            out << "//-->\n";
            out << "</script>";
            out << successPhrase;
            out << "<form action=\"Show.ovbo.tswl\" method=\"get\" id=\"backform\">\n"
                << "<input type=\"hidden\" name=\"ep\" value=\"" << ep << "\">\n"
                << "<input type=\"hidden\" name=\"#template\" value=\"" << tmpl << "\">\n"
                << "</form>";
            return true;
        }

        // show the current states
        StreamersInfo streamersInfo;
        if(!client.getStreamers(streamersInfo)) {
            out.SetLastError((std::string("Failed to get streamers info from ") + ep).c_str());
            return false;
        }
        ImportChannelsInfo icInfo;
        if(!client.getImportChannels(icInfo)) {
            out.SetLastError((std::string("Failed to get import channel info from ") + ep).c_str());
            return false;
        }
        // compute the summary info
        Ice::Long totalSessions = 0;
        Ice::Long totalLocalSessions = 0;
        double hitrate;
        {
            SsmOpenVBO::StreamersStatistics::const_iterator it;
            for(it = streamersInfo.streamersInfos.begin(); it != streamersInfo.streamersInfos.end(); ++it) {
                totalSessions += (it->localSessions + it->remoteSessions);
                totalLocalSessions += it->localSessions;
            }
            if(totalSessions != 0) {
                hitrate = (double)totalLocalSessions / (double)totalSessions;
            }
        }
        // summary
        out << "<table class=\"listTable\">\n"
            << "<tr style=\"text-align:center\"><th colspan=\"3\">Summary</th></tr>\n"
            << "<tr><th colspan=\"2\">SSM_OpenVBO</th><td>" << ep << "</td></tr>\n"
            << "<tr><th rowspan=\"4\" style=\"vertical-align:middle\">Session<br>Stat.</th><th>total</th><td>" << totalSessions << "</td></tr>\n"
            << "<tr><th>local</th><td>" << totalLocalSessions << "</td></tr>\n";
        char hrBuf[16];
        sprintf(hrBuf, "%.1f%%", hitrate * 100);
        out << "<tr><th>hitrate</th><td>" << hrBuf << "</td></tr>\n"
            << "<tr><th>since</th><td>" << streamersInfo.stampMeasuredSince << "</td></tr>\n"
            << "</table>";

        out << "<script type=\"text/javascript\" src=\"tsweb.js\"></script>\n";
        out << "<script type=\"text/javascript\">\n"
            << "<!--\n";
        out << "function getStreamers() {\n"
            << "  return _Array(els(el('f'), 'input'))._Filter(_And(_With({'type':'checkbox','checked':true}), _WithOut({'id':'all'})))._Map(function(o){return o.value;}).join(',');\n"
            << "}\n";
        out << "function enableStreamers(enabled) {\n"
            << "  var streamers = getStreamers();\n"
            << "  if(streamers.length == 0) {\n"
            << "    alert('No streamers selected!'); return;\n"
            << "  }\n"
            << "  el('streamers').value = streamers;\n"
            << "  el('action').value = enabled ? 'enable' : 'disable';\n"
            << "  el('f').submit();\n"
            << "}\n";
        out << "function resetCounters() {\n"
            << "  el('action').value = 'reset';\n"
            << "  el('f').submit();\n"
            << "}\n";
        out << "function filterStreamers() {\n"
            << "  if(el('fval').value.length == 0) {\n"
            << "    alert('Need \\'' + el('fkey').value + '\\' values(divided by SPACE).');\n"
            << "    return;\n"
            << "  }\n;"
            << "  el('action').value = 'filter';\n"
            << "  el('fkey').name = 'fkey';\n"
            << "  el('fval').name = 'fval';\n"
            << "  el('f').submit();\n"
            << "}\n";
        out << "var toolbar = new Toolbar();\n"
            << "var iconReset = new Icon();\n"
            << "iconReset.setImageSrc('images/toolbar/reset.gif');\n"
            << "iconReset.setImageHoverSrc('images/toolbar/reset_over.gif');\n"
            << "iconReset.setLabel('Reset');\n"
            << "iconReset.setAction('resetCounters()');\n"
            << "iconReset.setTips('reset the counters');\n";
        out << "toolbar.addIcon(iconReset);\n";
        out << "el('toolbar').innerHTML = toolbar.build();\n";
        out << "//-->\n"
            << "</script>\n";

        out << "<form action=\"Show.ovbo.tswl\" method=\"get\" id=\"f\">\n";

        out << "<input type=\"hidden\" name=\"ep\" value=\"" << ep << "\">\n"
            << "<input type=\"hidden\" name=\"#template\" value=\"" << tmpl << "\">\n"
            << "<input type=\"hidden\" id=\"action\" name=\"action\" value=\"\">\n"
            << "<input type=\"hidden\" id=\"streamers\" name=\"streamers\" value=\"\">\n";
        // filter bar
        const char* fkey = ctx->GetRequestVar("fkey");
        enum {
            fUnknown,
            fSource,
            fNode,
            fStatus
        } eFKey = NULL == fkey ? fUnknown : 0 == strcmp(fkey, "source") ? fSource : 0 == strcmp(fkey, "node") ? fNode : 0 == strcmp(fkey, "status") ? fStatus : fUnknown;
        const char* fval = ctx->GetRequestVar("fval");
        out << "<select id=\"fkey\">"
            << "<option value=\"source\"" << (eFKey == fSource ? " selected" : "") << ">Source</option>"
            << "<option value=\"node\"" << (eFKey == fNode ? " selected" : "") << ">Node</option>"
            << "<option value=\"status\"" << (eFKey == fStatus ? " selected" : "") << ">Status</option>"
            << "</select>\n";
        out << "<input type=\"text\" id=\"fval\" size=\"50\" value=\"" << (eFKey != fUnknown ? fval != NULL ? fval : "" : "") << "\">"
            << "<input type=\"button\" value=\"Search\" onclick=\"filterStreamers()\">"
            << "<input type=\"checkbox\" onclick=\"_Array(els(el('f'), 'input'))._Filter(_With({'type':'checkbox'}))._Map(_Set({'checked':this.checked}))\" id=\"all\" style=\"width:20px\">"
            << "<span style=\"margin-right:5px\">all</span>"
            << "<input type=\"button\" value=\"Enable\" onclick=\"enableStreamers(true)\">\n"
            << "<input type=\"button\" value=\"Disable\" onclick=\"enableStreamers(false)\">\n";
            //<< "<input type=\"button\" value=\"Unmark\" onclick=\"_Array(els(el('f'), 'input'))._Filter(_With({'type':'checkbox'}))._Map(_Set({'checked':false}))\">\n";
        out << "<table class=\"listTable\">\n"
            << "<tr style=\"text-align:center\"><th colspan=\"14\">"
            << "<span>Streamers</span>"
            //<< "<span style=\"font-weight:lighter;font-style:italic;margin-left:5px\">"
            //<< "(Measured Since:" << streamersInfo.stampMeasuredSince << ")</span>"
            << "</th></tr>\n"
            << "<tr style=\"text-align:center\">"
            << "<th rowspan=\"2\">Source</th>"
            << "<th rowspan=\"2\" colspan=\"2\">StreamerNetId</th>"
            << "<th rowspan=\"2\">StreamService</th>"
            << "<th rowspan=\"2\">Volume</th>"
            << "<th rowspan=\"2\">Status</th>"
            << "<th rowspan=\"2\">Penalty</th>"
            << "<th rowspan=\"2\">Used/Failed/Err%.</th>"
            << "<th colspan=\"2\">Bandwidth Cap.</th>"
            << "<th colspan=\"2\">Stream Cap.</th>"
            << "<th colspan=\"2\">Session Stat.</th>"
            << "</tr>\n<tr style=\"text-align:center\">"
            << "<th>used</th><th>total</th>" // bandwidth
            << "<th>active</th><th>max</th>" // stream count
            << "<th>local</th><th>total</th>" // session
            << "</tr>\n";
        std::vector<std::string> fvals;
        if(fval != NULL && '\0' != *fval) {
            ZQ::common::stringHelper::SplitString(fval, fvals, " ");
        }
        SsmOpenVBO::StreamersStatistics::const_iterator it;
        for(it = streamersInfo.streamersInfos.begin(); it != streamersInfo.streamersInfos.end(); ++it) {
            bool bMatch = eFKey == fUnknown || fvals.empty();
            for(size_t i = 0; !bMatch && i < fvals.size(); ++i) {
                const std::string& val = fvals[i];
                if (eFKey == fSource) {
                    bMatch = val == it->streamerSource;
                } else if (eFKey == fNode) {
                    bMatch = val == it->streamerNetId.substr(0, it->streamerNetId.find('/'));
                } else if (eFKey == fStatus) {
                    bMatch = val == "avail" ? it->adminEnabled > 0 && it->available > 0 : val == "unavail" ? it->adminEnabled > 0 && it->available <= 0 : val == "disable" ? it->adminEnabled <= 0 : false;
                }
            }
            if (!bMatch) {
                continue;
            }
            out << "<tr style=\"text-align:center\">"
                << "<td>" << it->streamerSource << "</td>"
                << "<td>"
                << "<input type=\"checkbox\" onclick=\"el('all').checked=false\" value=\"" << it->streamerNetId << "\" style=\"width:20px\">"
                << "</td>"
                << "<td>" << it->streamerNetId << "</td>"
                << "<td>" << it->streamerEndpoint << "</td>"
                << "<td>" << it->attachedVolumeName << "</td>"
                // status
                << "<td>" << (it->adminEnabled > 0 ? (it->available > 0 ? "avail" : "unavail") : "disable") << "</td>"
                // penalty
                << "<td>" << it->penaltyValue << "</td>";

            uint64 totalSessions = it->usedSessions + it->failedSessions;
            double errorRate = 0.0;
            if (totalSessions != 0) {
                errorRate = (double)(it->failedSessions) / totalSessions;
                errorRate *= 100;
            }
            char errorRateStr[64];
            sprintf(errorRateStr, "%4.1f", errorRate);
            out << "<td>" << it->usedSessions
                << "/" << it->failedSessions
                << "/" << errorRateStr << "</td>"
                // bandwidth
                << "<td>" << it->usedBandwidth << "</td>"
                << "<td>" << it->totalBandwidth << "</td>"
                // stream count
                << "<td>" << it->usedStreamCount << "</td>"
                << "<td>" << it->maxStreamCount << "</td>"
                // session
                << "<td>" << it->localSessions << "</td>"
                << "<td>" << (it->localSessions + it->remoteSessions) << "</td>"
                << "</tr>\n";
        }
        out << "</table>\n";
        out << "</form>\n";

        out << "<table class=\"listTable\">"
            << "<tr style=\"text-align:center\" ><th colspan=\"4\">"
            << "<span>Import Channels</span>"
            //<< "<span style=\"font-weight:lighter;font-style:italic;margin-left:5px\">"
            //<< "(Measured Since:" << icInfo.stampMeasuredSince << ")</span>"
            << "</th></tr>\n"
            << "<tr style=\"text-align:center\">"
            << "<th rowspan=\"2\">Channel</th>"
            << "<th rowspan=\"2\">Session</th>"
            << "<th colspan=\"2\">Bandwidth</th></tr>\n"
            << "<tr style=\"text-align:center\"><th>used</th><th>total</th></tr>\n";
        SsmOpenVBO::ImportChannelsStatistics::const_iterator itIc;
        for(itIc = icInfo.importChannelsInfos.begin(); itIc != icInfo.importChannelsInfos.end(); ++itIc) {
            out << "<tr><td>" << itIc->channelName << "</td>"
                << "<td>" << itIc->runningSessCount << "</td>"
                << "<td>" << itIc->usedImportBandwidth << "</td>"
                << "<td>" << itIc->totalImportBandwidth << "</td>"
                << "</tr>";
        }
        out << "</table>\n";
        return true;
    }
}
