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
package Vitesse::SignedImage;
$VERSION = "1.00";

use strict;
use warnings;

use Carp;
use Digest::HMAC_SHA1 qw(hmac_sha1);
use Data::Dumper;

use constant {
    TLV_ARCH       => 1,
    TLV_CHIP       => 2,
    TLV_IMGTYPE    => 3,
    TLV_IMGSUBTYPE => 4,
};

use constant {
    TLV_TYPE_DWORD     => 1,
    TLV_TYPE_STRING    => 2,
    TLV_TYPE_BINARY    => 3,
};

use constant {
    TLV_IMGTYPE_BOOT_LOADER => 1,
    TLV_IMGTYPE_SWITCH_APP  => 2,
};

use constant TRAILERCOOKIE => 0x64972FEB;

my ($signature) = "[(#)VtssImageTrailer(#)]"; # 24 bytes
my ($key) = "THjwbSx!w(yShw71-ShS18%153&|91jshjdi";

our (@typename) = qw(undef dword string binary);
our (@tlvname)  = qw(undef arch chip imgtype imgsubtype);

# OO interface

sub new
{
    my($class) =  shift;
    my $self = bless {}, $class;
    $self->{key} = shift || $key;
    #printf "Use key %s\n", $self->{key}; 
    $self;
}

sub fromfile
{
    my $self = shift;
    my $file = shift;
    
    open(F, '<:raw', $file) || croak("$file: $!");

    my($hmac);
    my($fsize) = -s $file;
    my($fdata);

    croak "$file: $!" unless(sysread(F, $fdata, $fsize) == $fsize);
    close(F);

    my $traileroff;
    if(($traileroff = index($fdata, $signature)) > 0) {
        #printf STDERR "Found $signature at offset $traileroff\n";
        my($fileblob) = substr($fdata, 0, $traileroff);
        my($trailer) = substr($fdata, $traileroff + length($signature));
        my($flen, $fhash, $tlen, $thash) = unpack("Va20Va20", $trailer);
        #print Dumper($flen, unpack("H*", $fhash), $tlen, unpack("H*", $thash));
        croak "header $flen != seen $traileroff does not match" if($flen != $traileroff);
        $hmac =  hmac_sha1($fileblob, $self->{key});
        #printf "Compare %s vs %s\n", unpack("H*", $fhash), unpack("H*", $hmac); 
        croak "Hash mismatch" if($fhash ne $hmac);
        $self->{data} = $fileblob;
        my ($trailerdata) = substr($trailer, 2*(4+20), $tlen);
        $hmac = hmac_sha1($trailerdata, $self->{key});
        croak sprintf("Trailer length error %d vs %d",$tlen, length($trailerdata)) if(length($trailerdata) != $tlen);
        if(length($trailerdata) == $tlen &&
           $hmac eq $thash &&
           $self->parsetrailer($trailerdata)) {
            return 1;
        } else {
             croak "Trailer corrupted";
        }
    }
    0;
}

sub setfile
{
    my $self = shift;
    my $file = shift;
    my($fsize) = -s $file;
    my($fdata);

    open(F, '<:raw', $file) || croak("$file: $!");
    croak "$file: $!" unless(sysread(F, $fdata, $fsize) == $fsize);
    close(F);

    $self->{data} = $fdata;
    1;
}

sub parsetrailer
{
    my $self = shift;
    my $data = shift;
data:
    while(length($data)) {
        my($type, $id, $dlen) = unpack("VVV", $data);
        $data = substr($data, 3*4); # Skip read parts
        if($type == TLV_TYPE_DWORD) {
            my($value) = unpack("V", $data);
            push @{$self->{tlv}}, [$type, $id, $value];
        } elsif($type == TLV_TYPE_STRING) {
            my($value) = substr($data, 0, $dlen-1);
            push @{$self->{tlv}}, [$type, $id, $value];
        } elsif($type == TLV_TYPE_BINARY) {
            my($value) = substr($data, 0, $dlen);
            push @{$self->{tlv}}, [$type, $id, $value];
        } else {
            carp sprintf("Skip invalid TLV type: %d, len %d", $type, $dlen);
        }
        printf STDERR "Have %d bytes, chopping %d\n", length($data), $dlen;
        $data = substr($data, $dlen); # Skip read parts
        #printf("%s left (%d bytes)\n", unpack("H*", $data), length($data));
    }
    1;
}

sub add_tlv {
    my $self = shift;
    my ($type, $id, $dlen, $value) = @_;
    push @{$self->{tlv}}, [$type, $id, $dlen, $value];
}

sub add_tlv_dword
{
    my $self = shift;
    my ($id, $value) = @_;
    $self->add_tlv(TLV_TYPE_DWORD, $id, $value);
}

sub add_tlv_string
{
    my $self = shift;
    my ($id, $value) = @_;
    $self->add_tlv(TLV_TYPE_STRING, $id, $value);
}

sub add_tlv_binary
{
    my $self = shift;
    my ($id, $value) = @_;
    $self->add_tlv(TLV_TYPE_BINARY, $id, $value);
}

sub writefile
{
    my $self = shift;
    my $out = shift;
    my $hmac;

    open(O, ">:raw", $out) || croak("$out: $!");
    # Output File trailer parts, HMAC
    syswrite( O, $self->{data} );
    syswrite( O, $signature);
    syswrite( O, pack("V", length $self->{data}));
    $hmac = hmac_sha1($self->{data}, $self->{key});
    syswrite( O, $hmac );
    #printf "Digest: %s\n", unpack("H*", $hmac); 

    my ($trailerdata);
    for my $t (@{$self->{tlv}}) {
        #print Dumper($t);
        my($type, $id, $value) = @{$t};
        if($t->[0] == TLV_TYPE_DWORD) {
            $trailerdata .= pack("VVVV", $type, $id, 4, $value);
        } elsif($t->[0] == TLV_TYPE_STRING) {
            my($dlen) = length($value)+1;
            $trailerdata .= pack("VVVa${dlen}", $type, $id, $dlen, $value);
        } elsif($t->[0] == TLV_TYPE_BINARY) {
            $trailerdata .= pack("VVV/a*", $type, $id, length($value), $value);
        } else {
            croak sprintf("Invalid TLV type: %d", $t->[0]);
        }
    }
    #print Dumper($trailerdata);
    $hmac = hmac_sha1($trailerdata, $self->{key});

    # Trailer parts, HMAC
    #printf STDERR "Trailer total %d\n", length $trailerdata;
    syswrite( O, pack("V", length $trailerdata));
    syswrite( O, $hmac );
    syswrite( O, $trailerdata );
    syswrite( O, pack("VV", TRAILERCOOKIE, 8 + length($trailerdata) + 2*(4+20) + length($signature)));
    #printf "TDigest: %s\n", unpack("H*", $hmac); 

    close(O);
}

## Misc helpers 

sub get_chiptarget 
{
    my($chip) = @_;

    my($num);
    open(I, '<', "../../vtss_api/include/vtss_init_api.h") || die($!);
    while(<I>) {
        if(/\s+VTSS_TARGET_(\w+)\s*=\s*(\w+)/ && $1 eq $chip) {
            $num = $2;
            last;
        }
    }
    close(I);

    return hex($num);
}

1;
