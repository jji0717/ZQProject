define(['app/Utils/Color','Chart','app/Utils/DateFormat'],function (Color){
	//line
	/*
		ctx: context
		srcData: source data 
		color:   line color
		dataTitle: give a title for this picture
		lazy:  update the data's time default:1000ms
	 */
	var drawLine = function(ctx,srcData,color,dataTitle,lazy){
		if(arguments.length < 2)
		{
		}
		else if(arguments.length < 3)
		{
			
		}
		if(!arguments[2]) color = "black";
		if(!arguments[3]) dataTitle = "Undefined";
		if(!arguments[4]) lazy = 1000;
		
		var config = {
			type: 'line',
			data: {
				labels:[],
				datasets: [{
					label: dataTitle,
					backgroundColor: color,
					pointRadius: 2.5,
					borderColor: color,
					borderWidth:2,
					data: [],
					fill: false
				}]
			},
			options: {
				responsive: true,
				title:{
					display:true,
					text: dataTitle + ' Real Time',
					fontSize: 15
				},
				tooltips: {
					mode: 'index',
					intersect: false,
				},
				hover: {
					mode: 'nearest',
					intersect: true
				},
				scales: {
					xAxes: [{
						display: true,
						interval: 5,
						scaleLabel: {
							display: true,
							stacked: true,
							labelString: 'Time'
						}
						/*,
						ticks: {						
							userCallback: function(tick) {
								var remain = tick.split(":");
								if (remain[remain.length-1] % 3 === 0) {//隔3个单位显示x坐标的刻度
									return tick;
								}
								return '';
							},
						}*/
					}],
					yAxes: [{
						display: true,
						//interval: 2,
						scaleLabel: {
							display: true,
							stacked: false,
							labelString: 'percent'
						}
					}]
				}
			}
		};
		//new a charts	
		var myLine = new Chart(ctx, config);

		//cpu and memory
		var YData = config.data.datasets[0].data;
		var XData = config.data.labels;		
		var datanum = 60;
		if(srcData.length < datanum)
		{
			datanum = srcData.length;
		}
		//init data
		var time;
		var curPos = 0;
		for (var i = 0; i < datanum; i++,curPos++) {
			time = new Date((new Date).getTime() - ((59 - i) * 1000)); // for pageViewsPerSecond chart 
			time.setMilliseconds(0);
			//console.log("tiem:" + time.Format("hh:mm:ss"));
			XData.push(time.Format("hh:mm:ss"));
			YData.push(srcData[i]);//cpu
		}
		myLine.update();
		//update data
		function Update(){
			var time, time2;
			time = new Date();
			time.setMilliseconds(0);
			if(curPos >= srcData.length-1)
				curPos = 0;
			XData.push(time.Format("hh:mm:ss"));
			curPos++;
			YData.push(srcData[curPos]);//cpu
		
			if (YData.length > datanum)
			{
				XData.shift();
				YData.shift();				
			}				
			myLine.update(10);
		};
		setInterval(Update,lazy);	
	};
	
	//polarArea || pie || doughnut || bar
	/*
		ctx: context
		srcData: source data 
		dataTitle: give a title for this picture
		typeCircle: You can chose: polarArea || pie || doughnut || bar
	 */
	var drawCircle = function(ctx,srcData,dataTitle,typeCircle){
		if(arguments.length < 2)
		{
			
		}
		else if(arguments.length < 3)
		{
			
		}
		if(!arguments[2]) dataTitle = "Undefined";
		if(!arguments[3]) typeCircle = "doughnut";
		
		var config = {
			type: typeCircle.toLocaleLowerCase(),
			data: {
				datasets: [
					{
						data: [],
						backgroundColor: []
					}
				],
				labels: []
			},
			options: {
				responsive: true,
				title:{
					display:true,
					text: dataTitle + ' Static',
					fontSize: 15,
					position: 'bottom'
				}				
			}
		};
		var myPie = new Chart(ctx,config);
		var configData = config.data.datasets[0].data;
		var bgColor    = config.data.datasets[0].backgroundColor;
		var dataLabel = config.data.labels;
		if(config.type.toLocaleLowerCase() == "bar")
			config.data.datasets[0].label = '';
		var index = 0;
		for(var key in srcData) {
			bgColor.push(Color.eachColor(index++));
			dataLabel.push(key);
			configData.push(srcData[key]);
		};
		myPie.update();
	};
	
	//return function
	return {
		drawLine:drawLine,
		drawCircle:drawCircle
	}
});