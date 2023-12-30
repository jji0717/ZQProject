#include "ZQSnmpMgmt.hpp"
#include "SubAgent.hpp"
#include <FileLog.h>

using namespace ZQ::common;
using namespace ZQ::Snmp;
class IntVar: public ZQ::Snmp::IVariable
{
public:
    IntVar(int i):val_(i){}
    ~IntVar(){}
    virtual bool get(SmiValue& val, AsnType desiredType) const
    {
        return smivalFrom(val, val_, desiredType);
    }
    virtual bool set(const SmiValue& val)
    {
        return smivalTo(val, val_);
    }
    virtual bool validate(const SmiValue& val) const
    {
        return true;
    }
public:
    int val_;
};
class Int64Var: public ZQ::Snmp::IVariable
{
public:
    Int64Var(int64 i):val_(i){}
    ~Int64Var(){}
    virtual bool get(SmiValue& val, AsnType desiredType) const
    {
        return smivalFrom(val, val_, desiredType);
    }
    virtual bool set(const SmiValue& val)
    {
        return smivalTo(val, val_);
    }
    virtual bool validate(const SmiValue& val) const
    {
        return true;
    }
public:
    int64 val_;
};
class StrVar: public ZQ::Snmp::IVariable
{
public:
    StrVar(const std::string& s):val_(s){}
    ~StrVar(){}
    virtual bool get(SmiValue& val, AsnType desiredType) const
    {
        return smivalFrom(val, val_, desiredType);
    }
    virtual bool set(const SmiValue& val)
    {
        return smivalTo(val, val_);
    }
    virtual bool validate(const SmiValue& val) const
    {
        return true;
    }
public:
    std::string val_;
};
void hexPrint(void* v, int len) {
    char tbl[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
    const unsigned char* bs = (const unsigned char*)v;
    for(int i = 0; i < len; ++i) {
      unsigned char b = bs[i];
      printf("%c%c ", tbl[(b >> 4) & 0x0F], tbl[b & 0x0F]);
    }
}
int
main()
{
    FileLog* pLog;
    try
    {
        pLog = new FileLog("SnmpTrace.log", Log::L_DEBUG);
    }
    catch(const FileLogException& e)
    {
        printf("**exception**  %s\n", e.getString());
        return 0;
    }

// TEST decode/encode in SnmpUtil
    /*
Oid i("1.2.3.4.5");
netsnmp_variable_list vb = {0};
oidTo(i, vb);
SmiValue v;
smivalFrom(v, int64(0x101010101010), AsnType_Counter64);
smivalTo(v, vb);
    */
u_char buf[1024];
size_t buflen = 1024;
//ZQSNMP::Util::encodeMsg(buf, &buflen, 0x01, 0, &vb);
 counter64 c64;
 zeroU64(&c64);
 incrByU32(&c64, 0xffff);
 printU64((char*)buf, &c64);
 printf("%s\n", buf);
 asn_build_unsigned_int64(buf, &buflen, ASN_COUNTER64, &c64, sizeof(struct counter64));
 buflen = 1024 - buflen;
 u_char type;
 asn_parse_unsigned_int64(buf, &buflen, &type, &c64, sizeof(counter64));
 printU64((char*)buf, &c64);
 printf("%s\n", buf);
 return 3;
 printf("Encoded MSG:");
 hexPrint(buf, buflen);
 printf("\n");
netsnmp_variable_list vb2 = {0};
u_char mode;
int32_t err;
//ZQSNMP::Util::decodeMsg(buf, buflen, &mode, &err, &vb2);
return 3;
// TEST decode/encode in SnmpUtil

    TablePtr tbl(new Table());
    // addColumn(columnId, type, accessLevel)
    tbl->addColumn(1, AsnType_Integer, aReadWrite);
    tbl->addColumn(2, AsnType_Integer, aReadOnly);
    tbl->addColumn(3, AsnType_Octets, aReadOnly);
    tbl->addColumn(4, AsnType_Counter64, aReadWrite);

    // addRowData(columnId, rowIndex, variable)
    tbl->addRowData(1, Table::buildIndex("1st"), VariablePtr(new IntVar(11)));
    tbl->addRowData(2, Table::buildIndex("1st"), VariablePtr(new IntVar(12)));
    tbl->addRowData(3, Table::buildIndex("1st"), VariablePtr(new StrVar("13")));
    tbl->addRowData(4, Table::buildIndex("1st"), VariablePtr(new Int64Var(12345678901111)));
    tbl->addRowData(1, Table::buildIndex("2nd"), VariablePtr(new IntVar(21)));
    tbl->addRowData(2, Table::buildIndex("2nd"), VariablePtr(new IntVar(22)));
    tbl->addRowData(3, Table::buildIndex("2nd"), VariablePtr(new StrVar("23")));
    tbl->addRowData(4, Table::buildIndex("2nd"), VariablePtr(new Int64Var(12345678902222)));
    tbl->addRowData(1, Table::buildIndex("3rd"), VariablePtr(new IntVar(31)));
    tbl->addRowData(2, Table::buildIndex("3rd"), VariablePtr(new IntVar(32)));
    tbl->addRowData(3, Table::buildIndex("3rd"), VariablePtr(new StrVar("33")));
    tbl->addRowData(4, Table::buildIndex("3rd"), VariablePtr(new Int64Var(12345678903333)));
    ObjectPtr obj(new CompositeObject);
    obj->add(Oid("1.2.3.4"), tbl);
    ZQ::Snmp::Subagent agent(1000, 3);
    agent.addObject(Oid(), obj);
    agent.setLogger(pLog);
    agent.start();
sleep(1000);
    agent.stop();
    pLog->flush();
    return 0;
};
/*
class Table: public 
{
public:
    addColumn(id, obj);
    addRow(instanceId, obj);
};
*/
