// ===========================================================================
// Copyright (c) 2004 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : $Id: CdmiFuseOps.h Exp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : common CDMI file operations for FUSE purposes
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/Generic/CdmiFuse/CdmiFuseOps.cpp $
// 
// 297   4/21/17 11:39a Hui.shao
// 
// 296   4/18/17 6:14p Hui.shao
// 
// 295   4/18/17 3:47p Hui.shao
// opendir/readdir
// 
// 294   4/17/17 3:24p Hui.shao
// 
// 293   4/14/17 3:46p Hui.shao
// 
// 292   4/14/17 3:24p Hui.shao
// 
// 291   4/10/17 2:31p Hui.shao
// 
// 290   4/10/17 11:12a Hui.shao
// uncacheChild()
// 
// 289   4/07/17 10:01a Hui.shao
// 
// 288   4/06/17 5:57p Hui.shao
// 
// 287   4/06/17 5:52p Hui.shao
// to reduce multiple concurrent queries for a same file to Aqua
// 
// 286   4/05/17 5:35p Hui.shao
// 
// 285   4/05/17 5:34p Hui.shao
// readContainer yield 1/30 of childrenTTL
// 
// 284   3/30/17 6:01p Hui.shao
// added readDir()
// 
// 283   3/28/17 6:10p Hui.shao
// 
// 282   3/28/17 6:07p Hui.shao
// 
// 281   3/28/17 5:46p Hui.shao
// _lkLogin
// 
// 280   3/28/17 5:01p Hui.shao
// 
// 279   3/28/17 4:55p Li.huang
// 278   10/17/16 5:20p Hui.shao
// 
// 277   10/17/16 5:15p Hui.shao
// 
// 276   10/17/16 5:08p Hui.shao
// 
// 275   10/17/16 2:46p Hui.shao
// 
// 274   10/17/16 2:37p Hui.shao
// lower the load to readDomain()
// 
// 273   9/30/16 1:14p Hongquan.zhang
// 
// 272   8/24/16 3:51p Hui.shao
// 
// 271   7/18/16 3:52p Hui.shao
// 
// 270   7/15/16 10:00a Hui.shao
// 
// 269   7/14/16 6:45p Hui.shao
// 
// 268   5/05/16 2:46p Hui.shao
// 
// 267   4/18/16 2:02p Hongquan.zhang
// 
// 266   4/15/16 3:25p Hongquan.zhang
// add keep_cache == false in CdmiFuse::open
// 
// 265   4/08/16 11:05a Hui.shao
// Aqua added an extension: x-aqua-move-overwrite=true
// 
// 264   4/05/16 2:39p Hui.shao
// 
// 263   3/09/16 3:39p Hui.shao
// merged from V2.5
// 
// 267   3/04/16 2:33p Hui.shao
// 
// 266   3/03/16 7:21p Hui.shao
// 
// 265   3/03/16 7:09p Hui.shao
// 
// 264   3/03/16 6:44p Hui.shao
// 
// 263   3/03/16 4:49p Hui.shao
// 
// 262   3/03/16 11:13a Hui.shao
// 
// 261   3/02/16 7:10p Hui.shao
// 
// 260   3/02/16 6:29p Hui.shao
// 
// 259   2/01/16 2:34p Li.huang
// 
// 258   2/01/16 2:27p Li.huang
// 
// 257   1/28/16 3:13p Hui.shao
// 
// 256   1/28/16 1:56p Hui.shao
// adjust ACL value by comparing cdmi_owner
// 
// 255   1/27/16 2:14p Hui.shao
// seperated threadpool for slow SearchByParent
// 
// 254   1/20/16 3:16p Hui.shao
// 
// 253   1/15/16 2:44p Hui.shao
// 
// 252   1/08/16 4:16p Hui.shao
// cache of dir children
// 
// 251   11/12/15 4:47p Hui.shao
// indicateClose must be about DataObject
// 
// 250   11/12/15 4:37p Hui.shao
// 
// 249   10/19/15 12:05p Dejian.fei
// 
// 248   9/10/15 4:41p Hongquan.zhang
// 
// 246   8/27/15 5:10p Li.huang
// fix bug 21650
// 
// 245   8/20/15 4:53p Li.huang
// 
// 244   8/20/15 3:08p Li.huang
// 
// 243   8/20/15 3:04p Li.huang
// fix bug 21651
// 
// 242   8/13/15 2:58p Li.huang
// 
// 241   8/13/15 2:45p Li.huang
// 
// 240   8/13/15 11:40a Li.huang
// add check root server healthy thread
// 
// 239   5/15/15 11:11a Hui.shao
// reset the payload len=0 of response-buflist for reading
// 
// 238   5/13/15 5:19p Hongquan.zhang
// 
// 237   4/23/15 1:31p Hui.shao
// 
// 236   4/08/15 4:41p Li.huang
// 
// 235   4/02/15 2:15p Hui.shao
// ReadDomain to call getServerIp() to sort the IPs
// 
// 234   3/31/15 5:42p Li.huang
// 
// 233   3/31/15 5:17p Li.huang
// 
// 232   3/31/15 2:21p Li.huang
// 
// 231   2/15/15 3:02p Hui.shao
// 
// 230   2/13/15 9:32a Hui.shao
// another stupid Aqua definition: it indeed did the action when redirect
// a empty PUT, no need to post the request to the new location
// 
// 229   2/12/15 11:29a Hui.shao
// cdmi_IndicateClose()
// 
// 228   2/03/15 1:20p Hui.shao
// 
// 227   1/26/15 12:22p Hui.shao
// 
// 226   1/26/15 12:21p Hui.shao
// 
// 224   1/21/15 10:22a Hui.shao
// 
// 223   12/04/14 2:19p Hui.shao
// readAquaDomain() to output the real domainURI of execution
// 
// 222   12/04/14 1:58p Hongquan.zhang
// 
// 220   11/21/14 1:06p Hui.shao
// take that of the source as prefered FrondEnd for cloning file
// 
// 219   11/19/14 8:01p Hui.shao
// userDomain
// 
// 218   11/18/14 3:57p Hongquan.zhang
// 
// 217   11/13/14 2:21p Hui.shao
// ticket#16748 to a) cleanup the cache of move-from, b) prefer to take
// the location of move-from to create new move-to
// 
// 216   11/07/14 12:49p Hui.shao
// 
// 215   11/07/14 11:24a Hui.shao
// 
// 214   9/28/14 11:41a Hongquan.zhang
// change working directory to crash folder
// change core filename pattern
// 
// 212   9/16/14 10:20a Hongquan.zhang
// 
// 211   8/28/14 10:35a Hui.shao
// 
// 210   8/27/14 3:41p Li.huang
// 
// 209   8/26/14 4:34p Hui.shao
// 
// 208   8/26/14 4:32p Hui.shao
// 
// 207   8/26/14 4:12p Hui.shao
// 
// 206   8/26/14 3:37p Hui.shao
// 
// 205   8/21/14 5:23p Hui.shao
// 
// 204   8/18/14 6:46p Hui.shao
// enable CURL_INSTANCE_KEEPALIVE
// 
// 203   8/14/14 5:55p Hui.shao
// CURL_INSTANCE_KEEPALIVE
// 
// 202   8/14/14 4:46p Hui.shao
// persistent cURL connection when callRemote()
// 
// 201   7/31/14 10:41a Hongquan.zhang
// 
// 200   7/21/14 2:46p Hongquan.zhang
// remove big lock && cache partition by uri
// 
// 199   7/17/14 9:34a Hongquan.zhang
// 
// 198   7/08/14 4:41p Hongquan.zhang
// add maxMergeCount in write queue
// 
// 197   7/08/14 11:38a Hongquan.zhang
// 
// 196   7/04/14 1:09p Hongquan.zhang
// 
// 195   7/02/14 3:27p Hongquan.zhang
// 
// 194   7/01/14 1:07p Hongquan.zhang
// 
// 193   6/25/14 2:52p Hongquan.zhang
// fix bug 19049, FUSE to dynamically determine readahead
// 
// 192   6/16/14 11:48a Hui.shao
// 
// 191   6/12/14 3:21p Hongquan.zhang
// support dynamic cache size
// 
// 190   6/11/14 1:49p Hui.shao
// 
// 188   6/11/14 10:47a Hui.shao
// 
// 187   6/09/14 9:56a Hui.shao
// cover the bug in jsoncpp that didn't test value type in isMember()
// 
// 186   6/06/14 9:41p Hui.shao
// 
// 185   6/06/14 9:08p Hui.shao
// 
// 184   6/06/14 9:05p Hui.shao
// 
// 183   6/06/14 5:27p Hui.shao
// 
// 182   6/05/14 8:03p Hui.shao
// 
// 181   6/05/14 7:20p Hui.shao
// 
// 179   6/04/14 6:02p Hui.shao
// stop get PathName from Aqua response
// 
// 178   6/04/14 4:55p Hui.shao
// 
// 176   6/04/14 3:35p Hongquan.zhang
// 
// 175   6/04/14 3:34p Hongquan.zhang
// 
// 174   6/04/14 3:31p Hongquan.zhang
// 
// 173   6/03/14 11:54a Li.huang
// 
// 172   5/28/14 6:45p Hui.shao
// 
// 171   5/28/14 4:52p Hui.shao
// take common::BufferList
// 
// 169   5/28/14 10:25a Hui.shao
// take smartpointer for contentbuffer 
// 
// 168   5/28/14 9:36a Li.huang
// 
// 166   5/27/14 9:40a Hui.shao
// invoke curl via buffer aggregation
// 
// 165   5/23/14 11:17a Hongquan.zhang
// 
// 164   4/30/14 10:50a Hui.shao
// swapped the low 16bit flag for curl to high-16bit
// 
// 163   4/23/14 5:00p Hui.shao
// more http status codes
// 
// 162   3/27/14 9:53a Hui.shao
// corrected some log printing
// 
// 161   3/24/14 7:21p Ketao.zhang
// 
// 160   3/24/14 1:30p Ketao.zhang
// check in for cdmifuseLinux clone
// 
// 159   2/26/14 1:04p Hongquan.zhang
// 
// 158   2/26/14 11:43a Hui.shao
// 
// 157   2/26/14 11:19a Hui.shao
// to export the server-side parameters retrived from AquaDomain
// 
// 156   2/24/14 1:47p Hui.shao
// to call AquaClient in a DLL
// 
// 155   12/30/13 5:23p Hui.shao
// 
// 153   12/27/13 12:40p Hongquan.zhang
// 
// 152   12/18/13 3:03p Hongquan.zhang
// 
// 151   12/04/13 6:45p Hui.shao
// reduce memory copies on request/response body
// 
// 150   11/29/13 4:58p Hui.shao
// 
// 149   11/29/13 4:48p Hui.shao
// json config for sdk
// 
// 148   11/26/13 11:32a Hongquan.zhang
// 
// 147   11/22/13 12:40p Hui.shao
// replaced resize with reserve
// 
// 144   11/18/13 11:28a Hui.shao
// 
// 143   11/01/13 9:42a Hui.shao
// 
// 142   11/01/13 9:37a Hui.shao
// 
// 141   10/31/13 1:40p Hui.shao
// ST_MODE_BIT_OFFSET_USER
// 
// 140   10/29/13 9:19a Hui.shao
// 
// 139   10/25/13 9:45a Hongquan.zhang
// 
// 138   9/06/13 3:06p Hongquan.zhang
// 
// 137   9/06/13 1:50p Hui.shao
// 
// 136   9/06/13 11:16a Hongquan.zhang
// 
// 135   9/06/13 10:23a Hongquan.zhang
// 
// 134   8/16/13 3:22p Li.huang
// fix domain uri error
// 
// 133   8/14/13 1:11p Hui.shao
// retry on some special cases of send-request failed
// 
// 132   8/13/13 11:46a Hui.shao
// 
// 131   8/13/13 11:33a Hui.shao
// Aqua3 start supporting search for both folder and files
// 
// 130   8/13/13 10:24a Hui.shao
// 
// 129   8/09/13 5:29p Li.huang
// 
// 128   8/08/13 4:05p Hongquan.zhang
// 
// 127   8/07/13 4:59p Hongquan.zhang
// add new configuration for cachelayer
// 
// 126   8/07/13 9:53a Hui.shao
// changed some log printing
// 
// 125   8/05/13 5:32p Hongquan.zhang
// 
// 124   7/17/13 10:25a Hongquan.zhang
// 
// 123   7/16/13 5:11p Hui.shao
// query home container for space domain if not yet
// 
// 122   7/16/13 3:38p Hongquan.zhang
// support write back cache
// 
// 121   7/12/13 5:19p Hongquan.zhang
// tmp check in, DO NOT release it out
// 
// 120   7/12/13 9:44a Hui.shao
// for HLSContent
// 
// 119   7/11/13 2:33p Li.huang
// 
// 118   7/11/13 2:13p Hui.shao
// 
// 117   7/03/13 7:23p Hui.shao
// 
// 116   6/25/13 2:40p Hongquan.zhang
// fix gcc compatible issue
// 
// 115   6/20/13 9:45a Hongquan.zhang
// fix compiling error in CentOS6.3
// 
// 114   6/19/13 5:15p Li.huang
// 
// 113   6/19/13 4:04p Li.huang
// 
// 112   6/19/13 3:36p Li.huang
// add username to login path
// 
// 111   6/13/13 6:09p Hui.shao
// flags
// 
// 110   6/13/13 4:41p Hui.shao
// Aqua- specify object/container version
// 
// 109   6/07/13 4:10p Li.huang
// 
// 108   6/06/13 11:39a Li.huang
// 
// 107   6/06/13 9:55a Li.huang
// 
// 106   6/04/13 2:34p Ketao.zhang
// 
// 105   6/03/13 7:43p Hui.shao
// 
// 104   6/03/13 7:36p Hui.shao
// ChildReader dispatch to searchByParent() and readByEnumeration() 
// 
// 103   5/31/13 3:37p Hongquan.zhang
// 
// 102   5/31/13 10:07a Build
// 
// 101   5/30/13 7:09p Hui.shao
// 
// 100   5/30/13 6:36p Hui.shao
// calling the /cdmi_search/? for the children
// 
// 99    5/30/13 5:55p Hongquan.zhang
// 
// 98    5/30/13 5:36p Hui.shao
// 
// 97    5/30/13 11:53a Hui.shao
// cleaned the old code replaced by callRemote()
// 
// 96    5/30/13 11:45a Hui.shao
// sunk fileinfo cache into CdmiFuseOps
// 
// 95    5/23/13 4:54p Hui.shao
// enh#17961 to read the space usage from aqua domain
// 
// 94    5/22/13 11:33a Hongquan.zhang
// invalidate cache content if the same file was created again ( include
// rename )
// 
// 93    5/17/13 12:29p Hongquan.zhang
// support read cache
// 
// 92    5/15/13 3:30p Li.huang
// 
// 91    5/13/13 5:14p Li.huang
// fix bug  18095
// 
// 90    5/13/13 4:25p Hui.shao
// 
// 88    5/13/13 9:51a Hui.shao
// 
// 87    5/12/13 1:44p Li.huang
// fix blue screen
// 
// 86    5/11/13 3:53p Li.huang
// 
// 84    5/10/13 3:58p Li.huang
// 
// 83    5/10/13 1:53p Hui.shao
// 
// 82    5/10/13 12:11p Hui.shao
// log message to show trystage
// 
// 81    5/10/13 11:29a Li.huang
// 
// 80    5/09/13 6:25p Hui.shao
// adjustments by execising cdmi_UpdateDataObject() and
// cdmi_ReadDataObject()
// 
// 79    5/09/13 5:34p Hui.shao
// merge generateURL into assembleURL
// 
// 78    5/09/13 5:01p Hui.shao
// x-aqua-redirect-ip and x-aqua-redirect-tag
// 
// 77    5/09/13 4:35p Hui.shao
// 
// 76    5/09/13 3:46p Li.huang
// 
// 75    5/09/13 3:18p Hui.shao
// drafted retry loop
// 
// 74    5/08/13 5:22p Li.huang
// 
// 73    5/08/13 10:33a Li.huang
// 
// 72    5/07/13 4:45p Li.huang
// 
// 71    5/07/13 2:53p Li.huang
// 
// 70    5/07/13 11:45a Li.huang
// 
// 69    5/07/13 11:24a Li.huang
// 
// 68    5/06/13 5:13p Li.huang
// 
// 67    5/06/13 4:42p Li.huang
// get auqa basic conifg
// 
// 66    5/06/13 2:33p Hui.shao
// draft the enh per Aqua configuration and domainURI
// 
// 65    4/24/13 1:31p Li.huang
// 
// 64    4/19/13 9:25a Li.huang
// 
// 63    4/11/13 4:18p Li.huang
// 
// 62    4/11/13 3:00p Li.huang
// add S3 
// 
// 61    4/09/13 10:32a Hui.shao
// draft aqua auth
// 
// 60    4/09/13 10:02a Hui.shao
// draft generateSignature()
// 
// 59    3/15/13 4:13p Li.huang
// 
// 58    2/28/13 2:39p Hongquan.zhang
// 
// 57    2/25/13 4:11p Hui.shao
// separated rootUrl and homeContainer (a sub container under rootUrl)
// 
// 56    2/25/13 11:46a Li.huang
// 
// 55    2/22/13 3:10p Li.huang
// remove url's last letter if equal '='
// 
// 54    2/22/13 2:42p Hui.shao
// Agree to take x-aqua-file-truncate as the unified method instead of
// Content-Range + Content-Length
// 
// 52    2/21/13 4:15p Li.huang
// 
// 51    2/21/13 3:54p Li.huang
// 
// 50    2/21/13 3:29p Hui.shao
// 
// 49    2/21/13 11:29a Hongquan.zhang
// 
// 48    2/21/13 10:57a Hui.shao
// 
// 47    2/19/13 3:57p Li.huang
// 
// 46    2/08/13 3:03p Hui.shao
// 
// 45    2/08/13 10:15a Hui.shao
// 
// 44    2/07/13 5:33p Hui.shao
// reviewed logging
// 
// 43    1/30/13 12:17p Li.huang
// 
// 42    1/29/13 5:23p Hongquan.zhang
// 
// 41    1/28/13 5:38p Hui.shao
// 
// 40    1/28/13 4:28p Li.huang
// 
// 39    1/28/13 3:47p Li.huang
// 
// 38    1/28/13 3:21p Hui.shao
// Time_t
// 
// 37    1/28/13 1:58p Li.huang
// 
// 36    1/28/13 12:48p Li.huang
// 
// 35    1/25/13 4:57p Hongquan.zhang
// 
// 33    1/24/13 4:00p Li.huang
// 
// 32    1/24/13 10:42a Hui.shao
// 
// 31    1/23/13 5:21p Li.huang
// 
// 30    1/23/13 2:20p Li.huang
// 
// 29    1/23/13 10:14a Hui.shao
// wrapperd fstat into fileinfo
// 
// 28    1/18/13 10:28a Li.huang
// 
// 27    1/16/13 4:45p Li.huang
// 
// 26    1/16/13 10:17a Li.huang
// 
// 25    1/14/13 5:03p Li.huang
// 
// 24    1/10/13 2:55p Li.huang
// 
// 23    1/10/13 11:16a Hui.shao
// 
// 22    1/09/13 2:58p Li.huang
// add cdmirc code
// 
// 21    1/08/13 9:24a Li.huang
// 
// 20    1/07/13 10:53p Hui.shao
// 
// 19    1/07/13 8:35p Hui.shao
// 
// 18    1/07/13 5:31p Hui.shao
// 
// 17    1/07/13 5:25p Li.huang
// 
// 16    1/06/13 5:14p Li.huang
// 
// 15    1/06/13 2:11p Li.huang
// 
// 14    1/06/13 11:02a Li.huang
// 
// 13    1/06/13 10:49a Hui.shao
// removed _mountPoint
// 
// 12    1/06/13 10:44a Li.huang
// 
// 11    1/05/13 3:07p Li.huang
// 
// 10    1/04/13 4:07p Hui.shao
// fstat <-> metadata
// 
// 9     1/04/13 3:45p Li.huang
// 
// 8     1/04/13 3:08p Hui.shao
// 
// 7     12/27/12 3:56p Hui.shao
// 
// 6     12/27/12 3:36p Hui.shao
// 
// 5     12/27/12 3:21p Hui.shao
// 
// 4     12/27/12 11:23a Hui.shao
// listed the cdmi apis
// 
// 3     12/26/12 7:59p Hui.shao
// 
// 2     12/26/12 2:27p Hui.shao
// 
// 1     12/25/12 12:17p Hui.shao
// 
// 2     12/24/12 6:51p Hui.shao
// 
// 1     12/24/12 5:08p Hui.shao
// created
// ===========================================================================

#include <boost/functional/hash.hpp>

#define _CUSTOM_TYPE_DEFINED_
#include "CdmiFuseOps.h"
#include "TimeUtil.h"
#include "urlstr.h"
#include "SystemUtils.h"
#include "CryptoAlgm.h"
#include <algorithm>


#define JSON_HAS(OBJ, CHILDNAME) (OBJ.isObject() && OBJ.isMember(CHILDNAME)) // for jsoncpp bug that didn't test type in isMember()
#define OMITTED_SUFFIX ">>omitted"
#define OMITTED_LENGTH (1600)

// -----------------------------
// class ChildReader
// -----------------------------
ChildReader::ChildReader(CdmiFuseOps& fuse, int Id, const std::string& parentId, const std::string& parentName, const CdmiFuseOps::StrList& children2read, const std::string& txnId, int rangeStart, int rangeEnd, const char* acceptType)
: ThreadRequest(fuse._slowThrdPool), _Id(Id), _fuse(fuse), _bQuit(false), _txnId(txnId), _children2read(children2read),
  _parentId(parentId), _parentName(parentName)
{
	char buf[100], *p=buf;
	if (rangeStart <0)
		rangeStart = 0;

	p += snprintf(p, buf + sizeof(buf) -p-2, "cdmi_search/?parentID=%s&range=%d-", _parentId.c_str(), rangeStart); 
	if (rangeEnd >0)
	{
		if (rangeEnd <rangeStart)
			rangeEnd = rangeStart;
		p += snprintf(p, buf + sizeof(buf) -p-2, "%d", rangeEnd);
	}
	_uri = buf;

	_acceptType = acceptType ? acceptType : CDMI_NODE_TYPE;

	if (!_parentName.empty() && LOGIC_FNSEPC != _parentName[_parentName.length()-1])
		_parentName += LOGIC_FNSEPS;
}

int ChildReader::run(void)
{
	if (!_parentId.empty())
		return searchByParent();

	return readByEnumeration();
}

int ChildReader::searchByParent(void)
{
	int c =0;
	std::string finalURL, strStatus, strResponse, reqBody; 
	CdmiFuseOps::Properties reqHeaders, respHeaders;
	uint buflen = 0;

	_fuse._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ChildReader, "searchByParent() reader[%d@%s] reading children uri[%s] for txn[%s]"), 
		_Id, _parentName.c_str(), _uri.c_str(), _txnId.c_str());

	int64 stampStart = ZQ::common::now();
	MAPSET(CdmiFuseOps::Properties, reqHeaders, "Accept", _acceptType);
	int cdmiRetCode = _fuse.callRemote("ChildReader", _uri, finalURL, strStatus, CdmiFuseOps::CDMI_FILE, false, reqHeaders, reqBody, NULL, 0,
		respHeaders, strResponse, buflen, NULL,	ZQ::common::CURLClient::HTTP_GET, _fuse._flags>>16);

	if (CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	char hint[512]= "";
	int  cdmiLatency = (int)(ZQ::common::now() - stampStart);
	snprintf(hint,  sizeof(hint) -1, CLOGFMT(ChildReader, "searchByParent() reader[%d@%s] url[%s]: %s(%d)<=[%s]took %dms"),
		_Id, _parentName.c_str(), finalURL.c_str(), CdmiFuseOps::cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), cdmiLatency);
	_fuse._log.hexDump(ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), hint, 0 == (CdmiFuseOps::flgHexDump &_fuse._flags));

   //1.2 parser cdmi response body
	Json::Value result;
	try
	{	
		Json::Reader reader;
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_fuse._log(ZQ::common::Log::L_ERROR, CLOGFMT(ChildReader, "searchByParent() reader[%d@%s] uri[%s] failed to parse response len[%d]: %s"), 
				_Id, _parentName.c_str(), _uri.c_str(), strResponse.length(), reader.getFormatedErrorMessages().c_str());
			cdmiRetCode = CdmiFuseOps::cdmirc_RequestFailed;
		}
	}
	catch(std::exception& ex)
	{
		_fuse._log(ZQ::common::Log::L_ERROR, CLOGFMT(ChildReader, "searchByParent() reader[%d@%s] uri[%s] failed to parse response caught exception[%s]"),
			_Id, _parentName.c_str(), _uri.c_str(), ex.what());
		cdmiRetCode = CdmiFuseOps::cdmirc_RequestFailed;
	}
	catch (...)
	{
		_fuse._log(ZQ::common::Log::L_ERROR, CLOGFMT(ChildReader, "searchByParent() reader[%d@%s] uri[%s] failed to parse response caught unknown exception[%d]"),
			_Id, _parentName.c_str(), _uri.c_str(), SYS::getLastErr());
		cdmiRetCode = CdmiFuseOps::cdmirc_RequestFailed;
	}

	if ( !JSON_HAS(result, "objects"))
		return -1;

	Json::FastWriter writer;
	size_t cSubDirs =0;

	Json::Value& children = result["objects"];
	for (Json::Value::iterator it = children.begin(); !_bQuit && it != children.end(); it++)
	{
		Json::Value& child = *it;
		if( !child.isObject() )
			continue;//ignore those bad object member

		CdmiFuseOps::FileInfo fi;

		std::string name;
		if (JSON_HAS(child, "objectName"))
			name = child["objectName"].asString();

		if (_parentName.empty() && JSON_HAS(child, "parentName"))
			_parentName = child["parentName"].asString();

		bool bDir = false;
		if (JSON_HAS(child, "objectType"))
		{
			if (std::string::npos != child["objectType"].asString().find("container"))
				bDir = true;
		}

		CdmiFuseOps::FileInfo_reset(fi);
		fi.revision = -1;
		fi.stampInfoKnown = ZQ::common::now();
		if (JSON_HAS(child, "version"))
		{
			try {
				fi.revision = child["version"].asInt();
			}
			catch(...) {}
		}

		if (!JSON_HAS(child, "metadata") || !_fuse.dataObjMetadataToFstat(name, child["metadata"], fi))
		{
			std::string childstr = writer.write(child);
			_fuse._log(ZQ::common::Log::L_ERROR, CLOGFMT(ChildReader, "searchByParent() reader[%d@%s] uri[%s] of txn[%s] got illegal child: %s"),
				_Id, _parentName.c_str(), _uri.c_str(), _txnId.c_str(), childstr.c_str());
			name = "";
		}

		if (bDir)
		{
			fi.filestat.st_mode |= _S_IFDIR;
			if (!name.empty() && LOGIC_FNSEPC != name[name.length()-1])
				name += LOGIC_FNSEPS;
		}

		if (_bQuit || name.empty())
			break;

		_fuse.OnChildInfo(_parentName + name, fi, _txnId);
		_fuse.cacheFileInfo(_parentName + name, fi);
		c++;
		if (bDir)
			cSubDirs++;
	}
	
	_fuse.stampChildren(_parentName);
	_fuse._log((c < (_children2read.size()*3/4)) ? ZQ::common::Log::L_WARNING : ZQ::common::Log::L_INFO, CLOGFMT(ChildReader, "searchByParent() reader[%d@%s] has read %d of %d children for txn[%s], subdirs[%d], took %d/%dmsec"),
		_Id, _parentName.c_str(), c, (int)_children2read.size(), _txnId.c_str(), cSubDirs, cdmiLatency, (int)(ZQ::common::now() - stampStart));
	return 0;
}

int ChildReader::readByEnumeration(void)
{
	_fuse._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ChildReader, "readByEnumeration() reader[%d@%s] reading %d children for txn[%s]"),
		_Id, _parentName.c_str(), _children2read.size(), _txnId.c_str());
	int c =0;
	// step 2. get the file attrs
	for (size_t i=0; !_bQuit && i < _children2read.size(); i++)
	{
		try {
			std::string& pathname = _children2read[i];
			CdmiFuseOps::FileInfo fi;
#ifdef ZQ_OS_MSWIN
			CdmiFuseOps::CdmiRetCode cdmirc = _fuse.getFileInfo(pathname, fi, false);
			if (_bQuit || CdmiRet_FAIL(cdmirc))
				continue;
#endif//ZQ_OS_MSWIN
			_fuse.OnChildInfo(pathname, fi, _txnId);
			c++;
		}
		catch(...) {}
	}

	_fuse.stampChildren(_parentName);
	_fuse._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ChildReader, "readByEnumeration() reader[%d@%s] read %d of %d children for txn[%s]"),
		_Id, _parentName.c_str(), c, _children2read.size(), _txnId.c_str());
	return 0;
}

void ChildReader::final(int retcode, bool bCancelled)
{
	try {
		bCancelled = bCancelled || _bQuit;
		_fuse._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ChildReader, "reader[%d@%s] for txn[%s] cancelled[%c]"), 
			_Id, _parentName.c_str(), _txnId.c_str(), bCancelled?'T':'F');
		_fuse.OnChildReaderStopped(_Id, bCancelled, _txnId);
	}
	catch(...) {}

	delete this;
}

// -----------------------------
// class CdmiFuseOps
// -----------------------------
static CdmiFuseOps::ServerSideParams _serverSideParams;
static ZQ::common::Mutex _lkLogin;

void CdmiFuseOps::getServerSideParams(ServerSideParams& ssp) const
{
	ZQ::common::MutexGuard g(_lkLogin);
	ssp = _serverSideParams;
}

CdmiFuseOps::CdmiFuseOps(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootUrl, const std::string& userDomain, const std::string& homeContainer, const uint32 flags, const FuseOpsConf& conf, const std::string& bindIp, const uint retryInterval)
	:_rootOk(false), _log(log), _rootUrl(rootUrl), _userDomain(userDomain), _homeContainer(homeContainer), _thrdPool(thrdPool), _connectTimeout(5000), _flags(flags), _operationTimeout(10000), _retryTimeout(10000),
	_locationCache(2000), _fileInfos(conf.attrCache_size), _fuseOpsConf(conf), _retryInterval(retryInterval), _pCheckHealthyTrd(NULL), _idxServer(0), _slowThrdPool(5), _mdOwnerInEmailFmt(false),
	_stampDomainReadStart(0), _stampDomainAsOf(0), _domainFreeBytes(0), _domainTotalBytes(0), _bindIp(bindIp)
{
	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuse, "CdmiFuseOps bindIp[%s]"), bindIp.c_str());

	_pCheckHealthyTrd = new CheckServerHealthy(*this);			//fdj
	if(_pCheckHealthyTrd)										//fdj
		_pCheckHealthyTrd->start();								//fdj
	_secretKeyId = "";
	_accessKeyId = "";

	if(!_rootUrl.empty() && _rootUrl[_rootUrl.length() -1] != '/')
		_rootUrl += "/";

	ZQ::common::URLStr strUrl(_rootUrl.c_str());
	_password = strUrl.getPwd();
	_username = strUrl.getUserName();
	
	// cut off the leading '/'
	size_t startpos = _homeContainer.find_first_not_of("/");
	if (std::string::npos != startpos)
		_homeContainer = _homeContainer.substr(startpos);

	// make sure there is an ending '/'
	if(!_homeContainer.empty() && _homeContainer[_homeContainer.length() -1] != '/')
		_homeContainer += "/";

	CacheLayer::DataTankConf& tankConf = _fuseOpsConf.tankConf;
	tankConf.partitionCount = MIN( 100, MAX( tankConf.partitionCount, 1));
	tankConf.readBlockCount /= tankConf.partitionCount;
	tankConf.writeBlockCount /= tankConf.partitionCount;
	tankConf.writeBufferCountOfYield = tankConf.writeBufferCountOfYield * tankConf.writeBlockCount / 100;

	if( _fuseOpsConf.enableCache ){
		mWriteBufferPool = new ZQ::common::NativeThreadPool(tankConf.flushThreadPoolSize);
		for( size_t i = 0 ; i <tankConf.partitionCount; i ++ ) { 
			CacheTank* t = new CacheTank( thrdPool, *mWriteBufferPool, log, *this, _fuseOpsConf.tankConf );
			t->createTank();
			_cacheTanks.push_back(t);
		}
	}
}

CdmiFuseOps::~CdmiFuseOps()
{
	if(_pCheckHealthyTrd)
	{
		_pCheckHealthyTrd->quit();

		delete _pCheckHealthyTrd;	
		_pCheckHealthyTrd = NULL;
	}
	//fdj
	if( _fuseOpsConf.enableCache ) {
		std::vector<CacheTank*>::iterator it = _cacheTanks.begin();
		while( it != _cacheTanks.end() ) {
			(*it)->destroyTank();
			delete *it;
			it++;
		}
		_cacheTanks.clear();
		mWriteBufferPool->stop();
		delete mWriteBufferPool;
	}

	ZQ::common::MutexGuard g(_lkClients);
	while (!_clients.empty())
		_clients.pop_back();
}

void CdmiFuseOps::setAccount(const std::string& serverLogin, const std::string& userDomain, const std::string& password, const std::string& localRunAs)
{
	_username = serverLogin;
	_password = password;
	_userDomain = userDomain;
	_localRunAs = localRunAs;
}

void  CdmiFuseOps::setTimeout(uint connectTimeout, uint operationTimeout, uint retryTimeout)
{	
	_connectTimeout = connectTimeout;
	_operationTimeout = operationTimeout;
	_retryTimeout = retryTimeout;

	if (_operationTimeout < _connectTimeout)
		_operationTimeout = _connectTimeout;

	if (_retryTimeout <= 0)
		_retryTimeout = _operationTimeout*2;
	else if (_retryTimeout < _operationTimeout)
		_retryTimeout = _operationTimeout;
}

std::string CdmiFuseOps::pathToUri(const std::string& pathname)
{
	// cut off the leading slashes
	std::string uri;
	size_t startpos = pathname.find_first_not_of(FNSEPS LOGIC_FNSEPS);
	if (std::string::npos != startpos)
		uri += pathname.substr(startpos);
	
	if (FNSEPC != LOGIC_FNSEPC && !uri.empty())
		std::replace(uri.begin(), uri.end(), FNSEPC, LOGIC_FNSEPC);

	// see if it is necessary to perform URL encoding for some parameters/fields
	std::string olduri = uri; uri="";
	char buf[400];
	for (size_t pos = olduri.find_first_of(LOGIC_FNSEPS "?=&;"); 
		std::string::npos != pos; 
		olduri = olduri.substr(pos+1), pos = olduri.find_first_of(LOGIC_FNSEPS "?=&;"))
	{
		std::string token = olduri.substr(0, pos);
		ZQ::common::URLStr::encode(token.c_str(), buf, sizeof(buf)-2);
		uri +=buf; uri += olduri[pos];
	}

	if (!olduri.empty())
	{
		ZQ::common::URLStr::encode(olduri.c_str(), buf, sizeof(buf)-2);
		uri +=buf;
	}

	uri = _homeContainer + uri;
	return uri;
}

int CdmiFuseOps::parseDataObject(const std::string& path, const Json::Value& jsonDataObj, in out FileInfo& fi, const char* logHint)
{
	FileInfo_reset(fi);
	fi.revision = -1;
	fi.stampInfoKnown = ZQ::common::now();
	if (JSON_HAS(jsonDataObj, "version"))
	{
		try {
			fi.revision = jsonDataObj["version"].asInt();
		}
		catch(...) {}
	}

	if (!JSON_HAS(jsonDataObj, "metadata"))
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuse, "%s no metadata received"), logHint);
		return -1;
	}

	if (!dataObjMetadataToFstat(path, jsonDataObj["metadata"], fi))
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuse, "%s failed to convert fstat"), logHint);
		return -2;
	}

	return 0;
}

#define ACEFLAG_Declare(_FLAGNAME, _FLAGSTR)     { CdmiFuseOps::_FLAGNAME, #_FLAGSTR }
#define ACEMask_Declare(_FLAGSTR)                ACEFLAG_Declare(CDMI_ACE_##_FLAGSTR, _FLAGSTR)

// acetypes
const CdmiFuseOps::FlagID CdmiFuseOps::ACE_FlagTypes[] = {
	ACEFLAG_Declare(CDMI_ACE_ACCESS_ALLOWED_TYPE, ALLOWED),
	ACEFLAG_Declare(CDMI_ACE_ACCESS_DENIED_TYPE,  DENIED),
	ACEFLAG_Declare(CDMI_ACE_SYSTEM_AUDIT_TYPE,   AUDIT),
	{-1, NULL} };

// aceflags
const CdmiFuseOps::FlagID CdmiFuseOps::ACE_FlagFlags[] = {
	ACEFLAG_Declare(CDMI_ACE_OBJECT_INHERIT_ACE, ALLOWED),
	ACEFLAG_Declare(CDMI_ACE_CONTAINER_INHERIT_ACE,  DENIED),
	ACEFLAG_Declare(CDMI_ACE_NO_PROPAGATE_INHERIT_ACE,   AUDIT),
	ACEFLAG_Declare(CDMI_ACE_INHERIT_ONLY_ACE,   AUDIT),
	{-1, NULL} };

const CdmiFuseOps::FlagID CdmiFuseOps::ACE_FlagMasks[] = {
	ACEMask_Declare(READ_OBJECT),
	ACEMask_Declare(LIST_CONTAINER),
	ACEMask_Declare(WRITE_OBJECT),
	ACEMask_Declare(ADD_OBJECT),
	ACEMask_Declare(APPEND_DATA),
	ACEMask_Declare(ADD_SUBCONTAINER),
	ACEMask_Declare(READ_METADATA),
	ACEMask_Declare(WRITE_METADATA),
	ACEMask_Declare(EXECUTE),
	ACEMask_Declare(DELETE_OBJECT),
	ACEMask_Declare(DELETE_SUBCONTAINER),
	ACEMask_Declare(READ_ATTRIBUTES),
	ACEMask_Declare(WRITE_ATTRIBUTES),
	ACEMask_Declare(WRITE_RETENTION),
	ACEMask_Declare(WRITE_RETENTION_HOLD),
	ACEMask_Declare(DELETE),
	ACEMask_Declare(READ_ACL),
	ACEMask_Declare(WRITE_ACL),
	ACEMask_Declare(WRITE_OWNER),
	ACEMask_Declare(SYNCHRONIZE),
	//
	ACEMask_Declare(ALL_PERMS),
	ACEMask_Declare(READ_ALL),
	ACEMask_Declare(RW_ALL),
	{-1, NULL} };

uint32 CdmiFuseOps::readFlags(const Json::Value& value, const CdmiFuseOps::FlagID flagIds[], uint32 defaultVal)
{
	uint32 ret =0;
	std::string str = value.asString();
	size_t pos = str.find("0x");
	if (std::string::npos == pos)
		pos = str.find("0X");

	if (std::string::npos != pos)
	{
		str = str.substr(pos);
		sscanf(str.c_str(), "%x", &ret);
	}
	else ret = atoi(str.c_str());

	if (str.length() <=0)
		return defaultVal;

	if (0 != ret)
		return ret;

	// convert the string flags into numeric flags
	for (int i=0; flagIds[i].name; i++)
	{
		if (std::string::npos != str.find(flagIds[i].name))
			ret |= flagIds[i].flag;
	}
	
	return ret;
}

#ifdef ZQ_OS_MSWIN
#define TIME_DIFF 11644473600000
CdmiFuseOps::Time_t CdmiFuseOps::time2time_t(int64 t)
{
	return (Time_t) ((t - TIME_DIFF)/1000);
}

int64 CdmiFuseOps::time_t2time(CdmiFuseOps::Time_t t64)
{
	return ((int64) t64)*1000 +TIME_DIFF;
}
#elif defined ZQ_OS_LINUX
time_t iso8601toTimet( const std::string& timestr )
{//2013-01-28T16:52:06
	struct tm t;
	memset(&t,0,sizeof(t));
	if( strptime(timestr.c_str(), "%Y-%m-%dT%H:%M:%S",&t) == NULL )
		return 0;
	return timegm(&t);
}

time_t getTimeFromMetadata( const Json::Value& metadata, const std::string& key )
{
	if( !JSON_HAS(metadata, key))
		return 0;
	return iso8601toTimet(metadata[key].asString());
}

int64 getInt64FromMetadata( const Json::Value& metadata, const std::string& key )
{
	if( !JSON_HAS(metadata, key))
		return 0;
	const Json::Value& v = metadata[key];
	if( !v.isString())
		return 0;
	long long retValue = 0;
	sscanf(v.asString().c_str(), "%lld", &retValue);
	return retValue;
}
#endif//ZQ_OS

std::vector<std::string> strSplit(std::string str,std::string pattern)  
{  
	std::string::size_type pos;  
	std::vector<std::string> result;  
	str += pattern; 
	int size = str.size();  
	for(int i = 0; i< size; i++)  
	{  
		pos = str.find(pattern,i);  
		if(pos < size  && i != pos)  
		{  
			std::string s = str.substr(i,pos - i);  
			result.push_back(s);  
			i = pos+ pattern.size() - 1;  
		}  
	}  
	return result;  
} 

std::string CdmiFuseOps::userString()
{
	if (!_mdOwnerInEmailFmt)
		return _username;

	std::string username = _username;
	std::vector<std::string> result = strSplit(_userDomain, "/");
	if(result.size() > 1)
	{
		username +="@";
		for(int i = result.size() -1; i > 0; i--)
		{
			username += result[i];
			if( i != 1)
				username += ".";
		}
	}

	return username;
}

bool CdmiFuseOps::dataObjMetadataToFstat(const std::string& filename, const Json::Value& metadata, CdmiFuseOps::FileInfo& fileInfo)
{
	std::string tmpstr;
	bool isOwner = false;

	if (!metadata.isObject())
		return false;

	// detemin if the current user is the owner
	if (JSON_HAS(metadata, "cdmi_owner") && metadata["cdmi_owner"].isString())
	{
		fileInfo.owner = metadata["cdmi_owner"].asString();
		std::string username = _username;

		if (std::string::npos != fileInfo.owner.find('@'))
		{
			_mdOwnerInEmailFmt = true;
			username = userString();
		}

		// _log(ZQ::common::Log::L_DEBUG, CLOGFMT(ChildReader, "dataObjMetadataToFstat() fileowner[%s] username[%s] userDomain[%s]"), fileInfo.owner.c_str(), username.c_str(), _userDomain.c_str());

		if (0 == username.compare(fileInfo.owner))
			isOwner = true;
	}

#ifdef ZQ_OS_MSWIN
	//	COPY_METADATA_VAL(fileInfo.filestat.st_size, metadata, cdmi_size, Int);
	if (JSON_HAS(metadata, "cdmi_size"))
	{
		if (metadata["cdmi_size"].isIntegral())
			fileInfo.filestat.st_size = metadata["cdmi_size"].asInt64();
		else
			fileInfo.filestat.st_size = _atoi64(metadata["cdmi_size"].asString().c_str());
	}
	// COPY_METADATA_VAL(fileInfo.filestat.st_uid,  metadata, cdmi_owner, Int);

	COPY_METADATA_TIMET(fileInfo.filestat.st_ctime, metadata, cdmi_ctime);
	COPY_METADATA_TIMET(fileInfo.filestat.st_atime, metadata, cdmi_atime);
	COPY_METADATA_TIMET(fileInfo.filestat.st_mtime, metadata, cdmi_mtime);
#elif defined ZQ_OS_LINUX
	fileInfo.filestat.st_size	= (size_t)getInt64FromMetadata(metadata, "cdmi_size");
	fileInfo.filestat.st_ctime	= getTimeFromMetadata( metadata, "cdmi_ctime");
	fileInfo.filestat.st_atime	= getTimeFromMetadata( metadata, "cdmi_atime");
	fileInfo.filestat.st_mtime	= getTimeFromMetadata( metadata, "cdmi_mtime");
#endif//ZQ_OS

	std::string metaDataAclKey = "";
	if(JSON_HAS(metadata, "cdmi_macl")) // about 16.1 Access Control
		metaDataAclKey = "cdmi_macl";
	else if(JSON_HAS(metadata, "cdmi_acl")) // about 16.1 Access Control
		metaDataAclKey = "cdmi_acl";

	if(!metaDataAclKey.empty())
	{
		// "cdmi_acl" : [
		//      {
		//        "acetype" : "0x00",
		//        "identifier" : "EVERYONE@",
		//        "aceflags" : "0x00",
		//        "acemask" : "0x00020089"
		// }]

		const Json::Value& metadataACL = metadata[metaDataAclKey];
		std::string permstr;
		for (Json::Value::const_iterator it = metadataACL.begin(); it != metadataACL.end(); it++)
		{
			if (!(*it).isObject())
				continue;

			// about aceflags
			uint32 aceflags = 0x00; // inherit=1, container_inherit=2, no_propagate_inherit=0x4, inherit_only=0x8

			if (JSON_HAS((*it), "aceflags"))
				aceflags = readFlags((*it)["aceflags"], ACE_FlagFlags, aceflags);

			// about acemask
			uint32 acemask = 0x00;
			if (JSON_HAS((*it), "acemask"))
				acemask = readFlags((*it)["acemask"], ACE_FlagMasks, acemask);

			// about acetype
			uint32 acetype = 0x00; // allow=0x00, deny=0x01, audit=0x02
			if (JSON_HAS((*it), "acetype"))
				acetype = readFlags((*it)["acetype"], ACE_FlagTypes, acetype);

			// convert the (acetype, flag, mask) to fileInfo.filestat.st_mode flags per identifier
			// S_IRUSR(S_IREAD) 00400 
			// S_IWUSR(S_IWRITE)00200 
			// S_IXUSR(S_IEXEC) 00100 
			// S_IRGRP 00040 
			// S_IWGRP 00020 
			// S_IXGRP 00010  
			// S_IROTH 00004
			// S_IWOTH 00002 
			// S_IXOTH 00001 
			int leftMvBits = -1;
			COPY_METADATA_VAL(tmpstr, (*it), identifier, String);
			if (isOwner && 0 == tmpstr.compare("OWNER@"))
				leftMvBits = ST_MODE_BIT_OFFSET_USER;
			else if (0 == tmpstr.compare("AUTHENTICATED@"))
			{
				if (!isOwner)				 
					leftMvBits = ST_MODE_BIT_OFFSET_USER;  // treated for this user
				else
					leftMvBits = ST_MODE_BIT_OFFSET_GROUP;
			}
			else if (0 == tmpstr.compare("GROUP@"))
				leftMvBits = ST_MODE_BIT_OFFSET_GROUP;
			else if (0 == tmpstr.compare("EVERYONE@"))
				leftMvBits = ST_MODE_BIT_OFFSET_OTHER;  // treated as others
//			else if (0 == tmpstr.compare("ANONYMOUS@"))
//				leftMvBits = ST_MODE_BIT_OFFSET_OTHER;  // treated as others
//			else if (0 == tmpstr.compare("ADMINISTRATOR@"))
//				leftMvBits = ST_MODE_BIT_OFFSET_GROUP;  // treated as group users
//			else if (0 == tmpstr.compare("ADMINUSERS@"))
//				leftMvBits = ST_MODE_BIT_OFFSET_GROUP;  // treated as group users

#define PERM777_READ   CDMI_ACE_READ_ALL
#define PERM777_WRITE  (CDMI_ACE_RW_ALL & ~CDMI_ACE_READ_ALL)
#define PERM777_EXEC   CDMI_ACE_EXECUTE

			uint32 perms =0;
			if (acemask & PERM777_READ) // read permission
				perms |= 0x4;
			if (acemask & PERM777_WRITE) // write permission
				perms |= 0x2;
			if (acemask & PERM777_EXEC) // execute permission
				perms |= 0x1;
			if (leftMvBits >=0 && leftMvBits < (sizeof(fileInfo.filestat.st_mode)*8 -4))
			{
				perms <<= leftMvBits;
				if (CDMI_ACE_ACCESS_ALLOWED_TYPE == acetype)
					fileInfo.filestat.st_mode |= perms;
				else if (CDMI_ACE_ACCESS_DENIED_TYPE == acetype)
					fileInfo.filestat.st_mode &= ~perms;

				char buf[200];
				snprintf(buf, sizeof(buf)-2, "%s:acemask[0x%X]=>perm[0%o],", tmpstr.c_str(), acemask, perms);
				permstr += buf;

				if (!isOwner && ST_MODE_BIT_OFFSET_USER == leftMvBits)
				{
					// duplicate user's to group's
					perms >>= (ST_MODE_BIT_OFFSET_USER -ST_MODE_BIT_OFFSET_GROUP);
					if (CDMI_ACE_ACCESS_ALLOWED_TYPE == acetype)
						fileInfo.filestat.st_mode |= perms;
					else if (CDMI_ACE_ACCESS_DENIED_TYPE == acetype)
						fileInfo.filestat.st_mode &= ~perms;

					snprintf(buf, sizeof(buf)-2, "%s:acemask[0x%X]=>perm[0%o]N,", tmpstr.c_str(), acemask, perms);
					permstr += buf;
				}
			}

#pragma message ( __MSGLOC__ "TODO:  update the fileInfo.filestat field per (identifier, acetype, aceflags, acemask)")
		}
		
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "dataObjMetadataToFstat() path[%s] owner[%s] isOwner[%c], perm[0%o] by R[0x%x]W[0x%x]X[0x%x] by %s"), filename.c_str(), fileInfo.owner.c_str(), isOwner?'T':'F', fileInfo.filestat.st_mode, PERM777_READ, PERM777_WRITE, PERM777_EXEC, permstr.c_str());
	}

	return true;
}

bool InConnectedFailed(const CURLcode code)
{
	bool bret = true;
	switch(code)
	{
	case CURLE_COULDNT_RESOLVE_PROXY:
		break;
	case CURLE_COULDNT_RESOLVE_HOST:
		break;
	case CURLE_COULDNT_CONNECT:
		break;
	case CURLE_REMOTE_ACCESS_DENIED:
		break;
	case CURLE_OPERATION_TIMEDOUT:
		break;
	case CURLE_SSL_CONNECT_ERROR:
		break;
	case CURLE_PEER_FAILED_VERIFICATION:
		break;
	default:
		bret = false;
		break;
	}
	return bret;
}
/*
std::string CdmiFuseOps::generateURL(const std::string& uri, bool bContainer, bool includeUsername)
{
	std::string url = getRootURL(_rootUrl) + ((!uri.empty() && uri[0] == '/') ? uri.substr(1): uri);
	ZQ::common::URLStr strURL((char*)url.c_str());
	std::string path = strURL.getPath();
	if (bContainer && !path.empty() && path[path.size() -1] != '/')
	{
		path +="/";
		strURL.setPath((char*)path.c_str());
	}

	if (includeUsername)
	{
		strURL.setUserName(_username.c_str());
		strURL.setPwd(_password.c_str());
	}
	else
	{
		strURL.setUserName("");
		strURL.setPwd("");
	}

	url = strURL.generate();
	if(!url.empty() && url[url.size() -1] == '=')
		url = url.substr(0, url.size() - 1);
	return url;
}
*/
#define CDMIRC_CASE(_RC) case _RC: return #_RC
const char* CdmiFuseOps::cdmiRetStr(int retCode)
{
	switch(retCode)
	{
		CDMIRC_CASE(cdmirc_OK);
		CDMIRC_CASE(cdmirc_Created);
		CDMIRC_CASE(cdmirc_Accepted);
		CDMIRC_CASE(cdmirc_NoContent);
		CDMIRC_CASE(cdmirc_PartialContent);

		CDMIRC_CASE(cdmirc_Found);

		CDMIRC_CASE(cdmirc_BadRequest);
		CDMIRC_CASE(cdmirc_Unauthorized);
		CDMIRC_CASE(cdmirc_Forbidden);
		CDMIRC_CASE(cdmirc_NotFound);
		CDMIRC_CASE(cdmirc_NotAcceptable);
		CDMIRC_CASE(cdmirc_Conflict);
		CDMIRC_CASE(cdmirc_ServerError);
		CDMIRC_CASE(cdmirc_InvalidRange);
		CDMIRC_CASE(cdmirc_ServerUnavailable);

		CDMIRC_CASE(cdmirc_LengthRequired);
		CDMIRC_CASE(cdmirc_UriTooLong);
		CDMIRC_CASE(cdmirc_UnknownMediaType);
		CDMIRC_CASE(cdmirc_NotImplemented);
		CDMIRC_CASE(cdmirc_HttpVersion);

		CDMIRC_CASE(cdmirc_RequestFailed);
		CDMIRC_CASE(cdmirc_RequestTimeout);
		CDMIRC_CASE(cdmirc_AquaLocation);
		CDMIRC_CASE(cdmirc_RetryFailed);

	default: return "Unknown";
	}
}

static std::string nowToAquaData()
{
	return CdmiFuseOps::timeToAquaData(ZQ::common::now());
}

std::string CdmiFuseOps::timeToAquaData(int64 time)
{
	static const int64 stamp1970= ZQ::common::TimeUtil::ISO8601ToTime("1970-01-01T00:00:00Z");
	char buf[64];
	snprintf(buf, sizeof(buf)-2, "%lld", time - stamp1970);
	return buf;
}

bool CdmiFuseOps::addObjMetadataByFstat(Json::Value& metadata, const CdmiFuseOps::FileInfo& fileInfo)
{
	char buf[200];
	bool bOwner = false;

	if (fileInfo.owner.empty())
	{
		metadata["cdmi_owner"] = userString();
		bOwner = true;
	}
	else
	{
		metadata["cdmi_owner"] = fileInfo.owner;
		bOwner = (fileInfo.owner == _username);
	}

	metadata["cdmi_size"]  = (long long)fileInfo.filestat.st_size;


	ZQ::common::TimeUtil::Time2Iso(fileInfo.filestat.st_ctime, buf, sizeof(buf)-2); metadata["cdmi_ctime"] = buf;
	ZQ::common::TimeUtil::Time2Iso(fileInfo.filestat.st_atime, buf, sizeof(buf)-2); metadata["cdmi_atime"] = buf;
	ZQ::common::TimeUtil::Time2Iso(fileInfo.filestat.st_mtime, buf, sizeof(buf)-2); metadata["cdmi_mtime"] = buf;

	if (!JSON_HAS(metadata, "cdmi_acl"))
		metadata.append("cdmi_acl");

	Json::Value aceMD;
	uint32 perms =0;
	aceMD["acetype"] = "0x00";
	aceMD["aceflags"] = "0x00@";

#ifdef ZQ_OS_MSWIN
	perms =0;
	if (S_IREAD & fileInfo.filestat.st_mode)
		perms |= CDMI_ACE_READ_ALL;
	if (S_IWRITE & fileInfo.filestat.st_mode)
		perms |= (CDMI_ACE_RW_ALL & ~CDMI_ACE_READ_ALL);
	if (S_IEXEC & fileInfo.filestat.st_mode)
		perms |= CDMI_ACE_EXECUTE;

	aceMD["acemask"] = perms;
	aceMD["identifier"] = "EVERYONE@";
	metadata["cdmi_acl"].append(aceMD);

	aceMD["identifier"] = "GROUP@";
	metadata["cdmi_acl"].append(aceMD);

	aceMD["identifier"] = bOwner ? "OWNER@": "AUTHENTICATED@";
	metadata["cdmi_acl"].append(aceMD);
#else 
	// non-Windows

	// about others
	aceMD["identifier"] = "EVERYONE@";
	perms =0;
	if (S_IROTH & fileInfo.filestat.st_mode)
		perms |= CDMI_ACE_READ_ALL;
	if (S_IWOTH & fileInfo.filestat.st_mode)
		perms |= (CDMI_ACE_RW_ALL & ~CDMI_ACE_READ_ALL);
	if (S_IXOTH & fileInfo.filestat.st_mode)
		perms |= CDMI_ACE_EXECUTE;

	aceMD["acemask"] = perms;
	metadata["cdmi_acl"].append(aceMD);

	// about group's permissions
	aceMD["identifier"] = "GROUP@";
	perms =0;
	if (S_IRGRP & fileInfo.filestat.st_mode)
		perms |= CDMI_ACE_READ_ALL;
	if (S_IWGRP & fileInfo.filestat.st_mode)
		perms |= (CDMI_ACE_RW_ALL & ~CDMI_ACE_READ_ALL);
	if (S_IXGRP & fileInfo.filestat.st_mode)
		perms |= CDMI_ACE_EXECUTE;

	aceMD["acemask"] = perms;
	metadata["cdmi_acl"].append(aceMD);

	// about user's permissions
	aceMD["identifier"] = "OWNER@";
	perms =0;
	if (S_IRUSR & fileInfo.filestat.st_mode)
		perms |= CDMI_ACE_READ_ALL;
	if (S_IWUSR & fileInfo.filestat.st_mode)
		perms |= (CDMI_ACE_RW_ALL & ~CDMI_ACE_READ_ALL);
	if (S_IXUSR & fileInfo.filestat.st_mode)
		perms |= CDMI_ACE_EXECUTE;

	aceMD["acemask"] = perms;
	metadata["cdmi_acl"].append(aceMD);
#endif // WIN32

#pragma message ( __MSGLOC__ "TODO:  any other metadata of stat to copy")

	return true;
}

CacheTank* CdmiFuseOps::getCacheTank( const std::string& uri ) {
	boost::hash<std::string> hasher;
	size_t idx = hasher(uri) % _cacheTanks.size();
	return _cacheTanks[idx];
}

#define SET_OPTIONAL_STR_PARAM(_JSONROOT, _KEY, _VAL) if (!_VAL.empty()) _JSONROOT[_KEY] = _VAL

CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_CreateDataObject(Json::Value& result, const std::string& uri, const std::string& mimetype, const Properties& metadata, const std::string& value,
															const StrList& valuetransferencoding,
															const std::string& domainURI, const std::string& deserialize, const std::string& serialize,
															const std::string& copy, const std::string& move, const std::string& reference, 
															const std::string& deserializevalue, bool fastClone)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_CreateDataObject[%s]"), uri.c_str());
	
	if(_fuseOpsConf.enableCache )
	{
		CacheTank* cache = getCacheTank(uri);
		cache->validate_writebuffer(uri);
		cache->validate_readcache(uri);
	}

	Json::Value requestParams;
	Json::FastWriter writer;
	Json::Value v;

	// the mimetype
	requestParams["mimetype"] = mimetype.empty() ? DEFAULT_CONTENT_MIMETYPE : mimetype;

	// the metadata
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		v[it->first] = it->second;
	if (v.size() >0)
		requestParams["metadata"] = v;
	v.clear();

	SET_OPTIONAL_STR_PARAM(requestParams, "value", value);

	// valuetransferencoding 
	for (StrList::const_iterator itS= valuetransferencoding.begin(); itS <valuetransferencoding.end(); itS++)
		v.append(*itS);
	if (v.size() >0)
		requestParams["valuetransferencoding"] = v;
	v.clear();

	// the URIs	
	SET_OPTIONAL_STR_PARAM(requestParams, "domainURI",   domainURI);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserialize", deserialize);
	SET_OPTIONAL_STR_PARAM(requestParams, "serialize",   serialize);
	SET_OPTIONAL_STR_PARAM(requestParams, fastClone?"clone":"copy", copy);
	SET_OPTIONAL_STR_PARAM(requestParams, "move",        move);
	SET_OPTIONAL_STR_PARAM(requestParams, "reference",   reference);

	//1.1 sending the request to the server
	result.clear();
	std::string requestBody = writer.write(requestParams);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_CreateDataObject[%s] requestbody [%s]"),uri.c_str(), requestBody.c_str());

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Accept", CDMI_DATAOBJECT_TYPE);
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_DATAOBJECT_TYPE);
	MAPSET(Properties, requestHeaders, "X-CDMI-Specification-Version", CDMI_Version);
	MAPSET(Properties, requestHeaders, "X-CDMI-Partial", "false");
	if (!move.empty())
		MAPSET(Properties, requestHeaders, "x-aqua-move-overwrite", "true");

	Properties resHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("cdmi_CreateDataObject", uri, finalURL, strStatus, CDMI_FILE, false, requestHeaders, requestBody, NULL, 0,
		resHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_PUT,  (_flags>>16) | ZQ::common::CURLClient::sfEnableOutgoingDataCB, fastClone?copy:move);

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "cdmi_CreateDataObject[%s]: %s(%d)<=[%s]took %dms"),
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	// clean the cache for the move-from file
	if (!move.empty())
	{
		uncacheFileInfo(move);
		uncacheLocation(move);
	}

   //1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_CreateDataObject[%s] failed to parse response len[%d]: %s"), uri.c_str(), strResponse.length(), reader.getFormatedErrorMessages().c_str());
			cdmiRetCode =  cdmirc_RequestFailed;
		}
	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_CreateDataObject[%s] failed to parse response caught exception[%s]"),uri.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_CreateDataObject[%s] failed to parse response caught unknown exception[%d]"),uri.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}

	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_CreateDataObject(const std::string& uri, const std::string& contentType, const char* value, uint32 size)
{	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "nonCdmi_CreateDataObject[%s]"), uri.c_str());

	if(_fuseOpsConf.enableCache ) {
		CacheTank* cache = getCacheTank(uri);
		cache->validate_writebuffer(uri);
		cache->validate_readcache(uri);
	}

	std::string strConType = contentType.empty() ? DEFAULT_CONTENT_MIMETYPE : contentType; // DEFAULT_MIMETYPE ";charset=utf-8" : contentType;

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;
	
	char rangeStr[100];
	snprintf(rangeStr, sizeof(rangeStr)-2, "bytes=0-%d", size);
	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Content-Type", strConType);
	MAPSET(Properties, requestHeaders, "Content-Range", rangeStr);

	std::string requestBody;
	Properties resHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("nonCdmi_CreateDataObject", uri, finalURL, strStatus, CDMI_FILE, false, requestHeaders, requestBody, (char*)value, size,
		resHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_PUT,  (_flags >>16) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "nonCdmi_CreateDataObject[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_ReadDataObject(Json::Value& result, const std::string& uri, std::string& location)
{	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_ReadDataObject[%s]"), uri.c_str());
//////////////////////////////////
	std::string finalURL;
	Properties requestHeaders, respHeaders;
	MAPSET(Properties, requestHeaders, "Accept",       CDMI_DATAOBJECT_TYPE); ///Optional
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_DATAOBJECT_TYPE);///Mandatory

	std::string requestBody;
	std::string strResponse, respStatus;
	uint buflen=0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_ReadDataObject", uri, finalURL, respStatus, CDMI_FILE, false,
							 requestHeaders, requestBody, NULL, 0,
							 respHeaders, strResponse, buflen, NULL,
							 ZQ::common::CURLClient::HTTP_GET, _flags>>16);

	if (!CdmiRet_SUCC(cdmiRetCode))
		return (CdmiFuseOps::CdmiRetCode) cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "cdmi_ReadDataObject[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, respStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	if(respHeaders.find("Location") != respHeaders.end())
		location = respHeaders["Location"];

	//1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_ReadDataObject[%s] failed to parse response len[%d]: %s"), finalURL.c_str(), strResponse.length(), reader.getFormatedErrorMessages().c_str());
			return cdmirc_RequestFailed;
		}
	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_ReadDataObject[%s] failed to parse response caught exception[%s]"), finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_ReadDataObject[%s] failed to parse response caught unknown exception[%d]"), finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}

	return cdmiRetCode;
}
	
CacheTank::CacheTank( ZQ::common::NativeThreadPool& readPool, ZQ::common::NativeThreadPool& writePool, ZQ::common::Log& logger, CdmiFuseOps& ops, CacheLayer::DataTankConf& conf)
	:CacheLayer::DataTank(readPool,writePool,logger, conf),mOps(ops){
}
CacheTank::~CacheTank(){
}

int CacheTank::directWrite( const std::string& filename, const char* buf, unsigned long long offset , size_t& size ) 
{
	std::string contentType,location;
	uint len = size;
	CdmiFuseOps::CdmiRetCode rc = mOps.nonCdmi_UpdateDataObject_direct(filename,location, contentType,offset,len,buf);
	size = len;
	return rc;
}

int	CacheTank::directRead( const std::string& filename, char* buf, unsigned long long offset, size_t& size ) 
{
	std::string contentType,location;
	uint len = size;
	CdmiFuseOps::CdmiRetCode rc = mOps.nonCdmi_ReadDataObject_direct(filename,contentType, location,offset,len,buf);
	size = len;
	return rc;
}

int	CacheTank::directRead( const std::string& filename, unsigned long long offset, const std::vector<CacheLayer::DataBuffer>& bufs, size_t& sizeTotal )
{
	std::string contentType,location;
	CdmiFuseOps::CdmiRetCode rc = mOps.nonCdmi_ReadDataObject_direct(filename,contentType, location,offset,bufs, sizeTotal);
	return rc;
}

int	CacheTank::directWrite( const std::string& filename, const std::vector<CacheLayer::DataBuffer>& bufs, size_t& sizeTotal)
{
	std::string contentType,location;
	CdmiFuseOps::CdmiRetCode rc = mOps.nonCdmi_UpdateDataObject_direct( filename, location, contentType, bufs, sizeTotal );
	return rc;
}
bool CacheTank::isSuccess( int err , size_t* size  ) const 
{
	if( err == 0 )
		return true;
	if( CdmiRet_SUCC(err) )
		return true;
	if( err == CdmiFuseOps::cdmirc_InvalidRange )
	{
		if(size) *size = 0;
		return true;
	}
	return false;	
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_ReadDataObject(const std::string& uri, std::string& contentType,
															 std::string& location,
															 uint64 startOffset, in out uint& len, char* recvbuff, bool disableCache)
{	
	if(!_fuseOpsConf.enableCache || disableCache)
	{
		return nonCdmi_ReadDataObject_direct(uri,contentType, location,startOffset,len,recvbuff);
	}

	int r =  getCacheTank(uri)->cacheRead(uri,recvbuff,startOffset,len);
	if( r == 0 )
		return cdmirc_OK;
	else
		return r;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_ReadDataObject_direct(const std::string& uri, std::string& contentType, std::string& location, uint64 startOffset, in out uint& len, char* recvbuff)
{
	std::vector<CacheLayer::DataBuffer> bufs;
	CacheLayer::DataBuffer buf;
	buf.buf = recvbuff;
	buf.size = len;
	size_t sizeTotal = len;
	bufs.push_back(buf);
	CdmiRetCode retCode =  nonCdmi_ReadDataObject_direct(uri,contentType,location,startOffset,bufs,sizeTotal);
	len = sizeTotal;
	return retCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_ReadDataObject_direct(const std::string& uri, std::string& contentType, std::string& location, uint64 startOffset, const std::vector<CacheLayer::DataBuffer>& bufs, size_t& sizeTotal)
{  
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "nonCdmi_ReadDataObject[%s], offset[%lld] len[%u]"), uri.c_str(), startOffset, sizeTotal);

	if (bufs.empty() || sizeTotal <=0)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "nonCdmi_ReadDataObject[%s], illegal recvbuf w/ len[%u]"), uri.c_str(), sizeTotal);
		return cdmirc_RequestFailed;
	}

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	char strRang[100] ="", *p = strRang;
	snprintf(p, sizeof(strRang) -1, "bytes=%llu-", startOffset); p += strlen(p);
	if (sizeTotal >0)
		snprintf(p, strRang + sizeof(strRang) -p -1, "%llu", startOffset+sizeTotal-1);

	MAPSET(Properties, requestHeaders, "Range", strRang);

	std::string requestBody;

	Properties resHeaders;
	std::string strResponse;

	char purpose[200];
	snprintf(purpose, sizeof(purpose) -2, "nonCdmi_ReadDataObject(%s)", strRang);

	ZQ::common::BufferList::Ptr bufReq = new ZQ::common::BufferList();
	ZQ::common::BufferList::Ptr bufBody = new ZQ::common::BufferList(true);
	for (std::vector<CacheLayer::DataBuffer>::const_iterator itBuf = bufs.begin(); itBuf != bufs.end(); itBuf++)
		bufBody->join( (uint8*)itBuf->buf, itBuf->size, 0);

	uint recBufSize = sizeTotal;
	int cdmiRetCode = callRemote(purpose, uri, finalURL, strStatus, CDMI_FILE, false, 
		requestHeaders, bufReq, 
		resHeaders, bufBody,
		ZQ::common::CURLClient::HTTP_GET, _flags>>16);

	if (CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	sizeTotal = bufBody->length();
	int elapsedTime = (int)(ZQ::common::now() - lStartTime);

	// calculate the crc32
	size_t nLeft = sizeTotal;
	ZQ::common::CRC32 crc32;
	for (std::vector<CacheLayer::DataBuffer>::const_iterator itBuf = bufs.begin(); nLeft >0 && itBuf != bufs.end(); itBuf++)
	{
		size_t bytesInBuf = MIN(nLeft, itBuf->size);
		crc32.update(itBuf->buf, bytesInBuf);
		nLeft -= bytesInBuf;
	}

	if (flgDumpMsgBody & _flags)
	{
		char buf[512]= "";
		snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "nonCdmi_ReadDataObject[%s]: "), finalURL.c_str());
		_log.hexDump(ZQ::common::Log::L_DEBUG, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));
	}

	if (resHeaders.find("Location") != resHeaders.end())
		location = resHeaders["Location"];
	if (resHeaders.find("Content-Type") != resHeaders.end())	
		contentType = resHeaders["Content-Type"];

//	if( len > recBufSize)
//		len = recBufSize;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "nonCdmi_ReadDataObject[%s] took %dms: %s(%d)<=[%s] len(%d/%d) requestRange(%s), crc32[0x%08X]"),
		finalURL.c_str(), elapsedTime, cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), sizeTotal, recBufSize, strRang, crc32.get());

    // memcpy(recvbuff, (char*)strResponse.c_str(), len);
	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_UpdateDataObject(out std::string& location, const std::string& uri, const Properties& metadata,
		uint64 startOffset, const std::string& value, bool partial, const StrList& valuetransferencoding,
		const std::string& domainURI, const std::string& deserialize, const std::string& copy, const std::string& deserializevalue)
{
	Json::Value jMetadata;
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		jMetadata[it->first] = it->second;

	return CdmiFuseOps::cdmi_UpdateDataObjectEx(location, uri, jMetadata,
		startOffset, value, -1, partial, valuetransferencoding,
		domainURI, deserialize, copy, deserializevalue);
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_UpdateDataObjectEx(out std::string& location, const std::string& uri, const Json::Value& metadata,
		uint64 startOffset, const std::string& value, int base_version, bool partial, const StrList& valuetransferencoding,
		const std::string& domainURI, const std::string& deserialize, const std::string& copy, const std::string& deserializevalue)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_UpdateDataObjectEx[%s]"), uri.c_str());

	if(_fuseOpsConf.enableCache ) {
		getCacheTank(uri)->validate_readcache(uri);
	}

	Json::Value requestParams;
	Json::FastWriter writer;
	Json::Value v;

	// the mimetype
	requestParams["mimetype"] = DEFAULT_CONTENT_MIMETYPE ;

	// the metadata
	/*
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		v[it->first] = it->second;
	if (v.size() >0)
		requestParams["metadata"] = v;
	v.clear();
	*/
	if (metadata.size() >0)
		requestParams["metadata"] = metadata;

	SET_OPTIONAL_STR_PARAM(requestParams, "value", value);

	// valuetransferencoding 
	for (StrList::const_iterator itS= valuetransferencoding.begin(); itS <valuetransferencoding.end(); itS++)
		v.append(*itS);
	if (v.size() >0)
		requestParams["valuetransferencoding"] = v;
	v.clear();

	// the URIs
	SET_OPTIONAL_STR_PARAM(requestParams, "domainURI",   domainURI);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserialize",      deserialize);
	SET_OPTIONAL_STR_PARAM(requestParams, "copy",             copy);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserializevalue", deserializevalue);

	//1.1 sending the request to the server
	std::string requestBody = writer.write(requestParams);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_UpdateDataObjectEx[%s] requestbody [%s]"),uri.c_str(), requestBody.c_str());

	std::string url;
	Properties requestHeaders, respHeaders;
	MAPSET(Properties, requestHeaders, "Content-Type",   CDMI_DATAOBJECT_TYPE);///Mandatory
	MAPSET(Properties, requestHeaders, "X-CDMI-Partial", partial == true? "true":"false");///Optional

	if (base_version >=0)
	{
		// http://192.168.87.16/mediawiki/index.php/Specify_object/container_version_-_CDMI_-_Function_Design_-_Aqua
		char verstr[10];
		snprintf(verstr, sizeof(verstr), "%d", base_version);
		MAPSET(Properties, requestHeaders, "x-aqua-verify-version", verstr);///Optional
	}

	std::string strResponse, strStatus;
	uint buflen=0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_UpdateDataObjectEx", uri, url, strStatus, CDMI_FILE, false,
							 requestHeaders, requestBody, NULL, 0,
							 respHeaders, strResponse, buflen, NULL,
							 ZQ::common::CURLClient::HTTP_PUT, (_flags >>16) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if (!CdmiRet_SUCC(cdmiRetCode))
		return (CdmiFuseOps::CdmiRetCode) cdmiRetCode;

	if(respHeaders.find("Location") != respHeaders.end())
		location = respHeaders["Location"];

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "cdmi_UpdateDataObjectEx[%s]: %s(%d)<=[%s] took %dms, loc[%s]"), 
		url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime), location.c_str());

	// cdmiClient = NULL;
	return cdmiRetCode;
}

	
CdmiFuseOps::CdmiRetCode 
CdmiFuseOps::nonCdmi_UpdateDataObject(const std::string& uri, out std::string& location, const std::string& contentType, 
									  uint64 startOffset, uint len, const char* buff, int64 objectSize, bool partial,bool disableCache)
{
	if(!_fuseOpsConf.enableCache || objectSize != -1|| disableCache)
	{
		if( objectSize != -1 && _fuseOpsConf.enableCache ) {
			CacheTank* cache = getCacheTank( uri );
			cache->validate_writebuffer(uri);
			cache->validate_readcache(uri);
		}
		return nonCdmi_UpdateDataObject_direct(uri,location, contentType,startOffset,len,buff,objectSize,partial);
	}
	int r =  getCacheTank(uri)->cacheWrite(uri,buff,startOffset,len);
	if( r == 0 )
		return cdmirc_OK;
	else
		return r;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_UpdateDataObject_direct(const std::string& uri, out std::string& location, const std::string& contentType, uint64 startOffset, uint len, const char* buff, int64 objectSize, bool partial)
{
	std::vector<CacheLayer::DataBuffer> bufs;
	CacheLayer::DataBuffer buf;
	buf.buf = const_cast<char*>(buff);
	buf.size = len;
	buf.offset = (unsigned long long)startOffset;
	bufs.push_back(buf);

	return nonCdmi_UpdateDataObject_direct( uri, location, contentType, bufs, len, objectSize, partial);
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_UpdateDataObject_direct(const std::string& uri, out std::string& location, const std::string& contentType, const std::vector<CacheLayer::DataBuffer>& bufs , uint len, int64 objectSize, bool partial ) 
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "nonCdmi_UpdateDataObject[%s]: "), uri.c_str());
    
	if ( bufs.empty() && objectSize < 0  )
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "nonCdmi_UpdateDataObject[%s]: value is null "), uri.c_str());
		return cdmirc_RequestFailed;
	}

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;

	ZQ::common::BufferList::Ptr bufBody= new ZQ::common::BufferList(true);
	ZQ::common::BufferList::Ptr bufResp = new ZQ::common::BufferList();

	Properties requestHeaders;
	std::ostringstream ossRange;
	std::string strRange;

	size_t totalLen = 0;

	if (len <=0)
	{
		if (objectSize >=0)
		{
			char sizestr[30];
			snprintf(sizestr, sizeof(sizestr)-2, "%llu", objectSize);
			MAPSET(Properties, requestHeaders, "x-aqua-file-truncate", sizestr);
		}
	}
	else
	{
		std::vector<CacheLayer::DataBuffer>::const_iterator itBuf = bufs.begin();	
		ossRange << "bytes ";

		do {
			bufBody->join( (uint8*)itBuf->buf, itBuf->size );
			totalLen += itBuf->size;
			itBuf ++;	
		} while(itBuf != bufs.end() );
		ossRange << CacheLayer::dataBuffersToRangeStr(bufs);
		strRange = ossRange.str();
		MAPSET(Properties, requestHeaders, "Content-Range", strRange);
	}
	MAPSET(Properties, requestHeaders, "Content-Type", contentType);
	MAPSET(Properties, requestHeaders, "X-CDM-Partial", partial == true ? "true":"false");

	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;

	uint buflen = 0;
	std::string purpose = std::string("nonCdmi_UpdateDataObject(")+strRange+")";

	int cdmiRetCode = callRemote(purpose.c_str(), uri, finalURL, strStatus, CDMI_FILE, false,
			requestHeaders, bufBody,
			respHeaders, bufResp,
			ZQ::common::CURLClient::HTTP_PUT,  (_flags>>16) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "nonCdmi_UpdateDataObject[%s] bufferCount[%d] totalLen[%u] range[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), (int)bufs.size(), totalLen, strRange.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	if(respHeaders.find("Location") != respHeaders.end())
		location = respHeaders["Location"];

// TODO:	uncacheFileInfo(uri);

	return cdmiRetCode;

}


CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_DeleteDataObject(Json::Value& result, const std::string& uri)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_DeleteDataObject[%s]"), uri.c_str());

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_DATAOBJECT_TYPE);

	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("cdmi_DeleteDataObject", uri, finalURL, strStatus, CDMI_FILE, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_DEL,  (_flags>>16));

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	uncacheFileInfo(uri);

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "cdmi_DeleteDataObject[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_DeleteDataObject(const std::string& uri)
{	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "nonCdmi_DeleteDataObject[%s]"), uri.c_str());

	int64 lStartTime = ZQ::common::now();
	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;

	uint buflen = 0;

	int cdmiRetCode = callRemote("nonCdmi_DeleteDataObject", uri, finalURL, strStatus, CDMI_FILE, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen, NULL,
		ZQ::common::CURLClient::HTTP_DEL, (_flags>>16));

	if(CdmiRet_FAIL(cdmiRetCode))
		return cdmiRetCode;

	uncacheFileInfo(uri);

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "nonCdmi_DeleteDataObject[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_CreateContainer(Json::Value& result, const std::string& uri, const Properties& metadata,
														   const Json::Value& exports,
														   const std::string& domainURI, const std::string& deserialize, 
														   const std::string& copy, const std::string& move, const std::string& reference, 
														   const std::string& deserializevalue)
{
//	std::string url = generateURL(uri, true);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_CreateContainer[%s]"), uri.c_str());
	Json::Value requestParams(Json::objectValue);
	Json::FastWriter writer;
	Json::Value v;

	// the metadata
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		v[it->first] = it->second;
	if (v.size() >0)
		requestParams["metadata"] = v;
	v.clear();

	// exprots
	if (exports.size() >0)
		requestParams["exports"] = exports;

	// the URIs	
	SET_OPTIONAL_STR_PARAM(requestParams, "domainURI",   domainURI);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserialize", deserialize);
	SET_OPTIONAL_STR_PARAM(requestParams, "copy",        copy);
	SET_OPTIONAL_STR_PARAM(requestParams, "move",        move);
	SET_OPTIONAL_STR_PARAM(requestParams, "reference",   reference);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserializevalue",   deserializevalue);

	//1.1 sending the request to the server
	result.clear();
	std::string requestBody = writer.write(requestParams);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_CreateContainer[%s] requestbody [%s]"),uri.c_str(), requestBody.c_str());

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Accept", CDMI_CONTAINER_TYPE);
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_CONTAINER_TYPE);
	if (!move.empty())
		MAPSET(Properties, requestHeaders, "x-aqua-move-overwrite", "true");

	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_CreateContainer", uri, finalURL, strStatus, CDMI_CONTAINER, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_PUT, (_flags>>16) | ZQ::common::CURLClient::sfEnableOutgoingDataCB, move);

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiFuseOps::CdmiRetCode)cdmiRetCode;
	
	// clean the cache for the move-from container
	if (!move.empty())
	{
		uncacheFileInfo(move);
		uncacheLocation(move);
	}

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "cdmi_CreateContainer[%s]: %s(%d)<=[%s] took %dms"),
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	//1.2 parser cdmi response boy
	Json::Reader reader;

	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_CreateContainer[%s] failed to parse response len[%d]: %s"),finalURL.c_str(), strResponse.length(), reader.getFormatedErrorMessages().c_str());
			return cdmirc_RequestFailed;
		}

	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_CreateContainer[%s] failed to parse response caught exception[%s]"),finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_CreateContainer[%s] failed to parse response caught unknown exception[%d]"),finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_CreateContainer(const std::string& uri)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "nonCdmi_CreateContainer[%s]"), uri.c_str());
	
	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Expect", "");
	MAPSET(Properties, requestHeaders, "Transfer-Encoding", "");

	std::string requestBody;

	Properties respHeaders;
	std::string responseTxtBody;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("nonCdmi_CreateContainer", uri, finalURL, strStatus, CDMI_CONTAINER, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, responseTxtBody, buflen,NULL,
		ZQ::common::CURLClient::HTTP_PUT, (_flags>>16) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiFuseOps::CdmiRetCode)cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "nonCdmi_CreateContainer[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	return (CdmiFuseOps::CdmiRetCode) cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_ReadContainer(Json::Value& result, const std::string& uri)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_ReadContainer[%s]"), uri.c_str());

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Accept", CDMI_CONTAINER_TYPE);
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_CONTAINER_TYPE);

	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_ReadContainer", uri, finalURL, strStatus, CDMI_CONTAINER, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_GET, (_flags>>16));

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiFuseOps::CdmiRetCode)cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "cdmi_ReadContainer[%s] %s(%d)<=[%s] took %dms: "), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

//	cdmiClient = NULL;
	//1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_ReadContainer[%s] failed to parse response len[%d]: %s"), finalURL.c_str(), strResponse.length(), reader.getFormatedErrorMessages().c_str());
			return cdmirc_RequestFailed;
		}

	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_ReadContainer[%s] failed to parse response caught exception[%s]"),finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_ReadContainer[%s] failed to parse response caught unknown exception[%d]"),finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}

	if (strResponse.size() > 600 && JSON_HAS(result, "children"))
	{
		std::string childrenlist;
		Json::Value& children = result["children"];
		for (Json::Value::iterator itF = children.begin(); itF != children.end(); itF++)
			childrenlist += (*itF).asString() +",";
		if (childrenlist.length() > OMITTED_LENGTH)
			childrenlist = childrenlist.substr(0, OMITTED_LENGTH) + OMITTED_SUFFIX;
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_ReadContainer[%s] children(%d): %s"), finalURL.c_str(), children.size(), childrenlist.c_str());
	}

	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_UpdateContainer(Json::Value& result, out std::string& location,
														   const std::string& uri, const Properties& metadata,	const Json::Value& exports,
														   const std::string& domainURI, const std::string& deserialize, const std::string& copy,
														   const std::string& snapshot, const std::string& deserializevalue)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_UpdateContainer[%s]"), uri.c_str());
	Json::Value requestParams;
	Json::FastWriter writer;
	Json::Value v;

	// the metadata
	for (Properties::const_iterator it= metadata.begin(); it!=metadata.end(); it++)
		v[it->first] = it->second;
	if (v.size() >0)
		requestParams["metadata"] = v;
	v.clear();

	requestParams["exports"] = exports;
	// the URIs
	SET_OPTIONAL_STR_PARAM(requestParams, "domainURI",		  domainURI);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserialize",      deserialize);
	SET_OPTIONAL_STR_PARAM(requestParams, "copy",             copy);
	SET_OPTIONAL_STR_PARAM(requestParams, "snapshot",         snapshot);
	SET_OPTIONAL_STR_PARAM(requestParams, "deserializevalue", deserializevalue);

	//1.1 sending the request to the server
	result.clear();
	std::string requestBody = writer.write(requestParams);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_UpdateContainer[%s] requestbody [%s]"),uri.c_str(), requestBody.c_str());

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Accept", CDMI_CONTAINER_TYPE);
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_CONTAINER_TYPE);

	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_UpdateContainer", uri, finalURL, strStatus, CDMI_CONTAINER, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_PUT, (_flags>>16) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiFuseOps::CdmiRetCode)cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "cdmi_UpdateContainer[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	if(respHeaders.find("Location") != respHeaders.end())
		location = respHeaders["Location"];

	//1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_UpdateContainer[%s] failed to parse response len[%d] took %dms: %s"),
				finalURL.c_str(), strResponse.length(), (int)(ZQ::common::now() - lStartTime), reader.getFormatedErrorMessages().c_str());
			return cdmirc_RequestFailed;
		}
	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_UpdateContainer[%s] failed to parse response caught exception[%s]"),finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_UpdateContainer[%s] failed to parse response caught unknown exception[%d]"),finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_DeleteContainer(Json::Value& result, const std::string& uri)
{
//	std::string url = generateURL(uri, true);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_DeleteContainer[%s]"), uri.c_str());
	
	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_CONTAINER_TYPE);

	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_DeleteContainer", uri, finalURL, strStatus, CDMI_CONTAINER, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_DEL, (_flags>>16));

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiFuseOps::CdmiRetCode)cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "cdmi_DeleteContainer[%s]: %s(%d)<=[%s] took %dms"), finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::nonCdmi_DeleteContainer(const std::string& uri)
{
//	std::string url = generateURL(uri, true);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "nonCdmi_DeleteContainer[%s]"), uri.c_str());

	std::string finalURL;
	std::string strStatus;
	Properties requestHeaders;
	std::string requestBody;
	Properties respHeaders;
	std::string responseTxtBody;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("nonCdmi_DeleteContainer", uri, finalURL, strStatus, CDMI_CONTAINER, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, responseTxtBody, buflen,NULL,
		ZQ::common::CURLClient::HTTP_DEL, (_flags>>16));
	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiFuseOps::CdmiRetCode)cdmiRetCode;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "nonCdmi_DeleteContainer[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

	return (CdmiFuseOps::CdmiRetCode) cdmiRetCode;

}

 CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_IndicateClose(const std::string& uri)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_IndicateClose[%s]"), uri.c_str());
	if (uri.empty() || (LOGIC_FNSEPC == uri[uri.length() -1]))
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_IndicateClose() ignored due to non-DataObject uri[%s]"), uri.c_str());
		return cdmirc_SDK_BadArgument;
	}

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Content-Type", CDMI_DATAOBJECT_TYPE);

	std::string requestBody = "{ \"close\": \"1\" }";
	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_IndicateClose", uri, finalURL, strStatus, CDMI_FILE, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_PUT, (_flags>>16) | ZQ::common::CURLClient::sfEnableOutgoingDataCB);

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "cdmi_IndicateClose[%s]: %s(%d)<=[%s] took %dms"), finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	return cdmiRetCode;
}


CdmiFuseOps::CdmiRetCode CdmiFuseOps::cdmi_ReadAquaDomain(Json::Value& result, std::string& domainUri)
{
	if (_serverSideParams.domainURI.empty())
		getServerIp(false, true);

	if (domainUri.empty())
		domainUri = _serverSideParams.domainURI;

	bool bHomeTouchNeeded = false;
	{
		ZQ::common::MutexGuard g(_lkLocationCache); // just borrow a lock
		if (_domainURIOfMP.empty())
			bHomeTouchNeeded = true;
		else domainUri = _domainURIOfMP;
	}

	if (bHomeTouchNeeded)
	{
		Json::Value result;
		if (CdmiRet_SUCC(cdmi_ReadContainer(result, pathToUri(""))) && JSON_HAS(result, "domainURI"))
		{
			ZQ::common::MutexGuard g(_lkLocationCache); // just borrow a lock
			domainUri = _domainURIOfMP = result["domainURI"].asString();
			_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "cdmi_ReadAquaDomain() got domainURI[%s] of homeContainer"), _domainURIOfMP.c_str());
		}
	}

	if (!domainUri.empty() && '/' == domainUri[0])
		domainUri = domainUri.substr(1);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cdmi_ReadAquaDomain[%s]"), domainUri.c_str());

	std::string finalURL;
	Properties requestHeaders, respHeaders;
	MAPSET(Properties, requestHeaders, "Accept",       "application/cdmi-domain");

	std::string requestBody;
	std::string strResponse, respStatus;
	uint buflen=0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("cdmi_ReadAquaDomain", domainUri, finalURL, respStatus, CDMI_CONTAINER, false,
							 requestHeaders, requestBody, NULL, 0,
							 respHeaders, strResponse, buflen, NULL,
							 ZQ::common::CURLClient::HTTP_GET, (_flags>>16));

	if (!CdmiRet_SUCC(cdmiRetCode))
		return (CdmiFuseOps::CdmiRetCode) cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "cdmi_ReadAquaDomain[%s]: %s(%d)<=[%s] took %dms"), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, respStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	//1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_ReadAquaDomain[%s] failed to parse response len[%d]: %s"),finalURL.c_str(), strResponse.length(), reader.getFormatedErrorMessages().c_str());
			return cdmirc_RequestFailed;
		}
	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_ReadAquaDomain[%s] failed to parse response caught exception[%s]"), finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "cdmi_ReadAquaDomain[%s] failed to parse response caught unknown exception[%d]"), finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}

	return cdmiRetCode;
}

static std::string methodToStr(const ZQ::common::CURLClient::HTTPMETHOD& method)
{
	switch(method)
	{
	case ZQ::common::CURLClient::HTTP_GET:
		return "GET";
	case ZQ::common::CURLClient:: HTTP_DEL:
		return "DELETE";
	case ZQ::common::CURLClient::HTTP_PUT:
		return "PUT";
	case ZQ::common::CURLClient::HTTP_POST:
		return "POST";
	case ZQ::common::CURLClient::HTTP_HEAD:
		return "HEAD";
	default:
		return "CUSTOM";
	}
}

bool CdmiFuseOps::generateSignature(std::string& signature, const std::string& uri, const std::string& contentType, ZQ::common::CURLClient::HTTPMETHOD method, const std::string& xAquaDate)
{
	std::string url="";
	int durLogin =0;
	int64 stampStart = ZQ::common::now();

	// step 1. fetch secretKeyId
	std::string secretKeyId;
	do {
		{
			ZQ::common::MutexGuard g(_lkLogin);
			if (!_secretKeyId.empty())
			{
				secretKeyId = _secretKeyId;
				break;
			}
		}

		Json::Value result;
		std::string serverIp = getServerIp();
		if(serverIp.empty())
			return false;

		std::string rootUrl = assembleURL(serverIp, "");
		int cdmiCode = login(result, _log, _thrdPool, rootUrl, _username, _userDomain, _password, (_flags>>16), this);
		if (CdmiRet_FAIL(cdmiCode))
			return false;

		durLogin = (int)(ZQ::common::now() - stampStart);

		try
		{	
			if (!JSON_HAS(result, "secretAccessKey") || !JSON_HAS(result, "objectID"))
				return false;

			// save the _secretKeyId/_accessKeyId just gotten
			ZQ::common::MutexGuard g(_lkLogin);
			secretKeyId = _secretKeyId = result["secretAccessKey"].asString();
			_accessKeyId = result["objectID"].asString();
		}
		catch(std::exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "generateSignature[%s] failed to parse response caught exception[%s]"), url.c_str(), ex.what());
		}
		catch (...)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "generateSignature[%s] failed to parse response caught unknown exception[%d]"), url.c_str(), SYS::getLastErr());
		}

	} while (0);

	if (secretKeyId.empty() || uri.empty() || xAquaDate.empty())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "generateSignature[%s] invalid input parameters uri, secretKey[%s] or xAquaDate[%s]"),
			 uri.c_str(), secretKeyId.c_str(), xAquaDate.c_str());
		return false;
	}

	std::string CanonicalizedResource  = uri;
    if(!CanonicalizedResource.empty() && CanonicalizedResource[0] != '/')
		CanonicalizedResource = "/" + CanonicalizedResource;

	std::string strToSign = methodToStr(method) + "\n" + contentType + "\n" + xAquaDate + "\n" + CanonicalizedResource;
	{
		char buf[512]= "";
		snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "generateSignature() stringToSign:"));
		_log.hexDump(ZQ::common::Log::L_DEBUG, strToSign.c_str(), (int)strToSign.size(), buf, 0 == (flgHexDump&_flags));
	}

	unsigned char digestBuf[HMAC_SHA1_DIGEST_LENGTH+1]="";
	// hmac_sha1((unsigned char*)strToSign.c_str(), strToSign.size(), (unsigned char*)_secretKeyId.c_str(),  _secretKeyId.size(), digestBuf);
	ZQ::common::HMAC_SHA1::calcSignature((unsigned char*)strToSign.c_str(), strToSign.size(), (unsigned char*)secretKeyId.c_str(),  secretKeyId.size(), digestBuf);
   
	char* pOutBufPtr = NULL;
	size_t outBufLen = 0;
	signature = ZQ::common::Base64::encode(digestBuf, HMAC_SHA1_DIGEST_LENGTH);
	//bool bret= base64Encode((const char*)digestBuf, HMAC_SHA1_DIGEST_LENGTH, &pOutBufPtr, &outBufLen);
	//if(!bret)
	//{
	//	_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "generateSignature[%s] failed to encode to base64 with secretKey[%s]"), url.c_str(), secretKeyId.c_str());
	//	// discard the _secretKeyId/_accessKeyId to force to get at next time
	//	ZQ::common::MutexGuard g(_lkLogin);
	//	_secretKeyId = "";
	//	return false;
	//}

	//signature.clear();
	//signature.append(pOutBufPtr, outBufLen);

	try
	{
		if(pOutBufPtr)
			free( pOutBufPtr);
		pOutBufPtr = NULL;
	}	
	catch (...){ }

	char buf[512]= "";

	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "generateSignature[%s] signature calc-ed, took %d/%dmsec:"), url.c_str(), secretKeyId.c_str(), durLogin, (int) (ZQ::common::now() - stampStart));
	_log.hexDump(ZQ::common::Log::L_DEBUG, signature.c_str(), (int)outBufLen, buf, true);

	return true;
}

int CdmiFuseOps::login(Json::Value& result, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootUrl, const std::string& userName, const std::string& userDomain, const std::string &password, uint16 curlflags, void* pCtx)
{
	ZQ::common::MutexGuard g(_lkLogin);

	ZQ::common::URLStr rooturl(rootUrl.c_str());
	const char* host = rooturl.getHost();
	ZQ::common::URLStr strUrl;
	std::string path = DEFAULT_LOGIN_PATH + userName;
	strUrl.setPath(path.c_str());
	strUrl.setUserName(userName.c_str());
	strUrl.setPwd(password.c_str());
	strUrl.setProtocol("https");
	strUrl.setHost(host);
	//strUrl.setPort(port);
	if(_serverSideParams.portHTTPS > 0)
		strUrl.setPort(_serverSideParams.portHTTPS);
	else
		strUrl.setPort(8443);

	std::string url = (char*)strUrl.generate();

	int64 lStartTime = ZQ::common::now();
	ZQ::CDMIClient::CDMIHttpClient::Ptr cdmiClient = new ZQ::CDMIClient::CDMIHttpClient((char*)url.c_str(), log, thrdPool, curlflags, ZQ::common::CURLClient::HTTP_GET); 
	if(!cdmiClient)
	{
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	if(!cdmiClient->init())
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "login[%s] failed to init libcurl"), url.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	cdmiClient->setHeader("Accept", CDMI_URER_TYPE);
	cdmiClient->setHeader("X-CDMI-Specification-Version", CDMI_Version);
	if(_serverSideParams.domainURI.empty())
		cdmiClient->setHeader("x-aqua-user-domain-uri", "/cdmi_domains/defaultdomainname/");
	else
		cdmiClient->setHeader("x-aqua-user-domain-uri", userDomain.c_str());

	ZQ::common::BufferList::Ptr pRequestBody  = new ZQ::common::BufferList();
	ZQ::common::BufferList::Ptr pResponseBody = new ZQ::common::BufferList();
	if (!pRequestBody || !pResponseBody)
	{
		log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "login() [%s] failed to allocate BufferList for request/response"), url.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	// step 1 build up the requestBody
	// empty request, nothing to do

	// step 2 build up the responseBody
	// take the non-reference buffer of BufferList, nothing to do

	size_t reqBodyLen =cdmiClient->setRequestBody(pRequestBody);
	if (reqBodyLen <0)
	{
		log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "login[%s] failed to set request boby"), url.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	size_t respBodyLen = cdmiClient->setResponseBody(pResponseBody);
	if (respBodyLen <0)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "login[%s] failed to set response buffer"), url.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	CURLcode retcode = CURLE_OK;
	bool bRet = cdmiClient->sendRequest(retcode, true);
	if(!bRet)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "login[%s] request failed took %dms"), url.c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	std::string strStatus;
	int cdmiRetCode = cdmiClient->getStatusCode(strStatus);

/*	if(pCtx != NULL)
	{
		CdmiFuseOps* cdmiFuseOps = (CdmiFuseOps*)pCtx;
		if(InConnectedFailed(retcode))
		{
			cdmiFuseOps->getRootURL(rootUrl,true); // roundrobin to take the next server IP
		}
	}
*/
	if (!CdmiRet_SUCC(cdmiRetCode))
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "login[%s] failed to login: %s(%d)<=[%s]: %s took %dms"),
			url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return cdmiRetCode;
	}
/*
	size_t dataSize = 0;
	const char* pStrResponse = cdmiClient->getRespBuf(dataSize);

	std::string strResponse;
	strResponse.append(pStrResponse, dataSize);
*/
	size_t dataSize = pResponseBody->length();
	std::string strResponse;
	pResponseBody->readToString(strResponse);

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "login[%s]: %s(%d)<=[%s] took %dms"), 
		url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	log.hexDump(ZQ::common::Log::L_INFO, (char*)strResponse.c_str(), (int)dataSize, buf, true);

	cdmiClient = NULL;
	//1.1 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "login[%s] failed to parse response len[%d] took %dms: %s"), 
				url.c_str(), strResponse.length(), (int)(ZQ::common::now() - lStartTime), reader.getFormatedErrorMessages().c_str());
			return cdmirc_RequestFailed;
		}

//		_username = userName;
		return cdmirc_OK;
	}
	catch(std::exception& ex)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "login[%s] failed to parse response caught exception[%s]"),url.c_str(), ex.what());
	}
	catch (...)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "login[%s] failed to parse response caught unknown exception[%d]"),url.c_str(), SYS::getLastErr());
	}

  return cdmirc_RequestFailed;
}

// thread unsafe
bool CdmiFuseOps::_getServerSideConfig(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string& rootUrl, uint16 curlflags, std::string bindIp)
{
	log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "_getServerSideConfig() populating the server configuration from %s"), rootUrl.c_str());
	// refer to http://192.168.87.16/mediawiki/index.php/Other_-_CDMI_-_High_Level_Design_-_Aqua#Get_configuration
	// to send GET _rootURL "/aqua/rest/cdmi/cdmi_config" to retrieve the configurations
	// into _serverSideParams.serverIPs, _serverSideParams.portHTTP, _serverSideParams.portHTTPS and _serverSideParams.domainURI

	_serverSideParams.portHTTPS = 0;
	_serverSideParams.portHTTP = 0;
	_serverSideParams.domainURI.clear();
	_serverSideParams.serverIPs.clear();

	std::string url =  rootUrl + "cdmi_config";
	int64 lStartTime = ZQ::common::now();
	ZQ::CDMIClient::CDMIHttpClient::Ptr cdmiClient = new ZQ::CDMIClient::CDMIHttpClient((char*)url.c_str(), log, thrdPool, curlflags, ZQ::common::CURLClient::HTTP_GET, (char*)bindIp.c_str());
	if(!cdmiClient)
	{
		cdmiClient = NULL;
		return false;
	}

	if(!cdmiClient->init())
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "_getServerSideConfig[%s] failed to init libcurl"), url.c_str());
		cdmiClient = NULL;
		return false;
	}

	cdmiClient->setHeader("Accept", CDMI_CONFIG_TYPE);

	ZQ::common::BufferList::Ptr pRequestBody  = new ZQ::common::BufferList();
	ZQ::common::BufferList::Ptr pResponseBody = new ZQ::common::BufferList();
	if (!pRequestBody || !pResponseBody)
	{
		log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "_getServerSideConfig() [%s] failed to allocate BufferList for request/response"), url.c_str());
		cdmiClient = NULL;
		return false;
	}

	// step 1 build up the requestBody
	// empty request, nothing to do

	// step 2 build up the responseBody
	// take the non-reference buffer of BufferList, nothing to do

	size_t reqBodyLen =cdmiClient->setRequestBody(pRequestBody);
	if (reqBodyLen <0)
	{
		log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "_getServerSideConfig() [%s] failed to set request boby"), url.c_str());
		cdmiClient = NULL;
		return false;
	}

	size_t respBodyLen =cdmiClient->setResponseBody(pResponseBody);
	if (respBodyLen <0)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "_getServerSideConfig() [%s] failed to set response buffer"), url.c_str());
		cdmiClient = NULL;
		return false;
	}

	CURLcode retcode = CURLE_OK;
	bool bRet = cdmiClient->sendRequest(retcode, true);
	if(!bRet)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "_getServerSideConfig() [%s] request failed took %dms"), url.c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return false;
	}

	std::string strStatus;
	int cdmiRetCode = cdmiClient->getStatusCode(strStatus);
	if (!CdmiRet_SUCC(cdmiRetCode))
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "_getServerSideConfig() [%s] failed to get root URL: %s(%d)<=[%s]: %s took %dms"),
			url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return false;
	}

/*	size_t dataSize = 0;
	const char* pStrResponse = cdmiClient->getRespBuf(dataSize);

	std::string strResponse;
	strResponse.append(pStrResponse, dataSize);
*/ 
	size_t dataSize = pResponseBody->length();
	std::string strResponse;
	pResponseBody->readToString(strResponse);

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "_getServerSideConfig[%s]: %s(%d)<=[%s] took %dms"), 
		url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	log.hexDump(ZQ::common::Log::L_INFO, (char*)strResponse.c_str(), (int)dataSize, buf, true);

	cdmiClient = NULL;
	//1.1 parser cdmi response boy
	Json::Reader reader;
	Json::Value result;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "_getServerSideConfig[%s] failed to parse response len[%d] took %dms: %s"),
				url.c_str(), strResponse.length(), (int)(ZQ::common::now() - lStartTime), reader.getFormatedErrorMessages().c_str());
			return false;
		}
		/*       
		HTTP/1.1 200 OK
		Content-Type: application/cdmi-config

		{
		"rootURI":"/aqua/rest/cdmi",
		"httpport":8080,
		"httpsport":8443,
		"frontendIPs":["10.50.16.180","10.50.16.182"],
		"defaultDomainURI":"/cdmi_domains/default/"
		}
		*/

		if (!JSON_HAS(result, "httpport") || !JSON_HAS(result, "httpsport") || !JSON_HAS(result, "rootURI") || !JSON_HAS(result, "defaultDomainURI") || !JSON_HAS(result, "frontendIPs"))
		{
			log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "_getServerSideConfig[%s]Auqa server missed some parameter"), url.c_str());
			return false;
		}

		_serverSideParams.portHTTPS = result["httpsport"].asInt();
		_serverSideParams.portHTTP = result["httpport"].asInt();
		_serverSideParams.rootURI = result["rootURI"].asString();
		_serverSideParams.domainURI = result["defaultDomainURI"].asString();

		std::string frontendIps;
		Json::Value& children = result["frontendIPs"];

		for (Json::Value::iterator itF = children.begin(); itF != children.end(); itF++)
		{
			_serverSideParams.serverIPs.push_back((*itF).asString());
			frontendIps += (*itF).asString()+ ",";
		}

		if (!frontendIps.empty())
			frontendIps = frontendIps.substr(1);

		log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "_getServerSideConfig() by[%s] got: rootURI[%s], httpport[%d], httpsPort[%d], defaultDomainURI[%s], frontendIPs[%s]. took %dmsec"),
			url.c_str(),_serverSideParams.rootURI.c_str(), _serverSideParams.portHTTP, _serverSideParams.portHTTPS, _serverSideParams.domainURI.c_str(), frontendIps.c_str(), (int)(ZQ::common::now() - lStartTime));

		return true;
	}
	catch(std::exception& ex)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "_getServerSideConfig[%s] failed to parse response caught exception[%s]"),url.c_str(), ex.what());
	}
	catch (...)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "_getServerSideConfig[%s] failed to parse response caught unknown exception[%d]"),url.c_str(), SYS::getLastErr());
	}

	return false;
}

int CdmiFuseOps::getContainer(Json::Value& result, ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, const std::string &rootUrl, uint16 curlflags, std::string bindIp)
{
	int64 lStartTime = ZQ::common::now();
	ZQ::CDMIClient::CDMIHttpClient::Ptr cdmiClient = new ZQ::CDMIClient::CDMIHttpClient((char*)rootUrl.c_str(), log, thrdPool, curlflags, ZQ::common::CURLClient::HTTP_GET, (char*)bindIp.c_str());
	cdmiClient->setHeader("Accept", CDMI_CONTAINER_TYPE);
	cdmiClient->setHeader("Content-Type", CDMI_CONTAINER_TYPE);
	cdmiClient->setHeader("X-CDMI-Specification-Version", CDMI_Version);
	cdmiClient->setHeader("X-CDMI-Partial", "false");

	if(!cdmiClient)
	{
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}
	if(!cdmiClient->init())
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "getContainer[%s] failed to init libcurl"), rootUrl.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	ZQ::common::BufferList::Ptr pRequestBody  = new ZQ::common::BufferList();
	ZQ::common::BufferList::Ptr pResponseBody = new ZQ::common::BufferList();
	if (!pRequestBody || !pResponseBody)
	{
		log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "getContainer() failed to allocate BufferList for request/response"));
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	// step 1 build up the requestBody
	// empty request, nothing to do

	// step 2 build up the responseBody
	// take the non-reference buffer of BufferList, nothing to do

	size_t reqBodyLen =cdmiClient->setRequestBody(pRequestBody);
	if (reqBodyLen <0)
	{
		log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "getContainer[%s] failed to set request boby"), rootUrl.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	size_t respBodyLen =cdmiClient->setResponseBody(pResponseBody);
	if (respBodyLen <0)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "getContainer[%s] failed to set response buffer"), rootUrl.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	CURLcode retcode = CURLE_OK;
	bool bRet = cdmiClient->sendRequest(retcode, true);
	if(!bRet)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "getContainer[%s] request failed took %dms"), rootUrl.c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}
	std::string strStatus;
	int cdmiRetCode = cdmiClient->getStatusCode(strStatus);
	if (!CdmiRet_SUCC(cdmiRetCode))
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "getContainer[%s] failed to login: %s(%d)<=[%s]: %s took %dms"),
			rootUrl.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return cdmiRetCode;
	}

	size_t dataSize = pResponseBody->length();
	std::string strResponse;
	pResponseBody->readToString(strResponse);

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "getContainer[%s]: %s(%d)<=[%s] took %dms"), 
		rootUrl.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	log.hexDump(ZQ::common::Log::L_INFO, (char*)strResponse.c_str(), (int)dataSize, buf, true);
	cdmiClient = NULL;
	//1.1 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "getContainer[%s] failed to parse response len[%d] took %dms: %s"),
				rootUrl.c_str(), strResponse.length(), (int)(ZQ::common::now() - lStartTime), reader.getFormatedErrorMessages().c_str());
			return cdmirc_RequestFailed;
		}
		return cdmirc_OK;
	}
	catch(std::exception& ex)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "getContainer[%s] failed to parse response caught exception[%s]"),rootUrl.c_str(), ex.what());
	}
	catch (...)
	{
		log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "getContainer[%s] failed to parse response caught unknown exception[%d]"),rootUrl.c_str(), SYS::getLastErr());
	}

	return cdmirc_RequestFailed;
}

std::string CdmiFuseOps::assembleURL(const std::string& serverIp, const std::string& uri, ObjectType objType, bool includeUsername)
{
//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "assembleURL()serverIP[%s] uri[%s]Container[%s]includeUsername[%s]rootURI[%s]"),
//		serverIp.c_str(), uri.c_str(), bContainer ?"yes": "no", includeUsername ? "yes":"no", _serverSideParams.rootURI.c_str());

	std::string newuri = _serverSideParams.rootURI;
	if (!newuri.empty() && newuri[0]!= '/')
		newuri = std::string("/") + newuri;

	if(!newuri.empty() && newuri[newuri.length() -1] != '/')
		newuri += "/";
	newuri += ((!uri.empty() && uri[0] == '/') ? uri.substr(1): uri);
		
	ZQ::common::URLStr urlst((std::string("http://") + serverIp + newuri).c_str(),true);
	urlst.setPort(_serverSideParams.portHTTP);
	std::string path = urlst.getPath();

//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "assembleURL()path[%s]"),path.c_str());

	if (objType == CDMI_CONTAINER && !path.empty() && path[path.length() -1] != '/')
	{
		path +="/";
		urlst.setPath((char*)path.c_str());
	}

	if (includeUsername)
	{
		urlst.setUserName(_username.c_str());
		urlst.setPwd(_password.c_str());
	}

	newuri = urlst.generate();
//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "assembleURL()1 newuri[%s]"),newuri.c_str());

	if(!newuri.empty() && newuri[newuri.length() -1] == '=')
		newuri = newuri.substr(0, newuri.length() - 1);

//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "assembleURL()2 newuri[%s]"),newuri.c_str());

	// fixup those "key=&" to "key&"
	size_t pos;
	while (std::string::npos != (pos = newuri.find("=&")))
		newuri.replace(pos, sizeof("=&")-1, "&");

//	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "assembleURL()3 newuri[%s]"),newuri.c_str());

	return newuri;
}

std::string CdmiFuseOps::getServerIp(bool next, bool forceReadConfig)
{
	ZQ::common::MutexGuard g(_lkLogin);
	if (forceReadConfig || _serverSideParams.portHTTP <= 0 || _serverSideParams.portHTTPS <= 0 || _serverSideParams.rootURI.empty())
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "getServerIp() enforce to query for server-side config per forceReadConfig[%c] portHTTP[%d] portHTTPS[%d] rootURI[%s]"), 
			forceReadConfig?'T':'F', _serverSideParams.portHTTP, _serverSideParams.portHTTPS, _serverSideParams.rootURI.c_str());

		if (!_getServerSideConfig(_log, _thrdPool, _rootUrl, (_flags >> 16), _bindIp))
			return "";

		// re-park the idxServer by searching the fetched IP list
		ZQ::common::URLStr urlst(_rootUrl.c_str());
		std::string currentServer = urlst.getHost();

		StrList ipBak = _serverSideParams.serverIPs;
		_serverSideParams.serverIPs.clear();
		_serverSideParams.serverIPs.push_back(currentServer);

		for (_idxServer =0; _idxServer < ipBak.size(); _idxServer++)
		{
			if (0 == currentServer.compare(ipBak[_idxServer]))
				continue;

			_serverSideParams.serverIPs.push_back(ipBak[_idxServer]);
		}

		_serverSideParams.serverIPs.push_back(currentServer);
		next = false; // always false if just refreshed the server IP list 
		_idxServer = 0;
	}

	if (next)
	{
		if (!_rootOk && _serverSideParams.serverIPs.size() > 1)
		{
			_idxServer = (++_idxServer) % _serverSideParams.serverIPs.size();
			//fdj
			if(_serverSideParams.serverIPs.size() > 2  && _idxServer == 1 )
			{
				_pCheckHealthyTrd->notify();									
				_rootOk = false;
			}
		}
		else 
		{
			_idxServer =0;
			if(_rootOk)
				_rootOk = false;
		}
	}

	//	if (next && _serverSideParams.serverIPs.size() >1)
	//		idxServer++;
	//	idxServer %= _serverSideParams.serverIPs.size();

	return _serverSideParams.serverIPs[_idxServer];
}

bool CdmiFuseOps::readLocationFromCache(const std::string path, ResourceLocation& resLoc)
{
	std::string key = path;
	if (!key.empty() && key[key.length() -1] == '/')
		key = key.substr(0, key.length() -1);

	ZQ::common::MutexGuard g(_lkLocationCache);
	LocationCache::iterator it = _locationCache.find(key);
	if (_locationCache.end() ==it)
		return false;
	
	resLoc = it->second;
	return true;
}

void CdmiFuseOps::cacheLocation(const ResourceLocation& resLoc)
{
	std::string key = resLoc.path;
	if (!key.empty() && key[key.length() -1] == '/')
		key = key.substr(0, key.length() -1);

	ZQ::common::MutexGuard g(_lkLocationCache);
	_locationCache[key] = resLoc;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cacheLocation()[%s]=>[%s]:[%s]"), key.c_str(), resLoc.locationIp.c_str(), resLoc.paramsAppended.c_str());
}

void CdmiFuseOps::uncacheLocation(const std::string path)
{
    ZQ::common::MutexGuard g(_lkLocationCache);
	_locationCache.erase(path);
}

typedef enum _TryStage
{
	TryStage_PreCached,
	TryStage_CurrentRootURL,
	TryStage_NextServerIP,
	TryStage_Redirected,
	TryStage_GivenUp,
	TryStage_MAX
} TryStage;

static const char* RestryStageName(int tryStage)
{
#define CASE_STAGE(_STAGE_CODE) case TryStage_##_STAGE_CODE: return #_STAGE_CODE
	switch(tryStage)
	{
	CASE_STAGE(PreCached);
	CASE_STAGE(CurrentRootURL);
	CASE_STAGE(NextServerIP);
	CASE_STAGE(Redirected);
	default:
	CASE_STAGE(GivenUp);
	}
}

uint CdmiFuseOps::getCurrentServerIdx()
{
	return _currentServerIdx;
}

std::string CdmiFuseOps::getRelocatedURL(std::string& uri, const ResourceLocation& loc, ObjectType objType, bool includeUsername)
{
	if (std::string::npos == uri.find(loc.paramsAppended))
	{
		char* leadingDelimitor = "&";
		if (std::string::npos == uri.find_last_of("?"))
			leadingDelimitor = "?";

		uri += std::string(leadingDelimitor) + loc.paramsAppended;
	}

	return assembleURL(loc.locationIp, uri, objType, includeUsername);
}

int CdmiFuseOps::callRemote(const char* purposeStr, std::string uri, std::string& finalURL, std::string& respStatusLine,  ObjectType objType, bool includeUsername,
							const Properties& requestHeaders, const ZQ::common::BufferList::Ptr pRequestBody,
							Properties& responseHeaders, in out ZQ::common::BufferList::Ptr pResponseBody,
							ZQ::common::CURLClient::HTTPMETHOD method, int clientFlags, std::string uriAffinity)
{
	std::string path_original = uri, param_original, path;
	size_t pos = uriAffinity.find_first_of("?#");
	if (std::string::npos != pos)
		uriAffinity = uriAffinity.substr(0, pos);

	pos = uri.find_first_of("?#");
	if (std::string::npos != pos)
	{
		path_original = uri.substr(0, pos);
		param_original = uri.substr(pos);
	}

	if (objType == CDMI_CONTAINER && !path_original.empty() && path_original[path_original.length() -1] != '/')
		path_original += "/";

	if (!pRequestBody || !pResponseBody)
	{
		_log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "callRemote() %s[%s] null BufferList for request or response"), purposeStr, uri.c_str());
		return cdmirc_RequestFailed;
	}

	std::string startServerIp, serverIp;
	int cdmiRetCode = cdmirc_RetryFailed;

	ResourceLocation loc;
	int nextTryStage = TryStage_PreCached;
    ZQ::CDMIClient::CDMIHttpClient::Ptr cdmiClient;
	std::string cliDesc;
	int64 stampNow = ZQ::common::now();
	int64 lTxnStartTime = stampNow, lStartTime=lTxnStartTime;
	int durStageDetermine =0;
	const char* stageName = RestryStageName(TryStage_GivenUp);

	for (int retry=1; TryStage_GivenUp != nextTryStage && retry < TryStage_MAX; retry++) // maximally try 3 times
	{
		stageName = RestryStageName(nextTryStage);
		lStartTime = ZQ::common::now();
		if (lStartTime > lTxnStartTime + _retryTimeout)
		{
			// quit the loop if the retries in sum had exceeded the timeout
			char stampBuf[64];
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "path[%s] retries had exceeded retryTimeout[%d]msec: currentTry[%d@%s], txnStart[%s]"), path.c_str(),
				_retryTimeout, retry, stageName, ZQ::common::TimeUtil::TimeToUTC(lTxnStartTime, stampBuf, sizeof(stampBuf)-2, true));
			cdmiRetCode = cdmirc_RetryFailed;
			break; 
		}
	
		// step 1. determin the various uri and url per the different try stages
		switch(nextTryStage)
		{
		case TryStage_PreCached: // try the location cache
			nextTryStage = TryStage_CurrentRootURL;
			if (readLocationFromCache(uriAffinity.empty() ? path_original: uriAffinity, loc))
			{
				// adjust the uri and url with cached information
				uri = path_original + param_original;
				finalURL = getRelocatedURL(uri, loc, objType, includeUsername);
				break; // of switch
			}
			// NOTE: no "break;" if no cache found here

		case TryStage_CurrentRootURL:
			stageName = RestryStageName(TryStage_CurrentRootURL); // because could be continued from TryStage_PreCached

			nextTryStage = TryStage_NextServerIP;

			// previous cached location must became invalid when reach here, get rid of it
			uncacheLocation(path_original);

			// remember where we start with the server ip
			startServerIp = serverIp = getServerIp();

			if(serverIp.empty())
				return cdmirc_RequestFailed;
			// compose the new uri and url with the current url
			uri = path_original + param_original;
			finalURL = assembleURL(serverIp, uri, objType, includeUsername);
			break;

		case TryStage_NextServerIP:
			serverIp = getServerIp(true);
			if(serverIp.empty())
			   return cdmirc_RequestFailed;
			// if all the server IPs has been tried, give up the retry
			if (serverIp == startServerIp)
			{
				nextTryStage = TryStage_GivenUp;
				continue;
			}

			uri = path_original + param_original;
			finalURL = assembleURL(serverIp, uri, objType, includeUsername);

			break;

		case TryStage_Redirected:

			nextTryStage = TryStage_GivenUp;

			if (!readLocationFromCache(path_original, loc) || loc.locationIp.empty())
			{
				_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "path[%s] can't find locationIp from Cache()"), path.c_str());
				continue; // should be recently cached, if no, move the next stage = TryStage_GivenUp
			}

			// compose the new uri and url up to the location
			uri = path_original + param_original;
			finalURL = getRelocatedURL(uri, loc, objType, includeUsername);

			break;

		case TryStage_GivenUp:
		default:
			//TODO: log

			// no thing to do but get out the retry loop
			nextTryStage = TryStage_GivenUp;
			cdmiRetCode = cdmirc_RetryFailed;
			continue; 
		}

		durStageDetermine = (int)(ZQ::common::now() -lStartTime);

		// step 2.0 generate the signature
		std::string contentType="";
		Properties::const_iterator itHd = requestHeaders.find("Content-Type");
		if (requestHeaders.end() != itHd)
			contentType = itHd->second;
		std::string signature, xAquaDate = nowToAquaData();

		ZQ::common::URLStr tempURL((char*)finalURL.c_str());
		std::string path = tempURL.getPath();

		if (!generateSignature(signature, path, contentType, method, xAquaDate))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "%s[%s] failed to generate Signature"), purposeStr, finalURL.c_str());
			return cdmirc_RequestFailed;
		}

		// step 2. create the client and compose the outgoing request
		//cdmiClient = NULL;
		//cdmiClient = new ZQ::CDMIClient::CDMIHttpClient((char*)finalURL.c_str(), _log, _thrdPool, clientFlags, method); 
		cdmiClient = openClient(finalURL, clientFlags, method);

		if (!cdmiClient)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "%s[%s] failed to instant client"), purposeStr, finalURL.c_str());
			// cdmiClient = NULL;
			return cdmirc_RequestFailed;
		}

		uint thisTimeout = (uint) (lTxnStartTime + _retryTimeout -lStartTime);
		if (thisTimeout > _operationTimeout)
			thisTimeout = _operationTimeout;

		cdmiClient->setTimeout(_connectTimeout, thisTimeout);

		if (!cdmiClient->init())
		{		
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "%s[%s] failed to init libcurl"), purposeStr, finalURL.c_str());
			// cdmiClient = NULL;
			return cdmirc_RequestFailed;
		}

		cliDesc = cdmiClient->getDesc();

		// 2.1 apply the request headers
		for (Properties::const_iterator it= requestHeaders.begin(); it != requestHeaders.end(); it++)
		{
			if (it->first.empty())
				continue;
			cdmiClient->setHeader(it->first.c_str(), it->second.c_str());
		}

		// 2.2 apply the signature
		cdmiClient->setHeader("Authorization" , std::string("AQUA ") + _accessKeyId + ":" + signature);
		cdmiClient->setHeader("x-aqua-date", xAquaDate);

		// 2.3 other common headers
		cdmiClient->setHeader("X-CDMI-Specification-Version",CDMI_Version);

		size_t reqBodyLen =cdmiClient->setRequestBody(pRequestBody);
		if (reqBodyLen <0)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "%s[%s]@%s failed to set request boby"), purposeStr, finalURL.c_str(), stageName);
			cdmiClient = NULL;
			continue; // continue to the next stage
		}

		size_t respBodyLen =cdmiClient->setResponseBody(pResponseBody);
		if (respBodyLen <0)
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "%s[%s]@%s failed to set response buffer"), purposeStr, finalURL.c_str(), stageName);
			cdmiClient = NULL;
			SYS::sleep(_retryInterval);
			continue; // continue to the next stage
		}

		// step 3. send the request out
		stampNow = ZQ::common::now();
		lStartTime = stampNow;
		CURLcode retcode = CURLE_OK;
		bool bRequestSent = false;
		for (int j =((reqBodyLen - SEND_REQ_RETRY_MIN_BODY_SZ)? SEND_REQ_MAX_RETRY:1); j >0; j--) // inner retry loop by using the same client
		{
			bRequestSent = cdmiClient->sendRequest(retcode, true);
			if (bRequestSent)
				break; // request is sent sucessfully

			switch (retcode)
			{
			case CURLE_COULDNT_CONNECT:
			case CURLE_WRITE_ERROR:
			case CURLE_UPLOAD_FAILED:
			case CURLE_READ_ERROR:
				break;

			case CURLE_OPERATION_TIMEDOUT:
			default:
				j=0; break;
			}

			stampNow = ZQ::common::now();
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "%s[%s]@%s by[%s] request failed[%s,%s] took %dms, r[%d]left, [%d, sum%d/%d]msec"), purposeStr, finalURL.c_str(), stageName, cliDesc.c_str(),
				ZQ::CDMIClient::CDMIHttpClient::errorStr(retcode).c_str(), cdmiClient->getErrorMessage().c_str(),(int)(stampNow - lStartTime), (j>1)?j-1:0, _operationTimeout, (int)(stampNow - lTxnStartTime), _retryTimeout);
			
			if (j)
				SYS::sleep(_retryInterval);
		}

		if (!bRequestSent)
		{
			std::string clientId = cdmiClient->getClientSessionId();
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuseOps, "%s[%s]@%s sendRequest() failed, client[%s] eliminated with lasterr(%d), yield %dmsec"), purposeStr, finalURL.c_str(), stageName, clientId.c_str(), retcode, _retryInterval);
			cdmiClient = NULL;
			SYS::sleep(_retryInterval);
			continue; // continue to the next stage
		}

		cliDesc = cdmiClient->getDesc();

		// step 4. read the response

		// step 4.1 pre-dispatch the known error cases
		cdmiRetCode = cdmiClient->getStatusCode(respStatusLine);
		
		// case 4.1.1 stupid Aqua redirect, cache the location and try TryStage_Redirected
		if (cdmirc_AquaLocation == cdmiRetCode)
		{
			loc.locationIp = loc.paramsAppended = "";
			loc.stampInfoKnown = ZQ::common::now();
			loc.path = path_original;

			responseHeaders = cdmiClient->getResponseHeaders();
			itHd = responseHeaders.find("x-aqua-redirect-ip");
			if (responseHeaders.end()!=itHd)
				loc.locationIp = itHd->second;

			itHd = responseHeaders.find("x-aqua-redirect-tag");
			if (responseHeaders.end()!=itHd)
				loc.paramsAppended = itHd->second;

			if (!loc.path.empty() && !loc.locationIp.empty())
				cacheLocation(loc);

			_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "%s[%s]@%s by[%s] server indicates redirect: %s(%d)<=[%s]: %s took %dms, x-aqua-redirect-ip[%s], x-aqua-redirect-tag[%s]"), purposeStr, finalURL.c_str(), stageName, cliDesc.c_str(),
				cdmiRetStr(cdmiRetCode), cdmiRetCode, respStatusLine.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime), loc.locationIp.c_str(), loc.paramsAppended.c_str());

			if (ZQ::common::CURLClient::HTTP_PUT == method && (NULL == pRequestBody.get() || pRequestBody->length() <=0))
			{
				// another stupid Aqua definition: it indeed did the action when redirect a empty PUT, no need to post the request to the new location
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "%s[%s]@%s by[%s] subtitute %s(%d) to cdmirc_OK"), purposeStr, finalURL.c_str(), stageName, cliDesc.c_str(),
					cdmiRetStr(cdmiRetCode), cdmiRetCode);
				cdmiRetCode = cdmirc_OK;
				break;
			}

			// continue to the next stage = STAGE_TRY_REDIRECTED
			cdmiClient = NULL;
			nextTryStage  = TryStage_Redirected;
			cdmiRetCode = cdmirc_RetryFailed;

            // SYS::sleep(_retryInterval);
			continue;  
		}

		// case 4.1.2 stupid Aqua authentication, no auto renewable expiration, to start over again if retry count allows
		if (cdmirc_Unauthorized == cdmiRetCode)
		{
			_secretKeyId = "";

			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "%s[%s]@%s by[%s] failed: %s(%d)<=[%s]: %s took %dms"), purposeStr, finalURL.c_str(), stageName, cliDesc.c_str(),
				cdmiRetStr(cdmiRetCode), cdmiRetCode, respStatusLine.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime));
	
			// start over again from next stage=TryStage_PreCached, if retry count allows
			cdmiClient  = NULL;
			nextTryStage   = TryStage_PreCached;
			cdmiRetCode = cdmirc_RetryFailed;
			SYS::sleep(_retryInterval);
			continue; 
		}

		// when reach here, there must be a explict return from the CDMI server, break the retry loop
		break;
	}

	// step Ex4.2 either explict response from the CDMI reserver or max-retry reached

	if (!CdmiRet_SUCC(cdmiRetCode))
	{
		std::string errorMessage = "";
		if (cdmiClient)
		{
			errorMessage = cdmiClient->getErrorMessage();
			if (errorMessage.empty())
			{
				Properties::const_iterator itHd = responseHeaders.find("ETag"); // Aqua's error message
				if (responseHeaders.end()!=itHd)
					errorMessage = itHd->second;
			}
		}

		stampNow = ZQ::common::now();
		_log((objType == CDMI_CONTAINER && (cdmirc_NotFound == cdmiRetCode)) ? ZQ::common::Log::L_INFO : ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "%s[%s]@%s by[%s] failed: %s(%d)<=[%s] took %dms, txn %dms: %s"), purposeStr, finalURL.c_str(), stageName, cliDesc.c_str(),
			cdmiRetStr(cdmiRetCode), cdmiRetCode, respStatusLine.c_str(), (int)(stampNow - lStartTime), (int)(stampNow - lTxnStartTime), errorMessage.c_str());

		if (cdmiRetCode < cdmirc_ExtErr)
			cacheClient(cdmiClient); // this is a good client, cache it for next use

		cdmiClient = NULL;
		return (CdmiFuseOps::CdmiRetCode) cdmiRetCode;
	}

	// responseBody = cdmiClient->getResponseBody();
	responseHeaders = cdmiClient->getResponseHeaders();
	stampNow = ZQ::common::now();
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "%s[%s]@%s by[%s]: %s(%d)<=[%s]: %s took %dms, txn %d/%dmsec; content-len[%d]"), purposeStr, finalURL.c_str(), stageName, cliDesc.c_str(),
		cdmiRetStr(cdmiRetCode), cdmiRetCode, respStatusLine.c_str(), cdmiClient->getErrorMessage().c_str(),(int)(stampNow- lStartTime), durStageDetermine, (int)(stampNow - lTxnStartTime), pResponseBody->length());

	cacheClient(cdmiClient); // this is a good client, cache it for next use
	cdmiClient = NULL;
	return cdmiRetCode;
}

int CdmiFuseOps::callRemote(const char* purposeStr, in std::string uri, out std::string& finalURL, out std::string& statusLine,  ObjectType objType, bool includeUsername,
							const Properties& requestHeaders, const std::string& requestBody, const char* reqBodyBuf, int reqBodyLen,
							Properties& responseHeaders, std::string& responseTxtBody, in out uint& len, char* recvbuff,
							ZQ::common::CURLClient::HTTPMETHOD method, int clientFlags, std::string uriAffinity)
{
	ZQ::common::BufferList::Ptr pRequestBody  = new ZQ::common::BufferList(true);
	if (!pRequestBody)
	{
		_log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "callRemote() %s[%s] failed to allocate BufferList for request"), purposeStr, uri.c_str());
		return cdmirc_RequestFailed;
	}

	// step 1 build up the requestBody
	if (NULL == reqBodyBuf || reqBodyLen <=0)
		pRequestBody->join((uint8*) requestBody.c_str(), requestBody.length());
	else if (reqBodyLen >0)
		pRequestBody->join((uint8*) reqBodyBuf, reqBodyLen);

	// step 2 build up the responseBody
	ZQ::common::BufferList::Ptr pResponseBody = NULL;
	bool bRespTextMode = false;
	if (NULL == recvbuff || len <=0)
		bRespTextMode = true;

	if (bRespTextMode)
	{
		pResponseBody = new ZQ::common::BufferList(); // the non-reference bufferlist
	}
	else
	{
		pResponseBody = new ZQ::common::BufferList(true);
		pResponseBody->join((uint8*) recvbuff, len);
	}

	if (!pResponseBody)
	{
		_log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "%s callRemote() [%s] failed to allocate BufferList for response"), purposeStr, uri.c_str());
		return cdmirc_RequestFailed;
	}

	// step 3. forward to callRemote()
	CdmiFuseOps::CdmiRetCode ret = callRemote(purposeStr, uri, finalURL, statusLine, objType, includeUsername,
		requestHeaders, pRequestBody, responseHeaders, pResponseBody,
		method, clientFlags, uriAffinity);

	if (!CdmiRet_SUCC(ret))
		return ret;

	// step 4. confirm the length of the response body
	len = pResponseBody->length();

	if (bRespTextMode)
	{
		responseTxtBody = "";
		pResponseBody->readToString(responseTxtBody);
	}

	return ret;
}

bool CdmiFuseOps::readFileInfoFromCache(const std::string& filepath, FileInfo& fileInfo)
{
	ZQ::common::MutexGuard g(_lkFileInfos);
	FileInfoMap::iterator itFileInfo = _fileInfos.find(filepath);
	if (_fileInfos.end() == itFileInfo)
		return false;

	/*
	NOTE: validating moved to getFileInfo()

		// if the source file has been recently written, consider the cached info as not stable
		if (itFileInfo->second.filestat.st_mtime >= (uint32)thrshTime)
			bTakingCached = false;

		// eliminate the cached info if it is too old
		if (itFileInfo->second.stampInfoKnown < stampExpired)
			bTakingCached = false;

		if (!bTakingCached) 
		{
			// remove the expired file info from the cache
			_fileInfos.erase(filepath);
			return false;
		}
*/

	// copy the cached info as the returning result
	fileInfo = itFileInfo->second; 
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "readFileInfoFromCache() file[%s] found in cached info, cachesize[%d]"), filepath.c_str(), _fileInfos.size());
	
	return true;
}

void CdmiFuseOps::cacheFileInfo(const std::string& pathname, const FileInfo& fileInfo)
{
	ZQ::common::MutexGuard g(_lkFileInfos);
	_fileInfos[pathname] = fileInfo;
	if (fileInfo.stampInfoKnown < 10000)
		_fileInfos[pathname].stampInfoKnown = ZQ::common::now();
#ifdef _DEBUG
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cacheFileInfo() path[%s] info cached"), pathname.c_str());
#endif // _DEBUG
}

void CdmiFuseOps::uncacheFileInfo(const std::string& pathname)
{
	ZQ::common::MutexGuard g(_lkFileInfos);
	_fileInfos.erase(pathname);
#ifdef _DEBUG
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "uncacheFileInfo() path[%s] info removed"), pathname.c_str());
#endif // _DEBUG
}

// this piece of data-struct is to avoid multiple concurrent queries for a same file to Aqua
typedef std::map<std::string, std::pair<int, int64> > Path2Thread;
static Path2Thread _busyQueries;
static ZQ::common::Mutex     _busyLocker;

int CdmiFuseOps::getFileInfo(const std::string& pathname, FileInfo& fi, bool skipGuessContainer)
{
	FileInfo_reset(fi);

#ifdef ZQ_OS_MSWIN
	Time_t thrshTime = _time64(NULL);
#else 
	time_t thrshTime = time(0);
#endif
	thrshTime -= 60; // a minute ago, time_t's unit is second

	int64 stampNow = ZQ::common::now();
	std::string cacheStatus;

	if (readFileInfoFromCache(pathname, fi))
	{
		cacheStatus += "C";
		// to avoid multiple concurrent queries for a same file to Aqua
		ZQ::common::MutexGuard g(_busyLocker);
		Path2Thread::iterator it = _busyQueries.find(pathname);
		if (_busyQueries.end() != it && it->second.second > (stampNow -1000))
		{
			cacheStatus +="Z";
			// some other thread is currentling query the infomation of this file, so just take whatever within the cache
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] take the cache due to info is currently queried by thread[%d] %dms ago"), 
				pathname.c_str(), it->second.first, (int)(stampNow - it->second.second));
			return cdmirc_OK; 
		}

		//valid if not recently written and not known too old
		do {
			if (fi.filestat.st_mtime >= (uint32)thrshTime)
			{
				cacheStatus += "W";
#ifdef _DEBUG
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] recently written, %lld"), 
					pathname.c_str(), (int64)fi.filestat.st_mtime);
#endif // _DEBUG
				break;
			}

			int64 stampExpired =0;
			if (_fuseOpsConf.attrCache_TTLsec >0)
				stampExpired = stampNow - _fuseOpsConf.attrCache_TTLsec *1000;

			if (fi.stampInfoKnown < stampExpired)
			{
				cacheStatus += "E";
#ifdef _DEBUG
				_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] attr expired, %lld<%lld<%lld"), 
					pathname.c_str(), (int64)fi.stampInfoKnown, (int64)stampExpired, (int64)stampNow);
#endif // _DEBUG
				break;
			}

#ifdef _DEBUG
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] takes attr of cache"), pathname.c_str());
#endif // _DEBUG
			return cdmirc_OK;

		} while (0);
	}

//#ifndef ZQ_OS_MSWIN
//	if (readFileInfoFromCache(pathname+LOGIC_FNSEPS, fi))
//		return cdmirc_OK;
//#endif // ZQ_OS_MSWIN

	std::string uri = pathname;
	CdmiFuseOps::CdmiRetCode cdmirc = cdmirc_BadRequest;
	Json::Value result;
	std::string contentType = "";

	if (!uri.empty() && uri[uri.length() -1] == '/')
		uri = uri.substr(0, uri.length() -1);

	{
		// to avoid multiple concurrent queries for a same file to Aqua
		ZQ::common::MutexGuard g(_busyLocker);
		std::pair <int, int64> v(__THREADID__, stampNow);
		MAPSET( Path2Thread, _busyQueries, pathname, v);
	}

	cdmirc = isExist(result, uri + "?metadata", contentType);

	if (!CdmiRet_SUCC(cdmirc))
	{
		_log(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] uri[%s] failed to get fsta: %s(%d)"), pathname.c_str(), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmirc;
	}

	if(contentType == CDMI_CONTAINER_TYPE)
		fi.filestat.st_mode |= _S_IFDIR;

	if (!JSON_HAS(result, "metadata"))
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] uri[%s] no metadata received"), pathname.c_str(), uri.c_str());
		return cdmirc_ServerError;
	}

	if (!dataObjMetadataToFstat(pathname, result["metadata"], fi))
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] uri[%s] failed to convert fstat"), pathname.c_str(), uri.c_str());
		return cdmirc_ServerError;
	}

/*
	bool bIsACertainDir = false;
	if (pathname.empty() || (FNSEPC == pathname[pathname.length() -1]))
		bIsACertainDir = true; // it is certainly a directory

	Json::Value result;
	std::string uri = pathToUri(pathname); // + LOGIC_FNSEPS;
	CdmiFuseOps::CdmiRetCode cdmirc = cdmirc_BadRequest;

	if (!skipGuessContainer || bIsACertainDir)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] trying cdmi_ReadContainer(%s) to touch"), pathname.c_str(), uri.c_str());
		cdmirc = cdmi_ReadContainer(result, uri);

		if (CdmiRet_SUCC(cdmirc))
		{
			bIsACertainDir = true; // this is confirmed as an existing container on the server
			fi.filestat.st_mode |= _S_IFDIR;
//			FileInfoByHandle->dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
			if (JSON_HAS(result, "metadata"))
			{
				Json::Value& metadata = result["metadata"];
				dataObjMetadataToFstat(metadata, fi);
				fi.filestat.st_mode |= _S_IFDIR;
			}

			return cdmirc_OK;
		} // if (CdmiRet_SUCC(cdmirc)
	}
	
	if (bIsACertainDir)
	{
		_log(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] cdmi_ReadContainer(%s) as folder failed: %s(%d)"), pathname.c_str(), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmirc;
	}

	// step 1. query CDMI for the attr of the source file
	uri = pathToUri(pathname);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] calling cdmi_ReadDataObject(%s) to read attributes"), pathname.c_str(), uri.c_str());
	std::string tmp;
	cdmirc = cdmi_ReadDataObject(result, uri +"?metadata", tmp);

	if (!CdmiRet_SUCC(cdmirc))
	{
		_log(LOGL_COND_WARN(!CdmiRet_ClientErr(cdmirc)), CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] cdmi_ReadDataObject(%s) as file failed: %s(%d)"), pathname.c_str(), uri.c_str(), cdmiRetStr(cdmirc), cdmirc);
		return cdmirc;
	}

	if (!JSON_HAS(result, "metadata"))
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] cdmi_ReadDataObject(%s) no metadata received"), pathname.c_str(), uri.c_str());
		return cdmirc_ServerError;
	}

	if (!dataObjMetadataToFstat(result["metadata"], fi))
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] cdmi_ReadDataObject(%s) failed to convert fstat"), pathname.c_str(), uri.c_str());
		return cdmirc_ServerError;
	}
*/
	// step 3. put this query result into the cache for next use
	cacheFileInfo(pathname, fi);

	{
		// to avoid multiple concurrent queries for a same file to Aqua
		ZQ::common::MutexGuard g(_busyLocker);
		_busyQueries.erase(pathname);
	}

#ifdef _DEBUG
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "getFileInfo() path[%s] attribute refreshed, previous cacheStatus[%s]"), pathname.c_str(), cacheStatus.c_str());
#endif // _DEBUG
	return cdmirc_OK;
}

static void fixupPathName(std::string& pathname)
{
	if (pathname.empty() || pathname[pathname.length()-1] != FNSEPC)
		pathname += FNSEPC;
}

void CdmiFuseOps::cacheChildren(const std::string& pathname, DirChildren& children)
{
	std::string path = pathname;
	fixupPathName(path);
	std::sort(children._children.begin(), children._children.end());

	ZQ::common::MutexGuard g(_lkFileInfos);
	_dirChildrens[path] = children;
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cacheChildren() path[%s] info cached, %d children"), path.c_str(), children._children.size());
}

/*
void CdmiFuseOps::uncacheChildren(const std::string& pathname)
{
	std::string path = pathname;
	fixupPathName(path);

	ZQ::common::MutexGuard g(_lkFileInfos);
	_dirChildrens.erase(path);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "uncacheChildren() path[%s] children is removed"), path.c_str());
}
*/

void CdmiFuseOps::cacheChild(const std::string& pathname)
{
	std::string parent, child = pathname;
	size_t sep = child.find_last_of(LOGIC_FNSEPS FNSEPS);
	if (std::string::npos != sep)
	{
		sep++;
		parent = child.substr(0, sep);
		child = child.substr(sep);
	}

	ZQ::common::MutexGuard g(_lkFileInfos);
	DirChildrenMap::iterator itParent = _dirChildrens.find(parent);
	if (_dirChildrens.end() == itParent)
		return;

	StrList::iterator itChild = std::upper_bound(itParent->second._children.begin(), itParent->second._children.end(), child);
	if (itChild > itParent->second._children.begin() && itChild < itParent->second._children.end() && *(itChild -1) == child)
		return; // already there

	itParent->second._children.insert(itChild, child);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "cacheChild() path[%s] child[%s] is added into dir[%s]"), pathname.c_str(), child.c_str(), parent.c_str());
}

void CdmiFuseOps::uncacheChild(const std::string& pathname)
{
	// uncache the children of parent
	std::string parent, child = pathname;
	size_t sep = child.find_last_of(LOGIC_FNSEPS FNSEPS);
	if (std::string::npos != sep)
	{
		sep++;
		parent = child.substr(0, sep);
		child = child.substr(sep);
	}

	ZQ::common::MutexGuard g(_lkFileInfos);
	DirChildrenMap::iterator itParent = _dirChildrens.find(parent);
	if (_dirChildrens.end() == itParent)
		return;

	StrList::iterator itChild = std::lower_bound(itParent->second._children.begin(), itParent->second._children.end(), child);
	if (itParent->second._children.end() == itChild || *itChild != child)
		return; // not found

	itParent->second._children.erase(itChild);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "uncacheChild() path[%s] child[%s] is removed from dir[%s]"), pathname.c_str(), child.c_str(), parent.c_str());
}

void CdmiFuseOps::stampChildren(const std::string& pathname)
{
	std::string path = pathname;
	fixupPathName(path);

	ZQ::common::MutexGuard g(_lkFileInfos);
	DirChildrenMap::iterator it = _dirChildrens.find(path);
	if (_dirChildrens.end() == it)
		return;

	it->second.stampLast = ZQ::common::now();
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "stampChildren() path[%s] children is stamped"), path.c_str());
}

CdmiFuseOps::DirChildren CdmiFuseOps::getCachedChildren(const std::string& pathname)
{
	std::string path = pathname;
	fixupPathName(path);

	const static DirChildren NIL = {0, 0};
	int64 stampExpired =0;
	if (_fuseOpsConf.attrCache_childrenTTLsec >0)
	{
		stampExpired = ZQ::common::now() - _fuseOpsConf.attrCache_childrenTTLsec *1000;
		if (_slowThrdPool.pendingRequestSize() >_slowThrdPool.size() *9)
			stampExpired -= _fuseOpsConf.attrCache_childrenTTLsec *2000;
	}

	ZQ::common::MutexGuard g(_lkFileInfos);
	DirChildrenMap::iterator itchdr = _dirChildrens.find(path);
	if (_dirChildrens.end() == itchdr)
		return NIL;

	if (itchdr->second.stampOpen < stampExpired)
	{ 
		// the cached children has been expired, erase directly
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "getCachedChildren() expired %d children of dir[%s]"), itchdr->second._children.size(), path.c_str());
		_dirChildrens.erase(path);
		return NIL;
	}

	return itchdr->second;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::getDiskSpace(int64& freebytes, int64& totalbytes)
{
	Json::Value result;
	std::string domainURI;

	int exp = MIN(_fuseOpsConf.attrCache_childrenTTLsec, _fuseOpsConf.attrCache_TTLsec)/10 *1000;
	if (exp >20*1000)
		exp = 20*1000;
	if (_domainFreeBytes < _domainTotalBytes /10) // speed up if near full
		exp = 2*1000;

	int64 stampExp = ZQ::common::now()  - exp;

	if (_stampDomainReadStart >stampExp || (_domainTotalBytes>0 &&  _stampDomainAsOf > stampExp))
	{
		freebytes = _domainFreeBytes; totalbytes = _domainTotalBytes;
		char stampBuf1[60], stampBuf2[60];
		ZQ::common::TimeUtil::TimeToUTC(_stampDomainReadStart, stampBuf1, sizeof(stampBuf1)-2, true);
		ZQ::common::TimeUtil::TimeToUTC(_stampDomainAsOf, stampBuf2, sizeof(stampBuf2)-2, true);

		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFUSE, "getDiskSpace() cdmi_ReadAquaDomain() was recently issued at %s, taking the latest values[%lld/%lld] as of %s, exp[%d]"), stampBuf1, freebytes, totalbytes, stampBuf2, exp);
		return cdmirc_Conflict;
	}

	_stampDomainReadStart = ZQ::common::now();
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFUSE, "getDiskSpace() calling cdmi_ReadAquaDomain() to read space usage"));
	CdmiFuseOps::CdmiRetCode cdmirc = cdmi_ReadAquaDomain(result, domainURI);
	
	if (CdmiRet_SUCC(cdmirc))
	{
		if (JSON_HAS(result, "metadata"))
		{
			Json::Value& metadata = result["metadata"];
			totalbytes = freebytes =0;
			if (JSON_HAS(metadata, "cdmi_assignedsize"))
				sscanf(metadata["cdmi_assignedsize"].asString().c_str(), "%lld", &totalbytes);

			if (JSON_HAS(metadata, "cdmi_size"))
				sscanf(metadata["cdmi_size"].asString().c_str(), "%lld", &freebytes);

			_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFUSE, "getDiskSpace() cdmi_ReadAquaDomain(%s) got cdmi_size[%lld] cdmi_assignedsize[%lld]"), domainURI.c_str(), freebytes, totalbytes);

			if (freebytes>0 && freebytes > totalbytes)
				totalbytes = freebytes;

			freebytes = totalbytes - freebytes;
			_log((totalbytes < MIN_BYTE_SIZE) ? ZQ::common::Log::L_WARNING : ZQ::common::Log::L_INFO, CLOGFMT(CdmiFUSE, "getDiskSpace() cdmi_ReadAquaDomain(%s) determined free[%lld] total[%lld]"), domainURI.c_str(), freebytes, totalbytes);
			_domainFreeBytes = freebytes; 
			_domainTotalBytes = totalbytes;
			_stampDomainAsOf = ZQ::common::now();
		}
	}
	else
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFUSE, "getDiskSpace() calling cdmi_ReadAquaDomain(%s) failed: %s(%d)"), domainURI.c_str(), cdmiRetStr(cdmirc), cdmirc);
		_stampDomainReadStart =0; // invalidate _stampDomainReadStart
	}

	if (totalbytes < MIN_BYTE_SIZE)
	{
		totalbytes = MIN_BYTE_SIZE;
		freebytes = totalbytes>>2;
	}

	if (freebytes < 0)
		freebytes =0;

	return cdmirc;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::flushdata( const std::string& uri ) {
	if( !_fuseOpsConf.enableCache )
		return cdmirc_OK;
	int err = getCacheTank(uri)->validate_writebuffer( uri );
	if( err == 0 )
		err = cdmirc_OK;
	_log(ZQ::common::Log::L_INFO,CLOGFMT(CdmiFUSE,"flushdata() flushed data for file[%s]: %s"),uri.c_str(), cdmiRetStr(err) );
	return err;
}

#define READ_JSON_CONF(_TYPE, _VAR, _CONFIG, _TAGNAME) if (JSON_HAS(_CONFIG, #_TAGNAME)) _VAR = _CONFIG[#_TAGNAME].as##_TYPE()
void CdmiFuseOps::readEnvConfig(const Json::Value& jsonConfig, std::string& logdir, int& loglevel, long& logsize, int& logcount, int& threads)
{
	READ_JSON_CONF(String, logdir,           jsonConfig, log.dir);
	READ_JSON_CONF(Int,    loglevel,         jsonConfig, log.level);
	READ_JSON_CONF(Int,    logsize,             jsonConfig, log.size);
	READ_JSON_CONF(Int,    logcount,            jsonConfig, log.count);
		
	loglevel &= 0x7;
	if (logsize < 100*1024)
		logsize = 100*1024;

	if (logcount < 1)
		logcount = 1;

	if (logdir.empty())
		logdir = "." FNSEPS;
	else if (FNSEPC != logdir[logdir.length() -1])
		logdir += FNSEPC;

	READ_JSON_CONF(Int,    threads,            jsonConfig, threads);
}

void CdmiFuseOps::readClientConfig(const Json::Value& jsonConfig, FuseOpsConf& clientConf)
{
	READ_JSON_CONF(Int, clientConf.fuseFlag,           jsonConfig, client.flags);

	// about the cache
	READ_JSON_CONF(Int, clientConf.enableCache,        jsonConfig, cache.enable);
	clientConf.enableCache = clientConf.enableCache?1:0;
	
	READ_JSON_CONF(Int, clientConf.tankConf.readCacheBlockSize,           jsonConfig, cache.block.size);
	READ_JSON_CONF(Int, clientConf.tankConf.readBlockCount,      jsonConfig, cache.block.count.read);
	READ_JSON_CONF(Int, clientConf.tankConf.writeBlockCount,     jsonConfig, cache.block.count.write);
	READ_JSON_CONF(Int, clientConf.tankConf.readAheadCount,      jsonConfig, cache.block.count.readahead);
	READ_JSON_CONF(Int64, clientConf.tankConf.cacheInvalidationInterval,  jsonConfig, cache.block.ttl.read);
	READ_JSON_CONF(Int64, clientConf.tankConf.bufferInvalidationInterval, jsonConfig, cache.block.ttl.write);
	
	READ_JSON_CONF(Int, clientConf.tankConf.logFlag,             jsonConfig, cache.log.flags);
	READ_JSON_CONF(Int, clientConf.tankConf.flushThreadPoolSize, jsonConfig, cache.threads.flush);
}

void CdmiFuseOps::startCURLenv()
{
	ZQ::common::CURLClient::startCurlClientManager();
}

void CdmiFuseOps::stopCURLenv()
{
	ZQ::common::CURLClient::stopCurlClientManager();
}

void CdmiFuseOps::setCache( bool enable ) {
	_log(ZQ::common::Log::L_INFO,CLOGFMT(CdmiFUSE,"setCache() :%s"), enable ? "enable":"disable" );
	_fuseOpsConf.enableCache = enable;
	if(!enable) {
		std::vector<CacheTank*>::iterator it = _cacheTanks.begin();
		for( ; it != _cacheTanks.end(); it ++) {
			(*it)->flushAllWriteBuffer();
		}
	}

}

/*
void CdmiFuseOps::detachClient()
{
	ZQ::common::MutexGuard g(_lkClientMap);
	_clientMap.erase(__THREADID__);
}
*/

void CdmiFuseOps::cacheClient(ZQ::CDMIClient::CDMIHttpClient::Ptr& pClient)
{
	ZQ::common::MutexGuard g(_lkClients);
	_clients.push_front(pClient);
	int c = _thrdPool.size();
	if (c > 200)
		c = 200;
	c = _clients.size() - c;

	while (c-- >0)
		_clients.pop_back();
}

ZQ::CDMIClient::CDMIHttpClient::Ptr CdmiFuseOps::openClient(const std::string& forURL, int curlflags, ZQ::common::CURLClient::HTTPMETHOD method)
{
	ZQ::CDMIClient::CDMIHttpClient::Ptr client;
	
	{
		ZQ::common::MutexGuard g(_lkClients);
		while (!client && _clients.size() >0)
		{
			client = _clients.front();
			_clients.pop_front();
		}
	}

	if (client)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFUSE, "openClient() newURL[%s] reusing client"), forURL.c_str());
		client->resetURL(forURL, method, curlflags);
		return client;
	}

/*
#ifdef CURL_INSTANCE_KEEPALIVE
	{
		ZQ::common::MutexGuard g(_lkClientMap);
		ClientMap::iterator itClient = _clientMap.find(__THREADID__);
		if (_clientMap.end() != itClient)
			client = itClient->second;
	}

	// double check if the previous client connects to the same endpoint of this wish
	std::string previousUrl;
	do {
		if (!client)
			break;

		long cRedirect;
		if (!client->getWorkingURL(previousUrl, cRedirect))
		{
			client = NULL;
			break;
		}

		ZQ::common::URLStr purl(previousUrl.c_str()), nurl(forURL.c_str());
		if (purl.getPort() != nurl.getPort() || 0 != strcmp(purl.getHost(), nurl.getHost()) )
		{
			client = NULL;
			break;
		}
	} while(0);

	if (client)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFUSE, "openClient() newURL[%s] reusing client of prevURL[%s]"), forURL.c_str(), previousUrl.c_str());
		client->resetURL(forURL, method, curlflags);
		return client;
	}

#endif // CURL_INSTANCE_KEEPALIVE
*/
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFUSE, "openClient() URL[%s] creating new client"), forURL.c_str());
	client = new ZQ::CDMIClient::CDMIHttpClient((char*)forURL.c_str(), _log, _thrdPool, curlflags, method, (char*)_bindIp.c_str());
/*
#ifdef CURL_INSTANCE_KEEPALIVE
	if (client)
	{
		ZQ::common::MutexGuard g(_lkClientMap);
		MAPSET(ClientMap, _clientMap, __THREADID__, client);
	}
#endif // CURL_INSTANCE_KEEPALIVE
*/

	return client;
}

/*
//CheckMaster(result, _log, _thrdPool,_username,_userDomain,_password,(_flags>>16), this)
int CdmiFuseOps::CheckMaster()
{
	Json::Value result;
	ZQ::common::MutexGuard g(_lkLogin);		//data protecte

	ZQ::common::URLStr rooturl(_rootUrl.c_str());
 	const char* host = rooturl.getHost();
	ZQ::common::URLStr strUrl;
	std::string path = CHECK_MASTER_PATH;		//fdj
	strUrl.setPath(path.c_str());
	strUrl.setUserName(_username.c_str()); 	
	strUrl.setPwd(_password.c_str());
	strUrl.setProtocol("http");
	strUrl.setHost(host);
// 	if(_serverSideParams.portHTTPS > 0)
// 		strUrl.setPort(_serverSideParams.portHTTPS);
// 	else
	strUrl.setPort(8080);
		//strUrl.setPort(8443);
		//http://demo:demo@172.16.20.157:8080/aqua/rest/cdmi/cdmi_status 
	std::string url = (char*)strUrl.generate();

	int64 lStartTime = ZQ::common::now();
	ZQ::CDMIClient::CDMIHttpClient::Ptr cdmiClient =
		new ZQ::CDMIClient::CDMIHttpClient((char*)url.c_str(), _log, _thrdPool, _flags, ZQ::common::CURLClient::HTTP_GET); 
	if(!cdmiClient)
	{
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}
	if(!cdmiClient->init())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "CheckMaster[%s] failed to init libcurl"), url.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}
	cdmiClient->setHeader("Accept", CDMI_URER_TYPE);
	cdmiClient->setHeader("X-CDMI-Specification-Version", CDMI_Version);
	if(_serverSideParams.domainURI.empty())
		cdmiClient->setHeader("x-aqua-user-domain-uri", "/cdmi_domains/defaultdomainname/");
	else
		cdmiClient->setHeader("x-aqua-user-domain-uri", _userDomain.c_str());

	ZQ::common::BufferList::Ptr pRequestBody  = new ZQ::common::BufferList();
	ZQ::common::BufferList::Ptr pResponseBody = new ZQ::common::BufferList();
	if (!pRequestBody || !pResponseBody)
	{
		_log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "CheckMaster()[%s] failed to allocate BufferList for request/response"), url.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	// step 1 build up the requestBody
	// empty request, nothing to do

	// step 2 build up the responseBody
	// take the non-reference buffer of BufferList, nothing to do

	size_t reqBodyLen =cdmiClient->setRequestBody(pRequestBody);
	if (reqBodyLen <0)
	{
		_log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "CheckMaster[%s] failed to set request boby"), url.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	size_t respBodyLen = cdmiClient->setResponseBody(pResponseBody);
	if (respBodyLen <0)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "CheckMaster[%s] failed to set response buffer"), url.c_str());
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	CURLcode retcode = CURLE_OK;
	bool bRet = cdmiClient->sendRequest(retcode, true);
	if(!bRet)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "CheckMaster[%s] request failed took %dms"), url.c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return cdmirc_RequestFailed;
	}

	std::string strStatus;
	int cdmiRetCode = cdmiClient->getStatusCode(strStatus);

	if (!CdmiRet_SUCC(cdmiRetCode))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "CheckMaster[%s] failed to CheckMaster: %s(%d)<=[%s]: %s took %dms"),
			url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return cdmiRetCode;
	}

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "CheckMaster[%s]: %s(%d)<=[%s] took %dms"), 
		url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));

  return cdmirc_RequestFailed;
}
*/

bool CdmiFuseOps::isHealthy()
{
/*	if(CheckMaster() == cdmirc_OK )
	{
			return true;
	}
	return false;*/
	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "isHealthy() check root server [%s] status"), _rootUrl.c_str());
	// refer to http://192.168.87.16/mediawiki/index.php/Other_-_CDMI_-_High_Level_Design_-_Aqua#Monitor
	// to send HEAD _rootURL "/aqua/rest/cdmi/cdmi_status" to check root server status the configurations

	std::string url =  _rootUrl + "cdmi_status";
	int64 lStartTime = ZQ::common::now();
	ZQ::CDMIClient::CDMIHttpClient::Ptr cdmiClient = new ZQ::CDMIClient::CDMIHttpClient((char*)url.c_str(), _log, _thrdPool, (_flags >> 16), ZQ::common::CURLClient::HTTP_GET, (char*)_bindIp.c_str());
	if(!cdmiClient)
	{
		cdmiClient = NULL;
		return false;
	}

	if(!cdmiClient->init())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "isHealthy[%s] failed to init libcurl"), url.c_str());
		cdmiClient = NULL;
		return false;
	}

	ZQ::common::BufferList::Ptr pRequestBody  = new ZQ::common::BufferList();
	ZQ::common::BufferList::Ptr pResponseBody = new ZQ::common::BufferList();
	if (!pRequestBody || !pResponseBody)
	{
		_log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "isHealthy() [%s] failed to allocate BufferList for request/response"), url.c_str());
		cdmiClient = NULL;
		return false;
	}

	// step 1 build up the requestBody
	// empty request, nothing to do

	// step 2 build up the responseBody
	// take the non-reference buffer of BufferList, nothing to do

	size_t reqBodyLen =cdmiClient->setRequestBody(pRequestBody);
	if (reqBodyLen <0)
	{
		_log(ZQ::common::Log::L_ERROR,  CLOGFMT(CdmiFuseOps, "isHealthy() [%s] failed to set request boby"), url.c_str());
		cdmiClient = NULL;
		return false;
	}

	size_t respBodyLen =cdmiClient->setResponseBody(pResponseBody);
	if (respBodyLen <0)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "isHealthy() [%s] failed to set response buffer"), url.c_str());
		cdmiClient = NULL;
		return false;
	}

	CURLcode retcode = CURLE_OK;
	bool bRet = cdmiClient->sendRequest(retcode, true);
	if(!bRet)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "isHealthy() [%s] request failed took %dms"), url.c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return false;
	}

	std::string strStatus;
	int cdmiRetCode = cdmiClient->getStatusCode(strStatus);
	if (!CdmiRet_SUCC(cdmiRetCode))
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "isHealthy() [%s] failed to check root server status, URL: %s(%d)<=[%s]: %s took %dms"),
			url.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), cdmiClient->getErrorMessage().c_str(), (int)(ZQ::common::now() - lStartTime));
		cdmiClient = NULL;
		return false;
	}

	return true;
}

//notify next connect root server
void CdmiFuseOps::notifyConnected()
{
	ZQ::common::MutexGuard g(_lkLogin);

	_rootOk = true;
	_idxServer = 0;
	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "notifyConnected root server[%d]:[%s] next time to connect"),
		_idxServer,_serverSideParams.serverIPs[_idxServer].c_str());
// 	std::string tmp1[3];
// 	tmp1[0] = _serverSideParams.serverIPs[0];
// 	tmp1[1] = _serverSideParams.serverIPs[1];
// 	tmp1[2] = _serverSideParams.serverIPs[2];
//	RestryStageName(TryStage_CurrentRootURL);		
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::isExist(Json::Value& result, const std::string& uri, std::string& contentType)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "isExist[%s]"), uri.c_str());

	std::string finalURL;
	std::string strStatus;

	Properties requestHeaders;
	MAPSET(Properties, requestHeaders, "Accept", CDMI_NODE_TYPE);

	std::string requestBody;
	Properties respHeaders;
	std::string strResponse;
	uint buflen = 0;

	int64 lStartTime = ZQ::common::now();
	int cdmiRetCode = callRemote("isExist", uri, finalURL, strStatus, CDMI_NODE, false, requestHeaders, requestBody, NULL, 0,
		respHeaders, strResponse, buflen,NULL,
		ZQ::common::CURLClient::HTTP_GET, (_flags>>16));

	if(CdmiRet_FAIL(cdmiRetCode))
		return  (CdmiFuseOps::CdmiRetCode)cdmiRetCode;

	char buf[512]= "";
	snprintf(buf,  sizeof(buf) -1, CLOGFMT(CdmiFuseOps, "isExist[%s] %s(%d)<=[%s] took %dms: "), 
		finalURL.c_str(), cdmiRetStr(cdmiRetCode), cdmiRetCode, strStatus.c_str(), (int)(ZQ::common::now() - lStartTime));
	_log.hexDump((flgHexDump&_flags) ? ZQ::common::Log::L_DEBUG : ZQ::common::Log::L_INFO, strResponse.c_str(), (int)strResponse.size(), buf, 0 == (flgHexDump&_flags));

	//	cdmiClient = NULL;
	//1.2 parser cdmi response boy
	Json::Reader reader;
	try
	{	
		if(!strResponse.empty() && !reader.parse(strResponse, result))
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "isExist[%s] failed to parse response len[%d]: %s"), finalURL.c_str(), strResponse.length(), reader.getFormatedErrorMessages().c_str());
			return cdmirc_RequestFailed;
		}

		Properties::iterator itorProps = respHeaders.find("Content-Type");
		if(itorProps != respHeaders.end())
		{
			contentType = itorProps->second;
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "isExist[%s] Content-Type(%s)"), finalURL.c_str(), contentType.c_str());
		}
	}
	catch(std::exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "isExist[%s] failed to parse response caught exception[%s]"),finalURL.c_str(), ex.what());
		cdmiRetCode = cdmirc_RequestFailed;
	}
	catch (...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "isExist[%s] failed to parse response caught unknown exception[%d]"),finalURL.c_str(), SYS::getLastErr());
		cdmiRetCode = cdmirc_RequestFailed;
	}

	return cdmiRetCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::openDir(const std::string& dirPath)
{
	int64 stampStart = ZQ::common::now();

	DirChildren dcOld = getCachedChildren(dirPath);

	// step 1. read the container
	CdmiRetCode retCode = cdmirc_OK;
	int  latencyReadContainer =0;
	bool bRefresh = false;
	uint32 yieldsec = 1;

	if (_fuseOpsConf.attrCache_childrenTTLsec >0)
	{
		yieldsec = _fuseOpsConf.attrCache_childrenTTLsec /60;
		if (yieldsec < 1)
			yieldsec =1;
		else if (yieldsec > 30)
			yieldsec = 30;
	}

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "openDir() trying to open folder[%s], last-in-cache[%d], yield %dsec"), dirPath.c_str(), dcOld._children.size(), yieldsec);
	// at least readContainer has 1sec idle
	size_t cChildren = 0;
	if (dcOld.stampLast > stampStart - yieldsec*1000)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "openDir() container[%s] has been recently read within %dsec, skip reading"),  dirPath.c_str(), yieldsec);
		cChildren = dcOld._children.size();
	}
	else
	{
		DirChildren dcNew;
		Json::Value result;
		retCode = cdmi_ReadContainer(result, dirPath);
		latencyReadContainer = (int)(ZQ::common::now() - stampStart);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "openDir() ReadContainer[%s] got[%s], took %dms"),  dirPath.c_str(), cdmiRetStr(retCode), latencyReadContainer);

		if( !CdmiRet_SUCC(retCode) )
		{
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CdmiFuseOps, "openDir() failed to ReadContainer[%s] took %dms: [%s(%d)]"),  dirPath.c_str(), cdmiRetStr(retCode), retCode);
			return retCode;
		}

		if (result.isMember("objectID"))
			dcNew.dirObjectId = result["objectID"].asString();

		//convert json value into dirent cache
		if (result.isMember("children"))
		{
			Json::Value& children = result["children"];//should never fail
			do {
				if (!children.isArray())
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuseOps, "openDir() path[%s] children of response is not an arrary"), dirPath.c_str(), dcOld._children.size(), children.size());
					break;
				}

				for( size_t i = 0 ; i < children.size(); i ++ )
				{
					const Json::Value& v = children[i];
					if(!v.isString())
					{	
						_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuseOps, "openDir() path[%s] got a non-string child entry, ignored"), dirPath.c_str());
						continue;
					}

					std::string name = v.asString();
					if(name.length() > 0 && name.at(name.length()-1) == '/')
						name = name.substr(0,name.length()-1);
					// dirCache.push_back( name );
					dcNew._children.push_back(name);
				}

			} while(0);
		}

		dcNew.stampOpen = stampStart;
		dcNew.stampLast =0;
		cacheChildren(dirPath, dcNew);
		cChildren = dcNew._children.size();
		bRefresh = true;
	} // end of read container

	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "openDir() path[%s] children refreshed[%c] size[%d] took %d/%dmsec"),
		dirPath.c_str(), bRefresh?'T':'F', cChildren, latencyReadContainer, (int)(ZQ::common::now() - stampStart));

	return retCode;
}

CdmiFuseOps::CdmiRetCode CdmiFuseOps::readDir(const std::string& dirPath, DirChildren& dc)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "readDir() trying to read folder[%s]"), dirPath.c_str());

	dc = getCachedChildren(dirPath);
	int64 stampStart = ZQ::common::now();

	// step 1. IGNORE due to there will always be an opendir() piror to readdir(): test if it is necessary to call openDir()

	// step 2. test if the last search in cache is still valid
	CdmiRetCode retCode = cdmirc_OK;
	bool bSkipCache = false;
	bSkipCache = bSkipCache || (dc.stampLast < dc.stampOpen);
	bSkipCache = bSkipCache || dc.dirObjectId.empty();
	bSkipCache = bSkipCache || (_fuseOpsConf.attrCache_childrenTTLsec >0 && (dc.stampLast < (stampStart - _fuseOpsConf.attrCache_childrenTTLsec*1000)));

	if (!bSkipCache)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOp, "readDir() the cached children(%d) looks good to take, skipping ChildReaders"), dc._children.size());
		return cdmirc_OK;
	}

	// step 3. issue the ChildReaders
	// now we have cached children in dcOld and newly list in dcNew, try to issue ChildReaders if neccessary
#define BATCH_ATTR_READ

#ifdef BATCH_ATTR_READ
#	define DEFAULT_BATCH_NO   (2)
#	define MAX_FILE_PER_BATCH (1000)
#	define MIN_FILE_PER_BATCH (50)
#endif

	int cReaders =0;

	do {
		if(0 == _fuseOpsConf.attrCache_byList)
		{
			// only take the new children list but skip ChildReaders
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CdmiFuseOps, "readDir() path[%s] skipping ChildReaders for %d children per attrCache_byList[0]"),
				dirPath.c_str(), dc._children.size());
			break;
		}

		size_t filesPerBatch = dc._children.size() / (DEFAULT_BATCH_NO +1);
		if (filesPerBatch >MAX_FILE_PER_BATCH)
			filesPerBatch = MAX_FILE_PER_BATCH;
		if (filesPerBatch <MIN_FILE_PER_BATCH)
			filesPerBatch = MIN_FILE_PER_BATCH;

		int maxPending = (filesPerBatch >0) ? (dc._children.size() /filesPerBatch +1) :1;
		if (maxPending < _slowThrdPool.size()*9)
			maxPending = _slowThrdPool.size()*9;

		if (_slowThrdPool.pendingRequestSize() > maxPending && dc._children.size() >0)
		{
			_log(ZQ::common::Log::L_WARNING, CLOGFMT(CdmiFuseOps, "readDir() path[%s] too many ChildReaders queued %d, force to taking the cached children[%d]"), dirPath.c_str(), _slowThrdPool.pendingRequestSize(), dc._children.size());
			break;
		}

		int readerId=0;
		int readOffset=0;
		StrList filesOfBatch;
		for (size_t i =0; i < dc._children.size(); i++)
		{
			std::string fullpath = dirPath + dc._children[i];
			filesOfBatch.push_back(fullpath);
			if (filesOfBatch.size() > filesPerBatch )
			{
				//issue read ahead for file attribute cache
				ChildReader* pReader = new ChildReader(*this, ++readerId, dc.dirObjectId, dirPath, filesOfBatch, 
					"", readOffset, readOffset + filesOfBatch.size() -1);
				pReader->start(); cReaders++;
				readOffset += filesOfBatch.size();
				filesOfBatch.clear();
			}
		}

		// the last ChildReader if necessary
		if (filesOfBatch.size() >0)
		{
			ChildReader* pReader = new ChildReader(*this, ++readerId, dc.dirObjectId, dirPath, filesOfBatch, 
				"", readOffset, readOffset + filesOfBatch.size() -1);

			pReader->start(); cReaders++;
			readOffset += filesOfBatch.size();
			filesOfBatch.clear();
		}

	} while(0);

	// step 3. refresh the children cache
	size_t cChildren = dc._children.size();
	
	_log(ZQ::common::Log::L_INFO, CLOGFMT(CdmiFuseOps, "readDir() path[%s] children size[%d], %d readers issued, took %dmsec"),
		dirPath.c_str(), cChildren, cReaders, (int)(ZQ::common::now() - stampStart));
	return retCode;
}

// -----------------------------
// class CheckServerHealthy
//
// watch the root server status thread
// -----------------------------
//fdj   2015.8.13.17£º42

CheckServerHealthy::CheckServerHealthy(CdmiFuseOps& cdmiOps,size_t checkTimes):_bQuit(false),_CheckTimes(checkTimes),_cdmiOps(cdmiOps)
{
}

CheckServerHealthy::~CheckServerHealthy()
{
	_bQuit = true;
}

//Check Server Healthy thread
int CheckServerHealthy::run()
{
	int OkTimes = 0;
	timeout_t interval = TIMEOUT_INF;
	while(!_bQuit)
	{
		_hNotify.wait(interval);

		if(_bQuit)
			break;

		if(checkHealth())
		{
			OkTimes++;
			if(OkTimes == _CheckTimes)
			{
				//root server is connection successful,notify CdmiFuseOps connect root server next time 
				_cdmiOps.notifyConnected();
				interval = TIMEOUT_INF;
				OkTimes = 0; 
				break;
			}
		}
		else
		{
			OkTimes = 0;
			interval = 500;
		}
	}
	return 0;
}

//judge the root server is connect or not
bool CheckServerHealthy::checkHealth()
{			
	return _cdmiOps.isHealthy();
}

void CheckServerHealthy::quit()
{
	_bQuit = true;
	_hNotify.signal();
}

void CheckServerHealthy::notify()
{
	_hNotify.signal();
}

