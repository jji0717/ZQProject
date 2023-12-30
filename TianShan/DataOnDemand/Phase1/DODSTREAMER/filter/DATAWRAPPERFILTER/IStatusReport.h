//
// IObjectControl.h
// Desc: 
//

#ifndef ISTATUSREPORT_H
#define ISTATUSREPORT_H

#ifdef __cplusplus
extern "C" {
#endif


	//----------------------------------------------------------------------------
	// IStatusReport's GUID

	// {F20C5F97-E043-4bdb-AAD0-BAF53DFA1009}
	//DEFINE_GUID( IID_IStatusReport, 
	//	0xf20c5f97, 0xe043, 0x4bdb, 0xaa, 0xd0, 0xba, 0xf5, 0x3d, 0xfa, 0x10, 0x9);

	//static const GUID IID_IStatusReport = 
	//{	0xf20c5f97, 0xe043, 0x4bdb, {0xaa, 0xd0, 0xba, 0xf5, 0x3d, 0xfa, 0x10, 0x9} );

	//----------------------------------------------------------------------------

	//----------------------------------------------------------------------------
	// IStatusReport: for Filter Graph.
	//----------------------------------------------------------------------------
	DECLARE_INTERFACE_(IStatusReport, IUnknown)
	{
		// Get the current state of the DataWrapperFilter.
		STDMETHOD(GetState) (THIS_
			long * outState
			) PURE;

		// Get last error code.
		STDMETHOD(GetLastError) (THIS_
			long * outError
			) PURE;

		// Get the description of certain error. 
		STDMETHOD(GetErrorMsg) (THIS_
			char * outMsg, BYTE * outLength     
			) PURE;

	};
	//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // ISTATUSREPORT_H