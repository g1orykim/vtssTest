#
# Vitesse Switch Software.
#
# Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
use strict;
use warnings;

use String::CRC::Cksum;
use File::Basename;
use File::Spec;
use Getopt::Std;

my(%opts) = (
              'f' => 'unmanaged.bin',
              );

getopts("f:o:t:C:T:F:P:", \%opts);

# -C <chipid>  - VSC chipset ID
# -F <flags>   -
# -T <target>  - Target CPU 1 = ARM, 2 = MIPS
# -P <product> - Product type ordinal

my($chipid) = get_chiptarget($opts{C});

die("Unable to determine chipid for $opts{C}") unless($chipid);

open(F, '<:raw', $opts{f}) || die("$opts{f}: $!");

my ($trailer);
# Construct trailer - has to be included in general CRC
if($opts{t}) {
    my ($tcksum) = String::CRC::Cksum->new;
    my($type) = hex($opts{t});
    my($flags) = ($opts{F} ? hex($opts{F}) : 0);
    my($product, $stack) = split(":", $opts{P} || "0:undefined");
    $product = hex($product);
    $product |= 0x80 if($stack eq "STACKABLE");
    
    printf "Adding type trailer: 0x%x, flags %0x, arch %d, product: 0x%02x, chip VSC%04x\n", 
    $type, $flags, $opts{T}, $product, $chipid;
    $trailer = pack("V6CCvV",
                    0xadfacade, # Magic cookie
                    0xead34bc3, # Auth key / Signed CRC placeholder
                    1,          # Version
                    $type,      #
                    $flags,     #
                    1,          # 0 = Image uses fixed flash layout, 1 = image attempts to use FIS to get flash sections and falls back to fixed flash layout (can't use Version field above, since the current app checks that it is == 1)
                    # Architecture, Product, chip type (2bytes little-endian)
                    $opts{T}, $product, $chipid,
                    0);   # Spare
    $tcksum->add($trailer);
    my ($tcrc, $tlen) = $tcksum->result();
    substr($trailer, 4, 4) = pack("V", $tcrc); # replace authkey signed CRC
    $trailer = pack("V", $tlen) . $trailer; # Prepend length
}

my ($cksum) = String::CRC::Cksum->new;
$cksum->addfile(\*F);
$cksum->add($trailer) if($trailer);
my ($res, $len) = $cksum->result();
$len -= length($trailer) if($trailer);
printf("%s: 0x%08x, len %d+%d\n", $opts{f}, $res, $len, length($trailer));

# Construct output file
my ($name,$path,$suffix) = fileparse($opts{f},qw(\.\w+));
my ($out) = File::Spec->catfile($path, $name . '.dat');
$out = $opts{o} if($opts{o});
open(O, ">:raw", $out) || die("$out: $!");

my $header = pack("VV", $res, $len);
syswrite( O, $header );

seek(F, 0, 0);
my($i, $rc, $buf);
$i = 0;
while(my $cr = sysread(F,$buf,4*1024)) {
    syswrite( O, $buf, $cr );
}

# Optional trailer
syswrite( O, $trailer ) if($trailer);

close(F);
close(O);

print "Wrote $out\n";

sub get_chiptarget 
{
    my($chip) = @_;

    my($num);
    open(I, '<', "../../vtss_api/include/vtss_init_api.h") || die($!);
    while(<I>) {
        if(/\s+VTSS_TARGET_(\w+)\s*=\s*(\w+)/ && $1 eq $chip) {
            print STDERR "$chip: TARGET $1 = $2\n" if($opts{v});
            $num = $2;
            last;
        }
    }
    close(I);

    return hex($num);
}
