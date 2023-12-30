#include "TianShanDefines.h"
#include "httpdInterface.h"
#include "C2Locator.h"
#include "Log.h"
#include "Text.h"
#include "urlstr.h"
#include "CDNDefines.h"
#include "TianShanIceHelper.h"
#include <set>

#ifdef ZQ_OS_MSWIN
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
#endif

ZQ::common::Log* _pLog = NULL;
#define LOG (*_pLog)
static std::string neighborLayout(const std::string& currentTemplate, const std::string& neighborLayoutId)
{
    std::string::size_type pos = currentTemplate.rfind(FNSEPC);
    return (pos == std::string::npos ? neighborLayoutId : currentTemplate.substr(0, pos + 1) + neighborLayoutId);
}

class C2LocMonitor
{
public:
    explicit C2LocMonitor(ZQ::common::Log& log);
    ~C2LocMonitor();
    bool connect(const std::string& ep);
    bool listClients(TianShanIce::SCS::ClientTransfers& clients);
    bool listPorts(TianShanIce::SCS::TransferPorts& ports);
    bool listSessionsByClient(TianShanIce::SCS::TransferSessions& sess, const std::string& client);
    bool listSessionsByPort(TianShanIce::SCS::TransferSessions& sess, const std::string& port);
    bool updatePortsAvailability(const TianShanIce::StrValues& ports, bool enabled);
private:
    ZQ::common::Log& _log;
    Ice::CommunicatorPtr _comm;
    TianShanIce::SCS::C2LocatorPrx _loc;
};

// all error case should be handled outside the *show* functions
static void showClientsTable(IHttpRequestCtx* ctx, const TianShanIce::SCS::ClientTransfers& clients);
static void showPortsTable(IHttpRequestCtx* ctx, const TianShanIce::SCS::TransferPorts& ports);
static void showSessionsTable(IHttpRequestCtx* ctx, const TianShanIce::SCS::TransferSessions& sess);

#define VAR_Key_Template "#template"
#define VAR_Key_Endpoint "ep"
#define VAR_Key_Category "c"
#define VAR_Key_Value "v"
#define VAR_Val_Category_Client "client"
#define VAR_Val_Category_Port "port"
#define VAR_Key_Ports "ports"
#define VAR_Key_EnablePorts "enabled"
extern "C"
{
    __EXPORT bool LibInit(ZQ::common::Log* pLog)
    {
        if(NULL == pLog)
            return false;

        _pLog = pLog;
        return true;
    }
    __EXPORT void LibUninit( )
    {
    }
    // clients pages
    __EXPORT bool ClientsPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
        {
            return false;
        }

        IHttpResponse& out = pHttpRequestCtx->Response();
        const char* tmpl = pHttpRequestCtx->GetRequestVar(VAR_Key_Template);
        const char* ep = pHttpRequestCtx->GetRequestVar(VAR_Key_Endpoint);

        if(NULL == tmpl)
        {
            out.SetLastError("Parameter Missed: template");
            return false;
        }

        if(NULL == ep)
        {
            out.SetLastError("Parameter Missed: endpoint");
            return false;
        }
        C2LocMonitor loc(LOG);
        if(!loc.connect(ep))
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "ClientsPage() failed to connect locator. endpoint=%s"), ep);
            out.SetLastError("Failed to connect the locator server!");
            return false;
        }
        TianShanIce::SCS::ClientTransfers clients;
        if(loc.listClients(clients))
        {
            out << "<H2>Client Transfers</H2>";

            showClientsTable(pHttpRequestCtx, clients);
        }
        else
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "ClientsPage() failed to query clients info. endpoint=%s"), ep);
            out.SetLastError("Failed to query clients info");
            return false;
        }

        return true;
    }

    // ports pages
    __EXPORT bool PortsPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
        {
            return false;
        }

        IHttpResponse& out = pHttpRequestCtx->Response();
        const char* tmpl = pHttpRequestCtx->GetRequestVar(VAR_Key_Template);
        const char* ep = pHttpRequestCtx->GetRequestVar(VAR_Key_Endpoint);

        if(NULL == tmpl)
        {
            out.SetLastError("Parameter Missed: template");
            return false;
        }

        if(NULL == ep)
        {
            out.SetLastError("Parameter Missed: endpoint");
            return false;
        }
        C2LocMonitor loc(LOG);
        if(!loc.connect(ep))
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "PortsPage() failed to connect locator. endpoint=%s"), ep);
            out.SetLastError("Failed to connect the locator server!");
            return false;
        }
        // update the ports
        const char* portList = pHttpRequestCtx->GetRequestVar(VAR_Key_Ports);
        const char* enablePorts = pHttpRequestCtx->GetRequestVar(VAR_Key_EnablePorts);
        if(portList && '\0' != *portList)
        { // need update the ports
            if(enablePorts && '\0' != *enablePorts)
            { // with method
                TianShanIce::StrValues vals;
                ZQ::common::Text::split(vals, portList, ",");
                if(0 == strcmp(enablePorts, "1"))
                { // enable ports
                    loc.updatePortsAvailability(vals, true);
                }
                else if(0 == strcmp(enablePorts, "0"))
                {
                    loc.updatePortsAvailability(vals, false);
                }
                else
                { // warning here

                }
            }
        }
        TianShanIce::SCS::TransferPorts ports;
        if(loc.listPorts(ports))
        {
            out << "<H2>Transfer Ports</H2>";

            showPortsTable(pHttpRequestCtx, ports);
        }
        else
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "PortsPage() failed to query ports info. endpoint=%s"), ep);
            out.SetLastError("Failed to query ports info");
            return false;
        }

        return true;
    }
    // sessions
    __EXPORT bool SessionsPage(IHttpRequestCtx *pHttpRequestCtx)
    {
        if(NULL == pHttpRequestCtx)
        {
            return false;
        }

        IHttpResponse& out = pHttpRequestCtx->Response();
        const char* tmpl = pHttpRequestCtx->GetRequestVar(VAR_Key_Template);
        if(NULL == tmpl)
        {
            out.SetLastError("Parameter Missed: template");
            return false;
        }

        const char* ep = pHttpRequestCtx->GetRequestVar(VAR_Key_Endpoint);
        if(NULL == ep)
        {
            out.SetLastError("Parameter Missed: endpoint");
            return false;
        }

        const char* category = pHttpRequestCtx->GetRequestVar(VAR_Key_Category);
        const char* value = pHttpRequestCtx->GetRequestVar(VAR_Key_Value);
        if(NULL != category && '\0' != *category)
        { // validate the query parameter
            if(0 != strcmp(category, VAR_Val_Category_Client) && 0 != strcmp(category, VAR_Val_Category_Port))
            {
                out.SetLastError("Bad Parameter: query category");
                return false;
            }
        }

        C2LocMonitor loc(LOG);
        if(!loc.connect(ep))
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "SessionsPage() failed to connect locator. endpoint=%s"), ep);
            out.SetLastError("Failed to connect the locator server!");
            return false;
        }

        out << "<H2>Transfer Sessions</H2>";
        // total summary
        TianShanIce::SCS::ClientTransfers clients;
        loc.listClients(clients);
        TianShanIce::SCS::TransferPorts ports;
        loc.listPorts(ports);
        // sum up the total available bandwidth and the consumed bandwidth
        Ice::Long totalBw = 0;
        Ice::Long totalConsumed = 0;
        Ice::Long totalSessions = 0;
        for(size_t i = 0; i < ports.size(); ++i)
        {
            if(!ports[i].isUp)
                continue;

            totalBw += ports[i].capacity;
            totalConsumed += ports[i].activeBandwidth;
            totalSessions += ports[i].activeTransferCount;
        }
#define SummaryItemProp " style='padding-left:15px'"
#define Summary(nSess, consumed, bw, nPort, nClient) "<span>"\
        << "<span " SummaryItemProp << ">SessionCount:" << nSess << "</span>"\
        << "<span " SummaryItemProp << ">BwUsage:" << consumed << "/" << bw << "</span>"\
        << "<span " SummaryItemProp << ">TransferPortCount:" << nPort << "</span>"\
        << "<span " SummaryItemProp << ">ClientTransferCount:" << nClient << "</span>"\
        << "</span>"

        out << "<div><span style='font-weight:bold'>Total:</span>"
            << Summary(totalSessions, totalConsumed, totalBw, (Ice::Long)ports.size(), (Ice::Long)clients.size())
            << "</div>";
        out << "<hr>";
        // show the search panel
        out << "<FORM style='display:inline' method='GET' action='SessionsPage.c2loc.tswl'>"
            << "<select name='" VAR_Key_Category "'>"
            << "<option value='" VAR_Val_Category_Client "'>ClientTransfer</option>"
            << "<option value='" VAR_Val_Category_Port "' "
            << (category && 0 == strcmp(category, VAR_Val_Category_Port) ? "selected" : "")
            << ">TranferPort</option>"
            << "</select>";
        out << ":<input type='text' name='" VAR_Key_Value "' value='"
            << (value ? value : "")
            << "'>"
            << "<input type='hidden' name='" VAR_Key_Template "' value='" << tmpl << "'>"
            << "<input type='hidden' name='" VAR_Key_Endpoint "' value='" << ep << "'>"
            << "<input type='submit' value='Search'>";
        out << "</FORM><br><br>";

        Ice::Long subtotalBw = 0;
        Ice::Long subtotalConsumed = 0;
        Ice::Long subtotalPorts = 0;
        Ice::Long subtotalClients = 0;
        TianShanIce::SCS::TransferSessions sess;        
        if(NULL == category || '\0' == *category ||  NULL == value || '\0' == *value)
        {
        }
        else if(0 == strcmp(category, VAR_Val_Category_Client))
        {
            if(loc.listSessionsByClient(sess, value))
            {
                // compute the subtotal
                for(size_t i = 0; i < clients.size(); ++i)
                {
                    if(clients[i].address == value)
                    {
                        subtotalBw = clients[i].ingressCapacity;
                        break;
                    }
                }

                std::set<std::string> portSet;
                for(size_t i = 0 ; i < sess.size(); ++i)
                {
                    portSet.insert(sess[i].transferPort);
                    subtotalConsumed += sess[i].allocatedBW;
                }
                subtotalClients = 1;
                subtotalPorts = portSet.size();
            }
            else
            {
                LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "SessionsPage() failed to query sessions info. endpoint=%s"), ep);
                out.SetLastError("Failed to query sessions info");
                return false;
            }
        }
        else // port
        {
            if(loc.listSessionsByPort(sess, value))
            {
                // compute the subtotal
                for(size_t i = 0; i < ports.size(); ++i)
                {
                    if(ports[i].name == value)
                    {
                        subtotalBw = ports[i].capacity;
                        break;
                    }
                }

                std::set<std::string> clientSet;
                for(size_t i = 0 ; i < sess.size(); ++i)
                {
                    clientSet.insert(sess[i].transferPort);
                    subtotalConsumed += sess[i].allocatedBW;
                }
                subtotalPorts = 1;
                subtotalClients = clientSet.size();
            }
            else
            {
                LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "SessionsPage() failed to query sessions info. endpoint=%s"), ep);
                out.SetLastError("Failed to query sessions info");
                return false;
            }
        }
        out << "<div style='font-style:italic'><span style='font-weight:bold'>Subtotal:</span>"
            << Summary((Ice::Long)sess.size(), subtotalConsumed, subtotalBw, subtotalPorts, subtotalClients)
            << "</div>";

        showSessionsTable(pHttpRequestCtx, sess);
        if(sess.empty())
        {
            out << "<br><div>No matched result found.</div>";
        }
        return true;
    }
} // extern "C"

C2LocMonitor::C2LocMonitor(ZQ::common::Log& log)
    :_log(log)
{
}
C2LocMonitor::~C2LocMonitor()
{
    if(_loc)
    {
        _loc = NULL;
    }

    if(_comm)
    {
        try{ _comm->destroy(); } catch(...){}
        _comm = NULL;
    }
}
bool C2LocMonitor::connect(const std::string& ep)
{
    if(ep.empty())
        return false;

    if(!_comm)
    {
        try
        {
            Ice::InitializationData initData;
            _comm = Ice::initialize(initData);
        }catch(const Ice::Exception& e)
        {
            // log here
            _log(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "failed to initialize communicator with %s"), e.ice_name().c_str());
            return false;
        }
    }

    try
    {
        _loc = TianShanIce::SCS::C2LocatorPrx::checkedCast(_comm->stringToProxy("C2Locator:" + ep));
    }
    catch(const Ice::Exception& e)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "failed to connect locator with %s"), e.ice_name().c_str());
        return false;
    }

    return true;
}
bool C2LocMonitor::listClients(TianShanIce::SCS::ClientTransfers& clients)
{
    if(_loc)
    {
        try
        {
            clients = _loc->listClients();
            return true;
        }
        catch(const Ice::Exception& e)
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "listClients() Caught %s, desc:%s"), e.ice_name().c_str(), e.what());
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool C2LocMonitor::listPorts(TianShanIce::SCS::TransferPorts& ports)
{
    if(_loc)
    {
        try
        {
            ports = _loc->listTransferPorts();
            return true;
        }
        catch(const Ice::Exception& e)
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "listPorts() Caught %s, desc:%s"), e.ice_name().c_str(), e.what());
            return false;
        }
    }
    else
    {
        return false;
    }
}
bool C2LocMonitor::listSessionsByClient(TianShanIce::SCS::TransferSessions& sess, const std::string& client)
{
    if(client.empty())
    {
        return false;
    }

    if(_loc)
    {
        try
        {
            sess = _loc->listSessionsByClient(client);
            return true;
        }
        catch(const Ice::Exception& e)
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "listSessionByClient() Caught %s, desc:%s"), e.ice_name().c_str(), e.what());
            return false;
        }
    }
    else
    {
        return false;
    }
}
bool C2LocMonitor::listSessionsByPort(TianShanIce::SCS::TransferSessions& sess, const std::string& port)
{
    if(port.empty())
    {
        return false;
    }

    if(_loc)
    {
        try
        {
            sess = _loc->listSessionsByPort(port);
            return true;
        }
        catch(const Ice::Exception& e)
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "listSessionByPort() Caught %s, desc:%s"), e.ice_name().c_str(), e.what());
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool C2LocMonitor::updatePortsAvailability(const TianShanIce::StrValues& ports, bool enabled)
{
    if(ports.empty())
    {
        return false;
    }

    if(_loc)
    {
        try
        {
            _loc->updatePortsAvailability(ports, enabled);
            return true;
        }
        catch(const Ice::Exception& e)
        {
            LOG(ZQ::common::Log::L_ERROR, CLOGFMT(c2loc, "updatePortsAvailability() Caught %s, desc:%s"), e.ice_name().c_str(), e.what());
            return false;
        }
    }
    else
    {
        return false;
    }
}

// all error case should be handled outside the *show* functions
static void showClientsTable(IHttpRequestCtx* ctx, const TianShanIce::SCS::ClientTransfers& clients)
{
    if(!ctx)
        return;
    const char* tmpl = ctx->GetRequestVar(VAR_Key_Template);
    const char* ep = ctx->GetRequestVar(VAR_Key_Endpoint);
    if(NULL == tmpl || NULL == ep)
        return;
    // headers
    IHttpResponse& out = ctx->Response();
    out << "<TABLE class='listTable'>"
        << "<TR class='heading'>"
        << "<TH>ClientTransfer</TH>"
        << "<TH>Sessions</TH>"
        << "<TH>Consumed</TH>"
        << "<TH>IngressCapacity</TH>"
        << "</TR>";
    if(!clients.empty())
    {
        // setup the url generator
        ZQ::common::URLStr urlGen(ctx->GetRootURL(), true); // case sensitive
        urlGen.setPath("SessionsPage.c2loc.tswl");
        urlGen.setVar(VAR_Key_Template, neighborLayout(tmpl, "sessions").c_str());
        urlGen.setVar(VAR_Key_Endpoint, ep);
        urlGen.setVar(VAR_Key_Category, VAR_Val_Category_Client);
        TianShanIce::SCS::ClientTransfers::const_iterator it;
        for(it = clients.begin(); it != clients.end(); ++it)
        {

            urlGen.setVar(VAR_Key_Value, it->address.c_str());
            out << "<TR>"
                << "<TD>" << it->address << "</TD>"
                << "<TD><A title='Show sessions of client " << it->address << "'"
                << " href='" << urlGen.generate() << "' style='padding:0px 10px'>" << it->activeTransferCount << "</A></TD>"
                << "<TD>" << it->consumedBandwidth << "</TD>"
                << "<TD>" << it->ingressCapacity << "</TD>"
                << "</TR>";
        }
    }

    out << "</TABLE>";
}
static void showPortsTable(IHttpRequestCtx* ctx, const TianShanIce::SCS::TransferPorts& ports)
{
    if(!ctx)
        return;
    const char* tmpl = ctx->GetRequestVar(VAR_Key_Template);
    const char* ep = ctx->GetRequestVar(VAR_Key_Endpoint);
    if(NULL == tmpl || NULL == ep)
        return;
    // headers
    IHttpResponse& out = ctx->Response();
    out << "<script type='text/javascript'>"
        << "function getCheckList(){\n"
        << "    return document.getElementById('ports-tbl').getElementsByTagName('input');\n"
        << "};\n\n"
        << "function checkAll(){\n"
        << "    var checked = document.getElementById('all').checked;\n"
        << "    var checkList = getCheckList();\n"
        << "    for(var i = 0; i < checkList.length; ++i)\n"
        << "        checkList[i].checked = checked;\n"
        << "}\n\n"
        << "function checkOne(){\n"
        << "    document.getElementById('all').checked = false;\n"
        << "}\n\n"
        << "function getCheckedPortList(){\n"
        << "    var ports = new Array();\n"
        << "    var checkList = getCheckList();\n"
        << "    for(var i = 0; i < checkList.length; ++i)\n"
        << "        if(checkList[i].checked) ports.push(checkList[i].value);\n"
        << "    return ports;\n"
        << "}\n\n"
        << "function enablePorts(){\n"
        << "    document.getElementById('ports').value = getCheckedPortList().join(',');\n"
        << "    document.getElementById('enable-ports').value = '1';\n"
        << "    document.getElementById('main-form').submit();\n"
        << "}\n\n"
        << "function disablePorts(){\n"
        << "    document.getElementById('ports').value = getCheckedPortList().join(',');\n"
        << "    document.getElementById('enable-ports').value = '0';\n"
        << "    document.getElementById('main-form').submit();\n"
        << "}\n\n"
        << "</script>\n";
    out << "<FORM id='main-form' action='PortsPage.c2loc.tswl' method='POST'>"
        << "<input type='hidden' name='" VAR_Key_Template "' value='" << tmpl << "'>"
        << "<input type='hidden' name='" VAR_Key_Endpoint "' value='" << ep << "'>"
        << "<input id='ports' type='hidden' name='" VAR_Key_Ports "' value=''>"
        << "<input id='enable-ports' type='hidden' name='" VAR_Key_EnablePorts "' value=''>";
    
    out << "<H3>Update selected ports' availability: "
        << "<input type='button' value='Enable' onclick='enablePorts()'>"
        << "<input type='button' value='Disable' onclick='disablePorts()'>";
    out << "<H3>";
    out << "<TABLE class='listTable'>"
        << "<COLGROUP><COL width='20px' allign='center' colspan='1'></COLGROUP>"
        << "<TR class='heading'>"
        << "<TH><input id='all' type='checkbox' title='select all ports' onclick='checkAll()'></TH>"
        << "<TH>TransferPort</TH>"
        << "<TH>Status</TH>"
        << "<TH>Sessions</TH>"
        << "<TH>Consumed</TH>"
        << "<TH>Capacity</TH>"
        << "<TH>Availability</TH>"
        << "<TH>Penalty</TH>"
        << "<TH>Addresses</TH>"
        << "</TR>";
    out << "<TBODY id='ports-tbl'>";
    if(!ports.empty())
    {
        // setup the url generator
        ZQ::common::URLStr urlGen(ctx->GetRootURL(), true); // case sensitive
        urlGen.setPath("SessionsPage.c2loc.tswl");
        urlGen.setVar(VAR_Key_Template, neighborLayout(tmpl, "sessions").c_str());
        urlGen.setVar(VAR_Key_Endpoint, ep);
        urlGen.setVar(VAR_Key_Category, VAR_Val_Category_Port);
        TianShanIce::SCS::TransferPorts::const_iterator it;
        for(it = ports.begin(); it != ports.end(); ++it)
        {

            urlGen.setVar(VAR_Key_Value, it->name.c_str());
            out << "<TR>"
                << "<TD><input type='checkbox' value='" << it->name << "' onclick='checkOne()'></TD>"
                << "<TD>" << it->name << "</TD>"
                << "<TD>" << (it->isUp ? "UP" : "DOWN") << "</TD>"
                << "<TD><A title='Show sessions of transfer port " << it->name << "'"
                << " href='" << urlGen.generate() << "' style='padding:0px 10px'>" << it->activeTransferCount << "</A></TD>"
                << "<TD>" << it->activeBandwidth << "</TD>"
                << "<TD>" << it->capacity << "</TD>"
                << "<TD>" << (it->enabled ? "enabled" : "disabled") << "</TD>"
                << "<TD>" << it->penalty << "</TD>"
                << "<TD>" << ZQ::common::Text::join(it->addressListIPv4) << "; " << ZQ::common::Text::join(it->addressListIPv6) << "</TD>"
                << "</TR>";
        }
    }

    out << "</TBODY></TABLE>";
    out << "</FORM>";
}
static void showSessionsTable(IHttpRequestCtx* ctx, const TianShanIce::SCS::TransferSessions& sess)
{
    if(!ctx)
        return;
    const char* tmpl = ctx->GetRequestVar(VAR_Key_Template);
    const char* ep = ctx->GetRequestVar(VAR_Key_Endpoint);
    if(NULL == tmpl || NULL == ep)
        return;
    // headers
    // setup the url generator
    ZQ::common::URLStr urlGen(ctx->GetRootURL(), true); // case sensitive
    urlGen.setVar(VAR_Key_Endpoint, ep);
    urlGen.setVar(VAR_Key_Template, neighborLayout(tmpl, "clients").c_str());

    IHttpResponse& out = ctx->Response();
    out << "<TABLE class='listTable'>"
        << "<TR class='heading'>"
        << "<TH>TransferId</TH>";
    urlGen.setPath("ClientsPage.c2loc.tswl");


    out << "<TH><A title='Show clients infomation'"
        << " href='" << urlGen.generate() << "'>ClientTransfer</A></TH>";
    urlGen.setPath("PortsPage.c2loc.tswl");
    urlGen.setVar(VAR_Key_Template, neighborLayout(tmpl, "ports").c_str());

    out << "<TH><A title='Show transfer ports information'"
        << " href='" << urlGen.generate() << "'>TransferPort</A></TH>"
        << "<TH>AllocatedBw</TH>"
        << "<TH>PID</TH>"
        << "<TH>PAID</TH>"
        << "<TH>SubType/Ext</TH>"
        << "</TR>";

    //    urlGen.setPath("SessionsPage.c2loc.tswl");
    //    urlGen.setVar(VAR_Key_Template, neighborLayout(tmpl, "sessions").c_str());

    TianShanIce::SCS::TransferSessions::const_iterator it;
    for(it = sess.begin(); it != sess.end(); ++it)
    {

        out << "<TR>"
            << "<TD>" << it->transferId << "</TD>";
        // client
        //        urlGen.setVar(VAR_Key_Category, VAR_Val_Category_Client);
        //        urlGen.setVar(VAR_Key_Value, it->clientTransfer.c_str());
        //        out << "<TD><A title='Show sessions of client " << it->clientTransfer << "'"
        //            << " href='" << urlGen.generate() << "'>" << it->clientTransfer << "</A></TD>";
        out << "<TD>" << it->clientTransfer << "</TD>";
        //port
        //        urlGen.setVar(VAR_Key_Category, VAR_Val_Category_Port);
        //        urlGen.setVar(VAR_Key_Value, it->transferPort.c_str());
        //        out << "<TD><A title='Show sessions of transfer port " << it->transferPort << "'"
        //            << " href='" << urlGen.generate() << "'>" << it->transferPort << "</A></TD>"
        out << "<TD>" << it->transferPort << "</TD>"
            << "<TD>" << it->allocatedBW << "</TD>";
        std::string val;
        ZQTianShan::Util::getValueMapDataWithDefault(it->others, CDN_PID, "", val);
        out << "<TD>" << val << "</TD>";
        ZQTianShan::Util::getValueMapDataWithDefault(it->others, CDN_PAID, "", val);
        out << "<TD>" << val << "</TD>";
        ZQTianShan::Util::getValueMapDataWithDefault(it->others, CDN_SUBTYPE, "", val);
        if(val.empty())
            ZQTianShan::Util::getValueMapDataWithDefault(it->others, CDN_EXTENSIONNAME, "", val);
        out << "<TD>" << val << "</TD>";
        out << "</TR>";
    }
    
    out << "</TABLE>";
}
