// implement the NGOD-DEVICE-MIB
// base on the GEN_Network Mgt Arch (NGOD 2.0)

#include <snmp.h>
#include <mgmtapi.h>
#include <string>

BOOL APIENTRY DllMain( HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
         break;
    }
    return TRUE;
}
static AsnObjectIdentifier gOidNgodDev = {0};
static AsnObjectIdentifier gOidNgodDevServer = {0};

struct
{
  AsnAny dhcp;
  AsnAny ntp;
  AsnAny tftpType;
  AsnAny tftp;
  AsnAny config;
  AsnAny configTrigger;
  AsnAny configStatus;
} gNgodDev;

#define BUILD_OctetString(str, size) 		\
  if(size > 0){					\
    str.stream = (BYTE*)SnmpUtilMemAlloc(size);	\
    str.length = size;				\
    str.dynamic = TRUE;				\
    memset(str.stream, 0, size);		\
  } else {					\
    str.stream = NULL;				\
    str.length = 0;				\
    str.dynamic = FALSE;			\
  }
#define BUILD_IpAddress(val) {			\
    val.asnType = ASN_IPADDRESS;		\
    BUILD_OctetString((val.asnValue.address), 4);	\
  }

static void buildNgodDevMib()
{
  // init ngod dev
  BUILD_IpAddress(gNgodDev.dhcp);
  BUILD_IpAddress(gNgodDev.ntp);

  gNgodDev.tftpType.asnType = ASN_INTEGER; // InetAddressType
  gNgodDev.tftpType.asnValue.number = 0; // unknown(0). Is the Mib definition OK?
  BUILD_IpAddress(gNgodDev.tftp);

  gNgodDev.config.asnType = ASN_OCTETSTRING; // SnmpAdminString
  BUILD_OctetString(gNgodDev.config.asnValue.string, 0);

  gNgodDev.configTrigger.asnType = ASN_INTEGER; // TruthValue
  gNgodDev.configTrigger.asnValue.number = 2; // false(2)

  gNgodDev.configStatus.asnType = ASN_INTEGER;
  gNgodDev.configStatus.asnValue.number = 5; // other(5)
}
static void freeNgodDevMib()
{
  SnmpUtilAsnAnyFree(&gNgodDev.dhcp);
  SnmpUtilAsnAnyFree(&gNgodDev.ntp);
  SnmpUtilAsnAnyFree(&gNgodDev.tftp);
  SnmpUtilAsnAnyFree(&gNgodDev.config);
}
typedef AsnInteger32 (*VarVisitor)(AsnAny *val);

// dhcp server: readonly
static AsnInteger32 getDhcp(AsnAny *val)
{
  SnmpUtilAsnAnyCpy(val, &gNgodDev.dhcp);
  return SNMP_ERRORSTATUS_NOERROR;
}

// ntp server: readonly
static AsnInteger32 getNtp(AsnAny *val)
{
  SnmpUtilAsnAnyCpy(val, &gNgodDev.ntp);
  return SNMP_ERRORSTATUS_NOERROR;
}

// tftp address type: readwrite
static AsnInteger32 getTftpType(AsnAny *val)
{
  SnmpUtilAsnAnyCpy(val, &gNgodDev.tftpType);
  return SNMP_ERRORSTATUS_NOERROR;
}
static AsnInteger32 setTftpType(AsnAny *val)
{
  return SNMP_ERRORSTATUS_BADVALUE;
}

// tftp address: readwrite
static AsnInteger32 getTftp(AsnAny *val)
{
  SnmpUtilAsnAnyCpy(val, &gNgodDev.tftp);
  return SNMP_ERRORSTATUS_NOERROR;
}
static AsnInteger32 setTftp(AsnAny *val)
{
  return SNMP_ERRORSTATUS_BADVALUE;
}

// config: readwrite
static AsnInteger32 getConfig(AsnAny *val)
{
  SnmpUtilAsnAnyCpy(val, &gNgodDev.config);
  return SNMP_ERRORSTATUS_NOERROR;
}
static AsnInteger32 setConfig(AsnAny *val)
{
  return SNMP_ERRORSTATUS_BADVALUE;
}

// config trigger: readwrite
static AsnInteger32 getConfigTrigger(AsnAny *val)
{
  SnmpUtilAsnAnyCpy(val, &gNgodDev.configTrigger);
  return SNMP_ERRORSTATUS_NOERROR;
}
static AsnInteger32 setConfigTrigger(AsnAny *val)
{
  return SNMP_ERRORSTATUS_BADVALUE;
}

// config status: readonly
static AsnInteger32 getConfigStatus(AsnAny *val)
{
  SnmpUtilAsnAnyCpy(val, &gNgodDev.configStatus);
  return SNMP_ERRORSTATUS_NOERROR;
}

struct ManagedObject
{
  AsnInteger32 type;
  VarVisitor get;
  VarVisitor set;  
};

ManagedObject gNgodDevServerVariables[] =
  {
    {ASN_IPADDRESS, getDhcp, NULL},
    {ASN_IPADDRESS, getNtp, NULL},
    {ASN_INTEGER, getTftpType, setTftpType},
    {ASN_OCTETSTRING, getTftp, setTftp},
    {ASN_OCTETSTRING, getConfig, setConfig},
    {ASN_INTEGER, getConfigTrigger, setConfigTrigger},
    {ASN_INTEGER, getConfigStatus, NULL}
  };
static AsnInteger32 getNgodDevObject(SnmpVarBind *pVarBind)
{
  // check the oid first
  if((pVarBind->name.idLength != gOidNgodDevServer.idLength + 1) ||
     (0 != SnmpUtilOidNCmp(&pVarBind->name, &gOidNgodDevServer, gOidNgodDevServer.idLength)))
    { // oid prefix not fit
      return SNMP_ERRORSTATUS_NOSUCHNAME;
    }
  UINT varId = pVarBind->name.ids[gOidNgodDevServer.idLength];
  if(varId < 1 || varId > (sizeof(gNgodDevServerVariables) / sizeof(ManagedObject)))
    { // variable not exist
      return SNMP_ERRORSTATUS_NOSUCHNAME;
    }
  const ManagedObject& var = gNgodDevServerVariables[varId - 1]; // 0-based array
  if(var.get)
    {
      SnmpUtilAsnAnyFree(&(pVarBind->value));
      return var.get(&(pVarBind->value));
    }
  else // no getter? impossible!
    {
      return SNMP_ERRORSTATUS_GENERR;
    }
}
static AsnInteger32 setNgodDevObject(SnmpVarBind *pVarBind)
{
  // check the oid first
  if((pVarBind->name.idLength != gOidNgodDevServer.idLength + 1) ||
     (0 != SnmpUtilOidNCmp(&pVarBind->name, &gOidNgodDevServer, gOidNgodDevServer.idLength)))
    { // oid prefix not fit
      return SNMP_ERRORSTATUS_NOSUCHNAME;
    }
  UINT varId = pVarBind->name.ids[gOidNgodDevServer.idLength];
  if(varId < 1 || varId > (sizeof(gNgodDevServerVariables) / sizeof(ManagedObject)))
    { // variable not exist
      return SNMP_ERRORSTATUS_NOSUCHNAME;
    }
  const ManagedObject& var = gNgodDevServerVariables[varId - 1]; // 0-based array
  if(var.set)
    {
      return var.set(&(pVarBind->value));
    }
  else // no setter: not writeable
    {
      return SNMP_ERRORSTATUS_READONLY;
    }
}

static AsnInteger32 getNextNgodDevObject(SnmpVarBind *pVarBind)
{
  UINT nextVarId = 0; // the next object's id
  // check the oid first
  int prefixRel = SnmpUtilOidNCmp(&pVarBind->name, &gOidNgodDevServer, gOidNgodDevServer.idLength);
  if(prefixRel > 0) // out of this region
    {
      return SNMP_ERRORSTATUS_NOSUCHNAME;
    }
  else if(prefixRel < 0)
    { // next object is the first entry
      nextVarId = 1;
    }
  else // prefix equal
    {
      if(pVarBind->name.idLength > gOidNgodDevServer.idLength)
	{
	  nextVarId = pVarBind->name.ids[gOidNgodDevServer.idLength] + 1;
	}
      else
	{
	  nextVarId = 1; // the first entry
	}
    }

  if(nextVarId < 1 || nextVarId > (sizeof(gNgodDevServerVariables) / sizeof(ManagedObject)))
    { // variabe not exist
      return SNMP_ERRORSTATUS_NOSUCHNAME;
    }

  const ManagedObject& var = gNgodDevServerVariables[nextVarId - 1]; // 0-based array
  if(var.get)
    {
      // update the oid
      SnmpUtilVarBindFree(pVarBind);
      pVarBind->name.idLength = gOidNgodDevServer.idLength + 1;
      pVarBind->name.ids = (UINT*)SnmpUtilMemAlloc(sizeof(UINT) * pVarBind->name.idLength);
      memcpy(pVarBind->name.ids, gOidNgodDevServer.ids, sizeof(UINT) * gOidNgodDevServer.idLength);
      pVarBind->name.ids[gOidNgodDevServer.idLength] = nextVarId;
      // update the value
      return var.get(&(pVarBind->value));
    }
  else // no getter? impossible!
    {
      return SNMP_ERRORSTATUS_GENERR;
    }
}
// get the root oid from registry
static std::string getNgodRootOid()
{
/*
#define Ngod_Snmp_Oid_Root ".1.3.6.1.4.1.22839.99"
#define Ngod_Snmp_Oid_Dev Ngod_Snmp_Oid_Root ".1"
#define Ngod_Snmp_Oid_DevMib Ngod_Snmp_Oid_Dev ".1"
#define Ngod_Snmp_Oid_DevServer Ngod_Snmp_Oid_DevMib ".1"
*/
  HKEY kNgodSnmp;
  LONG ret = ERROR_SUCCESS;
  ret = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\ZQ Interactive\\NgodSnmp", 0, KEY_READ, &kNgodSnmp);
  if( ERROR_SUCCESS != ret)
    {
      return "";
    }

  char rootOid[256] = {0};
  DWORD dwType = 0;
  DWORD dwSize = 255;
  ret = ::RegQueryValueEx(kNgodSnmp, "RootOid", NULL, &dwType, (unsigned char*)rootOid, &dwSize);
  ::RegCloseKey(kNgodSnmp);

  if(!(ERROR_SUCCESS == ret || REG_SZ == dwType))
    {
      rootOid[0] = '\0';
    }
  // verify the oid
  if(rootOid[0] != '\0' && rootOid[0] != '.')
    return (std::string(".") + rootOid);
  else
    return rootOid;
}
// When exported function will be called during DLL loading and initialization
BOOL SNMP_FUNC_TYPE SnmpExtensionInit(DWORD dwUptimeReference,
                                      HANDLE *phSubagentTrapEvent,
                                      AsnObjectIdentifier *pFirstSupportedRegion)
{
  std::string oidNgod = getNgodRootOid();
  if(oidNgod.empty())
    return FALSE;

  std::string oidNgodDev = oidNgod + ".1";
  std::string oidNgodDevServer = oidNgod + ".1.1.1";
  if(!SnmpMgrStrToOid((char*)oidNgodDev.c_str(), &gOidNgodDev))
      return FALSE;

  if(!SnmpMgrStrToOid((char*)oidNgodDevServer.c_str(), &gOidNgodDevServer))
    {
      SnmpUtilOidFree(&gOidNgodDev);
      return FALSE;
    }

  buildNgodDevMib();
  *phSubagentTrapEvent = NULL;
  *pFirstSupportedRegion = gOidNgodDev;
  return TRUE;
}

// this export is to query the MIB table and fields
BOOL SNMP_FUNC_TYPE SnmpExtensionQuery(BYTE bPduType, 
                                       SnmpVarBindList *pVarBindList, 
                                       AsnInteger32 *pErrorStatus, AsnInteger32 *pErrorIndex)
{
  for(UINT iVbl = 0; iVbl < pVarBindList->len; ++iVbl)
    {
        SnmpVarBind* pVb = &(pVarBindList->list[iVbl]);

        AsnInteger32 err = SNMP_ERRORSTATUS_GENERR;
        switch(bPduType)
        {
	    case SNMP_PDU_GET:
	      err = getNgodDevObject(pVb);
	      break;
	    case SNMP_PDU_GETNEXT:
	      err = getNextNgodDevObject(pVb);
	      break;
	    case SNMP_PDU_SET:
	      err = setNgodDevObject(pVb);
	      break;
	    default: // unsupported operation
          break;
        }
      if( err != SNMP_ERRORSTATUS_NOERROR )
	    {
	      *pErrorStatus = err;
	      *pErrorIndex = iVbl + 1; // 1-based
	    }
    }
  return TRUE;
}
/*
BOOL SNMP_FUNC_TYPE SnmpExtensionTrap(AsnObjectIdentifier *pEnterpriseOid, AsnInteger32 *pGenericTrapId, AsnInteger32 *pSpecificTrapId, AsnTimeticks *pTimeStamp, SnmpVarBindList *pVarBindList)
{
    return FALSE;
}
*/
VOID SNMP_FUNC_TYPE SnmpExtensionClose()
{
  SnmpUtilOidFree(&gOidNgodDev);
  SnmpUtilOidFree(&gOidNgodDevServer);
  freeNgodDevMib();
}
