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
// **********************************  QOS_UTIL.JS  ********************************
// *
// * Author: Joergen Andreasen
// *
// * --------------------------------------------------------------------------
// *
// * Description:  Common QoS JavaScript functions.
// *
// * To include in HTML file use:
// *
// * <script type="text/javascript" src="lib/qos_util.js"></script>
// *
// * --------------------------------------------------------------------------

/*
 * Calculate the actual weight in percent.
 * This calculation includes the round off errors that is caused by the
 * conversion from weight to cost that is done in the API.
 * See TN1049.
 *
 * param weight     [IN]   Array of weights (integer 1..100)
 * param nr_of_bits [IN]   Nr of bits in cost (5 bits on L26, Serval and Jaguar line ports, 7 bits on Jaguar host ports)
 * param pct        [OUT]  Array of percent (integer 1..100)
 *
 * return true if conversion is possible, otherwise false.
 */

function qos_weight2pct(weight, nr_of_bits, pct)
{
    var i;
    var cost       = [];
    var new_weight = [];
    var w_min      = 100;
    var c_max_arch = Math.pow(2, nr_of_bits);
    var c_max      = 0;
    var w_sum      = 0;

    // Check input parameters and save the lowest weight for use in next round
    for (i = 0; i < weight.length; i++) {
        if (isNaN(weight[i]) || (weight[i] < 1) || (weight[i] > 100)) {
            return false;  // Bail out on invalid weight
        }
        else {
            w_min = Math.min(w_min, weight[i]);
        }
    }

    for (i = 0; i < weight.length; i++) {
        cost[i] = Math.max(1, Math.round((c_max_arch * w_min) / weight[i])); // Calculate cost for each weight (1..c_max_arch)
        c_max = Math.max(c_max, cost[i]); // Save the highest cost for use in next round
    }

    for (i = 0; i < weight.length; i++) {
        new_weight[i] = Math.round(c_max / cost[i]); // Calculate back to weight
        w_sum += new_weight[i]; // Calculate the sum of weights for use in next round
    }

    for (i = 0; i < weight.length; i++) {
        pct[i] = Math.max(1, Math.round((new_weight[i] * 100) / w_sum)); // Convert new_weight to percent (1..100)
    }
    return true;
}
