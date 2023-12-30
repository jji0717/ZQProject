
#include "stdafx.h"
#include "common.h"

//#include <stdlib.h>
//MAX_PATH	// windef.h  260

///////////////////////////////////////////////////////////////////////////////////////////////////////////
static const GUID IID_IWrapperControl = 
{ 0xf55141e8, 0x4b2e, 0x4ddc, {0xa6, 0x71, 0xc1, 0xf4, 0xfe, 0x1, 0x3, 0xf} };

static const GUID IID_IStatusReport = 
{	0xf20c5f97, 0xe043, 0x4bdb, {0xaa, 0xd0, 0xba, 0xf5, 0x3d, 0xfa, 0x10, 0x9} };

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



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------
//vector<CCatalog *> Catalogs;
//vector<DWS_SubChannelList > SubChannellists;
//vector<DWS_SubChannelList > TempSubChannellists;
//------------------
//TCHAR g_szMsg[256];

