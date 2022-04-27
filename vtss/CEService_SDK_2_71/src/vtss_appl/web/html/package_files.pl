#!/bin/perl -w
# 
# Vitesse Switch API software.
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

use File::Find;
use File::Compare;
use Getopt::Std;
use File::Temp qw(tempfile);
use File::Path;
use File::Spec;
use File::Basename;

my ($canzip) = 1;
eval "use Compress::Zlib;";
if($@) {
    $canzip = 0;
    warn "WARNING: Unable to zip assets, generated binary will bloat!";
}

my ($files, $inbytes, $outbytes, $zipbytes) = (0) x 4;

my(%opt) = ( );
getopts("dvo:C:", \%opt);       # -c,-m is obsoleted

our ($topdir, $custdir);

# Exceptions to default MIME creation
our (%ext2mime) = (
                   'jpg' => 'image/jpeg',
                   );

sub file2asset {
    my($fn) = @_;
    $fn =~ s|[/\.-]|_|g;
    return '__content__' . $fn;
}

sub preprocess {
    my($fn, $fd) = @_;
    open(I, '<:raw', $fn) || die "$fn: $!";
    my($line) = 0;
line:
    while(<I>) {
        $line++;
        # Loose full comments
        if(m|^\s*//| || m|^<!--.+-->$|) {
            print STDERR "Skip: $_" if($opt{d});
            next line;
        }
        s/^\s+//;              # Squash leading spaces
        s/\r$//;                # CRLF -> NL
        s|\s*//[a-z0-9 ]+$||i;   # Strip comments parts
        print $fd $_;
    }
    my $len = tell($fd);
    seek($fd, 0, 0);
    $len;
}

sub emit_hex {
    my($asset, $buffer, $zeroterminate) = @_;

    $buffer .= "\0" if($zeroterminate);

    print <<"EOT"
static cyg_uint8 $asset [] = \{
EOT
    ;

    my($offset) = 0;
    while(my $chunk = substr($buffer, $offset, 16)) {
        print "\t", join(", ", 
                         map { sprintf "%#02x", $_ }
                         unpack("C*", $chunk)), ", \n";
        $offset += length($chunk);
    }
    print "\};\n";
    $outbytes += length($buffer);
}

sub emit_asset {
    my($zeroterminate, $fn, $asset) = @_;
    my($fd, $assetlen);

    if($fn =~ /\.(html?|css|js|xml|svg)/ && !($fn =~ /acknowledgments\.htm$/)) {
        $fd = tempfile();
        $assetlen = preprocess($fn, $fd);
    } else {
        open($fd, '<:raw', $fn) || die "$fn: $!";
        $assetlen = -s $fn;
    }

    my($assetbin, $len);
    if(($len = sysread($fd, $assetbin, $assetlen)) != $assetlen) {
        die "$fn: expected $assetlen, got $len";
    }
    close($fd);

    $inbytes += $assetlen;
    
    if($canzip && $fn =~ /\.(html?|css|js|xml)/) {
        my($assetzip) = Compress::Zlib::memGzip($assetbin);
        my($assetziplen) = length($assetzip);
        $zipbytes += $assetziplen;
        emit_hex($asset, $assetzip, $zeroterminate);
        printf STDERR "$fn: $assetlen, $assetziplen zipped\n" if($opt{d});
    } else {
        emit_hex($asset, $assetbin, $zeroterminate);
        printf STDERR "$fn: $assetlen\n" if($opt{d});
    }
    $files++;
}

sub emit_content_data {
    my($fn, $nfn, $asset, $url, $ext) = @_;

    emit_asset(0, $fn, $asset);

    my($mime);
    if(!($mime = $ext2mime{$ext})) {
        $mime = "image/$ext";   # Default
    }

    print <<"EOT"

CYG_HTTPD_IRES_TABLE_ENTRY( ${asset}_data,
                            "${url}",
                            ${asset},
                            sizeof(${asset}) );

EOT
    ;
}

sub process {
    if(-f $_ ) {
        my($fn, $nfn) = $_;
        ($nfn = $fn) =~ s/^\Q$topdir\E//;
        my($asset) = file2asset($nfn);
        my($url) = $nfn;        # Straigt mapping (normalized) file name => URL
        return if(m!lib/(config|navbarupdate).js$!); # config is "dynamic" in real life
        return if(m|grocx_|);    # Skip G-RocX files
        if($custdir) {
            my ($custfile) = File::Spec->catfile($custdir, $nfn);
            if(-f $custfile) {
                print STDERR "Customized file: Map $fn -> $custfile\n" if($opt{v});
                $fn = $custfile;
            }
        }
        if(/\.(html?|css|js|xml|svg)$/) {
            emit_content_data($fn, $nfn, $asset, $url, lc($1));
        } elsif(/\.(png|gif|ico|jpe?g)$/) {
            emit_content_data($fn, $nfn, $asset, $url, lc($1));
        }
    }
}

my ($out);
if($opt{o}) {
    if(-r $opt{o}) {
        $out = $opt{o} . ".new";
    } else {
        $out = $opt{o};
    }
    open(STDOUT, '+>', $out) || die("$out: $!");
}

print <<"EOT"
#include <network.h>
#include <cyg/athttpd/http.h>
#include <cyg/athttpd/handler.h>

EOT
;

# 'overlay' HTML files
$custdir = $opt{C};
print STDERR "Customization dir: $custdir\n" if($custdir && $opt{v});
die "$custdir: $!" if($custdir && !-d $custdir);

# Package up the stuff
for $topdir (@ARGV) {
    die("Not a directory: $topdir\n") unless(-d $topdir);
    find({ wanted => \&process, no_chdir => 1 }, $topdir);
}

my ($notzipped) = ($outbytes - $zipbytes); # PNG, gif, etc already zipped
my ($zipped) = $inbytes - $notzipped;
printf STDERR 
    "Packaged %d files, Size %d => to %d bytes (Factor %.2f, %d bytes already compressed)\n", 
    $files, $inbytes, $outbytes, $zipbytes ? $zipped/$zipbytes : 1, $notzipped;

if($out && $out ne $opt{o}) {
    open(STDOUT, ">/dev/console"); # To close the former...
    if (compare($out, $opt{o}) == 0) {
        print STDERR "$opt{o} unchanged\n";
        unlink($out);
    } else {
        rename($out, $opt{o}) || die("rename($out, $opt{o}): $!");
    }
}
