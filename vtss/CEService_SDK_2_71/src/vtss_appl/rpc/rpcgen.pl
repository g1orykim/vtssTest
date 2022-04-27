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
use strict;
use warnings;

use XML::Simple;
use Data::Dumper;
use Template;
use Getopt::Std;

our %opts = ( 
    'o' => \*STDOUT,
    's' => "rpc.xml",
    'I' => ".",
    ) ;
getopts('o:s:I:', \%opts) || die("Invalid options");

my $tree = XMLin($opts{s}, keyattr => [], forcearray => [ qw(group entry param type elm event include) ]);

sub ref_arg {
    my ($param, $deref) = @_;
    return "" if($param->{'array'});
    return is_struct($param->{'type'}) || $param->{'out'} ? $deref : "";
}

sub msgid {
    my ($arg) = shift;
    return "MSG_T_" . uc($arg);
}

sub grapple {
    my $arg = shift;
    return Dumper($arg);
}

sub paramlist {
    my ($entry, $what) = @_;
    my @params;
    for my $param (@{$entry->{param}}) {
        if($what && $what eq "out") {
            push @params, $param if($param->{out});
        } elsif($what && $what eq "in") {
            push @params, $param unless($param->{out});
        } else {
            push @params, $param;
        }
    }
    \@params;
}

sub arglist {
    my ($first, $list) = @_;
    return join( ", ", $first, map { (defined($_->{array}) ? "" : "&") . $_->{name} } @{$list});
} 

sub namelist {
    my ($first, $list, $prefix) = @_;
    $prefix ||= "";
    return join( ", ", $first, map { $prefix . $_->{name} } @{$list});
} 

sub is_struct {
    my ($type) = shift;
    my ($t);
    while($type && ($t = get_type($type))) {
        return 1 if($t && $t->{struct});
        $type = $t->{base};
    } 
    undef;
}

sub is_empty {
    my ($type) = shift;
    my ($t);
    while($type && ($t = get_type($type))) {
        return 1 if($t && $t->{struct} && !defined($t->{elm}));
        $type = $t->{base};
    } 
    undef;
}

sub namelist_p {
    my ($list) = shift;
    my @args;
    for my $a (@{$list}) {
        push @args, ($a->{out} || is_struct($a->{type})) && !defined($a->{array}) ? "&" . $a->{name} : $a->{name};
    }
    return join(", ", @args);
} 

sub decllist {
    my ($first, $list, $ptr) = @_;
    my @params;
    push @params, $first if($first);
    for my $param (@{$list}) {
        if($ptr) {
            push @params, join(" ",
                               $param->{'type'},
                               ($param->{'array'} ? "$param->{name}\[$param->{array}\]" : "*$param->{name}"));
        } else {
            push @params, join(" ",
                               $param->{'out'} ? "" : "const",
                               $param->{'type'},
                               ref_arg($param, "*"),
                               $param->{'name'},
                               $param->{'array'} ? "\[$param->{'array'}\]" : "");
        }
    }    
    return join ", ", @params;
}

sub nonempty {
    my ($list) = @_;
    return scalar @{$list};
} 

sub get_type {
    my ($name) = @_;
    for my $t (@{$tree->{types}->{type}}) {
        return $t if($t->{name} eq $name);
    }
    undef;
}    

# ---------------
# Main perl logic
# ---------------

#print STDERR Dumper($tree->{types});

my $tt = Template->new({ INCLUDE_PATH => $opts{I},
                         RELATIVE => 1,
                         INTERPOLATE  => 1, 
                       }) || die "$Template::ERROR\n";

my $vars = {
    debug => \&grapple,
    decllist => \&decllist,
    paramlist => \&paramlist,
    arglist => \&arglist,
    namelist => \&namelist,
    namelist_p => \&namelist_p,
    nonempty => \&nonempty,
    msgid => \&msgid,
    is_empty => \&is_empty,
    get_type => \&get_type,
    prefix => $tree->{prefix} ? $tree->{prefix} : "",
    groups => $tree->{group},
    types => $tree->{types},
    events => $tree->{events},
    includes => $tree->{includes},
};

#print Dumper(\@ARGV);
#exit 1;

$tt->process($ARGV[0], $vars, $opts{o}) || die $tt->error(), "\n";
