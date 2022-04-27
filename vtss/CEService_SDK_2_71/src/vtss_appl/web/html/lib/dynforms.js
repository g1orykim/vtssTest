// * -*- Mode: java; tab-width: 8; -*-
/*

 Vitesse Switch Software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.

*/
// *******************************  DYNFORMS.JS  *****************************
// *
// * Author: Lars Povlsen
// *
// * --------------------------------------------------------------------------
// *
// * Description:  Client-side JavaScript functions.
// *
// * To include in HTML file use:
// *
// * <script language="javascript" type="text/javascript" src="lib/config.js"></script>
// * <script language="javascript" type="text/javascript" src="lib/dynforms.js"></script>
// *
// * --------------------------------------------------------------------------


// * --------------------------------------------------------------------------
// *  (Web) Settings access

function settingsRefreshInterval()
{
    return 1000 * 3;
}

// * --------------------------------------------------------------------------
// *  Array generation

function genArrayInt(start, end)
{
    var a = new Array(end-start+1);
    var i;
    for(i = 0; i < a.length; i++) {
        a[i] = i + start;
    }
    return a;
}

function genArrayStr(start, end, prefix)
{
    var a = new Array(end-start+1);
    var i;
    for(i = 0; i < a.length; i++) {
        var val = String(i + start);
        a[i] = prefix ? prefix + val : val;
    }
    return a;
}

// * --------------------------------------------------------------------------
// *  Dynamic forms etc.

// Remove existing content of an element - and return it
function clearChildNodes(elem) {
    while (elem && elem.childNodes.length > 0) {
        elem.removeChild(elem.firstChild);
    }
    return elem;
}

function addCell(tr, td, content)
{
    td.appendChild(content);    // Add content to cell
    tr.appendChild(td);         // Add cell to row
}

function CreateStyledElement(element, style)
{
    var elm = document.createElement(element);
    elm.setAttribute("class", style); // For FF
    elm.setAttribute("className", style); // For MSIE
    return elm;
}

function CreateTd(style)
{
    return CreateStyledElement("td", style);
}

function addTypedTextCell(tr, type, value, style, colspan, rowspan)
{
    var cell = CreateStyledElement(type, style);
    if(colspan) {
        cell.setAttribute("colSpan", colspan);
    }
    if(rowspan) {
        cell.setAttribute("rowSpan", rowspan);
    }
    addCell(tr, cell, document.createTextNode(value));
    return cell;
}

function addTextCell(tr, value, style, colspan)
{
    return addTypedTextCell(tr, "td", value, style, colspan);
}

function addTextHeaderCell(tr, value, style, colspan, rowspan)
{
    return addTypedTextCell(tr, "th", value, style, colspan, rowspan);
}

function addLabelTextCell(tr, value, style, id)
{
    var td = CreateTd(style);
    var label = document.createElement("label");
    label.setAttribute("for", id);
    addCell(td, label, document.createTextNode(value));
    tr.appendChild(td);
    return td;
}

function addCheckBoxCell(tr, value, style, name)
{
    var td = CreateTd(style);
    var field = document.createElement("input");
    field.type = "checkbox";
    field.id = field.name = name;
    addCell(tr, td, field);
    if(value) {
        field.setAttribute("checked", true); // FF
        field.setAttribute("defaultChecked", true); // MSIE+W3C
    }
    return field;
}

function ChangeImg()
{
    var id = this.id;
    var imgsrc = (document.getElementById(id));
    var iid = "hidden_"+id;
    var infeildc = (document.getElementById(iid));
    var imgsc = infeildc.getAttribute("value");
    imgsrc.setAttribute('height',12);
    imgsrc.setAttribute('width',15);
    if(imgsc == 1) {
        document.getElementById(id).src ="images/forbid.gif";
        infeildc.setAttribute("value","2");
        imgsrc.setAttribute("alt","Forbidden Port");
        imgsrc.setAttribute("title","Forbidden Port");
    } else if(imgsc == 2) {
        document.getElementById(id).src ="images/menu_o.gif";
        infeildc.setAttribute("value","0");
        imgsrc.setAttribute("alt","Vlan not included");
        imgsrc.setAttribute("title","Vlan not included");
    } else if(imgsc == 0){
        document.getElementById(id).src ="images/port.gif";
        infeildc.setAttribute("value","1");
        imgsrc.setAttribute("alt","Vlan included");
        imgsrc.setAttribute("title","Vlan included");
    }
}

function addPortList(tr, value, style, name)
{
    var td = CreateTd(style);
	
    var field = document.createElement('IMG');
    var infield = document.createElement('input');
    infield.type = "hidden";
    infield.id = infield.name= "hidden_"+name;
    infield.setAttribute("value","0");
    field.setAttribute('border',1);
    field.setAttribute('height',12);
    field.setAttribute('width',15);
    field.id = name;
    field.onclick = ChangeImg;
    addCell(tr, td, field);
    addCell(tr,td,infield);
    if(value == 1) {
        field.setAttribute("src","images/port.gif");
        infield.setAttribute("value","1");
        field.setAttribute("alt","Vlan included");
        field.setAttribute("title","Vlan included");
    } else if(value == 2) {
        field.setAttribute("src","images/forbid.gif");
        infield.setAttribute("value","2");
        field.setAttribute("alt","Forbidden Port");
        field.setAttribute("title","Forbidden Port");
    } else {
        field.setAttribute("src","images/menu_o.gif");
        infield.setAttribute("value","0");
        field.setAttribute("alt","Vlan not included");
        field.setAttribute("title","Vlan not included");
    }
    return field;
}
function addSelectCell(tr, oT, oV, value, style, name, width, colspan)
{
    var td = CreateTd(style);
    var field = document.createElement('select');
    field.id = field.name = name;
    var x;
    for (x=0; x < oT.length; x++) {
        var optionItem = document.createElement('option');
        optionItem.value = oV[x];
        if(value == optionItem.value) {
            optionItem.setAttribute("selected", true);
            optionItem.setAttribute("defaultSelected", true);
            optionItem.defaultSelected = true; // Needed for MSIE 8
        }
        optionItem.appendChild(document.createTextNode(oT[x]));
        field.appendChild(optionItem);
    }
    if (width) {
        if (navigator.appName && 
            navigator.appName == 'Microsoft Internet Explorer' &&
            navigator.userAgent.match("MSIE [567]")) {
            field.setAttribute("width", "auto");
        } else {
            field.style.width = width;
        }
    }
    if (colspan) {
        td.setAttribute("colSpan", colspan);
    }
    addCell(tr, td, field);
    return field;
}

function addMultiSelectCell(tr, oT, oV, value, style, name, size, width)
{
    var td = CreateTd(style);
    var field = document.createElement('select');
    field.id = field.name = name;
    field.multiple = 'true';
    var x, y;
    for (x=0; x < oT.length; x++) {
        var optionItem = document.createElement('option');
        optionItem.value = oV[x];
        for (y=0; y < value.length; y++) {
            if(value[y] == optionItem.value) {
                optionItem.setAttribute("selected", true);
                optionItem.setAttribute("defaultSelected", true);
                optionItem.defaultSelected = true; // Needed for MSIE 8
            }
        }
        optionItem.appendChild(document.createTextNode(oT[x]));
        field.appendChild(optionItem);
    }
    field.size = size;
    if (width) {
        if (navigator.appName && 
            navigator.appName == 'Microsoft Internet Explorer' &&
            navigator.userAgent.match("MSIE [567]")) {
            field.setAttribute("width", "auto");
        } else {
            field.style.width = width;
        }
    }
    addCell(tr, td, field);
    return field;
}

function addInputCell(tr, value, style, name, size, maxsize, width, colspan)
{
    var td = CreateTd(style);
    var field = document.createElement('input');
    field.id = field.name = name;
    field.setAttribute("size", size);
    field.setAttribute("value", value);
    field.setAttribute("defaultValue", value);
    field.defaultValue = value; // Needed for MSIE 8
    if (maxsize) {
        field.setAttribute("maxLength", maxsize);
    }
    if (width) {
        if (navigator.appName && 
            navigator.appName == 'Microsoft Internet Explorer' &&
            navigator.userAgent.match("MSIE [567]")) {
            field.setAttribute("width", "auto");
        } else {
            field.style.width = width;
        }
    }
    if (colspan) {
        td.setAttribute("colSpan", colspan);
    }
    addCell(tr, td, field);
    return field;
}

function addInputHexCell(tr, value, style, name, size, maxsize)
{
    td = CreateTd(style);
    hex_header = document.createTextNode("0x");
    td.appendChild(hex_header);
    
    input_value = document.createElement('input');
    input_value.id = input_value.name = name;
    input_value.setAttribute("size", size);
    input_value.setAttribute("maxLength", maxsize);
    input_value.setAttribute("value", value);
    input_value.setAttribute("defaultValue", value);
    td.appendChild(input_value);
    tr.appendChild(td);
}

function addInputCellWithText(tr, value, style, name, size, maxsize, width, front_text, rear_text, input_type)
{
    var td = CreateTd(style);
    if (front_text) {
        var hex_header = document.createTextNode(front_text);
        td.appendChild(hex_header);
    }
    var field = document.createElement('input');
    if (input_type) {
        field.type = input_type;
    }
    field.id = field.name = name;
    field.setAttribute("size", size);
    if (maxsize) {
        field.setAttribute("maxLength", maxsize);
    }
    field.setAttribute("value", value);
    field.setAttribute("defaultValue", value);
    field.defaultValue = value; // Needed for MSIE 8
    if (width) {
        if (navigator.appName && 
            navigator.appName == 'Microsoft Internet Explorer' &&
            navigator.userAgent.match("MSIE [567]")) {
            field.setAttribute("width", "auto");
        } else {
            field.style.width = width;
        }
    }
    td.appendChild(field);
    if (rear_text) {
        var hex_header = document.createTextNode(rear_text);
        td.appendChild(hex_header);
    }
    tr.appendChild(td);
    return field;
}

function addInputAreaCell(tr, value, style, name, col_size, row_size)
{
    var td = CreateTd(style);
    var field = document.createElement('textarea');
    field.id = field.name = name;
    field.setAttribute("rows", row_size);
    field.setAttribute("cols", col_size);
    field.value = value;
    field.defaultValue = value;
    addCell(tr, td, field);
    return field;
}

function addHiddenInputCell(tr, value, style, name, size)
{
    var td = CreateTd(style);
    var field = document.createElement('input');
    field.type = 'hidden';
    field.id = field.name = name;
    field.setAttribute("size", size);
    field.setAttribute("value", value);
    field.setAttribute("defaultValue", value);
    field.defaultValue = value; // Needed for MSIE 8
    addCell(tr, td, field);
    return field;
}

function addPasswordCell(tr, value, style, name, size)
{
    var td = CreateTd(style);
    var field = document.createElement('input');
    field.type = 'password';
    field.id = field.name = name;
    field.setAttribute("size", size);
    field.setAttribute("value", value);
    field.setAttribute("defaultValue", value);
    field.defaultValue = value; // Needed for MSIE 8
    addCell(tr, td, field);
    return field;
}

function addImageCell(tr, style, src, text)
{
    var td = CreateTd(style);
    var field = document.createElement('img');
    field.src = src;
    field.border = 0;
    field.title = field.alt = text;
    addCell(tr, td, field);
    return field;
}

function addRadioCell(tr, value, style, name, id, text)
{
    var td = CreateTd(style);
    var field;

    try {
        field = document.createElement(document.all?'<input name="'+name+'">':'input');
    } catch(e) {
        field = document.createElement('input');
    }

    field.id = id;
    field.name = name;
    field.type = "radio";
    field.setAttribute("value", id);

    addCell(tr, td, field);
    if (value) {
        field.setAttribute("checked", "checked");
        field.setAttribute("defaultChecked", "checked"); // MSIE+W3C
    }
    if (text) {
        td.appendChild(document.createTextNode(text));
    }
    return field;
}

function addRadioSetCell(tr, value, style, name, id, text, onclick)
{
    var td = CreateTd(style);
    var field;

    for (var i = 0; i < id.length; i++) {
        try {
            field = document.createElement(document.all?'<input name="'+name+'">':'input');
        } catch(e) {
            field = document.createElement('input');
        }

        field.id = id[i];
        field.name = name;
        field.type = "radio";
        field.setAttribute("value", id[i]);
        if (value == id[i]) {
            field.setAttribute("checked", "checked");
            field.setAttribute("defaultChecked", "checked"); // MSIE+W3C
        }
        td.appendChild(field);
        if (text[i]) {
            td.appendChild(document.createTextNode(text[i]));
        }
        if (onclick) {
            field.onclick = onclick;
        }
    }
    tr.appendChild(td);
    return td;
}

function addLink(tr, style, url, text, target)
{
    // default value
    var new_target = (target == null) ? "" : target;


    var td = CreateStyledElement("td", style);
    var link = document.createElement("a");
    link.href = url;
    link.target = new_target;
    link.appendChild(document.createTextNode(text)); // Add Text
    td.appendChild(link);       // Add link to cell
    tr.appendChild(td);         // Add cell to row
}

function addHiddenParam(form, name, value)
{
    var field = document.createElement('input');
    field.type = 'hidden';
    field.id = field.name = name;
    field.value = value;
    form.appendChild(field);         // Add cell to row
    return field;
}

function addButtonCell(tr, value, style, name)
{
    var td = CreateTd(style);
    var field = document.createElement('input');
    field.id = field.name = name;
    field.type = "button";
    field.value = value;
    addCell(tr, td, field);
    return field;
}

function addFragment(frag, id, clear)
{
    var tbody = document.getElementById(id);
    if (tbody) {
        if (clear) {
            clearChildNodes(tbody);
        }
        if (!tbody.appendChild(frag)) {
            alert("This browser doesn't support dynamic tables.");
        }
    }
}

/* URL argument extraction */


function getParamFromURL(url, name)
{
    var arg_ix = url.indexOf('?');
    if (arg_ix != -1) {
        var args = url.substring(arg_ix+1, document.URL.length);
        var tups = args.split('&');
        var i;
        for(i = 0; i < tups.length; i++) {
            var tup = tups[i].split('=');
            if(tup.length == 2) {
                if(tup[0] == name) {
                    return  tup[1];
                }
            }
        }
    }
}

/* Update functions - previously in ajax.js */

function a2s(aText, val)
{
    if(aText[val]){
        return aText[val];
    }
    return "undefined (" + String(val) + ")";
}

function UpdateId(id, val)
{
    var elm = document.getElementById(id);
    if(elm) {
        elm.innerHTML = val;
    }
    return elm;
}

function UpdateIdValue(id, val)
{
    var elm = document.getElementById(id);
    if(elm) {
        elm.value = val;
        elm.defaultValue = val;
    }
    return elm;
}

function UpdateIdChecked(id, val, ignore_default)
{
    var elm = document.getElementById(id);
    if(elm) {
        elm.checked = val;
        if (ignore_default) {
            return elm;
        }
        if(val) {
            elm.setAttribute("checked", "checked");
            elm.setAttribute("defaultChecked", "checked"); // MSIE+W3C
        } else {
            elm.removeAttribute("checked");
            elm.removeAttribute("defaultChecked"); // MSIE+W3C
        }
    }
    return elm;
}

function UpdateRadioChecked(name, val, ignore_default)
{
    var elm = document.getElementsByName(name);
    if (elm) {
        for(var i = 0; i < elm.length; i++) {
            elm[i].checked = elm[i].value == val ? true : false;
            if(ignore_default) {
                continue;
            }
            if(elm[i].value == val) {
                elm[i].setAttribute("checked", "checked");
                elm[i].setAttribute("defaultChecked", "checked"); // MSIE+W3C
            } else {
                elm[i].removeAttribute("checked");
                elm[i].removeAttribute("defaultChecked"); // MSIE+W3C
            }
        }
    }
    return elm;
}

function getRadioCellValue(name)
{
    var elm = document.getElementsByName(name);
    if (elm) {
        for(var i = 0; i < elm.length; i++) {
            if(elm[i].checked) {
                return elm[i].value;
            }
        }
    }
    return elm;
}

function UpdateIdSelect(id, oT, oV, value)
{
    var elm = document.getElementById(id);
    if(elm) {
        clearChildNodes(elm);
        for (var x=0; x < oT.length; x++) {
            var optionItem = document.createElement('option');
            optionItem.value = oV[x];
            if(value == optionItem.value) {
                optionItem.setAttribute("selected", true);
                optionItem.setAttribute("defaultSelected", true);
                optionItem.defaultSelected = true; // Needed for MSIE 8
            }
            optionItem.appendChild(document.createTextNode(oT[x]));
            elm.appendChild(optionItem);
        }
    }
    return elm;
}

function UpdateIdSetSelect(id, value)
{
    var i;
    var opt;
    var sel;
    var elm = document.getElementById(id);
    if (elm && elm.options) {
        for (i = 0; i < elm.options.length; i++) {
            opt = elm.options[i];
            sel = (opt.value == value);
            opt.selected = sel;
            opt.defaultSelected = sel;
            if (sel) {
                elm.selectedIndex = i;
            }
        }
    }
    return elm;
}

function UpdateIdGroupedSelect(id, oG, oE, sep, value)
{
    var elm = document.getElementById(id);
    if(elm) {
        clearChildNodes(elm);
        for (var g = 0; g < oG.length; g++) {
            var groupItem = document.createElement('optgroup');
            groupItem.setAttribute("label", oG[g]);
            for (var x = 0; x < oE[g].length; x++) {
                var optionItem = document.createElement('option');
                var tuple = oE[g][x].split(sep);
                optionItem.value = tuple[0];
                if(value == optionItem.value) {
                    optionItem.setAttribute("selected", true);
                    optionItem.setAttribute("defaultSelected", true);
                    optionItem.defaultSelected = true; // Needed for MSIE 8
                }
                optionItem.appendChild(document.createTextNode(tuple[1]));
                groupItem.appendChild(optionItem);
            }
            elm.appendChild(groupItem);
        }
    }
    return elm;
}

function searchArgs(search)
{
    var tups = search.substring(1).split('&');
    var args = new Array();
    for(var i = 0; i < tups.length; i++) {
        var tup = tups[i].split('=');
        var idx = tups[i].search(/=/);
        if (idx == -1) {
            args[tup[0]] = 1;
        } else {
            args[tup[0]] = tups[i].substring(idx + 1, tups[i].length);
        }
    }
    return args;
}

function SetVisible(name, visible)
{
    var elm = document.getElementById(name);
    if(visible) {
        elm.style.display = '';
    } else {
        elm.style.display = 'none';
    }
}

function poag2portid(port)
{
    var portid = port.match(/^\d+$/) ? configPortName(port, 0) : port;
    return portid;
}



//
// Completes/finishes a table, and check for browser support for dynamic tables
//
function FinishTable(frag,table_name) {
    var tbody = document.getElementById(table_name);
    clearChildNodes(tbody);
    if (!tbody.appendChild(frag)) { // Add Frag to table body
    alert("This browser doesn't support dynamic tables.");
    }
}

// Creates a new row. If row_type = 1 the row is created with the color config_even.
// If row_type = 0 the row is colored with the config_odd color. If row_type is left out
// the row color the swap between config_odd and config_even colors every time the function
// is called.
// Returns the row element.
function CreateRow(row_type)
{
    var tr;
    if (row_type == "1") {
	CreateRow.row_even = 1;
	tr = CreateStyledElement("tr", "config_even");
    } else if (row_type == "0") {
	CreateRow.row_even = 0;
	tr = CreateStyledElement("tr", "config_odd");
    } else if (typeof CreateRow.row_even == 'undefined' ) {
	// It has not... perform the initilization
	CreateRow.row_even = 0;
	tr = CreateStyledElement("tr", "config_even");
    } else {
	if (CreateRow.row_even === 0) {
	    CreateRow.row_even = 1;
	} else {
	    CreateRow.row_even = 0;
	}
	tr = CreateStyledElement("tr", CreateRow.row_even ? "config_even" : "config_odd");
    }
    return tr; 
}
