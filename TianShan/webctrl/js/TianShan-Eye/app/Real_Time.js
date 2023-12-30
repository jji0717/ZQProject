require(['reader','draw'],function(read,line){
	//line
	var result  = read.readJson("/data/cpu_mem.json");
	var fundata =function sin(range)
	{
		var i = 0;
		var data = [];
		for(;i< range;i+=0.1)
		{
			data.push(Math.cos(i) * (i));
		}
		return data;
	}(10); 
	var ctx1 = document.getElementById("cpu").getContext("2d");
	var ctx2 = document.getElementById("mem").getContext("2d");
	line.drawLine(ctx1,result.cpu,"rgb(255,0,0)","CPU",1000);
	line.drawLine(ctx2,fundata,"green","Memory",1000);
	
	//pie
	var CircleData = read.readJson("/data/part.json");
	var ctx3 = document.getElementById("part").getContext("2d");
	line.drawCircle(ctx3,CircleData,"Server Static","");
	
	//
});
