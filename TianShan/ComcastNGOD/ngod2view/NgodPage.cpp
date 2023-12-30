
#include "NgodPage.h"
#include <cctype>
#include <algorithm>

namespace ngod2view
{

template<class CharT> CharT strTolower(CharT c)
{
	return tolower(c);
}

NgodPage::NgodPage( IHttpRequestCtx* pRequest )
:BasePage(pRequest)
{
}

NgodPage::~NgodPage(void)
{
}
bool NgodPage::get( )
{
	return displayNGODResUsage();
}

bool NgodPage::post( )
{
	//
	return get( );
}

#define ResetCountersKey "reset"

bool NgodPage::displayNGODResUsage( )
{
	IHttpResponse& responser = _reqCtx->Response();
	try
	{
        // reset the counters if need
        const char* needReset = _reqCtx->GetRequestVar(ResetCountersKey);
        if(needReset && 0 == strcmp(needReset, "1"))
        {
            _viewPrx->resetCounters();
        }

		const char* isenable = _reqCtx->GetRequestVar("isenable");
		isenable = isenable ? isenable : "";
		if (0 != strcmp(isenable, ""))
		{
			const char* names = _reqCtx->GetRequestVar("streamerNames");
			names = names ? names : "";
			if (0 != strcmp(names, ""))
			{
				TianShanIce::StrValues streamersName;
				splitString(streamersName, names, ";");
				bool bEnable = ((strcmp(isenable, "1") == 0) ? true : false);
				try
				{
					_viewPrx->enableStreamers(streamersName, bEnable);
				}
				catch (const Ice::Exception& ex)
				{
				}
			}
		}

		NGOD::NgodUsage				sopUsages;
		NGOD::ImportChannelUsageS	importChannelUsages;
		sopUsages.clear( );
		importChannelUsages.clear();

		std::string		strMeasuredSince;
		_viewPrx->getNgodUsage( sopUsages , strMeasuredSince );
		_viewPrx->getImportChannelUsage( importChannelUsages );

		responser<< "<script type='text/javascript'>"
            << "var toolbar = new Toolbar();\n";
		responser << "var icon0 = new Icon();\n"
			<< "icon0.setImageSrc('images/toolbar/modify.gif');\n"
			<< "icon0.setImageHoverSrc('images/toolbar/modify_over.gif');\n"
			<< "icon0.setLabel('reset');\n"
			<< "icon0.setAction('reset()');\n"
			<< "icon0.setTips('Request to reset');\n";
		responser << "toolbar.addIcon(icon0);\n";

        // construct the reset url
        {
            ZQ::common::URLStr url(_reqCtx->GetRootURL(), true); // case sensitive
            url.setPath(_reqCtx->GetUri());
            url.setVar(TemplateKey, _reqCtx->GetRequestVar(TemplateKey)); //template
            url.setVar(Ngod2BindAddressKey, _reqCtx->GetRequestVar(Ngod2BindAddressKey)); //bind address
            url.setVar(ResetCountersKey, "1");

            responser << "function reset(){\n"
                << "\twindow.location = '"
                << url.generate()
                << "';\n}\n";
        }
		responser << "function enableStreamers(isEnable)\n"
		          << "{\n"
				  <<      "var streamerNames=\"\";\n"
				  <<      "var obj=document.getElementsByName(\"enableStreamer\");\n"
				  <<      "for(var i=0;i<obj.length;i++)\n"
				  <<      "{\n"
				  <<           "if(obj[i].checked == true)\n"
				  <<           "{\n"
				  <<              "streamerNames+=obj[i].value;\n"
				  <<              "streamerNames+=\";\";\n"
				  <<           "}\n"
				  <<      "}\n"
				  <<      "document.getElementById(\"isenable\").value=isEnable;\n"
				  <<      "document.getElementById(\"streamerNames\").value=streamerNames;\n"
				//  <<      "alert(streamerNames);\n"
				  <<      "document.getElementById(\"searchform\").submit();\n"
			      << "}\n";
		responser << "function selectAll()\n"
			      << "{\n"
				  <<      "var all=document.getElementById(\"all\");\n"
				  <<      "var obj=document.getElementsByName(\"enableStreamer\");\n"
				  <<      "for(var i=0;i<obj.length;i++)\n"
				  <<      "{\n"
				  <<          "obj[i].checked=all.checked"
				  <<      "}\n"
			      << "}\n";
		responser << "function isEnableAll()\n"
				  << "{\n"
				  <<      "var count=0;\n"
				  <<      "var obj=document.getElementsByName(\"enableStreamer\");\n"
				 // <<      "alert(obj.length);\n"
				  <<      "for(var i=0;i<obj.length;i++)\n"
				  <<      "{\n"
				  <<          "if (obj[i].checked==true)\n"
				  <<          "{\n"
				  <<              "count++;\n"
				  <<          "}\n"
				  <<      "}\n"
				  <<      "if(count!=obj.length)\n"
				  <<      "{\n"
				  <<           "document.getElementById(\"all\").checked=false;\n"
				  <<      "}\n"
				  <<       "else\n"
				  <<      "{\n"
				  <<           "document.getElementById(\"all\").checked=true;\n"
				  <<      "}\n"
				  << "}\n";
		responser << "function clearSearch()\n"
			      << "{\n"
				  <<       "document.getElementById(\"searchname\").value=\"\";\n"
				  << "}\n";

		// add for AJAX


        responser << "document.getElementById('toolbar').innerHTML = toolbar.build();\n";
        responser << "</script>\n";

		long availStreamCount = 0;
		int64 remoteSetupCount = 0; //setup count for remote asset session
		int64 totalSetupCount = 0; 
		NGOD::NgodUsage::const_iterator itUsage = sopUsages.begin();
		for(  ; itUsage != sopUsages.end() ; itUsage ++ )
		{
			const ::NGOD::StreamerUsageS& streamerInfos = itUsage->second.streamerUsageInfo;			
			::NGOD::StreamerUsageS::const_iterator itStreamer = streamerInfos.begin( );
			for(  ;  itStreamer != streamerInfos.end() ; itStreamer++ )
			{
				availStreamCount += itStreamer->usedStreamCount;
				remoteSetupCount += itStreamer->histCountRemoteSess;
				totalSetupCount	 += itStreamer->histCountTotalSess;
			}
		}
		
		char szPercentage[32] = "n/a";
		if(totalSetupCount > 0 )
		{
			float percentage = 0.0f;
			percentage = (float)( totalSetupCount - remoteSetupCount )/(float)totalSetupCount;
			sprintf(szPercentage,"%6.3f%%",(percentage*100.0f));
		}
		

		std::string endpoint = "Ngod2View:";
		endpoint += _varMap[Ngod2BindAddressKey];

		responser << "<form id=\"searchform\" method=\"get\" action=\"" << _reqCtx->GetUri() << "\" style=\"display:inline\">\n";
		responser<<"<H2>Summary</H2>"
			<<"<TABLE class=listTable>"
			<<"<TBODY>"
			<<"<TR><TH align=right class=heading colSpan = 2 >SSM_OSTR</TH><TD>"<< endpoint <<"</TD></TR>"
			<<"<TR><TH align=left class=heading rowSpan=4>Session<br>Stat.</TH><TH align=right class=heading>total</TH><TD>"<<totalSetupCount<<"</TD></TR>"			
			<<"<TR><TH align=right class=heading>local</TH><TD>"<<( totalSetupCount- remoteSetupCount ) <<"</TD></TR>"
			<<"<TR><TH align=right class=heading>hitrate</TH><TD>"<< szPercentage <<"</TD></TR>"
			<<"<TR><TH align=right class=heading>since</TH><TD>"<< strMeasuredSince <<"</TD></TR>"
			<<"</TBODY></TABLE><BR><BR>";

		/*
		<H2>SOP Usage</H2>
		<TABLE class=listTable>
		<TBODY>
		<TR class=heading>
		<TH align=middle vAlign=center rowSpan=2>SopName</TH>
		<TH align=middle vAlign=center rowSpan=2>Streamer NetId</TH>
		<TH align=middle vAlign=center rowSpan=2>StreamService</TH>
		<TH align=middle vAlign=center rowSpan=2>Status</TH>
		<TH align=middle vAlign=center rowSpan=2>Penalty</TH>
		<TH align=middle colSpan=2>Bandwidth Cap.</TH>
		<TH align=middle colSpan=2>Stream Cap.</TH>
		<TH align=middle colSpan=2>Session Stat.</TH>
		</TR>
		<TR class=heading>
		<TH align=middle>used</TH>
		<TH align=middle>max</TH>
		<TH align=middle>running</TH>
		<TH align=middle>max</TH>
		<TH align=middle>remote</TH>
		<TH align=middle>total</TH> </TR>
		*/

		const char* category = _reqCtx->GetRequestVar("category");
		category = category ? category : "SOP";
		std::string strCategory = category;

		const char* searchName = _reqCtx->GetRequestVar("searchname");
		searchName = searchName ? searchName : "";
		std::string strSearchName = searchName;
		if (strSearchName != "") // erase empty space from strSearchName 
		{
			size_t nFirst = strSearchName.find_first_not_of(' ');
			if (nFirst != std::string::npos)
			{
				strSearchName = strSearchName.substr(nFirst, strSearchName.find_last_not_of(' ') - nFirst + 1);
			}
			else
			{
				strSearchName = "";
			}
		}

		std::map<std::string, NGOD::StreamerUsageS> filterUsage;
		itUsage = sopUsages.begin();
		for (; itUsage != sopUsages.end(); itUsage++)
		{
			const ::NGOD::StreamerUsageS& streamerInfos = itUsage->second.streamerUsageInfo;
			std::string strSopName = itUsage->first;
			if ((strCategory == "SOP") && (!strSearchName.empty()) &&(strSopName != strSearchName))
			{
				continue;
			}
			::NGOD::StreamerUsageS filterStreamers;
			filterStreamers.clear();
			::NGOD::StreamerUsageS::const_iterator itStreamer = streamerInfos.begin( );
			for (; itStreamer != streamerInfos.end(); itStreamer ++)
			{
				if (strCategory == "Node" && !strSearchName.empty())
				{
					size_t npos = (itStreamer->streamerNetId).find("/");
					std::string strNode = (itStreamer->streamerNetId).substr(0, npos);
					if (strNode == strSearchName)
					{
						filterStreamers.push_back(*itStreamer);
					}
					continue;
				}
				if (strCategory == "Status" && !strSearchName.empty())
				{
					std::transform(strSearchName.begin(), strSearchName.end(), strSearchName.begin(), &strTolower<char>);
					int maintenanceEnable = 2; // no match condition
					if (strSearchName == "disable")
					{
						maintenanceEnable = 0;
						if (itStreamer->maintenanceEnable == maintenanceEnable)
						{
							filterStreamers.push_back(*itStreamer);
						}
						continue;
					}
					if (strSearchName == "enable")
					{
						maintenanceEnable = 1;
						if (itStreamer->maintenanceEnable == maintenanceEnable)
						{
							filterStreamers.push_back(*itStreamer);
						}
						continue;
					}
					int available = 2;
					if (strSearchName == "unavail")
					{
						available = 0;
						if (itStreamer->maintenanceEnable ==1 && itStreamer->available == available)
						{
							filterStreamers.push_back(*itStreamer);
						}
						continue;
					}
					if (strSearchName == "avail")
					{
						available = 1;
						if (itStreamer->maintenanceEnable ==1 && itStreamer->available == available)
						{
							filterStreamers.push_back(*itStreamer);
						}
						continue;
					}
					continue;
				}
				filterStreamers.push_back(*itStreamer);
			} // end  for streamer
			filterUsage.insert(std::make_pair(strSopName, filterStreamers));
		} // end for sop


		responser << "<H2>SOP Status & Statictics</H2>"
			<< "<div>&nbsp&nbsp\n"
			<< "<select name=\"category\" id=\"category\" onchange=\"clearSearch()\">\n"
			<< "<option value=\"SOP\" " ;
		if (0 == strcmp(category, "Node"))
		{
			responser << "selected=\"selected\"";
		}
		responser << ">SOP</option>\n"
			<< "<option value=\"Node\" " ;
		if (0 == strcmp(category, "Node"))
		{
			responser << "selected=\"selected\"";
		}
		responser << ">Node</option>\n"
			<< "<option value=\"Status\" " ;
		if (0 == strcmp(category, "Status"))
		{
			responser << "selected=\"selected\"";
		}
		responser << ">Status</option>\n"
			<< "</select>\n";
		responser << "<input type=\"text\" name=\"searchname\" id=\"searchname\" value =\""<< strSearchName <<"\"size=\"50\" />&nbsp&nbsp\n";
		responser << "<input type=\"submit\" name=\"search\" id=\"search\" value=\"search\"/>&nbsp&nbsp\n"
			<< "<input type=\"checkbox\" name=\"all\" id=\"all\" value=\"all\" onclick=\"selectAll()\"/>all&nbsp&nbsp\n"
			<< "<input type=\"button\" value=\"enable\" onclick=\"enableStreamers(1)\"/>&nbsp&nbsp\n"
			<< "<input type=\"button\" value=\"disable\" onclick=\"enableStreamers(0)\"/><br /><br />\n"
			<< "<input type=\"hidden\" name=\"isenable\" id=\"isenable\" value=\"\" />\n"
			<< "<input type=\"hidden\" name=\"streamerNames\" id=\"streamerNames\" value=\"\" />\n"
			<< "</select>\n"
			<< "</div>\n"
		//	<< "<TABLE border=\"1\" cellpadding=\"0\" cellspacing=\"0\"><TBODY><TR class=heading>"
		    << "<TABLE class=\"chunk listTable\">"
			<< "<TBODY>"
			<< "<TR class=heading>"
			<<"<TH align=center rowSpan=2>SOP</TH>"
			<<"<TH align=middle vAlign=center rowSpan=2 colSpan=2>Streamer NetId</TH>"
            <<"<TH align=middle vAlign=center rowSpan=2>StreamService</TH>"
            <<"<TH align=middle vAlign=center rowSpan=2>Volume</TH>"
			<<"<TH align=middle vAlign=center rowSpan=2>Status</TH>"
			<<"<TH align=middle vAlign=center rowSpan=2>Penalty</TH>"
			<<"<TH align=middle rowSpan=2>Used/Failed/Err%.</TH>"
			<<"<TH align=middle colSpan=2>Bandwidth Cap.</TH>"
			<<"<TH align=middle colSpan=2>Stream Cap.</TH>"
			<<"<TH align=middle colSpan=2>Session Stat.</TH>"
			<<"</TR>"
			<<"<TR class=heading>"
			<<"<TH align=middle>used</TH>"
			<<"<TH align=middle>max</TH>"
			<<"<TH align=middle>active</TH>"
			<<"<TH align=middle>max</TH>"
			<<"<TH align=middle>local</TH>"
			<<"<TH align=middle>total</TH>"
			<<"</TR>";

		// display after filter table
		std::map<std::string, NGOD::StreamerUsageS>::iterator filterItUsage = filterUsage.begin();
		for (; filterItUsage != filterUsage.end(); filterItUsage++)
		{
			std::string strSopName = filterItUsage->first;
			bool bFirstLine = true;
			int		iStreamerCount = (int)filterItUsage->second.size();
			::NGOD::StreamerUsageS::const_iterator itStreamer = filterItUsage->second.begin( );
			for ( ; itStreamer != filterItUsage->second.end() ; itStreamer ++ )
			{
				if( bFirstLine )
				{//sopname
					responser<< "	<tr> <td valign=\"center\" align=\"center\" rowspan="<<iStreamerCount<<">"<<filterItUsage->first<<"</td>";
					bFirstLine = false;
				}
				else
				{
					responser<< "	<tr>";
				}
				char delimeter = 0x06;
				std::string strFullName = filterItUsage->first + delimeter + itStreamer->streamerNetId;
				responser<<"	" 
					<< "<td align = middle><input type=\"checkbox\" name=\"enableStreamer\" id=\"enableStreamer\" onclick=\"isEnableAll()\" value=\"" << strFullName <<"\" /><centry>&nbsp&nbsp&nbsp&nbsp&nbsp</centry></td>\n"
					<<	"<td align = middle>"<<itStreamer->streamerNetId << "</td>"
					<<  "<td align = middle>"<<itStreamer->streamerEndpoint << "</td>"
                    << "<td align=middle>" << itStreamer->attachedVolumeName << "</td>";
				if (itStreamer->maintenanceEnable > 0)
				{
					responser <<  "<td align = middle>"<< ((itStreamer->available == 1) ? "avail" : "unavail" ) << "</td>";
				}
				else
				{
					responser <<  "<td align = middle>disable</td>";
				}
				uint64 nTotoalSession = itStreamer->usedSession + itStreamer->failedSession;
				float dwErr = 0.0;
				char errorPercent[64];
				if (nTotoalSession != 0)
				{
					dwErr = (float)(itStreamer->failedSession) / nTotoalSession;
					dwErr *= 100;

				}
				sprintf(errorPercent, "%4.1f", dwErr);
				
				responser<<  "<td align = middle>"<<itStreamer->penaltyValue << "</td>"
					<<  "<td align = middle>"<< itStreamer->usedSession << "/"  
					                         <<  itStreamer->failedSession<< "/"
											 << errorPercent << "</td>"

					<<  "<td align = middle>"<<itStreamer->usedBandwidth << "</td>"
					<<  "<td align = middle>"<<itStreamer->totalBandwidth << "</td>"						
					<<  "<td align = middle>"<<itStreamer->usedStreamCount << "</td>" 
					<<  "<td align = middle>"<<itStreamer->maxStreamCount << "</td>"						
					<<  "<td align = middle>"<< (itStreamer->histCountTotalSess - itStreamer->histCountRemoteSess) << "</td>" 
					<<  "<td align = middle>"<<itStreamer->histCountTotalSess << "</td>" 

					<<	"</tr>";
			}
		}
		responser << "</table>\n";

		if( importChannelUsages.size() > 0 )
		{

			responser<<"<BR><BR><BR>\n"<<"<H2>C2 Transfer Interface Status</H2>";
			//add import channel usage		
			responser <<"<TABLE class=listTable><TBODY>\n"
				<<"\t<TR class=heading>\n"
				<<"\t<TH align=middle>Name</TH>\n"
				<<"\t<TH align=middle>Sessions</TH>\n"
				<<"\t<TH align=middle>Used BW</TH>\n"
                <<"\t<TH align=middle>Total BW</TH>\n"
                <<"\t<TH align=middle>Status</TH>\n"
				<<"\t</TR>\n";
			//NGOD::ImportChannelUsageS importChannelUsages
			NGOD::ImportChannelUsageS::const_iterator itImportChannel = importChannelUsages.begin();
			for( ; itImportChannel != importChannelUsages.end() ; itImportChannel++ )
			{
				responser<<"\t<TR>"
					<<"<td align = middle>"<< itImportChannel->channelName << "</td>"
					<<"<td align = middle>"<< itImportChannel->runningSessCount << "</td>"
					<<"<td align = middle>"<< itImportChannel->usedImportBandwidth << "</td>"
                    <<"<td align = middle>"<< itImportChannel->totalImportBandwidth << "</td>"
                    <<"<td align = middle>"<< (itImportChannel->totalImportBandwidth > itImportChannel->usedImportBandwidth ? "avail" : "unavail") << "</td>"
					<<"</TR>\n";
			}

			responser << "</table>\n<BR>";	
		}

	}
	catch( const Ice::Exception& ex)
	{
		char szBuf[2048];
		SNPRINTF(szBuf, 
			sizeof(szBuf) - 1,
			"get ngod resource sopUsages caught  ice Exception %s: %s<br>",
			ex.ice_name().c_str());
		setLastError(szBuf);
		glog(ErrorLog, CLOGFMT(NgodPage, "%s"), getLastError());
		responser.SetLastError( getLastError() );
		return false;
	}
	responser << "<input type=\"hidden\" name=\"" << Ngod2BindAddressKey << "\" id=\"" << Ngod2BindAddressKey<< "\" value=\""<< _reqCtx->GetRequestVar(Ngod2BindAddressKey) <<"\">\n" ;
	responser << "<input type=\"hidden\" name=\"" << TemplateKey << "\" id=\"" << TemplateKey << "\" value=\""<< _reqCtx->GetRequestVar(TemplateKey) <<"\">\n" ;
	responser << "</form>\n";
	return true;
}

void NgodPage::splitString(std::vector< std::string > &result, const std::string &str, const std::string &delimiter)
{
	using namespace std;
	result.clear();
	string::size_type pos_from = 0;
	while((pos_from = str.find_first_not_of(delimiter, pos_from)) != string::npos)
	{
		string::size_type pos_to = str.find_first_of(delimiter, pos_from);
		if(pos_to != string::npos)
		{
			result.push_back(str.substr(pos_from, pos_to - pos_from));
		}
		else
		{
			result.push_back(str.substr(pos_from));
			break;
		}
		pos_from = pos_to;
	}
}

}
