require(['jquery'],function($){
	var that = this;
	var side = $("#sidectl");
	var body = $(document.body);
	var footer = $(".footer");	
	side.each(function(){
	var li = $(this).children();
	li.mousedown(function(e){
		var rand = parseInt(Math.random()*1000);
		switch (e.which){
			case 1://×ó¼ü
				if(this.id == "lr")
				{
					var newdiv = $("<div></div>").attr('id',"box"+rand).addClass("wrap");
					$("<div></div>").attr('id','left'+rand).addClass("left").appendTo(newdiv);
					$("<div></div>").attr('id','right'+rand).addClass("right").appendTo(newdiv);
					newdiv.insertBefore(footer);
					return newdiv;
				}
				if(this.id == "ud")
				{
					var newdiv = $("<div></div>").attr('id',"box"+rand).addClass("wrap");
					$("<div id='up'+rand></div>").attr('id',"up"+rand).appendTo(newdiv);
					$("<div id='down'+rand></div>").attr('id',"down"+rand).appendTo(newdiv);
					newdiv.insertBefore(footer); 
					return newdiv;
				}
				console.log("cell number:" + this.cellIndex + " cell text:" + this.innerText);
			break;
			case 2://ÖÐ¼ü
				//console.log("cell number:" + this.cellIndex + " cell text:" + this.innerText);
			break;
			case 3://ÓÒ¼ü
				//console.log("cell number:" + this.cellIndex + " cell text:" + this.innerText);
			break;
			default:
			break;
		}
		});
	});
});
