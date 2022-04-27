#!/usr/bin/perl
#
# Vitesse Switch Software.
#
# Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

use File::Path;
use Getopt::Std;

my $err_cnt = 0;

# Figure out whether jsl (which is MUUUCH faster) exists on this system
my $use_jsl = length(`jsl 2>/dev/null`) > 0;

if (!$use_jsl) {
  print STDERR "Error: 'jsl' not found. Contact jslint makefile maintainer to get your shell set-up correctly. Resorting to javascript-based linter.\n";
}

foreach my $file (@ARGV) {
  my $lint_result;

  if ($use_jsl) {
	$lint_result = `jsl -nologo -nofilelisting -conf ../make/jslint.conf -process $file |
	                grep -v "0 error(s), 0 warning(s)" |
					grep -v ": unable to resolve path" |
					grep -v "error(s).*warning(s)" |
					grep -v "js.*can't open file"`;
	$lint_result =~ s/^\n//g;
  } else {
    # Use the sloooow JavaScript-based jslint, which uses java as interpreter :(
    $lint_result = `java -jar /usr/share/java/rhino1_7R2/js.jar /import/dk_config/scripts/jslint.js $file`;
	$lint_result = '' if $lint_result =~ m/No problems found in/;
  }

  # If there was a warning/error print out
  if ($lint_result eq '') {
    # Print out no error found
    print "No problems found in $file\n";
  } else {
    $err_cnt++;
    print STDERR "\n******* $file *******\n";
    print STDERR "$lint_result\n";
  }
}

if ($err_cnt > 0) {
  print STDERR ("Total warning/error files: $err_cnt\n");
}

