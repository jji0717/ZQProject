
#ifndef _ZQ_LISTINFO_DEF_H_
#define _ZQ_LISTINFO_DEF_H_

#include "mpfcommon.h"

// info query keys
//////////////////////////////////////////////////////////////////////////

//////// query method param keys
#define INFOTYPE_KEY			MPF_RESERVED_PREFIX "InfoType"
#define INFOPARAM_KEY			MPF_RESERVED_PREFIX "InfoParam"		

//////// query type values
#define INFOTYPE_NODETYPE 		MPF_RESERVED_PREFIX "NodeType"
#define INFOTYPE_GENERAL 		MPF_RESERVED_PREFIX "General"
#define INFOTYPE_TASK_TYPE		MPF_RESERVED_PREFIX "TaskType"
#define INFOTYPE_TASKDETAIL		MPF_RESERVED_PREFIX "TaskDetails"
#define INFOTYPE_TASK			MPF_RESERVED_PREFIX "Task"
#define INFOTYPE_WORKNODE		MPF_RESERVED_PREFIX "WorkNode"
#define INFOTYPE_SESSIONDETAIL	MPF_RESERVED_PREFIX "SessionDetails"
#define INFOTYPE_SESSIONS		MPF_RESERVED_PREFIX "Session"

//////// query item keys for each page

// General Page
#define INFO_OS_KEY				MPF_RESERVED_PREFIX "OS"
#define INFO_CPU_KEY			MPF_RESERVED_PREFIX "CPU"
#define INFO_MEMORY_KEY			MPF_RESERVED_PREFIX "Memory"
#define INFO_NETWORK_KEY		MPF_RESERVED_PREFIX "Network" /*reserved*/
#define INFO_PROCESS_KEY		MPF_RESERVED_PREFIX "Process"
#define INFO_MPFVERSION_KEY		MPF_RESERVED_PREFIX "MPFVersion"
#define INFO_INTERFACE_KEY		MPF_RESERVED_PREFIX "Interface"
#define INFO_NODEID_KEY			MPF_RESERVED_PREFIX "NodeID"

// Task Type Property Page
#define INFO_TYPENAME_KEY		MPF_RESERVED_PREFIX "TypeName"
#define INFO_INSTANCES_KEY		MPF_RESERVED_PREFIX "Instances"
#define INFO_AVAILABLE_KEY		MPF_RESERVED_PREFIX "Available"
#define INFO_PLUGIN_KEY			MPF_RESERVED_PREFIX "Plugin"
#define INFO_VENDOR_KEY			MPF_RESERVED_PREFIX "Vendor"

// Task Property Page
#define INFO_TASKURL_KEY		MPF_RESERVED_PREFIX "TaskURL"
#define INFO_SESSIONURL_KEY		MPF_RESERVED_PREFIX "SessionURL"
#define INFO_STARTTIME_KEY		MPF_RESERVED_PREFIX "StartTime"
#define INFO_LASTUPDATE_KEY		MPF_RESERVED_PREFIX "LastUpdateTime"
#define INFO_STATUS_KEY			MPF_RESERVED_PREFIX "Status"

// Task Details Page
#define INFO_TASKID_KEY			MPF_RESERVED_PREFIX "TaskID"

// Work Nodes Property Page
#define INFO_NODEID_KEY			MPF_RESERVED_PREFIX "NodeID"
#define INFO_IP_KEY				MPF_RESERVED_PREFIX "IP"
#define INFO_PORT_KEY			MPF_RESERVED_PREFIX "Port"
#define INFO_LASTHB_KEY			MPF_RESERVED_PREFIX "LastHeartbeat"
#define INFO_LEASETERM_KEY		MPF_RESERVED_PREFIX "LeaseTerm"
#define INFO_HARDWARE_KEY		MPF_RESERVED_PREFIX "Hardware"

// Session Property Page
#define INFO_CREATETIME_KEY		MPF_RESERVED_PREFIX "CreateTime"

// Session Details Page
#define INFO_SESSIONID_KEY		MPF_RESERVED_PREFIX "SessionID"


// Node type
#define NODE_TYPE_UNKNOWN		0
#define NODE_TYPE_WORKNODE		1
#define NODE_TYPE_MANAGERNODE	2
#define NODE_TYPE_MIXED			3

#endif