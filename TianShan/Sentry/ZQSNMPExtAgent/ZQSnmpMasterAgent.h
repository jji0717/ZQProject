// ZQSnmpMasterAgent.h: interface for the ZQSnmpMasterAgent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZQSNMPMASTERAGENT_H__957E69E3_5591_427F_A819_9D7718F87DB6__INCLUDED_)
#define AFX_ZQSNMPMASTERAGENT_H__957E69E3_5591_427F_A819_9D7718F87DB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <string>
#include <vector>

class ZQSnmpMasterAgent  
{
public:
	ZQSnmpMasterAgent();
	~ZQSnmpMasterAgent();

    ZQSNMP_STATUS ProcessRequest(BYTE pdutype, SnmpVarBindList *pVbl, AsnInteger32 *pErrStat, AsnInteger32 *pErrIdx);
private:
    struct MibRegion
    {
        UINT m_svcId;
        UINT m_svcProcess;
    };
    bool getRegion(const SnmpVarBind *pVb, MibRegion *pRegion);
    bool nextRegion(MibRegion *pRegion);
    ZQSNMP_STATUS dispatchRequest(MibRegion targetRegion, BYTE pdutype, SnmpVarBind *pVb, AsnInteger32 *pErrStat);

private:
    class RegionTable
    {
    public:
        bool next(MibRegion& r) const;
        bool refresh(uint32 & snmpUdpBasePort, timeout_t& snmpAgentPendTimeOut);

    private:
        struct Mod
        {
            Mod(UINT i, const std::string& n): id(i), name(n){}
            UINT id;
            std::string name;
        };
        struct Svc
        {
            UINT id;
            std::string name;
            std::vector< Mod > mods;
        };

        std::vector< Svc >  svcs_;
    } regionTbl_;
    
	timeout_t      snmpAgentPendTimeOut_;
	uint32         snmpUdpBasePort_;
	unsigned long  agentRecvCount_;

};

#endif // !defined(AFX_ZQSNMPMASTERAGENT_H__957E69E3_5591_427F_A819_9D7718F87DB6__INCLUDED_)
