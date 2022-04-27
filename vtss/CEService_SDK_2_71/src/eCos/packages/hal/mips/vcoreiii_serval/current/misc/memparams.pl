#
# Vitesse Switch Software.
#
# #####ECOSGPLCOPYRIGHTBEGIN#####
# -------------------------------------------
# This file is part of eCos, the Embedded Configurable Operating System.
# Copyright (C) 1998-2012 Free Software Foundation, Inc.
#
# eCos is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 or (at your option) any later
# version.
#
# eCos is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with eCos; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# As a special exception, if other files instantiate templates or use
# macros or inline functions from this file, or you compile this file
# and link it with other works to produce a work based on this file,
# this file does not by itself cause the resulting work to be covered by
# the GNU General Public License. However the source code for this file
# must still be made available in accordance with section (3) of the GNU
# General Public License v2.
#
# This exception does not invalidate any other reasons why a work based
# on this file might be covered by the GNU General Public License.
# -------------------------------------------
# #####ECOSGPLCOPYRIGHTEND#####
#
#
# Vitesse Switch Software.
#
# Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
# Rights Reserved.
#
# Unpublished rights reserved under the copyright laws of the United States of
# America, other countries and international treaties. Permission to use, copy,
# store and modify, the software and its source code is granted. Permission to
# integrate into other products, disclose, transmit and distribute the software
# in an absolute machine readable format (e.g. HEX file) is also granted.  The
# source code of the software may not be disclosed, transmitted or distributed
# without the written permission of Vitesse. The software and its source code
# may only be used in products utilizing the Vitesse switch products.
#
# This copyright notice must appear in any copy, modification, disclosure,
# transmission or distribution of the software. Vitesse retains all ownership,
# copyright, trade secret and proprietary rights in the software.
#
# THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
# INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR USE AND NON-INFRINGEMENT.
#
#!/usr/bin/perl -w

use warnings;
use strict;
use Getopt::Std;
use POSIX;                      # ceil
use Data::Dumper;

my (%opts) = ( 
    n => 3.2,                   # DDR625
    );

sub getParams {
    my($type, $clk_ns) = @_;
    my %params;
    $params{clk_ns} = $clk_ns;
    $params{_400_ns_dly_ns} = 400; # Automatically converted to _400_ns_dly
    # Actual RAM used
    if($type == 1) {            # Refboard
        $params{model} = "Hynix H5TQ1G63BFA (1Gbit DDR3, x16)";
        $params{mode} = 1;# DDR3
        $params{CL} = 6;
        $params{CWL} = 5;
        $params{minperiod_ns} = 1.50 ;# with the CL that is specified above!
        $params{tRCD_ns} = 13.5;
        $params{tRP_ns} = 13.5;
        $params{tRC_ns} = 49.5;
        $params{tRAS_min_ns} = 36;
        $params{tRRD} = 4;   # CHECK - max(4, 10ns)
        $params{tFAW_ns} = 50;  # NB: At DDR-800 speed (actual, not rated)
        $params{tWR_ns} = 15.0;
        $params{tWTR} = 4; # CHECK - max(4, 7.5ns)
        $params{tRFC_ns} = 110;
        $params{tMRD} = 4;
        $params{tREFI_ns} = 7800.0;
        $params{tXPR_ns} = 110+(10); # CHECK max(5, tRFC+10ns)
        $params{tMOD} = 12;       # CHECK max(12, 15ns)
        $params{row_addr_cnt} = 13;
        $params{bank_addr_cnt} = 3;
        $params{col_addr_cnt} = 10;
        $params{tDLLK} = 512;   # DLL locking time
    } elsif($type == 2) {       # Validation board
        $params{model} = "Micron MT41J128M16HA-15E:D (2Gbit DDR3, x16)";
        $params{mode} = 1;# DDR3
        $params{CL} = 5; # And CWL 5
        $params{CWL} = 5;
        $params{minperiod_ns} = 1.50 ;# with the CL that is specified above!
        $params{tRCD_ns} = 13.5;
        $params{tRP_ns} = 13.5;
        $params{tRC_ns} = 49.5;
        $params{tRAS_min_ns} = 36;
        $params{tRRD} = 4;   # CHECK - max(4, 10ns)
        $params{tFAW_ns} = 50;  # NB: At DDR-800 speed (actual, not rated)
        $params{tWR_ns} = 15.0;
        $params{tWTR} = 4; # CHECK - max(4, 7.5ns)
        $params{tRFC_ns} = 160;
        $params{tMRD} = 4;
        $params{tREFI_ns} = 7800.0;
        $params{tXPR_ns} = 160+(10); # CHECK max(5, tRFC+10ns)
        $params{tMOD} = 12;       # CHECK max(12, 15ns)
        $params{row_addr_cnt} = 14;
        $params{bank_addr_cnt} = 3;
        $params{col_addr_cnt} = 10;
        $params{tDLLK} = 512;   # DLL locking time
    } else {
        die "Unknown memory type: $type";
    }

    # check that the selected config can run as fast as required
    if ($params{minperiod_ns} > $params{clk_ns}) {
        die "Currently selected memory does not support the DDR2 frequency."
    }

    \%params;
}

sub convertParams {
    my($pref) = @_;
    my %params = %{$pref};

    # The memory parameters that are provided in 'ns' are converted to
    # clock cycles by ceiling divide. If there is also a corresponding
    # parmeters specified in 'ck' then the maximum is selected.
    my ($clk_ns) = $params{clk_ns};
    for my $param (keys %params) {
        if(!($param =~ /(minperiod_ns|clk_ns)/) && $param =~ /(.+)_ns$/) {
            my ($stem) = $1;
            my ($t);
            if($param eq "tREFI_ns") {
                $t = int(floor($params{$param}/$clk_ns));
            } else {
                $t = int(ceil($params{$param}/$clk_ns));
            }
            # check if a ck-cnt is also specified, if so - select max
            if (exists $params{$stem}) {
                $t = $t < $params{$stem} ? $params{$stem} : $t;
            }
            # store clock count
            $params{$stem} = $t;
            print STDERR "$param :: $stem = ", $params{$param}, " => ", $t, "\n" if($opts{D});
        }
    }

    \%params;
}

getopts("D:n:t:", \%opts);

for my $t ($opts{t}) {
    my ($p) = getParams($t, $opts{n});
    # Convert parameters in NS to clock equivalents
    $p = convertParams($p);

    printf("/* %s \@ %.2fns */\n", $p->{model}, $p->{clk_ns});
    emit($p, qw(bank_addr_cnt row_addr_cnt col_addr_cnt));
    emit($p, qw(tREFI));
    emit($p, qw(tRAS_min CL tWTR));
    emit($p, qw(tRC tFAW tRP tRRD tRCD));
    emit($p, qw(tMRD tRFC));
    if($p->{mode}) {
        emit($p, qw(CWL tXPR tMOD tDLLK));
    } else {
        emit($p, qw(tRPA _400_ns_dly));
    }
    emit($p, qw(tWR));
}

sub emit {
    my($params, @names) = @_;
    for my $name (@names) {
        printf("\#define %-25.30s %s\n", "VC3_MPAR_" . $name, $params->{$name});
    }
}
