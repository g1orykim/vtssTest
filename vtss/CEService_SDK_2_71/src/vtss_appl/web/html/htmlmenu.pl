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
use CGI qw/:standard *table *Tr *td *thead *tbody *ul *li *div/;
use Getopt::Long;
use Config;

sub preamble {
    print start_html(-title=>"Index",
                     -script=>[
                          {'src'=>'lib/mootools-core.js'},
                          {'src'=>'lib/mootools-more.js'},
                          {'src'=>'lib/config.js'},
                          {'src'=>'lib/menu.js'},
                          {'src'=>'lib/navbarupdate.js'},
                          {'src'=>'lib/ajax.js'},
                     ],
                     -style=> [
                          {'src'=>'lib/menu.css'},
                     ],
        ), "\n";
    
    print 
        start_form({-action=>"#"});
    
    print
        start_table({-id=>"menu",
                     -summary=>"Main navigation menu"}), "\n";
    
    # Use invalid SIDs for selector values below, in order to force
    # SpomNavigationLoading() to return true while not really loaded yet.
    my ($sel) = Select({-onchange=>"stackSelect(this);",
                        -id => "stackselect"},
                       option({-value=>-1}, "Switch 1"),
                       option({-value=>99}, "Switch 99:M"));
    
    # Stacking selector - invisible when standalone
    print
        Tr ( {-class=>"selector", 
              -id=>"selrow",
              -style=>"display:none"},
             td ( $sel ), 
             td ({-width=>"100%"},
                 img({-alt=>"Refresh Stack List",
                      -onclick=>"DoStackRefresh();",
                      -align=>"bottom",
                      -title=>"Refresh Stack List",
                      -src=>"images/refresh.gif"}) 
             ) ), "\n";
    
    print 
        start_Tr, start_td({-colspan=>"2"}), 
        start_ul({-class=>"level0"}), "\n";
}

sub postamble {
    print 
        " ", 
        end_ul(), "\n",
        end_td, 
        end_Tr, "\n",
        end_table, "\n",
        "</form>",  "\n",
        end_html(), "\n";
}

sub process_menu {
    my($input) = @_;
    my(@trail) = (0);

  line:
    while(<$input>) {
        next line if(/^\#/);
        chop;
        if(/^(\s*)(.+)$/) {
            my($level, $line) = ($1, $2);
            my($menu, $url, $target) = split(",", $line);
            $target = "main" unless($target);
            #$url = "" unless($url);
            #print STDERR "$level|$menu|$url|$target\n";
            
            my ($depth) = length($level);
            if($depth > scalar(@trail)-1) {
                #print STDERR "Down, level $depth\n";
                print 
                    " " x @trail, 
                    br, "\n",
                    " " x @trail, 
                    start_ul({
                        -class=>sprintf("submenu level%d", scalar @trail),
                             }), "\n";
                push @trail, 1; # Push a new level
            } elsif($depth < scalar(@trail)-1) {
                #print STDERR "Up, level $depth\n";
                while($depth != (scalar(@trail)-1)) {
                    print 
                        " " x @trail, 
                        end_ul, 
                        end_li, 
                        "\n";
                    pop @trail; # Pop back a level
                }
                $trail[-1]++; 
            } else {
                $trail[-1]++;
            }
            my($cell);
            if(!$url) {
                $cell = start_li({});
                $cell .= a({-class=>"actuator",
                            -target=>"_self",
                            -href=>"#"}, $menu );
            } else {
                $cell = li({-class=>"link"},a({-target=>"$target", -id=>"$url", -href=>"$url"}, $menu));
            }
            print $level, "  ", $cell, "\n";
        }
    }

    while(@trail > 1) {
        print 
            " " x @trail, 
            end_ul, 
            end_li, 
            "\n";
        pop @trail; # Pop back a level
    }
    
}

my $cpp = $ENV{CPP} || $Config{cppstdin} || "/usr/bin/cpp";

preamble();
my(@opts, @files);
for my $arg (@ARGV) {
    if(-r $arg) {
        push @files, $arg;
    } else {
        $arg =~ s/ /\\ /g;     # Quote spaces
        push @opts, $arg;
    }
}
my($cmd) = "cat " . join(" ", @files);
$cmd .= " | $cpp " . join(" ", @opts) . " -" if(@opts);
#print STDERR "CMD = '$cmd'\n";
open(I, $cmd . "|") || die "$cmd: $!";
process_menu(\*I);
postamble();
