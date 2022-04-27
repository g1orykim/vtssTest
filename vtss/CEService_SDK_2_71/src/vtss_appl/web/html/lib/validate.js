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
// *******************************  VALIDATE.JS  ******************************
// *
// * Description:  Client-side JavaScript functions - form validation
// *
// * To include in HTML file use:
// *
// * <script language="javascript" type="text/javascript" src="lib/validate.js"></script>
// *
// * --------------------------------------------------------------------------


function GiveAlert(Message, fld)
{
    if(fld) {
        fld.focus();
        if(fld.select && typeof(fld.select) == "function") // Text field?
            fld.select();
    }
    alert(Message);
    return false;
}

//
// Check if a number is a valid hex number
//

function IsHex(Value,AlertOn)
{
    // Default value
    var  AlertOn = (AlertOn == null) ? 0 : AlertOn;
    var ValueIsHex = Value.match(/^[0-9a-f]+$/i);

    if (!ValueIsHex && AlertOn) {
        alert ("Value " + Value + " is not a valid hex number");
    }

    return ValueIsHex;
}

//
// Check if a number is a valid digit number
//

function IsDigit(Value,AlertOn)
{
    // Default value
    var  AlertOn = (AlertOn == null) ? 0 : AlertOn;

    ValidChars = "0123456789";

    var ValueIsDigit = true;

    // check for valid characters
    for (var i = 0 ; i <= Value.length; i ++ ) {
	var ValueChar = Value.charAt(i);
	if (ValidChars.indexOf(ValueChar) == -1 ) {
	    ValueIsDigit = false;
	}
    }

    if (!ValueIsDigit && AlertOn) {
        alert ("Value " + Value + " is not a valid digit number");
    }

    return ValueIsDigit;
}

//
// Check if a number is a valid integer
//

function isInt (str)
{
    if(!str.match(/^-?\d+$/))
        return false;

    var i = parseInt (str);
    if (isNaN (i))
	return false;

    i = i.toString ();
    if (i != str)
	return false;

    return true;
}

//
// Check if a field value is a valid integer
// Uses GiveAlert in case of errors
//

function isIntId(fld_id)
{
    var fld = document.getElementById(fld_id);
    if(!fld)
	// Programming error
	return GiveAlert("No such field: " + fld_id, fld);

    if(!isInt(fld.value))
	// User-input error
	return GiveAlert("The value " + fld.value + " is not a valid integer", fld);

    return true;
}



// isWithinRange()
// Function that checks that input value is within a valid range and
// gives an error if not.
// @fld_id is the name of the field. It's value is used as input in the
//         check. It's also used to set focus if an error occurs.
// @MinVal is included in the valid range. If empty, assumed to be 0
// @MaxVal is included in the valid range. If empty, assumed to be 65535
// @start_text and @end_text are used if an error occurs. The error
//         will look as follows:
//         "The value of " + start_text + " is restricted to " + MinVal + " - " + MaxVal + end_text
function isWithinRange(fld_id, MinVal, MaxVal, start_text, end_text)
{
    // Default values
    var minval  = (MinVal == null)  ? 0     : MinVal;
    var maxval  = (MaxVal == null)  ? 65535 : MaxVal;
    var val;

    if(!start_text) {
      start_text = fld_id; // What else to do?
    }
    if(!end_text) {
      end_text = "";
    }

    var fld = document.getElementById(fld_id);
    if(!fld)
        // Programming error
        return GiveAlert("No such field: " + fld_id, fld);

    val = fld.value;
    if(!isInt(val) || val < minval || val > maxval) {
        return GiveAlert(start_text + " must be an integer value between " + minval + " and " + maxval + end_text, fld);
    } else {
        return true;
    }
}

//
// Same as isWithinRange funtion, but supporting floating point.
//
function isWithinRangeFloat(fld_id, MinVal, MaxVal, start_text, end_text)
{
    // Default values
    var minval  = (MinVal == null)  ? 0     : MinVal;
    var maxval  = (MaxVal == null)  ? 65535 : MaxVal;
    var val;

    if(!start_text) {
      start_text = fld_id; // What else to do?
    }
    if(!end_text) {
      end_text = "";
    }

    var fld = document.getElementById(fld_id);
    if(!fld)
        // Programming error
        return GiveAlert("No such field: " + fld_id, fld);

    val = fld.value;

    if(val.length == 0)
        // User-input error
        return GiveAlert("The value of " + start_text + " cannot be empty", fld);

    // isNaN() works on a string, so don't parseInt(fld.value)
    // before calling this function, or isNaN() won't be able
    // to detect e.g. "1K", since parseInt() will parse this
    // string as '1' first.
    if(isNaN(val) || val < minval || val > maxval) {
        return GiveAlert("The value of " + start_text + " is restricted to " + minval + " - " + maxval + end_text, fld);
    } else {
        return true;
    }
}


//
// Input check for MAC address. Returns false if it isn't a valid MAC address else true
//

function IsMacAddress(Value,AlertOn)
{
    // Default value
    var  AlertOn = (AlertOn == null) ? 0 : AlertOn;

    // Split the max address up in 6 part. The format is 'xx-xx-xx-xx-xx-xx' or 'xx.xx.xx.xx.xx.xx' or 'xxxxxxxxxxxx' (x is a hexadecimal digit).
    if (Value.indexOf("-") != -1 || Value.indexOf(".") != -1 || Value.indexOf(":") != -1) {
        var MACAddr;
        if (Value.indexOf("-") != -1) {
	        MACAddr = Value.split("-");
        } else if (Value.indexOf(".") != -1) {
	        MACAddr = Value.split(".");
	    } else {
	        MACAddr = Value.split(":");
	    }

        if (MACAddr.length == 6) {
            for (var i = 0; i < MACAddr.length; i++) {
                if (MACAddr[i].length > 2) {
                    if (AlertOn) {
                        alert ("MAC address contains more than 2 digits");
                    }
                    return false;
                } else {
                    if (!IsHex(MACAddr[i],AlertOn)) {
                        return false;
                    }
                }
            }
        } else {
	        if (AlertOn) {
                alert ("MAC should have 6 digit groups");
            }
	        return false;
        }
    } else {
	    if (Value.length != 12) {
	        if (AlertOn) {
                alert ("MAC address must be 12 characters long");
            }
	        return false;
	    }

        if (!IsHex(Value,AlertOn)) {
            return false;
        }
    }

    return true;
}

function IsValidMacAddress(f)
{
    var ret;
    if(typeof(f) == "string")
        f = document.getElementById(f);
    if(!(ret = IsMacAddress(f.value, 1)))
        f.select();
    return ret;
}

// The main validation parts
function _isValidIpStr(val, is_mask, allow_what)
{
  var error = false;

  if(val.length == 0) {
    // User-input error
    error = true;
  } else {
    var myReg;
    var show_error = false;
    myReg = /^((?:0x)[0-9A-Fa-f]{1,2}|[0][0-7]{0,3}|[1-9]\d{0,2})\.((?:0x)[0-9A-Fa-f]{1,2}|[0][0-7]{0,3}|[1-9]\d{0,2})\.((?:0x)[0-9A-Fa-f]{1,2}|[0][0-7]{0,3}|[1-9]\d{0,2})\.((?:0x)[0-9A-Fa-f]{1,2}|[0][0-7]{0,3}|[1-9]\d{0,2})$/;
    if(myReg.test(val)) {
      var ip1, ip2, ip3, ip4;
      if(RegExp.$1[0] == "0" && RegExp.$1[1] != "x") {
        ip1 = parseInt(RegExp.$1,8);
      } else {
        ip1 = parseInt(RegExp.$1);
      }
      if(RegExp.$2[0] == "0" && RegExp.$2[1] != "x") {
        ip2 = parseInt(RegExp.$2,8);
      } else {
        ip2 = parseInt(RegExp.$2);
      }
      if(RegExp.$3[0] == "0" && RegExp.$3[1] != "x") {
        ip3 = parseInt(RegExp.$3,8);
      } else {
        ip3 = parseInt(RegExp.$3);
      }
      if(RegExp.$4[0] == "0" && RegExp.$4[1] != "x") {
        ip4 = parseInt(RegExp.$4,8);
      } else {
        ip4 = parseInt(RegExp.$4);
      }
      if(ip1 > 255 || ip2 > 255 || ip3 > 255 || ip4 > 255) {
        // At least one of ip1, ip2, ip3, or ip4 is not in interval 0-255.
        error = true;
      } else if(is_mask) {
        // Check that the mask is contiguous
        var ip_as_int = (ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4;
        var zero_found = false, i;
        for(i = 31; i >= 0; i--) {
          if((ip_as_int & (1 << i)) == 0) {
            // Cleared bit was found.
            zero_found = 1;
          } else if(zero_found) {
            // Set bit was found and cleared bit was previously found => Error
            error = true;
            break;
          }
        }
      } else {
        if(allow_what == 0 || allow_what == 1) {
          // Disallow x > 223, x == 127, and 0.x.x.x
          if(ip1 == 127 || ip1 > 223) {
            // ip1 indicates loopback (127), a multicast address (224-239), or an experimental address (240-255).
            error = true;
          } else if(ip1 == 0 && (ip2 != 0 || ip3 != 0 || ip4 != 0)) {
            // Class 0 is not allowed
            error = true;
          } else  if(allow_what == 1 && ip1 == 0 && ip2 == 0 && ip3 == 0 && ip4 == 0) {
            // 0.0.0.0 is not allowed
            error = true;
          }
        } else if(allow_what == 3) {
          // Disallow x < 223, x == 127, and 0.x.x.x
          if(ip1 == 127 || ip1 < 224 || ip1 > 239) {
            // ip1 indicates loopback (127), or an experimental address (240-255).
            error = true;
          } else if(ip1 == 0 && (ip2 != 0 || ip3 != 0 || ip4 != 0)) {
            // Class 0 is not allowed
            error = true;
          } else  if(ip1 == 0 && ip2 == 0 && ip3 == 0 && ip4 == 0) {
            // 0.0.0.0 is not allowed
            error = true;
          }
        }
      }
    } else {
      // Didn't match regexp
      error = true;
    }
  }

  return !error;
}

// Shorthand function
function _isValidIpAddress(val)
{
    return _isValidIpStr(val, false, 1);
}

// isIpStr()
// Function that checks that input value is a valid IP string (xxx.xxx.xxx.xxx).
// @fld_id:     is the name of the field. It's value is used as input in the
//              check. It's also used to set focus if an error occurs.
// @is_mask:    Set to true if the input must be a mask, i.e. all-ones on the left and all-zeros on the right of the boundary.
// @start_text: In case of an error, focus is set to @fld_id and one of the following strings is shown:
//              "The value of " + start_text + " is not a valid IP address.\nValid IP strings have the format 'xxx.xxx.xxx.xxx' where 'xxx' is a number between 0 and 255."
//              "The value of " + start_text + " is not a valid IP mask.\nValid IP masks have the format 'xxx.xxx.xxx.xxx' where 'xxx' is a number between 0 and 255."
// @allow_what. Used only if @is_mask is false. 'Enumeration' of which IP addresses (x.y.z.w) are allowed.
//      @allow_what    || Allow x > 223 | Allow x == 127 | Allow 0.x.x.x | Allow 0.0.0.0
//      ---------------||---------------|----------------|---------------|--------------
//      Undefined or 0 ||      No       |      No        |      No       |      Yes
//                   1 ||      No       |      No        |      No       |      No
//                   2 ||      Yes      |      Yes       |      Yes      |      Yes
//                   3 ||      Yes      |      No        |      No       |      No
function isIpStr(fld_id, is_mask, start_text, allow_what, ignore_alert)
{
    if(!allow_what || allow_what < 0) {
        // In case allow_what is undefined
        allow_what = 0;
    } else if (allow_what > 3) {
        allow_what = 2;
    }

    var fld = document.getElementById(fld_id);
    if(!fld) {
        // Programming error
        return GiveAlert("No such field: " + fld_id, fld);
    }

    var isok = _isValidIpStr(fld.value, is_mask, allow_what);
    if(!isok) {
        if(is_mask) {
            if (!ignore_alert) {
                GiveAlert("The value of " + start_text + " is not a valid IP mask.\n\n" +
                          "A valid IP mask is a dotted decimal string ('x.y.z.w'), where\n" +
                          "  1) x, y, z, and w are decimal numbers between 0 and 255,\n" +
                          "  2) when converted to a 32-bit binary string and read from left to right,\n"+
                          "     all bits following the first zero must also be zero.", fld);
            }
        } else {
            if(allow_what == 0 || allow_what == 1) {
                if (!ignore_alert) {
                    GiveAlert("The value of " + start_text + " must be a valid IP address in dotted decimal notation ('x.y.z.w').\n\n" +
                              "The following restrictions apply:\n" +
                              "  1) x, y, z, and w must be decimal numbers between 0 and 255,\n" +
                              "  2) x must not be 0" + (allow_what == 0 ? " unless also y, z, and w are 0" : "") + ",\n" +
                              "  3) x must not be 127, and\n" +
                              "  4) x must not be greater than 223.", fld);
                }
            } else if(allow_what == 3) {
                if (!ignore_alert) {
                    GiveAlert("The value of " + start_text + " must be a valid IP address in dotted decimal notation ('x.y.z.w').\n\n" +
                              "The following restrictions apply:\n" +
                              "  1) x must be a decimal number between 224 and 239,\n" +
                              "  2) y, z, and w must be decimal numbers between 0 and 255", fld);
                }
            } else {
                if (!ignore_alert) {
                    GiveAlert("The value of " + start_text + " must be a valid IP address in dotted decimal notation ('x.y.z.w'),\n" +
                              "where x, y, z, and w are decimal numbers between 0 and 255.", fld);
                }
            }
        }
    } else {
        // Get rid of insignificant preceding zeros - if any.
        // Well, in fact, if a string is preceded by a zero, then it's perceived as an octal number (e.g. parseInt("010") == 8)!!
        fld.value = parseInt(RegExp.$1) + "." + parseInt(RegExp.$2) + "." + parseInt(RegExp.$3) + "." + parseInt(RegExp.$4);
    }

    return isok;
}

function isIpAddrZero(addr_id, is_mask)
{
    var ipa = 0;
    var fld = document.getElementById(addr_id);

    if (!fld) {
        return GiveAlert("No such field: " + addr_id, fld);
    } else {
        var isok = _isValidIpStr(fld.value, is_mask, 0);
        if (!isok) {
            if (is_mask) {
                return GiveAlert("Invalid IPv4 Mask: " + addr_id, fld);
            } else {
                return GiveAlert("Invalid IPv4 Address: " + addr_id, fld);
            }
        }

        ipa = parseInt(RegExp.$1);
        ipa = ipa << 8;
        ipa |= parseInt(RegExp.$2);
        ipa = ipa << 8;
        ipa |= parseInt(RegExp.$3);
        ipa = ipa << 8;
        ipa |= parseInt(RegExp.$4);
    }

    if (ipa) {
        return false;
    } else {
        return true;
    }
}

function isIpAddrsLocalConnected(addr1_id, addr2_id, mask_id)
{
    var fld, isok;
    var ip1, ip2, ipm;

    fld = document.getElementById(addr1_id);
    if (!fld) {
        return GiveAlert("No such field: " + addr1_id, fld);
    } else {
        isok = _isValidIpStr(fld.value, false, 0);
        if (!isok) {
            return GiveAlert("Invalid IPv4 Address: " + addr1_id, fld);
        }

        ip1 = parseInt(RegExp.$1);
        ip1 = ip1 << 8;
        ip1 |= parseInt(RegExp.$2);
        ip1 = ip1 << 8;
        ip1 |= parseInt(RegExp.$3);
        ip1 = ip1 << 8;
        ip1 |= parseInt(RegExp.$4);
    }

    fld = document.getElementById(addr2_id);
    if (!fld) {
        return GiveAlert("No such field: " + addr2_id, fld);
    } else {
        isok = _isValidIpStr(fld.value, false, 0);
        if (!isok) {
            return GiveAlert("Invalid IPv4 Address: " + addr2_id, fld);
        }

        ip2 = parseInt(RegExp.$1);
        ip2 = ip2 << 8;
        ip2 |= parseInt(RegExp.$2);
        ip2 = ip2 << 8;
        ip2 |= parseInt(RegExp.$3);
        ip2 = ip2 << 8;
        ip2 |= parseInt(RegExp.$4);
    }

    fld = document.getElementById(mask_id);
    if (!fld) {
        return GiveAlert("No such field: " + mask_id, fld);
    } else {
        isok = _isValidIpStr(fld.value, true, 0);
        if (!isok) {
            return GiveAlert("Invalid IPv4 Mask: " + mask_id, fld);
        }

        ipm = parseInt(RegExp.$1);
        ipm = ipm << 8;
        ipm |= parseInt(RegExp.$2);
        ipm = ipm << 8;
        ipm |= parseInt(RegExp.$3);
        ipm = ipm << 8;
        ipm |= parseInt(RegExp.$4);
    }

    if ((ip1 & ipm) != (ip2 & ipm)) {
        alert("The IP address and the router are not on the same subnet");
        return false;
    }

    return true;
}

/* Return true IFF Valid && Unspecified */
function isIpv6AddrUnspecified(fld_id)
{
    var fld = document.getElementById(fld_id);
    if(!fld) {
        /* Programming error */
        return false;
    }

    var idx, myReg, inputTextCheck, pass = 1, inputText = fld.value.split("::");

    if (inputText.length > 2) {
        pass = 0;
    } else if (inputText.length == 1) {
        /* x:x:x:x:x:x:x:x */
        myReg = /[^0\:]/;
        if (myReg.test(fld.value)) {
            pass = 0;
        } else {
            inputTextCheck = inputText[0].split(":");
            if (inputTextCheck.length != 8) {
                pass = 0;
            } else {
                for (idx = 0; idx < inputTextCheck.length; idx++) {
                    if ((inputTextCheck[idx].length < 1) || (inputTextCheck[idx].length > 4)) {
                        pass = 0;
                        break;
                    }
                }
            }
        }
    } else if (inputText.length == 2) {
        myReg = /[^0\:]/;
        if (myReg.test(fld.value)) {
            return false;
        }

        if (fld.value == "::") {
            return true;
        }

        /* x::x */
        var addrTokenCnt = (inputText[0].split(":")).length + (inputText[1].split(":")).length;
        inputTextCheck = inputText[0].split(":");
        if (inputTextCheck[0] == "") {
            addrTokenCnt--;
        }
        inputTextCheck = inputText[1].split(":");
        if (inputTextCheck[0] == "") {
            addrTokenCnt--;
        }

        /*
            IPv6 ZERO-Compression consumes at least 1 token
            Thus
            #TokenMax.=7
            #TokenMin.=1
        */
        if ((addrTokenCnt > 0) && (addrTokenCnt < 8)) {
            inputTextCheck = inputText[0].split(":");
            if (inputTextCheck[0] != "") {
                for (idx = 0; idx < inputTextCheck.length; idx++) {
                    if ((inputTextCheck[idx].length < 1) || (inputTextCheck[idx].length > 4)) {
                        pass = 0;
                        break;
                    }
                }
            }

            inputTextCheck = inputText[1].split(":");
            if (pass && (inputTextCheck[0] != "")) {
                for (idx = 0; idx < inputTextCheck.length; idx++) {
                    if ((inputTextCheck[idx].length < 1) || (inputTextCheck[idx].length > 4)) {
                        pass = 0;
                        break;
                    }
                }
            }
        } else {
            pass = 0;
        }
    } else {
        pass = 0;
    }

    if (pass) {
        return true;
    } else {
        return false;
    }
}

/*
    allow_what:
    0->Allow any valid address
    1->unicast address only
    2->multicast address only
    3->unicast address including unspecified
    4->unicast address except link-local
    5->unicast IPv4-Mapped
    6->unicast address except loopback

    Follow RFC-4291 & RFC-5952
*/
function isIpv6Str(fld_id, caller_text, allow_what, ignore_alert)
{
    var fld = document.getElementById(fld_id);
    if(!fld) {
        /* Programming error */
        return GiveAlert("No such field: " + fld_id, fld);
    }

    var idx, myReg, inputTextCheck, inputText = fld.value.split("::");
    if (inputText[0].charAt(0) == ":") {
        if (!ignore_alert) {
            GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                      "\nIs not a valid IPv6 address.", fld);
        }

        return false;
    }

    if (inputText.length > 2) {
        if (!ignore_alert) {
            GiveAlert("The value of " + caller_text + " (" + fld.value + ")" +
                      "\nMust be a valid IPv6 address.\n" +
                      "The symbol '::' can appear only once.", fld);
        }

        return false;
    } else if (inputText.length == 1) {
        inputTextCheck = inputText[0].split(":");

        /* x:x:x:x:x:x:x:x */
        if (inputTextCheck[0].length == 4) {
            if (inputTextCheck[0].charAt(0).toLowerCase() == "f" && inputTextCheck[0].charAt(1).toLowerCase() == "f") {
                if ((allow_what != 0) && (allow_what != 2)) {
                    if (!ignore_alert) {
                        GiveAlert("Using IPv6 multicast address is not allowed here.\n", fld);
                    }

                    return false;
                }
            } else {
                if (allow_what == 2) {
                    if (!ignore_alert) {
                        GiveAlert("Using IPv6 unicast address is not allowed here.\n", fld);
                    }

                    return false;
                }

                if (isIpv6AddrUnspecified(fld_id)) {
                    if ((allow_what == 0) || (allow_what == 3)) {
                        return true;
                    } else {
                        if (!ignore_alert) {
                            GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                                      "\nIs not a valid IPv6 address." +
                                      "\nThe Unspecified Address must never be assigned to any node.", fld);
                        }

                        return false;
                    }
                }

                if (allow_what == 4) {
                    if (inputTextCheck[0].charAt(0).toLowerCase() == "f" && inputTextCheck[0].charAt(1).toLowerCase() == "e" && inputTextCheck[0].charAt(2) == "8" && inputTextCheck[0].charAt(3) == "0") {
                        if (!ignore_alert) {
                            GiveAlert("Using IPv6 link-local address is not allowed here.\n", fld);
                        }

                        return false;
                    }
                }

                if (allow_what == 5) {
                    if (!ignore_alert) {
                        GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                                  "\nIs not a valid IPv4-Mapped address.", fld);
                    }

                    return false;
                }
            }
        } else {
            if (allow_what == 2) {
                if (!ignore_alert) {
                    GiveAlert("Using IPv6 unicast address is not allowed here.\n", fld);
                }

                return false;
            }

            if (isIpv6AddrUnspecified(fld_id)) {
                if ((allow_what == 0) || (allow_what == 3)) {
                    return true;
                } else {
                    if (!ignore_alert) {
                        GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                                  "\nIs not a valid IPv6 address." +
                                  "\nThe Unspecified Address must never be assigned to any node.", fld);
                    }

                    return false;
                }
            }

            if (allow_what == 5) {
                if (!ignore_alert) {
                    GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                              "\nIs not a valid IPv4-Mapped address.", fld);
                }

                return false;
            }
        }

        myReg = /^([\da-fA-F]{1,4}\:){7}[\da-fA-F]{1,4}$/;

        if (myReg.test(fld.value)) {
            return true;
        }
    } else if (inputText.length == 2) {
        myReg = /[^\da-fA-F\:\.]/;
        if (myReg.test(fld.value)) {
            if (!ignore_alert) {
                GiveAlert("The value of " + caller_text + " (" + fld.value + ")" +
                          "\nMust be a valid IPv6 address in 128-bit records represented as" +
                          "\nEight fields of up to four hexadecimal digits with a colon (:) separating each field.", fld);
            }

            return false;
        }

        /* Pick up loopback address ::1 */
        if (inputText[0].length == 0) {
            inputTextCheck = inputText[1].split(":");
            inputTextCheck2 = inputText[1].split(".");
            if (inputTextCheck.length == 1 && inputTextCheck2.length == 1) {
                if (allow_what == 6) {
                    if (parseInt(inputTextCheck, 16) == 1) {
                        if (!ignore_alert) {
                            GiveAlert("The input value " + caller_text + " (" + fld.value + ")"+ "using IPv6 loopback address is not allowed here.\n", fld);
                        }
                        return false;
                    }
                }
            }
        }

        inputTextCheck = inputText[0].split(":");

        /* x::x */
        if (inputTextCheck[0].length == 4) {
            if (inputTextCheck[0].charAt(0).toLowerCase() == "f" && inputTextCheck[0].charAt(1).toLowerCase() == "f") {
                if ((allow_what != 0) && (allow_what != 2)) {
                    if (!ignore_alert) {
                        GiveAlert("Using IPv6 multicast address is not allowed here.\n", fld);
                    }

                    return false;
                }
            } else {
                if (allow_what == 2) {
                    if (!ignore_alert) {
                        GiveAlert("Using IPv6 unicast address is not allowed here.\n", fld);
                    }

                    return false;
                }

                if (isIpv6AddrUnspecified(fld_id)) {
                    if ((allow_what == 0) || (allow_what == 3)) {
                        return true;
                    } else {
                        if (!ignore_alert) {
                            GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                                      "\nIs not a valid IPv6 address." +
                                      "\nThe Unspecified Address must never be assigned to any node.", fld);
                        }

                        return false;
                    }
                }

                if (allow_what == 4) {
                    if (inputTextCheck[0].charAt(0).toLowerCase() == "f" && inputTextCheck[0].charAt(1).toLowerCase() == "e" && inputTextCheck[0].charAt(2) == "8" && inputTextCheck[0].charAt(3) == "0") {
                        if (!ignore_alert) {
                            GiveAlert("Using IPv6 link-local address is not allowed here.\n", fld);
                        }

                        return false;
                    }
                }

                if (allow_what == 5) {
                    if (!ignore_alert) {
                        GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                                  "\nIs not a valid IPv4-Mapped address.", fld);
                    }

                    return false;
                }
            }

            myReg = /[\.]/;
            if (myReg.test(fld.value)) {
                if (allow_what == 2) {
                    if (!ignore_alert) {
                        GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                                  "\nIs an invalid IPv6 multicast address.", fld);
                    }
                } else {
                    if (!ignore_alert) {
                        GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                                  "\nIs not a valid IPv6 address.", fld);
                    }
                }

                return false;
            }
        } else {
            if (allow_what == 2) {
                if (!ignore_alert) {
                    GiveAlert("Using IPv6 unicast address is not allowed here.\n", fld);
                }

                return false;
            }

            if (isIpv6AddrUnspecified(fld_id)) {
                if ((allow_what == 0) || (allow_what == 3)) {
                    return true;
                } else {
                    if (!ignore_alert) {
                        GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                                  "\nIs not a valid IPv6 address." +
                                  "\nThe Unspecified Address must never be assigned to any node.", fld);
                    }

                    return false;
                }
            }

            myReg = /[\.]/;
            if (myReg.test(fld.value)) {
                /* Check for IPv4 Compatible/Mapped Address ONLY */
                inputTextCheck = inputText[1].split(".");
                myReg = /[^\dfF\:\.]/;
                if (!myReg.test(fld.value) && (inputTextCheck.length == 4)) {
                    var addr4TokenCnt = (inputText[0].split(":")).length + (inputText[1].split(":")).length;
                    inputTextCheck = inputText[0].split(":");
                    if (inputTextCheck[0] == "") {
                        addr4TokenCnt--;
                    }
                    inputTextCheck = inputText[1].split(":");
                    if (inputTextCheck[0] == "") {
                        addr4TokenCnt = 0;  /* Use 0 to present invalid */
                    }

                    /*
                        IPv6 ZERO-Compression consumes at least 1 token + 1 for IPv4-Format
                        Thus
                        #TokenMax.=6
                        #TokenMin.=1
                    */
                    if ((addr4TokenCnt > 0) && (addr4TokenCnt < 7)) {
                        var pass4 = 1;

                        /* Left side of :: should be all zero */
                        inputTextCheck = inputText[0].split(":");
                        if (inputTextCheck[0] != "") {
                            for (idx = 0; idx < inputTextCheck.length; idx++) {
                                if ((inputTextCheck[idx].length < 1) || (inputTextCheck[idx].length > 4)) {
                                    pass4 = 0;
                                    break;
                                }

                                if (parseInt(inputTextCheck[idx], 16)) {
                                    pass4 = 0;
                                    break;
                                }
                            }
                        }

                        /* Right side of :: should be 0:a.b.c.d OR 0:ffff:a.b.c.d */
                        if (pass4) {
                            var prev4, mapFound = 0;
                            inputTextCheck = inputText[1].split(":");

                            for (idx = 0; idx < inputTextCheck.length; idx++) {
                                if(_isValidIpStr(inputTextCheck[idx], false, 1)) {
                                    if ((idx + 1) != inputTextCheck.length) {
                                        /* The last token is not IPv4 */
                                        pass4 = 0;
                                    }

                                    break;
                                } else {
                                    if (mapFound) {
                                        /* FFFF should be adjacent to IPv4 */
                                        pass4 = 0;
                                        break;
                                    }
                                }

                                if ((inputTextCheck[idx].length < 1) || (inputTextCheck[idx].length > 4)) {
                                    pass4 = 0;
                                    break;
                                }

                                prev4 = parseInt(inputTextCheck[idx], 16);
                                if (prev4) {
                                    if (prev4 != 0xFFFF) {
                                        pass4 = 0;
                                        break;
                                    } else {
                                        mapFound = 1;
                                    }
                                }
                            }
                        }

                        if (pass4) {
                            return true;
                        }
                    }
                }

                if (!ignore_alert) {
                    GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                              "\nIs not a valid IPv4-Mapped address.", fld);
                }

                return false;
            } else {
                if (allow_what == 5) {
                    if (!ignore_alert) {
                        GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                                  "\nIs not a valid IPv4-Mapped address.", fld);
                    }

                    return false;
                }
            }
        }

        var addrTokenCnt = (inputText[0].split(":")).length + (inputText[1].split(":")).length;
        inputTextCheck = inputText[0].split(":");
        if (inputTextCheck[0] == "") {
            addrTokenCnt--;
        }
        inputTextCheck = inputText[1].split(":");
        if (inputTextCheck[0] == "") {
            addrTokenCnt--;
        }

        /*
            IPv6 ZERO-Compression consumes at least 1 token
            Thus
            #TokenMax.=7
            #TokenMin.=1
        */
        if ((addrTokenCnt > 0) && (addrTokenCnt < 8)) {
            var pass6 = 1;

            myReg = /[\da-fA-F]{1,4}/;

            inputTextCheck = inputText[0].split(":");
            if (inputTextCheck[0] != "") {
                for (idx = 0; idx < inputTextCheck.length; idx++) {
                    if ((inputTextCheck[idx].length < 1) || (inputTextCheck[idx].length > 4)) {
                        pass6 = 0;
                        break;
                    }

                    if (!myReg.test(inputTextCheck[idx].value)) {
                        pass6 = 0;
                        break;
                    }
                }
            }

            inputTextCheck = inputText[1].split(":");
            if (pass6 && (inputTextCheck[0] != "")) {
                for (idx = 0; idx < inputTextCheck.length; idx++) {
                    if ((inputTextCheck[idx].length < 1) || (inputTextCheck[idx].length > 4)) {
                        pass6 = 0;
                        break;
                    }

                    if (!myReg.test(inputTextCheck[idx].value)) {
                        pass6 = 0;
                        break;
                    }
                }
            }

            if (pass6) {
                return true;
            }
        }
    } else {
        if (!ignore_alert) {
            GiveAlert("The input value " + caller_text + " (" + fld.value + ")" +
                      "\nIs not a valid IPv6 address.", fld);
        }

        return false;
    }

    if (!ignore_alert) {
        GiveAlert("The value of " + caller_text + " (" + fld.value + ")" +
                  "\nMust be a valid IPv6 address in 128-bit records represented as" +
                  "\nEight fields of up to four hexadecimal digits with a colon (:) separating each field.", fld);
    }

    return false;
}

function isIpAddr(fld, text)
{
    return isIpStr(fld, false, text, 1, false);
}

function isIpNet(fld, text)
{
    return isIpStr(fld, false, text, 1, false);
}

function isIpMask(fld, text)
{
    return isIpStr(fld, true, text, 1, false);
}

function IpRangeDiff(ip1, ip2)
{
    var i, diff;
    ip1 = ip1.split('.');
    ip2 = ip2.split('.');
    for(diff = i = 0; i < ip1.length; i++) {
        var d1 = parseInt(ip1[i]) - parseInt(ip2[i]);
        diff <<= 8;
        diff += d1;
    }
    return diff;
}

function IpLarger(ip1, ip2)     // ip1 > ip2
{
    var res = IpRangeDiff(ip1, ip2) > 0;
    return res;
}

function IpSmaller(ip1, ip2)    // ip1 < ip2
{
    var res = IpRangeDiff(ip1, ip2) < 0;
    return res;
}

function Ip6AdrsCmp(ip1, ip2)
{
    var idx, adr, len, zc, v4;
    var pos, cnt, adr_buf;
    var adr1 = Array();
    var adr2 = Array();
    var cmp;

    /* ip1 & ip2 MUST be valid IPv6 address string */

    /* GET IP1 */
    zc = 0;
    adr = ip1.split('::');
    if (adr.length === 2) {
        zc = 1;
    }
    v4 = 0;
    adr = ip1.split('.');
    if (adr.length === 4) {
        v4 = 1;
    }

    adr = ip1.split(':');
    len = adr.length;
    cnt = 0;
    for (pos = 0; pos < 8; pos++) {
        adr1[pos] = 0;
    }
    if (v4) {
        len--;
        adr_buf = adr[len].split('.');

        adr1[7] = parseInt(adr_buf[2], 10);
        adr1[7] <<= 8;
        adr1[7] += parseInt(adr_buf[3], 10);
        adr1[6] = parseInt(adr_buf[0], 10);
        adr1[6] <<= 8;
        adr1[6] += parseInt(adr_buf[1], 10);

        cnt += 2;
    }
    if (zc) {
        cnt = 8 - ((len + cnt) - 1);
    }
    for (pos = idx = 0; idx < len; idx++) {
        if (adr[idx] === "") {
            if (idx) {
                for (zc = 0; zc < cnt; zc++) {
                    adr1[pos + zc] = 0;
                }

                pos += (cnt - 1);
            } else {
                adr1[pos] = 0;
            }
        } else {
            adr1[pos] = parseInt(adr[idx], 16);
        }

        pos++;
    }
    /* GET IP2 */
    zc = 0;
    adr = ip2.split('::');
    if (adr.length === 2) {
        zc = 1;
    }
    v4 = 0;
    adr = ip2.split('.');
    if (adr.length === 4) {
        v4 = 1;
    }

    adr = ip2.split(':');
    len = adr.length;
    cnt = 0;
    for (pos = 0; pos < 8; pos++) {
        adr2[pos] = 0;
    }
    if (v4) {
        len--;
        adr_buf = adr[len].split('.');

        adr2[7] = parseInt(adr_buf[2], 10);
        adr2[7] <<= 8;
        adr2[7] += parseInt(adr_buf[3], 10);
        adr2[6] = parseInt(adr_buf[0], 10);
        adr2[6] <<= 8;
        adr2[6] += parseInt(adr_buf[1], 10);

        cnt += 2;
    }
    if (zc) {
        cnt = 8 - ((len + cnt) - 1);
    }
    for (pos = idx = 0; idx < len; idx++) {
        if (adr[idx] === "") {
            if (idx) {
                for (zc = 0; zc < cnt; zc++) {
                    adr2[pos + zc] = 0;
                }

                pos += (cnt - 1);
            } else {
                adr2[pos] = 0;
            }
        } else {
            adr2[pos] = parseInt(adr[idx], 16);
        }

        pos++;
    }

    /* CMP IP1 & IP2 */
    cmp = 0;
    for (idx = 0; idx < 8; idx++) {
        if (adr1[idx] > adr2[idx]) {
            cmp = 1;
            break;
        } else {
            if (adr1[idx] < adr2[idx]) {
                cmp = -1;
                break;
            }
        }
    }

    return cmp;
}

function Ip6Smaller(ip1, ip2)   // ip1 < ip2
{
    var res = (Ip6AdrsCmp(ip1, ip2) < 0);
    return res;
}

function Ip6Equal(ip1, ip2)     // ip1 == ip2
{
    var res = (Ip6AdrsCmp(ip1, ip2) == 0);
    return res;
}

function Ip6Larger(ip1, ip2)    // ip1 > ip2
{
    var res = (Ip6AdrsCmp(ip1, ip2) > 0);
    return res;
}

function IpRangeOverlapping(range1, range2)
{
    var x1, x2, y1, y2;
    range1 = range1.split('-');
    range2 = range2.split('-');
    x1 = range1[0];
    x2 = range1[1];
    y1 = range2[0];
    y2 = range2[1];
    if(IpLarger(y1, x2) ||      // y1 > x2 || y2 < x1
       IpSmaller(y2, x1))
        return false;
    return true;
}

function isValidIpRange(f, fname)
{
    if(typeof(f) == "string")
        f = document.getElementById(f);
    var ipAddr = f.value.split("-");
    if(ipAddr.length != 2)
        return GiveAlert(fname + " must contain two IP address separated by a \"-\"", f);
    if(!_isValidIpAddress(ipAddr[0]))
        return GiveAlert("First IP address in " + fname + " is invalid", f);
    if(!_isValidIpAddress(ipAddr[1]))
        return GiveAlert("Second IP address in " + fname + " is invalid", f);
    if(IpLarger(ipAddr[0], ipAddr[1]))
        return GiveAlert("Second IP address must be larger than first", f);
    return true;
}

function isEmpty(f, errmsg)
{
    if(typeof(f) == "string")
        f = document.getElementById(f);
    if(f && f.value.length == 0)
        return !GiveAlert(errmsg, f);
}

function trimInput(f)
{
    if(typeof(f) == "string")
        f = document.getElementById(f);
    if(f) {
        var str = f.value;
        str = str.replace(/^\s+/, "");
        str = str.replace(/\s+$/, "");
        f.value = str;
    }
    return f;
}

function _isValidHostname(hostname, ignore_null)
{
    /* The hostname is defined in RFC 1123 and 952. But we implement the below scenario to fit with eCos DNS package.
       A valid hostname is a string drawn from the alphabet (A-Za-z), digits (0-9), dot (.), hyphen (-).
       Spaces are not allowed, the first character must be an alphanumeric character,
       and the first and last characters must not be a dot or a hyphen.
    */
    if (ignore_null && hostname.length == 0) {
        return true;
    } else if (hostname.match(/^[-\.A-Za-z\d]+$/) && hostname.match(/^[A-Za-z\d]/) && hostname.match(/[A-Za-z\d]$/)) {
        return true;
    }
    return false;
}

// isValidHostOrIP()
// Function that checks that input value is a valid Hostname, IPv4 unicast address or (if enabled) IPv6 unicast address.
// @fid:        Field id.
// @errmsg:     String to use in error message.
function isValidHostOrIP(fid, errmsg)
{
    var fld = document.getElementById(fid);

    if (!fld) {
        return GiveAlert("No such field: " + fid, fld);
    }

    if (isEmpty(fld, errmsg + " must be a valid hostname or IP address")) {
        return false;
    }

    if (_isValidIpStr(fld.value, false, 2)) {
        return isIpStr(fid, 0, errmsg, 1); // Verify valid IPv4 unicast address
    } else if (configIPv6Support && isIpv6Str(fid, errmsg, 0, true)) {
        return isIpv6Str(fid, errmsg, 1); // Verify valid IPv6 unicast address
    } else if (_isValidHostname(fld.value)) {
        return true;
    }
    return GiveAlert(errmsg + " must be a valid hostname or IP address", fld);
}

function isValidDNS(f, errmsg)
{
    if(typeof(f) == "string")
        f = document.getElementById(f);
    if(isEmpty(f, errmsg))
        return false;
    if(!_isValidHostname(f.value))
        return GiveAlert(errmsg, f);
    return true;
}

// Checks whether testStr only contains of characters in containStr
function onlyContains(teststr, containstr)
{
    var testStr=teststr;
    var containStr=containstr;
    for(var i=0;i<testStr.length;i++) {
        if(containStr.indexOf(testStr.charAt(i))==-1)
            return false;
    }
    return true;
}

// Checks whether testStr does not contain any characters in containStr
function doesNotContain(testStr, containStr)
{
    for(var i=0;i<testStr.length;i++) {
        if(containStr.indexOf(testStr.charAt(i))!=-1)
            return false;
    }
    return true;
}

function _isValidEmail(address)
{
	return address.match(/^[^@\s]+@([^@\s]+\.)+[^@\s]+$/);
}

function _isValidAtHost(address)
{
	return address.match(/^@([^@\s]+)$/);
}

function _isValidDistinguishedName(dn)
{
    var keys = dn.split(/\s*[,;]\s*/);
    for(var i = 0; i < keys.length; i++) {
        var key = keys[i];
        if(!key.match(/^\s*(CN|L|ST|O|OU|C)\s*=\s*.+/i)) {
            //alert("Bad part: " + key);
            return false;
        }
    }
    return true;
}

// Check that field.value is a valid identity
function IsValidIdentity(f, fname)
{
    if(typeof(f) == "string")
        f = document.getElementById(f);

    if((f.value.length == 0) ||
       _isValidHostname(f.value) ||
       _isValidIpStr(f.value, false, 1) ||
       _isValidEmail(f.value) ||
       _isValidAtHost(f.value) ||
       _isValidDistinguishedName(f.value))
	return true;

    return GiveAlert(fname + ' must be a valid identity. I.e. a valid IP address, DNS name, email-address or ASN.1 distinguished name.', f);
}

// Check that field.value is a valid string
function IsValidString(field, badchars, maxlen, what)
{
    if(typeof(field) == "string")
        field = document.getElementById(field);

    if (field.value.length == 0)
        return GiveAlert(what + " cannot be empty", field);

    if (field.value.length > maxlen)
        return GiveAlert(what + " must not exceed " + maxlen + " characters", field);

    for (var i = 0; i < badchars.length; i++)
        if (field.value.indexOf(badchars.charAt(i)) != -1)
            return GiveAlert(what + " contains one or more illegal characters", field);

    return true;
}

// Check that field.value is a valid, non-zero integer
function IsValidIntNonZero(f, fname)
{
    if(typeof(f) == "string")
        f = document.getElementById(f);

    if(f && f.value) {
        if(f.value.match(/^\d+$/)) {
            if(parseInt(f.value))
                return parseInt(f.value);
            else
                return GiveAlert(fname + " must be non-zero.", f);
        } else
            return GiveAlert(fname + " must only contain digits.", f);
    }
    else
        return GiveAlert(fname + " must be a valid integer", f);
}

/* When using this func, html file should include "<script type="text/javascript" src="lib/config.js"></script>" */
function isHostName(fld_id, fld_text, max_length, allow_what)
{
  var fld = document.getElementById(fld_id);

  if (!fld) {
    return false;
  }

  // User-input error
  if (max_length && fld.value.length > max_length) {
    return GiveAlert("The length of " + fld_text + " is restricted to " + max_length, fld);
  } else if (fld.value.length > 45) {
    return GiveAlert("The length of " + fld_text + " is restricted to 45", fld);
  }

  if (/^(((?:0x)[0-9A-Fa-f]{1,2}|[0][0-7]{0,3}|[1-9][0-9]{0,2})\.){3}((?:0x)[0-9A-Fa-f]{1,2}|[0][0-7]{0,3}|[1-9][0-9]{0,2})$/.test(fld.value)) {
    if (!isIpStr(fld_id, false, fld_text, allow_what, false)) {
      return false;
    }
  } else if (configIPDNSSupport) {
    if (fld.value.length && !(_isValidHostname(fld.value, 1))) {
      return GiveAlert("The format of " + fld_text + " is invalid.\n\n" +
                       "It must either be a valid IP address in dotted decimal notation ('x.y.z.w') or a valid hostname.\n" +
                       "A valid hostname is a string drawn from the alphabet (A-Za-z), digits (0-9), dot (.), hyphen (-).\n" +
                       "Spaces are not allowed, the first character must be an alphanumeric character,\n" +
                       "and the first and last characters must not be a dot or a hyphen.\n", fld);
    }
  } else {
    return GiveAlert("The format of " + fld_text + " is invalid.\n\n" +
                     "It must be a valid IP in dotted decimal notation ('x.y.z.w').\n", fld);
  }
  return true;
}

//
// Input check for OUI address. Returns false if it isn't a valid OUI address else true
//

function IsOuiAddress(Value, AlertOn)
{
    // Default value
    var  AlertOn = (AlertOn == null) ? 0 : AlertOn;

    if (Value == "00-00-00") {
        if (AlertOn) {
            alert ("The null OUI address isn't allowed");
        }
        return false;
    }

    // Split the max address up in 3 part ( Allowed format is 00-11-22
    if (Value.indexOf("-") != -1) {
	    var OuiAddr = Value.split("-");

        if (OuiAddr.length == 3) {
            for (var i = 0; i < OuiAddr.length; i++) {
                if (OuiAddr[i].length > 2) {
                    if (AlertOn) {
                        alert ("OUI address contains more than 2 digits");
                    }
                    return false;
                } else {
                    if (!IsHex(OuiAddr[i], AlertOn)) {
                        return false;
                    }
                }
            }
        } else {
	        if (AlertOn) {
                alert ("OUI should have 3 digit groups");
            }
	        return false;
        }

    } else {
        if (AlertOn) {
            alert ("Allowed format is 'xx-xx-xx' (x is a hexadecimal digit)");
        }
        return false;
    }

    return true;
}

function isMacStr(str) {
    if (!str) {
        return false;
    }
    var myReg = /^([A-Fa-f0-9]{1,2}[-]){5}[A-Fa-f0-9]{1,2}$/;
    return myReg.test(str);
}
