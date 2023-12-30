#ifndef __ZQ_C2Snmp_H__
#define __ZQ_C2Snmp_H__
#include <snmp/SubAgent.hpp>
#include "TransferPortManager.h"
class StrVar: public ZQ::Snmp::IVariable
{
public:
    StrVar(const std::string& s):val_(s){}
    virtual ~StrVar(){}
    virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
    {
        return ZQ::Snmp::smivalFrom(val, val_, desiredType);
    }
    virtual bool set(const ZQ::Snmp::SmiValue& val)
    {
        return ZQ::Snmp::smivalTo(val, val_);
    }
    virtual bool validate(const ZQ::Snmp::SmiValue& val) const
    {
        return true;
    }
public:
    std::string val_;
};

class IntVar: public ZQ::Snmp::IVariable
{
public:
    IntVar(int i):val_(i){}
    virtual ~IntVar(){}
    virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
    {
        return ZQ::Snmp::smivalFrom(val, val_, desiredType);
    }
    virtual bool set(const ZQ::Snmp::SmiValue& val)
    {
        return ZQ::Snmp::smivalTo(val, val_);
    }
    virtual bool validate(const ZQ::Snmp::SmiValue& val) const
    {
        return true;
    }
public:
    int val_;
};

class IntRef: public ZQ::Snmp::IVariable
{
public:
    IntRef(int& i):val_(i){}
    virtual ~IntRef(){}
    virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
    {
        return ZQ::Snmp::smivalFrom(val, val_, desiredType);
    }
    virtual bool set(const ZQ::Snmp::SmiValue& val)
    {
        return ZQ::Snmp::smivalTo(val, val_);
    }
    virtual bool validate(const ZQ::Snmp::SmiValue& val) const
    {
        return true;
    }
public:
    int& val_;
};

class LoglvlRef: public ZQ::Snmp::IVariable
{
public:
    LoglvlRef(ZQ::common::Log& log):log_(log) {}
    virtual ~LoglvlRef() {}
    virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
    {
        int lvl = log_.getVerbosity();
        return ZQ::Snmp::smivalFrom(val, lvl, desiredType);
    }
    virtual bool set(const ZQ::Snmp::SmiValue& val)
    {
        int lvl = 0;
        if(ZQ::Snmp::smivalTo(val, lvl)) {
            log_.setVerbosity(lvl);
            return true;
        } else {
            return false;
        }
    }
    virtual bool validate(const ZQ::Snmp::SmiValue& val) const
    {
        return true;
    }
public:
    ZQ::common::Log& log_;
};

class LongAsIntRef: public ZQ::Snmp::IVariable
{
public:
    LongAsIntRef(Ice::Long& l, Ice::Long ratio):val_(l), ratio_(ratio){
        if(ratio_ == 0)
            ratio_ = 1;
    }
    virtual ~LongAsIntRef(){}
    virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType)
    {
        int v = (int)(val_ / ratio_);
        return ZQ::Snmp::smivalFrom(val, v, desiredType);
    }
    virtual bool set(const ZQ::Snmp::SmiValue& val)
    {
        int v = 0;
        if(ZQ::Snmp::smivalTo(val, v)) {
            val_ = ((Ice::Long)v) * ratio_;
            return true;
        } else {
            return false;
        }
    }
    virtual bool validate(const ZQ::Snmp::SmiValue& val) const
    {
        return true;
    }
public:
    Ice::Long& val_;
    Ice::Long ratio_;
};

#define PortAttrId_Name 2
#define PortAttrId_IsUp 3
#define PortAttrId_Capacity 4
#define PortAttrId_ActiveBandwidth 5
#define PortAttrId_ActiveTransferCount 6
#define PortAttrId_Enabled 7
#define PortAttrId_Penalty 8
#define PortAttrId_Addresses 9
#define PortAttrId_ErrorRate 10

class PortAttributeVisitor : public ZQ::Snmp::IVariable {
public:
    PortAttributeVisitor(ZQTianShan::CDN::TransferPortManager& mgr, const std::string& name, int attrId)
        :portMgr_(mgr), name_(name), attrId_(attrId) {
    }
    virtual ~PortAttributeVisitor(){}
    virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType) 
	{
        ZQTianShan::CDN::TransferPortManager::PortInfo port;
        if(!portMgr_.queryPort(name_, port)) {
            // reset the port attributes
            port = ZQTianShan::CDN::TransferPortManager::PortInfo();
        }
        switch(attrId_) {
        case PortAttrId_Name:
            return ZQ::Snmp::smivalFrom(val, name_, desiredType);
        case PortAttrId_IsUp:
            {
                int isUp = port.isUp ? 1 : 0;
                return ZQ::Snmp::smivalFrom(val, isUp, desiredType);
            }
        case PortAttrId_Capacity:
            {
                int capacityKbps = (int)(port.capacity / 1024); // kbps
                return ZQ::Snmp::smivalFrom(val, capacityKbps, desiredType);
            }
        case PortAttrId_ActiveBandwidth:
            {
                int activeBwKpbs = (int)(port.activeBandwidth / 1024); // kbps
                return ZQ::Snmp::smivalFrom(val, activeBwKpbs, desiredType);
            }
        case PortAttrId_ActiveTransferCount:
            {
                int sessCount = (int)port.activeTransferCount;
                return ZQ::Snmp::smivalFrom(val, sessCount, desiredType);
            }
        case PortAttrId_Enabled:
            {
                int enabled = port.enabled ? 1 : 0;
                return ZQ::Snmp::smivalFrom(val, enabled, desiredType);
            }
        case PortAttrId_Penalty:
            {
                int penalty = (int)port.penalty;
                return ZQ::Snmp::smivalFrom(val, penalty, desiredType);
            }                
        case PortAttrId_Addresses:
            {
                std::ostringstream buf;
                for(size_t i = 0; i < port.addressListIPv4.size(); ++i) {
                    if(i != 0) {
                        buf << ",";
                    }
                    buf << port.addressListIPv4[i];
                }
                if(!port.addressListIPv4.empty() && !port.addressListIPv6.empty()) {
                    buf << ";";
                }
                for(size_t i = 0; i < port.addressListIPv6.size(); ++i) {
                    if(i != 0) {
                        buf << ",";
                    }
                    buf << port.addressListIPv6[i];
                }
                return ZQ::Snmp::smivalFrom(val, buf.str(), desiredType);
            }
        case PortAttrId_ErrorRate:
            {
                int errRatePercent = 0;
                if(port.sessionCountTotal > 0) {
                    errRatePercent = (int)(port.sessionCountFailed * 100 / port.sessionCountTotal);
                }
                return ZQ::Snmp::smivalFrom(val, errRatePercent, desiredType);
            }
        default:
            return false;
        }
    }
    virtual bool set(const ZQ::Snmp::SmiValue& val) {
        // not implement yet
        return false;
    }
    virtual bool validate(const ZQ::Snmp::SmiValue& val) const {
        return false;
    }
public:
    ZQTianShan::CDN::TransferPortManager& portMgr_;
    std::string name_;
    int attrId_;
};

class PortCounterReset : public ZQ::Snmp::IVariable {
public:
    PortCounterReset(ZQTianShan::CDN::TransferPortManager& portMgr)
        :portMgr_(portMgr) {
    }
    virtual ~PortCounterReset(){}
    virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType) 
	{
        return ZQ::Snmp::smivalFrom(val, int(0), desiredType);
    }
    virtual bool set(const ZQ::Snmp::SmiValue& val) {
        int i = 0;
        if (ZQ::Snmp::smivalTo(val, i)) {
            if(i > 0) {
                portMgr_.resetStreamCreationCounter();
            }
            return true;
        } else {
            return false;
        }
    }
    virtual bool validate(const ZQ::Snmp::SmiValue& val) const {
        return (val.syntax() == ZQ::Snmp::AsnType_Integer);
    }
public:
    ZQTianShan::CDN::TransferPortManager& portMgr_;
};
class PortSnmpManager {
public:
    PortSnmpManager() {
        portMgr_ = NULL;
        portCount_ = 0;
    }
    void updatePort(const std::string& name) {
        ZQ::common::MutexGuard guard(lock_);
        if(registered_.end() == std::find(registered_.begin(), registered_.end(), name)) {
            // new port
            registered_.push_back(name);
            portCount_ = (int)registered_.size();
            if(tbl_ && portMgr_) {
                registerPort(name, portCount_);
            }
        } else {
            return;
        }
    }
    void publish(ZQTianShan::CDN::TransferPortManager& mgr, ZQ::Snmp::Subagent& agent, const ZQ::Snmp::Oid& subOid) {
        if(tbl_ || portMgr_) {
            return;
        }
        ZQ::common::MutexGuard guard(lock_);
        portMgr_ = &mgr;
        using namespace ZQ::Snmp;
        tbl_ = TablePtr(new Table());
        tbl_->addColumn(1, AsnType_Integer, aReadOnly);
        tbl_->addColumn(PortAttrId_Name, AsnType_Octets, aReadOnly);
        tbl_->addColumn(PortAttrId_IsUp, AsnType_Integer, aReadOnly);
        tbl_->addColumn(PortAttrId_Capacity, AsnType_Integer, aReadOnly);
        tbl_->addColumn(PortAttrId_ActiveBandwidth, AsnType_Integer, aReadOnly);
        tbl_->addColumn(PortAttrId_ActiveTransferCount, AsnType_Integer, aReadOnly);
        tbl_->addColumn(PortAttrId_Enabled, AsnType_Integer, aReadOnly);
        tbl_->addColumn(PortAttrId_Penalty, AsnType_Integer, aReadOnly);
        tbl_->addColumn(PortAttrId_Addresses, AsnType_Integer, aReadOnly);
        tbl_->addColumn(PortAttrId_ErrorRate, AsnType_Integer, aReadOnly);

		int i = 1;
        //for(size_t i = 1; i <= registered_.size(); ++i) {
		for(std::vector<std::string>::const_iterator it = registered_.begin() ; 
			it != registered_.end() ; it ++  )
		{
            registerPort(*it, i++);
        }
        agent.module().add(subOid + Oid("1.1"), tbl_);
        agent.module().add(subOid + Oid("2"), ManagedPtr(new SimpleObject(VariablePtr(new IntRef(portCount_)), AsnType_Integer, aReadOnly)));
        agent.module().add(subOid + Oid("3"), ManagedPtr(new SimpleObject(VariablePtr(new PortCounterReset(mgr)), AsnType_Integer, aReadWrite)));

    }
    void reset() {
        ZQ::common::MutexGuard guard(lock_);
        registered_.clear();
        tbl_.reset();
        portMgr_ = NULL;
    }
private:
    void registerPort(const std::string& name, int rowIndex) const {
        if(tbl_ && portMgr_) {
            using namespace ZQ::Snmp;
            Oid rowIdx = ZQ::Snmp::Table::buildIndex(rowIndex);
            tbl_->addRowData(1, rowIdx, VariablePtr(new IntVar(rowIndex)));
            tbl_->addRowData(PortAttrId_Name, rowIdx, VariablePtr(new PortAttributeVisitor(*portMgr_, name, PortAttrId_Name)));
            tbl_->addRowData(PortAttrId_IsUp, rowIdx, VariablePtr(new PortAttributeVisitor(*portMgr_, name, PortAttrId_IsUp)));
            tbl_->addRowData(PortAttrId_Capacity, rowIdx, VariablePtr(new PortAttributeVisitor(*portMgr_, name, PortAttrId_Capacity)));
            tbl_->addRowData(PortAttrId_ActiveBandwidth, rowIdx, VariablePtr(new PortAttributeVisitor(*portMgr_, name, PortAttrId_ActiveBandwidth)));
            tbl_->addRowData(PortAttrId_ActiveTransferCount, rowIdx, VariablePtr(new PortAttributeVisitor(*portMgr_, name, PortAttrId_ActiveTransferCount)));
            tbl_->addRowData(PortAttrId_Enabled, rowIdx, VariablePtr(new PortAttributeVisitor(*portMgr_, name, PortAttrId_Enabled)));
            tbl_->addRowData(PortAttrId_Penalty, rowIdx, VariablePtr(new PortAttributeVisitor(*portMgr_, name, PortAttrId_Penalty)));
            tbl_->addRowData(PortAttrId_Addresses, rowIdx, VariablePtr(new PortAttributeVisitor(*portMgr_, name, PortAttrId_Addresses)));
            tbl_->addRowData(PortAttrId_ErrorRate, rowIdx, VariablePtr(new PortAttributeVisitor(*portMgr_, name, PortAttrId_ErrorRate)));
        }
    }
private:
    ZQ::common::Mutex lock_;
    std::vector<std::string> registered_;
    ZQ::Snmp::TablePtr tbl_;
    ZQTianShan::CDN::TransferPortManager* portMgr_;
    int portCount_;
};

class HitRateVisitor : public ZQ::Snmp::IVariable {
public:
    HitRateVisitor(const ZQTianShan::CDN::HitCounter& hitCounter)
        :hitCounter_(hitCounter) {
    }
    virtual ~HitRateVisitor(){}
    virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType) 
	{
        int hitRate = (int) floor(hitCounter_.getHitRate() * 100);
        return ZQ::Snmp::smivalFrom(val, hitRate, desiredType);
    }
    virtual bool set(const ZQ::Snmp::SmiValue& val) {
        // not implement yet
        return false;
    }
    virtual bool validate(const ZQ::Snmp::SmiValue& val) const {
        return false;
    }
public:
    const ZQTianShan::CDN::HitCounter& hitCounter_;
};
class HitRateReset : public ZQ::Snmp::IVariable {
public:
    HitRateReset(ZQTianShan::CDN::HitCounter& hitCounter)
        :hitCounter_(hitCounter) {
    }
    virtual ~HitRateReset(){}
    virtual bool get(ZQ::Snmp::SmiValue& val, ZQ::Snmp::AsnType desiredType) 
	{
        return ZQ::Snmp::smivalFrom(val, int(0), desiredType);
    }
    virtual bool set(const ZQ::Snmp::SmiValue& val) {
        int i = 0;
        if (ZQ::Snmp::smivalTo(val, i)) {
            if(i > 0) {
                hitCounter_.reset();
            }
            return true;
        } else {
            return false;
        }
    }
    virtual bool validate(const ZQ::Snmp::SmiValue& val) const {
        return (val.syntax() == ZQ::Snmp::AsnType_Integer);
    }
public:
    ZQTianShan::CDN::HitCounter& hitCounter_;
};
#endif
