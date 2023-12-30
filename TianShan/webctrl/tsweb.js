// convert a list-like object to array
function _Array(objs) {
    var result = new Array();
    for(var i = 0; i < objs.length; ++i){
        result.push(objs[i]);
    }
    return result;
}
// filter objects
Array.prototype._Filter = function(fn) {
    var result = new Array();
    for(var i = 0; i < this.length; ++i) {
        if(fn(this[i])) {
            result.push(this[i]);
        }
    }
    return result;
}
Array.prototype._Map = function(fn) {
    var result = new Array();
    for(var i = 0; i < this.length; ++i) {
        result.push(fn(this[i]));
    }
    return result
}
function _With(props) {
    return function(obj) {
        for(var k in props) {
            if(!(k in obj) || obj[k] != props[k])
                return false;
        }
        return true;
    }
}
function _WithOut(props) {
    return function(obj) {
        for(var k in props) {
            if((k in obj) && obj[k] == props[k])
                return false;
        }
        return true;
    }
}
function _And(fn1, fn2) {
    return function (obj) {
        return fn1(obj) && fn2(obj);
    }
}
function _Or(fn1, fn2) {
    return function (obj) {
        return fn1(obj) || fn2(obj);
    }
}

function _Set(props) {
    return function(obj) {
        for(var k in props) {
            obj[k] = props[k];
        }
        return true;
    }
}

// function shortcut
function el(id) {
    return document.getElementById(id);
}
function els(elem, tag) {
    return elem.getElementsByTagName(tag);
}