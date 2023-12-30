/*
 * @file AlertId.h
 *
 *
 *  header file contains Defines and externs for AlertIds
 *  
 *
 *
 *  Revision History
 *  
 *  06-24-2010 	KH		Initial creation
 *
 */

#ifndef ALERTID_H
#define ALERTID_H

using namespace std;

//
// Defines for default in case entry not found in seamonlx.conf
//

//
// seamonlx general errors
//
#define			AI_FILE_MISSING					0x100
#define			AI_THREAD_NOT_RUNNING			0x101
#define			AI_SIGNAL_HANDLER_ERROR			0x102


//
// program errors
//
#define			AI_POPEN_ERROR					0x200
#define			AI_FOPEN_ERROR					0x201
#define			AI_MALLOC_FAILED				0x203
#define			AI_THREAD_FAILED_TO_START		0x204
#define			AI_SIGNAL_HANDLER_SETUP_FAILED	0x205
#define			AI_SOCKET_ERROR					0x206
#define			AI_FILE_DELETE_ERROR			0x207

// svcs chks
#define			AI_SVC_NOT_STARTED				0x300
#define			AI_SVC_NOT_CURRENT_RUN_LEVEL	0x301

// rpm chks
#define			AI_RPM_NOT_INSTALLED			0x400
#define			AI_RPM_WRONG_VERSION			0x401

//
// Disk status errors
//
#define			AI_DISK_OFFLINE					0x500
#define			AI_DISK_SMART_ERROR				0x501
#define			AI_DISK_TEMP_ERROR				0x502
#define			AI_DISK_IO_ERROR				0x503
#define			AI_DISK_BLKR_ERROR				0x504
#define			AI_DISK_BLKW_ERROR				0x505
#define			AI_DISK_UNKNOWN_ERROR			0x506

//
// Enclosure status errors
//
#define			AI_ENC_STATUS					0x600
#define			AI_ENC_PHY_LINK_STATUS			0x601
#define			AI_ENC_PHY_LINK_RATE			0x602
#define			AI_ENC_PHY_ERROR_COUNT			0x603
#define			AI_ENC_TEMP_STATUS				0x604
#define			AI_ENC_PWR_STATUS				0x605
#define			AI_ENC_FANS_STATUS				0x606
#define			AI_ENC_DISK_ELEM_SES_STATUS		0x607
#define			AI_ENC_UNKNOWN_ERROR			0x608

//
// Storage Adapter status errors
//
#define			AI_SA_STATUS					0x650
#define			AI_SA_PHY_LINK_STATUS			0x651

//
// Shas has its own erro ids
//
#define			AI_SHAS_FAIL_TO_GET_CORE_INFO	0x700
#define			AI_SHAS_PARSE_MSG_ERROR			0x701


//
// UDEV events
//
#define			AI_FAIL_ON_UPDATE_DISKS			0x800
#define			AI_UDEV_ADD_MODULE				0x801
#define			AI_UDEV_REMOVE_MODULE			0x802
#define			AI_UDEV_ADD_DISK				0x803
#define			AI_UDEV_REMOVE_DISK				0x804
#define			AI_UDEV_UNKNOWN_STATUS			0x805

//
// Alert specific error
//
#define			AI_ALERT_RANGE_ERROR			0x901
#define			AI_FAIL_TO_LOG					0x902

//
// Server Env status errors
//
#define			AI_SVRENV_FAN_STATUS			0xA00
#define			AI_SVRENV_PWR_STATUS			0xA01
#define			AI_SVRENV_TEMP_STATUS			0xA02
#define			AI_SVRENV_VOLT_STATUS			0xA03

//
// lspci changed
//
#define			AI_LSPCI_CHANGED				0xB00

//
// Network Adapter
//
#define			AI_NETWORK_ADAPTER_ERROR		0xC00
#define			AI_NETWORK_INTERFACE_ERROR		0xC01

//
// Infiniband Adapter
//
#define			AI_INFINIBAND_ADAPTER_ERROR		0xD00
#define			AI_INFINIBAND_INTERFACE_ERROR	0xD01

// 
// externs of Global ALertId ints
//
//
// seamonlx general errors
//
extern int			Gsv_AI_FILE_MISSING;
extern int			Gsv_AI_THREAD_NOT_RUNNING;
extern int			Gsv_AI_SIGNAL_HANDLER_ERROR;

//
// lspci  check
//
extern int			Gsv_AI_LSPCI_CHANGED;

//
// program errors
//
extern int			Gsv_AI_POPEN_ERROR;
extern int			Gsv_AI_FOPEN_ERROR;
extern int			Gsv_AI_FILE_DELETE_ERROR;
extern int			Gsv_AI_MALLOC_FAILED;
extern int			Gsv_AI_THREAD_FAILED_TO_START;
extern int			Gsv_AI_SIGNAL_HANDLER_SETUP_FAILED;
extern int			Gsv_AI_SOCKET_ERROR;

// service chks
extern int			Gsv_AI_SVC_NOT_STARTED;
extern int			Gsv_AI_SVC_NOT_CURRENT_RUN_LEVEL;

// rpm chks
extern int			Gsv_AI_RPM_NOT_INSTALLED;
extern int			Gsv_AI_RPM_WRONG_VERSION;

//
// Disk status errors
//
extern int			Gsv_AI_DISK_OFFLINE;
extern int			Gsv_AI_DISK_SMART_ERROR;
extern int			Gsv_AI_DISK_TEMP_ERROR;
extern int			Gsv_AI_DISK_IO_ERROR;
extern int			Gsv_AI_DISK_BLKR_ERROR;
extern int			Gsv_AI_DISK_BLKW_ERROR;
extern int			Gsv_AI_DISK_UNKNOWN_ERROR;

//
// Enclosure status errors
//
extern int			Gsv_AI_ENC_STATUS;
extern int			Gsv_AI_ENC_PHY_LINK_STATUS;
extern int			Gsv_AI_ENC_PHY_LINK_RATE;
extern int			Gsv_AI_ENC_PHY_ERROR_COUNT;
extern int			Gsv_AI_ENC_TEMP_STATUS;
extern int			Gsv_AI_ENC_PWR_STATUS;
extern int			Gsv_AI_ENC_FANS_STATUS;
extern int			Gsv_AI_ENC_DISK_ELEM_SES_STATUS;
extern int			Gsv_AI_ENC_UNKNOWN_ERROR;

//
// Storage Adapter status errors
//
extern int			Gsv_AI_SA_STATUS;
extern int			Gsv_AI_SA_PHY_LINK_STATUS;

//
// Server env
//
extern int			Gsv_AI_SVRENV_FAN_STATUS;
extern int			Gsv_AI_SVRENV_PWR_STATUS;
extern int			Gsv_AI_SVRENV_TEMP_STATUS;
extern int			Gsv_AI_SVRENV_VOLT_STATUS;

// Shas has its own erro ids
//
extern int			Gsv_AI_SHAS_FAIL_TO_GET_CORE_INFO;
extern int			Gsv_AI_SHAS_PARSE_MSG_ERROR;


//
// UDEV events
//
extern int			Gsv_AI_FAIL_ON_UPDATE_DISKS;
extern int			Gsv_AI_UDEV_ADD_MODULE;
extern int			Gsv_AI_UDEV_REMOVE_MODULE;
extern int			Gsv_AI_UDEV_ADD_DISK;
extern int			Gsv_AI_UDEV_REMOVE_DISK;
extern int			Gsv_AI_UDEV_UNKNOWN_STATUS;

//
// Alert specific error
//
extern int			Gsv_AI_ALERT_RANGE_ERROR;
extern int			Gsv_AI_FAIL_TO_LOG;

//
// Network Adapter Errors
//
extern int			Gsv_AI_NETWORK_ADAPTER_ERROR;
extern int			Gsv_AI_NETWORK_INTERFACE_ERROR;

//
// Infiniband Adapter Errors
//
extern int			Gsv_AI_INFINIBAND_ADAPTER_ERROR;
extern int			Gsv_AI_INFINIBAND_INTERFACE_ERROR;

#endif /* ALERTID_H */
