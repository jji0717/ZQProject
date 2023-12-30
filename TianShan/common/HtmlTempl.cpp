// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: HtmlTempl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/common/HtmlTempl.cpp $
// 
// 2     10-11-18 16:29 Xiaohui.chai
// fix the FireFox display issue
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 8     08-06-11 12:01 Xiaohui.chai
// changed toolbar style
// 
// 7     08-03-10 16:41 Xiaohui.chai
// added toolbar to page
// 
// 6     08-02-28 16:59 Xiaohui.chai
// added page attribute charset
// 
// 5     08-02-20 16:31 Xiaohui.chai
// 
// 4     08-02-19 14:50 Hui.shao
// temp checkin for XiaoHui
// 
// 3     08-02-19 14:42 Xiaohui.chai
// 
// 2     07-06-04 14:46 Hui.shao
// add log to Adapter
// ===========================================================================

#include "HtmlTempl.h"

namespace ZQTianShan {
namespace HtmlTempl {

void HtmlHeader_MainPage(std::ostream& out, const char* subTitle, const Tab tabs[], const int tabCount, const int current, const char* charset)
{
    if(NULL == charset || '\0' == (*charset))
        charset = "utf-8";

	out << "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>\n"
	       "<html><head><meta http-equiv='Content-Type' content='text/html;charset=" << charset << "'>\n"
	       "<link href='tsweb.css' rel='stylesheet' type='text/css'>\n";
	       
	out << "<title>TianShan Architecture - " << (subTitle? subTitle : "") << "</title>\n"
		<< "</head><body class='coolscrollbar'>\n"
        << "<div id='toolbar'></div>\n"
        << "<script src='toolbar.js'></script><script type='text/javascript'>\n"
        << "var g_toolbar = new Toolbar(); document.getElementById('toolbar').innerHTML = g_toolbar.build();\n"
        << "</script><div class='scrollPanel'><div class='backgroundPanel'>";
		
	if (tabCount >0)
	{
		out << "<div class='tab_panel'><ul style=\"padding:0;margin:0\">\n";
		for (int i = 0; i< tabCount; i++)
		{
			if (tabs[i].name.empty())
				continue;

			out << "  <li "	 << (current==i ? "id='current'" : "") << "><a href='" << tabs[i].href.c_str() 	<< "'><span>" 
				<< tabs[i].name.c_str()	<< "</span></a></li>\n";
		}
		out << "</ul></div>\n";
	}
	out << "<p>\n";
}

void HtmlFooter_MainPage(std::ostream& out, const char* hostname, const char* url)
{
/* for VC8
	char buf[256];
	sprintf(buf, "<a target='_parent' href='%s/index.html' >", (url ? url : ""));
	out <<	"<hr size='1'><address style='align: right;'><small><b>" << buf << "<img src='logo_tianshan.png' alt='TianShan' align='middle' border='0' width='50'></a>&nbsp;&nbsp; TianShan Server ";
	if (NULL != hostname)
		out <<	buf << hostname << "</a>";

	out << "</b>&nbsp;&nbsp;&nbsp;" << ::ZQTianShan::TimeToUTC(::ZQTianShan::now(), buf, sizeof(buf) -2) << "<br></small></address></body></html>";
*/
	std::string hostref = std::string("<a target='_parent' href='") + (url?url:"/index.html") + "'>";

	out <<	"<hr size='1'><address style='align: right;'><small><b>" << hostref << "<img src='logo_tianshan.png' alt='TianShan' align='middle' border='0' width='50'></a>&nbsp;&nbsp; TianShan Products -- " << (hostname? hostref+hostname +"</a>" :"") << "</b>&nbsp;&nbsp;&nbsp; Fri May 25 15:17:28 2007 &nbsp;<br>"
			"</small></address></div></div></body></html>";
}

void HtmlFooter_MainPage_trivial(std::ostream& out)
{
    out << "</div></div></body></html>";
}
}} // namespace
