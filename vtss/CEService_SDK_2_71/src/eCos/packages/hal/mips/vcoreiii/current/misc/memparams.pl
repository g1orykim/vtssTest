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
#!/usr/bin/perl -w

use warnings;
use strict;
use Getopt::Std;
use POSIX;                      # ceil
use Data::Dumper;

my (%opts) = ( 
    n => 4.8,                   # DDR clock timing in ns (half speed of CPU) 4.0~500MHz 4.8~416MHz
    );

sub getParams {
    my($type, $clk_ns) = @_;
    my %params;
    $params{clk_ns} = $clk_ns;
    $params{_400_ns_dly_ns} = 400; # Automatically converted to _400_ns_dly
    # Actual RAM used
    if($type == 1) {
        $params{model} = "Micron 333MHz x16 SG3";
        $params{mode} = 0 ;# DDR2
        $params{minperiod_ns} = 3.75 ;# with the CL that is specified below!
        $params{CL} = 4 ;# we do not use AL: AL is good for increasing BW - bad for latency!
        $params{tREFI_ns} = 7800.0;
        $params{tWR_ns} = 15.0;
        $params{tRAS_min_ns} = 40.0;
        $params{tWTR_ns} = 7.5;
        $params{tRRD_ns} = 10.0;
        $params{tFAW_ns} = 0.0 ;# only for 8bank chip (set to 0.0 when 4bank)
        $params{tRC_ns} = 55.0;
        $params{tRP_ns} = 15.0;
        $params{tRPA_ns} = 0.0 ;# only for 8bank chip {set to 0.0 when 4bank)
        $params{tRCD_ns} = 15.0;
        $params{tMRD} = 2;
        $params{tRFC_ns} = 75.0;
        $params{row_addr_cnt} = 13;
        $params{bank_addr_cnt} = 2;
        $params{col_addr_cnt} = 9;
    } elsif($type == 2) {
        $params{model} = "Micron 2Gb MT47H128M16-37E 16Meg x 16 x 8 banks, DDR-533\@CL4";
        $params{mode} = 0 ;# DDR2
        $params{CL} = 4 ;# we do not use AL: AL is good for increasing BW - bad for latency!
        $params{minperiod_ns} = 3.75 ;# with the CL that is specified above!
        $params{tRC_ns} = 55.0;
        $params{tRCD_ns} = 15.0;
        $params{tRAS_min_ns} = 40.0;
        $params{tRP_ns} = 15.0;
        $params{tRPA_ns} = 18.75 ;# only for 8bank chip {set to 0.0 when 4bank)
        $params{tRRD_ns} = 10.0;
        $params{tFAW_ns} = 50.0 ;# only for 8bank chip (set to 0.0 when 4bank)
        $params{tWR_ns} = 15.0;
        $params{tWTR_ns} = 7.5;
        $params{tMRD} = 2;
        $params{tRFC_ns} = 197.5;
        $params{tREFI_ns} = 7800.0;
        $params{row_addr_cnt} = 14;
        $params{bank_addr_cnt} = 3;
        $params{col_addr_cnt} = 10;
    } elsif($type == 3) {
        $params{model} = "Micron 1Gb MT47H128M8-25E 16Meg x 8 x 8 banks, DDR-533\@CL4";
        $params{mode} = 0 ;# DDR2
        $params{CL} = 4 ;# we do not use AL: AL is good for increasing BW - bad for latency!
        $params{minperiod_ns} = 3.75 ;# with the CL that is specified above!
        $params{tRC_ns} = 55.0;
        $params{tRCD_ns} = 12.5;
        $params{tRAS_min_ns} = 40.0;
        $params{tRP_ns} = 12.5;
        $params{tRPA_ns} = 15.0 ;# only for 8bank chip {set to 0.0 when 4bank)
        $params{tRRD_ns} = 7.5;
        $params{tFAW_ns} = 35.0 ;# only for 8bank chip (set to 0.0 when 4bank)
        $params{tWR_ns} = 15.0;
        $params{tWTR_ns} = 7.5;
        $params{tMRD} = 2;
        $params{tRFC_ns} = 127.5;
        $params{tREFI_ns} = 7800.0;
        $params{row_addr_cnt} = 14;
        $params{bank_addr_cnt} = 3;
        $params{col_addr_cnt} = 10;
    } elsif($type == 4) {
        $params{model} = "Micron 1Gb MT47H64M16-37E 8Meg x 16 x 8 banks, DDR-533\@CL4";
        $params{mode} = 0 ;# DDR2
        $params{CL} = 4 ;# we do not use AL: AL is good for increasing BW - bad for latency!
        $params{minperiod_ns} = 3.75 ;# with the CL that is specified above!
        $params{tRC_ns} = 55.0;
        $params{tRCD_ns} = 15;
        $params{tRAS_min_ns} = 40.0;
        $params{tRP_ns} = 15;
        $params{tRPA_ns} = 18.75 ;# only for 8bank chip {set to 0.0 when 4bank)
        $params{tRRD_ns} = 7.5;
        $params{tFAW_ns} = 37.5 ;# only for 8bank chip (set to 0.0 when 4bank)
        $params{tWR_ns} = 15.0;
        $params{tWTR_ns} = 7.5;
        $params{tMRD} = 2;
        $params{tRFC_ns} = 127.5;
        $params{tREFI_ns} = 7800.0;
        $params{row_addr_cnt} = 13;
        $params{bank_addr_cnt} = 3;
        $params{col_addr_cnt} = 10;
    } elsif($type == 5) {
        $params{model} = "Micron 1Gb MT47H64M16-3 8Meg x 16 x 8 banks, DDR-533\@CL4";
        $params{mode} = 0 ;# DDR2
        $params{CL} = 4 ;# we do not use AL: AL is good for increasing BW - bad for latency!
        $params{minperiod_ns} = 3.75 ;# with the CL that is specified above!
        $params{tRC_ns} = 55.0;
        $params{tRCD_ns} = 15;
        $params{tRAS_min_ns} = 40.0;
        $params{tRP_ns} = 15;
        $params{tRPA_ns} = 18 ;# only for 8bank chip {set to 0.0 when 4bank)
        $params{tRRD_ns} = 10;
        $params{tFAW_ns} = 50 ;# only for 8bank chip (set to 0.0 when 4bank)
        $params{tWR_ns} = 15.0;
        $params{tWTR_ns} = 7.5;
        $params{tMRD} = 2;
        $params{tRFC_ns} = 127.5;
        $params{tREFI_ns} = 7800.0;
        $params{row_addr_cnt} = 13;
        $params{bank_addr_cnt} = 3;
        $params{col_addr_cnt} = 10;
    } elsif($type == 6) {
        $params{model} = "Micron 1Gb MT47H128M8-3 16Meg x 8 x 8 banks, DDR-533\@CL4";
        $params{mode} = 0 ;# DDR2
        $params{CL} = 4 ;# we do not use AL: AL is good for increasing BW - bad for latency!
        $params{minperiod_ns} = 3.75 ;# with the CL that is specified above!
        $params{tRC_ns} = 55.0;
        $params{tRCD_ns} = 15;
        $params{tRAS_min_ns} = 40.0;
        $params{tRP_ns} = 15;
        $params{tRPA_ns} = 18 ;# only for 8bank chip {set to 0.0 when 4bank)
        $params{tRRD_ns} = 7.5;
        $params{tFAW_ns} = 37.5 ;# only for 8bank chip (set to 0.0 when 4bank)
        $params{tWR_ns} = 15.0;
        $params{tWTR_ns} = 7.5;
        $params{tMRD} = 2;
        $params{tRFC_ns} = 127.5;
        $params{tREFI_ns} = 7800.0;
        $params{row_addr_cnt} = 14;
        $params{bank_addr_cnt} = 3;
        $params{col_addr_cnt} = 10;
    } elsif($type == 7) {
        $params{model} = "Micron 1Gb MT47H128M8-25 16Meg x 8 x 8 banks, DDR-533\@CL4";
        $params{mode} = 0 ;# DDR2
        $params{CL} = 4 ;# we do not use AL: AL is good for increasing BW - bad for latency!
        $params{minperiod_ns} = 3.75 ;# with the CL that is specified above!
        $params{tRC_ns} = 55.0;
        $params{tRCD_ns} = 15;
        $params{tRAS_min_ns} = 40.0;
        $params{tRP_ns} = 15;
        $params{tRPA_ns} = 17.5 ;# only for 8bank chip {set to 0.0 when 4bank)
        $params{tRRD_ns} = 7.5;
        $params{tFAW_ns} = 35.0 ;# only for 8bank chip (set to 0.0 when 4bank)
        $params{tWR_ns} = 15.0;
        $params{tWTR_ns} = 7.5;
        $params{tMRD} = 2;
        $params{tRFC_ns} = 127.5;
        $params{tREFI_ns} = 7800.0;
        $params{row_addr_cnt} = 14;
        $params{bank_addr_cnt} = 3;
        $params{col_addr_cnt} = 10;
    } elsif($type == 8) {
        $params{model} = "Micron 2Gb MT47H128M16-3 16Meg x 16 x 8 banks, DDR-533\@CL4";
        $params{mode} = 0 ;# DDR2
        $params{CL} = 4 ;# we do not use AL: AL is good for increasing BW - bad for latency!
        $params{minperiod_ns} = 3.75 ;# with the CL that is specified above!
        $params{tRC_ns} = 55.0;
        $params{tRCD_ns} = 15;
        $params{tRAS_min_ns} = 40.0;
        $params{tRP_ns} = 15;
        $params{tRPA_ns} = 18 ;# only for 8bank chip {set to 0.0 when 4bank)
        $params{tRRD_ns} = 7.5;
        $params{tFAW_ns} = 50.0 ;# only for 8bank chip (set to 0.0 when 4bank)
        $params{tWR_ns} = 15.0;
        $params{tWTR_ns} = 7.5;
        $params{tMRD} = 2;
        $params{tRFC_ns} = 197.5;
        $params{tREFI_ns} = 7800.0;
        $params{row_addr_cnt} = 14;
        $params{bank_addr_cnt} = 3;
        $params{col_addr_cnt} = 10;
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
            my ($t) = int(ceil($params{$param}/$clk_ns));
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

getopts("D:n:", \%opts);

my (%rams) = ( 
    'CYG_HAL_VCOREIII_CHIPTYPE_JAGUAR' => 8,
    'CYG_HAL_VCOREIII_CHIPTYPE_LUTON26' => 6,
    );

for my $t (sort keys %rams) {
    my ($p) = getParams($rams{$t}, $opts{n});
    # Convert parameters in NS to clock equivalents
    $p = convertParams($p);

    printf("\n\#elif defined(%s)\n\n", $t);
    printf("/* %s \@ %.2fns */\n", $p->{model}, $p->{clk_ns});
    emit($p, qw(bank_addr_cnt row_addr_cnt col_addr_cnt));
    emit($p, qw(tREFI));
    emit($p, qw(tRAS_min CL tWTR));
    emit($p, qw(tRC tFAW tRP tRRD tRCD));
    emit($p, qw(tRPA tRP tMRD tRFC _400_ns_dly));
    emit($p, qw(tWR));
}

sub emit {
    my($params, @names) = @_;
    for my $name (@names) {
        printf("\#define %-25.30s %s\n", "VC3_MPAR_" . $name, $params->{$name});
    }
}
