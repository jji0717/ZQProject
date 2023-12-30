#ifndef __ERM_SERVICE_SNMP_H
#define __ERM_SERVICE_SNMP_H

#include "EdgeRMEnv.h"
#include "../common/snmp/SubAgent.hpp"

namespace ZQTianShan {
namespace EdgeRM {

class EnvSnmpRegistor
{
public:
	EnvSnmpRegistor();
	~EnvSnmpRegistor();

	bool regSnmp(ZQTianShan::EdgeRM::EdgeRMEnv* edgeRmEnv);
	bool unRegSnmp(void);

private:
	ZQ::Snmp::SubAgent* _ermSnmpAgent;
};

}} // namespace


#endif//__ERM_SERVICE_SNMP_H