/***********************************************************************
 File Name:     interfaceDefination.h
 Author:        Rose Lu
 Security:      SEACHANGE SHANGHAI
 Description:   define all interfaces 
 Function Inventory: 
 Modification Log:
 When           Version        Who							What
---------------------------------------------------------------------
 2005/03/10		1.0			   Rose.Lu						Created
**********************************************************************/ 

#ifndef __interfaceDefination_H__
#define __interfaceDefination_H__

#include <atlconv.h>
#include <unknwn.h>
#include "common.h"

//Catalog configuration for graphedit
interface ICatalogConfigure : public IUnknown
{
public:
	virtual HRESULT __stdcall 	SetCatalogName (LPCOLESTR pszCatalog) = 0;  //Set catalog name, which be monitored by DataWatcherSource filter.
	virtual HRESULT __stdcall	GetCatalogName (LPOLESTR* ppszCatalog) = 0; //Get catalog name, which be monitored by DataWatcherSource filter.
	virtual HRESULT __stdcall	UpdateCatalog () = 0;                       //Update catalog command. DataWatcherSource filter will send subchannel list to the extend module once received it. 
	virtual HRESULT __stdcall	SetDetectedFlag (BOOL bDetected) = 0;		//If bDetected is true, monitor catalog thread run. Otherwise,monitor catalog thread stop.
	virtual HRESULT __stdcall	GetDetectedFlag (BOOL* bDetected) = 0;		//Get detected flag.
	virtual HRESULT __stdcall   SetChannelEvent(HANDLE hStateEvent) = 0;	//If subchannel list changed, DataWatcherSource filter notify the filter graph through event. Then filter graph call GetCatalogs to get the latest sub channel list. 
	virtual HRESULT __stdcall	SetDetectedInterval (long lInterval) = 0;	//Set detected interval for monitor catalog thread. The default value is 0.
	virtual HRESULT __stdcall	GetDetectedInterval (long* lInterval) = 0;	//Get detected interval.
	virtual HRESULT __stdcall	GetCatalogs (vector<DWS_SubChannelList *> *pSubChannellists) = 0;//Get catalogs request¡ªFilter graph can get subchannel list from DataWatcherSource filter through this interface function.
	virtual HRESULT __stdcall	SetSubChannelRate(vector<DWS_SubChannelList *> pSubChannellists, long nListCount) = 0;//Set repeat times of files  in subchannel list.
};
//ICatalogState
interface ICatalogState : public IUnknown
{
public: 
	virtual HRESULT __stdcall SetCatalogStateEvent(HANDLE hStateEvent) = 0;	//Set catalog state event¡ªwhen filter status changed it can inform the extend module. 
	virtual HRESULT __stdcall GetLastErrorCode(int* nError)=0;				//Get last error code
	virtual HRESULT __stdcall GetCurrentStatus(int* pnStatus) = 0;			//Get current filter status.
};
//ICatalogRountine
interface ICatalogRountine : public IUnknown
{
public:
	virtual HRESULT __stdcall	SetDetectedRoutine(OnCatalogChanged pOnCatalogChanged, LPVOID lParam) = 0;//user defined call back function pointer. When subchannel list is channed, DataWatcherSource filter will send the latest subchannel list to the extend module.
	virtual HRESULT __stdcall 	SetFileName (LPCOLESTR lpwszFileName) = 0;//Set file name.
};

#endif

