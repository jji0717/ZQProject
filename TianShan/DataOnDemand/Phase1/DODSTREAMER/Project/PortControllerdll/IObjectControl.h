//
// IObjectControl.h
// Desc: 
//

#ifndef IOBJECTCONTROL_H
#define IOBJECTCONTROL_H

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif


	//----------------------------------------------------------------------------
	// IWrapperControl's GUID

	// {F55141E8-4B2E-4ddc-A671-C1F4FE01030F}
/*	DEFINE_GUID(IID_IWrapperControl, 
		0xf55141e8, 0x4b2e, 0x4ddc, 0xa6, 0x71, 0xc1, 0xf4, 0xfe, 0x1, 0x3, 0xf);
*/
	static const IID IID_IWrapperControl = 
	{ 0xf55141e8, 0x4b2e, 0x4ddc, { 0xa6, 0x71, 0xc1, 0xf4, 0xfe, 0x01, 0x03, 0x0f } };

	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	// IWrapperControl: for Filter Graph.
	//----------------------------------------------------------------------------
	DECLARE_INTERFACE_(IWrapperControl, IUnknown)
	{
		// apply the newest parameter set.
		STDMETHOD(UpdateParaSet)( THIS_
			) PURE;

		// Set the set of parameter of wrapper.
		STDMETHOD(SetWrapParaSet) (THIS_
			DW_ParameterSet inPara
			) PURE;

		// Get the set of parameter of wrapper.
		STDMETHOD(GetWrapParaSet) (THIS_
			DW_ParameterSet * outPara
			) PURE;

		// Set the PID of object in current channel. 
		// e.g. set the ¡°pic\0¡± as the tag of picture object.
		STDMETHOD(SetChannelPID) (THIS_
			long inPID
			) PURE;

		// Get the PID of object in current channel.
		STDMETHOD(GetChannelPID) (THIS_
			long * outPID
			) PURE;

		STDMETHOD (GetMode) (
			BYTE * byMode
			) PURE;

		STDMETHOD (SetMode) (
			BYTE byMode
			) PURE;

		STDMETHOD (GetStreamCount) (
			int * nCount
			) PURE;

		STDMETHOD (SetStreamCount) (
			int nCount,char *pcharPath
			) PURE;

		// Set the tag of object in current channel. 
		// e.g. set the ¡°pic\0¡± as the tag of picture object.
		STDMETHOD(SetChannelTag) (THIS_
			const char * inTag, BYTE inLength    
			) PURE;

		// Get the tag of object in current channel. 
		STDMETHOD(GetChannelTag) (THIS_
			char * outTag, BYTE * outLength     
			) PURE;

		// Add the version number of all object in current channel.
		STDMETHOD(AddVersionNumber) (THIS_	) PURE;
		
		// Reset the version number of all object in current channel to 1.
		STDMETHOD(ResetVersionNumber) (THIS_	) PURE;

		// Set the version number of one or all object in current channel.
		STDMETHOD(SetVersionNumber) (THIS_
			const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion
			) PURE;
		
		// Get the version number of one object in current channel.
		STDMETHOD(GetVersionNumber) (THIS_
			const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion
			) PURE;

		// Set the length of the object key field.
		STDMETHOD(SetObjectKeyLength) (THIS_
			BYTE inLength
			) PURE;

		// Get the length of the object key field.
		STDMETHOD(GetObjectKeyLength) (THIS_
			BYTE * outLength
			) PURE;		

		// Set the length of the descriptive information of Index Descriptor, the value can¡¯t be more than 255.
		STDMETHOD(SetIndexDescLength) (THIS_
			BYTE inLength
			) PURE;

		// Get the length of the descriptive information of Index Descriptor.
		STDMETHOD(GetIndexDescLength) (THIS_
			BYTE * outLength
			) PURE;		

		// Set the object key of certain object in current channel.
		STDMETHOD(SetObjectKey) (THIS_
			const char * inObjectName, int inObjectNameLen, const char * inObjectKey, BYTE inObjectKeyLen
			) PURE;

		// Get the object key of certain object in current channel.
		STDMETHOD(GetObjectKey) (THIS_
			const char * inObjectName, int inObjectNameLen, char * outObjectKey, BYTE * outObjectKeyLen
			) PURE;

		// Set the PID of object in current channel. 
		// e.g. set the ¡°pic\0¡± as the tag of picture object.
		STDMETHOD(SetEncrypt) (THIS_
			BYTE inEncrypt
			) PURE;

		// Get the PID of object in current channel.
		STDMETHOD(GetEncrypt) (THIS_
			BYTE * outEncrypt
			) PURE;
	
		// Set the max size of sample transferred from the DataWatcherSource Filter to the DataBroadcast Filter.
		STDMETHOD(SetMaxSizeOfSample) (THIS_
			long inSize
			) PURE;

		// Get the max size of sample transferred from the DataWatcherSource Filter to the DataBroadcast Filter.
		STDMETHOD(GetMaxSizeOfSample) (THIS_
			long * outSize
			) PURE;

		// Set the min size of sample transferred from the DataWatcherSource Filter to the DataBroadcast Filter.
		STDMETHOD(SetMinSizeOfSample) (THIS_
			long inSize
			) PURE;

		// Set the min size of sample transferred from the DataWatcherSource Filter to the DataBroadcast Filter.
		STDMETHOD(GetMinSizeOfSample) (THIS_
			long * outSize
			) PURE;
		
	};
	//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // IOBJECTCONTROL_H