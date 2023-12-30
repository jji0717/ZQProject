/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Dec 24 13:47:07 2007
 */
/* Compiler settings for D:\work\project\ZQProjs\TianShan\AccreditedPath\AdminControl\AdminControl.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_IAdminCtrl = {0x73C3E9F6,0x0A71,0x4A37,{0x90,0x93,0xD9,0xAE,0x64,0xF9,0xCA,0x24}};


const IID LIBID_ADMINCONTROLLib = {0xAEF669AA,0xDF5E,0x4FED,{0xA1,0x58,0xC2,0x0D,0xF8,0xCC,0xD3,0xAC}};


const IID DIID__IAdminCtrlEvents = {0x158CE249,0x683D,0x4B6E,{0x96,0xAC,0xB3,0x9F,0x0B,0xBB,0x39,0x7F}};


const CLSID CLSID_AdminCtrl = {0x85D19CA6,0xB302,0x40B0,{0xAB,0x41,0x4B,0x6B,0x00,0xD2,0x77,0xCB}};


#ifdef __cplusplus
}
#endif

