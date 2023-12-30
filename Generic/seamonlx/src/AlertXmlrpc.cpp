/** @File AlertXmlrpc.cpp
 *
 *
 *  SystemAlertXmlrpc class constructors and member functions
 *  implementation.
 *  
 *
 *
 *  Revision History
 *  
 *  03-10-2010 Created ( jie.zhang@schange.com)
 *  06-14-2010  mjc  Fixed memory leak: 	Do not call result.instantiate(), rather use result = finalResp;
 *  
 * 
 */

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include "AlertXmlrpc.h"
#include "common.h"


extern int getAlert(char *seqnum, ALERT_STRUCT *alert, FILE **fp, int *fileisopen, int doclose);
extern int getAlertRange(ALERT_RANGE *alertrange);

using namespace std;

/**
 * default constructor
 */

SystemAlertXmlrpc::SystemAlertXmlrpc()
{
	char	locstr[1024];
	string	formattedstring;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::SystemAlertXmlrpc, Enter");

	this->_signature = "A:s" ;

	sprintf(locstr, "For best viewing ensure your screen width is at least %d characters", (HELPLINEEND + 10));
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", "SystemAlert [AlertRange, Alert, AlertSequence, AlertComponent]");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","1) SystemAlert AlertRange");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     AlertRange - no additional params");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          returns 1st and last Sequence Num");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","2) SystemAlert Alert Seqxx");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","     Alert - 1 additional param Seqxx, returns Alert data");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          for given Sequence Number");
	FormatHelpString(formattedstring, locstr);

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","3) SystemAlert AlertSequence Seqxx Seqyy");
	FormatHelpString(formattedstring, locstr);	
	sprintf(locstr, "%s","     AlertSequence  - 2 additional params, request range of");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","          Seqxx Seqyy, returns Array of Alert data");
	FormatHelpString(formattedstring, locstr);	

	sprintf(locstr, "%s", " ");
	FormatHelpString(formattedstring, locstr);
	sprintf(locstr, "%s","4) SystemAlert AlertComponent Compname");
	FormatHelpString(formattedstring, locstr);	
	sprintf(locstr, "%s","     AlertComponent Compname - 1 additional param");
	FormatHelpString(formattedstring, locstr);	
	sprintf(locstr, "%s","          returns Array of Component Type");
	FormatHelpString(formattedstring, locstr);	
		
	this->_help = formattedstring;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::SystemAlertXmlrpc, Exit");
}
//
// helper function to check for existence of component name
//
int VerifyCponentName(string paramname)
{
	int rtn = FAILURE;
	if (paramname.compare(CN_Server_Environmentals) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Management_Port) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Target_Ports) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_InfiniBand) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_OpenIB_Subnet_Mgmt) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_SeaMon_LX) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_BMC_IPMI) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Enclosure_Environmentals) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Storage_Interconnect) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Storage_Configuration) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_SHAS_State) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_CIFS) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_FTP) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_System_Services) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Software_Configuration) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Kernel) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Hyper_FS) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_IPStor) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_StreamSmith) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_VFlow) == 0) {	
		rtn = SUCCESS;
	} else if (paramname.compare(CN_SeaFS) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Sparse_Cache) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Distributed_Cache) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Sentry_Service) == 0) {
		rtn = SUCCESS;
	} else if (paramname.compare(CN_C2_Server) == 0) {
		rtn = SUCCESS;
	} else {
		rtn = FAILURE;
	}
	return rtn;
}

/**
 * Function execute() basically a dispatch for type of XML RPC Alert request
 * 
 */

void 
SystemAlertXmlrpc::execute(xmlrpc_c::paramList const& paramList,
				xmlrpc_c::value *   const retvalP){

		
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::execute, Enter");
	int rtn = FAILURE;

	param1 = paramList.getString(0);		// Alert, AlertRange, AlertSequence
											
	if (param1.compare("Alert") == 0) {
		if(paramList.size() == 2) { 
			param2 =  paramList.getString(1);    // the Sequence number as a string to find
			buildAlertResp(*retvalP);

		} else {
			throw(xmlrpc_c::fault("Alert needs 1 additional param ", xmlrpc_c::fault::CODE_UNSPECIFIED));
		}

 	} else if (param1.compare("AlertSequence") == 0) {

		if (paramList.size() == 3) {
			param2 =  paramList.getString(1);		// the First Sequence number as a string to find
			param3 =  paramList.getString(2);		// the Last Sequence number as a string to find
	
			buildAlertSequenceResp(*retvalP);

		} else {

			throw(xmlrpc_c::fault("AlertSequence needs 2 additional params ", xmlrpc_c::fault::CODE_UNSPECIFIED));
		}

	} else if (param1.compare("AlertRange") == 0 ) {
		buildAlertRangeResp(*retvalP);

	} else if (param1.compare("AlertComponent") == 0 ) {
		if(paramList.size() == 2) {
			param2 =  paramList.getString(1);			// Component Name
			rtn = VerifyCponentName(param2);
		}
		if (rtn == SUCCESS) {
			buildSortedComponentResp(*retvalP);
		} else {

			throw(xmlrpc_c::fault("Invalid ComponentName check spelling ", xmlrpc_c::fault::CODE_UNSPECIFIED));
		}
	} else{

		throw(xmlrpc_c::fault("Invalid Alert request method check spelling ", xmlrpc_c::fault::CODE_UNSPECIFIED));
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::execute, Exit");
}

/**
 * SystemAlertXmlrpc::buildAlertRangeResp()            
 *
 * Private method to get the info of baseboard and build
 * the data member baseboardResp.
 * 
 */

void
SystemAlertXmlrpc::buildAlertRangeResp(xmlrpc_c::value &result)
{
	ALERT_RANGE					alertrange;	
	char						tmpstr[BUFF80];
	int							rtn = FAILURE;
	string						locstr;

	
	map<string, xmlrpc_c::value> rsp;     // Watch this subscript and change it if this list grows!
    traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::buildAlertRangeResp, Enter");
	rtn = getAlertRange(&alertrange);

	if (rtn == SUCCESS) {
		sprintf(tmpstr,"%ld", alertrange.OldestSeqNum); 
		locstr = tmpstr;
		rsp.insert(make_pair( "Oldest sequence number", xmlrpc_c::value_string(locstr)));

		sprintf(tmpstr,"%ld", alertrange.NewestSeqNum);
		locstr = tmpstr;
		rsp.insert(make_pair("Newest sequence number", xmlrpc_c::value_string(locstr))); 
		
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SystemAlertXmlrpc::buildAlertRangeResp, Exit");
		
	} else {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SystemAlertXmlrpc::buildAlertRangeResp, getAlertRange() call Failed");
	}

	xmlrpc_c::value_struct param(rsp);
	result = param;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::buildAlertRangeResp, Exit");
}

/**
 * SystemAlertXmlrpc::buildAlertSequenceResp()            
 *
 * Private method to get the info of chassis and build
 * the data member AlertSequenceResp.
 * 
 */

void
SystemAlertXmlrpc::buildAlertResp(xmlrpc_c::value &result)
{
	vector<xmlrpc_c::value>		elements;
	ALERT_STRUCT				alert;	
	int							rtn = FAILURE;
	char						tmpstr[BUFF80];
	string						locstr;
	FILE						*fp;
	int							isopen = 0;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::buildAlertResp, Enter");
	memset(tmpstr, 0, sizeof(tmpstr));
	sprintf(tmpstr,"%s", param2.c_str());
	rtn = getAlert(tmpstr, &alert, &fp, &isopen, 1);

	if (rtn == SUCCESS) {
		map<string, xmlrpc_c::value> rsp; 

		sprintf(tmpstr,"%ld", alert.seqnum);
		locstr = tmpstr;
		rsp.insert(make_pair("Sequence number", xmlrpc_c::value_string(locstr)));
		
		sprintf(tmpstr,"%d", alert.alertid);
		locstr = tmpstr;
		rsp.insert(make_pair("Alert ID", xmlrpc_c::value_string(locstr)));

		locstr = alert.timestamp;
		rsp.insert(make_pair("Timestamp", xmlrpc_c::value_string(locstr)));

		locstr = alert.severity;
		rsp.insert(make_pair("Severity", xmlrpc_c::value_string(locstr)));

		locstr = alert.facility;
		rsp.insert(make_pair("Facility", xmlrpc_c::value_string(locstr)));

		locstr = alert.componentname;
		rsp.insert(make_pair("Component", xmlrpc_c::value_string(locstr)));

		locstr = alert.description;
		rsp.insert(make_pair("Alert description", xmlrpc_c::value_string(locstr)));

		locstr = alert.recommendation;
		rsp.insert(make_pair("Suggested Action description", xmlrpc_c::value_string(locstr)));

		elements.push_back(xmlrpc_c::value_struct( rsp )); 
		xmlrpc_c::value_array param(elements);
		result.instantiate( param.cValue() );

	} else {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SystemAlertXmlrpc::buildAlertResp, getALert() seqnum out of range");	
		xmlrpc_c::value_array param(elements);
		result = param;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::buildAlertResp, Exit");
}

/**
 * SystemAlertXmlrpc::buildAlertSequenceResp()            
 *
 * Private method to get the info of bios and build
 * the data member biosResp.
 * 
 */

void
SystemAlertXmlrpc::buildAlertSequenceResp(xmlrpc_c::value &result)
{
	ALERT_STRUCT				alert;	
	char						tmpstr[BUFF80];
	char						seqnum[BUFF32];
	FILE						*fp;
	int							isopen = 0;
	
	long						bseq = 0;
	long						eseq = 0;
	int							count = 0;
	int							i = 0;
	int							rtn = FAILURE;
	string						locstr;

	vector<xmlrpc_c::value> elements;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::buildAlertSequenceResp, Enter");
	bseq = atol(param2.c_str());
	eseq = atol(param3.c_str());
	count = ((int)(eseq - bseq) + 1);

	for (i=0; i < count; i++) {
		sprintf(seqnum,"%ld", bseq+i);
		rtn = getAlert(seqnum, &alert, &fp, &isopen, 0);

		if (rtn == SUCCESS) {
			map<string, xmlrpc_c::value> rsp; 
			sprintf(tmpstr,"%ld", alert.seqnum);
			locstr = tmpstr;
			rsp.insert(make_pair("Sequence number", xmlrpc_c::value_string(locstr)));
			
			sprintf(tmpstr,"%d", alert.alertid);
			locstr = tmpstr;
			rsp.insert(make_pair("Alert ID", xmlrpc_c::value_string(locstr)));

			locstr = alert.timestamp;
			rsp.insert(make_pair("Timestamp", xmlrpc_c::value_string(locstr)));

			locstr = alert.severity;
			rsp.insert(make_pair("Severity", xmlrpc_c::value_string(locstr)));

			locstr = alert.facility;
			rsp.insert(make_pair("Facility", xmlrpc_c::value_string(locstr)));

			locstr = alert.componentname;
			rsp.insert(make_pair("Component", xmlrpc_c::value_string(locstr)));

			locstr = alert.description;
			rsp.insert(make_pair("Alert description", xmlrpc_c::value_string(locstr)));

			locstr = alert.recommendation;
			rsp.insert(make_pair("Suggested Action description", xmlrpc_c::value_string(locstr)));
			
			elements.push_back(xmlrpc_c::value_struct( rsp )); 
		}
	}

	fclose(fp);

	//
	// Make it an array and get it activated for the return result
	//
	xmlrpc_c::value_array param(elements);
	result = param;

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::buildAlertSequenceResp, Exit");
}

int AssignComponentList(string paramname, list<AlertComponent>	*mylist)
{
	int rtn = FAILURE;

	pthread_mutex_lock(&ComponentListMutex);
	if (paramname.compare(CN_Server_Environmentals) == 0) {
		rtn = SUCCESS;
		(*mylist) = Server_Environmentals_complist;
	} else if (paramname.compare(CN_Management_Port) == 0) {
		(*mylist) = Management_Port_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Target_Ports) == 0) {
		(*mylist) = Target_Ports_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_InfiniBand) == 0) {
		(*mylist) = InfiniBand_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_OpenIB_Subnet_Mgmt) == 0) {
		(*mylist) = OpenIB_Subnet_Mgmt_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_SeaMon_LX) == 0) {
		(*mylist) = SeaMon_LX_complist;	
		rtn = SUCCESS;
	} else if (paramname.compare(CN_BMC_IPMI) == 0) {
		(*mylist) = BMC_IPMI_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Enclosure_Environmentals) == 0) {
		(*mylist) = Enclosure_Environmentals_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Storage_Interconnect) == 0) {
		(*mylist) = Storage_Interconnect_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Storage_Configuration) == 0) {
		(*mylist) = Storage_Configuration_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_SHAS_State) == 0) {
		(*mylist) = SHAS_State_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_CIFS) == 0) {
		(*mylist) = CIFS_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_FTP) == 0) {
		(*mylist) = FTP_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Software_Configuration) == 0) {
		(*mylist) = Software_Configuration_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_System_Services) == 0) {
		(*mylist) = System_Services_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Kernel) == 0) {
		(*mylist) = Kernel_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Hyper_FS) == 0) {
		(*mylist) = Hyper_FS_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_IPStor) == 0) {
		(*mylist) = IPStor_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_StreamSmith) == 0) {
		(*mylist) = StreamSmith_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_VFlow) == 0) {
		(*mylist) = VFlow_complist;	
		rtn = SUCCESS;
	} else if (paramname.compare(CN_SeaFS) == 0) {
		(*mylist) = SeaFS_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Sparse_Cache) == 0) {
		(*mylist) = Sparse_Cache_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Distributed_Cache) == 0) {
		(*mylist) = Distributed_Cache_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_Sentry_Service) == 0) {
		(*mylist) = Sentry_Service_complist;
		rtn = SUCCESS;
	} else if (paramname.compare(CN_C2_Server) == 0) {
		(*mylist) = C2_Server_complist;
		rtn = SUCCESS;
	} else {
		rtn = FAILURE;
	}
	pthread_mutex_unlock(&ComponentListMutex);
	return rtn;
}

/**
 * SystemAlertXmlrpc::buildSortedComponentResp()            
 *
 * Private method to get the info of bios and build
 * the data member biosResp.
 * 
 */

void
SystemAlertXmlrpc::buildSortedComponentResp(xmlrpc_c::value &result)
{	
	int rtn = FAILURE;
	list<AlertComponent>		loclist;
	
	vector<xmlrpc_c::value> elements;
	
	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::buildSortedComponentResp, Enter");

	//
	// determine which list by componentname  ***
	//
	rtn = AssignComponentList(param2, &loclist);
	if (rtn == SUCCESS) {
		
		list<AlertComponent>::iterator it;

		for (it = loclist.begin(); it != loclist.end(); it++) {
			map<string, xmlrpc_c::value> rsp; 
			rsp.clear();
			rsp.insert(make_pair("TimeStamp", xmlrpc_c::value_string(it->TimeStamp)));
			rsp.insert(make_pair("Severity", xmlrpc_c::value_string(it->Severity)));
			rsp.insert(make_pair("Alert Description", xmlrpc_c::value_string(it->descr)));
			rsp.insert(make_pair("Recommendation", xmlrpc_c::value_string(it->recc)));
			rsp.insert(make_pair("Severity", xmlrpc_c::value_string(it->ComponentName)));

			elements.push_back(xmlrpc_c::value_struct( rsp )); 
		}

		//
		// Make it an array and get it activated for the return result
		//
		xmlrpc_c::value_array param(elements);
		result = param;
	} else {
		traceClass->LogTrace(ZQ::common::Log::L_INFO, "SystemAlertXmlrpc::buildSortedComponentResp, call to AssignComponetList() failed");	
		xmlrpc_c::value_array param(elements);
		result = param;
	}

	traceClass->LogTrace(ZQ::common::Log::L_DEBUG, "SystemAlertXmlrpc::buildSortedComponentResp, Exit");
}
