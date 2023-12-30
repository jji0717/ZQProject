define(['jquery'],function ($){
	var readJson = function (url)
	{
		var rt;
		//console.log("reading.....");
		$.ajaxSettings.async = false;//保证json已经解析完成：同步操作	
		$.getJSON(url,function(result)
		{
			rt = result;
			//console.log(result);
		});	
		return rt;
	};
	return{
		readJson:readJson
	};
});