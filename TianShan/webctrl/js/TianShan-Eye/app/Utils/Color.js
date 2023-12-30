define(function (){	
	var colorset = {
		black: 'rgb(0,0,0)',
		red: 'rgb(255, 99, 132)',
		orange: 'rgb(255, 159, 64)',
		yellow: 'rgb(255, 205, 86)',
		green: 'rgb(75, 192, 192)',
		blue: 'rgb(54, 162, 235)',
		purple: 'rgb(153, 102, 255)',
		grey: 'rgb(231,233,237)'
	};
	var eachColor = function(index)
	{
		var i = 0;
		for(var key in colorset) {			
			if(i++ === index)
				return colorset[key];			
		}
	}
	return {
		color: colorset,
		eachColor:eachColor
	}
});
