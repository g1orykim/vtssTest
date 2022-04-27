// * -*- Mode: java; c-basic-offset: 4; tab-width: 8; c-comment-only-line-offset: 0; -*-
/*

 Vitesse Switch Software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
// **********************************  AJAX.JS  ********************************
// *
// * Author: Lars Povlsen
// *
// * --------------------------------------------------------------------------
// *
// * Description:  Client-side JavaScript functions.
// *
// * To include in HTML file use:
// *
// * <script language="javascript" type="text/javascript" src="lib/ajax.js"></script>
// *
// * --------------------------------------------------------------------------

function initXMLHTTP() {
    // branch for native XMLHttpRequest object
    var req = null;
    if(window.XMLHttpRequest) {
        try {
            req = new XMLHttpRequest();
        } catch(e) {
            req = null;
        }
    // branch for IE/Windows ActiveX version
    } else if(window.ActiveXObject) {
        try {
            req = new ActiveXObject("Msxml2.XMLHTTP");
        } catch(e) {
            try {
                req = new ActiveXObject("Microsoft.XMLHTTP");
            } catch(e) {
                req = null;
            }
        }
    }
    return req;
}

//disabling submit buttons
function changeForm(req)
{
    var elem;

    elem = document.getElementById("update");
    if(elem) {
        if(req == "grayOut") {
            elem.style.visibility = "visible";
        } else {
            elem.style.visibility = "hidden";
        }
    }

    elem = document.getElementsByTagName("input");
    if(elem) {
        for(var i = 0; i < elem.length; i++) {
            if(elem[i].value == "Save" ||
               elem[i].id == "addNewEntry" ||
               elem[i].id == "addNewEntry1" ||
               elem[i].value == "Next >" ||
               elem[i].value == "Clear" ||
               elem[i].value == "Clear All" ||
               elem[i].value == "Clear This" ||
               elem[i].value == "Remove All" ||
               elem[i].value == "Delete User" ||
               elem[i].value == "Start" ||
               elem[i].value == "Yes" ||
               elem[i].value == "No" ||
               elem[i].name == "configuration" ||
               elem[i].value == "Save Configuration" ||
               elem[i].value == "Translate dynamic to static" ||
               elem[i].value == "Upload") {
                if(req == "restore") {
                    elem[i].disabled = false;
                } else {
                    elem[i].disabled = true;
                }
            }

            if(elem[i].value == "Reset" ||
               elem[i].id == "autorefresh" ||
               elem[i].value == "Refresh" ||
               elem[i].value == " << " ||
               elem[i].value == " |<< " ||
               elem[i].value == " >> " ||
               elem[i].value == " >>| ") {
                if(req == "grayOut") {
                    elem[i].disabled = true;
                } else {
                    elem[i].disabled = false;
                }
            }
        }
    }

    if(req == "readOnly") {
        elem = document.getElementsByTagName("img");
        if(elem) {
            for(var j = 0; j < elem.length; j++) {
                if (elem[j].title) {
                    var pval = elem[j].title.split(" ");

                    if (pval[0] != "Navigate") {
                        elem[j].onclick = null;
                        elem[j].title = "You are not allowed to change settings";
                    }
                } else {
                    elem[j].onclick = null;
                }
            }
        }
    }
}

function loadXMLDoc(file,callback,ref)
{
    var req;

    if ((req = initXMLHTTP()) == null) {
        return null;
    }

    changeForm("grayOut");
    if(typeof(configURLRemap) == "function") {
        file = configURLRemap(file);
    }
    req.open("GET", file, true);
    req.onreadystatechange = function () {
        try {
            if (req.readyState == 4) {
                if (req.status && req.status == 200) {
                    if(req.getResponseHeader("X-ReadOnly") == "null") {
                        document.location.href = 'insuf_priv_lvl.htm';
                    } else {
                        if(typeof(callback) == "function") {
                            callback(req, ref);
                        }
                        if(req.getResponseHeader("X-ReadOnly") == "true") {
                            changeForm("readOnly");
                        } else {
                            changeForm("restore");
                        }
                    }
                        req = null; // MSIE leak avoidance
                } else {
                    try{
                        alert("There was a problem getting page data:\n" + req.statusText);
                        if(typeof(callback) == "function") {
                            callback(req, ref);
                        }
                    } catch(e) {
                        //Nothing to do - the page is probably unloading
                    }
                    req = null; // MSIE leak avoidance
                }
            }
        }
        catch(e){
            // If a page is currently being requested and the user clicks another link on
            // the web page, FireFox (2.0 at least - haven't tested with 1.5) will throw
            // an exception causing this piece of code to be called, whereas Internet
            // Explorer (7.0 at least - haven't tested with 6.0) doesn't. If the Web
            // page that requested the error calls SpomHandleError() then the main
            // page will get recalled. The best thing is therefore not to call the
            // callback function unless there's real data associated; hence the
            // commenting out of the callback() call below.
            // alert("Request error (file = " + file + ". e = " + e + "req = " + req + ")");
            // callback(req, ref);
            req = null; // MSIE leak avoidance
        }
    };
    req.setRequestHeader("If-Modified-Since", "0"); // None cache
    req.send("");
    return req;
}

function redirectOnErrorExtract(req, def_url)
{
    var str = req.responseText;
    var url = false;
    if(req.responseText) {
        if(str.match(/^Error:\s+/)) {
            url = str.replace(/^Error:\s+/, "");
        }
    } else {
        // Use default error URL, if any!
        if(def_url) {
            url = def_url;
        }
    }
    return url;
}

function redirectOnError(req, def_url)
{
    var url = redirectOnErrorExtract(req, def_url);
    if(url) {
        if(typeof(top.setErrorReferrer) == "function") {
            top.setErrorReferrer(window.location.pathname);
        }
        window.location.pathname = url;
    }
    return url;
}

