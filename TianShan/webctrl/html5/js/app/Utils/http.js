define(function(){
		
	// valid url
	// http://ip:port/getway/serverType/instanceId/varname?xxxx
	/*
	config = {		
		way: "svar"
		service:"rtspProxy",
		instance:"0",
		varname:"rtspProxy-Statistics-Total-Succeeded-Request-Count",
	}
	*/
	// @ ip:port : 
	// @ getway  : 控制获取的数据的方法：svar: 获取Tianshan中服务的数据ZQSnmp模块处理
	//                                   fvar: 获取文件
	//                                   mvar: 获取主机的主要信息,如cpu  memory
	// @ serverType @instanceId  @varname：只有在svar中存在
	var http = function http(config,locate){
		this.protocol = "http://";
		if (locate == undefined) {
			this.addr = "10.15.10.50:10001";
		}else{
			if(locate.split(":").length != 2){ //no port
				this.addr = locate + ":" + this.port + "/";
			}else{
				if(locate[locate.length - 1] == '/'){
					locate.length--;
				}
				this.addr = locate;				
			}
		}
		this.urls = [];
		this.result = [];
		this.reason = [];
		this.getWay = "";
		this.serviceType = "";
		this.instanceId = "";
		this.varname = "";
		this.set(config);
	}
		
	http.prototype.updateUrl = function(){
		if (this.getWay == "" || this.getWay == undefined) {
			this.getWay = "/svar"
		}
		if( this.getWay == "/svar" )
		{
			if (this.instanceId == "" || this.instanceId == undefined) {
				this.instanceId = "/0"
			}
			this.urls.push(this.protocol + this.addr + this.getWay + this.serviceType + this.instanceId + this.varname + "?");
		}else{//非svar模式，@instanceId 均不需要，故设置为空
			this.urls.push(this.protocol + this.addr + this.getWay + this.serviceType + this.instanceId + "?" + this.varname);
		}		
	}
	
	http.prototype.set = function(config){
		var configs = [];
		if(config instanceof Array){
			configs = config;
		}else{
			configs.push(config);
		}
		var i = 0;
		var len = configs.length;
		while(i < len){
			for(key in configs[i]){
				switch(key){
					case "service":
					this.serviceType = "/" + configs[i][key];
					break;
					case "varname":
					this.varname = configs[i][key];
					break;
					case "instance":
					this.instanceId = "/" + configs[i][key];
					break;
					case "way":
					this.getWay = "/" + configs[i][key];
					break;
					default:
					break;
				}
			}
			this.updateUrl();
			i++;
		}
	}

	http.prototype.get = function(url){
		var that = this;
		var xmlHttp = null;
		try {// Firefox, Opera 8.0+, Safari, IE7
			xmlHttp = new XMLHttpRequest();
		}catch (e) {// Old IE
			try {
				xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
			}
			catch (e) {
				alert("Your browser does not support XMLHTTP!");
				return;
			}
		}
		xmlHttp.onreadystatechange = function(){
			/*
			switch(this.readyState){
				case 1:
				//console.log("current State: " + 1);
				break;
				case 2:
				//console.log("current State: " + 2);
				break;
				case 3:
				//console.log("current State: " + 3);
				break;				
				case 4:
					if(this.status == 200){
						console.log(this.responseText);
						that.result.push(this.responseText);
						delete xmlHttp;  //收到返回结果后手动删除
						xmlHttp = null;
					}else{
						that.reason = "readyState: " + this.readyState + " status: " + this.status;
						delete xmlHttp;  //收到返回结果后手动删除
						xmlHttp = null;
					}
				break;
				default:
				break;
			}
			*/
			if(this.readyState == 4){
				if(this.status == 200){
					console.log(this.responseText);					
					that.result.length = 0;
					that.result.push(that.handleResp(this.responseText));
					delete xmlHttp;  //收到返回结果后手动删除
					xmlHttp = null;
				}else {
					that.reason.length = 0;
					that.reason.push("readyState: " + this.readyState + " status: " + this.status);
					delete xmlHttp;  //收到返回结果后手动删除
					xmlHttp = null;
				}
			}
		}
		xmlHttp.open("GET", url, true);
		xmlHttp.send(null);
	}
	
	http.prototype.handleResp = function(responseText){
		if(this.getWay == "/mvar"){
			var service = this.serviceType.substr(1);
			if(this.varname != ""){
				var varname = this.varname.substr(0);
				return JSON.parse(responseText)[service][varname];
			}
		}
		return responseText;
	}
	
	// @success : 数据获取成功后回调的函数
	// @fail    : 数据获取失败回调的函数，用于调用者的其他事务的处理
	http.prototype.GET = function(success,fail){
		var that = this;
		var timer = setInterval(function(){
		var curUrl = 0;
		var urllen = that.urls.length;
		while(curUrl < urllen){
			var res = that.get(that.urls[curUrl]);
			curUrl++;
		}
		if(that.reason.length != 0){
			clearInterval(timer);
			if(fail != undefined){
				fail(that.reason);	
			}			
		}
		if(that.result.length != 0){
			clearInterval(timer);
			if(success !=  undefined){
				success(that.result);
			}						
		}
		},200);
	}
	
	http.prototype.POST = function(callback){
		/*
		return $.post(
			this.url,
			callback,
			"*"
		);
		*/
	}

	return{
		http:http,
	};
});
