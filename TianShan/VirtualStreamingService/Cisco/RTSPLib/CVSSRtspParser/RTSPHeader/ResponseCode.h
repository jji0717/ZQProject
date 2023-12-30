#ifndef __RESPONSECODE_H__
#define __RESPONSECODE_H__

typedef enum
{
	UnknowError				= 0,
	RTSPOK					= 200,
	BadRequest				= 400,
	Forbidden				= 403,
	NotFound				= 404,
	MethodNotAllowed		= 405,
	NotAcceptable			= 406,
	RequestTimeOut			= 408,
	Gone					= 410,
	RequestTooLarge			= 413,
	UnsupportedMedia		= 415,
	InvalidParameter		= 451,
	NotEnoughBandwidth		= 453,
	SessionNotFound			= 454,
	InvalidRange			= 457,
	OperationNotAllowed		= 459,
	UnsupportedTransport	= 461,
	DestUnreachable			= 462,
	GatewayTimeout			= 504,
	VersionNotSupported		= 505,
	SetupAssetNotFound		= 771,
	SOPNotAvailable			= 772,
	UnknownSOPGroup			= 773,
	UnknownSOPNames			= 774,
	InsufficientBandwidth	= 775
}RTSPSessionState;

#endif __RESPONSECODE_H__