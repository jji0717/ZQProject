/* File Name: Controller.h
   Date     : 26th Nov
   Purpose  : Definition of Controller class
**/

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <httpdInterface.h>
#include <Ice/Ice.h>
#include <TsStorage.h>

using namespace TianShanIce::Storage;
using namespace TianShanIce;

namespace StorageWeb
{
class Controller
{
public:
	Controller(IHttpRequestCtx *pHttpRequestCtx);
	~Controller(void);
public:
	bool listVolumes(); // list all volumes
	bool listContents(); // list contents according condition

	// list contents in volume according index page
	// @param[out] contentInfos -- contents infos in volume according index page
	bool listVolume(std::string& strVolumeName, int nIndexPage,ContentInfos& contentInfos); 
    
	// list volumes including content accroding index page
	// @param[out] contentInfos 
	bool listContent(std::string& strContentName, int nIndexPage, ContentInfos& contentInfos);
private:
	bool init();
	void finalize();
private:
	int _nMaxCount;  // max rows of table of each page
	const char* _template; // template value
    const char* _endpoint; // endpoint value
    StrValues _strMetaDataNames; // metadata value 
	StrValues _strTableHeaders; // Table header
private:
    IHttpRequestCtx* _pHttpRequestCtx;
	ContentStorePrx _contentStorePrx;
	Ice::CommunicatorPtr _ic;  // handle of ice run time 
private:   // constant variables
	static const char* const CS_VAR_MODULE_NAME; // module name 
	static const char* const CS_VAR_TEMPLATE; // template name
	static const char* const CS_VAR_ENDPOINT; // endpoint name
	static const char* const CS_VAR_MAXCOUNT; // maxcount name 
	static const char* const CS_VAR_METADATA_NAMES; // metadata names
	static const char* const CS_VAR_VOLOUMEINFO_STRUCT;
};
}
#endif

