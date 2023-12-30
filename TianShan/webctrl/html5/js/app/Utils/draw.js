define(['js/app/Utils/Color','js/Chart.bundle.min','js/app/Utils/DateFormat'],function (Color){
//define(['Color','Chart','DateFormat'],function (Color){
	var deepcopy = function (o) {
		if (o instanceof Array) {
			var n = [];
			for (var i = 0; i < o.length; ++i) {
				n[i] = deepcopy(o[i]);
			}
			return n;

		} else if (o instanceof Object) {
			var n = {}
			for (var i in o) {
				n[i] = deepcopy(o[i]);
			}
			return n;
		} else {
			return o;
		}
	}
	
	var dataSet = {
		label: "",
		backgroundColor: "",
		pointRadius: 1.5,
		borderColor: "",
		borderWidth: 1,
		data: [],
		fill: false
	}
	
	//构造函数
	var render = function render(ctx,method,srcData,dataTitle,color,typeCircle){
		//default value set
		if(color == undefined) color = ["black"];
		if(dataTitle == undefined) dataTitle = ["Undefined"];
		//if(lazy == undefined) lazy = 1000;
		if(typeCircle == undefined) typeCircle = "doughnut";
		
		this.ctx = ctx;                 //绘图环境的上下文
		this.method = method;           //绘制的图表类型(line,circle等)
		//this.lazy = lazy;               //自动更新的timeout时间
		this.typeCircle = typeCircle;   //用于在绘制饼状系列图的时候的类型选择,有(polarArea || pie || doughnut || bar)
		this.config = null;
		this.CurDataPostion = [];        //记录当前新增的数据坐标,在update的时候会增加，在超过this.MaxDataLength 是会减小
		this.MaxDataLength = 20;        //设置整个绘制的区域的横坐标的容纳数据的个数
		//多个图像在一起显示，有不同的数据 标题 颜色
		this.data = [];                 //二维数组 每一个元素是一个数组,用来保存一组数据
		this.dataTitle = [];
		this.color = [];
		
		if(dataTitle instanceof Array)
			this.dataTitle = dataTitle;
		else
			this.dataTitle.push(dataTitle);
		if(color instanceof Array)
			this.color = color;
		else
			this.color.push(color);	
		//绘图环境的配置项
		switch(method){
			case "line":
			this.config = {
				type: 'line',
				data: {
					labels:[],
					datasets:[]
				},
				options: {
					responsive: true,
					title:{
						display:true,
						text: 'Real Time Analysis',
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
			break;
			case "circle":
			this.config = {
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
			break;
			default:
			break;
		}
		//new a charts	
		this.chart = new Chart(ctx, this.config);
		//这是用来张开整个绘图区的数据的，保证绘图的点个数是 this.MaxDataLength;
		//如果没有这么多数据,会自动用nan补全,在绘制是nan的数据将被忽略而不会在绘制出来
		if(method == "line"){
			if(srcData[0] instanceof Array){  //srcData 是二维数组
				this.data = srcData;
			}else{
				this.data.push(srcData);
			}			
			for(var di = 0; di < this.data.length;++di){
				this.config.data.datasets.push(deepcopy(dataSet));
				//this.config.data.labels.push([]);
				if(this.dataTitle[di] == undefined) this.dataTitle[di] = "Undefined";
				this.config.data.datasets[di].label = this.dataTitle[di];
				if(this.color[di] == undefined) this.color[di] = Color.getColor(di);
				this.config.data.datasets[di].backgroundColor = this.color[di];	
				this.config.data.datasets[di].borderColor = this.color[di];
				this.CurDataPostion[di] = 0;
				if(this.data[di].length < this.MaxDataLength){
					var dealtLen = this.MaxDataLength - this.data[di].length;
					for(var i = 0;i < dealtLen; ++i){
						this.data[di].unshift("nan");
					}
				}
			}
		}
		if(method == "circle"){
			this.data.push(srcData);
			this.CurDataPostion[0] = 0;
		}
	}
	
	/*  //line
		ctx: context
		srcData: source data 
		color:   line color
		dataTitle: give a title for this picture
		lazy:  update the data's time default:1000ms
	 */	
	render.prototype.drawLine = function(){
		var srcData = this.data;
		var curPos = this.CurDataPostion;
		var dataSize = this.data.length;
		var YData =  null;
		var XData = this.config.data.labels;
		var time;
		var di = 0;
		var dataOutRange = false;
		for (var i = curPos[di]; i < srcData[di].length; i++) 
		{
			time = new Date((new Date).getTime() - ((59 - i) * 1000)); // for pageViewsPerSecond chart 
			time.setMilliseconds(0);
			//console.log("tiem:" + time.Format("hh:mm:ss"));
			XData.push(time.Format("hh:mm:ss"));
			for(; di < dataSize; this.CurDataPostion[di]++,++di)
			{
				YData = this.config.data.datasets[di].data;			
				YData.push(srcData[di][i]);
			}
			di = 0;
		}		
		this.chart.update(10); //这个数字是用来控制画线的延时效果的
	}
	
	render.prototype.lineUpdate = function(){
		var srcData = this.data;
		var curPos = this.CurDataPostion;
		var dataSize = this.data.length;
		var YData =  null;
		var XData = this.config.data.labels;
		var time;
		var di = 0;
		var dataOutRange = false;
		time = new Date((new Date).getTime() - ((59 - curPos[di]) * 1000)); // for pageViewsPerSecond chart 
		time.setMilliseconds(0);
		//console.log("tiem:" + time.Format("hh:mm:ss"));
		if(XData.length > this.MaxDataLength)
		{
			XData.shift();
			dataOutRange = true;
		}
		XData.push(time.Format("hh:mm:ss"));
		for(; di < dataSize; this.CurDataPostion[di]++,++di)
		{
			YData = this.config.data.datasets[di].data;			
			YData.push(srcData[di][curPos[di]]);
			if(dataOutRange)
			{
				srcData[di].shift();
				YData.shift();
				this.CurDataPostion[di]--;
			}	
		}
		if(dataOutRange){
			dataOutRange = false;
		}
		this.chart.update(10); //这个数字是用来控制画线的延时效果的
	}
		
	/*  //polarArea || pie || doughnut || bar
		ctx: context
		srcData: source data 
		dataTitle: give a title for this picture
		typeCircle: You can chose: polarArea || pie || doughnut || bar
	 */
	render.prototype.drawCircle = function(){
		var config = this.config;
		var configData = config.data.datasets[0].data;
		var bgColor    = config.data.datasets[0].backgroundColor;
		var dataLabel = config.data.labels;
		if(config.type.toLocaleLowerCase() == "bar")
			config.data.datasets[0].label = '';
		var srcData = this.data[0];
		//for(var key in srcData) {
		var curPos = this.CurDataPostion[0];
		for(var i = curPos;i < srcData.length;++i,this.CurDataPostion[0]++){
			var col = Color.getColor(i);
			bgColor.push(col);
			dataLabel.push(srcData[i][0]);
			configData.push(srcData[i][1]);
		};		
		this.chart.update();
	}
	
	render.prototype.draw = function(){
		var method = this.method;
		switch(method){
			case "line":
			this.drawLine();
			break;
			case "circle":
			this.drawCircle();
			break;
			default:
			break;
		}
	}
	render.prototype.update = function(Adata){
		var method = this.method;
		switch(method){
			case "line":
			var Adatas = [];
			if(Adata instanceof Array){
				Adatas = Adata;				
			}else{
				Adatas.push(Adata);
			}
			for(var i = 0; i < this.data.length;++i){
				if(Adatas[i] == undefined){
					this.data[i].push(this.data[i][this.MaxDataLength]);
				}else{
					this.data[i].push(Adatas[i]);
				}
			}
			this.lineUpdate();
			break;
			case "circle":
			this.data[0].push(Adata);
			this.drawCircle();
			break;
			default:
			break;
		}
	}
	
	//export public function
	//return function
	return {
		render:render
	}
});
