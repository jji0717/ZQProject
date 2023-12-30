// Reminder: Modify typemap.dat to customize the header file generated by wsdl2h
/* E:\WorkPath\ZQProjs\Telewest\PlaylistAS_gSoap\SoapInterface.h
   Generated by wsdl2h 1.2.1 from PlaylistSoapInterface.wsdl and typemap.dat
   2005-06-23 04:17:10 GMT
   Copyright (C) 2001-2005 Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL or Genivia's license for commercial use.
*/

/* NOTE:

 - Compile this file with soapcpp2 to complete the code generation process.
 - Use wsdl2h option -l to view the software license terms.
 - Use wsdl2h options -c and -s to generate pure C code or C++ code without STL.
 - To build with STL, stlvector.h from the gSOAP distribution must be in the
   current directory. Or use soapcpp2 option -I<path> with path to stlvector.h.
 - Use typemap.dat to control schema namespace bindings and type mappings.
   It is strongly recommended to customize the names of the namespace prefixes
   generated by wsdl2h. To do so, modify the prefix bindings in the Namespaces
   section below and add the modified lines to typemap.dat to rerun wsdl2h.
 - Use Doxygen (www.doxygen.org) to browse this file.

*/

/******************************************************************************\
 *                                                                            *
 * http://10.7.0.41:90/services/PlaylistSoapInterface                         *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * License                                                                    *
 *                                                                            *
\******************************************************************************/

/*
--------------------------------------------------------------------------------
gSOAP XML Web services tools
Copyright (C) 2001-2005, Robert van Engelen, Genivia Inc. All Rights Reserved.

This software is released under one of the following two licenses:
GPL or Genivia's license for commercial use.

GPL license.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org
--------------------------------------------------------------------------------
A commercial use license is available from Genivia, Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

//gsoapopt w

#import "stlvector.h"

/******************************************************************************\
 *                                                                            *
 * Schema Namespaces                                                          *
 *                                                                            *
\******************************************************************************/


/* NOTE:

It is strongly recommended to customize the names of the namespace prefixes
generated by wsdl2h. To do so, modify the prefix bindings below and add the
modified lines to typemap.dat to rerun wsdl2h:

impl = "http://10.7.0.41:90/services/PlaylistSoapInterface"
tns1 = http://model.playlist.izq.com

*/

//gsoap impl  schema namespace:	http://10.7.0.41:90/services/PlaylistSoapInterface
//gsoap tns1  schema namespace:	http://model.playlist.izq.com
//gsoap impl  schema form:	unqualified
//gsoap tns1  schema form:	unqualified

/******************************************************************************\
 *                                                                            *
 * Schema Types                                                               *
 *                                                                            *
\******************************************************************************/


/// Built-in attribute "SOAP-ENC:arrayType"
typedef std::string SOAP_ENC__arrayType;

class ArrayOf_USCORExsd_USCOREstring;
class tns1__SimplePlaylistModel;

/// Schema http://model.playlist.izq.com:"SimplePlaylistModel"

class tns1__SimplePlaylistModel
{ public:
/// Element elements of type "http://10.7.0.41:90/services/PlaylistSoapInterface":ArrayOf_xsd_string
    ArrayOf_USCORExsd_USCOREstring*      elements                      ;	///< Nullable pointer
/// Element playlistID of type xs:string
    std::string*                         playlistID                    ;	///< Nullable pointer
/// A handle to the soap struct that manages this instance (automatically set)
    struct soap                         *soap                          ;
};

/// Schema http://10.7.0.41:90/services/PlaylistSoapInterface:"ArrayOf_xsd_string"

/// SOAP encoded array of xs:string
class ArrayOf_USCORExsd_USCOREstring
{ public:
/// Pointer to an array of std::string*
    std::string*                        *__ptr                         ;
/// Size of the dynamic array
    int                                  __size                        ;
/// A handle to the soap struct that manages this instance (automatically set)
    struct soap                         *soap                          ;
};

/******************************************************************************\
 *                                                                            *
 * Services                                                                   *
 *                                                                            *
\******************************************************************************/

//gsoap ns2  service name:	SoapWrap 
//gsoap ns2  service type:	PlaylistSoapInterface 
//gsoap ns2  service port:	http://10.7.0.41:90/services/PlaylistSoapInterface 
//gsoap ns2  service namespace:	http://service.playlist.izq.com 
//gsoap ns2  service transport:	http://schemas.xmlsoap.org/soap/http 

/** @mainpage Service Definitions

@section Service_bindings Bindings
  - @ref PlaylistSoapInterfaceSoapBinding

*/

/**

@page PlaylistSoapInterfaceSoapBinding Binding "PlaylistSoapInterfaceSoapBinding"

@section PlaylistSoapInterfaceSoapBinding_operations Operations of Binding  "PlaylistSoapInterfaceSoapBinding"
  - @ref ns2__getPlaylistDetailInformation
  - @ref ns2__teardownPlaylist
  - @ref ns2__playPlaylist

@section PlaylistSoapInterfaceSoapBinding_ports Endpoints of Binding  "PlaylistSoapInterfaceSoapBinding"
  - http://10.7.0.41:90/services/PlaylistSoapInterface

*/

/******************************************************************************\
 *                                                                            *
 * PlaylistSoapInterfaceSoapBinding                                           *
 *                                                                            *
\******************************************************************************/


/******************************************************************************\
 *                                                                            *
 * ns2__getPlaylistDetailInformation                                          *
 *                                                                            *
\******************************************************************************/

/// Operation response struct "ns2__getPlaylistDetailInformationResponse" of service binding "PlaylistSoapInterfaceSoapBinding" operation "ns2__getPlaylistDetailInformation"
struct ns2__getPlaylistDetailInformationResponse
{
    tns1__SimplePlaylistModel*          _getPlaylistDetailInformationReturn;
};

/// Operation "ns2__getPlaylistDetailInformation" of service binding "PlaylistSoapInterfaceSoapBinding"

/**

Operation details:

  - SOAP RPC encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call_ns2__getPlaylistDetailInformation(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    std::string                         homeID,
    std::string                         deviceID,
    std::string                         playlistID,
    std::string                         streamID,
    // response parameters:
    struct ns2__getPlaylistDetailInformationResponse&
  );
@endcode

C++ proxy class (defined in soapPlaylistSoapInterfaceSoapBindingProxy.h):
  class PlaylistSoapInterfaceSoapBinding;

*/

//gsoap ns2  service method-style:	getPlaylistDetailInformation rpc
//gsoap ns2  service method-encoding:	getPlaylistDetailInformation http://schemas.xmlsoap.org/soap/encoding/
//gsoap ns2  service method-action:	getPlaylistDetailInformation ""
int ns2__getPlaylistDetailInformation(
    std::string                         _homeID,
    std::string                         _deviceID,
    std::string                         _playlistID,
    std::string                         _streamID,
    struct ns2__getPlaylistDetailInformationResponse& ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * ns2__teardownPlaylist                                                      *
 *                                                                            *
\******************************************************************************/


/// Operation "ns2__teardownPlaylist" of service binding "PlaylistSoapInterfaceSoapBinding"

/**

Operation details:

  - SOAP RPC encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call_ns2__teardownPlaylist(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    std::string                         homeID,
    std::string                         deviceID,
    std::string                         playlistID,
    std::string                         streamID,
    std::string                         errorCode,
    // response parameters:
    int                                &_teardownPlaylistReturn
  );
@endcode

C++ proxy class (defined in soapPlaylistSoapInterfaceSoapBindingProxy.h):
  class PlaylistSoapInterfaceSoapBinding;

*/

//gsoap ns2  service method-style:	teardownPlaylist rpc
//gsoap ns2  service method-encoding:	teardownPlaylist http://schemas.xmlsoap.org/soap/encoding/
//gsoap ns2  service method-action:	teardownPlaylist ""
int ns2__teardownPlaylist(
    std::string                         _homeID,
    std::string                         _deviceID,
    std::string                         _playlistID,
    std::string                         _streamID,
    std::string                         _errorCode,
    int                                &_teardownPlaylistReturn ///< response parameter
);

/******************************************************************************\
 *                                                                            *
 * ns2__playPlaylist                                                          *
 *                                                                            *
\******************************************************************************/


/// Operation "ns2__playPlaylist" of service binding "PlaylistSoapInterfaceSoapBinding"

/**

Operation details:

  - SOAP RPC encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call_ns2__playPlaylist(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    // request parameters:
    std::string                         homeID,
    std::string                         deviceID,
    std::string                         playlistID,
    std::string                         streamID,
    // response parameters:
    int                                &_playPlaylistReturn
  );
@endcode

C++ proxy class (defined in soapPlaylistSoapInterfaceSoapBindingProxy.h):
  class PlaylistSoapInterfaceSoapBinding;

*/

//gsoap ns2  service method-style:	playPlaylist rpc
//gsoap ns2  service method-encoding:	playPlaylist http://schemas.xmlsoap.org/soap/encoding/
//gsoap ns2  service method-action:	playPlaylist ""
int ns2__playPlaylist(
    std::string                         _homeID,
    std::string                         _deviceID,
    std::string                         _playlistID,
    std::string                         _streamID,
    int                                &_playPlaylistReturn ///< response parameter
);

/* End of E:\WorkPath\ZQProjs\Telewest\PlaylistAS_gSoap\SoapInterface.h */
