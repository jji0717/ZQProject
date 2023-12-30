#ifndef _INCLUDED_CMEVSIS  // [ Only include me once
#define _INCLUDED_CMEVSIS
/*
 *  CME <-> VSIS communications are done via TCP. CME starts the ball rolling
 *  by connecting to CV_PORT on each cluster member, then sending a CVM_REGISTER
 *  message. VSIS responds with its own CVM_REGISTER message. Thereafter, all
 *  messages are sent and received asynchronously.
 */

#define CME_VSIS_PROT_VER 0x00010203 // Protocol version 1.2.3
#define CV_PORT 4723				 // TCP port VSIS listens on
#define INDEX_FILE_PARTITION 2347	 // Partition "number" of index file

/*
 *  CME <-> VSIS message format.
 *
 *  All messages between CME and VSIS use the following TLV (Tag-Length-Value) format.
 *  A "message" is exactly one TLV entry containing some number (maybe 0) of sub-entries,
 *  which, if any, are also TLV-encoded. The "Length" field always includes space used by
 *  the Tag and Length fields, and is therefore the message length in bytes.
 *
 *  Tags and Lengths are always unsigned short (i.e., 16-bit) ints.
 *
 *  +-------------------+--------------------+
 *  |      Length       |        Tag         |
 *  +-------------------+--------------------+
 *  | associated value data (length-4) bytes |
 *  | (if any) . . .                         |
 *  +----------------------------------------+
 *  
 *  The Length need not be a multiple of 4, or even 2. There is no packing or blocking.
 *
 *  Each message has a single message type as indicated in the CVM_xxxx enum below.
 *
 *  For many message types, additional data is required. For example, an asset-delete
 *  request needs the appropriate PID and PAID to identify the asset. Each item of
 *  additional data is specified in its own TLV structure within the overall message.
 *
 *  When a datum consists of a character string, it is expected to be a NUL-terminated
 *  string of 8-bit characters. In this case the "Length" field can be greater than the
 *  payload if desired, viz:
 *
 *  +-------------------+---------------------+
 *  |         44        |     CVT_DRQ_PAID    |
 *  +-------------------+---------------------+
 *  |XYZXYZXYZXYZ\0amanaP lanac a nalp a nam A| <-- little-endian (i386) format, 40 bytes
 *  +-----------------------------------------+
 *
 *  In the above, the characters after the NUL (indicated by repeated XYZ) are not part of
 *  the message and are to be ignored. Their only purpose is to pad the data length to 40,
 *  the amount specified, presumably because that makes life easy for the message sender.
 *  An equally acceptable alternative for the sender is to set the length exactly, as in:
 *
 *  +-------------+---------------+
 *  |      32     | CVT_DRQ_PAID  |
 *  +-------------+---------------+
 *  |\0amanaP lanac a nalp a nam A| <-- little-endian (i386) format, 28 bytes
 *  +-----------------------------+
 *
 *  The receiver must be prepared to handle either possibility, including possibly mixing
 *  formats in the same message (e.g., an unpadded PID and padded fixed-length PAID). But
 *  in either case the terminating 0-byte must be included in the data portion of the message.
 *
 */



// "Outer" message types. No message type may end in the low 8 bits of the
// CME/VSIS synch char (0x55). Excluded values include (decimal) 85, 341, 597, 853,
// 1109, 1365. Current maximum is wayyy below these numbers.

enum {
	CVM_INVALID, CVM_REGISTER, CVM_DELETE_REQUEST, CVM_STATUS_REQUEST, CVM_LIST_ASSETS_REQUEST, 
	CVM_CME_PARAMETERS, CVM_IMPORT_REQUEST, CVM_SESS_COUNT_REQ,
	CVM_ASSET_STATUS=20,  CVM_LIST_ASSETS_RESPONSE, CVM_DISK_USAGE_REPORT, CVM_SMART_PARAMETERS,
	CVM_EXPIRE_NOTICE=40, CVM_DEDUCT_NOTICE, CVM_SESSION_START_NOTICE, CVM_SESSION_END_NOTICE,
	CVM_SESS_COUNT_RSP,   CVM_IMPORT_FRAGMENT=101, CVM_DELETE_FRAGMENT, CVM_FRAGMENT_STATUS,
	CVM_FRAGSTS_REQUEST,  CVM_LIST_FRAGMENTS,      CVM_VOD_POP_DATA=120,CVM_LAST_VOD_POP_DATA, 
	CVM_KNOWN_FRAGMENTS,  CVM_KNOWN_FRAGMENTS_END, CVM_VOD_TLV_DATA, CVM_VSIS_CDN_STATE
};

/****
       I n d i v d u a l  M e s s a g e  F o r m a t s
****/


/**** CVM_INVALID message format:
 *
 * None. This typecode exists only to remind everyone that message type 0 is illegal.
 */

enum {CVT_REG_PROTOCOL=101, CVT_REG_CMENAME, CVT_REG_VSISCLU, CVT_REG_NODEBPS, 
	  CVT_REG_FRAGMENT, CVT_REG_FRAGCOUNT};

/**** CVM_REGISTER message format:
 *
 *  Both sides send CVM_REGISTER messages. CME first.
 *
 *  Case 1: CME message to VSIS (this starts the dialog between CME and VSIS)
 *
 *  +-------------------+--------------------+
 *  |      Length       |    CVM_REGISTER    |
 *  +-------------------+--------------------+
 *  |  sub-length=8     |  CVT_REG_PROTOCOL  |
 *  +----------------------------------------+
 *  |  Protocol version (see above) 32-bits  |   ; CME_VSIS_PROT_VER
 *  +----------------------------------------+
 *  |    sub-length     |  CVT_REG_CMENAME   |
 *  +----------------------------------------+
 *  |   CME node's name (as known to CME)    |
 *  +----------------------------------------+
 *
 *  CME provides its node name for VSIS reference / logging only. VSIS needs to
 *  remember this message can come at any time, due to CME failover and restart.
 *  In a CVM_REGISTER message, CVT_REG_PROTOCOL must be the first sub-entry.
 *
 *
 *  Case 2: VSIS message to CME (telling CME that VSIS will handle the imports
 *          for this cluster, and can process CME delete requests).
 *
 *  +-------------------+--------------------+
 *  |      Length       |    CVM_REGISTER    |
 *  +-------------------+--------------------+
 *  |  sub-length=8     |  CVT_REG_PROTOCOL  |
 *  +----------------------------------------+
 *  |  Protocol version (see above) 32-bits  |   ; CME_VSIS_PROT_VER
 *  +----------------------------------------+
 *  |    sub-length     |  CVT_REG_VSISCLU   |
 *  +----------------------------------------+
 *  |          VSIS cluster name             |
 *  +----------------------------------------+
 *  |    sub-length     |  CVT_REG_NODEBPS   |
 *  +----------------------------------------+
 *  |                                        |
 *  |   Node output capacity (bps, 64-bits)  |
 *  +----------------------------------------+
 *  |  sub-length=8     |  CVT_REG_FRAGMENT  |
 *  +----------------------------------------+
 *  |  VSIS import method, 1 == by fragments |
 *  +----------------------------------------+
 *  |  sub-length=12    |  CVT_REG_FRAGCOUNT |  // Only for CVT_REG_FRAGMENT == 1
 *  +----------------------------------------+
 *  |               (64 bits)                |
 *  |     Number of fragments in cluster     |
 *  +----------------------------------------+
 *
 *  In a CVM_REGISTER message, CVT_REG_PROTOCOL must be the first sub-entry.
 *
 *  Reminder: the "sub-length" field includes the length of the sub-type and
 *  sub-length fields, along with the length if the following object.
 *
 *  A "name" field (CVT_REG_VSISCLU) is a C-string, i.e., a NULL-terminated
 *  8-bit character string.
 *
 *  The "Node output capacity" (CVT_REG_NODEBPS) field is a 64-bit quantity
 *  expressing the node's output capacity in bits per second. For example, an
 *  FMS2500 would be capable of 2500 streams * 3750000 bits per second per
 *  stream or 9375000000 bits per second.
 *
 *  The "import method" (CVT_REG_FRAGMENT) take one of two values: 0 for
 *  whole-asset import and 1 for import-by-fragment. If this field is not
 *  present, 0 is assumed. The term "segment" is sometimes used synonymously
 *  with "fragment", but all codes will use the term "FRAGMENT" or "FRAG".
 *
 *  If the "import method" is import-by-fragment then VSIS needs to report the
 *  cluster flash capacity in fragments via the CVT_REG_FRAGCOUNT tag.
 *
 *  If VSIS is running but for some reason NOT handling import requests, then
 *  it MUST NOT RESPOND to CME's CVM_REGISTER. CME will retry periodically.
 */

enum {CVT_DRQ_PID=201, CVT_DRQ_PAID, CVT_DRQ_CLUID, CVT_DRQ_PARTITION};

/**** CVM_DELETE_REQUEST message format:
 *
 *  CME sends VSIS a CVM_DELETE_REQUEST when CME determines VSIS should
 *  delete an asset, or a part of an asset. ("Part" here refers to an
 *  entire file, not a fragment).
 *
 *  +-------------------+--------------------+
 *  |      Length       | CVM_DELETE_REQUEST |
 *  +-------------------+--------------------+
 *  |    sub-length     |    CVT_DRQ_PID     |
 *  +----------------------------------------+
 *  |         Asset "Provider ID"            |
 *  +----------------------------------------+
 *  |    sub-length     |    CVT_DRQ_PAID    |
 *  +----------------------------------------+
 *  |      Asset "Provider Asset ID"         |
 *  +----------------------------------------+
 *  |   sub-length=8    | CVT_DRQ_PARTITION  |  // Optional. If present, indicates
 *  +----------------------------------------+  // which partitions are to be deleted.
 *  |             partition number           |  // If absent, ALL partitions are to
 *  +----------------------------------------+  // be deleted.
 *
 *  VSIS should attempt to delete the file(s) implied by the asset and partition.
 *  If the CVT_DRQ_PARTITION tag is absent, all component files are to be deleted.
 *
 *  On success, VSIS sends a CVM_ASSET_STATUS message code 30 (CACHE_CONTENT_DELETED).
 *  If ALL partitions are deleted, the CVM_ASSET_STATUS message should not contain any
 *  partition information.
 *
 *  When specific partitions are supplied, VSIS should attempt -- on a "best-effort"
 *  basis -- to ensure that all requested partitions can be deleted before any
 *  partitions are deleted. If VSIS ascertains that some partitions can not be
 *  deleted, it should not attempt to delete any partitions and send a code 31
 *  response (i.e., CVM_ASSET_STATUS message with field CVT_LAR_STATUS containing
 *  CACHE_CONTENT_NOT_DELETED).
 *
 *  If the file is not present, the code 30 response is sent. Codes for other errors
 *  are not defined; currently code 500 (CACHE_CONTENT_INSRVERR) is used as a catchall
 *  for any conditions not predefined.
 *
 *  If the asset is in use (not deletable), CME expects somebody (RAIDDrv, SeaFile)
 *  to mark the file for deletion and delete the contents after the last user is
 *  done. The CACHE_CONTENT_DELETED response can be sent immediately, unless there
 *  is a convenient way for VSIS to know when the asset is officially deleted and
 *  can send the CACHE_CONTENT_DELETED message when content really is deleted.
 *
 *  VSIS only sends ONE CVM_ASSET_STATUS message in response to a
 *  CVM_DELETE_REQUEST. This is discussed in more detail in the CVM_ASSET_STATUS
 *  section.
 */

enum {CVT_STS_PID=301, CVT_STS_PAID};

/**** CVM_STATUS_REQUEST message format:
 *
 *  CME sends VSIS a CVM_STATUS_REQUEST when CME needs to know a particular asset's
 *  existence and/or metadata.
 *
 *  +-------------------+--------------------+
 *  |      Length       | CVM_STATUS_REQUEST |
 *  +-------------------+--------------------+
 *  |    sub-length     |    CVT_STS_PID     |
 *  +----------------------------------------+
 *  |         Asset "Provider ID"            |
 *  +----------------------------------------+
 *  |    sub-length     |    CVT_STS_PAID    |
 *  +----------------------------------------+
 *  |      Asset "Provider Asset ID"         |
 *  +----------------------------------------+
 *
 *  VSIS should attempt to locate the file(s) implied by the asset. If found, VSIS
 *  sends a CVM_ASSET_STATUS message (see below) with code 11 (CACHE_CONTENT_STREAMABLE).
 *  If the file is not present, VSIS responds with code 30 (CACHE_CONTENT_DELETED).
 *  Codes for other errors are not defined; currently code 500 (CACHE_CONTENT_INSRVERR)
 *  is used as a catchall for any conditions not predefined.
 *
 *  It is likely VSIS will see this message infrequently, typically when CME discovers
 *  an asset other than through normal (CVM_ASSET_STATUS) channels.
 */

enum {CVT_LAQ_PID=401, CVT_LAQ_PAID};

/**** CVM_LIST_ASSETS_REQUEST message format:
 *
 *  CME sends a CVM_LIST_ASSETS_REQUEST message to VSIS when it needs to validate its
 *  internal list of known assets. Typically this happens only at CME startup and possibly
 *  during idle time overnight.
 *
 *  +-----------------------+-------------------------+
 *  |        Length         | CVM_LIST_ASSETS_REQUEST |
 *  +-----------------------+-------------------------+
 *  |      sub-length       |      CVT_LAQ_PID        |
 *  +-----------------------+-------------------------+
 *  |           "Provider ID" to search for           |
 *  +-----------------------+-------------------------+
 *  |      sub-length       |      CVT_LAQ_PAID       |
 *  +-----------------------+-------------------------+
 *  |       "Provider Asset ID" to search for         |
 *  +-------------------------------------------------+
 *
 *  CVT_LAQ_PID, if present, specifies a PID or a wildcard mask to match PIDs against.
 *  Only assets that match the specified PID are to be returned. If not supplied, all
 *  PIDs are to be scanned.
 *
 *  CVT_LAQ_PAID, if present, specifies a PAID or a wildcard mask to match PAIDs against.
 *  Only assets that match the specified PAID are to be returned. If not supplied, all
 *  PAIDs are to be scanned.
 *  
 *  It is possible VSIS may see this message as a 4-byte request with neither PID nor PAID.
 *  In a first implementation that is what CME will use.
 *
 *  VSIS looks for cluster assets satisfying the match criteria. For each such asset found,
 *  VSIS constructs (or adds to) a CVM_LIST_ASSETS_RESPONSE message (below).
 *
 *  When VSIS reaches the end of the asset list it sends final status (below).
 *
 ****/

enum {CVT_ASS_STS=1001, CVT_ASS_PID,  CVT_ASS_PAID, CVT_ASS_CLUID, 
	  CVT_ASS_BITRATE, CVT_ASS_SIZE, CVT_ASS_DURATION, CVT_ASS_PARTITION, CVT_ASS_PWE};

#define PARTITION_MASK_MAIN  1		// Bit 0, main asset and index file
#define PARTITION_MASK_FF    2		// Bit 1, all Fast Forward Files
#define PARTITION_MASK_FR    4		// Bit 2, all Fast Reverse Files

enum {CACHE_CONTENT_NONLOCAL=1,    CACHE_CONTENT_COMPLETED=10, CACHE_CONTENT_STREAMABLE, 
	  CACHE_CONTENT_DELETED=30,    CACHE_CONTENT_NOT_DELETED,  CACHE_CONTENT_NOT_CDN=60,
	  CACHE_CONTENT_REQ_INVALID,   CACHE_CONTENT_NO_BANDWIDTH, CACHE_CONTENT_PENDING=70,
	  CACHE_CONTENT_FRAGMENT_ONLY, CACHE_CONTENT_INSRVERR=500};

/**** CVM_ASSET_STATUS message format:
 *
 *  VSIS sends CME a CVM_ASSET_STATUS message in response to a CVM_STATUS_REQUEST
 *  message, or whenever a cluster asset changes state.
 *
 *  +-------------------+--------------------+
 *  |      Length       |  CVM_ASSET_STATUS  |
 *  +-------------------+--------------------+
 *  |  sub-length=8     |    CVT_ASS_STS     |
 *  +----------------------------------------+
 *  |      CACHE_CONTENT_xxx (32-bits)       | // status code 10, 11 or 30
 *  +----------------------------------------+
 *  |    sub-length     |    CVT_ASS_PID     |
 *  +----------------------------------------+
 *  |         Asset "Provider ID"            |
 *  +----------------------------------------+
 *  |    sub-length     |    CVT_ASS_PAID    |
 *  +----------------------------------------+
 *  |      Asset "Provider Asset ID"         |
 *  +----------------------------------------+
 *  |    sub-length     |   CVT_ASS_CLUID    |
 *  +----------------------------------------+
 *  |          VSIS "Cluster ID"             |
 *  +----------------------------------------+
 *  |  sub-length=12    |   CVT_ASS_BITRATE  |  // Here and below only for code 10
 *  +----------------------------------------+  // but can be supplied for other
 *  |                                        |  // status codes if that is easier.
 *  | Asset bitrate (bits/sec) (64-bits)     |
 *  +----------------------------------------+
 *  |  sub-length=12    |    CVT_ASS_SIZE    |
 *  +----------------------------------------+
 *  |                                        |
 *  |   Total asset size (bytes) (64-bits)   |
 *  +----------------------------------------+
 *  |  sub-length=12    |  CVT_ASS_DURATION  |
 *  +----------------------------------------+
 *  |                                        |
 *  | Main asset duration (seconds) (64bits) |
 *  +----------------------------------------+
 *  |   sub-length=8    | CVT_ASS_PARTITION  |  // Present only if caller supplied
 *  +----------------------------------------+  // CVT_DRQ_PARTITION on CVM_DELETE_REQUEST
 *  |          partition number              |
 *  +----------------------------------------+
 *  |   sub-length=4    |    CVT_ASS_PWE     |  // Asset is PWE (playable while encoding)
 *  +----------------------------------------+
 *
 *  CVT_ASS_PWE interpretation:
 *
 *   VSIS includes this field if and only if the asset is currently being encoded. VSIS can
 *   only know this for Vstrm versions 6.3 and later. If Vstrm tells VSIS the asset "is PWE",
 *   as the saying goes, then VSIS adds this field to the message. If this field is absent,
 *   CME assumes the asset is not PWE.
 *
 *   Note that an asset may start out as a PWE entity but after some period of time will finish
 *   encoding and no longer have the PWE property. VSIS needs to ensure the PWE state for each
 *   CVM_STATUS_REQUEST message.
 *
 *  CVT_ASS_STS interpretations:
 *
 *   Message code 1 (CACHE_CONTENT_NONLOCAL) is sent when an asset is not local to
 *   the cluster, but is found on some other storage device.
 *
 *   Message code 10 (CACHE_CONTENT_COMPLETED) is sent when an asset is being imported
 *   into the cluster. Asset metadata (bitrate, size, duration) are essential for
 *   code 10. The "size" parameter includes all files.
 *
 *   Message code 11 (CACHE_CONTENT_STREAMABLE) is sent in response to a
 *   CVM_STATUS_REQUEST message. Asset metadata (bitrate, size, duration) are essential
 *   for code 11. The "size" parameter includes all files.
 *
 *   Message code 30 (CACHE_CONTENT_DELETED) is returned when VSIS feels the asset
 *   either has been deleted, is going to be deleted or doesn't exist on the cluster.
 *   Even in this case asset metadata is required.
 *
 *   Code 30 can be sent in response to a CVM_DELETE_REQUEST request that specifies
 *   only a subset of files to delete, via the CVT_DRQ_PARTITION tag on the request.
 *   In this case VSIS fills in the CVT_ASS_PARTITION field of the CVM_ASSET_STATUS
 *   message with a bitfield of deleted partitions.
 *
 *   If ALL partitions are deleted, the CVM_ASSET_STATUS message should not contain any
 *   partition information.
 *
 *   If CME requests that specific partition(s) be deleted, in the normal case VSIS
 *   would delete that (or those) partitions and respond with code 30 including a
 *   CVT_ASS_PARTITION tag specifying ONLY which partitions were deleted.
 *
 *   When specific partitions are to be deleted, VSIS should -- on a "best-effort"
 *   basis -- ensure that all requested partitions CAN be deleted before ANY
 *   partitions are deleted. If VSIS ascertains that some partitions can not be
 *   deleted, it should not attempt to delete any partitions and should send a code 31
 *   response (i.e., CVM_ASSET_STATUS message with field CVT_LAR_STATUS containing
 *   CACHE_CONTENT_NOT_DELETED), with NO partition information. All this only if VSIS
 *   can actually make the determination that not all assets can be deleted per request.
 *
 *   Message code 31 (CACHE_CONTENT_NOT_DELETED) is returned when VSIS is unable to
 *   delete a file. For example, if the asset is being imported. Or perhaps VSIS has
 *   some special files it doesn't want to delete. In this case, asset metadata is
 *   not necessary. 
 *
 *   If for some reason (meaning "best-effort" wasn't good enough) VSIS manages to
 *   delete some partitions but not all requested partitions, VSIS will need to send
 *   a CVM_ASSET_STATUS message identifying the cluster and asset, and code 30
 *   (CACHE_CONTENT_DELETED), with a CVT_ASS_PARTITION tag specifying ONLY which
 *   partitions were deleted. CME will then infer which partitions were not deleted.
 *
 *   Message codes 60 (CACHE_CONTENT_NOT_CDN) and 61 (CACHE_CONTENT_NO_BANDWIDTH) are
 *   only valid in response to a CVM_IMPORT_REQUEST message (q.v.)
 *
 *   Message code 70 (CACHE_CONTENT_PENDING) is internal to CME and is not to be
 *   returned by VSIS.
 *
 *   Message code 71 (CACHE_CONTENT_FRAGMENT_ONLY) is sent by VSIS when CME erroneously
 *   requests a whole asset status on a fragment-caching system.
 *
 *   Message code 500 (CACHE_CONTENT_INSRVERR) is not expected, but CME will log it
 *   and maybe hit a breakpoint. This happens from time to time with LAM.
 ****/

enum {CVT_LAR_CLUID=1101, CVT_LAR_ASSETS, CVT_LAR_STATUS};
enum {CVS_STS_ILLEGAL=0, CVS_STS_NORMAL, CVS_STS_NONEFOUND,
	  CVS_STS_NOTIMPLEMENTED=70, CVS_STS_BADPARAM, CVS_STS_OTHERERROR};

/**** CVM_LIST_ASSETS_RESPONSE message format:
 *
 *  CME sends a CVM_LIST_ASSETS_RESPONSE message to VSIS when it needs to validate its
 *  internal list of known assets. Typically this happens only at CME startup.
 *
 *  +-----------------------+--------------------------+
 *  |        Length         | CVM_LIST_ASSETS_RESPONSE |
 *  +-----------------------+--------------------------+
 *  |      sub-length       |      CVT_LAR_CLUID       |
 *  +-----------------------+--------------------------+
 *  |    "Cluster ID" of all assets in this message    |
 *  +-----------------------+--------------------------+
 *  |      sub-length       |      CVT_LAR_ASSETS      |
 *  +------------------------+-------------------------+
 *  |          Multi-SZ list of asset names            |
 *  +--------------------------------------------------+
 *  |                  ... etc. ...                    |
 *  +-----------------------+--------------------------+
 *  |    sub-length = 8     |     CVT_LAR_STATUS       |
 *  +------------------------+-------------------------+
 *  |          Final status (32-bit int)               |
 *  +--------------------------------------------------+
 *
 *  This message is a bit more complex than others because it attempts to be efficient.
 *  Within the CVM_LIST_ASSETS_RESPONSE message is a single CVT_LAR_CLUID entry, which
 *  identifies the VSIS cluster and applies to all assets contained within the message.
 *  The CVT_LAR_CLUID entry is required.
 *
 *  Following the CVT_PAR_CLUID entry is (usually) a CVT_LAR_ASSETS entry. The value data
 *  is 8-bit text specifying the asset names described by this message. Each asset name
 *  is a NUL-terminated character string; all assets in the message appear in a single
 *  CVT_LAR_ASSETS entry in Windows MULTI_SZ format, i.e., a sequence of asset names each
 *  NUL-terminated, with the entire sequence terminated by another NUL.
 *
 *  Asset "names" consist of initial 20-character PAIDs, remaining characters are the
 *  associated PID. It is porbably a layering violation for CME to know how Vstrm builds
 *  names from PID/PAID pairs, but this knowledge is isolated to one routine within the
 *  CMEVSIS module.
 *
 *  If there are no asset names in the message, which is legal, the CVT_LAR_ASSETS entry
 *  can be omitted.
 *
 *  When VSIS receives a CVM_LIST_ASSETS_REQUEST message it sends as many
 *  CVM_LIST_ASSETS_RESPONSE messages as are required to list all cluster assets
 *  matching the selection criteria in the original CVM_LIST_ASSETS_REQUEST message.
 *  Each message must include a CVT_LAR_CLUID entry. In the last message, VSIS includes
 *  a CVT_LAR_STATUS entry to indicate completion status. The possible CVT_LAR_STATUS
 *  values are:
 *
 *  CVS_STS_ILLEGAL=0       Code 0 is an illegal status and indicates a software bug
 *  CVS_STS_NORMAL          Normal successful completion
 *  CVS_STS_NONEFOUND       VSIS looked at its inventory but failed to find any assets
 *                          matching the specified PID and PAID
 *  CVS_STS_NOTIMPLEMENTED  The original CVM_LIST_ASSETS_REQUEST message specified an
 *                          option (e.g., wildcard) currently not implemented in VSIS
 *  CVS_STS_BADPARAM        VSIS did not recognize a parameter or option on the request
 *  CVS_STS_OTHERERROR      VSIS detected some other error not described above
 *
 *  Within the asset list there are no ordering requirements. The only ordering mandate
 *  is that CVT_LAR_STATUS be the last entry in the last message satisfying the original
 *  CVM_LIST_ASSETS_REQUEST message from CME.
 *
 ****/

enum {CVT_EXP_PID=1201, CVT_EXP_PAID};

/**** CVM_EXPIRE_NOTICE message format:
 *
 *  CME sends VSIS a CVM_EXPIRE_NOTICE when CME determines it will
 *  no longer store data about an asset.
 *
 *  +-------------------+--------------------+
 *  |      Length       | CVM_EXPIRE_NOTICE  |  CVM_EXPIRE_NOTICE==40
 *  +-------------------+--------------------+
 *  |    sub-length     |    CVT_EXP_PID     |
 *  +----------------------------------------+
 *  |         Asset "Provider ID"            |
 *  +----------------------------------------+
 *  |    sub-length     |    CVT_EXP_PAID    |
 *  +----------------------------------------+
 *  |      Asset "Provider Asset ID"         |
 *  +----------------------------------------+
 *
 * What VSIS chooses to do with this message is up to VSIS.
 *
 ****/

enum {CVT_DED_PID=1301, CVT_DED_PAID, CVT_DED_FRAC};
#define FRAC_SIZE (sizeof(float))			// Fraction of asset viewed (might be > 1.0)

/**** CVM_DEDUCT_NOTICE message format:
 *
 *  CME sends VSIS a CVM_DEDUCT_NOTICE when an uncached asset
 *  wasn't played fully on a whole-asset cache system.
 *
 *  +-------------------+--------------------+
 *  |      Length       | CVM_DEDUCT_NOTICE  |  CVM_DEDUCT_NOTICE==41
 *  +-------------------+--------------------+
 *  |    sub-length     |    CVT_DED_PID     |
 *  +----------------------------------------+
 *  |         Asset "Provider ID"            |
 *  +----------------------------------------+
 *  |    sub-length     |    CVT_DED_PAID    |
 *  +----------------------------------------+
 *  |      Asset "Provider Asset ID"         |
 *  +----------------------------------------+
 *  |    sub-length     |    CVT_DED_FRAC    |
 *  +----------------------------------------+
 *  |    Fraction of asset viewed (float)    | // 4 bytes floating
 *  +----------------------------------------+
 *
 * What VSIS chooses to do with this message is up to VSIS. CME imagines VSIS will set
 * AssetPlayCount += <Fraction> - 1.0, where <Fraction> is the floating value supplied.
 *
 ****/

enum {CVT_DISK_FREE=1401, CVT_DISK_AVAIL, CVT_DISK_CLUID, CVT_DISK_BW_USED, CVT_DISK_BW_LIMIT, CVT_STREAMING_USED, CVT_STREAMING_LIMIT};

/**** CVM_DISK_USAGE_REPORT message format:
 *
 *  VSIS sends CME a CVM_DISK_USAGE_REPORT message whenever it so desires. The term "Disk usage"
 *  is somewhat broad, encompassing both current disk space and current disk write bandwidth.
 *  It now also includes Streaming capacity and current streaming load
 *
 *  +---------------------+-----------------------+
 *  |      Length         | CVM_DISK_USAGE_REPORT |
 *  +---------------------+-----------------------+
 *  |    sub-length=12    |    CVT_DISK_FREE      |  12 = SIZEOF(USHORT) + SIZEOF(USHORT) + SIZEOF(INT64)
 *  +---------------------+-----------------------+
 *  |                                             |
 *  |           Storage Free (INT64)              |
 *  +---------------------+-----------------------+
 *  |    sub-length=12    |    CVT_DISK_AVAIL     |  12 = SIZEOF(USHORT) + SIZEOF(USHORT) + SIZEOF(INT64)
 *  +---------------------+-----------------------+
 *  |                                             |
 *  |         Storage Capacity (INT64)            |
 *  +---------------------+-----------------------+
 *  |    sub-length=12    |   CVT_DISK_BW_USED    |  12 = SIZEOF(USHORT) + SIZEOF(USHORT) + SIZEOF(INT64)
 *  +---------------------+-----------------------+
 *  |                                             |
 *  |   Current disk write rate bit/sec (INT64)   |  VSIS' best-guess. If not avaiable, do not provide.
 *  +---------------------+-----------------------+
 *  |    sub-length=12    |   CVT_DISK_BW_LIMIT   |  12 = SIZEOF(USHORT) + SIZEOF(USHORT) + SIZEOF(INT64)
 *  +---------------------+-----------------------+
 *  |                                             |
 *  |   Maximum disk write rate bit/sec (INT64)   |  VSIS' best-guess. If not available, do not provide.
 *  +---------------------+-----------------------+
 *  |    sub-length       |    CVT_DISK_CLUID     |
 *  +---------------------+-----------------------+
 *  |             VSIS "Cluster ID"               |
 *  +---------------------+-----------------------+
 *  |    sub-length=12    |    CVT_STREAMING_USED |
 *  +---------------------+-----------------------+
 *  |         Node Streaming Bandwidth            |
 *  +---------------------+-----------------------+
 *  |    sub-length=12    |    CVT_STREAMING_LIMIT|
 *  +---------------------+-----------------------+
 *  |           Node Streaming Limit              |
 *  +---------------------+-----------------------+
 *
 ****/

enum {CVT_SPACE_PERCENT=1501, CVT_PLAYS_IMPORT_ALL, CVT_EXPRESS_COUNT};

/**** CVM_CME_PARAMETERS message format:
 *
 *  CME sends (each) VSIS a CVM_CME_PARAMETERS message upon connecting and subsequent times as 
 *  required by the workload. This tells VSIS the value of pertinent CME parameters and suggested 
 *  VSIS parameters.
 *
 *  +---------------------+-----------------------+
 *  |       Length        |  CVM_CME_PARAMETERS   |  Currently length==12 but may increase
 *  +---------------------+-----------------------+
 *  |    sub-length=8     |   CVT_SPACE_PERCENT   |  8 = SIZEOF(USHORT) + SIZEOF(USHORT) + SIZEOF(ULONG)
 *  +---------------------+-----------------------+
 *  | 32-bit unsigned long free space % threshold |  (Default if not present: 5)
 *  +---------------------+-----------------------+
 *  |    sub-length=8     | CVT_PLAYS_IMPORT_ALL  |  8 = SIZEOF(USHORT) + SIZEOF(USHORT) + SIZEOF(ULONG)
 *  +---------------------+-----------------------+
 *  | 32-bit unsigned long plays-to-import count  |  (Default if not present: None)
 *  +---------------------+-----------------------+
 *  |    sub-length=8     |   CVT_EXPRESS_COUNT   |  8 = SIZEOF(USHORT) + SIZEOF(USHORT) + SIZEOF(ULONG)
 *  +---------------------+-----------------------+
 *  |       32-bit CDN-plays-to-report count      |  (Default if not present: None)
 *  +---------------------+-----------------------+
 *
 *  The "space limit" is CME's internal threshold for this cluster, in units of "percentage". 
 *  If cluster free space falls below this theshold, CME will send DELETE requests to VSIS.
 *
 *  If specified, CVT_PLAYS_IMPORT_ALL directs VSIS to import an asset once the play count
 *  reaches or exceeds the specified value.
 *
 *  If specified, CVT_EXPRESS_COUNT is a value VSIS needs to pass to VODDrv. If a fragment is
 *  played from CDN more than this many times, VODDrv should report a "popularity" event to
 *  VSIS, which VSIS should relay to CME using a CVM_VOD_POP_DATA message. VSIS should
 *  ignore this parameter on whole-asset-cache systems.
 *
 *  Additional parameters may be specified at a later time. Currently there is no response
 *  from VSIS to CME.
 *
 ****/

enum {CVT_SSN_CLUID=1601, CVT_SSN_PID, CVT_SSN_PAID, CVT_SSN_SID};

/**** CVM_SESSION_START_NOTICE message format:  --FUTURE--
 *
 *  VSIS sends CME a CVM_SESSION_START_NOTICE message when giving Vstrm the go-ahead for a session start.
 *
 *  +-------------------------+--------------------------+
 *  |         Length          | CVM_SESSION_START_NOTICE |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |      CVT_SSN_CLUID       |  Cluster ID ("name")
 *  +-------------------------+--------------------------+
 *  |                 VSIS "Cluster ID"                  |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |       CVT_SSN_PID        |  PID of asset being played
 *  +-------------------------+--------------------------+
 *  |                Asset "Provider ID"                 |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |       CVT_SSN_PAID       |  PAID of asset being played
 *  +-------------------------+--------------------------+
 *  |             Asset "Provider Asset ID"              |
 *  +-------------------------+--------------------------+
 *  |       sub-length=8      |       CVT_SSN_SID        |  Session ID (Vstrm SESSION_ID)
 *  +-------------------------+--------------------------+
 *  |                 Vstrm session ID                   |
 *  +-------------------------+--------------------------+
 *
 *
 ****/

enum {CVT_SSE_CLUID=1701, CVT_SSE_PID, CVT_SSE_STATUS, CVT_SSE_VODDATA};

/**** CVM_SESSION_END_NOTICE message format:
 *
 *  VSIS sends CME a CVM_SESSION_END_NOTICE message when Vstrm signals a session-end event.
 *
 *  +-------------------------+--------------------------+
 *  |         Length          |  CVM_SESSION_END_NOTICE  |
 *  +-------------------------+--------------------------+
 *  |      sub-length=8       |       CVT_SSE_SID        |  Session ID (Vstrm SESSION_ID)
 *  +-------------------------+--------------------------+
 *  |                 Vstrm session ID                   |
 *  +-------------------------+--------------------------+
 *  |      sub-length=8       |     CVT_SSE_STATUS       |  Session Final status
 *  +-------------------------+--------------------------+
 *  |                Vstrm status code                   |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |      CVT_SSE_CLUID       |  Cluster ID ("name")
 *  +-------------------------+--------------------------+
 *  |                 VSIS "Cluster ID"                  |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |     CVT_SSE_VODDATA      |
 *  +-------------------------+--------------------------+
 *  |                                                    |
 *  |   Opaque session statistics supplied by VodDrv     |
 *  |                                                    |
 *  +-------------------------+--------------------------+
 *
 *
 ****/

enum {CVT_IMP_PID=1901, CVT_IMP_PAID, CVT_IMP_COUNT};

/**** CVM_IMPORT_REQUEST message format:
 *
 *  CME sends VSIS a CVM_IMPORT_REQUEST when CME thinks a particular
 *  asset is popular enough to import into cache.
 *
 *  +-------------------+--------------------+
 *  |      Length       | CVM_IMPORT_REQUEST |
 *  +-------------------+--------------------+
 *  |    sub-length     |    CVT_IMP_PID     |
 *  +----------------------------------------+
 *  |         Asset "Provider ID"            |
 *  +----------------------------------------+
 *  |    sub-length     |    CVT_IMP_PAID    |
 *  +----------------------------------------+
 *  |      Asset "Provider Asset ID"         |
 *  +----------------------------------------+
 *  |   sub-length=8    |    CVT_IMP_COUNT   | // [Optional, PWE use]
 *  +----------------------------------------+
 *  |    Count of stream-thru sessions       |
 *  +----------------------------------------+
 *
 *  PID and PAID carry their usual meanings. CVT_IMP_COUNT tells VSIS
 *  how many streams are currently playing this asset in stream-through.
 *  This value is approximate but usually quite close. Its presence is
 *  optional; its use is to help VSIS determine the "importance" of an
 *  import request. Usually this value is present for PWE assets.
 *
 *  Barring cluster state transitions, CME will always ask the same VSIS
 *  to import any particular asset.
 *
 *  Note this is a "request", not a "command". VSIS should attempt to
 *  comply on a "best effort" basis but CME cannot depend on VSIS here,
 *  since VSIS may not be able to import the asset for a variety of
 *  reasons.
 *
 *  Under ideal circumstances, VSIS will fire off a worker thread to
 *  import the asset. Once the worker thread has started to import the
 *  asset (in particular, some bytes have been written to flash), VSIS
 *  will send CME a CVM_ASSET_STATUS message with code 11
 *  (CACHE_CONTENT_STREAMABLE) in the status field. Later on, after
 *  the import completes successfully, VSIS sends CME another
 *  CVM_ASSET_STATUS message with code 10 (CACHE_CONTENT_COMPLETED)
 *  for the asset.
 *
 *  There are various possibilities other than "ideal circumstances",
 *  which can be categorized as follows:
 *
 *   1. VSIS may not have the bandwidth to import the asset. In this
 *   case VSIS returns CACHE_CONTENT_NO_BANDWIDTH and stops trying to
 *   import. CME may request the import again at a later time, or may
 *   not, as cirsumstances dictate.
 *
 *   2. VSIS may be unable to locate the asset on the CDN. This might
 *   be a result of a race condition between making an asset available
 *   for play and initiating a PWE import on the CDN. In this case
 *   VSIS returns CACHE_CONTENT_NOT_CDN and stops trying to import.
 *   CME may request the import again at a later time, or may not, as
 *   cirsumstances dictate.
 *
 *   3. VSIS might already be importing the requested asset. In this
 *   case VSIS "drops" the request message and does not respond. (It
 *   is assumed the session doing the import will send the _STREAMABLE
 *   and _COMPLETED messages to CME when appropriate).
 *
 *   4. VSIS may issue a Vstrm call to import the asset, but the
 *   *call* may fail. If the call fails because of file-not-found or
 *   no bandwidth available, cases 1 and 2 above, VSIS has already
 *   returned an error to CME. In other cases, VSIS "drops" the
 *   request message and does not respond to CME. Since the *call*
 *   failed, no data has been imported and therefore VSIS has not yet
 *   sent CME a CACHE_CONTENT_STREAMABLE message. (This is admittedly
 *   a simplification of the more complex problem of importing a
 *   number of files. Main point is, if VSIS can't bring in the index
 *   file then it drops the request. For other files the gating issue
 *   is if VSIS has sent the _STREAMABLE message to CME as discussed
 *   below).
 *
 *   5. If the Vstrm call succeeds and (presumably) the import session
 *   has started, VSIS will find itself in one of two states.
 *   Initially no data will be brought into cache as there are normal
 *   latencies in Vstrm, then data will (probably) start arriving in
 *   the file. At some point "enough" data may have been brought in
 *   to convince VSIS that the operation is proceeding normally. What
 *   constitutes "enough" is a loose term, up to VSIS to decide, but
 *   must be greater than 0. This leads to two possibilities:
 *
 *   5a. The import session terminates before "enough" data has been
 *   imported. In this case, VSIS "drops" the request message and
 *   does not respond to CME. 
 *
 *   5b. If "enough" data has been brought in, VSIS will send CME a
 *   CVM_ASSET_STATUS message with code 11 (CACHE_CONTENT_STREAMABLE)
 *   in the status field. Once the code 11 message has been sent, VSIS
 *   is responsible for sending some sort of completion message
 *   regardless of the outcome. If the import completes successfully,
 *   VSIS sends a code 10 (CACHE_CONTENT_COMPLETED). If the import
 *   fails AFTER VSIS sent the CACHE_CONTENT_STREAMABLE message, VSIS
 *   can either send the asset to the doctor for a healing attempt, or
 *   (1) delete all the cached asset files and (2) send CME a code 30
 *   (CACHE_CONTENT_DELETED) message for that asset.
 *
 *   The general principle is that VSIS can (a) ignore the CME message
 *   or (b) when reasonable send a code 11 (CACHE_CONTENT_STREAMABLE).
 *   In case (b) VSIS must subsequently send a code 10 or code 30
 *   message depending on whether the import succeeded or failed,
 *   respectively. Furthermore, if the import failed VSIS must delete
 *   all the cached files associated with the asset.
 *
 *   ---------- Other ----------
 *
 *   CME may send VSIS a CVM_STATUS_REQUEST for the asset. Such a
 *   request would normally boil down to one of two possibilities:
 *   either the asset has been declared STREAMABLE or not, and if
 *   STREAMABLE either the import is still in progress or has
 *   completed, in which case the CACHE_CONTENT_COMPLETED message has
 *   also been sent to CME. The STREAMABLE and COMPLETED cases should
 *   already be handled by VSIS. The difficult case is if CME has
 *   requested an import and the import is in process (i.e., there is
 *   an active worker thread for the import) but VSIS has not yet
 *   gotten to the point where the asset is STREAMABLE. If CME should
 *   request asset status at this time, VSIS should *Ignore* the
 *   request. On the other hand, if CME has requested an import and
 *   VSIS "dropped" the message (cases 1-3 above) VSIS should respond
 *   with a status code 1 (CACHE_CONTENT_NONLOCAL). This should be how
 *   things work today, so VSIS need not keep extra state.
 *
 ****/



enum {CVT_SCQ_PID=2001, CVT_SCQ_PAID};

/**** CVM_SESS_COUNT_REQ message format:
 *
 *  CME sends VSIS a CVM_SESS_COUNT_REQ when CME needs to know how many
 *  sessions are playing an asset.
 *
 *  +-------------------+--------------------+
 *  |      Length       | CVM_SESS_COUNT_REQ |
 *  +-------------------+--------------------+
 *  |    sub-length     |    CVT_SCQ_PID     |
 *  +----------------------------------------+
 *  |         Asset "Provider ID"            |
 *  +----------------------------------------+
 *  |    sub-length     |    CVT_SCQ_PAID    |
 *  +----------------------------------------+
 *  |      Asset "Provider Asset ID"         |
 *  +----------------------------------------+
 *
 *  Upon receipt, VSIS should call an appropriate Vstrm routine,
 *  such as VstrmGetAllSessionChars() and count the number of 
 *  sessions playing the asset given by the PID/PAID. This value,
 *  which may be 0, would then be returned to CME in a CVM_SESS_COUNT_RSP
 *  response message.
 *
 *  Messages of this type are expected to be infrequent since
 *  they are used only for PWE assets and needed only when the
 *  asset play count exceeds the import threshold.
 *
 ****/


enum {CVT_SCR_PID=2101, CVT_SCR_PAID, CVT_SCR_CLUID, CVT_SCR_COUNT};

/**** CVM_SESS_COUNT_RSP message format:
 *
 *  VSIS sends CME a CVM_SESS_COUNT_RSP message in response to a previous
 *  CVM_SESS_COUNT_REQ message. All CVT_xxx fields are required.
 *
 *  +-------------------------+--------------------------+
 *  |         Length          |    CVM_SESS_COUNT_RSP    |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |      CVT_SCR_CLUID       |  Cluster ID ("name")
 *  +-------------------------+--------------------------+
 *  |                 VSIS "Cluster ID"                  |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |       CVT_SCR_PID        |  PID of asset being played
 *  +-------------------------+--------------------------+
 *  |                Asset "Provider ID"                 |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |       CVT_SCR_PAID       |  PAID of asset being played
 *  +-------------------------+--------------------------+
 *  |             Asset "Provider Asset ID"              |
 *  +-------------------------+--------------------------+
 *  |       sub-length=8      |      CVT_SCR_COUNT       |  Session count
 *  +-------------------------+--------------------------+
 *  |    Count of Vstrm sessions playing this asset      |
 *  +-------------------------+--------------------------+
 *
 *  CVT_SCR_COUNT passes the number of sessions found to be
 *  playing this asset at the time of the CVM_SESS_COUNT_REQ
 *  message.
 *
 ****/

enum {CVT_SMT_CLUID, CVT_SMT_DISKID, CVT_SMT_ERASES, CVT_SMT_ON_HOURS, CVT_SMT_USED_LIFE};

/**** CVM_SMART_PARAMETERS message format:
 *
 *  VSIS sends CME a CVM_SMART_PARAMETERS message periodically at VSIS' convenience. 
 *  VSIS is expected to obtain the data via DAC API calls made during "quiet time".
 *
 *  Each message supplies all data below for exactly one disk drive.
 *
 *  +-------------------------+--------------------------+
 *  |         Length          |   CVM_SMART_PARAMETERS   |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |      CVT_SMT_CLUID       |  Cluster ID ("name")
 *  +-------------------------+--------------------------+
 *  |                 VSIS "Cluster ID"                  |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |     CVT_SMT_DISKID       |  Drive ID (Serial Number)
 *  +-------------------------+--------------------------+
 *  |                 Disk drive serial                  |  ASCIZ string of Serial Number
 *  +-------------------------+--------------------------+
 *  |      sub-length=4       |     CVT_SMT_ERASES       |  Erase cycles this drive claims to support
 *  +-------------------------+--------------------------+
 *  |           Count of drive erase cycles              |  (INT32) "Rated PE Cycles" -- SMART parameter
 *  +-------------------------+--------------------------+
 *  |      sub-length=4       |     CVT_SMT_ON_HOURS     |  Hours drive has been "ON"
 *  +-------------------------+--------------------------+
 *  |            Number of hours drive "on"              |  (INT32) hours -- SMART parameter
 *  +-------------------------+--------------------------+
 *  |      sub-length=4       |    CVT_SMT_USED_LIFE     |  Drive lifetime used (0-100), per cent.
 *  +-------------------------+--------------------------+
 *  |            Drive Lifetime (Per Cent)               |  (INT32) Per Cent of drive lifetime used
 *  +-------------------------+--------------------------+
 *  
 *
 *  If a specified parameter is not available, it is omitted (i.e., not sent as 0).
 *
 ****/


enum {CVT_IMF_PID=2201, CVT_IMF_PAID, CVT_IMF_PARTITION, CVT_IMF_FRAGMENT};

/**** CVM_IMPORT_FRAGMENT message format:
 *
 *  CME sends VSIS a CVM_IMPORT_FRAGMENT message when CME thinks a particular
 *  fragment is popular enough to import into cache.
 *
 *  +-------------------+---------------------+
 *  |      Length       | CVM_IMPORT_FRAGMENT |
 *  +-------------------+---------------------+
 *  |    sub-length     |    CVT_IMF_PID      |
 *  +-----------------------------------------+
 *  |         Asset "Provider ID"             |
 *  +-----------------------------------------+
 *  |    sub-length     |    CVT_IMF_PAID     |
 *  +-----------------------------------------+
 *  |      Asset "Provider Asset ID"          |
 *  +-----------------------------------------+
 *  |   sub-length=8    |  CVT_IMF_PARTITION  |
 *  +-----------------------------------------+
 *  |             Partition number            |
 *  +-----------------------------------------+
 *  |   sub-length=8    |  CVT_IMF_FRAGMENT   |
 *  +-----------------------------------------+
 *  |             Fragment number             |
 *  +-----------------------------------------+
 *
 *  PID and PAID carry their usual meanings. CVT_IMF_PARTITION indicates
 *  which partition is being requested and CVT_IMF_FRAGMENT tells VSIS which
 *  fragment in that partition.
 *
 *  Under normal circumstances VSIS should be able to import the fragment and
 *  send a code 10 (CACHE_CONTENT_COMPLETED) status in a CVM_FRAGMENT_STATUS
 *  message to CME. If the import fails, VSIS still needs to return a
 *  completion message to CME, with as specific an error code as can be
 *  envisioned. (TBD)
 *
 *
 *
 ****/

enum {CVT_DLF_PID=2301, CVT_DLF_PAID, CVT_DLF_PARTITION, CVT_DLF_FRAGMENT};

/**** CVM_DELETE_FRAGMENT message format:
 *
 *  CME sends VSIS a CVM_DELETE_FRAGMENT when CME determines VSIS should
 *  delete a fragment.
 *
 *  +-------------------+---------------------+
 *  |      Length       | CVM_DELETE_FRAGMENT |
 *  +-------------------+---------------------+
 *  |    sub-length     |    CVT_DLF_PID      |
 *  +-----------------------------------------+
 *  |         Asset "Provider ID"             |
 *  +-----------------------------------------+
 *  |    sub-length     |    CVT_DLF_PAID     |
 *  +-----------------------------------------+
 *  |      Asset "Provider Asset ID"          |
 *  +-----------------------------------------+
 *  |    sub-length=8   | CVT_DLF_PARTITION   |
 *  +-----------------------------------------+ 
 *  |           partition number              | 
 *  +-----------------------------------------+ 
 *  |    sub-length=8   |  CVT_DLF_FRAGMENT   |
 *  +-----------------------------------------+ 
 *  |            fragment number              | 
 *  +-----------------------------------------+ 
 *
 *  On success, VSIS sends a CVM_FRAGMENT_STATUS message code 30
 *  (CACHE_CONTENT_DELETED). See the CVM_FRAGMENT_STATUS message for full
 *  details.
 *
 *  If CVT_DLF_PARTITION is INDEX_FILE_PARTITION, the entire asset is to be deleted.
 *
 */

enum {CVT_FST_PID=2401, CVT_FST_PAID, CVT_FST_CLUID, CVT_FST_PARTITION, CVT_FST_FRAGMENT, CVT_FST_STATUS};

/**** CVM_FRAGMENT_STATUS message format:
 *
 *  VSIS sends CME a CVM_FRAGMENT_STATUS message in response to import and
 *  delete requests.
 *
 *  +-------------------+---------------------+
 *  |      Length       | CVM_FRAGMENT_STATUS |
 *  +-------------------+---------------------+
 *  |    sub-length     |    CVT_FST_PID      |
 *  +-----------------------------------------+
 *  |         Asset "Provider ID"             |
 *  +-----------------------------------------+
 *  |    sub-length     |    CVT_FST_PAID     |
 *  +-----------------------------------------+
 *  |      Asset "Provider Asset ID"          |
 *  +-----------------------------------------+
 *  |    sub-length     |    CVT_FST_CLUID    |  Cluster ID ("name")
 *  +-------------------+---------------------+
 *  |           VSIS "Cluster ID"             |
 *  +-------------------+---------------------+
 *  |    sub-length=8   | CVT_FST_PARTITION   |
 *  +-----------------------------------------+ 
 *  |           partition number              | 
 *  +-----------------------------------------+ 
 *  |    sub-length=8   |  CVT_FST_FRAGMENT   |
 *  +-----------------------------------------+ 
 *  |            fragment number              | 
 *  +-----------------------------------------+ 
 *  |    sub-length=8   |   CVT_FST_STATUS    |
 *  +-----------------------------------------+ 
 *  |            fragment status              | 
 *  +-----------------------------------------+ 
 *
 *  The standard fields PIS/PAID/CLUID/PARTITION/FRAGMENT need no
 *  explanation.
 *
 *  The fragment status field is one of the standard status values:
 *
 *  Code                            Reason
 *
 *  CACHE_CONTENT_COMPLETED         Fragment imported OK
 *  CACHE_CONTENT_NOT_DELETED       Fragment not in cache, or file write-protected,
 *                                  or NTFS error creating file fragment.
 *  CACHE_CONTENT_REQ_INVALID       Fragment not part of file address space.
 *
 *  If the CVT_FST_PARTITION is INDEX_FILE_PARTITION then the entire asset has been deleted.
 *
 *
 ****/

enum {CVT_FSR_PID=2501, CVT_FSR_PAID, CVT_FSR_PARTITION, CVT_FSR_FRAGMENT};

/**** CVM_FRAGSTS_REQUEST message format:
 *
 *  CME sends VSIS a CVM_FRAGSTS_REQUEST when CME needs to know a particular fragment's
 *  cache state.
 *
 *  +-------------------+---------------------+
 *  |      Length       | CVM_FRAGSTS_REQUEST |
 *  +-------------------+---------------------+
 *  |    sub-length     |    CVT_FSR_PID      |
 *  +-----------------------------------------+
 *  |         Asset "Provider ID"             |
 *  +-----------------------------------------+
 *  |    sub-length     |    CVT_FSR_PAID     |
 *  +-----------------------------------------+
 *  |      Asset "Provider Asset ID"          |
 *  +-----------------------------------------+
 *  |    sub-length=8   | CVT_FSR_PARTITION   |
 *  +-----------------------------------------+ 
 *  |           partition number              | 
 *  +-----------------------------------------+ 
 *  |    sub-length=8   |  CVT_FSR_FRAGMENT   |
 *  +-----------------------------------------+ 
 *  |            fragment number              | 
 *  +-----------------------------------------+ 
 *
 *  VSIS is expected to respond with a CVM_FRAGMENT_STATUS message (see
 *  above). In this case the status fields have the following meaning:
 *
 *  CACHE_CONTENT_COMPLETED         Fragment present in cache
 *  CACHE_CONTENT_STREAMABLE        Fragment import requested, not yet complete
 *  CACHE_CONTENT_DELETED           Fragment not present in cache, not coming in,
 *                                  possible in the process of being deleted.
 *  CACHE_CONTENT_REQ_INVALID       No such fragment exists for this partition.
 *
 */

enum {CVT_VOD_CLUID=2601, CVT_VOD_VODDATA};

/**** CVM_VOD_POP_DATA and CVM_LAST_VOD_POP_DATA message format:
 *
 *  VSIS sends CME a CVM_VOD_POP_DATA message when VODDrv hands VSIS some
 *  fragment usage data. Typically the VODDrv data is carved into a number
 *  of smaller messages by VSIS, and CME collects these messages until the
 *  final buffer is sent in a CVM_LAST_VOD_POP_DATA message.
 *
 *  +-------------------------+--------------------------+
 *  |         Length          |     CVM_VOD_POP_DATA     |  Or CVM_LAST_VOD_POP_DATA
 *  +-------------------------+--------------------------+
 *  |       sub-length        |      CVT_VOD_CLUID       |  Cluster ID ("name")
 *  +-------------------------+--------------------------+
 *  |                 VSIS "Cluster ID"                  |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |     CVT_VOD_VODDATA      |
 *  +-------------------------+--------------------------+
 *  |                                                    |
 *  |   Opaque fragment statistics supplied by VODDrv    |
 *  |                                                    |
 *  +-------------------------+--------------------------+
 *
 *
 ****/

/**** CVM_LIST_FRAGMENTS message format:
 *
 *  +-------------------------+--------------------------+
 *  |        Length=4         |    CVM_LIST_FRAGMENTS    |
 *  +-------------------------+--------------------------+
 *
 *  This message requests VSIS to take an inventory of all known files and the 
 *  fragments cached for these files. The return is a series of CVM_KNOWN_FRAGMENTS
 *  messages as outlined below, followed by a CVM_KNOWN_FRAGMENTS_END message.
 *
 ****/


enum{CVT_KNF_PID=2801, CVT_KNF_PAID, CVT_KNF_CLUID, CVT_KNF_PARTITION, 
	 CVT_KNF_FRAGCOUNT, CVT_KNF_FRAGMENT};

/**** CVM_KNOWN_FRAGMENTS message format:
 *
 *  +-------------------+---------------------+
 *  |      Length       | CVM_KNOWN_FRAGMENTS |
 *  +-------------------+---------------------+
 *  |    sub-length     |    CVT_KNF_PID      |
 *  +-----------------------------------------+
 *  |         Asset "Provider ID"             |
 *  +-----------------------------------------+
 *  |    sub-length     |    CVT_KNF_PAID     |
 *  +-----------------------------------------+
 *  |      Asset "Provider Asset ID"          |
 *  +-----------------------------------------+
 *  |    sub-length     |    CVT_KNF_CLUID    |  Cluster ID ("name")
 *  +-------------------+---------------------+
 *  |           VSIS "Cluster ID"             |
 *  +-------------------+---------------------+
 *  |    sub-length=8   | CVT_KNF_PARTITION   |
 *  +-----------------------------------------+ 
 *  |           partition number              | 
 *  +-----------------------------------------+ 
 *  |    sub-length=8   | CVT_KNF_FRAGCOUNT   |
 *  +-----------------------------------------+ 
 *  |   count of fragements this partition    | 
 *  +-----------------------------------------+ 
 *  |     sub-length    |  CVT_KNF_FRAGMENT   |
 *  +-----------------------------------------+ 
 *  |            fragment bitmap              |  // As many as needed
 *  +-----------------------------------------+ 
 *  |            fragment bitmap              |  // ...
 *  +-----------------------------------------+ 
 *  |            fragment bitmap              |  // ...
 *  +-----------------------------------------+ 
 *  |            fragment bitmap              |  // ...
 *  +-----------------------------------------+ 
 *
 ****/


/**** CVM_KNOWN_FRAGMENTS_END message format:
 *
 *  +-------------------------+--------------------------+
 *  |        Length=4         | CVM_KNOWN_FRAGMENTS_END  |
 *  +-------------------------+--------------------------+
 *
 *  This message tells CME there are no more fragments on this cluster,
 *  i.e., the previous CVM_KNOWN_FRAGMENTS messages have described the
 *  entire cluster contents.
 *
 ****/


enum {CVT_TLV_CLUID=2901, CVT_TLV_VODDATA};

/**** CVM_VOD_TLV_DATA message format:
 *
 *  VSIS sends CME a CVM_VOD_TLV_DATA message when VODDrv hands VSIS a TLV structure.
 *
 *  +-------------------------+--------------------------+
 *  |         Length          |     CVM_VOD_TLV_DATA     |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |      CVT_TLV_CLUID       |  Cluster ID ("name")
 *  +-------------------------+--------------------------+
 *  |                 VSIS "Cluster ID"                  |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |     CVT_TLV_VODDATA      |
 *  +-------------------------+--------------------------+
 *  |                                                    |
 *  |     Opaque fragment data supplied by VODDrv        |
 *  |                                                    |
 *  +-------------------------+--------------------------+
 *
 *
 ****/

enum {CVT_CDN_CLUID=3001, CVT_CDN_NODEID, CVT_CDN_CDNID, CVT_CDN_STATUS};

/**** CVM_VSIS_CDN_STATE message format:
 *
 *  VSIS sends CME a CVM_VSIS_CDN_STATE message when it detects a change in
 *  CDN connectivity.
 *
 *  +-------------------------+--------------------------+
 *  |         Length          |    CVM_VSIS_CDN_STATE    |
 *  +-------------------------+--------------------------+
 *  |       sub-length        |      CVT_CDN_CLUID       |  Cluster ID ("name")
 *  +-------------------------+--------------------------+
 *  |                 VSIS "Cluster ID"                  |
 *  +-------------------------+--------------------------+
 *  |      sub-length=8       |     CVT_CDN_NODEID       |  Node ID ("number")
 *  +-------------------------+--------------------------+
 *  |                    VSIS node ID                    |
 *  +-------------------------+--------------------------+
 *  |      sub-length=8       |      CVT_CDN_STATUS      |
 *  +-------------------------+--------------------------+
 *  |   status (0=not connected to CDN, 1=connected)     |
 *  +-------------------------+--------------------------+
 *
 *  For reasons unknown, sometimes a cluster node can lose CDN connectivity.
 *  VSIS is responsible for monitoring CDN connectivity and reporting changes to
 *  CME.
 *
 *  At the present time the message assumes a single CDN. On multi-CDN systems,
 *  until this entire message interface can identify CDNs by name, VSIS should
 *  report "not connected" when ANY CDN is unreachable, so "connected" means ALL
 *  CDNs are reachable.
 *
 *  When CME initially connects to VSIS the default state is 1 ("connected").
 *
 *  VSIS sends this message unsolicited whenever it detects a change in CDN
 *  connectivity, and at any other times of VSIS' choosing.
 *
 ****/


#endif  // #ifndef _INCLUDED_CMEVSIS  ]
