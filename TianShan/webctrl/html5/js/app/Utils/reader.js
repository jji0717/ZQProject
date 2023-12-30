define(['js/jquery'],function ($){
	var readJson = function (url)
	{
		var rt;
		console.log("reading.....");
		$.ajaxSettings.async = false;//保证json已经解析完成：同步操作	
		$.getJSON(url,function(result)
		{
			rt = result;
			console.log(result);
		});	
		return rt;
	};
		
	var random = function(range){
		return Math.random()*range;
	}
	
	return{
		readJson:readJson,
		random:random,
	};
}); 
