// * -*- Mode: java; c-basic-offset: 4; tab-width: 8; c-comment-only-line-offset: 0; -*-
/*

 Vitesse Switch Software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
// **********************************  SPOM.JS  ********************************
// *
// * Author: Lars Povlsen
// *
// * --------------------------------------------------------------------------
// *
// * Description:  Client-side JavaScript functions.
// *
// * To include in HTML file use:
// *
// * <script language="javascript" type="text/javascript" src="config.js"></script>
// * <script language="javascript" type="text/javascript" src="lib/spom.js"></script>
// *
// * --------------------------------------------------------------------------

var current_sid = 0;

function SpomGetSelector()
{
    var nav = parent.contents;
    if(nav) {
        return nav.document.getElementById("stackselect");
    }
    return 0;                   // Undefined
}

function SpomSetSelectorSid(sid)
{
    current_sid = sid;
    var sel = SpomGetSelector();
    if(sel) {
        for(var i = 0; i < sel.options.length; i++) {
            if(sel.options[i].value == sid) {
                sel.selectedIndex = i;
                return;
            }
        }
    }
}

function SpomSetCurrentSid(sid)
{
    current_sid = sid;
}

function SpomNavigationLoading()
{
    if(configStackable == 0) {
        return false;           // Nothing to load
    }
    var sel = SpomGetSelector();
    if(sel && sel.selectedIndex >= 0) {
        var sel_sid = sel.options[sel.selectedIndex].value;
        if(sel_sid >= 0 && sel_sid <= configSidMax) { // sel_sid == 0 => standalone.
            return false;           // All OK, carry on
        }
    }
    return true;           // Must delay, bail out please
}

function SpomGetCurrentSid()
{
    var sel = SpomGetSelector();
    if(sel) {
        current_sid = sel.options[sel.selectedIndex].value;
    } else {
        // Maybe separated from frameset ?
        while(isNaN(current_sid) || current_sid == 0) {
            current_sid = 1;    // Deafult Sid
        }
    }
    return current_sid;         // Also stored as variable - above
}

function SpomCurrentSidProp(prop)
{
    var sel = SpomGetSelector();
    try {
        var opt = sel.options[sel.selectedIndex];
        return opt.get(prop);
    } catch(e) {
        return -1;
    }
}

function SpomCurrentPorts()
{
    if(configStackable == 0)
        return configNormalPortMax;
    return SpomCurrentSidProp('ports');
}

function SpomCurrentStackPortA()
{
    return SpomCurrentSidProp('stack_a');
}

function SpomCurrentStackPortB()
{
    return SpomCurrentSidProp('stack_b');
}

function SpomIsStack()
{
    var isS = SpomCurrentSidProp('stack_a') > 0;
    return isS;
}

function SpomStackPortCount()
{
    return SpomIsStack() ? 2 : 0;
}

function isStackPort(port)
{
    return (port == SpomCurrentStackPortA()) || (port == SpomCurrentStackPortB());
}

function SpomAddSidArg(url)
{
    if(!current_sid) {
        // Must retrieve SID from selector - loaded through <A> link
        SpomGetCurrentSid();
    }
    if(current_sid) {
        var delim = (url.indexOf("?") == -1) ? "?" : "&";
        url += delim + "sid=" + current_sid;
    }
    return url;
}

function SpomUpdateDisplaySid(elName)
{
    if(current_sid > 0) {
        document.getElementById(elName).innerHTML = "for Switch " + current_sid;
    }
}

function SpomUpdateFormSid(elName)
{
    var field = document.getElementById(elName);
    if(field) {
        field.setAttribute("value", current_sid);
        field.setAttribute("defaultValue", current_sid);
        field.defaultValue = current_sid; // MSIE 8 work-around.
    }
}

// It is possible to insert a element as the first element. E.g. it can be used to insert "All" as the first selector. 
function SpomUpdatePortSelector(elName, selectedPort, includeStackPorts, firstElement)
{
    var frag;
    var i;
    var opt;
    var portCount = parseInt(SpomCurrentPorts());
    var sel = document.getElementById(elName);

    if (sel) {
        // Check and possibly modify the selected port if it is invalid
        if (selectedPort > portCount || (isStackPort(selectedPort) && !includeStackPorts)) {
            selectedPort = configPortMin;
        }
        frag = document.createDocumentFragment();
        clearChildNodes(sel);

	if (firstElement) {
	    var Option = document.createElement("option");
	    sel.options.add(Option);
	    Option.value = firstElement;
	    Option.text = firstElement;
	    Option.setAttribute("selected", true);
	    selectedPort = firstElement; 
	}


        for (i = configPortMin; i <= portCount; i++) {
            if (!isStackPort(i) || includeStackPorts) {
                opt = document.createElement("option");
                opt.appendChild(document.createTextNode(configPortName(i, 1)));
                opt.setAttribute("value", i);
                if (selectedPort == i && !firstElement) {
                    opt.setAttribute("selected", true);
                }
                frag.appendChild(opt);
            }
        }
        sel.appendChild(frag);
    }
    return selectedPort; // Return the possibly modified selected port
}

function SpomHandleError()
{
    if(current_sid > 0) {
      alert("Switch " + current_sid + " does not respond.\nReloading switch list.");
    } else {
      alert("Switch does not respond.");
    }
    // Force reload - not refresh
    top.window.location = top.window.location;
}
