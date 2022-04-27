#
# Vitesse Switch Software.
#
# Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
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
#!/usr/bin/env perl

use strict;
use Data::Dumper;
use Crypt::Rijndael;

sub sak_update_hash_key {
    my ($sak) = @_;
    my $key = pack "H*", $sak->{'buf'};
    my $cipher = Crypt::Rijndael->new($key, Crypt::Rijndael::MODE_CBC());
    my @zeroes = (0) x 16;
    my $input = pack("C*", @zeroes);
    my $crypted = $cipher->encrypt($input);
    my @out = unpack "H*", $crypted;
    $sak->{'h_buf'} = @out[0];
    return $sak;
}

if (scalar(@ARGV) != 1) {
    print "Usage: hash_calc.pl <KEY>\n";
    exit 1;
}

if (length($ARGV[0]) != 32 and length($ARGV[0]) != 64) {
    print "Key length must either be 32 or 64 chars (128bits or 256bits)\n";
    exit 1;
}

my $sak_rx_sa_0 = { "len" => length($ARGV[0])/2,
                    "buf" => $ARGV[0]};



$sak_rx_sa_0 = sak_update_hash_key($sak_rx_sa_0);
print "$sak_rx_sa_0->{'h_buf'}\n";

#Usage from tclsh 
#%exec perl ./hash_calc.pl 00000000000000000000000000000000
#Returns: key


