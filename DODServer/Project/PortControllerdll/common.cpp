
#include "stdafx.h"
#include "common.h"

//#include <stdlib.h>
//MAX_PATH	// windef.h  260

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//media type

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// component guid

// {2D92E741-4EF7-4b4e-94F5-87DE78B01C9B}
static const GUID CLSID_IDataWatcherSource = 
{ 0x2d92e741, 0x4ef7, 0x4b4e, { 0x94, 0xf5, 0x87, 0xde, 0x78, 0xb0, 0x1c, 0x9b } };

// {20667A8C-E24E-4ee7-8E15-1DB8114EACBB}
static const GUID CLSID_CatalogConfigurePage = 
{ 0x20667a8c, 0xe24e, 0x4ee7, { 0x8e, 0x15, 0x1d, 0xb8, 0x11, 0x4e, 0xac, 0xbe } };
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//interafce iid.

// {C8CA6B9C-DACE-434c-9C98-8E64727E5F4F}
static const IID IID_ICatalogConfigure = 
{ 0xc8ca6b9c, 0xdace, 0x434c, { 0x9c, 0x98, 0x8e, 0x64, 0x72, 0x7e, 0x5f, 0x4f } };

// {C8CA6B9C-DACE-434c-9C98-8E64727E5F4F}
static const IID IID_ICatalogRountine = 
{ 0xc8ca6b9c, 0xdace, 0x434c, { 0x9c, 0x98, 0x8e, 0x64, 0x72, 0x7e, 0x5f, 0x5f } };
// {C8CA6B9C-DACE-434c-9C98-8E64727E5F4F}

static const IID IID_ICatalogState = 
{ 0xc8ca6b9c, 0xdace, 0x434c, { 0x9c, 0x98, 0x8e, 0x64, 0x72, 0x7e, 0x5f, 0x6f } };

/*
DEFINE_GUID(CLSID_DataWrapper, 
			0x869fec67, 0x11b1, 0x4387, 0x9f, 0x76, 0x60, 0xfb, 0xdd, 0x37, 0x65, 0x9e);*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------
//vector<CCatalog *> Catalogs;
//vector<DWS_SubChannelList *> SubChannellists;
//vector<DWS_SubChannelList *> TempSubChannellists;
//------------------



