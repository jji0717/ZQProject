
	<!-- // Hide script from old browsers
	function StatusScheme(bg, pic)
	{
		this.backgroundColor = bg;
		this.statusPic = pic;
	}
	var ST_ORIG = 0;
	var ST_OVER = 1;
	var ST_ACTIVE = 2;
	var _stDirOpen = new Array(new StatusScheme('#655EA2', 'images/tree/arrow_up.gif'), new StatusScheme('#655EA2', 'images/tree/arrow_up_over.gif'), new StatusScheme('#655EA2', 'images/tree/arrow_up_active.gif'))
	var _stDirClose = new Array(new StatusScheme('#655EA2', 'images/tree/arrow_down.gif'), new StatusScheme('#655EA2', 'images/tree/arrow_down_over.gif'), new StatusScheme('#655EA2', 'images/tree/arrow_down_active.gif'))
	var _stItem = 	new Array(new StatusScheme('#FFFFFF'), new StatusScheme('#ebf7fe'), new StatusScheme('#c1e7fc'));
	var _stTop = 	new Array(new StatusScheme('#ADADAD', 'images/tree/arrow_left.gif'), new StatusScheme('#ADADAD', 'images/tree/arrow_left_over.gif'), new StatusScheme('#ADADAD', 'images/tree/arrow_active.gif'))
	
	function changeStatus(node, st)
	{
		node.style.backgroundColor = st.backgroundColor;
		var nodes = node.childNodes;
		if(st.statusPic == null)
			return;

		for(var i = 0; i < nodes.length; ++i)
		{
			if(nodes[i].nodeName.toUpperCase() == "IMG")
			{
				nodes[i].src = st.statusPic;
				break;
			}
		}
	}
	function findOutStatusList(obj)
	{
		if(obj.className.indexOf('nav_top') != -1)
		{
			return _stTop;
		}
		if(obj.className.indexOf('nav_dir') != -1)
	  	{
			// find out the folder status
			var divNode = obj.nextSibling;
		    if(divNode != null && divNode.nodeName.toUpperCase() == "DIV")
		    {
			    if(divNode.style.display == 'block')
			    { // folder opened
			    	return _stDirOpen;
			    }
			   else
		    	{ // folder closed
		    		return _stDirClose;
		    	}
		    }
		}
		else if (obj.className.indexOf('nav_item') != -1)
		{
			return _stItem;
		}
	}
	function toggleFolder(id, node) 
	{
		// find out the current status
		var folder = document.getElementById(id);
		if(folder == null)
		{ // bad id
		}
		else if (folder.style.display == "block")
		{ // close the folder
			folder.style.display = "none";
			// change the status
			if(node.id == 'activeP')
			{ // active -> active
				changeStatus(node, _stDirClose[ST_ACTIVE]);
			}
			else // over -> over
			{
				changeStatus(node, _stDirClose[ST_OVER]);
			}
		}
		else
		{ // open the folder
			folder.style.display = "block";
			// change the status
			if(node.id == 'activeP')
			{ // close.active -> open.active
				changeStatus(node, _stDirOpen[ST_ACTIVE]);
			}
			else // close.over -> open.over
			{
				changeStatus(node, _stDirOpen[ST_OVER]);
			}
		}
	}
	
	_activePObj = null;
	function markActive(obj)
	{
        if(window.event)
		    window.event.cancelBubble = true;
		if(_activePObj)
		{
		  _activePObj.id = '';
		  var st = findOutStatusList(_activePObj);
		  if(st != null)
		  { // active -> original
		  	changeStatus(_activePObj, st[ST_ORIG]);
		  }
		  	_activePObj = null;
		}
		
		// get parent P
		var tmp = obj;
		while(tmp.nodeName.toUpperCase() != "P")
		{
			tmp = tmp.parentNode;
		}
		_activePObj = tmp;
		_activePObj.id = 'activeP';
		// update status
		{
			var st = findOutStatusList(_activePObj);
			if(st != null)
			{ // original -> active
				changeStatus(_activePObj, st[ST_ACTIVE]);
			}
		}
	}
	
	function mouseGoesOver(obj)
	{
		if(obj.id == "activeP")
			return; // no change

		var st = findOutStatusList(obj);
		if(st != null)
		{ // original -> over
			changeStatus(obj, st[ST_OVER]);
		}
		
	}

	function mouseGoesOut(obj)
	{
		if(obj.id == "activeP")
			return; // no change
			
		var st = findOutStatusList(obj);
		if(st != null)
		{ // over -> original
			changeStatus(obj, st[ST_ORIG]);
		}
	}
	-->