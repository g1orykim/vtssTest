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

use Getopt::Std;
use Vitesse::SignedImage;

my(%opts) = ();

getopts("vk:", \%opts);

# -k <string>  - HMAC key

my($image) = Vitesse::SignedImage->new($opts{k});

my($file) = $ARGV[0];

die "Provide file to read as input" unless(-f $file);

$image->fromfile($file) || die("Invalid image");

print "Image decoded OK\n";

if($opts{v}) {
    print "Image length = ", length($image->{data}), "\n";
    for my $tlv (@{$image->{tlv}}) {
        my($type, $id, $value) = @{$tlv};
        print 
            $Vitesse::SignedImage::typename[$type], " ",
            $Vitesse::SignedImage::tlvname[$id], " ", $value, "\n";
    }
}
