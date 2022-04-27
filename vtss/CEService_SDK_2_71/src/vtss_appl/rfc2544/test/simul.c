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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENCAP               4 /**< Number of encapsulation bytes                                     */
#define FS_C_MIN           64 /**< Start frame size on customer-facing side                          */
#define FS_C_MAX        17000 /**< Double frame size for each iteration. Stop when above this number */
#define R_D_C_MIN     1000000 /**< Minimum rate on customer-facing side, bps                         */
#define R_D_C_STEP    1000000 /**< Maximum rate on customer-facing side, bps                         */
#define R_D_C_MAX  1000000000 /**< Stop when frame rate above this number                            */

typedef signed long long   i64;
typedef unsigned long long u64;
typedef int                i32;
typedef unsigned int       u32;

#define MAX_DEPTH 10
static int max_depth;

#define PS_PER_SEC  1000000000000LLU /**< Picoseconds per second */
#define PS_PER_TICK 198400LLU        /**< Picoseconds per tick   */

/****************************************************************************/
// serval_float()
/****************************************************************************/
static double serval_float(u64 *ticks, u32 depth)
{
    double fps_real_a_n = 0.0;  // Actual number of frames per second, network, as computed with floats [fps]
    u32    i;

    for (i = 0; i < depth; i++) {
        fps_real_a_n += (double)PS_PER_SEC / ((double)ticks[i] * (double)PS_PER_TICK);  // Actual number of frames per second, network [fps]
    }

    return fps_real_a_n;
}

/****************************************************************************/
// serval_int()
/****************************************************************************/
static double serval_int(u32 fps_d_n)
{
    u64 ticks[MAX_DEPTH]; // List of ticks.
    u64 fps_a_n = 0;      // Actual number of frames per second, network, as computed with integers [fps]
    u32 depth   = 0;

    memset(ticks, 0, sizeof(ticks));

    while (fps_a_n < fps_d_n && depth < MAX_DEPTH) {
        u64 fps_missing_d = fps_d_n - fps_a_n;
        u64 fps_missing_a;

        ticks[depth] =  PS_PER_SEC / (fps_missing_d * PS_PER_TICK); // Ticks programmed to H/W

        if (ticks[depth] == 0) {
            printf("Error: 0 ticks?\n");
            exit(-1);
        }

        fps_missing_a = PS_PER_SEC / (ticks[depth]  * PS_PER_TICK);

        // Since we are truncating in the ticks computation, ticks will always be <= than had
        // we used floating point. This means that fps_missing_a will always be >= fps_missing_d.
        if (fps_missing_a < fps_missing_d) {
            printf("Error: Assumption didn't hold\n");
            exit(-2);
        }

        if (fps_missing_a > fps_missing_d) {
            // Try with a higher tick count, which will give a lower rate.
            ticks[depth]++;
            fps_missing_a = PS_PER_SEC / (ticks[depth] * PS_PER_TICK);
        }

        fps_a_n += fps_missing_a;
        depth++;
    }

    if (depth > max_depth) {
        max_depth = depth;
    }

    printf(" %5u %8llu %8llu %8llu %8llu", depth, ticks[0], ticks[1], ticks[2], ticks[3]);

    return serval_float(ticks, depth);
}

/****************************************************************************/
// ru()
// Up-rounding division of #a with #b.
/****************************************************************************/
static u32 ru(u32 a, u32 b)
{
    u32 result = a / b;

    if (a % b) {
        result++;
    }

    return result;
}

/****************************************************************************/
// mep()
/****************************************************************************/
static double mep(u32 fs_c, u32 r_d_c)
{
#define IFG       8
#define PREAMBLE 12
    u32    bpf_c    = 8 * (PREAMBLE + fs_c + IFG); // Bits per frame, customer                      [bpf]
    double fps_d_c  = (double)r_d_c / bpf_c;       // Desired number of frames per second, customer [fps]
    u32    fps_d_n  = ru(r_d_c, bpf_c);            // Desired number of frames per second, network  [fps]
    double fps_a_n  = serval_int(fps_d_n);         // Actual number of frames per second, network   [fps]
    double rate_err = (fps_a_n - fps_d_c) * bpf_c; // Rate error (positive if we send too much)     [bps]

    // Also show the actual rate on the network-facing side.
    u32    fs_n     = fs_c + ENCAP;                // Frame size, network                           [bytes]
    u32    bpf_n    = 8 * (PREAMBLE + fs_n + IFG); // Bits per frame, network                       [bits]
    double r_a_n    = bpf_n * fps_a_n;             // Actual rate, network                          [bps]

    printf(" %9.1lf %9.1lf %9.1lf %12.1lf", fps_d_c, fps_a_n, fps_a_n - fps_d_c, r_a_n);

    return rate_err;
#undef IFG
#undef PREAMBLE
}

/****************************************************************************/
// main()
/****************************************************************************/
int main(void)
{
    u32    fs_c;         // Desired frame size, customer                                  [bytes]
    u32    r_d_c;        // Desired rate, customer                                        [bps]
    double rate_abs_err; // Rate error. Positive if actual rate higher than desired rate. [bps]
    double rate_rel_err; // Rate error relative to desired rate.                          [%]
    double max_abs_err = 0.0, max_rel_err = 0.0;

    // The maximum allowed error is 0.5 permille of the absolute link speed.
    // The 0.5 permille is chosen from the fact that all step sizes are in permille
    // of the link speed, so an error of half of the resolution is desirable.
    double max_allowed_err = 0.5 /* 0.5 */ / 1000 /* permille */ * 1000000000 /* of link speed of 1Gbps */; // [bps]

    printf("fs_c  r_d_c      depth ticks[0] ticks[1] ticks[2] ticks[3] fps_d_c   fps_a_n   fps_err   r_a_n        rate err   rate_err\n");
    printf("----- ---------- ----- -------- -------- -------- -------- --------- --------- --------- ------------ ---------- --------\n");

    for (fs_c = FS_C_MIN; fs_c <= FS_C_MAX; fs_c *= 2) {
        for (r_d_c = R_D_C_MIN; r_d_c <= R_D_C_MAX; r_d_c += R_D_C_STEP) {
            printf("%5u %10u", fs_c, r_d_c);
            rate_abs_err = mep(fs_c, r_d_c);
            rate_rel_err = (rate_abs_err * 100) / r_d_c;

            printf(" %10.1lf %.2lf%%\n", rate_abs_err, rate_rel_err);

            if (rate_abs_err < 0) {
                rate_abs_err = -rate_abs_err;
            }

            if (rate_abs_err >= max_allowed_err) {
                printf("Error: error (%.1lf >= max_allowed_err (%1.lf)\n", rate_abs_err, max_allowed_err);
            }

            if (rate_abs_err > max_abs_err) {
                max_abs_err = rate_abs_err;
            }

            if (rate_rel_err > max_rel_err) {
                max_rel_err = rate_rel_err;
            }
        }
    }

    printf("Max. depth = %d, max_abs_error = %.1lf bps, max_rel_err = %.2lf%%\n\n", max_depth, max_abs_err, max_rel_err);
    return 0;
}

