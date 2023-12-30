
function BriefTable(){
    var _cols = new Array();
    var _rows = new Array();
    var _briefCols = new Array();

    var _detailWindow = null;

    String.prototype.trim = function(){
	return this.replace(/(^\s*)|(\s*$)/g, "");
    }

    this.setColumns = function(cols){
	_rows = new Array(); // reset row data
	_cols = new Array();
	for(var i = 0; i < cols.length; ++i)
	    _cols.push(cols[i].toString().trim());
    }
    
    this.setBriefColumns = function(cols){
	_briefCols = new Array();
	for(var i = 0; i < cols.length; ++i)
	    _briefCols.push(cols[i].toString().trim());
    }

    this.addRow = function(row){
	var theRow = new Array();
	for(var i = 0; i < row.length; ++i)
	    theRow.push(row[i].toString().trim());
	_rows.push(theRow);
    }
    Array.prototype.find = function(val){
	for(var i = 0; i < this.length; ++i){
	    if(this[i] == val)
		return i;
	}
	return -1;
    }
    var getScheme = function(){
	var ret = new Array();
	for(var i = 0; i < _briefCols.length; ++i){
	    var idx = _cols.find(_briefCols[i]);
	    if(idx != -1){
		ret.push(idx);
	    }
	}
	return ret;
    }

    var briefColumns = function(){
	var scheme = getScheme();
	var ret = new Array();
	for(var i = 0; i < scheme.length; ++i)
	    ret.push(_cols[scheme[i]]);
	return ret;
    }
    var briefRow = function(idx){
	var ret = new Array();
	if(idx >= _rows.length)
	    return ret;

	var scheme = getScheme();
	for(var i = 0; i < scheme.length; ++i){
	    ret.push(_rows[idx][scheme[i]]);
	}
	return ret;
    }

    // show the detail window
    var resetDetailWindow = function(){
	if(null == _detailWindow || _detailWindow.closed)
	    _detailWindow = window.open("", "", "top=300px,left=500px,height=500px,width=400px");
	_detailWindow.document.close();
    }
    var showDetail = function(idx){
	return function(){
	    resetDetailWindow();
	    var doc = new String();
	    doc = "<html><head><title>Detail</title>";
	    doc += "<link href='tsweb.css' rel='stylesheet' type='text/css'>";
	    doc += "</head><body>";
	    // TODO: put the detail table here
	    doc += "<table class='listTable'>"
	    for(var iCol = 0; iCol < _cols.length; ++iCol){
		doc += "<tr><th>" + _cols[iCol] + "</th><td>" + _rows[idx][iCol] + "</td></tr>";
	    }
	    doc += "</table";
	    doc += "</body></html>";

	    _detailWindow.document.open();
	    _detailWindow.document.write(doc);
	    _detailWindow.document.close();
	    _detailWindow.focus();
	}
    }

    function createTR(arr, header){
	var tag = header ? 'th' : 'td';
	var oTr = document.createElement('tr');
	for(var i = 0; i < arr.length; ++i){
	    var oCell = document.createElement(tag);
	    oCell.appendChild(document.createTextNode(arr[i]));
	    oTr.appendChild(oCell);
	}
	return oTr;
    }
    function hoverEffect(obj){
	return function(){
	    obj.style.cursor = 'pointer';
	}
    }
    this.generate = function(){
	// generate the HTML code

	var oTBody = document.createElement('tbody');

	// the header
	oTBody.appendChild(createTR(briefColumns(), true));

	// the content
	for(var iRow = 0; iRow < _rows.length; ++iRow){
	    var oTr = createTR(briefRow(iRow));
	    oTr.onclick = showDetail(iRow);
	    oTr.onmouseover = hoverEffect(oTr);
	    oTBody.appendChild(oTr);
	}

	var oTbl = document.createElement('table');
	oTbl.className = "listTable";
	oTbl.appendChild(oTBody);
	return oTbl;
    }
}