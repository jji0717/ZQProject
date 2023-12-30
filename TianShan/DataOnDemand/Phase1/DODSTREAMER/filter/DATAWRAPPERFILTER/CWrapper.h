
#ifndef WRAPPER_H
#define WRAPPER_H

#include "common.h"
#include "scqueue.h"

class CWrapper
{
public:
	CWrapper();
	virtual ~CWrapper();

	// applicate the update for new para set.
	virtual bool UpdateParaSet(){ return false;};

	// Set the set of parameter of wrapper.
	virtual bool SetWrapParaSet( DW_ParameterSet inPara ) = 0{ return false;};

	// Get the set of parameter of wrapper.
	virtual bool GetWrapParaSet( DW_ParameterSet * outPara ) = 0{ return false;};

	// Set the PID of object in current channel. 
	// e.g. set the ¡°pic\0¡± as the tag of picture object.
	virtual bool SetChannelPID(long inPID) { return false;};

	// Get the PID of object in current channel.
	virtual bool GetChannelPID(long * outPID) { return false;};

	virtual bool GetMode(BYTE * byMode){ return false;};

	virtual bool SetMode(BYTE byMode){ return false;};

	virtual bool GetStreamCount( int * nCount ){ return false; };

	virtual bool SetStreamCount( int nCount ){ return false; };

	// Set the tag of object in current channel. 
	// e.g. set the ¡°pic\0¡± as the tag of picture object.
	virtual bool SetChannelTag(const char * inTag, BYTE inLength) { return false;};

	// Get the tag of object in current channel. 
	virtual bool GetChannelTag(char * outTag, BYTE * outLength) { return false;};

	// Add the version number of all object in current channel.
	virtual bool AddVersionNumber() { return false;};

	// Reset the version number of all object in current channel to 1.
	virtual bool ResetVersionNumber() { return false;};

	// Set the version number of one or all object in current channel.
	virtual bool SetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE inVersion) { return false;};

	// Get the version number of one object in current channel.
	virtual bool GetVersionNumber(const char * inObjectKey, BYTE inObjectKeyLen, BYTE * outVersion) { return false;};

	// Set the length of the object key field.
	virtual bool SetObjectKeyLength(BYTE inLength) { return false;};

	// Get the length of the object key field.
	virtual bool GetObjectKeyLength(BYTE *outLength) { return false;};

	// Set the length of the descriptive information of Index Descriptor, the value can¡¯t be more than 255.
	virtual bool SetIndexDescLength(BYTE inLength) { return false;};

	// Get the length of the descriptive information of Index Descriptor.
	virtual bool GetIndexDescLength(BYTE *outLength) { return false;};	

	// Set the length of the descriptive information of Index Descriptor, the value can¡¯t be more than 255.
	virtual bool SetEncrypt(BYTE bEncrypt) { return false;};

	// Get the length of the descriptive information of Index Descriptor.
	virtual bool GetEncrypt(BYTE * bEncrypt) { return false;};	

	// Set the object key of certain object in current channel.
	virtual bool SetObjectKey( const char * inObjectName, int inObjectNameLen, const char * inObjectKey, BYTE inObjectKeyLen ) { return false;};

	// Get the object key of certain object in current channel.
	virtual bool GetObjectKey( const char * inObjectName, int inObjectNameLen, char * outObjectKey, BYTE * outObjectKeyLen) { return false;};

	// Set the max size of sample transferred from the DataWatcherSource Filter to the DataBroadcast Filter.
	virtual bool SetMaxSizeOfSample(long inSize) { return false;};

	// Get the max size of sample transferred from the DataWatcherSource Filter to the DataBroadcast Filter.
	virtual bool GetMaxSizeOfSample(long *outSize) { return false;};	

	// Set the min size of sample transferred from the DataWatcherSource Filter to the DataBroadcast Filter.
	virtual bool SetMinSizeOfSample(long inSize) { return false;};

	// Set the min size of sample transferred from the DataWatcherSource Filter to the DataBroadcast Filter.
	virtual bool GetMinSizeOfSample(long *outSize) { return false;};

// ------------------------------------------------------ Modified by zhenan_ji at 2005Äê3ÔÂ25ÈÕ 16:15:01
	virtual bool WrapData(  BYTE * pInBuffer, long inSize, BYTE * pOutBuffer, long bufsize, long * outSize, DWS_SubChannelList pSubChannel ) { return false; };
};

#endif