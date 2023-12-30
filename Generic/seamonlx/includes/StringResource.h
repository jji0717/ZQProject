/*
 * @file StringResource.h
 *
 *  Header file contains common utility functions for the String Resource Library
 *  
 * NOTE       //\\//\\//\\//\\//\\//\\//\\//\\//\\//
 *
 * If you add or change any of the Object and Component definitions within 
 * this file, you MUST update the corresponding StringResource.ini file that 
 * directly correletes with this file.
 * Failure to do so will render this file incompatible with the runtime strings 
  * library and may completely cripple the runtime of seamonlx.
 *
 * YOU HAVE BEEN WARNED !
 *
 *  Revision History
 *  
 *  05-26-2010 	mjc  	Created
 *  
 * 
 */

#ifndef STRINGRESOURCE_H
#define STRINGRESOURCE_H

#include <vector>
#include <utility>

#include "common.h"

using namespace std;


#define			STRRES_DEFAULT_STRING					"??????"
#define			STRRES_COMPONENT_NAME_LENGTH			64
#define			STRRES_MAX_STRING_LENGTH				256

/*
*   FACILITY DEFINITIONS
*
*  Facilities are two byte values (two hex digits)
*
*  Each Facility can equate to an system object, system thread, system task, etc.
*
*/


//
// SEAMONLX MAIN PROGRAM + COMMON BASE CODE
// PUT NON OBJECT COMPONENTS UNDER THIS LIST
//
#define		FACILITY_SEAMONLX_BASE_VAL						0x1000
#define		FACILITY_SEAMONLX_MAIN							FACILITY_SEAMONLX_BASE_VAL + 0x0000
#define		FACILITY_SEAMONLX_XMLRPC						FACILITY_SEAMONLX_BASE_VAL + 0x0100
#define		FACILITY_SEAMONLX_COMMON						FACILITY_SEAMONLX_BASE_VAL + 0x0200
#define		FACILITY_SEAMONLX_TRACE_CLASS					FACILITY_SEAMONLX_BASE_VAL + 0x0300
#define		FACILITY_SEAMONLX_MONITORING					FACILITY_SEAMONLX_BASE_VAL + 0x0400

//
// OBJECTS
//
#define		FACILITY_OBJECTS_BASE_VAL							0xA000
#define		FACILITY_SERVER_HW									FACILITY_OBJECTS_BASE_VAL + 0x0000
#define		FACILITY_SERVER_ENV									FACILITY_OBJECTS_BASE_VAL + 0x0100
#define		FACILITY_STORAGE_ADAPTERS							FACILITY_OBJECTS_BASE_VAL + 0x0200
#define		FACILITY_ENCLOSURES									FACILITY_OBJECTS_BASE_VAL + 0x0300
#define		FACILITY_DISKS										FACILITY_OBJECTS_BASE_VAL + 0x0400
#define		FACILITY_SHAS_CONFIG								FACILITY_OBJECTS_BASE_VAL + 0x0500
#define		FACILITY_SHAS_COUNTERS								FACILITY_OBJECTS_BASE_VAL + 0x0600
#define		FACILITY_SYSTEM_ALERT								FACILITY_OBJECTS_BASE_VAL + 0x0700
#define		FACILITY_MANAGEMENT_PORT							FACILITY_OBJECTS_BASE_VAL + 0x0800
#define		FACILITY_SYSTEM_HEALTH								FACILITY_OBJECTS_BASE_VAL + 0x0900

/*
*   SEVERITY DEFINITIONS
*
*  Severity are one byte values (two hex digits)
* 	Severity can be ignored if it is irrelevent
*
*		ignored	- Can be ignored as it's not any of the below conditions
*		emerg 	- Emergency condition, such as an imminent system crash, usually broadcast to all users
* 		alert		- Condition that should be corrected immediately, such as a corrupted system database
*		crit		- Critical condition, such as a hardware error
*		err		- Ordinary error
*		warning - Warning
*		notice	- Condition that is not an error, but possibly should be handled in a special way
*		info		- Informational message
*		debug	- Messages that are used when debugging programs
*
*/

#define		SEVERITY_BASE_VAL					0x00
#define		SEVERITY_IGNORED					SEVERITY_BASE_VAL + 0x00
#define		SEVERITY_EMERGENCY					SEVERITY_BASE_VAL + 0x01
#define		SEVERITY_ALERT						SEVERITY_BASE_VAL + 0x02
#define		SEVERITY_CRITICAL					SEVERITY_BASE_VAL + 0x03
#define		SEVERITY_ERROR						SEVERITY_BASE_VAL + 0x04
#define		SEVERITY_WARNING					SEVERITY_BASE_VAL + 0x05
#define		SEVERITY_NOTICE						SEVERITY_BASE_VAL + 0x06
#define		SEVERITY_INFO						SEVERITY_BASE_VAL + 0x07
#define		SEVERITY_DEBUG						SEVERITY_BASE_VAL + 0x08


/*
* 	FACILITY / COMPONENT   ERROR CODE DEFINITIONS
*
*  These are 32 bit values (8 digit hex) and are Facility specific
*
*/

//
// FACILITY_SEAMONLX_COMMON  (Facility ID 0x001; Subcomponent 0x2)
//
#define					FAC_SEA_COM_ERROR_21				0x21
#define					FAC_SEA_COM_ERROR_22				0x22
#define					FAC_SEA_COM_ERROR_21_FULL		0x0012000000000021

//
// FACILITY_SHAS_CONFIG (Facility ID 0xA5)
//
#define					FAC_SHAS_CONFIG_ERROR_1							0x1

#define					FAC_SHAS_CONFIG_CD_HDR_STRING					0x20
#define					FAC_SHAS_CONFIG_PD_HDR_STRING					0x21
#define					FAC_SHAS_CONFIG_LD_HDR_STRING					0x22
#define					FAC_SHAS_CONFIG_LE_HDR_STRING					0x23
#define					FAC_SHAS_CONFIG_ENCLOSURE_HDR_STRING			0x24
#define					FAC_SHAS_CONFIG_BAY_HDR_STRING					0x25
#define					FAC_SHAS_CONFIG_HANDLE_HDR_STRING				0x26


/*
*	Reccomendations	
*/
#define     FACILITY_SEMONLX_RECC						0x001000000000F000
//
// seamonlx general errors
//
#define			RECC_AI_FILE_MISSING					(FACILITY_SEMONLX_RECC + AI_FILE_MISSING)
#define			RECC_AI_THREAD_NOT_RUNNING				(FACILITY_SEMONLX_RECC + AI_THREAD_NOT_RUNNING)
#define			RECC_AI_SIGNAL_HANDLER_ERROR			(FACILITY_SEMONLX_RECC + AI_SIGNAL_HANDLER_ERROR)

//
// lspci check
//
#define			RECC_AI_LSPCI_CHANGED					(FACILITY_SEMONLX_RECC + AI_LSPCI_CHANGED)

//
// program errors
//
#define			RECC_AI_POPEN_ERROR						(FACILITY_SEMONLX_RECC + AI_POPEN_ERROR)
#define			RECC_AI_FOPEN_ERROR						(FACILITY_SEMONLX_RECC + AI_FOPEN_ERROR)
#define			RECC_AI_MALLOC_FAILED					(FACILITY_SEMONLX_RECC + AI_MALLOC_FAILED)
#define			RECC_AI_THREAD_FAILED_TO_START			(FACILITY_SEMONLX_RECC + AI_THREAD_FAILED_TO_START)
#define			RECC_AI_SIGNAL_HANDLER_SETUP_FAILED		(FACILITY_SEMONLX_RECC + AI_SIGNAL_HANDLER_SETUP_FAILED)
#define			RECC_AI_SOCKET_ERROR					(FACILITY_SEMONLX_RECC + AI_SOCKET_ERROR)
#define			RECC_AI_FILE_DELETE_ERROR				(FACILITY_SEMONLX_RECC + AI_FILE_DELETE_ERROR)

// svcs chks
#define			RECC_AI_SVC_NOT_STARTED					(FACILITY_SEMONLX_RECC + AI_SVC_NOT_STARTED)
#define			RECC_AI_SVC_NOT_CURRENT_RUN_LEVEL		(FACILITY_SEMONLX_RECC + AI_SVC_NOT_CURRENT_RUN_LEVEL)

// rpm chks
#define			RECC_AI_RPM_NOT_INSTALLED				(FACILITY_SEMONLX_RECC + AI_RPM_NOT_INSTALLED)
#define			RECC_AI_RPM_WRONG_VERSION				(FACILITY_SEMONLX_RECC + AI_RPM_WRONG_VERSION)

//
// Disk status errors
//
#define			RECC_AI_DISK_OFFLINE					(FACILITY_SEMONLX_RECC + AI_DISK_OFFLINE)
#define			RECC_AI_DISK_SMART_ERROR				(FACILITY_SEMONLX_RECC + AI_DISK_SMART_ERROR)
#define			RECC_AI_DISK_TEMP_ERROR					(FACILITY_SEMONLX_RECC + AI_DISK_TEMP_ERROR)
#define			RECC_AI_DISK_IO_ERROR					(FACILITY_SEMONLX_RECC + AI_DISK_IO_ERROR)
#define			RECC_AI_DISK_BLKR_ERROR					(FACILITY_SEMONLX_RECC + AI_DISK_BLKR_ERROR)
#define			RECC_AI_DISK_BLKW_ERROR					(FACILITY_SEMONLX_RECC + AI_DISK_BLKW_ERROR)
#define			RECC_AI_DISK_UNKNOWN_ERROR				(FACILITY_SEMONLX_RECC + AI_DISK_UNKNOWN_ERROR)

//
// Enclosure status errors
//
#define			RECC_AI_ENC_STATUS						(FACILITY_SEMONLX_RECC + AI_ENC_STATUS)
#define			RECC_AI_ENC_PHY_LINK_STATUS				(FACILITY_SEMONLX_RECC + AI_ENC_PHY_LINK_STATUS)
#define			RECC_AI_ENC_PHY_LINK_RATE				(FACILITY_SEMONLX_RECC + AI_ENC_PHY_LINK_RATE)
#define			RECC_AI_ENC_PHY_ERROR_COUNT				(FACILITY_SEMONLX_RECC + AI_ENC_PHY_ERROR_COUNT)
#define			RECC_AI_ENC_TEMP_STATUS					(FACILITY_SEMONLX_RECC + AI_ENC_TEMP_STATUS)
#define			RECC_AI_ENC_PWR_STATUS					(FACILITY_SEMONLX_RECC + AI_ENC_PWR_STATUS)
#define			RECC_AI_ENC_FANS_STATUS					(FACILITY_SEMONLX_RECC + AI_ENC_FANS_STATUS)
#define			RECC_AI_ENC_DISK_ELEM_SES_STATUS		(FACILITY_SEMONLX_RECC + AI_ENC_DISK_ELEM_SES_STATUS)
#define			RECC_AI_ENC_UNKNOWN_ERROR				(FACILITY_SEMONLX_RECC + AI_ENC_UNKNOWN_ERROR)

//
// Storage Adapter status errors
//
#define			RECC_AI_SA_STATUS						(FACILITY_SEMONLX_RECC + AI_SA_STATUS)
#define			RECC_AI_SA_PHY_LINK_STATUS				(FACILITY_SEMONLX_RECC + AI_SA_PHY_LINK_STATUS)

//
// Shas has its own erro ids
//
#define			RECC_AI_SHAS_FAIL_TO_GET_CORE_INFO		(FACILITY_SEMONLX_RECC + AI_SHAS_FAIL_TO_GET_CORE_INFO)
#define			RECC_AI_SHAS_PARSE_MSG_ERROR			(FACILITY_SEMONLX_RECC + AI_SHAS_PARSE_MSG_ERROR)


//
// UDEV events
//
#define			RECC_AI_FAIL_ON_UPDATE_DISKS			(FACILITY_SEMONLX_RECC + AI_FAIL_ON_UPDATE_DISKS)
#define			RECC_AI_UDEV_ADD_MODULE					(FACILITY_SEMONLX_RECC + AI_UDEV_ADD_MODULE)
#define			RECC_AI_UDEV_REMOVE_MODULE				(FACILITY_SEMONLX_RECC + AI_UDEV_REMOVE_MODULE)
#define			RECC_AI_UDEV_ADD_DISK					(FACILITY_SEMONLX_RECC + AI_UDEV_ADD_DISK)
#define			RECC_AI_UDEV_REMOVE_DISK				(FACILITY_SEMONLX_RECC + AI_UDEV_REMOVE_DISK)
#define			RECC_AI_UDEV_UNKNOWN_STATUS				(FACILITY_SEMONLX_RECC + AI_UDEV_UNKNOWN_STATUS)

//
// Alert specific error
//
#define			RECC_AI_ALERT_RANGE_ERROR				(FACILITY_SEMONLX_RECC + AI_ALERT_RANGE_ERROR)
#define			RECC_AI_FAIL_TO_LOG						(FACILITY_SEMONLX_RECC + AI_FAIL_TO_LOG)

//
// Server Env status errors
//
#define			RECC_AI_SVRENV_FAN_STATUS				(FACILITY_SEMONLX_RECC + AI_SVRENV_FAN_STATUS)
#define			RECC_AI_SVRENV_PWR_STATUS				(FACILITY_SEMONLX_RECC + AI_SVRENV_PWR_STATUS)
#define			RECC_AI_SVRENV_TEMP_STATUS				(FACILITY_SEMONLX_RECC + AI_SVRENV_TEMP_STATUS)
#define			RECC_AI_SVRENV_VOLT_STATUS				(FACILITY_SEMONLX_RECC + AI_SVRENV_VOLT_STATUS)

/*
* Defines and Macros
*
*/

// Creates the correct full 64 bit value based on facility and error
#define		MAKE_STRRES( fac, er )  (((unsigned long)fac << 40) | ((unsigned long)er))



/*
*  Function prototypes
*
*/
bool 				StringResourceInitialize( void );
bool 				StringResourceLookup( unsigned long id, string &returnString );
bool				StringResourceLookup( unsigned long facility, unsigned long errId, string &returnString );
bool 				StringResourceLookup( unsigned long id, char *returnString );
bool				StringResourceLookup( unsigned long facility, unsigned long errId, char *returnString );
void 				DumpStringResources( void );


/*
*
* Structures and Types
*
*/

typedef union _fseStruct
	{
		unsigned long					fseValue;

			// Note that we are running on a Big Endian machine
			// so there order here does matter in terms of the INI file field format
			struct  _idStruct {
				unsigned long		error				: 32;
				unsigned long		_reserved 			: 8;
				unsigned long		severity			: 8;
				unsigned long		facility			: 16;
			} idStruct;

} fseStruct;

typedef  	struct   stStringResourceTable {

		char			componentName[STRRES_COMPONENT_NAME_LENGTH];
		char			subComponentName[STRRES_COMPONENT_NAME_LENGTH];
		fseStruct	idValue;
		string		returnErrorString;

} tStringResourceTable;


#endif /* STRINGRESOURCE_H */
