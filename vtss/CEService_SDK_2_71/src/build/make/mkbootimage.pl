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

use File::Basename;
use File::Spec;
use Getopt::Std;

use Vitesse::SignedImage;

my(%opts) = ( 'T' => 1,
              'f' => 'redboot.bin' );

getopts("C:T:k:f:", \%opts);

# -f file      - File to sign
# -T <target>  - Target CPU 1 = ARM, 2 = MIPS
# -C <CHIP>    - CHIP type
# -k <string>  - HMAC key

my($chipid) = Vitesse::SignedImage::get_chiptarget($opts{C});
die("Unable to determine chipid for $opts{C}") unless($chipid);

my($image) = Vitesse::SignedImage->new($opts{k});
$image->setfile($opts{f}) || die("$opts{f}: $!");

printf "Adding trailer: Arch %d, Chip %x\n", $opts{T}, $chipid;

# Construct output file
my ($name,$path,$suffix) = fileparse($opts{f},qw(\.\w+));
my ($out) = File::Spec->catfile($path, $name . '.img');
$out = $opts{o} if($opts{o});

$image->add_tlv_dword(Vitesse::SignedImage::TLV_ARCH, $opts{T});
$image->add_tlv_dword(Vitesse::SignedImage::TLV_CHIP, $chipid);
$image->add_tlv_dword(Vitesse::SignedImage::TLV_IMGTYPE, 
                      Vitesse::SignedImage::TLV_IMGTYPE_BOOT_LOADER);
$image->writefile($out) || die("$out: $!");

print "Wrote $out\n";

