define(['jquery'],function ($){
	
	//构造函数
	var table = function table(tableId,tableJson) {
		this.id = '#'+tableId;
		//bind 到 html的div
		this.table = $("<table border=\"1\">");
		//样式设置
		this.table.addClass("tableCss");
		if(tableJson != undefined)
		{
			this.init(tableJson)
		}
		this.CurSortRow = 1;
		this.DefaultUorD = true;
		//大表格控制显示
		this.MaxRows = 13;     //每一次加载表格的最大行数		
		this.Edge = false;    //用来表示表格是否已经到达最大行数或最小行数
		this.OUTRANGE = false; //每一次加载的时候超过this.MaxRows时 为true	
	}
	
	table.prototype.init = function(tableJson){
		if(tableJson  instanceof Array)
			tableJson = tableJson[0];
		this.Json = tableJson;
		this.data = null;		
		var JsonData = null;
		this.theads = [];
		this.rows = 0;         //记录表格的行数
		try {
			JsonData = JSON.parse(tableJson);
			//JsonData = $.parseJSON(tableJson);
			//分离表名和数据
			for(var name in JsonData){//遍历对象属性名 
				this.name = name;
				this.data = JsonData[name];
			}
		}catch (e) {
			var csv = this.csv2json(tableJson);
			this.name = "Udp-State-Dump";
			this.data = csv[this.name];
		}
		for(var key in this.data){//遍历对象属性名
			this.theads.push(key);
		}
		this.columns = Object.getOwnPropertyNames(this.data).length; //记录表格的最大列数			
		this.canSortCol = [];
		this.CurRow = 0;       //记录当前加载到的行号
		//清空原来的表格中的数据
		$(this.id).find("table").empty();
		this.table.appendTo($(this.id));
		this.setSortCol([1,3,5]);
	}
		
	table.prototype.csv2json = function(data){		
		var out = new Object();
		var outarr = [];
		if(data instanceof Array)
			data = data[0];
		var sdata = data.split("\n");//设置每一行的分隔符
		this.rows = sdata.length - 2;
		for(var i = 0;i < sdata.length;i++){
			sdata[i] = sdata[i].replace(/\s/g,'').replace(/\"/g,'');
			outarr.push(sdata[i].split(","));
		}
		var head = outarr[0];		
		for(var j = 0;j < head.length;j++){	
			var row = [];
			for(var i = 1;i < outarr.length - 1;i++){
				if(!isNaN(outarr[i][j])){
					row.push(parseFloat(outarr[i][j]));	
				}else{
					row.push(outarr[i][j]);	
				}
			}
			out[head[j]] = row;
		}
		return {"Udp-State-Dump":out};
	}
	
	//处理表尾对表格的控制(扩展，收缩)
	table.prototype.handlefoot = function()
	{
		$(this.id).find("caption").html(this.name + "(" + this.CurRow + "/" + this.rows + ")");
		if(this.Edge == true){
			$(this.id).find("table tfoot").children().children().each(function(){
				if(this.id == "u")
					$(this).html("&and;");
				if(this.id == "d")
					$(this).html("&or;");
			});
		}
		//如果整个表加载完 删除扩展表尾
		if(this.CurRow >= this.rows){
			this.Edge = true;
			this.CurRow  = this.rows;
			this.MaxRows = this.CurRow;
			$(this.id).find("table tfoot").children().children().each(function(){
				if(this.id == "d")
					$(this).html("");
			});
		}
		//如果整个表被收缩为空 删除扩展表尾
		if(this.CurRow <= 0){
			this.Edge = true;
			this.CurRow  = 0;		
			this.MaxRows = 1;
			$(this.id).find("table tfoot").children().children().each(function(){
				if(this.id == "u")
					$(this).html("");
			});
		}
		
	}
	//创建表格
	table.prototype.create = function(){		
		var table = this.table;
		if(this.data == undefined)
			return;
		var data = this.data;		
		if(this.CurSortRow != 0){
			this.sort(this.theads[this.CurSortRow-1],this.DefaultUorD);			
		}
		
		//1: 加表头
		var thead = $("<thead></thead>");
		var trHeader = $("<tr></tr>");
		trHeader.appendTo(thead);
		var curCol = 0;
		for(var cell in data){//遍历对象属性名
			if(this.rows < data[cell].length){
				this.rows = data[cell].length
			}
			var th = $("<th>" + cell + "</th>");
			th.css("width",cell.length * 13 + "px"); //默认文字的像素大小
			
			//th.attr("title","点击在下可以按我" + cell + "排序哦!");
			//为表头添加排序控制选项
			if(this.canSortCol.length == 0 || this.canSortCol.indexOf(curCol) != -1){
				var tselect = $("<select><option>&and;</option><option>&or;</option></select>").addClass("select").appendTo(th);
			}
			th.addClass("unselect");
			if(this.theads[this.CurSortRow-1] == cell)
				th.css("background-color","#B2E0FF");
			th.appendTo(trHeader);
			curCol++;
		}
		//添加行删除控制的表头
		$("<th>delete</th>").addClass("unselect").appendTo(trHeader);
		this.columns++;
		thead.appendTo(table);
				
		//2: 加表体内容  tbody
		var tbody = $("<tbody></tbody>");
		for(var i = 0;i < this.rows; ++i,++this.CurRow){
			var tr = $("<tr></tr>");
			if(i < this.MaxRows){				
				for(var cell in data){//遍历对象属性名
					var td = $("<td>" + data[cell][i] + "</td>");				
					//td.css("width",String(data[cell][i]).length * 13 + "px"); //默认文字的像素大小
					td.addClass("grid");
					td.appendTo(tr);					
				}
				//添加删除控制的窗口
				$("<td>Yes</td>").addClass("grid").css("color","red").appendTo(tr);
			}else{
				this.OUTRANGE = true;
				break;
			}
			tr.appendTo(tbody);
		}
		//处理超过 MaxRows 的行，新增扩展控制行
		if(this.OUTRANGE){
			//3: 添加表尾tfoot，用来扩展大表格
			var tfoot = $("<tfoot></tfoot>");
			tfoot.attr("title","点击在下有惊喜哦！");
			$("<td><a id='u'>&and;</a>&nbsp;<a id='d'>&or;</a></td>").attr("colspan",this.columns).addClass("unselect").appendTo(tfoot);
			tfoot.appendTo(table);
			this.handlefoot();
		}
		tbody.appendTo(table);
		
		//5: caption 设置
		var tcap =  $("<caption>" + this.name + "(" + this.CurRow + "/" + this.rows + ")</caption>");
		tcap.addClass("unselect");
		tcap.appendTo(table);
		
		//6: 封闭table标签
		$(this.id).append("</table>");
		//添加事件处理函数
		this.click();
		this.dblclick();
	}
	
	//展开大表格的剩下部分(如果需要的话 this.OUTRANGE == true)
	table.prototype.expandBody = function(){
		this.OUTRANGE == false;
		var tbody = $(this.id).find("table tbody");
		var num = 0;
		for(var i = this.CurRow;i < this.rows; ++num,++i,++this.CurRow){
			var tr = $("<tr></tr>");
			if(num < this.MaxRows){				
				for(var cell in this.data){//遍历对象属性名
					var td = $("<td>" + this.data[cell][i] + "</td>");
					//td.css("width",String(data[cell][i].length) * 13 + "px"); //默认文字的像素大小
					td.appendTo(tr);
				}
				//添加删除控制的窗口
				$("<td>Yes</td>").css("color","red").appendTo(tr);
			}else{
				this.MaxRows *= 2;				
				this.OUTRANGE = true;
				break;
			}		
			tr.appendTo(tbody);
		}
		tbody.appendTo(this.table);
		this.handlefoot();

	}
	
	//收缩大表格的过长部分(如果需要的话 this.OUTRANGE == true)
	table.prototype.cutBody = function(){		
		this.OUTRANGE == false;
		this.MaxRows /= 2;
		for(var i = 0;i < this.MaxRows;++i,--this.CurRow){
			$(this.id).find("table tbody tr:last").remove();
		}
		this.handlefoot();
	}
	
	//在调用sort后要用这个函数来吧table的body刷新
	table.prototype.sortBody = function(){
		var data = this.data;
		//定位table body 然后删除他
		$(this.id).find("table tbody").remove();
		//重新加数据更新后的表体内容  tbody
		var tbody = $("<tbody></tbody>");
		for(var i = 0;i < this.rows; ++i){
			var tr = $("<tr></tr>");
			if(i < this.CurRow){
				for(var cell in data){//遍历对象属性名
					var td = $("<td>" + data[cell][i] + "</td>");
					//td.css("width",String(data[cell][i].length) * 13 + "px"); //默认文字的像素大小
					td.appendTo(tr);
				}
				//添加删除控制的窗口
				$("<td>Yes</td>").css("color","red").appendTo(tr);
			}
			tr.appendTo(tbody);
			tbody.appendTo(this.table);
		}
		//处理超过 MaxRows 的行，新增扩展控制行
		if($("table tfoot") == undefined && this.CurRow == this.rows){
			//3: 添加表尾tfoot，用来扩展大表格
			var tfoot = $("<tfoot></tfoot>");
			$("<td><a id='u'>&and;</a>&nbsp;<a id='d'>&or;</a></td>").attr("colspan",this.columns).appendTo(tfoot);
			tfoot.appendTo(this.table);
		}	
	}
	
	//单元格的事件(单击)
	table.prototype.click = function(){
		var that = this;
		//点击表体(body)获取点击的数据
		$(this.id).find("table tbody tr").each(function(){
			var tdArr = $(this).children();
			tdArr.mousedown(function(e){
				switch (e.which){
					case 1://左键
					console.log("cell number:" + this.cellIndex + " cell text:" + this.innerText);
					if(this.nextSibling == null){
						console.log("will delete this rows");
						//$(this).parent().css("text-decoration","line-through");
						$(this).parent().remove();
					}else{
						return this.innerText;
					}	
					break;
					case 2://中键
						console.log("cell number:" + this.cellIndex + " cell text:" + this.innerText);
					break;
					case 3://右键
						console.log("cell number:" + this.cellIndex + " cell text:" + this.innerText);
					break;
					default:
					break;
				}			
			});			
		});
		
		//点击表尾 扩展(收缩)未显示的部分表格
		$(this.id).find("table tfoot").each(function(){
			var tdArr = $(this).children().children();
			tdArr.click(function(){
				if(this.id == "u")
					that.cutBody();
				if(this.id == "d")
					that.expandBody();	
			});			
		});
		
		$(this.id).find("table thead tr").each(function(){
			var thArr = $(this).children();			
			thArr.click(function(){
				console.log("cell number:" + this.cellIndex + " cell text:" + this.innerText);
				that.CurSortRow = this.cellIndex + 1;
				that.DefaultUorD = !that.DefaultUorD;
			});
		});
		
		//点击表头的单元格,按照该列排序
		$(this.id).find("table thead tr th").each(function(){
			var thArr = $(this).children();
			var sortcell = thArr.parent();
			var thead = sortcell.parent();
			thArr.change(function(){				
				thead.each(function(){
					var th = $(this).children();
					th.css("background-color", "");
				});				
				var text = thArr.find("option:selected").text();
				var index = thArr.get(0).selectedIndex;
				console.log("selected " + text + " will sort");
				switch(index){
					case 0://升序
						that.sort(sortcell.get(0).firstChild.data,true);
						that.sortBody();
					break;
					case 1://降序
						that.sort(sortcell.get(0).firstChild.data,false);
						that.sortBody();
					break;
					default:
						console.log("uunknown" + index);
					break;
				}
			});
		});
	}
	
	//行间事件(双击)
	table.prototype.dblclick = function(){
		var that = this;
		$(this.id).find("table tbody").each(function(){
			var trArr = $(this).children();
			trArr.dblclick(function(){
				console.log("row number:" + this.rowIndex + " row texts:" + this.innerText);
				return this.innerText;
				$(this).removeAttr("style");
				$(this).toggleClass("line");
			});
		});
	};
	
	//hide:true -> hide others  false -> not hide others
	table.prototype.search = function(key,hide){
		if(hide == undefined){hide = false;}	
		if(hide){
			var res = $("table tbody tr").filter(":contains('"+ key +"')");
			if(res.length != 0){
				$("table tbody tr").hide().filter(":contains('"+ key +"')").css("background-color","#B2E0FF").show(); //行高亮
				$("table tbody tr td").filter(":contains('"+ key +"')").css("background-color","#99CC33"); //关键字单元格高亮
			}
			else
				console.log("search " + key + " is not exist!");
		}else{
			$("table tbody tr").filter(":contains('"+ key +"')").css("background-color","#B2E0FF"); //行高亮
			$("table tbody tr td").filter(":contains('"+ key +"')").css("background-color","#99CC33"); //关键字单元格高亮
		}
	}
		
	//UorD: true -> Up ;false -> Down
	//这个函数将更新this.data的值，最后调用sortBody即可刷新表格
	table.prototype.sort = function(keyword,UorD){
		if(keyword == undefined)
			return;
		if(UorD == undefined){UorD = false;}
		var data = this.data;
		var sorted = 0;
		if(UorD){
			while(1){
				for(var r = 0; r < this.rows;++r){
					if(data[keyword][r] > data[keyword][r+1]){			
						for(var key in data){
							//参考: http://stackoverflow.com/questions/16201656/how-to-swap-two-variables-in-javascript
							data[key][r+1] = [data[key][r],data[key][r] = data[key][r+1]][0];
						}
					}else{
						sorted++;
					}
				}
				if(sorted == this.rows){
					break;
				}else{
					sorted = 0;
				}
			}
		}else{
			while(1){
				for(var r = 0; r < this.rows;++r){
					if(data[keyword][r] < data[keyword][r+1]){			
						for(var key in data){
							//参考: http://stackoverflow.com/questions/16201656/how-to-swap-two-variables-in-javascript
							data[key][r+1] = [data[key][r],data[key][r] = data[key][r+1]][0];
						}
					}else{
						sorted++;
					}
				}
				if(sorted == this.rows){
					break;
				}else{
					sorted = 0;
				}
			}
		}
	}
	
	//设置可排序的列号。注意：已经将起始列号规范为从1开始
	table.prototype.setSortCol = function (col){
		if(col == undefined){
			for(var i = 0;i < this.columns;++i){
				this.canSortCol.push(i);
			}
		}else if(col instanceof Array){
			for(var i = 0;i < col.length;++i){
				if(col[i] < this.columns){
					this.canSortCol.push(col[i]-1);
				}
			}
		}else{
			this.canSortCol.push(col);
		}
	}
	
	table.prototype.update = function (newtableJson)
	{
		this.init(newtableJson);
		this.create();
	}
		
	return{
		table:table
	};
});