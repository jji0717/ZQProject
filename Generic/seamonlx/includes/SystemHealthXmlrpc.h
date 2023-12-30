/*
 * 
 * SystemHealthXml.h
 *
 *
 *  SystemHealthXmlrpc class definition. The xmlrpc_c responses
 *  of system health information
 *  Derived from xmlrpc_c::method.
 *  
 *
 *
 *  Revision History
 *  
 *  04-12-2010 mjc  Created ()
 *  
 * 
 */

#ifndef SYSTEMHEALTHXMLRPC_H
#define SYSTEMHEALTHXMLRPC_H

#include <xmlrpc-c/base.hpp>

#include "SystemHealth.h"
#include "ServerEnvXmlrpc.h"
#include "MgmtPortXmlrpc.h"


using namespace std;
using namespace seamonlx;

#define   REASON_CODE_BARF_ERROR  	"Error"

/**
 * A class derived from xmlrpc_c::method and seamonlx::SystemHealth
 */ 
class SystemHealthXmlrpc : public xmlrpc_c::method, public SystemHealth {
	
  public:

	/**
	 * A constructor
	 */ 
	SystemHealthXmlrpc();

	/**
	 * A member function.
	 * Extends the function execute in base class xmlrpc_c::method.
	 */ 
    void execute(xmlrpc_c::paramList const& paramList,
				 xmlrpc_c::value *   const  retvalP);

	/**
	 * A member function.
	 * Accessor to the member healthResp
	 */
	xmlrpc_c::value getHealthResp() const{
		return healthResp;
	}

	void updateAllHealth(void);
	
	void buildHealthResp(xmlrpc_c::value &result);

	/*
	 * 
	 * Class Object helper methods 
	 * 
	 */
    /* Server Env Class object */
	inline void setObjClassPointer( ServerEnvXmlrpc *pServerEnv ){
            pServerEnvObject = pServerEnv;
		}
    /* Management Port Class object */
	inline void setObjClassPointer( MgmtPortXmlrpc *pMgmtPort ){
            pMgmtPortObject = pMgmtPort;
		}
    /* Target Ports Class object */
	//inline void setObjClassPointer( MgmtPortXmlrpc *pTargetPorts ){
//            pTargetPortsObject = pTargetPorts;
//		}


		
	inline ObjectHealthState::ObjHealthStateEnumerator getServerEnvHealth( string &causeString ) {
		    causeString = (pServerEnvObject ? pServerEnvObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
			return ( pServerEnvObject ? pServerEnvObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );
		}

		inline ObjectHealthState::ObjHealthStateEnumerator getMgmtPortHealth( string causeString ) {
		    causeString = (pMgmtPortObject ? pMgmtPortObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
			return ( pMgmtPortObject ? pMgmtPortObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );
		}

		inline ObjectHealthState::ObjHealthStateEnumerator getTargetPortsHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pTargetPortsObject ? pTargetPortsObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pTargetPortsObject ? pTargetPortsObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
		
	inline ObjectHealthState::ObjHealthStateEnumerator getOpenIBHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pOpenIBObject ? pOpenIBObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pOpenIBObject ? pOpenIBObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getSeamonLXHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pSeamonLXObject ? pSeamonLXObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pSeamonLXObject ? pSeamonLXObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getEnclosureEnvHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pEnclosureEnvObject ? pEnclosureEnvObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pEnclosureEnvObject ? pEnclosureEnvObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getStorageInterconnectHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pStorageInterconnectObject ? pStorageInterconnectObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pStorageInterconnectObject ? pStorageInterconnectObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );			
			return ( ObjectHealthState::STATE_UNKNOWN );		
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getStorageConfigurationHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pStorageConfigurationObject ? pStorageConfigurationObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pStorageConfigurationObject ? pStorageConfigurationObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );		
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getSHASStateHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pStorageConfigurationObject ? pStorageConfigurationObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pStorageConfigurationObject ? pStorageConfigurationObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );		
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getHyperFSHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pHyperFSObject ? pHyperFSObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pHyperFSObject ? pHyperFSObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );		
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getIPStoreHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pIPStoreObject ? pIPStoreObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pIPStoreObject ? pIPStoreObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );		
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getCIFSHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pCIFSObject ? pCIFSObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pCIFSObject ? pCIFSObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );					
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getFTPHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pFTPObject ? pFTPObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pFTPObject ? pFTPObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );		
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getSoftwareConfigHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pSoftwareConfigObject ? pSoftwareConfigObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pSoftwareConfigObject ? pSoftwareConfigObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getSystemServicesHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pSystemServicesObject ? pSystemServicesObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pSystemServicesObject ? pSystemServicesObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getStreamSmithHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pStreamSmithObject ? pStreamSmithObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pStreamSmithObject ? pStreamSmithObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getVFlowHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pVFlowObject ? pVFlowObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pVFlowObject ? pVFlowObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getSeaFSHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pVFlowObject ? pVFlowObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pVFlowObject ? pVFlowObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getSparseCacheHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pSparseCacheObject ? pSparseCacheObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pSparseCacheObject ? pSparseCacheObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getDistributedCacheHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pDistributedCacheObject ? pDistributedCacheObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pDistributedCacheObject ? pDistributedCacheObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getSentryServiceHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pSentryServiceObject ? pSentryServiceObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pSentryServiceObject ? pSentryServiceObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
	inline ObjectHealthState::ObjHealthStateEnumerator getC2ServerHealth( string causeString ) {
			causeString = "Unimplemented";
//		    causeString = (pC2ServerObject ? pC2ServerObject -> getHealthReasonDescription() : REASON_CODE_BARF_ERROR );
//			return ( pC2ServerObject ? pC2ServerObject -> getHealthState() : ObjectHealthState::STATE_UNKNOWN );	
			return ( ObjectHealthState::STATE_UNKNOWN );
		}
		
	/*
	 *
	 * Convert health string helper method 
	 *
	 */
	inline string getHealthStateString( ObjectHealthState::ObjHealthStateEnumerator state ) {

		switch( state ) {
		    case ObjectHealthState::STATE_OK:
				return string("Ok");
				break;
				
			case ObjectHealthState::STATE_DEGRADED:
				return string("Degraded");
				break;

		    case ObjectHealthState::STATE_CRITICAL:
				return string("Critical");
				break;
				
			case ObjectHealthState::STATE_UNKNOWN:
				return string("Unknown");
				break;
			}
      return string("UNDEFINED");
	}
	
  protected:

	
	
  private:
	
	xmlrpc_c::value     healthResp;

	// Pointer to object class instances
	ServerEnvXmlrpc     *pServerEnvObject;
	MgmtPortXmlrpc     *pMgmtPortObject;
	
		
	
};

#endif /* SYSTEMHEALTHHWXML_H */
