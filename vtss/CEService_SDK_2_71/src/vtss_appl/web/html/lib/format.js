// * -*- Mode: java; c-basic-offset: 4; tab-width: 8; c-comment-only-line-offset: 0; -*-
/*

 Vitesse Switch Software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
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
/*
Description : Functions that formats strings.
*/


//
// Formats a string to XX-XX-XX-XX-XX-XX.
//
function toMacAddress(Value, AlertOn) {

    // Default value
    var  AlertOn = (AlertOn == null) ? 1 : AlertOn;

    if (!IsMacAddress(Value, AlertOn)) {
        return "00-00-00-00-00-00";
    }

    var MACAddr = Array();

    // Split the max address up in 6 part ( Allowed format is 00-11-22-33-44-55 or 001122334455 )
    if (Value.indexOf("-") != -1) {
        MACAddr = Value.split("-");
    } else {
        for (var j = 0; j <= 5; j++) {
            MACAddr[j] = Value.substring(j*2, j*2+2);
        }
    }

    // Generate return var with format XX-XX-XX-XX-XX-XX
    var ReturnMacAddr = new String;
    for (var i = 0; i <= 5; i++) {
        // Pad for 0 in case of MAC address format ( 1-2-23-44-45-46 )
        if (MACAddr[i].length == 1) {
            MACAddr[i] = "0" + MACAddr[i];
        }

        if (i == 5) {
            ReturnMacAddr += MACAddr[i];
        } else {
            ReturnMacAddr += MACAddr[i] + "-";
        }
    }

    return ReturnMacAddr.toUpperCase();
}

//
// Formats a string to XX-XX-XX.
//
function toOuiAddress(Value) {

    if (!IsOuiAddress(Value, 1)) {
        return "00-00-00";
    }

    var OuiAddr = Array();

    // Split the max address up in 3 part ( Allowed format is 00-11-22 or 001122 )
    if (Value.indexOf("-") != -1) {
        OuiAddr = Value.split("-");
    } else {
        for (var j = 0; j <= 2; j++) {
            OuiAddr[j] = Value.substring(j*2, j*2+2);
        }
    }

    // Generate return var with format XX-XX-XX
    var ReturnOuiAddr = new String;
    for (var i = 0; i <= 2; i++) {
        // Pad for 0 in case of MAC address format ( 1-2-23 )
        if (OuiAddr[i].length == 1) {
            OuiAddr[i] = "0" + OuiAddr[i];
        }

        if (i == 2) {
            ReturnOuiAddr += OuiAddr[i];
        } else {
            ReturnOuiAddr += OuiAddr[i] + "-";
        }
    }

    return ReturnOuiAddr.toUpperCase();
}

//
// Convert a string to another string with its ASCII codes (leading with '0x').
//
function textStrToAsciiStr(sText)
{
    var idx, nmx, ret = "";

    if (sText === null) {
        return ret;
    }

    for (idx = 0; idx < sText.length; idx++) {
        nmx = sText.charCodeAt(idx);
        ret = ret + "0x" + nmx.toString(16);
    }

    return ret;
}

//
// Convert a string in ASCII codes (with leading '0x') to another textual string.
//
function asciiStrToTextStr(sAscii)
{
    var idx, nmx, ret = "";

    if (sAscii === null) {
        return ret;
    }

    nmx = sAscii.split("0x");
    if (nmx.length > 0) {
        for (idx = 1; idx < nmx.length; idx++) {
            ret = ret + String.fromCharCode(parseInt(nmx[idx], 16))
        }
    }

    return ret;
}
