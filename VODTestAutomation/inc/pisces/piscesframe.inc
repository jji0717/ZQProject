[ ] //*****************************************************************************
[ ] //  NAME:					  PISCESFRAME.INC
[ ] //
[ ] //  DESCRIPTION:		Function library for Pisces component of Zodiac project.
[ ] //
[ ] //  HISTORY:
[ ] //
[ ] //  Date						Developer					Description
[ ] //  ********				***********				*****************************************
[ ] //  01/08/03        K. Sullivan       Initial design and development.
[ ] //
[ ] //*****************************************************************************************************
[ ] 
[ ] //The default base state for the Pisces test case.  It's a generic html page that I created that 
[ ] //should sit on the Silk Test box in the Autotest directory.
[ ] 
[ ] const wMainWindow = PiscesDefaultBaseState
[ ] 
[+] window BrowserChild PiscesDefaultBaseState
	[ ] tag "Default Base State Page for Pisces"
	[ ] 
	[ ] // The URL of this page
	[ ] const sLocation = "D:/autotest/DefaultBaseStatePage.html"
	[ ] 
	[ ] // The login user name
	[ ] // const sUserName = ?
	[ ] 
	[ ] // The login password
	[ ] // const sPassword = ?
	[ ] 
	[ ] // The size of the browser window
	[ ] // const POINT BrowserSize = {600, 400}
	[ ] 
	[ ] // Sets the browser font settings to the default
	[ ] // const bDefaultFont = TRUE
	[+] HtmlHeading DefaultBaseStatePagePisces
		[ ] tag "Default Base State Page for Pisces"
[ ] 
[ ] //The main window for ITV Reports.
[+] window BrowserChild ITVReports
	[ ] tag "ITV Reports"
	[ ] 
	[ ] // The URL of this page
	[ ] const sLocation = "http://192.168.21.237/itvreport"
	[ ] 
	[ ] // The login user name
	[ ] const sUserName = "administrator"
	[ ] 
	[ ] // The login password
	[ ] const sPassword = "deadsea"
	[ ] 
	[ ] // The size of the browser window
	[ ] // const POINT BrowserSize = {600, 400}
	[ ] 
	[ ] // Sets the browser font settings to the default
	[ ] // const bDefaultFont = TRUE
	[ ] 
	[ ] 
	[+] HtmlText ITVWebReports
		[ ] tag "ITV Web Reports"
	[+] HtmlPushButton ExpandAll
		[+] multitag "Expand All"
			[ ] "#1"
	[+] HtmlPushButton HideAll
		[+] multitag "Hide All"
			[ ] "#2"
	[+] HtmlText ClickToViewReport
		[ ] tag "Click the +?- button next to each report category below to view the reports available for that category. You can click the Exp*"
	[+] HtmlText GeneralText
		[ ] tag "General"
	[+] HtmlPushButton GeneralPushButton
		[+] multitag "General"
			[ ] "#3"
	[+] HtmlText N1
		[ ] tag "+[1]$d0"
	[+] HtmlLink ContentReport
		[+] multitag "Content Report"
			[ ] "#1"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[1]"
	[+] HtmlText DisplaysASummaryText
		[ ] tag "Displays a summary of the available ITV content."
	[+] HtmlLink SystemSummaryReport
		[+] multitag "System Summary Report"
			[ ] "#2"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[2]"
	[+] HtmlText DisplaysASummaryOfOverallText
		[ ] tag "Displays a summary of overall ITV system information."
	[+] HtmlLink TechnicalSummaryReport
		[+] multitag "Technical Summary Report"
			[ ] "#3"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[3]"
	[+] HtmlText DisplaysASummaryOfITVSystemText
		[ ] tag "Displays a summary of ITV system technical details."
	[+] HtmlText AccountingText
		[ ] tag "Accounting"
	[+] HtmlPushButton AccountingPushButton
		[+] multitag "Accounting"
			[ ] "#4"
	[+] HtmlText N2
		[ ] tag "+[2]$d1"
	[+] HtmlLink AccountingBuyRateReport
		[+] multitag "Accounting Buy Rate Report"
			[ ] "#4"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[4]"
	[+] HtmlText PresentsInformationAboutAsset
		[ ] tag "Presents information about asset purchases by title, along with revenue and royalty splits.."
	[+] HtmlLink InternalAccountingReport
		[+] multitag "Internal Accounting Report"
			[ ] "#5"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[5]"
	[+] HtmlText PresentsInformationAboutRevenue
		[ ] tag "Presents information about revenue and margin summaries on a per-studio basis."
	[+] HtmlText SubscriptionText
		[ ] tag "Subscription"
	[+] HtmlPushButton SubscriptionPushButton
		[+] multitag "Subscription"
			[ ] "#5"
	[+] HtmlText N3
		[ ] tag "+[3]$d2"
	[+] HtmlLink SubscriberViewsReport
		[+] multitag "Subscriber Views Report"
			[ ] "#6"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[6]"
	[+] HtmlText DisplaysAListingOfSubs1
		[ ] tag "Displays a listing of subscription assets viewed by each subscriber."
	[+] HtmlLink SubscriptionAssetViewsReport
		[+] multitag "Subscription Asset Views Report"
			[ ] "#7"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[7]"
	[+] HtmlText DisplaysAListingOfSubs2
		[ ] tag "Displays a listing of subscription assets and the number of times they were viewed."
	[+] HtmlLink SubscriptionReport
		[+] multitag "Subscription Report"
			[ ] "#8"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[8]"
	[+] HtmlText DisplaysASummaryOfAssetViews
		[ ] tag "Displays a summary of asset views by subscription."
	[+] HtmlText MoviesOnDemandText
		[ ] tag "Movies On Demand"
	[+] HtmlPushButton MoviesOnDemandPushButton
		[+] multitag "Movies On Demand"
			[ ] "#6"
	[+] HtmlText N4
		[ ] tag "+[4]$d3"
	[+] HtmlLink MonthlyBuyRateAnalysis
		[+] multitag "Monthly Buy Rate Analysis"
			[ ] "#9"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[9]"
	[+] HtmlText DisplaysMonthlySummaryText
		[ ] tag "Displays monthly summary information about asset purchases by title, broken out by day of the week."
	[+] HtmlLink MoviesOrderedReport
		[+] multitag "Movies Ordered Report"
			[ ] "#10"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[10]"
	[+] HtmlText DisplaysAGridText
		[ ] tag "Displays a grid showing the number of assets purchased during specific times of the day."
	[+] HtmlLink StudioReport
		[+] multitag "Studio Report"
			[ ] "#11"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[11]"
	[+] HtmlText DisplaysASummaryOfAssetPurch
		[ ] tag "Displays a summary of asset purchases, organized by studio."
	[+] HtmlLink WeeklyBuyRateAnalysis
		[+] multitag "Weekly Buy Rate Analysis"
			[ ] "#12"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[12]"
	[+] HtmlText DisplaysWeeklySummaryInfo
		[ ] tag "Displays weekly summary information about asset purchases by title, broken out by day of the week."
	[+] HtmlText VideoStreamAnalysisText
		[ ] tag "Video Stream Analysis"
	[+] HtmlPushButton VideoStreamAnalysisPushButton
		[+] multitag "Video Stream Analysis"
			[ ] "#7"
	[+] HtmlText N5
		[ ] tag "+[5]$d4"
	[+] HtmlLink BusyErrorsReport
		[+] multitag "Busy Errors Report"
			[ ] "#13"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[13]"
	[+] HtmlText DisplaysErrorsGeneratedWhenText
		[ ] tag "Displays errors generated when a request for an asset was not fulfilled."
	[+] HtmlLink ITVErrorsReport
		[+] multitag "ITV Errors Report"
			[ ] "#14"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[14]"
	[+] HtmlText DisplaysErrorInformationText
		[ ] tag "Displays error information that can be useful in diagnosing QAM and networking issues. Based on the ItvErrors.SQL script avail*"
	[+] HtmlLink StreamUtilizationReport
		[+] multitag "Stream Utilization Report"
			[ ] "#15"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[15]"
	[+] HtmlText DisplaysThePeakStreamUsage1
		[ ] tag "Displays the peak stream usage for the specified date range."
	[+] HtmlLink StreamUtilizationReportBy
		[+] multitag "Stream Utilization Report - By Node Group"
			[ ] "#16"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[16]"
	[+] HtmlText DisplaysThePeakStreamUsage2
		[ ] tag "Displays the peak stream usage for the specified date range, broken down by node group."
	[+] HtmlLink StreamingErrorsDetailReport
		[+] multitag "Streaming Errors Detail Report"
			[ ] "#17"
			[ ] //"$http:??192.168.21.237?itvreport?default.asp?ITVRPT_REPORT_CMD=S[17]"
	[+] HtmlText DisplaysAListingOfAllFail
		[ ] tag "Displays a listing of all failed streaming requests."
	[ ] 
[ ] //If the dialog box comes up for the user to enter a username and password.
[+] window DialogBox EnterNetworkPassword
	[ ] tag "Enter Network Password"
	[ ] parent Browser
	[+] CustomWin Icon1
		[ ] msw tag "[Icon]$-1"
	[+] StaticText PleaseTypeYourUserNameAnd
		[+] multitag "Please type your user name and password."
			[ ] "$-1"
	[+] StaticText SiteText
		[+] multitag "Site:"
			[ ] "$1042"
	[+] StaticText N19216821237Text
		[+] multitag "192.168.21.237"
			[ ] "$1041"
	[+] StaticText UserNameText
		[+] multitag "User Name"
			[ ] "$1046"
	[+] TextField UserName
		[+] multitag "User Name"
			[ ] "$1218"
	[+] StaticText PasswordText
		[+] multitag "Password"
			[ ] "$1047"
	[+] TextField Password
		[+] multitag "Password"
			[ ] "$1219"
	[+] StaticText DomainText
		[+] multitag "Domain"
			[ ] "$1043"
	[+] TextField Domain
		[+] multitag "Domain"
			[ ] "$1010"
	[+] CheckBox SaveThisPasswordInYourPas
		[+] multitag "Save this password in your password list"
			[ ] "$1024"
	[+] PushButton OK
		[+] multitag "OK"
			[ ] "$1"
	[+] PushButton Cancel
		[+] multitag "Cancel"
			[ ] "$2"
[ ] 
[ ] //The Accounting Buy Rate Report Parameters page.
[+] window BrowserChild AcctBuyRateRptParam
	[ ] tag "Accounting Buy Rate Report"
	[ ] parent Browser
	[+] HtmlText A
		[ ] tag "a"
	[+] HtmlText ReportParametersAccounting
		[ ] tag "Report Parameters - Accounting Buy Rate Report"
	[+] HtmlText EnterTheParametersForTheR
		[ ] tag "Enter the parameters for the report you are generating below."
	[+] HtmlText ForTheDateRangeEnterThe
		[ ] tag "For the date range, enter the dates in the format MM?DD?YY, and ensure the dates you enter fall in the valid date range shown*"
	[+] HtmlText CertainReportsEnableYouTo
		[ ] tag "Certain reports enable you to limit the scope of the report by selecting a specific genre or studio, rather than generating yo*"
	[+] HtmlText IfTheReportYouSelectedAll
		[ ] tag "If the report you selected allows, you can choose to group the data presented by genre or studio or rating, rather than the de*"
	[+] HtmlText WhenYouHaveFinishedText
		[ ] tag "When you have finished entering the desired parameters, click Run Report. Note that some reports may take several minutes to g*"
	[+] HtmlText Parameter
		[ ] tag "Parameter"
	[+] HtmlText Values
		[ ] tag "Values"
	[+] HtmlText Description
		[ ] tag "Description"
	[+] HtmlHidden WhenYouHaveFinishedHidden1
		[+] multitag "When you have finished entering the desired parameters, click Run Report. Note that some reports may take several minutes to g*[2]"
			[ ] "#2"
			[ ] "$ITVRPT_REPORT_CMD"
	[+] HtmlHidden WhenYouHaveFinishedHidden2
		[+] multitag "When you have finished entering the desired parameters, click Run Report. Note that some reports may take several minutes to g*[1]"
			[ ] "#1"
			[ ] "$ITVRPT_REPORT_ID"
	[+] HtmlText DateRangeFromText
		[ ] tag "Date Range:"
	[+] HtmlTextField DateRangeFromField
		[+] multitag "Date Range:"
			[ ] "#1"
			[ ] "$ITVRPT_START_DATE"
	[+] HtmlText DateRangeToText
		[ ] tag "to"
	[+] HtmlTextField DateRangeToField
		[+] multitag "to"
			[ ] "#2"
			[ ] "$ITVRPT_END_DATE"
	[+] HtmlText RequiredValidDatesText
		[ ] tag "(Required) Valid dates are 7?17?02 to 12?31?02"
	[+] HtmlText LimitToGenreText
		[ ] tag "Limit to Genre:"
	[+] HtmlPopupList LimitToGenrePopup
		[+] multitag "Limit to Genre:"
			[ ] "#1"
			[ ] "$ITVRPT_MBS_GENRE_ID"
	[+] HtmlText SelectAllGenresText
		[ ] tag "Select All Genres to display all data. Select a single genre to limit the report’s scope to data on assets in that genre."
	[+] HtmlText LimitToStudioText
		[ ] tag "Limit to Studio:"
	[+] HtmlPopupList LimitToStudioPopup
		[+] multitag "Limit to Studio:"
			[ ] "#2"
			[ ] "$ITVRPT_MBS_CONTENT_PROVIDER_ID"
	[+] HtmlText SelectAllStudiosText
		[ ] tag "Select All Studios to display all data. Select a single studio to limit the report’s scope to data on assets in from that stud*"
	[+] HtmlText LimitToProviderCodeText
		[ ] tag "Limit to Provider Code:"
	[+] HtmlPopupList LimitToProviderCodePopup
		[+] multitag "Limit to Provider Code:"
			[ ] "#3"
			[ ] "$ITVRPT_PROVIDER_CODE"
	[+] HtmlText SelectAllProvidersCodesTo
		[ ] tag "Select All Providers Codes to display all data. Select a single provider to limit the report’s scope to data on assets in fro*"
	[+] HtmlPushButton RunReport
		[+] multitag "Run Report"
			[ ] "#1"
			[ ] "$ITVRPT_SUBMIT_CMD"
[ ] 
[ ] //The page that displays the actual Accounting Buy Rate Report.  Contains mostly custom windows.
[+] window BrowserChild AcctBuyRateRptDisplay
	[ ] tag "Accounting Buy Rate Report"
	[ ] parent Browser
	[+] CustomWin ATL600588E01
		[ ] msw tag "[ATL:600588E0]$105409976"
		[+] CustomWin ATLSTATIC1
			[ ] msw tag "[ATL:STATIC]$0"
			[+] CustomWin ATL600582201
				[ ] msw tag "[ATL:60058220]$120"
				[+] CustomWin ATL600582804
					[ ] msw tag "[ATL:60058280]$121"
					[+] CustomWin SysAnimate321
						[ ] msw tag "[SysAnimate32]$20004"
				[+] CustomWin ATL600582803
					[ ] msw tag "[ATL:60058280]$249"
				[+] CustomWin ATL600582802
					[ ] msw tag "[ATL:60058280]$247"
					[+] CustomWin N1
						[+] msw multitag "[ATL:60059138]1"
							[ ] "[ATL:60059138]$20005"
					[+] StaticText Of1Text
						[+] multitag "of 1"
							[ ] "$20003"
				[+] CustomWin ATL600582801
					[ ] msw tag "[ATL:60058280]$203"
					[+] CustomWin N100
						[+] msw multitag "[ATL:COMBOBOX]100%"
							[ ] "[ATL:COMBOBOX]$20000"
						[+] TextField TextField1
							[ ] tag "$1001"
		[+] CustomWin ReportAlbum1
			[ ] msw tag "[ReportAlbum]$1000"
			[+] PageList PageList1
				[ ] tag "$0"
			[+] CustomWin ViewHolder1
				[ ] msw tag "[ViewHolder]$105447456"
				[+] TreeView TreeView1
					[ ] tag "$20004"
				[+] CustomWin ReportView1
					[ ] msw tag "[ReportView]$105455472"
	[+] HtmlPushButton RunAnotherReport
		[+] multitag "Run Another Report"
			[ ] "#1"
			[ ] "$button1"
	[+] HtmlPushButton RunThisReportAgain
		[+] multitag "Run This Report Again"
			[ ] "#2"
			[ ] "$button2"
	[+] HtmlPushButton ExportReport
		[+] multitag "Export Report"
			[ ] "#3"
			[ ] "$button3[1]"
	[+] HtmlPushButton PrintReport
		[+] multitag "Print Report"
			[ ] "#4"
			[ ] "$button3[2]"
	[+] CustomWin CRViewer
		[ ] msw tag "[]$CRViewer"
[ ] 
[ ] //The "Export This Page" page for all reports.
[+] window BrowserChild ExportReport
	[ ] tag "http:??192.168.21.237?itvreport?ExportForm.asp - Microsoft Internet Explorer"
	[ ] parent Browser
	[+] HtmlHeading ExportThisReport
		[ ] tag "Export This Report"
	[+] HtmlText TheExportReportPageEnables
		[ ] tag "The Export Report page enables you to choose a format for exporting the current report."
	[+] HtmlText ClickTheDesiredReportFormat
		[ ] tag "Click the desired report format below."
	[+] HtmlText IfTheAppText1
		[ ] tag "If the application associated with that format is available on your system, the file will open in the target application. Afte*"
	[+] HtmlText IfTheAppText2
		[ ] tag "If the application associated with that format is not available your system, you will be prompted to save the report to a fil*"
	[+] HtmlTable ExportTable
		[ ] tag "If the application associated with that format is not available your system, you will be prompted to save the report to a fil*"
		[+] HtmlColumn Link
			[ ] tag "Link"
			[+] HtmlLink AcctBuyRateRptTXT1
				[+] multitag "Accounting Buy Rate Report.TXT[1]"
					[ ] "#1"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=5"
			[+] HtmlLink AcctBuyRateRptDIF
				[+] multitag "Accounting Buy Rate Report.DIF"
					[ ] "#2"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=2[1]"
			[+] HtmlLink AcctBuyRateRptXLS1
				[+] multitag "Accounting Buy Rate Report.XLS[1]"
					[ ] "#3"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=2[2]"
			[+] HtmlLink AcctBuyRateRptXLS2
				[+] multitag "Accounting Buy Rate Report.XLS[2]"
					[ ] "#4"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=2[3]"
			[+] HtmlLink AcctBuyRateRptXLS3
				[+] multitag "Accounting Buy Rate Report.XLS[3]"
					[ ] "#5"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=2[4]"
			[+] HtmlLink AcctBuyRateRptWK1
				[+] multitag "Accounting Buy Rate Report.WK1"
					[ ] "#6"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=1[1]"
			[+] HtmlLink AcctBuyRateRptWK3
				[+] multitag "Accounting Buy Rate Report.WK3"
					[ ] "#7"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=1[2]"
			[+] HtmlLink AcctBuyRateRptWKS
				[+] multitag "Accounting Buy Rate Report.WKS"
					[ ] "#8"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=1[3]"
			[+] HtmlLink AcctBuyRateRptRTF
				[+] multitag "Accounting Buy Rate Report.RTF"
					[ ] "#9"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=4"
			[+] HtmlLink AcctBuyRateRptTXT2
				[+] multitag "Accounting Buy Rate Report.TXT[2]"
					[ ] "#10"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=9"
			[+] HtmlLink AcctBuyRateRptTXT3
				[+] multitag "Accounting Buy Rate Report.TXT[3]"
					[ ] "#11"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=6"
			[+] HtmlLink AcctBuyRateRptDOC
				[+] multitag "Accounting Buy Rate Report.DOC"
					[ ] "#12"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=1[4]"
			[+] HtmlLink SubsAssetViewsRptTXT1
				[+] multitag "Subscription Asset Views Report.TXT[1]"
					[ ] "#1"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=5"
			[+] HtmlLink SubsAssetViewsRptDIF
				[+] multitag "Subscription Asset Views Report.DIF"
					[ ] "#2"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=2[1]"
			[+] HtmlLink SubsAssetViewsRptXLS1
				[+] multitag "Subscription Asset Views Report.XLS[1]"
					[ ] "#3"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=2[2]"
			[+] HtmlLink SubsAssetViewsRptXLS2
				[+] multitag "Subscription Asset Views Report.XLS[2]"
					[ ] "#4"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=2[3]"
			[+] HtmlLink SubsAssetViewsRptXLS3
				[+] multitag "Subscription Asset Views Report.XLS[3]"
					[ ] "#5"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=2[4]"
			[+] HtmlLink SubsAssetViewsRptWK1
				[+] multitag "Subscription Asset Views Report.WK1"
					[ ] "#6"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=1[1]"
			[+] HtmlLink SubsAssetViewsRptWK3
				[+] multitag "Subscription Asset Views Report.WK3"
					[ ] "#7"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=1[2]"
			[+] HtmlLink SubsAssetViewsRptWKS
				[+] multitag "Subscription Asset Views Report.WKS"
					[ ] "#8"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=1[3]"
			[+] HtmlLink SubsAssetViewsRptRTF
				[+] multitag "Subscription Asset Views Report.RTF"
					[ ] "#9"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=4"
			[+] HtmlLink SubsAssetViewsRptTXT2
				[+] multitag "Subscription Asset Views Report.TXT[2]"
					[ ] "#10"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=9"
			[+] HtmlLink SubsAssetViewsRptTXT3
				[+] multitag "Subscription Asset Views Report.TXT[3]"
					[ ] "#11"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=6"
			[+] HtmlLink SubsAssetViewsRptDOC
				[+] multitag "Subscription Asset Views Report.DOC"
					[ ] "#12"
					[ ] "$http:??192.168.21.237?itvreport?ExportReport.asp?ExportFormat=1[4]"
		[+] HtmlColumn Description
			[ ] tag "Description"
			[+] HtmlText CommaSeperatedValues
				[ ] tag "Comma-seperated values"
			[+] HtmlText DataInterchangeFormat
				[ ] tag "Data interchange format"
			[+] HtmlText Excel50
				[ ] tag "Excel 5.0"
			[+] HtmlText Excel70
				[ ] tag "Excel 7.0"
			[+] HtmlText Excel80
				[ ] tag "Excel 8.0"
			[+] HtmlText LotusWK1
				[ ] tag "Lotus WK1"
			[+] HtmlText LotusWK3
				[ ] tag "Lotus WK3"
			[+] HtmlText LotusWKS
				[ ] tag "Lotus WKS"
			[+] HtmlText RichText
				[ ] tag "Rich Text"
			[+] HtmlText TabSeperatedText
				[ ] tag "Tab-seperated text"
			[+] HtmlText TabSeperatedValues
				[ ] tag "Tab-seperated values"
			[+] HtmlText WordForWindows
				[ ] tag "Word for Windows"
[ ] 
[ ] //The Subscriber Views Report Parameters page.
[+] window BrowserChild SubsViewsRptParam
	[ ] tag "Subscriber Views Report"
	[ ] parent Browser
	[+] HtmlText K
		[ ] tag "K"
	[+] HtmlText ReportParametersSubscriber
		[ ] tag "Report Parameters - Subscriber Views Report"
	[+] HtmlText EnterTheParameters
		[ ] tag "Enter the parameters for the report you are generating below."
	[+] HtmlText ForTheDateRange
		[ ] tag "For the date range, enter the dates in the format MM?DD?YY, and ensure the dates you enter fall in the valid date range shown*"
	[+] HtmlText CertainReportsEnableYouTo
		[ ] tag "Certain reports enable you to limit the scope of the report by selecting a specific genre or studio, rather than generating yo*"
	[+] HtmlText IfTheReportYouSelected
		[ ] tag "If the report you selected allows, you can choose to group the data presented by genre or studio or rating, rather than the de*"
	[+] HtmlText WhenYouHaveFinishedText
		[ ] tag "When you have finished entering the desired parameters, click Run Report. Note that some reports may take several minutes to g*"
	[+] HtmlText Parameter
		[ ] tag "Parameter"
	[+] HtmlText Values
		[ ] tag "Values"
	[+] HtmlText Description
		[ ] tag "Description"
	[+] HtmlHidden WhenYouHaveFinishedHidden1
		[+] multitag "When you have finished entering the desired parameters, click Run Report. Note that some reports may take several minutes to g*[1]"
			[ ] "#1"
			[ ] "$ITVRPT_REPORT_CMD"
	[+] HtmlHidden WhenYouHaveFinishedHidden2
		[+] multitag "When you have finished entering the desired parameters, click Run Report. Note that some reports may take several minutes to g*[2]"
			[ ] "#2"
			[ ] "$ITVRPT_REPORT_ID"
	[+] HtmlText DateRangeFromText
		[ ] tag "Date Range:"
	[+] HtmlTextField DateRangeFromField
		[+] multitag "Date Range:"
			[ ] "#1"
			[ ] "$ITVRPT_START_DATE"
	[+] HtmlText DateRangeToText
		[ ] tag "to"
	[+] HtmlTextField DateRangeToField
		[+] multitag "to"
			[ ] "#2"
			[ ] "$ITVRPT_END_DATE"
	[+] HtmlText RequiredValidDatesAre
		[ ] tag "(Required) Valid dates are 1?1?70 to 1?1?70"
	[+] HtmlText LimitToPackageText
		[ ] tag "Limit to Package:"
	[+] HtmlPopupList LimitToPackagePopup
		[+] multitag "Limit to Package:"
			[ ] "#1"
			[ ] "$ITVRPT_SUB_PACKAGE_ID"
	[+] HtmlText SelectAllPackagesText
		[ ] tag "Select All Packages to display all data. Select a single subscription package to limit the report’s scope to data on assets in*"
	[+] HtmlPushButton RunReport
		[+] multitag "Run Report"
			[ ] "#1"
			[ ] "$ITVRPT_SUBMIT_CMD"
[ ] 
[ ] //The Subscription Asset Views Report Parameters page.
[+] window BrowserChild SubsAssetViewsRptParam
	[ ] tag "Subscription Asset Views Report"
	[ ] parent Browser
	[+] HtmlText A
		[ ] tag "a"
	[+] HtmlText ReportParametersSubscripti
		[ ] tag "Report Parameters - Subscription Asset Views Report"
	[+] HtmlText EnterTheParametersForTheR
		[ ] tag "Enter the parameters for the report you are generating below."
	[+] HtmlText ForTheDateRangeEnterThe
		[ ] tag "For the date range, enter the dates in the format MM?DD?YY, and ensure the dates you enter fall in the valid date range shown*"
	[+] HtmlText CertainReportsEnableYouTo
		[ ] tag "Certain reports enable you to limit the scope of the report by selecting a specific genre or studio, rather than generating yo*"
	[+] HtmlText IfTheReportYouSelectedAll
		[ ] tag "If the report you selected allows, you can choose to group the data presented by genre or studio or rating, rather than the de*"
	[+] HtmlText WhenYouHaveFinishedEnterin1
		[ ] tag "When you have finished entering the desired parameters, click Run Report. Note that some reports may take several minutes to g*"
	[+] HtmlText Parameter
		[ ] tag "Parameter"
	[+] HtmlText Values
		[ ] tag "Values"
	[+] HtmlText Description
		[ ] tag "Description"
	[+] HtmlHidden WhenYouHaveFinishedEnterin2
		[+] multitag "When you have finished entering the desired parameters, click Run Report. Note that some reports may take several minutes to g*[1]"
			[ ] "#1"
			[ ] "$ITVRPT_REPORT_CMD"
	[+] HtmlHidden WhenYouHaveFinishedEnterin3
		[+] multitag "When you have finished entering the desired parameters, click Run Report. Note that some reports may take several minutes to g*[2]"
			[ ] "#2"
			[ ] "$ITVRPT_REPORT_ID"
	[+] HtmlText DateRangeFromText
		[ ] tag "Date Range:"
	[+] HtmlTextField DateRangeFromField
		[+] multitag "Date Range:"
			[ ] "#1"
			[ ] "$ITVRPT_START_DATE"
	[+] HtmlText DateRangeToText
		[ ] tag "to"
	[+] HtmlTextField DateRangeToField
		[+] multitag "to"
			[ ] "#2"
			[ ] "$ITVRPT_END_DATE"
	[+] HtmlText RequiredValidDatesAre11
		[ ] tag "(Required) Valid dates are 1?1?70 to 1?1?70"
	[+] HtmlText LimitToPackage1
		[ ] tag "Limit to Package:"
	[+] HtmlPopupList LimitToPackage2
		[+] multitag "Limit to Package:"
			[ ] "#1"
			[ ] "$ITVRPT_SUB_PACKAGE_ID"
	[+] HtmlText SelectAllPackagesToDisplay
		[ ] tag "Select All Packages to display all data. Select a single subscription package to limit the report’s scope to data on assets in*"
	[+] HtmlPushButton RunReport
		[+] multitag "Run Report"
			[ ] "#1"
			[ ] "$ITVRPT_SUBMIT_CMD"
[ ] 
[ ] //The page that displays the actual Subscription Asset Views Report.  Contains mostly custom windows.
[+] window BrowserChild SubsAssetViewsRptDisplay
	[ ] tag "Subscription Asset Views Report"
	[ ] parent Browser
	[+] CustomWin ATL600588E01
		[ ] msw tag "[ATL:600588E0]$79457720"
		[+] CustomWin ATLSTATIC1
			[ ] msw tag "[ATL:STATIC]$0"
			[+] CustomWin ATL600582201
				[ ] msw tag "[ATL:60058220]$120"
				[+] CustomWin ATL600582804
					[ ] msw tag "[ATL:60058280]$121"
					[+] CustomWin SysAnimate321
						[ ] msw tag "[SysAnimate32]$20004"
				[+] CustomWin ATL600582803
					[ ] msw tag "[ATL:60058280]$249"
				[+] CustomWin ATL600582802
					[ ] msw tag "[ATL:60058280]$247"
					[+] CustomWin N1
						[+] msw multitag "[ATL:60059138]1"
							[ ] "[ATL:60059138]$20005"
					[+] StaticText Of1Text
						[+] multitag "of 1"
							[ ] "$20003"
				[+] CustomWin ATL600582801
					[ ] msw tag "[ATL:60058280]$203"
					[+] CustomWin N100
						[+] msw multitag "[ATL:COMBOBOX]100%"
							[ ] "[ATL:COMBOBOX]$20000"
						[+] TextField TextField1
							[ ] tag "$1001"
		[+] CustomWin ReportAlbum
			[ ] msw tag "[ReportAlbum]$1000"
			[+] PageList PageList1
				[ ] tag "$0"
			[+] CustomWin ViewHolder
				[ ] msw tag "[ViewHolder]$79495200"
				[+] TreeView TreeView
					[ ] tag "$20004"
				[+] CustomWin ReportView
					[ ] msw tag "[ReportView]$79503216"
	[+] HtmlPushButton RunAnotherReport
		[+] multitag "Run Another Report"
			[ ] "#1"
			[ ] "$button1"
	[+] HtmlPushButton RunThisReportAgain
		[+] multitag "Run This Report Again"
			[ ] "#2"
			[ ] "$button2"
	[+] HtmlPushButton ExportReport
		[+] multitag "Export Report"
			[ ] "#3"
			[ ] "$button3[1]"
	[+] HtmlPushButton PrintReport
		[+] multitag "Print Report"
			[ ] "#4"
			[ ] "$button3[2]"
	[+] CustomWin CRViewer
		[ ] msw tag "[]$CRViewer"
[ ] 
[ ] //The window used to save the exported report to the desired file type.
[+] window DialogBox SaveWebPage
	[ ] tag "Save Web Page"
	[ ] parent Browser
	[+] StaticText SaveInText
		[+] multitag "Save in:"
			[ ] "$1091"
	[+] PopupList SaveInPopup
		[+] multitag "Save in:"
			[ ] "$1137"
	[+] ListView SaveInList
		[+] multitag "Save in:"
			[ ] "$1"
	[+] StaticText FileNameText
		[+] multitag "File name:"
			[ ] "$1090"
	[+] TextField FileName
		[+] multitag "File name:"
			[ ] "$1152"
	[+] StaticText SaveAsTypeText
		[+] multitag "Save as type:"
			[ ] "$1089"
	[+] PopupList SaveAsType
		[+] multitag "Save as type:"
			[ ] "$1136"
	[+] PushButton Save
		[+] multitag "Save"
			[ ] "$1"
	[+] PushButton Cancel
		[+] multitag "Cancel"
			[ ] "$2"
	[+] PushButton SaveIn1
		[+] multitag "Save in:[1]"
			[ ] "$40961"
	[+] PushButton SaveIn2
		[+] multitag "Save in:[2]"
			[ ] "$40962"
	[+] RadioList SaveInRadioList
		[+] multitag "Save in:"
			[ ] "$40963"
	[+] DialogBox DialogBox1
		[ ] tag "$0"
		[+] StaticText EncodingText
			[+] multitag "Encoding:"
				[ ] "$65535"
		[+] PopupList Encoding
			[+] multitag "Encoding:"
				[ ] "$13146"
		[+] CustomWin NativeFontCtl1
			[ ] msw tag "[NativeFontCtl]$65535"
	[+] CustomWin Icon1
		[ ] msw tag "[Icon]$13315"
	[+] PushButton Yes
		[+] multitag "Yes"
			[ ] "$6"
	[+] PushButton No
		[+] multitag "No"
			[ ] "$7"
	[+] StaticText ThisWebPageMayNotSaveText
		[+] multitag "This web page may not save correctly. Would you like to save it anyway?"
			[ ] "$13313"
	[+] CheckBox NeverAskMeAgain
		[+] multitag "Never ask me again"
			[ ] "$13314"
	[ ] 
[ ] 
[ ] //This window pops up with the A: drive frequently when one tries to save a file.
[+] window DialogBox DriveNotAccessible
	[ ] tag "Save Web Page"
	[ ] parent SaveWebPage
	[+] PushButton Retry
		[+] multitag "Retry"
			[ ] "$4"
	[+] PushButton Cancel
		[+] multitag "Cancel"
			[ ] "$2"
	[+] CustomWin Icon1
		[ ] msw tag "[Icon]$10"
	[+] StaticText DriveNotAccessibleText
		[+] multitag "A:\ is not accessible.????The device is not ready."
			[ ] "$65535"
[ ] 
[ ] //This window pops up if the filename that the user enters already exists in the file path.
[+] window DialogBox FileAlreadyExists
	[ ] tag "Save Web Page"
	[ ] parent SaveWebPage
	[+] PushButton Yes
		[+] multitag "Yes"
			[ ] "$6"
	[+] PushButton No
		[+] multitag "No"
			[ ] "$7"
	[+] CustomWin Icon1
		[ ] msw tag "[Icon]$10"
	[+] StaticText FileAlreadyExistsText
		[+] multitag "C:\AcctBuyRate_TXT.txt already exists.?Do you want to replace it?"
			[ ] "$65535"
[ ] 
[ ] //This box comes up if it's the first time running the Report Generator on the PC.  This will install
[ ] //the Crystal Reports plug-in (or something like that).
[+] window DialogBox SecurityWarning
	[ ] tag "Security Warning"
	[ ] parent Browser
	[+] PushButton No
		[+] multitag "No"
			[ ] "$7"
	[+] PushButton MoreInfo
		[+] multitag "More Info"
			[ ] "$5008"
	[+] TextField TextField1
		[ ] tag "$5011"
	[+] TextField TextField2
		[ ] tag "$5012"
	[+] TextField TextField3
		[ ] tag "$5013"
	[+] TextField TextField5
		[ ] tag "$5015"
	[+] PushButton Yes
		[+] multitag "Yes"
			[ ] "$6"
	[+] CheckBox AlwaysTrustSoftwareFrom1
		[+] multitag "Always trust software from %1"
			[ ] "$5020"
	[+] CustomWin BitMap1
		[ ] msw tag "[BitMap]$5023"
	[+] CustomWin EtchedHorz1
		[ ] msw tag "[EtchedHorz]$5022"
	[+] TextField TextField4
		[ ] tag "$5014"
[ ] 
[ ] //This is an error that shows up if the target machine running ITV Reports is out of disk space.
[+] window BrowserChild NotEnoughStorage
	[ ] tag "http:??192.168.21.237?itvreport?default.asp - Microsoft Internet Explorer"
	[ ] parent Browser
	[+] HtmlText MicrosoftOLEDBProviderFor
		[ ] tag "Microsoft OLE DB Provider for SQL Server"
	[+] HtmlText NotEnoughStorageIsAvail
		[ ] tag "Not enough storage is available to complete this operation."
	[+] HtmlText ItvreportDefaultAspLine5
		[ ] tag "?itvreport?default.asp, line 59"
[ ] 
[ ] //This error will come up when the main page of ITV Reports can't be displayed. To fix the problem
[ ] //the tester would have to troubleshoot the Data Warehouse box.
[+] window BrowserChild OpenReportsError
	[ ] tag "http:??192.168.21.237?itvreport? - Microsoft Internet Explorer"
	[ ] parent Browser
	[+] HtmlText ItvReportsOpenReportsText
		[ ] tag "ItvReports::OpenReports"
	[+] HtmlText MethodOfObjectFailedText
		[ ] tag "Method '~' of object '~' failed"
	[+] HtmlText ItvreportDefaultAspLine3
		[ ] tag "?itvreport?Default.asp, line 33"
[ ] 
[ ] //This error will come up if one tries to access ITV Reports with the wrong login or password.
[+] window BrowserChild Http19216821237Itvrepor
	[ ] tag "http:??192.168.21.237?itvreport?default.asp - Microsoft Internet Explorer"
	[ ] parent Browser
	[+] HtmlText ItvReportsOpenReports
		[ ] tag "ItvReports::OpenReports"
	[+] HtmlText MicrosoftODBCSQLServerDr
		[ ] tag "?Microsoft??ODBC SQL Server Driver??SQL Server?Cannot open database requested in login 'IdwData'. Login fails."
	[+] HtmlText ItvreportDefaultAspLine3
		[ ] tag "?itvreport?default.asp, line 33"
[ ] 
