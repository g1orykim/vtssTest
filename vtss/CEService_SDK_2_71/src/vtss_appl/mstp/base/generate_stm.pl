###############################################################################
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
# ------------------------------------------------------------------------
# 

use warnings;
use strict;
use Data::Dumper;

my($machine, $stmtype, $curstate, @states, %transitions, $global_transitions, %actions, %loopctl, $defguard);

# Tree port attributes
my (@tportattrs) = qw(

 agree agreed designatedPriority designatedTimes disputed fdWhile
 forward forwarding helloTime infoIs learn learning portPriority
 portTimes proposed proposing rbWhile rcvdInfo rcvdInfoWhile rcvdMsg
 rcvdTc reRoot reselect role rrWhile selected selectedRole selecting
 sync synced tcProp tcWhile updtInfo

);

# CIST port attributes
my (@cportattrs) = qw(

 adminEdge edgeDelayWhile helloWhen infoInternal mcheck mdelayWhile
 newInfo newInfoMsti operEdge rcvdBpdu rcvdInternal
 rcvdRSTP rcvdSTP rcvdTcAck rcvdTcn restrictedRole restrictedTcn
 sendRSTP tcAck tick txCount

);

my($re_tportattrs) = join('|', @tportattrs);
my($re_cportattrs) = join('|', @cportattrs);

# 17.20 State machine conditions and parameters
# The following variable evaluations are defined for notational
# convenience in the state machines.
my($stmcond_t) = qr/forwardDelay|EdgeDelay|FwdDelay|HelloTime|MaxAge/;
my($stmcond_c) = qr/AdminEdge|AutoEdge|portEnabled/;

EmitFileHdr();

line:
while(<>) {
    next line if(/^\s*$/ || /^\s*\#/);
    $_ =~ s/\#.+$//;                   # Strip comments (like this)
    if(/stm (\w+)\(([A-Z]+)\)/) {
        $machine = $1;
        $stmtype = lc($2);
        die("Invalid STM type: $stmtype") unless $stmtype =~ /^(msti|port|bridge)$/;
    } elsif(/^define\s+(\S+)\s+(.+)/) {
        my($name, $define) = ($1, Statement($2));
        print "#define $name $define\n";
    } elsif(/^>>GUARD:(.+)/) {
        $defguard = Statement($1);
    } elsif(/^(\w+):/) {
        $curstate = $1;
        push @states, $1 if($1 ne "GLOBAL");
    } elsif(/^\s*loop_protect\s*(\d+)/) {
        $loopctl{$curstate} = $1;
    } elsif(/>>/) {
        if($curstate ne "GLOBAL") {
            $transitions{$curstate} .= $_;
        } else {
            $global_transitions .= $_;
        }
    } else {
        $actions{$curstate} .= $_;
    }
}

my($thisdefine, $context, $enter_trace);
my($trmac) = ($machine =~ /(PortTransmit|PortTimers)/ ? "T_N" : "T_D");
if($stmtype eq "port") {
    $thisdefine = "mstp_port_t *port";
    $context = "mstp_cistport_t *cist __attribute__ ((unused)) = getCist(port);\n";
    $context .= "VTSS_ASSERT(cist == port->cistport);\n";
    $context .= "mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;\n";
    $enter_trace = $trmac . '("%s: Port %d - Enter %s", STM_NAME, port->port_no, stm_statename(state)); ';
} elsif($stmtype eq "msti") {
    $thisdefine = "mstp_port_t *port";
    $context = "mstp_cistport_t *cist __attribute__ ((unused)) = port->cistport;\n";
    $context .= "mstp_bridge_t *bridge __attribute__ ((unused)) = cist->bridge;\n";
    $context .= "struct mstp_tree *tree __attribute__ ((unused)) = port->tree;\n";
    $enter_trace = $trmac . '("%s: Port %d[%d] - Enter %s", STM_NAME, port->port_no, tree->msti, stm_statename(state)); ';
    $stmtype = "port"; # Equal in anything else
} elsif($stmtype eq "bridge") {
    $thisdefine = "struct mstp_tree *bridge";
    $context = "";
    $enter_trace = $trmac . '("%s: MSTI%d, Enter %s", STM_NAME, bridge->msti, stm_statename(state)); ';
}

EmitDefines();

EmitStatenameFunc();

EmitEnterFunc();

EmitBeginFunc();

EmitRunFunc();

EmitTail();

exit 0;

sub EmitFileHdr
{
    print <<"EOF"
/*

 Vitesse Switch API software.

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
 
*/

#include "mstp_priv.h"
#include "mstp_stm.h"
#include "mstp_util.h"

/*lint -esym(438,bridge,tree) -esym(529,bridge,tree) ... may be unused in port STM(s) */
/*lint --e{616, 527, 563} ... generated switch() code */
/*lint --e{801, 825}      ... goto, fallthrough */
/*lint --e{767}           ... STM_NAME multiple defined */
/*lint --e{818}           ... may be defined as const */

EOF
;

}

sub EmitDefines
{
    print << "EOF"
#define STM_NAME \"${machine}\"

enum ${machine}_states {
EOF
    ;

    print join(",\n", map { caselabel($_); } @states), "};\n\n";
}

sub EmitStatenameFunc
{
    print <<"EOF"
static const char *stm_statename(int state)
{
EOF
    ;

    print "switch(state) {\n";
    for my $st (@states) {
        printf "case %s: return \"%s\";\n", caselabel($st), $st;
    }

    print <<"EOF"
default:
        return "-unknown-";
}
}

EOF
    ;
}

sub EmitBeginFunc
{
    print <<"EOF"

static int stm_begin(${thisdefine})
\{
    ${context}
EOF
    ;

    my(@condtrans);
    for $_ (split(/\n+/, $transitions{'BEGIN'})) {
        chomp;
        if(/^\s*>>\s*(\w+)/) {
            printf "return stm_enter(${stmtype}, NULL, %s);\n", caselabel($1);
        } elsif(/^\s*when\s*(.+)>>\s*(\w+)/) {
            my($cond, $state) = (Statement($1), $2);
            push @condtrans, sprintf 
                "if(%s) return stm_enter(${stmtype}, NULL, %s);\n", $cond, caselabel($state);
        } else {
            die $_;
        }
    }
    if(@condtrans) {
        print @condtrans;
        print <<"EOF"
    T_E("%s: Unable to transition from BEGIN?", STM_NAME);
    VTSS_ABORT();
    return -1;
EOF
    ;
    }
    print "}\n\n";
}

sub EmitEnterFunc
{
    die "Actions given for BEGIN transition?" if($actions{'BEGIN'});

    print <<"EOF"
static int stm_enter(${thisdefine}, uint *transitions, int state)
\{
    ${context}${enter_trace}
    switch(state) \{
EOF
    ;

    for my $st (@states) {
        if($actions{$st}) {
            printf "case %s:\n", caselabel($st);
            for $_ (split(/\n+/, $actions{$st})) {
                print Statement($_), "\n";
            }
            print "break;\n";
        }
    }
    print "default:;\n\}\n\tif(transitions)\n(*transitions)++\n;return state;\}\n";
}
    
sub EmitRunFunc
{
    print <<"EOF"
static int stm_run(${thisdefine}, uint *transitions, int state)
\{
    uint loop_count = 0;
    ${context}switch(state) \{
EOF
    ;

state:
    for my $st (@states) {
        next state if($st eq "BEGIN");
        printf "%s_ENTER : __attribute__ ((unused))\n", caselabel($st);
        printf "case %s:\n", caselabel($st);
        die "No state transits for $st" unless($transitions{$st});
        my(@condtrans);
        if($loopctl{$st}) {
            my($maxl) = $loopctl{$st};
            printf "LOOP_PROTECT(state, %d);\n", $maxl;
        }
        for $_ (split(/\n+/, $transitions{$st})) {
            chomp;
            if(/^\s*>>\s*(\w+)/) {
                printf "NewState(%s, transitions, state, %s);\n", $stmtype, caselabel($1);
            } elsif(/^\s*when\s*(.+)>>\s*(\w+)/) {
                my($cond, $state) = (Statement($1), $2);
                push @condtrans, sprintf 
                    "if(%s) NewState(%s, transitions, state, %s);\n", $cond, $stmtype, caselabel($state);
            } else {
                die $_;
            }
        }
        if(@condtrans) {
            print "if(", $defguard, ") { /* DEFAULT GUARD */\n" if($defguard);
            print @condtrans;
            print "}\n", if($defguard);
        }
        print "break;\n";
    }
    print <<"EOF"
      default:
        T_E("%s: Entering illegal state %d (%s)", STM_NAME, state, stm_statename(state));
        VTSS_ABORT();
    \}
EOF
;
    if($global_transitions) {
        print "\t/* Global Transitions */\n";
        printf "if(%s) { /* DEFAULT GUARD */\n", $defguard if($defguard);
        for $_ (split(/\n+/, $global_transitions)) {
            chomp;
            if(/^\s*when\s*(.+)>>\s*(\w+)/) {
                my($cond, $state) = (Statement($1), caselabel($2));
                print <<"EOF"
if($cond)
 return stm_enter(${stmtype}, transitions, $state);
EOF
    ;
            } else {
                die $_;
            }
        }
        printf "}\n", $defguard if($defguard);
    }

    print <<"EOF"
    return state;
    \}
EOF
;

}
    
sub Statement
{
    my($s) = @_;
    $s =~ s/\b($re_cportattrs)\b/cist->${1}/g;
    $s =~ s/\b($re_tportattrs)\b/port->${1}/g;
    $s =~ s/\b($stmcond_t)\b(?!\()/${1}\(port)/g;
    $s =~ s/\b($stmcond_c)\b(?!\()/${1}\(cist)/g;
    $s;
}

sub caselabel
{
    my($l) = @_;
    return uc($machine) . '_' . $l;
}

sub EmitTail
{

    print <<"EOF"

const ${stmtype}_stm_t ${machine}_stm = {
    .name = STM_NAME,
    .begin = stm_begin,
    .run = stm_run,
    .statename = stm_statename,
};

EOF
    ;
}
