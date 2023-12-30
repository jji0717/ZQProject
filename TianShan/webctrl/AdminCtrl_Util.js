//
function TSPDElement(keyname, optional, defaultvalue){
    this.keyname = keyname;
    this.optional = optional;
    this.defaultvalue = defaultvalue;
}
function TSLinkType(name){
    this.name = name;
    this.schema = new Array();
    for(var i = 1; i < arguments.length; ++i){
	this.schema.push(arguments[i]);
    }
    this.variantTypeOf = function(keyname){
	for(var i = 0; i < this.schema.length; ++i){
	    if(keyname == this.schema[i].keyname)
		return typeOfVariant(this.schema[i].defaultvalue);
	}
	return '';
    }
}
function typeOfVariant(stdVarStr){
    return stdVarStr.substr(0, 2);
}
function valueOfVariant(stdVarStr){
    return stdVarStr.substr(2);
}
function standardizeVariantString(type, value){
    if(value.search(/\d+\s*~\s*\d+/) != -1) // range type detected
	return ("R" + type.charAt(1) + value);
    else
	return ("E" + type.charAt(1) + value);
}

function createInputNode(value, type, name){
    if('undefined' == typeof(type))
	type = 'text'
    if('undefined' == typeof(name))
	name = '';

    var inputNode = document.createElement('input');
    inputNode.value = value;
    inputNode.type = type;
    inputNode.name = name;

    return inputNode;
}
function createKVNode(key, value, keyFixed){
    var kvNode = document.createElement('div');
    var keyNode = createInputNode(key);
    keyNode.readOnly = (keyFixed ? true : false);
    var valNode = createInputNode(value);

    kvNode.appendChild(keyNode);
    kvNode.appendChild(document.createTextNode(' = '));
    kvNode.appendChild(valNode);
    return kvNode;
}
function createMapNode(obj, keyPrefix){
    if('undefined' == typeof(keyPrefix))
	keyPrefix = '';

    var mapNode = document.createElement('div');
    for(var prop in obj){
	var key = keyPrefix + prop;
	var value = obj[prop];
	mapNode.appendChild(createInputNode(value, 'hidden', key));
    }
    return mapNode;
}
function createOptionNode(value, label){
    if('undefined' == typeof(label))
	label = value;

    var optNode = document.createElement('option');
    optNode.value = value;
    optNode.appendChild(document.createTextNode(label));

    return optNode;
}
function createTableCell(content, hdr){
    var tag = (hdr ? 'th' : 'td');
    var tdNode = document.createElement(tag);
    tdNode.appendChild(document.createTextNode(content));
    return tdNode;
}