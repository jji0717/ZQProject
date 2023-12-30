//require(['app/Utils/reader','app/Utils/http','app/Utils/tabler','app/Utils/draw'],function(reader,httper,tabler,drawer){
//require(['reader','http','tabler','draw'],function(reader,httper,tabler,drawer){
require(['js/app/Utils/reader','js/app/Utils/http','js/app/Utils/tabler','js/app/Utils/draw'],function(reader,httper,tabler,drawer){
	//table 表格系列
	var tab = '{"icTable": {"icIndex": [6,2,3],"icIndex2": [2,1,5],"icChannelName": ["SEACnnnnnNx_PG","SEACnnnnnNy_PG","haha"],"icUsedBandwidth": [0,9,3],"icTotalBandwidth": [0,7,4],"icRunningSessCount": [6,0,4],"icStatus": ["n/a","n/a","hehe"]}}';

	
	//测试自定义数据 绘制表格
	var Table = new tabler.table("tableId",tab);
	Table.create();
	Table.search("SEACnnnnnNy_PG");
	/*
	//测试连接TianShan中的服务 绘制表格
	var Http1 = new httper.http({way:"svar",service:"rtspProxy",varname:"icTable"});
	Http1.GET(function(data){
	console.log((new Date).getTime() + " response " + ": " + data);
	var Table1 = new tabler.table("tableId2",data);
	Table1.create();
	Table1.search("SEACnnnnnNy_PG");});
	*/
	
	var Table2 = new tabler.table("tableId2");
	Table2.create();
	var Http2 = new httper.http({way:"mvar",service:"nic"},"192.168.81.71:10080");
	var timer2 = setInterval(function(){
		Http2.GET(function(data){
			console.log((new Date).getTime() + " response " + ": " + data);
			Table2.update(data[0]);
		},
		function(reason){//失败处理，关闭update定时器			
			console.log("fail: " + reason + "  timer will be close");
			clearInterval(timer2);
		});
	},1000);
	
	/*
	//测试读取csv变量绘制表格
	var csv = 'sessId,byteoffset,timeoffset,duration\n"JSNBF",4421342,4823,3214\n"D3sDLO",423424423,53523,424253453';
	var TableCsv = new tabler.table("tableId3",csv);
	TableCsv.create();
	*/
	//测试读取csv文件绘制表格
	var csv;
	var Httpcsv = new httper.http({way:"fvar",service:"data/kk",varname:"session2.csv"});
	var TableCsv = new tabler.table("tableId3",csv);
	TableCsv.create();
	var timercsv = setInterval(function(){
		Httpcsv.GET(function(data){
			console.log((new Date).getTime() + " response " + ": " + data);
			TableCsv.update(data);
		},
		function(reason){//失败处理，关闭update定时器			
			console.log("fail: " + reason + "  timer will be close");
			clearInterval(timercsv);
		});
	},1000);
	
	//line 线状图系列
	
	//测试多组数据绘制在一张图的情况
	var ctx1 = document.getElementById("canvas1").getContext("2d");
	var data = []
	var data3 = []
	var data4 = []
	var free = []
	var Render1 = new drawer.render(ctx1,"line",[data,data3,data4,free],["cpu","mem","bindwith","free"]);
	Render1.draw();
	var cur = 0;
	var Http3 = new httper.http({way:"mvar",service:"ram",varname:"free"},"192.168.81.71:10080");
	var timer3 = setInterval(
		 function(){
			Http3.GET(function(data){
				console.log((new Date).getTime() + " response " + ": " + data);
				cur = parseFloat(JSON.parse(data[0]))/100000;
			},
			function(reason){//失败处理，关闭update定时器			
				console.log("fail: " + reason + "  timer will be close");
				clearInterval(timer3);
			});
			Render1.update([reader.random(100),reader.random(100),reader.random(100),cur]);
		},
	1000);

	
	//测试一组自定义数据绘制在一张图的情况
	var ctx1 = document.getElementById("canvas2").getContext("2d");
	var data = []
	var Render2 = new drawer.render(ctx1,"line",data,"cpu","red");
	Render2.draw();	
	setInterval(
		 function(){
			 Render2.update(reader.random(100));			 
		 },
	1000);
	
	/*
	//测试连接TianShan中的服务	
	var ctxRtsp = document.getElementById("canvas2").getContext("2d");
	var rtspdata1 = [0];
	var rtspdata2 = [0];
	var rtspdata3 = [0];
	var rtspRender = new drawer.render(ctxRtsp,"line",[rtspdata1,rtspdata2,rtspdata3],["rtspdata1","rtspdata2","rtspdata3"]);
	rtspRender.draw();
	var Http = new httper.http([{
		service:"rtspProxy",
		varname:"rtspProxy-Statistics-Total-Succeeded-Request-Count"
	},
	{
		service:"rtspProxy",
		varname:"rtspProxy-Statistics-Average-Process-Latency"
	},
	{
		service:"rtspProxy",
		varname:"rtspProxy-Statistics-Request-Count"
	}
	]);
	var timer = setInterval(function(){
		Http.GET(
			function(result){
				console.log((new Date).getTime() + " response " + ": " + result);
				var len = result.length;
				var data = [];
				var i = 0;
				while(i < len){
					data.push(parseFloat(result[i].split(":")[1]) +reader.random(1))
					i++;
				}				
				rtspRender.update(data);
			},
			function(reason){//失败处理，关闭update定时器			
				console.log("fail: " + reason + "  timer will be close");
				clearInterval(timer);
			});
		},
	1000);
	*/
	//pie 圆形图系列
	// var CircleData = reader.readJson("/fvar/data/part.json");
	var data2 = [
		["Sentry",45],
		["EventChannl",20],
		["EventGateWay",22],
		["RtspProxy",13],
		["EdgeFE",30],
		["MQTT",13]
	]
	var ctx2 = document.getElementById("part").getContext("2d");
	var Render3 = new drawer.render(ctx2,"circle",data2,"server List");
	Render3.draw();
	Render3.update(["xxxx",35]);
	Render3.update(["yyyy",37]);
	Render3.update(["zzzz",37]);
	
});
