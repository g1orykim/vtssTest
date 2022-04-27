# 
# Vitesse Switch API software.
# 
# Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
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

use HTML::Parser;
use HTML::TokeParser;

use Data::Dumper;
use Getopt::Std;

my %opt = ( );
getopts("g:dG", \%opt);
# -g <glossary>
# -G: Glossary mode (when indexing the glossary itself)
# -d: debug

die "Must supply terms index with -g <glossary> option\n" unless(defined $opt{g} && -r $opt{g});

my $terms = getTerms($opt{g});

my $p = HTML::Parser->new(api_version => 3,
                          handlers => { 
                              start => [\&start_tag, 'self, tagname, attr, text, line'],
                              text => [\&text_tag, 'self, text, line, is_cdata'],
                              default => [\&default_handler, "self, text"],
                          });

# Main worker look
foreach my $file (map { glob $_ } @ARGV) {
    process($p, $file);
}
exit 0;

sub build_search {
    # NB: Longest match first!
    return  join "|", sort { length($b) <=> length($a) } @_;
}

sub process {
    my($p, $file) = @_;
    print STDERR "Processing $file\n" if($opt{d});
    my $new = $file . ".new";
    $p->{file} = $file;
    open($p->{fd}, ">:raw", $new) || die("$new: $!");
    $p->{terms} = {%{$terms}}; # Copy ref
    $p->{in_body} = 0;          # Only link in body text
    $p->{term_re} = build_search(keys %{$p->{terms}}); # Initial search regexp
    $p->parse_file($file);
    close($p->{fd});
    if(-s $file != -s $new) {
        rename $new, $file;
        print STDERR "$file was updated\n";
    } else {
        unlink $new;
    }
}

sub replacement {
    my ($parser, $match, $tail) = @_;
    my $replace = $parser->{terms}->{lc $match}->{replace};
    $replace =~ s/&&&/$match/;
    $replace .= $tail if(defined $tail);
    return $replace;
}

sub default_handler
{
    my($self, $text) = @_;
    my($fd) = $self->{fd};
    print $fd $text;
}

sub start_tag
{
    my($self, $tagname, $attr, $text, $line) = @_;
    my($fd) = $self->{fd};
    $self->{tag} = $tagname;

    if ($tagname eq "a" &&
        defined $attr->{'href'} &&
        defined $attr->{'class'} &&
        $attr->{'class'} eq "glossary") {
        if($attr->{'href'} =~ /\#(.+)/) {
            my $t = $1;
            $t =~ tr/_/ /;      # Back to original, lower-case form
            delete $self->{terms}->{$t};
            $self->{term_re} = build_search(keys %{$self->{terms}}); # Update search regexp
            printf( STDERR "%s:%d: Already have '%s' linked\n", 
                    $self->{file}, $line, $t) if($opt{d});
        }
    }

    if($opt{G} && $self->{tag} eq 'dd') {
        # Fresh sets for all descriptions
        $self->{terms} = {%{$terms}}; # Copy ref
        $self->{term_re} = build_search(keys %{$self->{terms}}); # 'Initial' search regexp
    }

    $self->{in_body}++ if($tagname eq 'body');

    print $fd $text;
}

sub text_tag
{
    my($self, $text, $line, $is_cdata) = @_;
    my($fd) = $self->{fd};
    if($is_cdata || !$self->{in_body} || 
       !$self->{tag} || $self->{tag} eq 'a' || $self->{tag} eq 'h1') {
        print $fd $text;
        return;
    }
    # Replace with @@<x>@@ token, posthone real text substitution
    my @hits;
    while($text =~ s/\b($self->{term_re})([s])?\b/'@@' . scalar @hits . '@@'/ie) {
        push @hits, replacement $self,$1,$2;
        my ($t) = lc $1;
        printf( STDERR "%s:%d: Found '%s' in %s\n", 
                $self->{file}, $line, $t, $self->{tag});
        delete $self->{terms}->{$t};
        $self->{term_re} = build_search(keys %{$self->{terms}}); # Update search regexp
    }
    # Now do real subsitution (to avoid links in links)
    for my $hit (0..@hits) {
        $text =~ s/\@\@$hit\@\@/$hits[$hit]/;
    }
    print $fd $text;
}

sub getTerms {
    my($file) = @_;
    my ($tp) = HTML::TokeParser->new($file);
    my(%terms);
    while (my $token = $tp->get_tag("a")) {
        my $text = $tp->get_trimmed_text("/a");
        my $search = lc $text;
        my $anchor = $token->[1]{name};
        if($anchor && 
           (!$token->[1]{class} || $token->[1]{class} ne "index")) {
            my $replace = qq(<a href="glossary.htm#$anchor" class="glossary">&&&</a>);
            $terms{$search} = { replace=>$replace, text=>$text, anchor=>$anchor };
            die "Space in anchor $anchor" if($anchor =~ /\s/);
        }
    }
    \%terms;
}
