var CODE_LeftButton = document.all ? 1 : 0; // ie: 1, firefox:0
function _toolbar_mouseover_handler(o,s)
{
  o.className = "iconRaised";
  o.firstChild.src = s;
}
function _toolbar_mouseup_handler(o, e)
{
  if(e.button==CODE_LeftButton)
  {
    o.className = "iconRaised";
  }
}
function _toolbar_mousedown_handler(o, e)
{
  if(e.button==CODE_LeftButton)
  {
    o.className = "iconSunken";
  }
}
function _toolbar_mouseout_handler(o,s)
{
  o.className = "icon";
  o.firstChild.src = s;
}

/**
 * Toolbar object
 */
function Toolbar()
{
  this.id = "_toolbar_" + (new Date()).getTime();
  this.icons = new Array();
  this.styleSheet = "toolbar.css";
  this.obj = null;

  this.build = _build;
  this.addIcon = _addIcon;
  this.addSeparator = _addSeparator;
  this.setStyleSheet = _setStyleSheet;
  this.init = _init;
  this.init();

  function _init()
  {
  }

  function _setStyleSheet(src)
  {
    this.styleSheet = src;
  }
  function _addIcon(o)
  {
    this.icons[this.icons.length] = o;
  }
  function _addSeparator(o)
  {
    this.icons[this.icons.length] = null;
  }
  function _build()
  {
    var hd = "";
    //toolbar start
    hd += "<table height=24 style=\"background-image:url('img/toolbar_bg.gif')\" width=100% border=0 cellpadding=0 cellspacing=0 onselectedstart='return false' oncontextmenu='return false'>";
    hd += "<tr>";
    hd += "<td width=9><img src='img/toolbar_left.gif' width=9 height=24></td>";
    hd += "<td>";

    hd += "<table border=0 cellpadding=0 cellspacing=1 class=toolbar>";
    hd += "<tr>";
    for(var i=0;i<this.icons.length;i++)
    { 
      if(this.icons[i]!=null)
      {
        this.icons[i].id = "_icon_" + i;
        hd += "<td id=\"" + this.icons[i].id + "\" ";
        hd += "onmouseover=\"_toolbar_mouseover_handler(this,'" + this.icons[i].imageHover.src + "');\" ";
        hd += "onmouseout=\"_toolbar_mouseout_handler(this,'" + this.icons[i].image.src + "');\" ";
        hd += "onmouseup=\"_toolbar_mouseup_handler(this, event);";
        if(this.icons[i].action!=null)
        {
            hd += "if(event.button==CODE_LeftButton){" + this.icons[i].action + ";}";
        }
        hd += "\" ";
        hd += "onmousedown=\"_toolbar_mousedown_handler(this, event);\" ";
        hd += "title=\"";
        if(this.icons[i].tips!=null)
        {
          hd += this.icons[i].tips;
        }
        else
        {
          hd += this.icons[i].label;
        }
        hd += "\"";
        hd += "class=icon>";
        hd += "<img src=\"" + this.icons[i].image.src + "\" width=20 height=20 align=absmiddle>"
        if(this.icons[i].label!=null)
        {
          if(this.icons[i].displayLabel)
          {
            //hd += "<span class=iconLabel>" + this.icons[i].label + "</span>";
            hd += this.icons[i].label;
          }
        }
        hd += "</td>";
      }
      else
      {
        hd += "<td width=6 align=center><img src=\"img/separator.gif\" width=\"2\" height=\"22\" align=\"absmiddle\"/></td>";
      }
    }
    hd += "</tr>";
    hd += "</table>";
    hd += "</td>";
    hd += "<td align=right><image src=\"img/logo_toolbar_right.gif\"></td>";
    hd += "</tr>";
    hd += "</table>";
    return hd;
  }
}

/**
 * Icon object
 */
function Icon()
{
  this.id = "_icon_" + (new Date()).getTime();
  this.obj = null;
  this.image = new Image();
  this.imageHover = new Image();
  this.label = null;
  this.tips = null;
  this.action = null;
  this.displayLabel = true;
  this.hide = _hide;
  this.show = _show;

  this.setDisplayLabel = _setDisplayLabel;
  this.setAction = _setAction;
  this.setLabel = _setLabel;
  this.setTips = _setTips;
  this.setImageSrc = _setImageSrc;
  this.setImageHoverSrc = _setImageHoverSrc;

  function _setDisplayLabel(f)
  {
    this.displayLabel = f;
  }
  function _setTips(t)
  {
    this.tips = t;
  }
  function _setImageHoverSrc(src)
  {
    this.imageHover.src = src;
  }
  function _setLabel(label)
  {
    this.label = label;
  }
  function _setAction(action)
  {
    this.action = action;
  }
  function _setImageSrc(src)
  {
    this.image.src = src;
  }
  function _disable()
  {
    var o = eval(this.id);
    o.onmouseover = "";
    o.onmouseout = "";
    o.onmousedown = "";
    o.onmouseup = "";
  }
  function _enable()
  {

  }
  function _hide()
  {
    eval(this.id).style.display = "none";
  }
  function _show()
  {
    eval(this.id).style.display = "block";
  }
}