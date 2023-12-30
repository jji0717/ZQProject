
/*
 * 
 * AlertId.cpp
 *
 *
 * File contains Global AlertIdints
 *  
 *
 *
 *  Revision History
 *  
 *  06-24-2010 Created ( Kim Hemphill)
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include "common.h"

using namespace std;

//
// Global AlertIdint declarations
//
//
// seamonlx general errors
//
int			Gsv_AI_FILE_MISSING					= AI_FILE_MISSING;
int			Gsv_AI_THREAD_NOT_RUNNING			= AI_THREAD_NOT_RUNNING;
int			Gsv_AI_SIGNAL_HANDLER_ERROR			= AI_SIGNAL_HANDLER_ERROR;

//
// lspci check
//
int			Gsv_AI_LSPCI_CHANGED				= AI_LSPCI_CHANGED;
//
// program errors
//
int			Gsv_AI_POPEN_ERROR					= AI_POPEN_ERROR;
int			Gsv_AI_FILE_DELETE_ERROR			= AI_FILE_DELETE_ERROR;
int			Gsv_AI_FOPEN_ERROR					= AI_FOPEN_ERROR;
int			Gsv_AI_MALLOC_FAILED				= AI_MALLOC_FAILED;
int			Gsv_AI_THREAD_FAILED_TO_START		= AI_THREAD_FAILED_TO_START;
int			Gsv_AI_SIGNAL_HANDLER_SETUP_FAILED	= AI_SIGNAL_HANDLER_SETUP_FAILED;
int			Gsv_AI_SOCKET_ERROR					= AI_SOCKET_ERROR;

// service chks
int			Gsv_AI_SVC_NOT_STARTED				= AI_SVC_NOT_STARTED;
int			Gsv_AI_SVC_NOT_CURRENT_RUN_LEVEL	= AI_SVC_NOT_CURRENT_RUN_LEVEL;

// rpm chks
int			Gsv_AI_RPM_NOT_INSTALLED			= AI_RPM_NOT_INSTALLED;
int			Gsv_AI_RPM_WRONG_VERSION			= AI_RPM_WRONG_VERSION;

//
// Disk status errors
//
int			Gsv_AI_DISK_OFFLINE					= AI_DISK_OFFLINE;
int			Gsv_AI_DISK_SMART_ERROR				= AI_DISK_SMART_ERROR;
int			Gsv_AI_DISK_TEMP_ERROR				= AI_DISK_TEMP_ERROR;
int			Gsv_AI_DISK_IO_ERROR				= AI_DISK_IO_ERROR;
int			Gsv_AI_DISK_BLKR_ERROR				= AI_DISK_BLKR_ERROR;
int			Gsv_AI_DISK_BLKW_ERROR				= AI_DISK_BLKW_ERROR;
int			Gsv_AI_DISK_UNKNOWN_ERROR			= AI_DISK_UNKNOWN_ERROR;

//
// Enclosure status errors
//
int			Gsv_AI_ENC_STATUS					= AI_ENC_STATUS;
int			Gsv_AI_ENC_PHY_LINK_STATUS			= AI_ENC_PHY_LINK_STATUS;
int			Gsv_AI_ENC_PHY_LINK_RATE			= AI_ENC_PHY_LINK_RATE;
int			Gsv_AI_ENC_PHY_ERROR_COUNT			= AI_ENC_PHY_ERROR_COUNT;
int			Gsv_AI_ENC_TEMP_STATUS				= AI_ENC_TEMP_STATUS;
int			Gsv_AI_ENC_PWR_STATUS				= AI_ENC_PWR_STATUS;
int			Gsv_AI_ENC_FANS_STATUS				= AI_ENC_FANS_STATUS;
int			Gsv_AI_ENC_DISK_ELEM_SES_STATUS		= AI_ENC_DISK_ELEM_SES_STATUS;
int			Gsv_AI_ENC_UNKNOWN_ERROR			= AI_ENC_UNKNOWN_ERROR;

//
// Storage Adapter Errors
//
int			Gsv_AI_SA_STATUS					= AI_SA_STATUS;
int			Gsv_AI_SA_PHY_LINK_STATUS			= AI_SA_PHY_LINK_STATUS;

//
// Shas has its own erro ids
//
int			Gsv_AI_SHAS_FAIL_TO_GET_CORE_INFO	= AI_SHAS_FAIL_TO_GET_CORE_INFO;
int			Gsv_AI_SHAS_PARSE_MSG_ERROR			= AI_SHAS_PARSE_MSG_ERROR;


//
// UDEV events
//
int			Gsv_AI_FAIL_ON_UPDATE_DISKS			= AI_FAIL_ON_UPDATE_DISKS;
int			Gsv_AI_UDEV_ADD_MODULE				= AI_UDEV_ADD_MODULE;
int			Gsv_AI_UDEV_REMOVE_MODULE			= AI_UDEV_REMOVE_MODULE;
int			Gsv_AI_UDEV_ADD_DISK				= AI_UDEV_ADD_DISK;
int			Gsv_AI_UDEV_REMOVE_DISK				= AI_UDEV_REMOVE_DISK;
int			Gsv_AI_UDEV_UNKNOWN_STATUS			= AI_UDEV_UNKNOWN_STATUS;

//
// Alert specific error
//
int			Gsv_AI_ALERT_RANGE_ERROR			= AI_ALERT_RANGE_ERROR;
int			Gsv_AI_FAIL_TO_LOG					= AI_FAIL_TO_LOG;

//
// Server Env status
//
int			Gsv_AI_SVRENV_FAN_STATUS			= AI_SVRENV_FAN_STATUS;
int			Gsv_AI_SVRENV_PWR_STATUS			= AI_SVRENV_PWR_STATUS;
int			Gsv_AI_SVRENV_TEMP_STATUS			= AI_SVRENV_TEMP_STATUS;
int			Gsv_AI_SVRENV_VOLT_STATUS			= AI_SVRENV_VOLT_STATUS;

//
// Network Adapter Errors
//
int			Gsv_AI_NETWORK_ADAPTER_ERROR		= AI_NETWORK_ADAPTER_ERROR;
int			Gsv_AI_NETWORK_INTERFACE_ERROR		= AI_NETWORK_INTERFACE_ERROR;

//
// Infiniband Adapter Errors
//
int			Gsv_AI_INFINIBAND_ADAPTER_ERROR		= AI_INFINIBAND_ADAPTER_ERROR;
int			Gsv_AI_INFINIBAND_INTERFACE_ERROR	= AI_INFINIBAND_INTERFACE_ERROR;
